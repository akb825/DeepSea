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

#include <DeepSea/Scene/SceneThreadManager.h>

#include "SceneThreadManagerInternal.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/SceneGlobalData.h>
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

#define COMMAND_BUFFER_POOL_SIZE 10
// 512 KB
#define THREAD_STACK_SIZE 524288

typedef enum ThreadState
{
	ThreadState_Initializing,
	ThreadState_Waiting,
	ThreadState_SharedItems,
	ThreadState_Pipeline,
	ThreadState_Stop,
	ThreadState_ThreadError,
	ThreadState_ResourceContextError
} ThreadState;

typedef struct DrawThread
{
	dsThread thread;
	dsSceneThreadManager* threadManager;
	ThreadState state;
	uint32_t sharedItemsIndex;
} DrawThread;

struct dsSceneThreadManager
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsMutex* stateMutex;
	dsSpinlock itemLock;
	dsConditionVariable* stateCondition;
	DrawThread* threads;
	uint32_t threadCount;
	uint32_t finishedCount;

	dsCommandBufferPool** computeCommandBuffers;
	uint32_t computeCommandBufferCount;
	uint32_t maxComputeCommandBuffers;

	dsCommandBufferPool** subpassCommandBuffers;
	uint32_t subpassCommandBufferCount;
	uint32_t maxSubpassCommandBuffers;

	dsCommandBuffer** commandBufferPointers;
	uint32_t commandBufferPointerCount;
	uint32_t maxCommandBufferPointers;

	const dsView* curView;
	const dsViewFramebufferInfo* curFramebufferInfos;
	dsFramebuffer** curFramebuffers;
	const uint32_t* curPipelineFramebuffers;

	uint32_t nextComputeCommandBuffer;
	uint32_t nextSubpassCommandBuffer;
	uint32_t nextItem;
	uint32_t nextSubpass;
	uint32_t nextSubpassItem;
	uint32_t nextCommandBuffer;
	uint64_t lastFrame;
};

static void processSharedItems(dsSceneThreadManager* threadManager, uint32_t index)
{
	const dsView* view = threadManager->curView;
	const dsScene* scene = view->scene;
	do
	{
		dsSceneItemList* itemList = NULL;
		dsCommandBuffer* commandBuffer = NULL;
		DS_VERIFY(dsSpinlock_lock(&threadManager->itemLock));
		const dsSceneItemLists* sharedItems = scene->sharedItems + index;
		if (threadManager->nextItem < sharedItems->count)
		{
			itemList = sharedItems->itemLists[threadManager->nextItem++];
			if (itemList->needsCommandBuffer)
			{
				commandBuffer =
					threadManager->commandBufferPointers[threadManager->nextCommandBuffer++];
				DS_ASSERT(threadManager->nextCommandBuffer <=
					threadManager->commandBufferPointerCount);
			}
		}
		DS_VERIFY(dsSpinlock_unlock(&threadManager->itemLock));

		if (!itemList)
			return;

		if (commandBuffer)
		{
			if (!dsCommandBuffer_begin(commandBuffer))
				continue;
		}

		itemList->commitFunc(itemList, threadManager->curView, commandBuffer);

		if (commandBuffer)
			DS_VERIFY(dsCommandBuffer_end(commandBuffer));
	} while (true);
}

