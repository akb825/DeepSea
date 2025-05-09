/*
 * Copyright 2016-2025 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Resources/MockResourceManager.h"

#include "Resources/MockDrawGeometry.h"
#include "Resources/MockFramebuffer.h"
#include "Resources/MockGfxBuffer.h"
#include "Resources/MockGfxFence.h"
#include "Resources/MockGfxQueryPool.h"
#include "Resources/MockMaterialDesc.h"
#include "Resources/MockRenderbuffer.h"
#include "Resources/MockShader.h"
#include "Resources/MockShaderModule.h"
#include "Resources/MockTexture.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Resources/DefaultShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

static bool vertexFormatSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_specialIndex(format) && !dsGfxFormat_compressedIndex(format);
}

static bool textureFormatSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(format);
	return true;
}

static bool textureBufferFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_compressedIndex(format) && !dsGfxFormat_specialIndex(format);
}

static bool imageFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_compressedIndex(format) &&
		(!dsGfxFormat_specialIndex(format) || format == dsGfxFormat_B10G11R11_UFloat);
}

static bool renderTargetFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_compressedIndex(format);
}

static bool copyFormatsSupported(const dsResourceManager* resourceManager, dsGfxFormat srcFormat,
	dsGfxFormat dstFormat)
{
	return textureFormatSupported(resourceManager, srcFormat) &&
		textureFormatSupported(resourceManager, dstFormat) && srcFormat == dstFormat;
}

static bool blitFormatsSupported(const dsResourceManager* resourceManager, dsGfxFormat srcFormat,
	dsGfxFormat dstFormat, dsBlitFilter filter)
{
	return renderTargetFormatSupported(resourceManager, srcFormat) &&
		renderTargetFormatSupported(resourceManager, dstFormat) && srcFormat == dstFormat &&
		filter == dsBlitFilter_Nearest;
}

static bool generateMipmapsFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_compressedIndex(format) && !dsGfxFormat_specialIndex(format);
}

static bool copyBufferTextureSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	return textureFormatSupported(resourceManager, format);
}

static dsResourceContext* acquireResourceContext(dsResourceManager* resourceManager)
{
	DS_ASSERT(resourceManager && resourceManager->allocator);
	return (dsResourceContext*)dsAllocator_alloc(resourceManager->allocator, 1);
}

static bool releaseResourceContext(dsResourceManager* resourceManager, dsResourceContext* context)
{
	DS_ASSERT(resourceManager && resourceManager->allocator && context);
	return dsAllocator_free(resourceManager->allocator, context);
}

dsResourceManager* dsMockResourceManager_create(dsRenderer* renderer, dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	dsResourceManager* resourceManager = DS_ALLOCATE_OBJECT(allocator, dsResourceManager);
	if (!resourceManager)
		return NULL;

	if (!dsResourceManager_initialize(resourceManager))
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, resourceManager);
		return NULL;
	}

	resourceManager->renderer = renderer;
	resourceManager->allocator = dsAllocator_keepPointer(allocator);
	resourceManager->maxResourceContexts = 1;
	resourceManager->minNonCoherentMappingAlignment = 16;
	resourceManager->minTextureBufferAlignment = 16;
	resourceManager->minUniformBlockAlignment = 16;
	resourceManager->minUniformBufferAlignment = 16;
	resourceManager->supportedBuffers = (dsGfxBufferUsage)(dsGfxBufferUsage_Index |
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_IndirectDraw |
		dsGfxBufferUsage_IndirectDispatch | dsGfxBufferUsage_UniformBlock |
		dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Texture | dsGfxBufferUsage_Image |
		dsGfxBufferUsage_CopyFrom | dsGfxBufferUsage_CopyTo);
	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Persistent;
	resourceManager->canCopyBuffers = true;
	resourceManager->hasTextureBufferSubrange = true;
	resourceManager->maxIndexSize = (uint32_t)sizeof(uint32_t);
	resourceManager->maxUniformBlockSize = 1024*1024*1024;
	resourceManager->maxTextureBufferElements = 16*1024*1024;
	resourceManager->maxVertexAttribs = 16;
	resourceManager->maxSamplers = 16;
	resourceManager->maxVertexSamplers = 16;
	resourceManager->maxTextureSize = 4096;
	resourceManager->maxTextureDepth = 256;
	resourceManager->maxTextureArrayLevels = 512;
	resourceManager->maxRenderbufferSize = 4096;
	resourceManager->maxFramebufferLayers = 1024;
	resourceManager->hasArbitraryMipmapping = true;
	resourceManager->hasCubeArrays = true;
	resourceManager->has3DTextureOffscreens = true;
	resourceManager->maxTextureSamples = 16;
	resourceManager->texturesReadable = true;
	resourceManager->requiresColorBuffer = false;
	resourceManager->requiresAnySurface = false;
	resourceManager->canMixWithRenderSurface = true;
	resourceManager->hasVertexPipelineWrites = true;
	resourceManager->hasFragmentWrites = true;
	resourceManager->maxComputeLocalWorkGroupSize[0] = 256;
	resourceManager->maxComputeLocalWorkGroupSize[1] = 256;
	resourceManager->maxComputeLocalWorkGroupSize[2] = 256;
	resourceManager->maxComputeLocalWorkGroupInvocations = 1024;
	resourceManager->maxClipDistances = 8;
	resourceManager->maxCullDistances = 8;
	resourceManager->maxCombinedClipAndCullDistances = 8;
	resourceManager->hasFences = true;
	resourceManager->hasQueries = true;
	resourceManager->hasPreciseOcclusionQueries = true;
	resourceManager->hasSecondaryCommandBufferOcclusionQueries = true;
	resourceManager->has64BitQueries = true;
	resourceManager->hasQueryBuffers = true;
	resourceManager->timestampPeriod = 1.0f;

	resourceManager->vertexFormatSupportedFunc = &vertexFormatSupported;
	resourceManager->textureFormatSupportedFunc = &textureFormatSupported;
	resourceManager->textureBufferFormatSupportedFunc = &textureBufferFormatSupported;
	resourceManager->imageFormatSupportedFunc = &imageFormatSupported;
	resourceManager->renderTargetFormatSupportedFunc = &renderTargetFormatSupported;
	resourceManager->generateMipmapFormatSupportedFunc = &generateMipmapsFormatSupported;
	resourceManager->textureCopyFormatsSupportedFunc = &copyFormatsSupported;
	resourceManager->surfaceBlitFormatsSupportedFunc = &blitFormatsSupported;
	resourceManager->copyBufferToTextureSupportedFunc = &copyBufferTextureSupported;
	resourceManager->copyTextureToBufferSupportedFunc = &copyBufferTextureSupported;
	resourceManager->acquireResourceContextFunc = &acquireResourceContext;
	resourceManager->releaseResourceContextFunc = &releaseResourceContext;

	resourceManager->createBufferFunc = &dsMockGfxBuffer_create;
	resourceManager->destroyBufferFunc = &dsMockGfxBuffer_destroy;
	resourceManager->mapBufferFunc = &dsMockGfxBuffer_map;
	resourceManager->unmapBufferFunc = &dsMockGfxBuffer_unmap;
	resourceManager->flushBufferFunc = &dsMockGfxBuffer_flush;
	resourceManager->invalidateBufferFunc = &dsMockGfxBuffer_invalidate;
	resourceManager->copyBufferDataFunc = &dsMockGfxBuffer_copyData;
	resourceManager->copyBufferFunc = &dsMockGfxBuffer_copy;
	resourceManager->copyBufferToTextureFunc = dsMockGfxBuffer_copyToTexture;

	resourceManager->createGeometryFunc = &dsMockDrawGeometry_create;
	resourceManager->destroyGeometryFunc = &dsMockDrawGeometry_destroy;

	resourceManager->createTextureFunc = &dsMockTexture_create;
	resourceManager->createOffscreenFunc = &dsMockTexture_createOffscreen;
	resourceManager->destroyTextureFunc = &dsMockTexture_destroy;
	resourceManager->copyTextureDataFunc = &dsMockTexture_copyData;
	resourceManager->copyTextureFunc = &dsMockTexture_copy;
	resourceManager->copyTextureToBufferFunc = &dsMockTexture_copyToBuffer;
	resourceManager->generateTextureMipmapsFunc = &dsMockTexture_generateMipmaps;
	resourceManager->getTextureDataFunc = &dsMockTexture_getData;

	resourceManager->createRenderbufferFunc = &dsMockRenderbuffer_create;
	resourceManager->destroyRenderbufferFunc = &dsMockRenderbuffer_destroy;

	resourceManager->createFramebufferFunc = &dsMockFramebuffer_create;
	resourceManager->destroyFramebufferFunc = &dsMockFramebuffer_destroy;

	resourceManager->createFenceFunc = &dsMockGfxFence_create;
	resourceManager->destroyFenceFunc = &dsMockGfxFence_destroy;
	resourceManager->setFencesFunc = &dsMockGfxFence_set;
	resourceManager->waitFenceFunc = &dsMockGfxFence_wait;
	resourceManager->resetFenceFunc = &dsMockGfxFence_reset;

	resourceManager->createQueryPoolFunc = &dsMockGfxQueryPool_create;
	resourceManager->destroyQueryPoolFunc = &dsMockGfxQueryPool_destroy;
	resourceManager->resetQueryPoolFunc = &dsMockGfxQueryPool_reset;
	resourceManager->beginQueryFunc = &dsMockGfxQueryPool_beginQuery;
	resourceManager->endQueryFunc = &dsMockGfxQueryPool_endQuery;
	resourceManager->queryTimestampFunc = &dsMockGfxQueryPool_queryTimestamp;
	resourceManager->getQueryValuesFunc = &dsMockGfxQueryPool_getValues;
	resourceManager->copyQueryValuesFunc = &dsMockGfxQueryPool_copyValues;

	resourceManager->createShaderModuleFunc = &dsMockShaderModule_create;
	resourceManager->destroyShaderModuleFunc = &dsMockShaderModule_destroy;

	resourceManager->createMaterialDescFunc = &dsMockMaterialDesc_create;
	resourceManager->destroyMaterialDescFunc = &dsMockMaterialDesc_destroy;

	resourceManager->createShaderVariableGroupDescFunc = &dsDefaultShaderVariableGroupDesc_create;
	resourceManager->destroyShaderVariableGroupDescFunc = &dsDefaultShaderVariableGroupDesc_destroy;

	resourceManager->createShaderFunc = &dsMockShader_create;
	resourceManager->destroyShaderFunc = &dsMockShader_destroy;
	resourceManager->bindShaderFunc = &dsMockShader_bind;
	resourceManager->updateShaderInstanceValuesFunc = &dsMockShader_updateInstanceValues;
	resourceManager->updateShaderDynamicRenderStatesFunc = &dsMockShader_updateDynamicRenderStates;
	resourceManager->unbindShaderFunc = &dsMockShader_unbind;
	resourceManager->bindComputeShaderFunc = &dsMockShader_bindCompute;
	resourceManager->updateComputeShaderInstanceValuesFunc = &dsMockShader_updateInstanceValues;
	resourceManager->unbindComputeShaderFunc = &dsMockShader_unbind;

	return resourceManager;
}

void dsMockResourceManager_destroy(dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsResourceManager_shutdown(resourceManager);
	if (resourceManager->allocator)
		dsAllocator_free(resourceManager->allocator, resourceManager);
}
