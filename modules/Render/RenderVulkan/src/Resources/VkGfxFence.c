/*
 * Copyright 2018 Aaron Barany
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

#include "Resources/VkGfxFence.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkResource.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>

dsGfxFence* dsVkGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator)
{
	dsVkGfxFence* fence = DS_ALLOCATE_OBJECT(allocator, dsVkGfxFence);
	if (!fence)
		return NULL;

	dsGfxFence* baseFence = (dsGfxFence*)fence;
	baseFence->resourceManager = resourceManager;
	baseFence->allocator = dsAllocator_keepPointer(allocator);

	dsVkResource_initialize(&fence->resource);
	return baseFence;
}

bool dsVkGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback)
{
	DS_UNUSED(resourceManager);
	for (uint32_t i = 0; i < fenceCount; ++i)
	{
		dsVkGfxFence* vkFence = (dsVkGfxFence*)(fences + i);
		if (!dsVkCommandBuffer_addResource(commandBuffer, &vkFence->resource))
			return false;
	}

	dsVkCommandBuffer_submitFence(commandBuffer, bufferReadback);
	return true;
}

dsGfxFenceResult dsVkGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout)
{
	dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;
	DS_VERIFY(dsSpinlock_lock(&vkFence->resource.lock));
	uint64_t submit = vkFence->resource.lastUsedSubmit;
	DS_VERIFY(dsSpinlock_unlock(&vkFence->resource.lock));
	return dsVkRenderer_waitForSubmit(resourceManager->renderer, submit, timeout);
}

bool dsVkGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_UNUSED(resourceManager);
	dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;
	DS_VERIFY(dsSpinlock_lock(&vkFence->resource.lock));
	vkFence->resource.lastUsedSubmit = DS_NOT_SUBMITTED;
	DS_VERIFY(dsSpinlock_unlock(&vkFence->resource.lock));
	return true;
}

bool dsVkGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	dsVkRenderer_deleteFence(resourceManager->renderer, fence);
	return true;
}

void dsVkGfxFence_destroyImpl(dsGfxFence* fence)
{
	dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;
	dsVkResource_shutdown(&vkFence->resource);
	if (fence->allocator)
		DS_VERIFY(dsAllocator_free(fence->allocator, fence));
}
