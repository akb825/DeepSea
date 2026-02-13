/*
 * Copyright 2018-2026 Aaron Barany
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

#include "GPUProfileContext.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/Resources/GfxQueryPool.h>
#include <DeepSea/Render/Renderer.h>

#include <string.h>

#define INVALID_INDEX ((uint32_t)-1)
#define QUERY_POOL_SIZE 1000
#define MAX_STRING_LENGTH 32
#define DELAY_FRAMES 2

typedef struct QueryNode
{
	dsHashTableNode node;

	uint64_t totalTime;
	bool visited;
	bool invalid;
} QueryNode;

typedef struct QueryInfo
{
	char category[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	uint64_t time;
	uint32_t beginIndex;
	QueryNode* node;
} QueryInfo;

typedef struct QueryPools
{
	dsGfxQueryPool** pools;
	uint32_t poolCount;
	uint32_t maxPools;

	QueryInfo* queries;
	uint32_t queryCount;
	uint32_t maxQueries;

	uint32_t totalRanges;
	uint32_t beginFrameIndex;
	uint32_t beginSwapIndex;
} QueryPools;

struct dsGPUProfileContext
{
	dsAllocator* allocator;
	dsResourceManager* resourceManager;
	dsCommandBufferPool* secondaryCommandBuffers;

	bool useQueries;

	/*
	 * Quad buffer pools:
	 * - Delay a frame before getting the results to avoid stalling the CPU to wait for the GPU.
	 * - Avoid having to keep the spinlock locked while processing the results of the previous
	 *   frame. This is important if there are command buffer operations happening on other threads
	 *   that aren't tied to the frame. (e.g. resource processing)
	 * - One extra to avoid delays for normal rendering double fuffering.
	 */
	QueryPools queryPools[DELAY_FRAMES + 2];

	QueryNode* nodes;
	dsHashTable* hashTable;

	uint32_t maxNodes;
	uint32_t queryPoolIndex;
	uint32_t swapCount;
	bool error;

	dsSpinlock spinlock;
};

static dsCommandBuffer* getMainCommandBuffer(dsGPUProfileContext* context)
{
	return context->resourceManager->renderer->mainCommandBuffer;
}

static dsCommandBuffer* startCommandBuffer(
	dsGPUProfileContext* context, dsCommandBuffer* commandBuffer)
{
	if (!context->secondaryCommandBuffers || !commandBuffer->secondaryRenderPassCommands)
		return commandBuffer;

	dsCommandBuffer** secondaryCommandBuffer = dsCommandBufferPool_createCommandBuffers(
		context->secondaryCommandBuffers, 1);
	if (!secondaryCommandBuffer)
		return NULL;

	DS_VERIFY(dsCommandBuffer_beginSecondary(*secondaryCommandBuffer,
		commandBuffer->boundFramebuffer, commandBuffer->boundRenderPass,
		commandBuffer->activeRenderSubpass, &commandBuffer->viewport, &commandBuffer->scissor,
		dsGfxOcclusionQueryState_Disabled));
	return *secondaryCommandBuffer;
}

static void endCommandBuffer(dsCommandBuffer* primaryCommandBuffer,
	dsCommandBuffer* secondaryCommandBuffer)
{
	if (primaryCommandBuffer == secondaryCommandBuffer)
		return;

	DS_VERIFY(dsCommandBuffer_end(secondaryCommandBuffer));
	DS_VERIFY(dsCommandBuffer_submit(primaryCommandBuffer, secondaryCommandBuffer));
}

static bool commandBufferValid(const dsCommandBuffer* commandBuffer)
{
	return (commandBuffer->usage &
		(dsCommandBufferUsage_MultiFrame | dsCommandBufferUsage_MultiSubmit)) == 0;
}

static uint32_t queryHash(const void* key)
{
	const QueryInfo* queryKey = (const QueryInfo*)key;
	uint32_t contextHash = dsHashString(queryKey->category);
	uint32_t nameHash = dsHashString(queryKey->name);
	return dsHashCombine(contextHash, nameHash);
}

static bool queryKeysEqual(const void* left, const void* right)
{
	const QueryInfo* leftKey = (const QueryInfo*)left;
	const QueryInfo* rightKey = (const QueryInfo*)right;
	return strcmp(leftKey->category, rightKey->category) == 0 &&
		strcmp(leftKey->name, rightKey->name) == 0;
}

