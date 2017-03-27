/*
 * Copyright 2016 Aaron Barany
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
#include "Resources/MockMaterialDesc.h"
#include "Resources/MockRenderbuffer.h"
#include "Resources/MockShader.h"
#include "Resources/MockShaderModule.h"
#include "Resources/MockShaderVariableGroupDesc.h"
#include "Resources/MockTexture.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
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

static bool offscreenFormatSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	DS_UNUSED(resourceManager);
	return !dsGfxFormat_compressedIndex(format);
}

static dsResourceContext* createResourceContext(dsResourceManager* resourceManager)
{
	DS_ASSERT(resourceManager && resourceManager->allocator);
	return (dsResourceContext*)dsAllocator_alloc(resourceManager->allocator, 1);
}

static bool destroyResourceContext(dsResourceManager* resourceManager, dsResourceContext* context)
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

	dsResourceManager* resourceManager = (dsResourceManager*)dsAllocator_alloc(allocator,
		sizeof(dsResourceManager));
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
	resourceManager->minMappingAlignment = 16;
	resourceManager->supportedBuffers = (dsGfxBufferUsage)(dsGfxBufferUsage_Index |
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Indirect | dsGfxBufferUsage_UniformBlock |
		dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Image | dsGfxBufferUsage_Sampler |
		dsGfxBufferUsage_CopyFrom | dsGfxBufferUsage_CopyTo);
	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Persistent;
	resourceManager->maxIndexBits = 32;
	resourceManager->maxVertexAttribs = 16;
	resourceManager->supportsInstancedDrawing = true;
	resourceManager->maxTextureSize = 4096;
	resourceManager->maxTextureDepth = 256;
	resourceManager->maxTextureArrayLevels = 512;
	resourceManager->maxFramebufferLayers = 1024;
	resourceManager->arbitraryMipmapping = true;
	resourceManager->texturesReadable = true;
	resourceManager->requiresColorBuffer = false;
	resourceManager->canMixWithRenderSurface = true;

	resourceManager->vertexFormatSupportedFunc = &vertexFormatSupported;
	resourceManager->textureFormatSupportedFunc = &textureFormatSupported;
	resourceManager->offscreenFormatSupportedFunc = &offscreenFormatSupported;
	resourceManager->createResourceContextFunc = &createResourceContext;
	resourceManager->destroyResourceContextFunc = &destroyResourceContext;

	resourceManager->createBufferFunc = &dsMockGfxBuffer_create;
	resourceManager->destroyBufferFunc = &dsMockGfxBuffer_destroy;
	resourceManager->mapBufferFunc = &dsMockGfxBuffer_map;
	resourceManager->unmapBufferFunc = &dsMockGfxBuffer_unmap;
	resourceManager->flushBufferFunc = &dsMockGfxBuffer_flush;
	resourceManager->invalidateBufferFunc = &dsMockGfxBuffer_invalidate;
	resourceManager->copyBufferDataFunc = &dsMockGfxBuffer_copyData;
	resourceManager->copyBufferFunc = &dsMockGfxBuffer_copy;

	resourceManager->createGeometryFunc = &dsMockDrawGeometry_create;
	resourceManager->destroyGeometryFunc = &dsMockDrawGeometry_destroy;

	resourceManager->createTextureFunc = &dsMockTexture_create;
	resourceManager->createOffscreenFunc = &dsMockTexture_createOffscreen;
	resourceManager->destroyTextureFunc = &dsMockTexture_destroy;
	resourceManager->copyTextureDataFunc = &dsMockTexture_copyData;
	resourceManager->copyTextureFunc = &dsMockTexture_copy;
	resourceManager->blitTextureFunc = &dsMockTexture_blit;
	resourceManager->getTextureDataFunc = &dsMockTexture_getData;

	resourceManager->createRenderbufferFunc = &dsMockRenderbuffer_create;
	resourceManager->destroyRenderbufferFunc = &dsMockRenderbuffer_destroy;

	resourceManager->createFramebufferFunc = &dsMockFramebuffer_create;
	resourceManager->destroyFramebufferFunc = &dsMockFramebuffer_destroy;

	resourceManager->createShaderModuleFunc = &dsMockShaderModule_create;
	resourceManager->destroyShaderModuleFunc = &dsMockShaderModule_destroy;

	resourceManager->createMaterialDescFunc = &dsMockMaterialDesc_create;
	resourceManager->destroyMaterialDescFunc = &dsMockMaterialDesc_destroy;

	resourceManager->createShaderVariableGroupDescFunc = &dsMockShaderVariableGroupDesc_create;
	resourceManager->destroyShaderVariableGroupDescFunc = &dsMockShaderVariableGroupDesc_destroy;

	resourceManager->createShaderFunc = &dsMockShader_create;
	resourceManager->destroyShaderFunc = &dsMockShader_destroy;

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
