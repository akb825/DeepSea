/*
 * Copyright 2019-2025 Aaron Barany
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

#include <DeepSea/Scene/SceneThreadManager.h>

#include "SceneThreadManagerInternal.h"
#include "SceneTypes.h"
#include "ViewInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/ThreadPool.h>
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Thread/ThreadTaskQueue.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/Render/RenderPass.h>

#include <string.h>

#define MAX_TASKS 100

// Keep to 32 bytes on a 64-bit system to ensure cache friendliness.
typedef struct CommandBufferInfo
{
	dsCommandBuffer* commandBuffer;
	dsSceneItemList* itemList;
	dsSceneRenderPass* renderPass;
	// Index of companion if renderPass is NULL and itemList has preRenderPassFunc.
	uint32_t subpass;
	uint32_t framebuffer;
} CommandBufferInfo;

typedef struct TaskData
{
	dsSceneThreadManager* threadManager;
	CommandBufferInfo* commandBufferInfo;
} TaskData;

typedef struct ThreadCommandBufferPools
{
	dsCommandBufferPool* computeCommandBuffers;
	dsCommandBufferPool* subpassCommandBuffers;
	bool inUse;
} ThreadCommandBufferPools;

struct dsSceneThreadManager
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsThreadPool* threadPool;
	dsThreadTaskQueue* taskQueue;

	ThreadCommandBufferPools* commandBufferPools;
	uint32_t commandBufferPoolsCount;
	uint32_t maxCommandBufferPools;
	dsSpinlock commandBufferPoolLock;
	dsThreadStorage threadCommandBufferPool;

	CommandBufferInfo* commandBufferInfos;
	uint32_t commandBufferInfoCount;
	uint32_t maxCommandBufferInfos;

	TaskData* taskData;
	uint32_t maxTaskData;
	uint32_t nextCommandBuffer;

	const dsView* curView;
	const dsViewFramebufferInfo* curFramebufferInfos;
	const dsRotatedFramebuffer* curFramebuffers;
	uint64_t lastFrame;
};

static ThreadCommandBufferPools* getCommandBufferPools(dsSceneThreadManager* threadManager)
{
	ThreadCommandBufferPools* commandBufferPools =
		(ThreadCommandBufferPools*)dsThreadStorage_get(threadManager->threadCommandBufferPool);
	if (commandBufferPools)
		return commandBufferPools;

	// Find a command buffer pool on the first call on this thread.
	DS_VERIFY(dsSpinlock_lock(&threadManager->commandBufferPoolLock));
	for (uint32_t i = 0; i < threadManager->commandBufferPoolsCount; ++i)
	{
		if (!threadManager->commandBufferPools[i].inUse)
		{
			commandBufferPools = threadManager->commandBufferPools + i;
			break;
		}
	}
	DS_ASSERT(commandBufferPools);
	commandBufferPools->inUse = true;
	DS_VERIFY(dsSpinlock_unlock(&threadManager->commandBufferPoolLock));

	DS_VERIFY(dsThreadStorage_set(threadManager->threadCommandBufferPool, commandBufferPools));
	return commandBufferPools;
}

static dsCommandBuffer* getComputeCommandBuffer(dsSceneThreadManager* threadManager)
{
	ThreadCommandBufferPools* commandBufferPools = getCommandBufferPools(threadManager);
	DS_ASSERT(commandBufferPools);

	if (!commandBufferPools->computeCommandBuffers)
	{
		commandBufferPools->computeCommandBuffers = dsCommandBufferPool_create(
			threadManager->renderer, threadManager->allocator, dsCommandBufferUsage_Standard);
		if (!commandBufferPools->computeCommandBuffers)
			return NULL;
	}

	dsCommandBuffer** commandBuffer =
		dsCommandBufferPool_createCommandBuffers(commandBufferPools->computeCommandBuffers, 1);
	if (!commandBuffer)
		return NULL;

	return *commandBuffer;
}

static dsCommandBuffer* getSubpassCommandBuffer(dsSceneThreadManager* threadManager)
{
	ThreadCommandBufferPools* commandBufferPools = getCommandBufferPools(threadManager);
	DS_ASSERT(commandBufferPools);

	if (!commandBufferPools->subpassCommandBuffers)
	{
		commandBufferPools->subpassCommandBuffers = dsCommandBufferPool_create(
			threadManager->renderer, threadManager->allocator, dsCommandBufferUsage_Secondary);
		if (!commandBufferPools->subpassCommandBuffers)
			return NULL;
	}

	dsCommandBuffer** commandBuffer =
		dsCommandBufferPool_createCommandBuffers(commandBufferPools->subpassCommandBuffers, 1);
	if (!commandBuffer)
		return NULL;

	return *commandBuffer;
}

static bool processCommandBufferRenderPass(dsCommandBuffer* commandBuffer,
	const dsView* view, dsSceneItemList* itemList, const dsSceneRenderPass* renderPass,
	uint32_t subpass, const dsRotatedFramebuffer* framebuffer,
	const dsViewFramebufferInfo* framebufferInfo)
{
	dsAlignedBox3f viewport = framebufferInfo->viewport;
	dsView_adjustViewport(&viewport, view, framebuffer->rotated);
	viewport.min.x *= (float)framebuffer->framebuffer->width;
	viewport.max.x *= (float)framebuffer->framebuffer->width;
	viewport.min.y *= (float)framebuffer->framebuffer->height;
	viewport.max.y *= (float)framebuffer->framebuffer->height;

	if (!dsCommandBuffer_beginSecondary(commandBuffer, framebuffer->framebuffer,
			renderPass->renderPass, subpass, &viewport, dsGfxOcclusionQueryState_Disabled))
	{
		return false;
	}

	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
	itemList->commitFunc(itemList, view, commandBuffer);
	DS_PROFILE_SCOPE_END();

	DS_VERIFY(dsCommandBuffer_end(commandBuffer));
	return true;
}

static void taskFunc(void* userData)
{
	const TaskData* taskData = (const TaskData*)userData;
	dsSceneThreadManager* threadManager = taskData->threadManager;
	CommandBufferInfo* commandBufferInfo = taskData->commandBufferInfo;
	const dsView* view = threadManager->curView;
	dsSceneItemList* itemList = commandBufferInfo->itemList;
	DS_ASSERT(itemList->commitFunc);
	dsSceneRenderPass* renderPass = commandBufferInfo->renderPass;
	if (renderPass)
	{
		dsCommandBuffer* commandBuffer = getSubpassCommandBuffer(threadManager);
		if (!commandBuffer)
			return;

		// If the render pass has a pre render pass call, it would have been handled then.
		DS_ASSERT(!itemList->preRenderPassFunc);
		processCommandBufferRenderPass(commandBuffer, view, itemList, renderPass,
			commandBufferInfo->subpass,
			threadManager->curFramebuffers + commandBufferInfo->framebuffer,
			threadManager->curFramebufferInfos + commandBufferInfo->framebuffer);
		commandBufferInfo->commandBuffer = commandBuffer;
	}
	else
	{
		if (itemList->preRenderPassFunc)
		{
			dsCommandBuffer* commandBuffer = getComputeCommandBuffer(threadManager);
			if (!commandBuffer || !dsCommandBuffer_begin(commandBuffer))
				return;

			DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
			itemList->preRenderPassFunc(itemList, view, commandBuffer);
			DS_PROFILE_SCOPE_END();
			DS_VERIFY(dsCommandBuffer_end(commandBuffer));
			commandBufferInfo->commandBuffer = commandBuffer;

			// Immediately process the corresponding render pass command buffer to avoid
			// thread synchronization issues.
			DS_ASSERT(commandBufferInfo->subpass < threadManager->commandBufferInfoCount);
			CommandBufferInfo* otherCommandBufferInfo =
				threadManager->commandBufferInfos + commandBufferInfo->subpass;
			DS_ASSERT(otherCommandBufferInfo->itemList == itemList);
			DS_ASSERT(otherCommandBufferInfo->renderPass);

			commandBuffer = getSubpassCommandBuffer(threadManager);
			if (!commandBuffer)
				return;

			processCommandBufferRenderPass(commandBuffer, view, itemList,
				otherCommandBufferInfo->renderPass, otherCommandBufferInfo->subpass,
				threadManager->curFramebuffers + otherCommandBufferInfo->framebuffer,
				threadManager->curFramebufferInfos + otherCommandBufferInfo->framebuffer);
			otherCommandBufferInfo->commandBuffer = commandBuffer;
		}
		else
		{
			dsCommandBuffer* commandBuffer = NULL;
			if (itemList->needsCommandBuffer)
			{
				commandBuffer = getComputeCommandBuffer(threadManager);
				if (!commandBuffer || !dsCommandBuffer_begin(commandBuffer))
					return;
			}

			DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
			itemList->commitFunc(itemList, view, commandBuffer);
			DS_PROFILE_SCOPE_END();

			if (commandBuffer)
			{
				DS_VERIFY(dsCommandBuffer_end(commandBuffer));
				commandBufferInfo->commandBuffer = commandBuffer;
			}
		}
	}
}

static bool triggerThreads(dsSceneThreadManager* threadManager)
{
	// Pre-allocate enough memory for all tasks.
	uint32_t taskDataCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator, threadManager->taskData, taskDataCount,
			threadManager->maxTaskData, threadManager->commandBufferInfoCount))
	{
		return false;
	}
	DS_ASSERT(taskDataCount == threadManager->commandBufferInfoCount);
	taskDataCount = 0;

	uint32_t firstCommandBuffer = threadManager->nextCommandBuffer;
	uint32_t commandBufferCount = threadManager->commandBufferInfoCount - firstCommandBuffer;

	// Set up the tasks for each command buffer.
	bool didQueue = false;
	uint32_t taskCount = 0;
	dsThreadTask tasks[MAX_TASKS];
	for (uint32_t i = 0; i < commandBufferCount; ++i)
	{
		CommandBufferInfo* commandBufferInfo =
			threadManager->commandBufferInfos + i + firstCommandBuffer;
		// Render passes with a pre render-pass function are processed with the pre render pass.
		if (commandBufferInfo->renderPass && commandBufferInfo->itemList->preRenderPassFunc)
			continue;

		DS_ASSERT(taskDataCount < threadManager->maxTaskData);
		TaskData* taskData = threadManager->taskData + (taskDataCount++);
		taskData->threadManager = threadManager;
		taskData->commandBufferInfo = commandBufferInfo;

		DS_ASSERT(taskCount < MAX_TASKS);
		dsThreadTask* task = tasks + (taskCount++);
		task->taskFunc = &taskFunc;
		task->userData = taskData;
		if (taskCount == MAX_TASKS)
		{
			didQueue = true;
			DS_VERIFY(dsThreadTaskQueue_addTasks(threadManager->taskQueue, tasks, taskCount));
			taskCount = 0;
		}
	}

	// Queue any remaining buffered tasks. Execute single task now if only one total.
	if (!didQueue && taskCount == 1)
		tasks[0].taskFunc(tasks[0].userData);
	else if (taskCount > 0)
	{
		didQueue = true;
		DS_VERIFY(dsThreadTaskQueue_addTasks(threadManager->taskQueue, tasks, taskCount));
	}

	if (didQueue)
		DS_VERIFY(dsThreadTaskQueue_waitForTasks(threadManager->taskQueue));
	threadManager->nextCommandBuffer = threadManager->commandBufferInfoCount;
	return true;
}

static bool triggerSharedItems(dsSceneThreadManager* threadManager, const dsScene* scene,
	uint32_t index)
{
	DS_ASSERT(index < scene->sharedItemCount);
	const dsSceneItemLists* sharedItems = scene->sharedItems + index;
	uint32_t processCount = 0;
	for (uint32_t i = 0; i < sharedItems->count; ++i)
		processCount += sharedItems->itemLists[i]->commitFunc != NULL;
	if (processCount == 0)
		return true;

	uint32_t commandBufferIndex = threadManager->commandBufferInfoCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
			threadManager->commandBufferInfos, threadManager->commandBufferInfoCount,
			threadManager->maxCommandBufferInfos, processCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < sharedItems->count; ++i)
	{
		dsSceneItemList* itemList = sharedItems->itemLists[i];
		if (!itemList->commitFunc)
			continue;

		CommandBufferInfo* commandBufferInfo = threadManager->commandBufferInfos +
			(commandBufferIndex++);
		commandBufferInfo->commandBuffer = NULL;
		commandBufferInfo->itemList = itemList;
		commandBufferInfo->renderPass = NULL;
		commandBufferInfo->subpass = 0;
		commandBufferInfo->framebuffer = 0;
	}

	return triggerThreads(threadManager);
}

static bool triggerDraw(dsSceneThreadManager* threadManager, const dsScene* scene,
	const uint32_t* pipelineFramebuffers)
{
	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* sceneRenderPass = scene->pipeline[i].renderPass;
		if (sceneRenderPass)
		{
			dsRenderPass* renderPass = sceneRenderPass->renderPass;
			uint32_t framebuffer = pipelineFramebuffers[i];
			// Skipped due to framebuffer out of range. (e.g. support up to N layers, but have fewer
			// in the currently bound offscreen)
			if (!threadManager->curFramebuffers[framebuffer].framebuffer)
				continue;

			// Pre-renderpass command buffers.
			uint32_t itemListCount = 0;
			uint32_t preRenderPassCount = 0;
			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				itemListCount += drawLists->count;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsSceneItemList* itemList = drawLists->itemLists[k];
					preRenderPassCount += itemList->preRenderPassFunc != NULL;
				}
			}

			if (preRenderPassCount > 0)
			{
				uint32_t startIndex = threadManager->commandBufferInfoCount;
				if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
						threadManager->commandBufferInfos, threadManager->commandBufferInfoCount,
						threadManager->maxCommandBufferInfos, preRenderPassCount))
				{
					return false;
				}

				// Keep track of the companion for the render pass command buffer.
				uint32_t companionIndex = startIndex + preRenderPassCount;
				uint32_t curIndex = startIndex;
				for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
				{
					const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
					for (uint32_t k = 0; k < drawLists->count; ++k, ++companionIndex)
					{
						dsSceneItemList* itemList = drawLists->itemLists[k];
						if (!itemList->preRenderPassFunc)
							continue;

						CommandBufferInfo* commandBufferInfo =
							threadManager->commandBufferInfos + (curIndex++);
						commandBufferInfo->commandBuffer = NULL;
						commandBufferInfo->itemList = itemList;
						commandBufferInfo->renderPass = NULL;
						commandBufferInfo->subpass = companionIndex;
						commandBufferInfo->framebuffer = 0;
					}
				}

				DS_ASSERT(curIndex == threadManager->commandBufferInfoCount);
				DS_ASSERT(companionIndex == threadManager->commandBufferInfoCount + itemListCount);
			}

			// Render pass command buffers.
			uint32_t curIndex = threadManager->commandBufferInfoCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->commandBufferInfos, threadManager->commandBufferInfoCount,
					threadManager->maxCommandBufferInfos, itemListCount))
			{
				return false;
			}

			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k, ++curIndex)
				{
					CommandBufferInfo* commandBufferInfo =
						threadManager->commandBufferInfos + curIndex;
					commandBufferInfo->commandBuffer = NULL;
					commandBufferInfo->itemList = drawLists->itemLists[k];
					commandBufferInfo->renderPass = sceneRenderPass;
					commandBufferInfo->subpass = j;
					commandBufferInfo->framebuffer = framebuffer;
				}
			}

			DS_ASSERT(curIndex == threadManager->commandBufferInfoCount);
		}
		else
		{
			dsSceneItemList* itemList = scene->pipeline[i].computeItems;
			if (!itemList->commitFunc)
				continue;

			uint32_t index = threadManager->commandBufferInfoCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->commandBufferInfos, threadManager->commandBufferInfoCount,
					threadManager->maxCommandBufferInfos, 1))
			{
				return false;
			}

			CommandBufferInfo* commandBufferInfo = threadManager->commandBufferInfos + index;
			commandBufferInfo->commandBuffer = NULL;
			commandBufferInfo->itemList = itemList;
			commandBufferInfo->renderPass = NULL;
			commandBufferInfo->subpass = 0;
			commandBufferInfo->framebuffer = 0;
		}
	}

	return triggerThreads(threadManager);
}

static bool submitCommandBuffers(dsSceneThreadManager* threadManager,
	dsCommandBuffer* commandBuffer)
{
	dsSceneRenderPass* prevRenderPass = NULL;
	uint32_t prevSubpass = 0;
	uint32_t prevFramebuffer = 0;
	for (uint32_t i = 0; i < threadManager->commandBufferInfoCount; ++i)
	{
		CommandBufferInfo* commandBufferInfo = threadManager->commandBufferInfos + i;
		if (!commandBufferInfo->commandBuffer)
			continue;

		if (commandBufferInfo->renderPass != prevRenderPass ||
			commandBufferInfo->framebuffer != prevFramebuffer)
		{
			if (prevRenderPass)
			{
				DS_ASSERT(prevSubpass + 1 == prevRenderPass->renderPass->subpassCount);
				dsRenderPass_end(prevRenderPass->renderPass, commandBuffer);
				prevRenderPass = NULL;
			}

			if (commandBufferInfo->renderPass)
			{
				dsSceneRenderPass* renderPass = commandBufferInfo->renderPass;
				const dsViewFramebufferInfo* framebufferInfo =
					threadManager->curFramebufferInfos + commandBufferInfo->framebuffer;
				const dsRotatedFramebuffer* framebuffer = threadManager->curFramebuffers +
					commandBufferInfo->framebuffer;
				DS_ASSERT(framebuffer->framebuffer);

				dsAlignedBox3f viewport = framebufferInfo->viewport;
				dsView_adjustViewport(&viewport, threadManager->curView, framebuffer->rotated);
				viewport.min.x *= (float)framebuffer->framebuffer->width;
				viewport.max.x *= (float)framebuffer->framebuffer->width;
				viewport.min.y *= (float)framebuffer->framebuffer->height;
				viewport.max.y *= (float)framebuffer->framebuffer->height;
				uint32_t clearValueCount =
					renderPass->clearValues ? renderPass->renderPass->attachmentCount : 0;
				if (!dsRenderPass_begin(renderPass->renderPass, commandBuffer,
						framebuffer->framebuffer, &viewport, renderPass->clearValues,
						clearValueCount, true))
				{
					return false;
				}

				DS_ASSERT(commandBufferInfo->subpass == 0);
				prevRenderPass = renderPass;
				prevSubpass = 0;
				prevFramebuffer = commandBufferInfo->framebuffer;
			}
		}
		else if (commandBufferInfo->renderPass && commandBufferInfo->subpass != prevSubpass)
		{
			DS_ASSERT(prevRenderPass);
			DS_ASSERT(commandBufferInfo->subpass == prevSubpass + 1);
			DS_ASSERT(commandBufferInfo->framebuffer == prevFramebuffer);
			if (!dsRenderPass_nextSubpass(prevRenderPass->renderPass, commandBuffer,
					commandBufferInfo->subpass))
			{
				return false;
			}

			prevSubpass = commandBufferInfo->subpass;
		}

		if (!dsCommandBuffer_submit(commandBuffer, commandBufferInfo->commandBuffer))
		{
			if (prevRenderPass)
				dsRenderPass_end(prevRenderPass->renderPass, commandBuffer);
			return false;
		}
	}

	if (prevRenderPass && !dsRenderPass_end(prevRenderPass->renderPass, commandBuffer))
		return false;

	return true;
}

dsSceneThreadManager* dsSceneThreadManager_create(dsAllocator* allocator, dsRenderer* renderer,
	dsThreadPool* threadPool)
{
	if (!allocator || !renderer || !threadPool)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene thread manager allocator must support freeing memory.");
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneThreadManager)) +
		dsThreadTaskQueue_fullAllocSize(MAX_TASKS);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneThreadManager* threadManager = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneThreadManager);
	DS_ASSERT(threadManager);
	memset(threadManager, 0, sizeof(dsSceneThreadManager));

	threadManager->allocator = dsAllocator_keepPointer(allocator);
	threadManager->renderer = renderer;
	threadManager->threadPool = threadPool;
	threadManager->taskQueue = dsThreadTaskQueue_create((dsAllocator*)&bufferAlloc, threadPool,
		MAX_TASKS, 0);
	DS_ASSERT(threadManager->taskQueue);
	DS_VERIFY(dsSpinlock_initialize(&threadManager->commandBufferPoolLock));
	return threadManager;
}

bool dsSceneThreadManager_draw(dsSceneThreadManager* threadManager, const dsView* view,
	dsCommandBuffer* commandBuffer, const dsViewFramebufferInfo* framebufferInfos,
	const dsRotatedFramebuffer* framebuffers, const uint32_t* pipelineFramebuffers)
{
	const dsScene* scene = view->scene;
	dsRenderer* renderer = scene->renderer;

	threadManager->commandBufferInfoCount = 0;
	threadManager->curView = view;
	threadManager->curFramebufferInfos = framebufferInfos;
	threadManager->curFramebuffers = framebuffers;
	threadManager->nextCommandBuffer = 0;

	// New thread-local storage each time so it gets re-initialized to NULL. Should be a fast
	// operation to just assign an index.
	DS_PROFILE_SCOPE_START("Prepare");
	if (!dsThreadStorage_initialize(&threadManager->threadCommandBufferPool))
	{
		DS_PROFILE_SCOPE_END();
		return false;
	}

	uint32_t totalThreadCount = dsThreadPool_getThreadCountUnlocked(threadManager->threadPool) + 1;
	if (threadManager->commandBufferPoolsCount < totalThreadCount)
	{
		// New command buffer pools.
		uint32_t origCount = threadManager->commandBufferPoolsCount;
		uint32_t addCount = totalThreadCount - origCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator, threadManager->commandBufferPools,
				threadManager->commandBufferPoolsCount, threadManager->maxCommandBufferPools,
				addCount))
		{
			dsThreadStorage_shutdown(&threadManager->threadCommandBufferPool);
			DS_PROFILE_SCOPE_END();
			return false;
		}

		DS_ASSERT(threadManager->commandBufferPoolsCount == totalThreadCount);
		memset(threadManager->commandBufferPools + origCount, 0,
			sizeof(ThreadCommandBufferPools)*addCount);
	}
	else if (threadManager->commandBufferPoolsCount > totalThreadCount)
	{
		// Free any command buffer pools now since number of threads isn't expected to change often.
		for (uint32_t i = totalThreadCount; i < threadManager->commandBufferPoolsCount; ++i)
		{
			ThreadCommandBufferPools* commandBufferPools = threadManager->commandBufferPools + i;
			if (!dsCommandBufferPool_destroy(commandBufferPools->computeCommandBuffers) ||
				!dsCommandBufferPool_destroy(commandBufferPools->subpassCommandBuffers))
			{
				dsThreadStorage_shutdown(&threadManager->threadCommandBufferPool);
				DS_PROFILE_SCOPE_END();
				return false;
			}

			threadManager->commandBufferInfoCount = totalThreadCount;
		}
	}

	for (uint32_t i = 0; i < threadManager->commandBufferPoolsCount; ++i)
		threadManager->commandBufferPools[i].inUse = false;

	if (threadManager->lastFrame != renderer->frameNumber)
	{
		for (uint32_t i = 0; i < threadManager->commandBufferPoolsCount; ++i)
		{
			ThreadCommandBufferPools* commandBufferPools = threadManager->commandBufferPools + i;
			if (commandBufferPools->computeCommandBuffers)
				DS_VERIFY(dsCommandBufferPool_reset(commandBufferPools->computeCommandBuffers));
			if (commandBufferPools->subpassCommandBuffers)
				DS_VERIFY(dsCommandBufferPool_reset(commandBufferPools->subpassCommandBuffers));
		}
		threadManager->lastFrame = renderer->frameNumber;
	}
	DS_PROFILE_SCOPE_END();

	// Shared items first.
	DS_PROFILE_SCOPE_START("Shared Items");
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
		triggerSharedItems(threadManager, scene, i);
	DS_PROFILE_SCOPE_END();

	// Once finished, main rendering pipeline.
	DS_PROFILE_SCOPE_START("Draw");
	triggerDraw(threadManager, scene, pipelineFramebuffers);
	DS_PROFILE_SCOPE_END();

	DS_PROFILE_SCOPE_START("Submit");
	bool success = submitCommandBuffers(threadManager, commandBuffer);
	DS_PROFILE_SCOPE_END();
	dsThreadStorage_shutdown(&threadManager->threadCommandBufferPool);
	return success;
}

bool dsSceneThreadManager_destroy(dsSceneThreadManager* threadManager)
{
	if (!threadManager)
		return true;

	for (uint32_t i = 0; i < threadManager->commandBufferPoolsCount; ++i)
	{
		ThreadCommandBufferPools* commandBufferPools = threadManager->commandBufferPools + i;
		if (!dsCommandBufferPool_destroy(commandBufferPools->computeCommandBuffers) ||
			!dsCommandBufferPool_destroy(commandBufferPools->subpassCommandBuffers))
		{
			return false;
		}
	}

	dsThreadTaskQueue_destroy(threadManager->taskQueue);
	dsSpinlock_shutdown(&threadManager->commandBufferPoolLock);
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->commandBufferPools));
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->commandBufferInfos));
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->taskData));
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager));
	return true;
}
