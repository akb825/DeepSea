/*
 * Copyright 2019-2023 Aaron Barany
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
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>

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

// 512 KB
#define THREAD_STACK_SIZE 524288

typedef enum ThreadState
{
	ThreadState_Initializing,
	ThreadState_Waiting,
	ThreadState_Processing,
	ThreadState_Stop,
	ThreadState_ThreadError,
	ThreadState_ResourceContextError
} ThreadState;

typedef struct CommandBufferInfo
{
	dsCommandBuffer* commandBuffer;
	dsSceneItemList* itemList;
	dsSceneRenderPass* renderPass;
	uint32_t subpass;
	uint32_t framebuffer;
} CommandBufferInfo;

typedef struct DrawThread
{
	dsThread thread;
	dsSceneThreadManager* threadManager;
	ThreadState state;
} DrawThread;

struct dsSceneThreadManager
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsMutex* stateMutex;
	dsConditionVariable* stateCondition;
	DrawThread* threads;
	uint32_t threadCount;
	uint32_t finishedCount;

	dsCommandBufferPool* computeCommandBuffers;
	dsCommandBufferPool* subpassCommandBuffers;
	bool resetComputeCommandBuffers;
	bool resetSubpassCommandBuffers;

	CommandBufferInfo* commandBufferInfos;
	uint32_t commandBufferInfoCount;
	uint32_t maxCommandBufferInfos;

	const dsView* curView;
	const dsViewFramebufferInfo* curFramebufferInfos;
	const dsRotatedFramebuffer* curFramebuffers;

	uint32_t nextCommandBuffer;
	uint64_t lastFrame;
};

static void processCommandBuffers(dsSceneThreadManager* threadManager)
{
	const dsView* view = threadManager->curView;
	do
	{
		uint32_t nextCommandBuffer = DS_ATOMIC_FETCH_ADD32(&threadManager->nextCommandBuffer, 1);
		if (nextCommandBuffer >= threadManager->commandBufferInfoCount)
			return;

		const CommandBufferInfo* commandBufferInfo =
			threadManager->commandBufferInfos + nextCommandBuffer;
		dsCommandBuffer* commandBuffer = commandBufferInfo->commandBuffer;
		dsSceneItemList* itemList = commandBufferInfo->itemList;
		DS_ASSERT(itemList->commitFunc);
		dsSceneRenderPass* renderPass = commandBufferInfo->renderPass;
		if (renderPass)
		{
			DS_ASSERT(commandBuffer);
			// Skipped due to framebuffer out of range. (e.g. support up to N layers, but have fewer
			// in the currently bound offscreen)
			const dsRotatedFramebuffer* framebuffer =
				threadManager->curFramebuffers + commandBufferInfo->framebuffer;
			if (!framebuffer->framebuffer)
				continue;

			const dsViewFramebufferInfo* framebufferInfo = threadManager->curFramebufferInfos +
				commandBufferInfo->framebuffer;
			dsAlignedBox3f viewport = framebufferInfo->viewport;
			dsView_adjustViewport(&viewport, view, framebuffer->rotated);
			viewport.min.x *= (float)framebuffer->framebuffer->width;
			viewport.max.x *= (float)framebuffer->framebuffer->width;
			viewport.min.y *= (float)framebuffer->framebuffer->height;
			viewport.max.y *= (float)framebuffer->framebuffer->height;

			if (!dsCommandBuffer_beginSecondary(commandBuffer, framebuffer->framebuffer,
					renderPass->renderPass, commandBufferInfo->subpass, &viewport))
			{
				continue;
			}

			itemList->commitFunc(itemList, view, commandBuffer);

			DS_VERIFY(dsCommandBuffer_end(commandBuffer));
		}
		else
		{
			if (commandBuffer)
			{
				if (!dsCommandBuffer_begin(commandBuffer))
					continue;
			}

			itemList->commitFunc(itemList, view, commandBuffer);

			if (commandBuffer)
				DS_VERIFY(dsCommandBuffer_end(commandBuffer));
		}
	} while (true);
}

static dsThreadReturnType threadFunc(void* userData)
{
	DrawThread* drawThread = (DrawThread*)userData;
	dsSceneThreadManager* threadManager = drawThread->threadManager;

	DS_ASSERT(drawThread->state == ThreadState_Initializing);
	dsResourceManager* resourceManager = threadManager->renderer->resourceManager;
	ThreadState nextState;
	if (dsResourceManager_createResourceContext(resourceManager))
		nextState = ThreadState_Waiting;
	else
		nextState = ThreadState_ResourceContextError;

	DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
	drawThread->state = nextState;
	++threadManager->finishedCount;
	DS_VERIFY(dsConditionVariable_notifyAll(threadManager->stateCondition));
	DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));

	if (nextState == ThreadState_ResourceContextError)
		return 0;

	do
	{
		DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
		ThreadState state;
		while (drawThread->state == ThreadState_Waiting)
			dsConditionVariable_wait(threadManager->stateCondition, threadManager->stateMutex);
		state = drawThread->state;
		DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));

		switch (state)
		{
			case ThreadState_Processing:
				processCommandBuffers(threadManager);
				break;
			case ThreadState_Stop:
				dsResourceManager_destroyResourceContext(resourceManager);
				return 0;
			default:
				DS_ASSERT(false);
				break;
		}

		DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
		drawThread->state = ThreadState_Waiting;
		++threadManager->finishedCount;
		DS_VERIFY(dsConditionVariable_notifyAll(threadManager->stateCondition));
		DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));

	} while (true);
}

static void waitForThreads(dsSceneThreadManager* threadManager)
{
	DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
	while (threadManager->finishedCount != threadManager->threadCount)
		dsConditionVariable_wait(threadManager->stateCondition, threadManager->stateMutex);
	DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));
	threadManager->finishedCount = 0;
}

static void triggerThreads(dsSceneThreadManager* threadManager)
{
	uint32_t count = threadManager->commandBufferInfoCount - threadManager->nextCommandBuffer;
	if (count == 0)
		return;
	else if (count == 1)
		processCommandBuffers(threadManager);
	else
	{
		DS_ASSERT(threadManager->finishedCount == 0);
		DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
		for (uint32_t i = 0; i < threadManager->threadCount; ++i)
			threadManager->threads[i].state = ThreadState_Processing;
		DS_VERIFY(dsConditionVariable_notifyAll(threadManager->stateCondition));
		DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));

		processCommandBuffers(threadManager);
		waitForThreads(threadManager);
	}

	// Over-counted in loop.
	threadManager->nextCommandBuffer = threadManager->commandBufferInfoCount;
}

static dsCommandBuffer* getComputeCommandBuffer(dsSceneThreadManager* threadManager)
{
	if (!threadManager->computeCommandBuffers)
	{
		threadManager->computeCommandBuffers = dsCommandBufferPool_create(threadManager->renderer,
			threadManager->allocator, dsCommandBufferUsage_Standard);
		if (!threadManager->computeCommandBuffers)
			return NULL;
		threadManager->resetComputeCommandBuffers = false;
	}
	else if (threadManager->resetComputeCommandBuffers)
	{
		if (!dsCommandBufferPool_reset(threadManager->computeCommandBuffers))
			return NULL;
		threadManager->resetComputeCommandBuffers = false;
	}

	dsCommandBuffer** commandBuffer =
		dsCommandBufferPool_createCommandBuffers(threadManager->computeCommandBuffers, 1);
	if (!commandBuffer)
		return NULL;

	return *commandBuffer;
}

static dsCommandBuffer** getSubpassCommandBuffers(dsSceneThreadManager* threadManager,
	uint32_t count)
{
	if (!threadManager->subpassCommandBuffers)
	{
		threadManager->subpassCommandBuffers = dsCommandBufferPool_create(threadManager->renderer,
			threadManager->allocator, dsCommandBufferUsage_Secondary);
		if (!threadManager->subpassCommandBuffers)
			return NULL;
		threadManager->resetSubpassCommandBuffers = false;
	}
	else if (threadManager->resetSubpassCommandBuffers)
	{
		if (!dsCommandBufferPool_reset(threadManager->subpassCommandBuffers))
			return NULL;
		threadManager->resetSubpassCommandBuffers = false;
	}

	return dsCommandBufferPool_createCommandBuffers(threadManager->subpassCommandBuffers, count);
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
		commandBufferInfo->itemList = itemList;
		commandBufferInfo->renderPass = NULL;
		commandBufferInfo->subpass = 0;
		commandBufferInfo->framebuffer = 0;
		if (itemList->needsCommandBuffer)
		{
			commandBufferInfo->commandBuffer = getComputeCommandBuffer(threadManager);
			if (!commandBufferInfo->commandBuffer)
				return false;
		}
		else
			commandBufferInfo->commandBuffer = NULL;
	}

	triggerThreads(threadManager);
	return true;
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
			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsSceneItemList* itemList = drawLists->itemLists[k];
					if (!itemList->preRenderPassFunc)
						continue;

					uint32_t index = threadManager->commandBufferInfoCount;
					if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
							threadManager->commandBufferInfos,
							threadManager->commandBufferInfoCount,
							threadManager->maxCommandBufferInfos, 1))
					{
						return false;
					}

					CommandBufferInfo* commandBufferInfo =
						threadManager->commandBufferInfos + index;

					commandBufferInfo->commandBuffer = getComputeCommandBuffer(threadManager);
					if (!commandBufferInfo->commandBuffer)
						return false;

					commandBufferInfo->itemList = itemList;
					commandBufferInfo->renderPass = NULL;
					commandBufferInfo->subpass = 0;
					commandBufferInfo->framebuffer = 0;
				}
			}

			// Render pass command buffers.
			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				uint32_t startIndex = threadManager->commandBufferInfoCount;
				if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
						threadManager->commandBufferInfos, threadManager->commandBufferInfoCount,
						threadManager->maxCommandBufferInfos, drawLists->count))
				{
					return false;
				}

				dsCommandBuffer** commandBuffers = getSubpassCommandBuffers(threadManager,
					drawLists->count);
				if (!commandBuffers)
					return false;

				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					CommandBufferInfo* commandBufferInfo =
						threadManager->commandBufferInfos + startIndex + k;
					commandBufferInfo->commandBuffer = commandBuffers[k];
					commandBufferInfo->itemList = drawLists->itemLists[k];
					commandBufferInfo->renderPass = sceneRenderPass;
					commandBufferInfo->subpass = j;
					commandBufferInfo->framebuffer = framebuffer;
				}
			}
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

			CommandBufferInfo* commandBufferInfo =
				threadManager->commandBufferInfos + index;

			if (itemList->needsCommandBuffer)
			{
				commandBufferInfo->commandBuffer = getComputeCommandBuffer(threadManager);
				if (!commandBufferInfo->commandBuffer)
					return false;
			}
			else
				commandBufferInfo->commandBuffer = NULL;

			commandBufferInfo->itemList = itemList;
			commandBufferInfo->renderPass = NULL;
			commandBufferInfo->subpass = 0;
			commandBufferInfo->framebuffer = 0;
		}
	}

	triggerThreads(threadManager);
	return true;
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
		{
			DS_ASSERT(!commandBufferInfo->renderPass);
			continue;
		}

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
		else if (commandBufferInfo->subpass != prevSubpass)
		{
			DS_ASSERT(prevRenderPass);
			DS_ASSERT(commandBufferInfo->subpass == prevSubpass + 1);
			DS_ASSERT(commandBufferInfo->framebuffer == prevFramebuffer);
			if (!dsRenderPass_nextSubpass(prevRenderPass->renderPass, commandBuffer,
					commandBufferInfo->subpass))
			{
				return false;
			}
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
	uint32_t threadCount)
{
	if (!allocator || !renderer || threadCount == 0)
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

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneThreadManager)) + dsMutex_fullAllocSize() +
		dsConditionVariable_fullAllocSize() + DS_ALIGNED_SIZE(sizeof(DrawThread)*threadCount);
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
	threadManager->stateMutex = dsMutex_create((dsAllocator*)&bufferAlloc, "Scene Thread State");
	DS_ASSERT(threadManager->stateMutex);

	threadManager->stateCondition = dsConditionVariable_create((dsAllocator*)&bufferAlloc,
		"Scene Thread Condition");
	DS_ASSERT(threadManager->stateCondition);

	threadManager->threads = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, DrawThread, threadCount);
	DS_ASSERT(threadManager->threads);
	threadManager->threadCount = threadCount;

	for (uint32_t i = 0; i < threadCount; ++i)
	{
		DrawThread* thread = threadManager->threads + i;
		thread->state = ThreadState_Initializing;
		thread->threadManager = threadManager;
		if (!dsThread_create(&thread->thread, &threadFunc, thread, THREAD_STACK_SIZE, "Scene Draw"))
		{
			// If the thread failed to create, manually set the state as if it did execute.
			DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
			thread->state = ThreadState_ThreadError;
			++threadManager->finishedCount;
			DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));
		}
	}
	waitForThreads(threadManager);

	// Handle errors for creating or running the threads.
	bool threadError = false;
	bool resourceContextError = false;
	for (uint32_t i = 0; i < threadCount; ++i)
	{
		switch (threadManager->threads[i].state)
		{
			case ThreadState_Waiting:
				break;
			case ThreadState_ThreadError:
				threadError = true;
				break;
			case ThreadState_ResourceContextError:
				resourceContextError = true;
				break;
			default:
				DS_ASSERT(false);
				break;
		}
	}

	if (resourceContextError)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Couldn't create a resource context for each draw thread.");
	}

	if (threadError || resourceContextError)
	{
		dsSceneThreadManager_destroy(threadManager);
		return NULL;
	}

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

	if (threadManager->lastFrame != renderer->frameNumber)
	{
		threadManager->resetComputeCommandBuffers = true;
		threadManager->resetSubpassCommandBuffers = true;
		threadManager->lastFrame = renderer->frameNumber;
	}

	// Shared items first.
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
		triggerSharedItems(threadManager, scene, i);

	// Once finished, main rendering pipeline.
	triggerDraw(threadManager, scene, pipelineFramebuffers);

	return submitCommandBuffers(threadManager, commandBuffer);
}

bool dsSceneThreadManager_destroy(dsSceneThreadManager* threadManager)
{
	if (!threadManager)
		return true;

	if (!dsCommandBufferPool_destroy(threadManager->computeCommandBuffers))
		return false;

	if (!dsCommandBufferPool_destroy(threadManager->subpassCommandBuffers))
	{
		DS_ASSERT(!threadManager->computeCommandBuffers);
		return false;
	}

	DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
	for (uint32_t i = 0; i < threadManager->threadCount; ++i)
	{
		DrawThread* thread = threadManager->threads + i;
		if (thread->state != ThreadState_ThreadError)
			thread->state = ThreadState_Stop;
	}
	DS_VERIFY(dsConditionVariable_notifyAll(threadManager->stateCondition));
	DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));

	for (uint32_t i = 0; i < threadManager->threadCount; ++i)
	{
		DrawThread* thread = threadManager->threads + i;
		if (thread->state == ThreadState_Stop)
			DS_VERIFY(dsThread_join(&thread->thread, NULL));
	}

	dsMutex_destroy(threadManager->stateMutex);
	dsConditionVariable_destroy(threadManager->stateCondition);

	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->commandBufferInfos));
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager));
	return true;
}
