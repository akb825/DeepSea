/*
 * Copyright 2017-2019 Aaron Barany
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

#include "MockCommandBuffer.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

typedef struct dsMockCommandBufferPool
{
	dsCommandBufferPool commandBufferPool;
	uint32_t createdCount;
	uint32_t maxCommandBuffers;
} dsMockCommandBufferPool;

dsCommandBufferPool* dsMockCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);

	dsMockCommandBufferPool* pool = DS_ALLOCATE_OBJECT(allocator, dsMockCommandBufferPool);
	if (!pool)
		return NULL;

	dsCommandBufferPool* basePool = (dsCommandBufferPool*)pool;
	basePool->renderer = renderer;
	basePool->allocator = dsAllocator_keepPointer(allocator);
	basePool->commandBuffers = NULL;
	basePool->count = 0;
	basePool->usage = usage;
	pool->createdCount = 0;
	pool->maxCommandBuffers = 0;

	return basePool;
}

bool dsMockCommandBufferPool_createCommandBuffers(dsRenderer* renderer,
	dsCommandBufferPool* pool, uint32_t count)
{
	DS_ASSERT(renderer);
	DS_ASSERT(pool);

	dsMockCommandBufferPool* mockPool = (dsMockCommandBufferPool*)pool;
	if (!DS_RESIZEABLE_ARRAY_ADD(pool->allocator, pool->commandBuffers, pool->count,
			mockPool->maxCommandBuffers, count))
	{
		return false;
	}

	for (; mockPool->createdCount < pool->count; ++mockPool->createdCount)
	{
		dsCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(pool->allocator, dsCommandBuffer);
		if (!commandBuffer)
		{
			pool->count -= count;
			return false;
		}

		// Only these members need to be initialized here.
		commandBuffer->renderer = pool->renderer;
		commandBuffer->allocator = pool->allocator;
		commandBuffer->usage = pool->usage;
		pool->commandBuffers[mockPool->createdCount] = commandBuffer;
	}

	return true;
}

bool dsMockCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	pool->count = 0;
	return true;
}

bool dsMockCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	dsMockCommandBufferPool* mockPool = (dsMockCommandBufferPool*)pool;
	for (uint32_t i = 0; i < mockPool->createdCount; ++i)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers[i]));
	DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers));
	DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