static QueryInfo* addQuery(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* category, const char* name, uint32_t beginIndex, uint32_t beginSwapCount)
{
	// Command buffers on other threads could have a swap between the begin and end. In this case
	// discard the sample.
	if (beginSwapCount != context->swapCount)
		return NULL;

	QueryPools* pools = context->queryPools + context->queryPoolIndex;
	DS_ASSERT(beginIndex == INVALID_INDEX || beginIndex < pools->queryCount);
	uint32_t poolIndex = pools->queryCount/QUERY_POOL_SIZE;
	uint32_t queryIndex = pools->queryCount%QUERY_POOL_SIZE;

	if (poolIndex >= pools->poolCount)
	{
		if (!DS_RESIZEABLE_ARRAY_ADD(context->allocator, pools->pools, pools->poolCount,
				pools->maxPools, 1))
		{
			context->error = true;
			return NULL;
		}

		pools->pools[poolIndex] = dsGfxQueryPool_create(context->resourceManager,
			context->allocator, dsGfxQueryType_Timestamp, QUERY_POOL_SIZE);
		if (!pools->pools[poolIndex])
		{
			context->error = true;
			--pools->poolCount;
			return NULL;
		}

		// Need to reset before first usage.
		dsGfxQueryPool_reset(pools->pools[poolIndex], commandBuffer, 0, QUERY_POOL_SIZE);
	}

	uint32_t index = pools->queryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(context->allocator, pools->queries, pools->queryCount,
			pools->maxQueries, 1))
	{
		context->error = true;
		return NULL;
	}

	if (beginIndex != INVALID_INDEX)
	{
		QueryInfo* beginQuery = pools->queries + beginIndex;
		category = beginQuery->category;
		name = beginQuery->name;
	}
	DS_ASSERT(category);

	QueryInfo* query = pools->queries + index;

	// This gets a little over-protective and there's no way to make this warning happy...
#if DS_GCC && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

	strncpy(query->category, category, MAX_STRING_LENGTH - 1);
	strncpy(query->name, name, MAX_STRING_LENGTH - 1);

#if DS_GCC && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif

	query->category[MAX_STRING_LENGTH - 1] = 0;
	query->name[MAX_STRING_LENGTH - 1] = 0;
	query->time = 0;
	query->beginIndex = beginIndex;
	query->node = NULL;

	if (beginIndex != INVALID_INDEX)
		++pools->totalRanges;

	dsGfxQueryPool_queryTimestamp(pools->pools[poolIndex], commandBuffer, queryIndex);
	return query;
}

static void submitGPUProfileResults(dsGPUProfileContext* context, QueryPools* pools)
{
	DS_PROFILE_FUNC_START();

	if (pools->totalRanges == 0)
		DS_PROFILE_FUNC_RETURN_VOID();

	// Get all of the times from the GPU.
	uint32_t queryPoolCount = (pools->queryCount + QUERY_POOL_SIZE - 1)/QUERY_POOL_SIZE;
	uint32_t lastPoolQueries = pools->queryCount%QUERY_POOL_SIZE;
	for (uint32_t i = 0; i < queryPoolCount; ++i)
	{
		uint32_t queryCount = i == queryPoolCount - 1 ? lastPoolQueries : QUERY_POOL_SIZE;
		QueryInfo* firstQuery = pools->queries + i*QUERY_POOL_SIZE;
		size_t bufferSize = (pools->queryCount - i*QUERY_POOL_SIZE)*sizeof(QueryInfo) -
			offsetof(QueryInfo, time);
		DS_VERIFY(dsGfxQueryPool_getValues(pools->pools[i], 0, queryCount, &firstQuery->time,
			bufferSize, sizeof(QueryInfo), sizeof(pools->queries->time), false));
	}

	// Set up the hash table to manage duplicates.
	uint32_t dummySize = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(context->allocator, context->nodes, dummySize, context->maxNodes,
			pools->totalRanges))
	{
		DS_PROFILE_FUNC_RETURN_VOID();
	}
	memset(context->nodes, 0, pools->totalRanges*sizeof(QueryNode));

	size_t hashTableSize = dsHashTable_tableSize(pools->totalRanges);
	if (!context->hashTable || hashTableSize > context->hashTable->tableSize)
	{
		dsAllocator_free(context->allocator, context->hashTable);
		context->hashTable = (dsHashTable*)dsAllocator_alloc(context->allocator,
			dsHashTable_fullAllocSize(hashTableSize));
		if (!context->hashTable)
			DS_PROFILE_FUNC_RETURN_VOID();

		DS_VERIFY(dsHashTable_initialize(context->hashTable, hashTableSize, &queryHash,
			&queryKeysEqual));
	}

	// First pass: accumulate the times for the same context/name pairs.
	uint32_t nodeCount = 0;
	for (uint32_t i = 0; i < pools->queryCount; ++i)
	{
		QueryInfo* query = pools->queries + i;
		QueryNode* node = (QueryNode*)dsHashTable_find(context->hashTable, query);
		if (!node)
		{
			DS_ASSERT(nodeCount < pools->totalRanges);
			node = context->nodes + nodeCount++;
			DS_VERIFY(dsHashTable_insert(context->hashTable, query, (dsHashTableNode*)node, NULL));
		}
		query->node = node;

		if (query->beginIndex == INVALID_INDEX)
			continue;

		DS_ASSERT(query->beginIndex < pools->queryCount);
		QueryInfo* beginQuery = pools->queries + query->beginIndex;

		// Some drivers seem to wrap the timestamp value rather than using all 64 bits.
		if (beginQuery->time > query->time)
			node->invalid = true;

		if (!node->invalid)
		{
			uint64_t timeDiff = query->time - beginQuery->time;
			timeDiff = (uint64_t)roundf((float)timeDiff*context->resourceManager->timestampPeriod);
			node->totalTime += timeDiff;
		}
	}

	// Second pass: add the GPU timings based on the order they were encountered.
	for (uint32_t i = 0; i < pools->queryCount; ++i)
	{
		QueryInfo* query = pools->queries + i;
		if (query->node->visited || query->node->invalid)
			continue;

		dsProfile_gpu(query->category, query->name, query->node->totalTime);
		query->node->visited = true;
	}

	dsHashTable_clear(context->hashTable);

	DS_PROFILE_FUNC_RETURN_VOID();
}

