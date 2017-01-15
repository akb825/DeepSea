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

#include "MockResourceManager.h"

#include "MockDrawGeometry.h"
#include "MockGfxBuffer.h"
#include "MockTexture.h"
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
	if (allocator->freeFunc)
		resourceManager->allocator = allocator;
	else
		resourceManager->allocator = NULL;
	resourceManager->maxResourceContexts = 1;
	resourceManager->minMappingAlignment = 16;
	resourceManager->supportedBuffers = (dsGfxBufferUsage)(dsGfxBufferUsage_Index |
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Indirect | dsGfxBufferUsage_UniformBlock |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_Sampler | dsGfxBufferUsage_CopyFrom |
		dsGfxBufferUsage_CopyTo);
	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Persistent;
	resourceManager->maxIndexBits = 32;
	resourceManager->maxVertexAttribs = 16;
	resourceManager->supportsInstancedDrawing = true;
	resourceManager->maxTextureSize = 4096;
	resourceManager->maxTextureDepth = 256;
	resourceManager->maxTextureArrayLevels = 512;
	resourceManager->arbitraryMipmapping = true;
	resourceManager->texturesReadable = true;

	resourceManager->vertexFormatSupportedFunc = &vertexFormatSupported;
	resourceManager->textureFormatSupportedFunc = &textureFormatSupported;
	resourceManager->offscreenFormatSupportedFunc = &offscreenFormatSupported;
	resourceManager->createResourceContextFunc = &createResourceContext;
	resourceManager->destroyResourceContextFunc = &destroyResourceContext;

	resourceManager->createBufferFunc = (dsCreateGfxBufferFunction)&dsMockGfxBuffer_create;
	resourceManager->destroyBufferFunc = (dsDestroyGfxBufferFunction)&dsMockGfxBuffer_destroy;
	resourceManager->mapBufferFunc = (dsMapGfxBufferFunction)&dsMockGfxBuffer_map;
	resourceManager->unmapBufferFunc = (dsUnmapGfxBufferFunction)&dsMockGfxBuffer_unmap;
	resourceManager->flushBufferFunc = (dsFlushGfxBufferFunction)&dsMockGfxBuffer_flush;
	resourceManager->invalidateBufferFunc =
		(dsInvalidateGfxBufferFunction)&dsMockGfxBuffer_invalidate;
	resourceManager->copyBufferDataFunc = (dsCopyGfxBufferDataFunction)&dsMockGfxBuffer_copyData;
	resourceManager->copyBufferFunc = (dsCopyGfxBufferFunction)&dsMockGfxBuffer_copy;

	resourceManager->createGeometryFunc = (dsCreateDrawGeometryFunction)&dsMockDrawGeometry_create;
	resourceManager->destroyGeometryFunc =
		(dsDestroyDrawGeometryFunction)&dsMockDrawGeometry_destroy;

	resourceManager->createTextureFunc = (dsCreateTextureFunction)&dsMockTexture_create;
	resourceManager->createOffscreenFunc =
		(dsCreateOffscreenFunction)&dsMockTexture_createOffscreen;
	resourceManager->destroyTextureFunc = (dsDestroyTextureFunction)&dsMockTexture_destroy;
	resourceManager->copyTextureDataFunc = (dsCopyTextureDataFunction)&dsMockTexture_copyData;
	resourceManager->copyTextureFunc = (dsCopyTextureFunction)&dsMockTexture_copy;
	resourceManager->blitTextureFunc = (dsBlitTextureFunction)&dsMockTexture_blit;
	resourceManager->getTextureDataFunc = (dsGetTextureDataFunction)&dsMockTexture_getData;

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
