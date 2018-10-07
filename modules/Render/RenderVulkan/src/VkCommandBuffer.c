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
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>

bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkResourceList* usedResources = &vkCommandBuffer->usedResources;

	dsVkCommandBuffer* vkSubmitBuffer = (dsVkCommandBuffer*)submitBuffer;
	dsVkResourceList* submitUsedResources = &vkSubmitBuffer->usedResources;

	// Copy over the used resources.
	uint32_t offset = usedResources->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, usedResources->buffers,
		usedResources->bufferCount, usedResources->maxBuffers, submitUsedResources->bufferCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < submitUsedResources->bufferCount; ++i)
	{
		dsVkGfxBufferData* buffer = submitUsedResources->buffers[i];
		DS_ATOMIC_FETCH_ADD32(&buffer->commandBufferCount, 1);
		usedResources->buffers[offset + i] = buffer;
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

bool dsVkCommandBuffer_addBuffer(dsCommandBuffer* commandBuffer, dsVkGfxBufferData* buffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkResourceList* usedResources = &vkCommandBuffer->usedResources;
	uint32_t index = usedResources->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, usedResources->buffers,
		usedResources->bufferCount, usedResources->maxBuffers, 1))
	{
		return false;
	}

	DS_ATOMIC_FETCH_ADD32(&buffer->commandBufferCount, 1);
	usedResources->buffers[index] = buffer;
	return true;
}

void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkResourceList* usedResources = &vkCommandBuffer->usedResources;

	for (uint32_t i = 0; i < usedResources->bufferCount; ++i)
		DS_ATOMIC_FETCH_ADD32(&usedResources->buffers[i]->commandBufferCount, -1);
	usedResources->bufferCount = 0;
}

void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkResourceList* usedResources = &vkCommandBuffer->usedResources;

	for (uint32_t i = 0; i < usedResources->bufferCount; ++i)
	{
		dsVkGfxBufferData* buffer = usedResources->buffers[i];
		DS_ATOMIC_FETCH_ADD32(&buffer->commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&buffer->lock));
		buffer->lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_lock(&buffer->lock));
	}
	usedResources->bufferCount = 0;
}