dsGPUProfileContext* dsGPUProfileContext_create(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!DS_PROFILING_ENABLED || !resourceManager)
		return NULL;

	if (!allocator->freeFunc)
		return NULL;

	dsGPUProfileContext* context = DS_ALLOCATE_OBJECT(allocator, dsGPUProfileContext);
	if (!context)
		return NULL;

	memset(context, 0, sizeof(dsGPUProfileContext));
	context->resourceManager = resourceManager;
	context->allocator = allocator;
	context->useQueries = DS_GPU_PROFILING_ENABLED && resourceManager->timestampPeriod > 0.0f;

	if (context->useQueries && resourceManager->renderer->strictRenderPassSecondaryCommands)
	{
		context->secondaryCommandBuffers = dsCommandBufferPool_create(
			resourceManager->renderer, allocator, dsCommandBufferUsage_Secondary);
		if (!context->secondaryCommandBuffers)
		{
			dsAllocator_free(allocator, context);
			return NULL;
		}
	}

	if (context->useQueries)
		dsSpinlock_initialize(&context->spinlock);

	return context;
}

void dsGPUProfileContext_beginFrame(dsGPUProfileContext* context)
{
	if (!context)
		return;

	if (context->secondaryCommandBuffers)
		dsCommandBufferPool_reset(context->secondaryCommandBuffers);

	dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);
	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_pushDebugGroup(renderer, commandBuffer, "Frame");

	if (context->useQueries)
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryPools* pools = context->queryPools + context->queryPoolIndex;
			pools->beginFrameIndex = pools->queryCount;
			addQuery(context, commandBuffer, "Frame", "Total", INVALID_INDEX, context->swapCount);
			pools->beginSwapIndex = pools->queryCount;
			addQuery(
				context, commandBuffer, "Frame", "Pre-swap", INVALID_INDEX, context->swapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}
}

void dsGPUProfileContext_endFrame(dsGPUProfileContext* context)
{
	if (!context)
		return;

	dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);

	if (context->useQueries)
	{
		dsSpinlock_lock(&context->spinlock);
		bool submitResults = false;
		QueryPools* pools = context->queryPools + context->queryPoolIndex;
		if (!context->error)
		{
			addQuery(context, commandBuffer, NULL, NULL, pools->beginSwapIndex, context->swapCount);
			addQuery(
				context, commandBuffer, NULL, NULL, pools->beginFrameIndex, context->swapCount);
			submitResults = !context->error;
		}
		uint32_t prevIndex;
		if (context->queryPoolIndex < DELAY_FRAMES)
			prevIndex = DS_ARRAY_SIZE(context->queryPools) - DELAY_FRAMES + context->queryPoolIndex;
		else
			prevIndex = context->queryPoolIndex - DELAY_FRAMES;

		context->queryPoolIndex = (context->queryPoolIndex + 1)%DS_ARRAY_SIZE(context->queryPools);
		pools = context->queryPools + context->queryPoolIndex;
		pools->queryCount = 0;
		pools->totalRanges = 0;
		for (uint32_t i = 0; i < pools->poolCount; ++i)
			DS_VERIFY(dsGfxQueryPool_reset(pools->pools[i], commandBuffer, 0, QUERY_POOL_SIZE));

		++context->swapCount;
		context->error = false;
		dsSpinlock_unlock(&context->spinlock);

		if (submitResults)
			submitGPUProfileResults(context, context->queryPools + prevIndex);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_popDebugGroup(renderer, commandBuffer);
}

