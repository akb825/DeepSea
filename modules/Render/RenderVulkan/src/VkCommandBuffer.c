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

#include "VkCommandBuffer.h"
#include "VkBarrierList.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;

	memset(commandBuffer, 0, sizeof(*commandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;
	dsVkBarrierList_initialize(&commandBuffer->barriers, allocator, &vkRenderer->device);
}

bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkCommandBuffer* vkSubmitBuffer = (dsVkCommandBuffer*)submitBuffer;

	// Copy over the used resources.
	uint32_t offset = vkCommandBuffer->usedResourceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->usedResources,
		vkCommandBuffer->usedResourceCount, vkCommandBuffer->maxUsedResources,
		vkSubmitBuffer->usedResourceCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < vkSubmitBuffer->usedResourceCount; ++i)
	{
		dsVkResource* resource = vkSubmitBuffer->usedResources[i];
		DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, 1);
		vkCommandBuffer->usedResources[offset + i] = resource;
	}

	DS_VK_CALL(device->vkCmdExecuteCommands)(vkCommandBuffer->vkCommandBuffer, 1,
		&vkSubmitBuffer->vkCommandBuffer);

	// Reset immediately if not submitted multiple times. This frees any internal references to
	// resources.
	if (!(submitBuffer->usage &
		(dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
	{
		dsVkCommandBuffer_clearUsedResources(submitBuffer);
	}

	return true;
}

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->usedResourceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->usedResources,
		vkCommandBuffer->usedResourceCount, vkCommandBuffer->maxUsedResources, 1))
	{
		return false;
	}

	vkCommandBuffer->usedResources[index] = resource;
	DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, 1);
	return true;
}

void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
		DS_ATOMIC_FETCH_ADD32(&vkCommandBuffer->usedResources[i]->commandBufferCount, -1);

	vkCommandBuffer->usedResourceCount = 0;
}

void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
	{
		dsVkResource* resource = vkCommandBuffer->usedResources[i];
		DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
		resource->lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
	}

	vkCommandBuffer->usedResourceCount = 0;
}

void dsVkCommandBuffer_shutdown(dsVkCommandBuffer* commandBuffer)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->usedResources));
	dsVkBarrierList_shutdown(&commandBuffer->barriers);
}
