/*
 * Copyright 2026 Aaron Barany
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

#include "ResourceCommandBuffers.h"

#include "GPUProfileContext.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/Render/CommandBuffer.h>

struct dsResourceCommandBuffers
{
	dsAllocator* allocator;
	dsRenderer* renderer;

	dsCommandBufferPool** availableCommandBuffers;
	uint32_t availableCommandBufferCount;
	uint32_t maxAvailableCommandBuffers;

	dsCommandBufferPool** pendingCommandBuffers;
	uint32_t pendingCommandBufferCount;
	uint32_t maxPendingCommandBuffers;

	dsCommandBufferPool** activeCommandBuffers;
	uint32_t activeCommandBufferCount;
	uint32_t maxActiveCommandBuffers;

	dsSpinlock lock;
};

dsResourceCommandBuffers* dsResourceCommandBuffers_create(
	dsRenderer* renderer, dsAllocator* allocator)
{
	dsResourceCommandBuffers* commandBuffers = DS_ALLOCATE_OBJECT(
		allocator, dsResourceCommandBuffers);
	if (!commandBuffers)
		return NULL;

	commandBuffers->allocator = allocator;
	commandBuffers->renderer = renderer;

	commandBuffers->availableCommandBuffers = NULL;
	commandBuffers->availableCommandBufferCount = 0;
	commandBuffers->maxAvailableCommandBuffers = 0;

	commandBuffers->pendingCommandBuffers = NULL;
	commandBuffers->pendingCommandBufferCount = 0;
	commandBuffers->maxPendingCommandBuffers = 0;

	commandBuffers->activeCommandBuffers = NULL;
	commandBuffers->activeCommandBufferCount = 0;
	commandBuffers->maxActiveCommandBuffers = 0;

	DS_VERIFY(dsSpinlock_initialize(&commandBuffers->lock));

	return commandBuffers;
}

dsCommandBuffer* dsResourceCommandBuffers_acquire(dsResourceCommandBuffers* commandBuffers)
{
	DS_VERIFY(dsSpinlock_lock(&commandBuffers->lock));

	uint32_t activeIndex = commandBuffers->activeCommandBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffers->allocator, commandBuffers->activeCommandBuffers,
			commandBuffers->activeCommandBufferCount, commandBuffers->maxActiveCommandBuffers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		return NULL;
	}

	dsCommandBufferPool* pool;
	bool justCreated;
	if (commandBuffers->availableCommandBufferCount > 0)
	{
		justCreated = false;
		uint32_t poolIndex = --commandBuffers->availableCommandBufferCount;
		pool = commandBuffers->availableCommandBuffers[poolIndex];
	}
	else
	{
		justCreated = true;
		pool = dsCommandBufferPool_create(
			commandBuffers->renderer, commandBuffers->allocator, dsCommandBufferUsage_Resource);
		if (!pool)
		{
			DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
			return NULL;
		}
	}

	dsCommandBuffer** commandBufferPtr = dsCommandBufferPool_createCommandBuffers(pool, 1);
	if (!commandBufferPtr)
	{
		if (justCreated)
			DS_VERIFY(dsCommandBufferPool_destroy(pool));
		else
			++commandBuffers->availableCommandBufferCount;
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		return NULL;
	}

	DS_ASSERT(pool->count == 1);
	DS_ASSERT(pool->commandBuffers == commandBufferPtr);
	commandBuffers->activeCommandBuffers[activeIndex] = pool;
	DS_VERIFY(dsCommandBuffer_begin(*commandBufferPtr));

	DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
	return *commandBufferPtr;
}

bool dsResourceCommandBuffers_flush(
	dsResourceCommandBuffers* commandBuffers, dsCommandBuffer* commandBuffer)
{
	DS_VERIFY(dsSpinlock_lock(&commandBuffers->lock));

	uint32_t foundIndex;
	for (foundIndex = 0; foundIndex < commandBuffers->activeCommandBufferCount; ++foundIndex)
	{
		if (commandBuffers->activeCommandBuffers[foundIndex]->commandBuffers[0] == commandBuffer)
			break;
	}

	if (foundIndex == commandBuffers->activeCommandBufferCount)
	{
		errno = ENOTFOUND;
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		return false;
	}

	uint32_t pendingIndex = commandBuffers->pendingCommandBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffers->allocator, commandBuffers->pendingCommandBuffers,
			commandBuffers->pendingCommandBufferCount, commandBuffers->maxPendingCommandBuffers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		return false;
	}

	DS_VERIFY(dsCommandBuffer_end(commandBuffer));
	commandBuffers->pendingCommandBuffers[pendingIndex] =
		commandBuffers->activeCommandBuffers[foundIndex];

	// Constant-time removal since order doesn't matter.
	--commandBuffers->activeCommandBufferCount;
	commandBuffers->activeCommandBuffers[foundIndex] =
		commandBuffers->activeCommandBuffers[commandBuffers->activeCommandBufferCount];

	DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
	return true;
}

void dsResourceCommandBuffers_submit(dsResourceCommandBuffers* commandBuffers)
{
	DS_PROFILE_FUNC_START();
	DS_VERIFY(dsSpinlock_lock(&commandBuffers->lock));

	if (commandBuffers->pendingCommandBufferCount == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	uint32_t availableIndex = commandBuffers->availableCommandBufferCount;
	if (!DS_CHECK(DS_RENDER_LOG_TAG, DS_RESIZEABLE_ARRAY_ADD(commandBuffers->allocator,
			commandBuffers->availableCommandBuffers, commandBuffers->availableCommandBufferCount,
			commandBuffers->maxAvailableCommandBuffers, commandBuffers->pendingCommandBufferCount)))
	{
		DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	dsCommandBuffer* mainCommandBuffer = commandBuffers->renderer->mainCommandBuffer;
	dsGPUProfileContext* profileContext = commandBuffers->renderer->_profileContext;
	dsGPUProfileContext_beginDeferredResources(profileContext);

	for (uint32_t i = 0; i < commandBuffers->pendingCommandBufferCount; ++i)
	{
		dsCommandBufferPool* pool = commandBuffers->pendingCommandBuffers[i];
		DS_CHECK(DS_RENDER_LOG_TAG, dsCommandBuffer_submit(
			mainCommandBuffer, pool->commandBuffers[0]));

		// Once it's submitted, can reset the command buffer pool and move it to the available list.
		DS_VERIFY(dsCommandBufferPool_reset(pool));
		commandBuffers->availableCommandBuffers[availableIndex++] = pool;
	}

	dsGPUProfileContext_endDeferredResources(profileContext);
	commandBuffers->pendingCommandBufferCount = 0;

	DS_VERIFY(dsSpinlock_unlock(&commandBuffers->lock));
	DS_PROFILE_FUNC_END();
}

void dsResourceCommandBuffers_destroy(dsResourceCommandBuffers* commandBuffers)
{
	if (!commandBuffers)
		return;

	for (uint32_t i = 0; i < commandBuffers->availableCommandBufferCount; ++i)
		dsCommandBufferPool_destroy(commandBuffers->availableCommandBuffers[i]);
	DS_VERIFY(dsAllocator_free(commandBuffers->allocator, commandBuffers->availableCommandBuffers));

	for (uint32_t i = 0; i < commandBuffers->pendingCommandBufferCount; ++i)
		dsCommandBufferPool_destroy(commandBuffers->pendingCommandBuffers[i]);
	DS_VERIFY(dsAllocator_free(commandBuffers->allocator, commandBuffers->pendingCommandBuffers));

	for (uint32_t i = 0; i < commandBuffers->activeCommandBufferCount; ++i)
		dsCommandBufferPool_destroy(commandBuffers->activeCommandBuffers[i]);
	DS_VERIFY(dsAllocator_free(commandBuffers->allocator, commandBuffers->activeCommandBuffers));

	dsSpinlock_shutdown(&commandBuffers->lock);
	DS_VERIFY(dsAllocator_free(commandBuffers->allocator, commandBuffers));
}
