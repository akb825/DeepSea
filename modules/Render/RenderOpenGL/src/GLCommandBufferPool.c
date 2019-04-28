/*
 * Copyright 2017 Aaron Barany
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

#include "GLCommandBufferPool.h"
#include "GLOtherCommandBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsCommandBufferPool* dsGLCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	DS_ASSERT(count > 0);

	size_t bufferArraySize = count*sizeof(dsCommandBuffer*);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsCommandBufferPool)) +
		DS_ALIGNED_SIZE(bufferArraySize);
	if (usage & dsCommandBufferUsage_DoubleBuffer)
		fullSize += DS_ALIGNED_SIZE(bufferArraySize);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsCommandBufferPool* pool = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsCommandBufferPool);
	DS_ASSERT(pool);

	pool->renderer = renderer;
	pool->allocator = dsAllocator_keepPointer(allocator);
	pool->currentBuffers = (dsCommandBuffer**)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		bufferArraySize);
	DS_ASSERT(pool->currentBuffers);
	memset(pool->currentBuffers, 0, bufferArraySize);

	if (usage & dsCommandBufferUsage_DoubleBuffer)
	{
		pool->previousBuffers = (dsCommandBuffer**)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			bufferArraySize);
		DS_ASSERT(pool->previousBuffers);
		memset(pool->currentBuffers, 0, bufferArraySize);
	}
	else
		pool->previousBuffers = NULL;

	pool->count = count;
	pool->usage = usage;

	for (uint32_t i = 0; i < count; ++i)
	{
		pool->currentBuffers[i] = (dsCommandBuffer*)dsGLOtherCommandBuffer_create(renderer,
			allocator, (dsCommandBufferUsage)usage);
		if (!pool->currentBuffers[i])
			dsGLCommandBufferPool_destroy(renderer, pool);
	}

	if (pool->previousBuffers)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			pool->previousBuffers[i] = (dsCommandBuffer*)dsGLOtherCommandBuffer_create(renderer,
				allocator, (dsCommandBufferUsage)usage);
			if (!pool->previousBuffers[i])
				dsGLCommandBufferPool_destroy(renderer, pool);
		}
	}

	return pool;
}

bool dsGLCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	if (pool->previousBuffers)
	{
		dsCommandBuffer** temp = pool->previousBuffers;
		pool->previousBuffers = pool->currentBuffers;
		pool->currentBuffers = temp;
	}

	for (uint32_t i = 0; i < pool->count; ++i)
		dsGLOtherCommandBuffer_reset(pool->currentBuffers[i]);
	return true;
}

bool dsGLCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	for (uint32_t i = 0; i < pool->count; ++i)
	{
		if (pool->currentBuffers[i])
			dsGLOtherCommandBuffer_destroy((dsGLOtherCommandBuffer*)pool->currentBuffers[i]);
	}

	if (pool->previousBuffers)
	{
		for (uint32_t i = 0; i < pool->count; ++i)
		{
			if (pool->previousBuffers[i])
				dsGLOtherCommandBuffer_destroy((dsGLOtherCommandBuffer*)pool->previousBuffers[i]);
		}
	}

	if (pool->allocator)
		return dsAllocator_free(pool->allocator, pool);
	return true;
}