static void processPipeline(dsSceneThreadManager* threadManager)
{
	const dsView* view = threadManager->curView;
	const dsScene* scene = view->scene;
	do
	{
		const dsScenePipelineItem* item = NULL;
		uint32_t subpass = 0;
		uint32_t subpassItem = 0;
		const dsViewFramebufferInfo* framebufferInfo = NULL;
		dsFramebuffer* framebuffer = NULL;
		dsCommandBuffer* commandBuffer = NULL;

		DS_VERIFY(dsSpinlock_lock(&threadManager->itemLock));
		if (threadManager->nextItem < scene->pipelineCount)
		{
			bool needsCommandBuffer;
			item = scene->pipeline + threadManager->nextItem;
			if (item->renderPass)
			{
				uint32_t framebufferIndex =
					threadManager->curPipelineFramebuffers[threadManager->nextItem];
				framebufferInfo = threadManager->curFramebufferInfos + framebufferIndex;
				framebuffer = threadManager->curFramebuffers[framebufferIndex];

				subpass = threadManager->nextSubpass;
				subpassItem = threadManager->nextSubpassItem++;

				if (threadManager->nextSubpassItem >= item->renderPass->drawLists[subpass].count)
				{
					++threadManager->nextSubpass;
					threadManager->nextSubpassItem = 0;
					if (threadManager->nextSubpass >= item->renderPass->renderPass->subpassCount)
					{
						++threadManager->nextItem;
						threadManager->nextSubpass = 0;
					}
				}
				needsCommandBuffer = true;
			}
			else
			{
				++threadManager->nextItem;
				needsCommandBuffer = item->computeItems->needsCommandBuffer;
			}

			if (needsCommandBuffer)
			{
				commandBuffer =
					threadManager->commandBufferPointers[threadManager->nextCommandBuffer++];
				DS_ASSERT(threadManager->nextCommandBuffer <=
					threadManager->commandBufferPointerCount);
			}
		}
		DS_VERIFY(dsSpinlock_unlock(&threadManager->itemLock));

		if (!item)
			return;

		if (item->renderPass)
		{
			dsAlignedBox3f viewport = framebufferInfo->viewport;
			viewport.min.x *= (float)framebuffer->width;
			viewport.max.x *= (float)framebuffer->width;
			viewport.min.y *= (float)framebuffer->height;
			viewport.max.y *= (float)framebuffer->height;

			dsRenderPass* renderPass = item->renderPass->renderPass;
			if (!dsCommandBuffer_beginSecondary(commandBuffer, framebuffer, renderPass, subpass,
					&viewport))
			{
				continue;
			}

			dsSceneItemList* itemList = item->renderPass->drawLists[subpass].itemLists[subpassItem];
			itemList->commitFunc(itemList, threadManager->curView, commandBuffer);

			DS_VERIFY(dsCommandBuffer_end(commandBuffer));
		}
		else
		{
			if (commandBuffer)
			{
				if (!dsCommandBuffer_begin(commandBuffer))
					continue;
			}

			dsSceneItemList* itemList = item->computeItems;
			itemList->commitFunc(itemList, threadManager->curView, commandBuffer);

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
			case ThreadState_SharedItems:
				processSharedItems(threadManager, drawThread->sharedItemsIndex);
				break;
			case ThreadState_Pipeline:
				processPipeline(threadManager);
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

static void triggerThreads(dsSceneThreadManager* threadManager, ThreadState state,
	uint32_t sharedItemsIndex)
{
	DS_ASSERT(threadManager->finishedCount == 0);
	DS_VERIFY(dsMutex_lock(threadManager->stateMutex));
	for (uint32_t i = 0; i < threadManager->threadCount; ++i)
	{
		threadManager->threads[i].state = state;
		threadManager->threads[i].sharedItemsIndex = sharedItemsIndex;
	}
	DS_VERIFY(dsConditionVariable_notifyAll(threadManager->stateCondition));
	DS_VERIFY(dsMutex_unlock(threadManager->stateMutex));
}

static dsCommandBuffer* getComputeCommandBuffer(dsSceneThreadManager* threadManager)
{
	uint32_t curPool = threadManager->nextComputeCommandBuffer/COMMAND_BUFFER_POOL_SIZE;
	uint32_t curBuffer = threadManager->nextComputeCommandBuffer % COMMAND_BUFFER_POOL_SIZE;
	if (curBuffer == 0)
	{
		if (curPool >= threadManager->computeCommandBufferCount)
		{
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->computeCommandBuffers, threadManager->computeCommandBufferCount,
					threadManager->maxComputeCommandBuffers, 1))
			{
				return NULL;
			}

			threadManager->computeCommandBuffers[curPool] =
				dsCommandBufferPool_create(threadManager->renderer, threadManager->allocator,
					dsCommandBufferUsage_Standard, COMMAND_BUFFER_POOL_SIZE);
			if (!threadManager->computeCommandBuffers[curPool])
			{
				--threadManager->computeCommandBufferCount;
				return NULL;
			}
		}
		else
		{
			if (!dsCommandBufferPool_reset(threadManager->computeCommandBuffers[curPool]))
				return NULL;
		}
	}

	++threadManager->nextComputeCommandBuffer;
	return threadManager->computeCommandBuffers[curPool]->currentBuffers[curBuffer];
}

static dsCommandBuffer* getSubpassCommandBuffer(dsSceneThreadManager* threadManager)
{
	uint32_t curPool = threadManager->nextSubpassCommandBuffer/COMMAND_BUFFER_POOL_SIZE;
	uint32_t curBuffer = threadManager->nextSubpassCommandBuffer % COMMAND_BUFFER_POOL_SIZE;
	if (curBuffer == 0)
	{
		if (curPool >= threadManager->subpassCommandBufferCount)
		{
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->subpassCommandBuffers, threadManager->subpassCommandBufferCount,
					threadManager->maxSubpassCommandBuffers, 1))
			{
				return NULL;
			}

			threadManager->subpassCommandBuffers[curPool] =
				dsCommandBufferPool_create(threadManager->renderer, threadManager->allocator,
					dsCommandBufferUsage_Secondary, COMMAND_BUFFER_POOL_SIZE);
			if (!threadManager->subpassCommandBuffers[curPool])
			{
				--threadManager->subpassCommandBufferCount;
				return NULL;
			}
		}
		else
		{
			if (!dsCommandBufferPool_reset(threadManager->subpassCommandBuffers[curPool]))
				return NULL;
		}
	}

	++threadManager->nextSubpassCommandBuffer;
	return threadManager->subpassCommandBuffers[curPool]->currentBuffers[curBuffer];
}

