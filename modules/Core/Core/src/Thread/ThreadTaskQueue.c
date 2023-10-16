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

#include <DeepSea/Core/Thread/ThreadTaskQueue.h>

#include "ThreadPoolImpl.h"
#include "ThreadTaskQueueImpl.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

bool dsThreadTaskQueue_popTask(dsThreadTask* outTask, dsThreadTaskQueue* taskQueue)
{
	DS_ASSERT(taskQueue);
	dsThreadTaskEntry* entry = taskQueue->taskHead;
	if (!entry)
		return false;

	*outTask = entry->task;
	taskQueue->taskHead = entry->next;
	if (!taskQueue->taskHead)
		taskQueue->taskTail = NULL;
	DS_VERIFY(dsAllocator_free((dsAllocator*)&taskQueue->taskAllocator, entry));
	return true;
}

size_t dsThreadTaskQueue_sizeof(void)
{
	return sizeof(dsThreadTaskQueue);
}

size_t dsThreadTaskQueue_fullAllocSize(uint32_t maxTasks)
{
	if (maxTasks == 0)
		return 0;

	return DS_ALIGNED_SIZE(sizeof(dsThreadTaskQueue)) +
		dsPoolAllocator_bufferSize(sizeof(dsThreadTaskEntry), maxTasks);
}

dsThreadTaskQueue* dsThreadTaskQueue_create(dsAllocator* allocator,
	dsThreadPool* threadPool, uint32_t maxTasks, uint32_t maxConcurrency)
{
	if (!allocator || threadPool == NULL || maxTasks == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsThreadTaskQueue_fullAllocSize(maxTasks);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsThreadTaskQueue* taskQueue = DS_ALLOCATE_OBJECT(&bufferAlloc, dsThreadTaskQueue);
	DS_ASSERT(taskQueue);

	taskQueue->allocator = dsAllocator_keepPointer(allocator);
	taskQueue->threadPool = threadPool;

	size_t taskPoolSize = dsPoolAllocator_bufferSize(sizeof(dsThreadTaskEntry), maxTasks);
	void* taskBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, taskPoolSize);
	DS_ASSERT(taskBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&taskQueue->taskAllocator, sizeof(dsThreadTaskEntry),
		maxTasks, taskBuffer, taskPoolSize));
	taskQueue->taskHead = NULL;
	taskQueue->taskTail = NULL;

	taskQueue->maxConcurrency = maxConcurrency;
	taskQueue->executingTasks = 0;
	DS_VERIFY(dsSpinlock_initialize(&taskQueue->addTaskLock));
	if (!dsThreadPool_addTaskQueue(threadPool, taskQueue))
	{
		dsSpinlock_shutdown(&taskQueue->addTaskLock);
		if (taskQueue->allocator)
			DS_VERIFY(dsAllocator_free(taskQueue->allocator, taskQueue));
	}

	return taskQueue;
}

uint32_t dsThreadTaskQueue_getMaxConcurrency(const dsThreadTaskQueue* taskQueue)
{
	if (!taskQueue)
		return 0;

	uint32_t maxConcurrency;
	DS_ATOMIC_LOAD32(&taskQueue->maxConcurrency, &maxConcurrency);
	return maxConcurrency;
}

bool dsThreadTaskQueue_setMaxConcurrency(dsThreadTaskQueue* taskQueue, uint32_t maxConcurrency)
{
	if (!taskQueue)
	{
		errno = EINVAL;
		return false;
	}

	dsThreadPool* threadPool = taskQueue->threadPool;
	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	uint32_t prevMaxConcurrency;
	DS_ATOMIC_EXCHANGE32(&taskQueue->maxConcurrency, &maxConcurrency, &prevMaxConcurrency);

	// Wake up the threads if we increased concurrency.
	if (prevMaxConcurrency < maxConcurrency)
		DS_VERIFY(dsConditionVariable_notifyAll(taskQueue->threadPool->stateCondition));

	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
	return true;
}

