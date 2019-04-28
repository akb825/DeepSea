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
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkCommandPoolData)) +
		DS_ALIGNED_SIZE(sizeof(dsVkCommandBuffer)*count) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer*)*count);
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
	pool->vkCommandBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsVkCommandBuffer,
		count);
	DS_ASSERT(pool->vkCommandBuffers);
	memset(pool->vkCommandBuffers, 0, sizeof(dsVkCommandBuffer)*count);
	pool->commandBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsCommandBuffer*,
		count);
	DS_ASSERT(pool->commandBuffers);
	memset(pool->commandBuffers, 0, sizeof(dsCommandBuffer*)*count);
	pool->count = count;

	for (uint32_t i = 0; i < count; ++i)
	{
		dsVkCommandBuffer* commandBuffer = pool->vkCommandBuffers + i;
		dsVkCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator, usage);
		commandBuffer->resource = &pool->resource;
		pool->commandBuffers[i] = (dsCommandBuffer*)commandBuffer;;
	}

	return pool;
}

bool dsVkCommandPoolData_prepare(dsVkCommandPoolData* pool)
{
	dsVkResource_waitUntilNotInUse(&pool->resource, pool->renderer);
	for (uint32_t i = 0; i < pool->count; ++i)
	{
		dsVkCommandBuffer_prepare(pool->commandBuffers[i]);
		dsVkCommandBuffer_clearUsedResources(pool->commandBuffers[i]);
	}
	return true;
}

void dsVkCommandPoolData_destroy(dsVkCommandPoolData* pool)
{
	for (uint32_t i = 0; i < pool->count; ++i)
		dsVkCommandBuffer_shutdown(pool->vkCommandBuffers + i);

	dsVkResource_shutdown(&pool->resource);
	if (pool->allocator)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool));
}
