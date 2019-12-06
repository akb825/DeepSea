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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

typedef struct dsMTLCommandBufferPool
{
	dsCommandBufferPool commandBufferPool;
	uint32_t createdCount;
	uint32_t maxCommandBuffers;
} dsMTLCommandBufferPool;

inline static bool needsSoftwareCommandBuffer(dsCommandBufferUsage usage)
{
	return (usage & (dsCommandBufferUsage_Secondary | dsCommandBufferUsage_MultiSubmit |
		dsCommandBufferUsage_MultiFrame)) != 0;
}

dsCommandBufferPool* dsMTLCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	dsMTLCommandBufferPool* pool = DS_ALLOCATE_OBJECT(allocator, dsMTLCommandBufferPool);
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

bool dsMTLCommandBufferPool_createCommandBuffers(dsRenderer* renderer, dsCommandBufferPool* pool,
	uint32_t count)
{
	dsMTLCommandBufferPool* mtlPool = (dsMTLCommandBufferPool*)pool;
	if (!DS_RESIZEABLE_ARRAY_ADD(pool->allocator, pool->commandBuffers, pool->count,
			mtlPool->maxCommandBuffers, count))
	{
		return false;
	}

	for (; mtlPool->createdCount < pool->count; ++mtlPool->createdCount)
	{
		dsCommandBuffer* commandBuffer;
		if (needsSoftwareCommandBuffer(pool->usage))
		{
			dsMTLSoftwareCommandBuffer* mtlCommandBuffer = DS_ALLOCATE_OBJECT(pool->allocator,
				dsMTLSoftwareCommandBuffer);
			commandBuffer = (dsCommandBuffer*)mtlCommandBuffer;
			if (mtlCommandBuffer)
			{
				dsMTLSoftwareCommandBuffer_initialize(mtlCommandBuffer, renderer,
					renderer->allocator, pool->usage);
			}
		}
		else
		{
			dsMTLHardwareCommandBuffer* mtlCommandBuffer = DS_ALLOCATE_OBJECT(pool->allocator,
				dsMTLHardwareCommandBuffer);
			commandBuffer = (dsCommandBuffer*)mtlCommandBuffer;
			if (mtlCommandBuffer)
			{
				dsMTLHardwareCommandBuffer_initialize(mtlCommandBuffer, renderer,
					renderer->allocator, pool->usage);
			}
		}

		if (!commandBuffer)
		{
			pool->count -= count;
			return false;
		}

		pool->commandBuffers[mtlPool->createdCount] = commandBuffer;
	}

	return true;
}

bool dsMTLCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);

	pool->count = 0;
	return true;
}

bool dsMTLCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);

	dsMTLCommandBufferPool* mtlPool = (dsMTLCommandBufferPool*)pool;
	if (needsSoftwareCommandBuffer(pool->usage))
	{
		for (uint32_t i = 0; i < mtlPool->createdCount; ++i)
		{
			dsMTLSoftwareCommandBuffer_shutdown(
				(dsMTLSoftwareCommandBuffer*)pool->commandBuffers[i]);
			DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers[i]));
		}
	}
	else
	{
		for (uint32_t i = 0; i < mtlPool->createdCount; ++i)
		{
			dsMTLHardwareCommandBuffer_shutdown(
				(dsMTLHardwareCommandBuffer*)pool->commandBuffers[i]);
			DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers[i]));
		}
	}

	DS_VERIFY(dsAllocator_free(pool->allocator, pool->commandBuffers));
	DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
