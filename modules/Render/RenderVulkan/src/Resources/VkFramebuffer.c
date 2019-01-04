/*
 * Copyright 2018-2019 Aaron Barany
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

#include "Resources/VkFramebuffer.h"

#include "Resources/VkRealFramebuffer.h"
#include "Resources/VkResourceManager.h"
#include "VkRendererInternal.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

dsFramebuffer* dsVkFramebuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* name, const dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, uint32_t layers)
{
	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*surfaceCount);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkFramebuffer* framebuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkFramebuffer);
	DS_ASSERT(framebuffer);

	dsFramebuffer* baseFramebuffer = (dsFramebuffer*)framebuffer;
	baseFramebuffer->resourceManager = resourceManager;
	baseFramebuffer->allocator = dsAllocator_keepPointer(allocator);
	baseFramebuffer->name = name;
	if (surfaceCount > 0)
	{
		baseFramebuffer->surfaces = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsFramebufferSurface, surfaceCount);
		DS_ASSERT(baseFramebuffer->surfaces);
		memcpy(baseFramebuffer->surfaces, surfaces, sizeof(dsFramebufferSurface)*surfaceCount);
	}
	else
		baseFramebuffer->surfaces = NULL;
	baseFramebuffer->surfaceCount = surfaceCount;
	baseFramebuffer->width = width;
	baseFramebuffer->height = height;
	baseFramebuffer->layers = layers;

	framebuffer->scratchAllocator = resourceManager->allocator;
	dsSpinlock_initialize(&framebuffer->lock);
	framebuffer->realFramebuffers = NULL;
	framebuffer->framebufferCount = 0;
	framebuffer->maxFramebuffers = 0;

	return baseFramebuffer;
}

bool dsVkFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	for (uint32_t i = 0; i < vkFramebuffer->framebufferCount; ++i)
		dsVkRenderer_deleteFramebuffer(renderer, vkFramebuffer->realFramebuffers[i]);

	DS_VERIFY(dsAllocator_free(vkFramebuffer->scratchAllocator, vkFramebuffer->realFramebuffers));
	if (framebuffer->allocator)
		DS_VERIFY(dsAllocator_free(framebuffer->allocator, framebuffer));
	return true;
}

dsVkRealFramebuffer* dsVkFramebuffer_getRealFramebuffer(dsFramebuffer* framebuffer,
	VkRenderPass renderPass, bool update)
{
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	DS_VERIFY(dsSpinlock_lock(&vkFramebuffer->lock));

	for (uint32_t i = 0; i < vkFramebuffer->framebufferCount; ++i)
	{
		dsVkRealFramebuffer* realFramebuffer = vkFramebuffer->realFramebuffers[i];
		if (realFramebuffer->renderPass == renderPass)
		{
			if (update)
			{
				dsVkRealFramebuffer_updateRenderSurfaceImages(realFramebuffer,
					framebuffer->surfaces, framebuffer->surfaceCount);
			}
			DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));
			return realFramebuffer;
		}
	}

	uint32_t index = vkFramebuffer->framebufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(vkFramebuffer->scratchAllocator, vkFramebuffer->realFramebuffers,
		vkFramebuffer->framebufferCount, vkFramebuffer->maxFramebuffers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));
		return NULL;
	}

	dsVkRealFramebuffer* realFramebuffer = dsVkRealFramebuffer_create(framebuffer->resourceManager,
		vkFramebuffer->scratchAllocator, renderPass, framebuffer->surfaces,
		framebuffer->surfaceCount, framebuffer->width, framebuffer->height, framebuffer->layers);
	vkFramebuffer->realFramebuffers[index] = realFramebuffer;

	DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));
	return realFramebuffer;
}
