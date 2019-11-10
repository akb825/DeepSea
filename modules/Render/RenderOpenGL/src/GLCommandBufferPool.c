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

#include "GLCommandBufferPool.h"
#include "GLOtherCommandBuffer.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

typedef struct dsGLCommandBufferPool
{
	dsCommandBufferPool commandBufferPool;
	uint32_t createdCount;
	uint32_t maxCommandBuffers;
} dsGLCommandBufferPool;

dsCommandBufferPool* dsGLCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);

	dsGLCommandBufferPool* pool = DS_ALLOCATE_OBJECT(allocator, dsGLCommandBufferPool);
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

bool dsGLCommandBufferPool_createCommandBuffers(dsRenderer* renderer, dsCommandBufferPool* pool,
	uint32_t count)
{
	DS_ASSERT(renderer);
	DS_ASSERT(pool);

	dsGLCommandBufferPool* glPool = (dsGLCommandBufferPool*)pool;
	if (!DS_RESIZEABLE_ARRAY_ADD(pool->allocator, pool->commandBuffers, pool->count,
			glPool->maxCommandBuffers, count))
	{
		return false;
	}

	for (; glPool->createdCount < pool->count; ++glPool->createdCount)
	{
		dsGLOtherCommandBuffer* commandBuffer = dsGLOtherCommandBuffer_create(renderer,
			pool->allocator, pool->usage);
		if (!commandBuffer)
		{
			pool->count -= count;
			return false;
		}

		pool->commandBuffers[glPool->createdCount] = (dsCommandBuffer*)commandBuffer;
	}

	return true;
}

bool dsGLCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	for (uint32_t i = 0; i < pool->count; ++i)
		dsGLOtherCommandBuffer_reset(pool->commandBuffers[i]);
	pool->count = 0;
	return true;
}

bool dsGLCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	dsGLCommandBufferPool* glPool = (dsGLCommandBufferPool*)pool;
	for (uint32_t i = 0; i < glPool->createdCount; ++i)
		dsGLOtherCommandBuffer_destroy((dsGLOtherCommandBuffer*)pool->commandBuffers[i]);

	DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers));
	DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