void dsGPUProfileContext_beginDeferredResources(dsGPUProfileContext* context)
{
	if (!context)
		return;

	dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);
	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_pushDebugGroup(renderer, commandBuffer, "Deferred Resources");

	if (context->useQueries)
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryPools* pools = context->queryPools + context->queryPoolIndex;
			commandBuffer->_profileInfo.beginDeferredResourcesIndex = pools->queryCount;
			commandBuffer->_profileInfo.beginDeferredResourcesSwapCount = context->swapCount;
			addQuery(context, commandBuffer, "Deferred Resources", "Total", INVALID_INDEX,
				context->swapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}
}

void dsGPUProfileContext_endDeferredResources(dsGPUProfileContext* context)
{
	if (!context)
		return;

	dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);

	if (context->useQueries)
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			addQuery(context, commandBuffer, NULL, NULL,
				commandBuffer->_profileInfo.beginDeferredResourcesIndex,
				commandBuffer->_profileInfo.beginDeferredResourcesSwapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_popDebugGroup(renderer, commandBuffer);
}

void dsGPUProfileContext_beginSwapBuffers(dsGPUProfileContext* context)
{
	if (!context || !context->useQueries)
		return;

	dsSpinlock_lock(&context->spinlock);
	if (!context->error)
	{
		dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);
		QueryPools* pools = context->queryPools + context->queryPoolIndex;
		addQuery(context, commandBuffer, NULL, NULL, pools->beginSwapIndex, context->swapCount);
		pools->beginSwapIndex = pools->queryCount;
		addQuery(context, commandBuffer, "Frame", "Swap buffers", INVALID_INDEX,
			context->swapCount);
	}
	dsSpinlock_unlock(&context->spinlock);
}

void dsGPUProfileContext_endSwapBuffers(dsGPUProfileContext* context)
{
	if (!context || !context->useQueries)
		return;

	dsSpinlock_lock(&context->spinlock);
	if (!context->error)
	{
		dsCommandBuffer* commandBuffer = getMainCommandBuffer(context);
		QueryPools* pools = context->queryPools + context->queryPoolIndex;
		addQuery(context, commandBuffer, NULL, NULL, pools->beginSwapIndex, context->swapCount);
		pools->beginSwapIndex = pools->queryCount;
		addQuery(context, commandBuffer, "Frame", "Post-swap", INVALID_INDEX, context->swapCount);
	}
	dsSpinlock_unlock(&context->spinlock);
}

void dsGPUProfileContext_beginSurface(
	dsGPUProfileContext* context, dsCommandBuffer* commandBuffer, const char* surfaceName)
{
	if (!context)
		return;

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_pushDebugGroup(renderer, commandBuffer, surfaceName);

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryPools* pools = context->queryPools + context->queryPoolIndex;
			commandBuffer->_profileInfo.beginSurfaceIndex = pools->queryCount;
			commandBuffer->_profileInfo.beginSurfaceSwapCount = context->swapCount;
			addQuery(
				context, commandBuffer, surfaceName, "Total", INVALID_INDEX, context->swapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}
}