static bool setupForDraw(dsSceneThreadManager* threadManager, const dsScene* scene)
{
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
	{
		const dsSceneItemLists* sharedItems = scene->sharedItems + i;
		for (uint32_t j = 0; j < sharedItems->count; ++j)
		{
			if (!sharedItems->itemLists[j]->needsCommandBuffer)
				continue;

			uint32_t index = threadManager->commandBufferPointerCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->commandBufferPointers, threadManager->commandBufferPointerCount,
					threadManager->maxCommandBufferPointers, 1))
			{
				return false;
			}

			threadManager->commandBufferPointers[index] = getComputeCommandBuffer(threadManager);
			if (!threadManager->commandBufferPointers[index])
				return false;
		}
	}

	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* sceneRenderPass = scene->pipeline[i].renderPass;
		if (sceneRenderPass)
		{
			dsRenderPass* renderPass = sceneRenderPass->renderPass;
			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				uint32_t index = threadManager->commandBufferPointerCount;
				if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
						threadManager->commandBufferPointers,
						threadManager->commandBufferPointerCount,
						threadManager->maxCommandBufferPointers, drawLists->count))
				{
					return false;
				}

				for (uint32_t k = 0; k < drawLists->count; ++k, ++index)
				{
					threadManager->commandBufferPointers[index] =
						getSubpassCommandBuffer(threadManager);
					if (!threadManager->commandBufferPointers[index])
						return false;
				}
			}
		}
		else
		{
			DS_ASSERT(scene->pipeline[i].computeItems);
			if (!scene->pipeline[i].computeItems->needsCommandBuffer)
				continue;

			uint32_t index = threadManager->commandBufferPointerCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(threadManager->allocator,
					threadManager->commandBufferPointers, threadManager->commandBufferPointerCount,
					threadManager->maxCommandBufferPointers, 1))
			{
				return false;
			}

			threadManager->commandBufferPointers[index] = getComputeCommandBuffer(threadManager);
			if (!threadManager->commandBufferPointers[index])
				return false;
		}
	}

	return true;
}

