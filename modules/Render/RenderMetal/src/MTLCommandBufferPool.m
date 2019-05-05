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
#include "MTLHardwareCommandBuffer.h"
#include "MTLSoftwareCommandBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

inline static bool needsSoftwareCommandBuffer(dsCommandBufferUsage usage)
{
	return (usage & (dsCommandBufferUsage_Secondary | dsCommandBufferUsage_MultiSubmit |
		dsCommandBufferUsage_MultiFrame)) != 0;
}

static size_t fullAllocSize(dsCommandBufferUsage usage, uint32_t count)
{
	unsigned int lists = usage & dsCommandBufferUsage_DoubleBuffer ? 2 : 1;
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsCommandBufferPool)) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer*)*count)*lists;
	if (needsSoftwareCommandBuffer(usage))
		totalSize += DS_ALIGNED_SIZE(sizeof(dsMTLSoftwareCommandBuffer))*count*lists;
	else
		totalSize += DS_ALIGNED_SIZE(sizeof(dsMTLHardwareCommandBuffer))*count*lists;
	return totalSize;
}

static void createCommandBuffers(dsCommandBuffer** outCommandBuffers, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage, uint32_t count)
{
	if (needsSoftwareCommandBuffer(usage))
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			dsMTLSoftwareCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(allocator,
				dsMTLSoftwareCommandBuffer);
			DS_ASSERT(commandBuffer);
			dsMTLSoftwareCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator,
				usage);
			outCommandBuffers[i] = (dsCommandBuffer*)commandBuffer;
		}
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			dsMTLHardwareCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(allocator,
				dsMTLHardwareCommandBuffer);
			DS_ASSERT(commandBuffer);
			dsMTLHardwareCommandBuffer_initialize(commandBuffer, renderer, renderer->allocator,
				usage);
			outCommandBuffers[i] = (dsCommandBuffer*)commandBuffer;
		}
	}
}

static void shutdownCommandBuffers(dsCommandBufferUsage usage, dsCommandBuffer** commandBuffers,
	uint32_t count)
{
	if (needsSoftwareCommandBuffer(usage))
	{
		for (uint32_t i = 0; i < count; ++i)
			dsMTLSoftwareCommandBuffer_shutdown((dsMTLSoftwareCommandBuffer*)commandBuffers[i]);
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
			dsMTLHardwareCommandBuffer_shutdown((dsMTLHardwareCommandBuffer*)commandBuffers[i]);
	}
}

dsCommandBufferPool* dsMTLCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	size_t totalSize = fullAllocSize(usage, count);
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
	createCommandBuffers(pool->currentBuffers, renderer, (dsAllocator*)&bufferAllocator, usage,
		count);

	if (usage & dsCommandBufferUsage_DoubleBuffer)
	{
		pool->previousBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsCommandBuffer*, count);
		DS_ASSERT(pool->previousBuffers);
		createCommandBuffers(pool->previousBuffers, renderer, (dsAllocator*)&bufferAllocator, usage,
			count);
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

	shutdownCommandBuffers(pool->usage, pool->currentBuffers, pool->count);
	if (pool->previousBuffers)
		shutdownCommandBuffers(pool->usage, pool->previousBuffers, pool->count);

	if (pool->allocator)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
