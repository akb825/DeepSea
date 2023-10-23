/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Core/Thread/ThreadPool.h>

#include "ThreadPoolImpl.h"
#include "ThreadTaskQueueImpl.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <string.h>

static dsThreadReturnType threadFunc(void* userData)
{
	dsThreadPool* threadPool = (dsThreadPool*)userData;

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	// Find the index for this thread.
	dsThreadID thisThreadID = dsThread_thisThreadID();
	uint32_t threadIndex = 0;
	for (; threadIndex < threadPool->threadCount; ++threadIndex)
	{
		if (dsThread_equal(thisThreadID, dsThread_getID(threadPool->threads + threadIndex)))
			break;
	}
	DS_ASSERT(threadIndex < threadPool->threadCount);

	// Signal that this thread has started.
	DS_ASSERT(threadPool->waitThreadCount > 0);
	if (--threadPool->waitThreadCount == 0)
		DS_VERIFY(dsConditionVariable_notifyAll(threadPool->waitThreadCondition));

	dsThreadTask curTask = {NULL, NULL};
	do
	{
		if (threadPool->stop)
		{
			DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
			break;
		}
		else if (threadIndex >= threadPool->threadCount)
		{
			// Signal that this thread is stopping.
			DS_ASSERT(threadPool->waitThreadCount > 0);
			if (--threadPool->waitThreadCount == 0)
				DS_VERIFY(dsConditionVariable_notifyAll(threadPool->waitThreadCondition));
			DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
			break;
		}

		// Find a task to execute.
		dsThreadTaskQueue* curTaskQueue = NULL;
		for (uint32_t i = 0; i < threadPool->taskQueueCount; ++i)
		{
			uint32_t threadPoolIndex = (threadPool->curTaskQueue + i) % threadPool->taskQueueCount;
			dsThreadTaskQueue* thisTaskQueue = threadPool->taskQueues[threadPoolIndex];

			// Make sure this won't exceed the concurrency.
			uint32_t maxConcurrency;
			DS_ATOMIC_LOAD32(&thisTaskQueue->maxConcurrency, &maxConcurrency);
			uint32_t curConcurrency = DS_ATOMIC_FETCH_ADD32(&thisTaskQueue->executingTasks, 1);
			if ((maxConcurrency == 0 || curConcurrency < maxConcurrency) &&
				dsThreadTaskQueue_popTask(&curTask, thisTaskQueue))
			{
				curTaskQueue = thisTaskQueue;
				// Start searching at the next task queue for the next task that will be popped off
				// to use a round-robin.
				threadPool->curTaskQueue = (threadPoolIndex + 1) % threadPool->taskQueueCount;
				break;
			}
			else
				DS_ATOMIC_FETCH_ADD32(&thisTaskQueue->executingTasks, -1);
		}

		if (!curTaskQueue)
		{
			// Nothing to do: wait until the state changes.
			dsConditionVariable_wait(threadPool->stateCondition, threadPool->stateMutex);
			continue;
		}
		DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));

		DS_ASSERT(curTask.taskFunc);
		curTask.taskFunc(curTask.userData);

		// Decrement doesn't need to be locked.
		DS_ATOMIC_FETCH_ADD32(&curTaskQueue->executingTasks, -1);

		DS_VERIFY(dsMutex_lock(threadPool->stateMutex));
	} while (true);

	return 0;
}

bool dsThreadPool_addTaskQueue(dsThreadPool* threadPool, dsThreadTaskQueue* taskQueue)
{
	DS_ASSERT(threadPool);
	DS_ASSERT(taskQueue);

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	bool success = false;
	uint32_t index = threadPool->taskQueueCount;
	if (DS_RESIZEABLE_ARRAY_ADD(threadPool->allocator, threadPool->taskQueues,
			threadPool->taskQueueCount, threadPool->maxTaskQueues, 1))
	{
		threadPool->taskQueues[index] = taskQueue;
		success = true;
	}

	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
	return success;
}

void dsThreadPool_removeTaskQueue(dsThreadPool* threadPool, dsThreadTaskQueue* taskQueue)
{
	DS_ASSERT(threadPool);
	DS_ASSERT(taskQueue);

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	for (uint32_t i = 0; i < threadPool->taskQueueCount; ++i)
	{
		dsThreadTaskQueue** curTaskQueue = threadPool->taskQueues + i;
		if (*curTaskQueue != taskQueue)
			continue;

		// Constant-time removal since order doesn't matter.
		*curTaskQueue = threadPool->taskQueues[threadPool->taskQueueCount - 1];
		--threadPool->taskQueueCount;
		break;
	}

	if (threadPool->curTaskQueue >= threadPool->taskQueueCount)
		threadPool->curTaskQueue = 0;

	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
}

unsigned int dsThreadPool_defaultThreadCount(void)
{
	unsigned int threadCount = dsThread_logicalCoreCount();
	if (threadCount <= 1)
		return 1;
	else if (threadCount > DS_THREAD_POOL_MAX_THREADS)
		return DS_THREAD_POOL_MAX_THREADS;

	return threadCount - 1;
}

