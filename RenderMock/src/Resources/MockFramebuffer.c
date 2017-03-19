/*
 * Copyright 2017 Aaron Barany
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

#include "Resources/MockFramebuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsFramebuffer* dsMockFramebuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsFramebufferSurface* surfaces, uint32_t surfaceCount,
	uint32_t width, uint32_t height, uint32_t layers)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*surfaceCount);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, bufferSize));

	dsFramebuffer* framebuffer = (dsFramebuffer*)dsAllocator_alloc(
		(dsAllocator*)&bufferAllocator, sizeof(dsFramebuffer));
	DS_ASSERT(framebuffer);
	framebuffer->resourceManager = resourceManager;
	framebuffer->allocator = dsAllocator_keepPointer(allocator);

	if (surfaceCount)
	{
		framebuffer->surfaces = (dsFramebufferSurface*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsFramebufferSurface)*surfaceCount);
		DS_ASSERT(framebuffer->surfaces);
		memcpy(framebuffer->surfaces, surfaces, sizeof(dsFramebufferSurface)*surfaceCount);
	}
	else
		framebuffer->surfaces = NULL;
	framebuffer->surfaceCount = surfaceCount;
	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->arrayLayers = layers;
	return framebuffer;
}

bool dsMockFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(framebuffer);
	if (framebuffer->allocator)
		return dsAllocator_free(framebuffer->allocator, framebuffer);
	return true;
}
