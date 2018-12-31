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

#include "VkCommandBufferPool.h"

#include "VkCommandPoolData.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsCommandBufferPool* dsVkCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	dsVkCommandBufferPool* pool = DS_ALLOCATE_OBJECT(allocator, dsVkCommandBufferPool);
	if (!pool)
		return NULL;

	dsCommandBufferPool* basePool = (dsCommandBufferPool*)pool;
	basePool->renderer = renderer;
	basePool->allocator = dsAllocator_keepPointer(allocator);
	basePool->currentBuffers = NULL;
	basePool->prevBuffers = NULL;
	basePool->count = count;
	basePool->usage = usage;

	memset(pool->commandPools, 0, sizeof(pool->commandPools));
	pool->curCommandPool = 0;

	for (uint32_t i = 0; i < DS_DELAY_FRAMES; ++i)
	{
		pool->commandPools[i] = dsVkCommandPoolData_create(allocator, renderer, usage, count);
		if (!pool->commandPools[i])
		{
			dsVkCommandBufferPool_destroy(renderer, basePool);
			return NULL;
		}
	}

	basePool->currentBuffers = pool->commandPools[0]->commandBuffers;
	return basePool;
}

bool dsVkCommandBufferPool_reset(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	dsVkCommandBufferPool* vkPool = (dsVkCommandBufferPool*)pool;
	uint32_t nextCommandPool = (vkPool->curCommandPool + 1) % DS_DELAY_FRAMES;

	dsVkCommandPoolData* poolData = vkPool->commandPools[nextCommandPool];
	if (!dsVkCommandPoolData_prepare(poolData))
		return false;

	pool->prevBuffers = pool->currentBuffers;
	pool->currentBuffers = poolData->commandBuffers;
	return true;
}

bool dsVkCommandBufferPool_destroy(dsRenderer* renderer, dsCommandBufferPool* pool)
{
	DS_UNUSED(renderer);
	dsVkCommandBufferPool* vkPool = (dsVkCommandBufferPool*)pool;
	for (uint32_t i = 0; i < DS_DELAY_FRAMES; ++i)
	{
		dsVkCommandPoolData* poolData = vkPool->commandPools[i];
		if (poolData)
			dsVkCommandPoolData_destroy(poolData);
	}

	if (pool->allocator)
		DS_VERIFY(dsAllocator_free(pool->allocator, pool));
	return true;
}