dsThreadPool* dsThreadPool_create(dsAllocator* allocator, unsigned int threadCount,
	size_t stackSize)
{
	if (!allocator || threadCount > DS_THREAD_POOL_MAX_THREADS)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "Thread pool allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsThreadPool* threadPool = DS_ALLOCATE_OBJECT(allocator, dsThreadPool);
	if (!threadPool)
		return NULL;

	memset(threadPool, 0, sizeof(dsThreadPool));
	threadPool->allocator = dsAllocator_keepPointer(allocator);
	threadPool->stackSize = stackSize;

	threadPool->stateMutex = dsMutex_create(allocator, "Thread Pool Mutex");
	if (!threadPool->stateMutex)
	{
		DS_VERIFY(dsAllocator_free(allocator, threadPool));
		return NULL;
	}

	threadPool->stateCondition = dsConditionVariable_create(allocator, "Thread Pool Condition");
	if (!threadPool->stateCondition)
	{
		DS_VERIFY(dsThreadPool_destroy(threadPool));
		return NULL;
	}

	threadPool->waitThreadCondition =
		dsConditionVariable_create(allocator, "Thread Pool Start/Stop Condition");
	if (!threadPool->waitThreadCondition)
	{
		DS_VERIFY(dsThreadPool_destroy(threadPool));
		return NULL;
	}

	if (!dsThreadPool_setThreadCount(threadPool, threadCount))
	{
		DS_VERIFY(dsThreadPool_destroy(threadPool));
		return NULL;
	}

	return threadPool;
}

unsigned int dsThreadPool_getThreadCount(const dsThreadPool* threadPool)
{
	if (!threadPool)
		return 0;

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));
	unsigned int threadCount = (unsigned int)threadPool->threadCount;
	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
	return threadCount;
}

unsigned int dsThreadPool_getThreadCountUnlocked(const dsThreadPool* threadPool)
{
	return threadPool ? (unsigned int)threadPool->threadCount : 0U;
}

bool dsThreadPool_setThreadCount(dsThreadPool* threadPool, unsigned int threadCount)
{
	if (!threadPool || threadCount > DS_THREAD_POOL_MAX_THREADS)
	{
		errno = EINVAL;
		return false;
	}

	bool success = true;
	uint32_t stopThreadCount = 0;
	dsThread waitThreads[DS_THREAD_POOL_MAX_THREADS];

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	// Wait if in the middle of waiting for threads to start or stop when setting the number of
	// threads concurrently.
	while (threadPool->waitThreadCount > 0)
		dsConditionVariable_wait(threadPool->waitThreadCondition, threadPool->stateMutex);

	if (threadCount < threadPool->threadCount)
	{
		stopThreadCount = threadPool->threadCount - threadCount;
		threadPool->waitThreadCount = stopThreadCount;

		// Move the threads to the local array so joining doesn't depend on the thread pool state.
		memcpy(waitThreads, threadPool->threads + threadCount, sizeof(dsThread)*stopThreadCount);
		threadPool->threadCount = threadCount;

		// Wake threads so they can be shut down. Also ensures that thread queues with a limited
		// concurrency will have the next tasks executed on threads that are still running.
		DS_VERIFY(dsConditionVariable_notifyAll(threadPool->stateCondition));
	}
	else if (threadCount > threadPool->threadCount)
	{
		uint32_t firstThread = threadPool->threadCount;
		uint32_t newThreads = threadCount - firstThread;
		threadPool->waitThreadCount = newThreads;
		success = DS_RESIZEABLE_ARRAY_ADD(threadPool->allocator, threadPool->threads,
			threadPool->threadCount, threadPool->maxThreads, newThreads);
		if (success)
		{
			for (uint32_t i = firstThread; i < threadCount; ++i)
			{
				if (!dsThread_create(threadPool->threads + i, &threadFunc, threadPool,
						threadPool->stackSize, "Thread Pool Worker"))
				{
					success = false;
					threadPool->threadCount = i;
					break;
				}
			}
		}
	}

	// Wait for the threads to either start or stop based on the new state. Avoids potential state
	// conflicts if the number of threads is set concurrently.
	while (threadPool->waitThreadCount > 0)
		dsConditionVariable_wait(threadPool->waitThreadCondition, threadPool->stateMutex);

	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));

	// Wait for any stopped threads. This no longer has any state tied directly to threadPool.
	for (unsigned int i = 0; i < stopThreadCount; ++i)
		dsThread_join(waitThreads + i, NULL);

	return success;
}

bool dsThreadPool_destroy(dsThreadPool* threadPool)
{
	if (!threadPool)
		return true;

	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	// Can't destroy when there's still task queues alive.
	bool success = threadPool->taskQueueCount == 0;
	if (success)
	{
		threadPool->stop = true;
		DS_VERIFY(dsConditionVariable_notifyAll(threadPool->stateCondition));
	}
	else
		errno = EPERM;

	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));

	if (!success)
		return false;

	for (uint32_t i = 0; i < threadPool->threadCount; ++i)
		dsThread_join(threadPool->threads + i, NULL);

	DS_VERIFY(dsAllocator_free(threadPool->allocator, threadPool->taskQueues));
	DS_VERIFY(dsAllocator_free(threadPool->allocator, threadPool->threads));
	dsMutex_destroy(threadPool->stateMutex);
	dsConditionVariable_destroy(threadPool->stateCondition);
	dsConditionVariable_destroy(threadPool->waitThreadCondition);
	DS_VERIFY(dsAllocator_free(threadPool->allocator, threadPool));
	return true;
}
