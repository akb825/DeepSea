/*
 * Copyright 2018-2025 Aaron Barany
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

#include "VkCommandPoolData.h"

#include "Resources/VkResource.h"
#include "VkCommandBuffer.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsVkCommandPoolData* dsVkCommandPoolData_create(dsAllocator* allocator, dsRenderer* renderer,
	dsCommandBufferUsage usage)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkCommandPoolData* pool = DS_ALLOCATE_OBJECT(allocator, dsVkCommandPoolData);
	if (!pool)
		return NULL;

	pool->allocator = dsAllocator_keepPointer(allocator);
	pool->renderer = renderer;
	dsVkResource_initialize(&pool->resource);
	pool->commandBuffers = NULL;
	pool->usage = usage;
	pool->commandPool = 0;
	pool->count = 0;
	pool->createdCount = 0;
	pool->maxCommandBuffers = 0;

	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		usage & dsCommandBufferUsage_MultiFrame ? 0 : VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		device->queueFamilyIndex
	};

	VkResult result = DS_VK_CALL(device->vkCreateCommandPool)(device->device,
		&commandPoolCreateInfo, instance->allocCallbacksPtr, &pool->commandPool);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create command pool"))
		return false;

	return pool;
}

bool dsVkCommandPoolData_createCommandBuffers(dsVkCommandPoolData* pool, uint32_t count)
{
	if (!DS_RESIZEABLE_ARRAY_ADD(pool->allocator, pool->commandBuffers, pool->count,
			pool->maxCommandBuffers, count))
	{
		return false;
	}

	for (; pool->createdCount < pool->count; ++pool->createdCount)
	{
		dsVkCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(pool->allocator, dsVkCommandBuffer);
		if (!commandBuffer)
		{
			pool->count -= count;
			return false;
		}

		dsVkCommandBuffer_initialize(commandBuffer, pool->renderer, pool->allocator, pool->usage,
			pool->commandPool);
		commandBuffer->resource = &pool->resource;
		pool->commandBuffers[pool->createdCount] = (dsCommandBuffer*)commandBuffer;
	}

	return true;
}

bool dsVkCommandPoolData_prepare(dsVkCommandPoolData* pool)
{
	dsVkDevice* device = &((dsVkRenderer*)pool->renderer)->device;
	dsVkResource_waitUntilNotInUse(&pool->resource, pool->renderer);
	DS_VK_CALL(device->vkResetCommandPool)(device->device, pool->commandPool, 0);
	// Clear resources so they don't stick around, but delay calling dsVkCommandBuffer_prepare()
	// until when begin is called on the command buffers to avoid performance issues on some
	// drivers.
	for (uint32_t i = 0; i < pool->count; ++i)
		dsVkCommandBuffer_clearUsedResources(pool->commandBuffers[i], false);
	pool->count = 0;
	return true;
}

void dsVkCommandPoolData_destroy(dsVkCommandPoolData* pool)
{
	dsVkDevice* device = &((dsVkRenderer*)pool->renderer)->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < pool->createdCount; ++i)
	{
		dsVkCommandBuffer_shutdown((dsVkCommandBuffer*)pool->commandBuffers[i]);
		DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers[i]));
	}

	if (pool->commandPool)
	{
		DS_VK_CALL(device->vkDestroyCommandPool)(device->device, pool->commandPool,
			instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&pool->resource);
	DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers));
	DS_VERIFY(dsAllocator_free(pool->allocator, pool));
}
