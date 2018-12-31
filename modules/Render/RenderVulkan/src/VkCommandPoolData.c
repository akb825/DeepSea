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

#include "VkCommandPoolData.h"

#include "Resources/VkResource.h"
#include "VkCommandBuffer.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsVkCommandPoolData* dsVkCommandPoolData_create(dsAllocator* allocator, dsRenderer* renderer,
	dsCommandBufferUsage usage, uint32_t count)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkCommandPoolData)) +
		DS_ALIGNED_SIZE(sizeof(VkCommandBuffer*)*count) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer*)*count) +
		DS_ALIGNED_SIZE(sizeof(dsVkCommandBuffer))*count;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkCommandPoolData* pool = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkCommandPoolData);
	DS_ASSERT(pool);

	pool->allocator = dsAllocator_keepPointer(allocator);
	pool->renderer = renderer;
	dsVkResource_initialize(&pool->resource);
	pool->vkCommandPool = 0;
	pool->vkCommandBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkCommandBuffer,
		count);
	DS_ASSERT(pool->vkCommandBuffers);
	memset(pool->vkCommandBuffers, 0, sizeof(VkCommandBuffer)*count);
	pool->commandBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsCommandBuffer*,
		count);
	DS_ASSERT(pool->commandBuffers);
	memset(pool->commandBuffers, 0, sizeof(dsVkCommandBuffer*)*count);
	pool->count = count;

	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		0,
		device->queueFamilyIndex
	};

	VkResult result = DS_VK_CALL(device->vkCreateCommandPool)(device->device,
		&commandPoolCreateInfo, instance->allocCallbacksPtr, &pool->vkCommandPool);
	if (!dsHandleVkResult(result))
	{
		dsVkCommandPoolData_destroy(pool);
		return false;
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		pool->vkCommandPool,
		VK_COMMAND_BUFFER_LEVEL_SECONDARY,
		count
	};

	result = DS_VK_CALL(device->vkAllocateCommandBuffers)(device->device,
		&commandBufferAllocateInfo, pool->vkCommandBuffers);
	if (!dsHandleVkResult(result))
	{
		dsVkCommandPoolData_destroy(pool);
		return NULL;
	}

	for (uint32_t i = 0; i < count; ++i)
	{
		dsVkCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
			dsVkCommandBuffer);
		DS_ASSERT(commandBuffer);
		dsVkCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator, usage);
		commandBuffer->resource = &pool->resource;
		commandBuffer->vkCommandBuffer = pool->vkCommandBuffers[i];
	}

	return pool;
}

bool dsVkCommandPoolData_prepare(dsVkCommandPoolData* pool)
{
	dsVkDevice* device = &((dsVkRenderer*)pool->renderer)->device;
	dsVkResource_waitUntilNotInUse(&pool->resource, pool->renderer);
	VkResult result = DS_VK_CALL(device->vkResetCommandPool)(device->device, pool->vkCommandPool,
		0);
	if (!dsHandleVkResult(result))
		return false;

	for (uint32_t i = 0; i < pool->count; ++i)
		dsVkCommandBuffer_prepare(pool->commandBuffers[i], false);
	return true;
}

void dsVkCommandPoolData_destroy(dsVkCommandPoolData* pool)
{
	dsVkDevice* device = &((dsVkRenderer*)pool->renderer)->device;
	dsVkInstance* instance = &device->instance;
	if (pool->vkCommandPool)
	{
		DS_VK_CALL(device->vkDestroyCommandPool)(device->device, pool->vkCommandPool,
			instance->allocCallbacksPtr);
	}

	for (uint32_t i = 0; i < pool->count; ++i)
	{
		dsVkCommandBuffer* commandBuffer = (dsVkCommandBuffer*)pool->commandBuffers[i];
		if (commandBuffer)
			dsVkCommandBuffer_shutdown(commandBuffer);
	}

	dsVkResource_shutdown(&pool->resource);
	if (pool->allocator)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool));
}
