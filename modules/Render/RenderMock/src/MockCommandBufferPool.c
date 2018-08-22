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

#include "MockCommandBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

dsCommandBufferPool* dsMockCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	DS_ASSERT(count);

	unsigned int lists = usage & dsCommandBufferUsage_DoubleBuffer ? 2 : 1;

	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsCommandBufferPool)) +
		lists*(DS_ALIGNED_SIZE(sizeof(dsCommandBuffer*)*count) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer))*count);
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
		dsCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
			dsCommandBuffer);
		DS_ASSERT(commandBuffer);
		commandBuffer->renderer = renderer;
		commandBuffer->allocator = pool->allocator;
		commandBuffer->usage = pool->usage;
		commandBuffer->frameActive = true;
		commandBuffer->boundSurface = NULL;
		commandBuffer->boundFramebuffer = NULL;
		commandBuffer->boundRenderPass = NULL;
		commandBuffer->activeRenderSubpass = 0;
		commandBuffer->indirectCommands = false;
		commandBuffer->boundShader = NULL;
		commandBuffer->boundComputeShader = NULL;
		pool->currentBuffers[i] = commandBuffer;
	}

	if (lists == 2)
	{
		pool->otherBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsCommandBuffer*, count);
		DS_ASSERT(pool->currentBuffers);
		for (uint32_t i = 0; i < count; ++i)
		{
			dsCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
				dsCommandBuffer);
			DS_ASSERT(commandBuffer);
			commandBuffer->renderer = renderer;
			commandBuffer->allocator = pool->allocator;
			commandBuffer->usage = pool->usage;
			commandBuffer->frameActive = true;
			commandBuffer->boundSurface = NULL;
			commandBuffer->boundFramebuffer = NULL;
			commandBuffer->boundRenderPass = NULL;
			commandBuffer->activeRenderSubpass = 0;
			commandBuffer->indirectCommands = false;
			commandBuffer->boundShader = NULL;
			commandBuffer->boundComputeShader = NULL;
			pool->otherBuffers[i] = commandBuffer;
		}
	}
	else
		pool->otherBuffers = NULL;

	return pool;
}

bool dsMockCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	if (pool->usage & dsCommandBufferUsage_DoubleBuffer)
	{
		dsCommandBuffer** temp = pool->currentBuffers;
		pool->currentBuffers = pool->otherBuffers;
		pool->otherBuffers = temp;
	}

	return true;
}

bool dsMockCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(pool);

	if (pool->allocator)
		return dsAllocator_free(pool->allocator, pool);
	return true;
}
