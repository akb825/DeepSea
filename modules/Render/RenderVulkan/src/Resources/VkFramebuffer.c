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
#include "VkRenderPassData.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

static const dsRenderSurface* getRenderSurface(const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount)
{
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		switch (surfaces[i].surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			case dsGfxSurfaceType_DepthRenderSurface:
			case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			case dsGfxSurfaceType_DepthRenderSurfaceRight:
				return (dsRenderSurface*)surfaces->surface;
			default:
				continue;
		}
	}

	return NULL;
}

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

	framebuffer->renderSurface = getRenderSurface(surfaces, surfaceCount);

	framebuffer->lifetime = dsLifetime_create(allocator, framebuffer);
	if (!framebuffer->lifetime)
	{
		dsVkFramebuffer_destroy(resourceManager, baseFramebuffer);
		return NULL;
	}

	return baseFramebuffer;
}

bool dsVkFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&vkFramebuffer->lock));
	dsVkRealFramebuffer** framebuffers = vkFramebuffer->realFramebuffers;
	uint32_t framebufferCount = vkFramebuffer->framebufferCount;
	vkFramebuffer->realFramebuffers = NULL;
	vkFramebuffer->framebufferCount = 0;
	vkFramebuffer->maxFramebuffers = 0;
	DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));

	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		dsVkRenderPassData* renderPass =
			(dsVkRenderPassData*)dsLifetime_acquire(framebuffers[i]->renderPassData);
		if (renderPass)
		{
			dsVkRenderPassData_removeFramebuffer(renderPass, framebuffer);
			dsLifetime_release(framebuffers[i]->renderPassData);
		}
		dsVkRenderer_deleteFramebuffer(renderer, framebuffers[i]);
	}
	DS_VERIFY(dsAllocator_free(vkFramebuffer->scratchAllocator, framebuffers));
	DS_ASSERT(!vkFramebuffer->realFramebuffers);

	dsLifetime_destroy(vkFramebuffer->lifetime);

	DS_VERIFY(dsAllocator_free(vkFramebuffer->scratchAllocator, vkFramebuffer->realFramebuffers));
	if (framebuffer->allocator)
		DS_VERIFY(dsAllocator_free(framebuffer->allocator, framebuffer));
	return true;
}

dsVkRealFramebuffer* dsVkFramebuffer_getRealFramebuffer(dsFramebuffer* framebuffer,
	const dsVkRenderPassData* renderPass)
{
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	const dsVkRenderSurfaceData* renderSurface = NULL;
	if (vkFramebuffer->renderSurface)
		renderSurface = ((dsVkRenderSurface*)vkFramebuffer->renderSurface)->surfaceData;

	DS_VERIFY(dsSpinlock_lock(&vkFramebuffer->lock));

	for (uint32_t i = 0; i < vkFramebuffer->framebufferCount; ++i)
	{
		dsVkRealFramebuffer* realFramebuffer = vkFramebuffer->realFramebuffers[i];
		if (dsLifetime_getObject(realFramebuffer->renderPassData) == renderPass)
		{
			if (realFramebuffer->renderSurface != renderSurface)
			{
				realFramebuffer = dsVkRealFramebuffer_create(vkFramebuffer->scratchAllocator,
					framebuffer, renderPass, renderSurface);
				if (realFramebuffer)
				{
					dsVkRenderer_deleteFramebuffer(framebuffer->resourceManager->renderer,
						vkFramebuffer->realFramebuffers[i]);
					vkFramebuffer->realFramebuffers[i] = realFramebuffer;
				}
				else
					realFramebuffer = NULL;
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

	dsVkRealFramebuffer* realFramebuffer = dsVkRealFramebuffer_create(
		vkFramebuffer->scratchAllocator, framebuffer, renderPass, renderSurface);;
	if (!realFramebuffer)
	{
		--vkFramebuffer->framebufferCount;
		DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));
		return NULL;
	}

	vkFramebuffer->realFramebuffers[index] = realFramebuffer;

	DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));

	dsVkRenderPassData_addFramebuffer((dsVkRenderPassData*)renderPass, framebuffer);
	return realFramebuffer;
}

void dsVkFramebuffer_removeRenderPass(dsFramebuffer* framebuffer,
	const dsVkRenderPassData* renderPass)
{
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	DS_VERIFY(dsSpinlock_lock(&vkFramebuffer->lock));
	for (uint32_t i = 0; i < vkFramebuffer->framebufferCount; ++i)
	{
		void* usedRenderPass =
			dsLifetime_getObject(vkFramebuffer->realFramebuffers[i]->renderPassData);
		DS_ASSERT(usedRenderPass);
		if (usedRenderPass == renderPass)
		{
			dsVkRenderer_deleteFramebuffer(framebuffer->resourceManager->renderer,
				vkFramebuffer->realFramebuffers[i]);
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(vkFramebuffer->realFramebuffers,
				vkFramebuffer->framebufferCount, i, 1));
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkFramebuffer->lock));
}
