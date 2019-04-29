/*
 * Copyright 2019 Aaron Barany
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

#include "MTLCommandBufferPool.h"

#include "MTLCommandBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsCommandBufferPool* dsMTLCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	unsigned int lists = usage & dsCommandBufferUsage_DoubleBuffer ? 2 : 1;

	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsCommandBufferPool)) +
		lists*(DS_ALIGNED_SIZE(sizeof(dsMTLCommandBuffer*)*count) +
			DS_ALIGNED_SIZE(sizeof(dsMTLCommandBuffer))*count);
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, totalSize));

	dsCommandBufferPool* pool = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
		dsCommandBufferPool);
	DS_ASSERT(pool);

	pool->renderer = renderer;
	pool->allocator = dsAllocator_keepPointer(allocator);
	pool->count = count;
	pool->usage = usage;

	pool->currentBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
		dsCommandBuffer*, count);
	DS_ASSERT(pool->currentBuffers);
	for (uint32_t i = 0; i < count; ++i)
	{
		dsMTLCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
			dsMTLCommandBuffer);
		DS_ASSERT(commandBuffer);
		dsMTLCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator, usage);
		pool->currentBuffers[i] = (dsCommandBuffer*)commandBuffer;
	}

	if (lists == 2)
	{
		pool->previousBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsCommandBuffer*, count);
		DS_ASSERT(pool->currentBuffers);
		for (uint32_t i = 0; i < count; ++i)
		{
			dsMTLCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
				dsMTLCommandBuffer);
			DS_ASSERT(commandBuffer);
			dsMTLCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator, usage);
			pool->previousBuffers[i] = (dsCommandBuffer*)commandBuffer;
		}
	}
	else
		pool->previousBuffers = NULL;

	return pool;
}

bool dsMTLCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);

	if (pool->usage & dsCommandBufferUsage_DoubleBuffer)
	{
		dsCommandBuffer** temp = pool->currentBuffers;
		pool->currentBuffers = pool->previousBuffers;
		pool->previousBuffers = temp;
	}

	return true;
}

bool dsMTLCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);

	for (uint32_t i = 0; i < pool->count; ++i)
		dsMTLCommandBuffer_shutdown((dsMTLCommandBuffer*)pool->currentBuffers[i]);

	if (pool->previousBuffers)
	{
		for (uint32_t i = 0; i < pool->count; ++i)
			dsMTLCommandBuffer_shutdown((dsMTLCommandBuffer*)pool->previousBuffers[i]);
	}

	if (pool->allocator)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