bool dsThreadTaskQueue_addTasks(dsThreadTaskQueue* taskQueue, const dsThreadTask* tasks,
	uint32_t taskCount)
{
	if (!taskQueue || (taskCount > 0 && !tasks))
	{
		errno = EINVAL;
		return false;
	}

	if (taskCount == 0)
		return true;

	for (uint32_t i = 0; i < taskCount; ++i)
	{
		if (!tasks[i].taskFunc)
		{
			errno = EINVAL;
			return false;
		}
	}

	dsThreadPool* threadPool = taskQueue->threadPool;
	// Keep a local list to only need one synchronization point with the thread pool.
	dsThreadTaskEntry* newTaskHead = NULL;
	dsThreadTaskEntry* newTaskTail = NULL;

	// Lock to keep tasks queued together unless maxTasks limit is exceeded.
	DS_VERIFY(dsSpinlock_lock(&taskQueue->addTaskLock));

	dsThreadTask curTask = {NULL, NULL};
	for (uint32_t i = 0; i < taskCount; ++i)
	{
		dsThreadTaskEntry* newEntry;
		do
		{
			// Don't check free count directly before allocation since locks are different between
			// adding tasks and popping tasks to run. Allocator is thread-safe, so only issue is it
			// may unnecessarily set errno, which shouldn't be a problem for a successful operation.
			newEntry = DS_ALLOCATE_OBJECT(&taskQueue->taskAllocator, dsThreadTaskEntry);
			if (newEntry)
				break;

			// No space: flush the current list of items and pop off the next item to execute.
			DS_VERIFY(dsSpinlock_unlock(&taskQueue->addTaskLock));
			DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

			// Commit any tasks queued so far.
			if (newTaskHead)
			{
				DS_ASSERT(newTaskTail);
				if (taskQueue->taskTail)
					taskQueue->taskTail->next = newTaskHead;
				else
					taskQueue->taskHead = newTaskHead;
				taskQueue->taskTail = newTaskTail;

				newTaskHead = NULL;
				newTaskTail = NULL;
			}

			if (!dsThreadTaskQueue_popTask(&curTask, taskQueue))
			{
				// Other threads may have already popped off all remaining tasks immediately after
				// the pool allocation failed, try again.
				DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
				DS_VERIFY(dsSpinlock_lock(&taskQueue->addTaskLock));
				continue;
			}
			DS_ASSERT(curTask.taskFunc);

			// Make sure the executing task count remains consistent in case a separate thread is
			// waiting on tasks to complete.
			DS_ATOMIC_FETCH_ADD32(&taskQueue->executingTasks, 1);

			// Also make sure that other threads can grab tasks in the meantime.
			if (taskQueue->taskHead)
				DS_VERIFY(dsConditionVariable_notifyAll(taskQueue->threadPool->stateCondition));
			DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));

			curTask.taskFunc(curTask.userData);
			DS_ATOMIC_FETCH_ADD32(&taskQueue->executingTasks, -1);

			DS_VERIFY(dsSpinlock_lock(&taskQueue->addTaskLock));
		} while (true);

		newEntry->next = NULL;
		newEntry->task = tasks[i];
		if (newTaskTail)
			newTaskTail->next = newEntry;
		else
		{
			DS_ASSERT(!taskQueue->taskHead);
			newTaskHead = newEntry;
		}
		newTaskTail = newEntry;
	}

	DS_VERIFY(dsSpinlock_unlock(&taskQueue->addTaskLock));

	// Update the task queue list and notify the thread pool to start executing.
	DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

	DS_ASSERT(newTaskHead);
	DS_ASSERT(newTaskTail);
	if (taskQueue->taskTail)
		taskQueue->taskTail->next = newTaskHead;
	else
		taskQueue->taskHead = newTaskHead;
	taskQueue->taskTail = newTaskTail;

	DS_VERIFY(dsConditionVariable_notifyAll(taskQueue->threadPool->stateCondition));
	DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));
	return true;
}

bool dsThreadTaskQueue_waitForTasks(dsThreadTaskQueue* taskQueue)
{
	if (!taskQueue)
	{
		errno = EINVAL;
		return false;
	}

	dsThreadPool* threadPool = taskQueue->threadPool;
	dsThreadTask curTask = {NULL, NULL};
	do
	{
		// Queue list is synchronized with the thread pool.
		DS_VERIFY(dsMutex_lock(threadPool->stateMutex));

		// Try to pull a task off a queue.
		bool isDone;
		if (dsThreadTaskQueue_popTask(&curTask, taskQueue))
		{
			// Increment the number of current executing tasks to allow other threads to respect the
			// max concurrency, but don't avoid executing the task here if it's exceeded.
			DS_ATOMIC_FETCH_ADD32(&taskQueue->executingTasks, 1);
			isDone = false;
		}
		else
		{
			// Need to keep waiting if there's tasks currently executing on other threads.
			uint32_t executingTasks;
			DS_ATOMIC_LOAD32(&taskQueue->executingTasks, &executingTasks);
			isDone = executingTasks == 0;
			curTask.taskFunc = NULL;
		}

		DS_VERIFY(dsMutex_unlock(threadPool->stateMutex));

		if (isDone)
			break;

		// Execute the task if we pulled one off, otherwise yeild for other processes.
		if (curTask.taskFunc)
		{
			curTask.taskFunc(curTask.userData);
			DS_ATOMIC_FETCH_ADD32(&taskQueue->executingTasks, -1);
		}
		else
			dsThread_yield();
	} while (true);
	return true;
}

void dsThreadTaskQueue_destroy(dsThreadTaskQueue* taskQueue)
{
	if (!taskQueue)
		return;

	DS_VERIFY(dsThreadTaskQueue_waitForTasks(taskQueue));
	dsThreadPool_removeTaskQueue(taskQueue->threadPool, taskQueue);
	dsSpinlock_shutdown(&taskQueue->addTaskLock);
	if (taskQueue->allocator)
		DS_VERIFY(dsAllocator_free(taskQueue->allocator, taskQueue));
}