void dsGPUProfileContext_endSurface(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer)
{
	if (!context)
		return;

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			addQuery(context, commandBuffer, NULL, NULL,
				commandBuffer->_profileInfo.beginSurfaceIndex,
				commandBuffer->_profileInfo.beginSurfaceSwapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_popDebugGroup(renderer, commandBuffer);
}

void dsGPUProfileContext_beginSubpass(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* framebufferName, const char* subpassName, bool secondary)
{
	if (!context)
		return;

	DS_ASSERT(!commandBuffer->secondaryRenderPassCommands);
	dsRenderer* renderer = commandBuffer->renderer;

	// Use only framebuffer for debug groups if debug groups would be disallowed for secondary
	// command buffers.
	commandBuffer->_profileInfo.subpassDebugGroups =
		!renderer->strictRenderPassSecondaryCommands || !secondary;
	if (commandBuffer->_profileInfo.subpassDebugGroups)
		dsRenderer_pushDebugGroup(renderer, commandBuffer, subpassName);
	else
		dsRenderer_pushDebugGroup(renderer, commandBuffer, framebufferName);

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryPools* pools = context->queryPools + context->queryPoolIndex;
			commandBuffer->_profileInfo.beginSubpassIndex = pools->queryCount;
			commandBuffer->_profileInfo.beginSubpassSwapCount = context->swapCount;
			addQuery(context, commandBuffer, framebufferName, subpassName, INVALID_INDEX,
				context->swapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}
}

void dsGPUProfileContext_nextSubpass(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* subpassName, bool secondary)
{
	if (!context)
		return;

	DS_ASSERT(commandBuffer->secondaryRenderPassCommands == secondary);
	dsRenderer* renderer = commandBuffer->renderer;

	// Avoid debug groups if originally couldn't or currently can't use them.
	if (commandBuffer->_profileInfo.subpassDebugGroups &&
		renderer->strictRenderPassSecondaryCommands && secondary)
	{
		dsRenderer_popDebugGroup(renderer, commandBuffer);
		dsRenderer_pushDebugGroup(renderer, commandBuffer, subpassName);
	}

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsCommandBuffer* submitBuffer = startCommandBuffer(context, commandBuffer);
		if (!submitBuffer)
			return;

		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryInfo* query = addQuery(context, submitBuffer, NULL, NULL,
				commandBuffer->_profileInfo.beginSubpassIndex,
				commandBuffer->_profileInfo.beginSubpassSwapCount);

			if (query)
			{
				QueryPools* pools = context->queryPools + context->queryPoolIndex;
				commandBuffer->_profileInfo.beginSubpassIndex = pools->queryCount;
				commandBuffer->_profileInfo.beginSubpassSwapCount = context->swapCount;

				// NOTE: The call to addQuery() might invalidate the memory for the category name,
				// so need to make a copy.
				char category[MAX_STRING_LENGTH];
				memcpy(category, query->category, sizeof(category));
				addQuery(context, submitBuffer, category, subpassName, INVALID_INDEX,
					context->swapCount);
			}
		}
		dsSpinlock_unlock(&context->spinlock);
		endCommandBuffer(commandBuffer, submitBuffer);
	}
}

void dsGPUProfileContext_endSubpass(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer)
{
	if (!context)
		return;

	DS_ASSERT(!commandBuffer->secondaryRenderPassCommands);
	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			addQuery(context, commandBuffer, NULL, NULL,
				commandBuffer->_profileInfo.beginSubpassIndex,
				commandBuffer->_profileInfo.beginSubpassSwapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_popDebugGroup(renderer, commandBuffer);
}

void dsGPUProfileContext_beginCompute(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer,
	const char* moduleName, const char* shaderName)
{
	if (!context)
		return;

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_pushDebugGroup(renderer, commandBuffer, shaderName);

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			QueryPools* pools = context->queryPools + context->queryPoolIndex;
			commandBuffer->_profileInfo.beginComputeIndex = pools->queryCount;
			commandBuffer->_profileInfo.beginComputeSwapCount = context->swapCount;
			addQuery(
				context, commandBuffer, moduleName, shaderName, INVALID_INDEX, context->swapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}
}

void dsGPUProfileContext_endCompute(dsGPUProfileContext* context, dsCommandBuffer* commandBuffer)
{
	if (!context)
		return;

	if (context->useQueries && commandBufferValid(commandBuffer))
	{
		dsSpinlock_lock(&context->spinlock);
		if (!context->error)
		{
			addQuery(context, commandBuffer, NULL, NULL,
				commandBuffer->_profileInfo.beginComputeIndex,
				commandBuffer->_profileInfo.beginComputeSwapCount);
		}
		dsSpinlock_unlock(&context->spinlock);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	dsRenderer_popDebugGroup(renderer, commandBuffer);
}

void dsGPUProfileContext_destroy(dsGPUProfileContext* context)
{
	if (!context)
		return;

	dsCommandBufferPool_destroy(context->secondaryCommandBuffers);
	if (context->useQueries)
	{
		for (uint32_t i = 0; i < DS_ARRAY_SIZE(context->queryPools); ++i)
		{
			QueryPools* pools = context->queryPools + i;
			for (uint32_t j = 0; j < pools->poolCount; ++j)
				DS_VERIFY(dsGfxQueryPool_destroy(pools->pools[j]));
			dsAllocator_free(context->allocator, pools->pools);
			dsAllocator_free(context->allocator, pools->queries);
		}

		dsAllocator_free(context->allocator, context->nodes);
		dsAllocator_free(context->allocator, context->hashTable);
		dsSpinlock_shutdown(&context->spinlock);
	}
	dsAllocator_free(context->allocator, context);
}