static bool submitCommandBuffers(dsSceneThreadManager* threadManager,
	dsCommandBuffer* commandBuffer)
{
	const dsScene* scene = threadManager->curView->scene;
	uint32_t nextCommandBuffer = 0;
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
	{
		const dsSceneItemLists* sharedItems = scene->sharedItems + i;
		for (uint32_t j = 0; j < sharedItems->count; ++j)
		{
			if (!sharedItems->itemLists[j]->needsCommandBuffer)
				continue;

			dsCommandBuffer* submitBuffer =
				threadManager->commandBufferPointers[nextCommandBuffer++];
			if (!dsCommandBuffer_submit(commandBuffer, submitBuffer))
				return false;
		}
	}

	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* sceneRenderPass = scene->pipeline[i].renderPass;
		if (sceneRenderPass)
		{
			dsRenderPass* renderPass = sceneRenderPass->renderPass;
			uint32_t framebufferIndex = threadManager->curPipelineFramebuffers[i];
			const dsViewFramebufferInfo* framebufferInfo =
				threadManager->curFramebufferInfos + framebufferIndex;
			dsFramebuffer* framebuffer = threadManager->curFramebuffers[framebufferIndex];

			dsAlignedBox3f viewport = framebufferInfo->viewport;
			viewport.min.x *= (float)framebuffer->width;
			viewport.max.x *= (float)framebuffer->width;
			viewport.min.y *= (float)framebuffer->height;
			viewport.max.y *= (float)framebuffer->height;
			uint32_t clearValueCount =
				sceneRenderPass->clearValues ? sceneRenderPass->renderPass->attachmentCount : 0;
			if (!dsRenderPass_begin(renderPass, commandBuffer, framebuffer, &viewport,
					sceneRenderPass->clearValues, clearValueCount))
			{
				return false;
			}

			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsCommandBuffer* submitBuffer =
						threadManager->commandBufferPointers[nextCommandBuffer++];
					if (!dsCommandBuffer_submit(commandBuffer, submitBuffer))
						return false;
				}

				if (j != renderPass->subpassCount - 1)
					DS_VERIFY(dsRenderPass_nextSubpass(renderPass, commandBuffer));
			}

			DS_VERIFY(dsRenderPass_end(renderPass, commandBuffer));
		}
		else
		{
			DS_ASSERT(scene->pipeline[i].computeItems);
			if (!scene->pipeline[i].computeItems->needsCommandBuffer)
				continue;

			dsCommandBuffer* submitBuffer =
				threadManager->commandBufferPointers[nextCommandBuffer++];
			if (!dsCommandBuffer_submit(commandBuffer, submitBuffer))
				return false;
		}
	}
	DS_ASSERT(nextCommandBuffer == threadManager->commandBufferPointerCount);

	return true;
}

dsSceneThreadManager* dsSceneThreadManager_create(dsAllocator* allocator,
	dsRenderer* renderer, uint32_t threadCount)
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
	DS_VERIFY(dsSpinlock_initialize(&threadManager->itemLock));

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
	dsFramebuffer** framebuffers, const uint32_t* pipelineFramebuffers)
{
	const dsScene* scene = view->scene;
	dsRenderer* renderer = scene->renderer;

	threadManager->curView = view;
	threadManager->curFramebufferInfos = framebufferInfos;
	threadManager->curFramebuffers = framebuffers;
	threadManager->curPipelineFramebuffers = pipelineFramebuffers;
	threadManager->commandBufferPointerCount = 0;

	if (threadManager->lastFrame != renderer->frameNumber)
	{
		threadManager->nextComputeCommandBuffer = 0;
		threadManager->nextSubpassCommandBuffer = 0;
		threadManager->lastFrame = renderer->frameNumber;
	}

	if (!setupForDraw(threadManager, scene))
		return false;

	// Shared items first.
	threadManager->nextItem = 0;
	threadManager->nextCommandBuffer = 0;
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
	{
		if (scene->sharedItems[i].count == 0)
			continue;

		triggerThreads(threadManager, ThreadState_SharedItems, i);
		processSharedItems(threadManager, i);
		waitForThreads(threadManager);
	}

	// Once finished, main rendering pipeline.
	threadManager->nextItem = 0;
	threadManager->nextSubpass = 0;
	threadManager->nextSubpassItem = 0;
	triggerThreads(threadManager, ThreadState_Pipeline, 0);
	processPipeline(threadManager);
	waitForThreads(threadManager);

	return submitCommandBuffers(threadManager, commandBuffer);
}

bool dsSceneThreadManager_destroy(dsSceneThreadManager* threadManager)
{
	if (threadManager)
		return true;

	for (uint32_t i = 0; i < threadManager->computeCommandBufferCount; ++i)
	{
		if (!dsCommandBufferPool_destroy(threadManager->computeCommandBuffers[i]))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->computeCommandBuffers));

	for (uint32_t i = 0; i < threadManager->subpassCommandBufferCount; ++i)
	{
		if (!dsCommandBufferPool_destroy(threadManager->subpassCommandBuffers[i]))
		{
			DS_ASSERT(i == 0);
			DS_ASSERT(threadManager->maxComputeCommandBuffers == 0);
			return false;
		}
	}
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->subpassCommandBuffers));

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
	dsSpinlock_shutdown(&threadManager->itemLock);
	dsConditionVariable_destroy(threadManager->stateCondition);

	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager->commandBufferPointers));
	DS_VERIFY(dsAllocator_free(threadManager->allocator, threadManager));
	return true;
}
