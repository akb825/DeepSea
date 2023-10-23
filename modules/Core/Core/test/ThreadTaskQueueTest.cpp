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

#include "Helpers.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Thread/ThreadPool.h>
#include <DeepSea/Core/Thread/ThreadTaskQueue.h>
#include <gtest/gtest.h>

class ThreadTaskQueueTest : public testing::Test
{
public:
	ThreadTaskQueueTest()
		: allocator(reinterpret_cast<dsAllocator*>(&systemAllocator))
	{
	}

	void SetUp() override
	{
		EXPECT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
		threadPool = dsThreadPool_create(allocator, 0, 0, NULL, NULL, NULL);
		ASSERT_TRUE(threadPool);
	}

	void TearDown() override
	{
		EXPECT_TRUE(dsThreadPool_destroy(threadPool));
		EXPECT_EQ(0U, allocator->size);
	}

	dsSystemAllocator systemAllocator;
	dsAllocator* allocator;
	dsThreadPool* threadPool;
};

TEST_F(ThreadTaskQueueTest, Create)
{
	EXPECT_NULL_ERRNO(EINVAL, dsThreadTaskQueue_create(nullptr, threadPool, 20, 0));
	EXPECT_NULL_ERRNO(EINVAL, dsThreadTaskQueue_create(allocator, nullptr, 20, 0));
	EXPECT_NULL_ERRNO(EINVAL, dsThreadTaskQueue_create(allocator, threadPool, 0, 0));
	dsThreadTaskQueue* taskQueue = dsThreadTaskQueue_create(allocator, threadPool, 20, 0);
	EXPECT_TRUE(taskQueue);
	EXPECT_FALSE_ERRNO(EPERM, dsThreadPool_destroy(threadPool));
	dsThreadTaskQueue_destroy(taskQueue);
}

TEST_F(ThreadTaskQueueTest, WaitForTasks)
{
	constexpr unsigned int taskCount = 5;
	dsThreadTaskQueue* taskQueue = dsThreadTaskQueue_create(allocator, threadPool, taskCount, 0);
	ASSERT_TRUE(taskQueue);

	struct TaskState
	{
		unsigned int index;
		unsigned int* finishedCounter;
	};

	auto taskFunc = [](void* userData)
	{
		auto state = reinterpret_cast<TaskState*>(userData);
		EXPECT_EQ(state->index, *state->finishedCounter);
		++*state->finishedCounter;
	};

	unsigned int finishedCounter = 0;
	TaskState states[taskCount];
	dsThreadTask tasks[taskCount];
	for (unsigned int i = 0; i < taskCount; ++i)
	{
		states[i].index = i;
		states[i].finishedCounter = &finishedCounter;
		tasks[i].taskFunc = taskFunc;
		tasks[i].userData = states + i;
	}

	EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueue, tasks, taskCount));
	EXPECT_EQ(0U, finishedCounter);
	EXPECT_TRUE(dsThreadTaskQueue_waitForTasks(taskQueue));
	EXPECT_EQ(taskCount, finishedCounter);

	dsThreadTaskQueue_destroy(taskQueue);
}

TEST_F(ThreadTaskQueueTest, AddOverLimit)
{
	constexpr unsigned int taskCount = 5;
	dsThreadTaskQueue* taskQueue =
		dsThreadTaskQueue_create(allocator, threadPool, taskCount - 2, 0);
	ASSERT_TRUE(taskQueue);

	struct TaskState
	{
		unsigned int index;
		unsigned int* finishedCounter;
	};

	auto taskFunc = [](void* userData)
	{
		auto state = reinterpret_cast<TaskState*>(userData);
		EXPECT_EQ(state->index, *state->finishedCounter);
		++*state->finishedCounter;
	};

	unsigned int finishedCounter = 0;
	TaskState states[taskCount];
	dsThreadTask tasks[taskCount];
	for (unsigned int i = 0; i < taskCount; ++i)
	{
		states[i].index = i;
		states[i].finishedCounter = &finishedCounter;
		tasks[i].taskFunc = taskFunc;
		tasks[i].userData = states + i;
	}

	EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueue, tasks, taskCount));
	EXPECT_EQ(2U, finishedCounter);
	EXPECT_TRUE(dsThreadTaskQueue_waitForTasks(taskQueue));
	EXPECT_EQ(taskCount, finishedCounter);

	dsThreadTaskQueue_destroy(taskQueue);
}

TEST_F(ThreadTaskQueueTest, WaitOnDestroy)
{
	constexpr unsigned int taskCount = 5;
	dsThreadTaskQueue* taskQueue = dsThreadTaskQueue_create(allocator, threadPool, taskCount, 0);
	ASSERT_TRUE(taskQueue);

	struct TaskState
	{
		unsigned int index;
		unsigned int* finishedCounter;
	};

	auto taskFunc = [](void* userData)
	{
		auto state = reinterpret_cast<TaskState*>(userData);
		EXPECT_EQ(state->index, *state->finishedCounter);
		++*state->finishedCounter;
	};

	unsigned int finishedCounter = 0;
	TaskState states[taskCount];
	dsThreadTask tasks[taskCount];
	for (unsigned int i = 0; i < taskCount; ++i)
	{
		states[i].index = i;
		states[i].finishedCounter = &finishedCounter;
		tasks[i].taskFunc = taskFunc;
		tasks[i].userData = states + i;
	}

	EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueue, tasks, taskCount));
	EXPECT_EQ(0U, finishedCounter);
	dsThreadTaskQueue_destroy(taskQueue);
	EXPECT_EQ(taskCount, finishedCounter);
}

TEST_F(ThreadTaskQueueTest, RoundRobin)
{
	constexpr unsigned int taskQueueCount = 5;
	constexpr unsigned int taskCount = 5;
	dsThreadTaskQueue* taskQueues[taskQueueCount];
	for (unsigned int i = 0; i < taskQueueCount; ++i)
	{
		taskQueues[i] = dsThreadTaskQueue_create(allocator, threadPool, taskCount, 0);
		ASSERT_TRUE(taskQueues[i]);
	}

	dsMutex* finishMutex = dsMutex_create(allocator, "Finish");
	ASSERT_TRUE(finishMutex);

	dsConditionVariable* finishCondition = dsConditionVariable_create(allocator, "Finish");
	ASSERT_TRUE(finishCondition);

	struct TaskState
	{
		dsMutex* finishMutex;
		dsConditionVariable* finishCondition;
		unsigned int* prevTaskQueue;
		int* prevTask;
		unsigned int* executedCount;
		unsigned int taskQueue;
		unsigned int task;
	};

	unsigned int prevTaskQueue = taskQueueCount - 1;
	int prevTask = -1;
	unsigned int executedCount = 0;

	auto taskFunc = [](void* userData)
	{
		// MSVC won't implicitly capture constexpr values.
		constexpr unsigned int taskQueueCount = 5;
		constexpr unsigned int taskCount = 5;

		auto state = reinterpret_cast<TaskState*>(userData);
		unsigned int prevTaskQueue;
		int prevTask;
		if (state->taskQueue == 0)
		{
			prevTaskQueue = taskQueueCount - 1;
			prevTask = state->task - 1;
		}
		else
		{
			prevTaskQueue = state->taskQueue - 1;
			prevTask = state->task;
		}
		EXPECT_EQ(prevTaskQueue, *state->prevTaskQueue);
		EXPECT_EQ(prevTask, *state->prevTask);

		EXPECT_TRUE(dsMutex_lock(state->finishMutex));
		*state->prevTaskQueue = state->taskQueue;
		*state->prevTask = state->task;
		if (++*state->executedCount == taskQueueCount*taskCount)
		{
			EXPECT_TRUE(dsConditionVariable_notifyAll(state->finishCondition));
		}
		EXPECT_TRUE(dsMutex_unlock(state->finishMutex));
	};

	TaskState taskStates[taskQueueCount][taskCount];
	dsThreadTask tasks[taskQueueCount][taskCount];
	for (unsigned int i = 0; i < taskQueueCount; ++i)
	{
		for (unsigned int j = 0; j < taskCount; ++j)
		{
			taskStates[i][j].finishMutex = finishMutex;
			taskStates[i][j].finishCondition = finishCondition;
			taskStates[i][j].prevTaskQueue = &prevTaskQueue;
			taskStates[i][j].prevTask = &prevTask;
			taskStates[i][j].executedCount = &executedCount;
			taskStates[i][j].taskQueue = i;
			taskStates[i][j].task = j;

			tasks[i][j].taskFunc = taskFunc;
			tasks[i][j].userData = taskStates[i] + j;
		}

		EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueues[i], tasks[i], taskCount));
	}

	EXPECT_TRUE(dsThreadPool_setThreadCount(threadPool, 1));

	EXPECT_TRUE(dsMutex_lock(finishMutex));
	while (executedCount < taskQueueCount*taskCount)
		dsConditionVariable_wait(finishCondition, finishMutex);
	EXPECT_TRUE(dsMutex_unlock(finishMutex));

	for (unsigned int i = 0; i < taskQueueCount; ++i)
		dsThreadTaskQueue_destroy(taskQueues[i]);
	dsMutex_destroy(finishMutex);
	dsConditionVariable_destroy(finishCondition);
}

TEST_F(ThreadTaskQueueTest, MaxConcurrency)
{
	constexpr unsigned int taskCount = 20;
	constexpr unsigned int threadCount = 4;
	constexpr unsigned int maxConcurrency = 2;
	dsThreadTaskQueue* taskQueue = dsThreadTaskQueue_create(allocator, threadPool, taskCount,
		maxConcurrency);
	ASSERT_TRUE(taskQueue);

	struct TaskState
	{
		unsigned int concurrent = 0;
		unsigned int maxConcurrent = 0;
		unsigned int finishedCount = 0;
		unsigned int total = taskCount;
		dsMutex* stateMutex;
		dsConditionVariable* finishCondition;
	};

	TaskState state;

	state.stateMutex = dsMutex_create(allocator, "State");
	ASSERT_TRUE(state.stateMutex);

	state.finishCondition = dsConditionVariable_create(allocator, "Finish");
	ASSERT_TRUE(state.finishCondition);

	auto taskFunc = [](void* userData)
	{
		auto state = reinterpret_cast<TaskState*>(userData);

		EXPECT_TRUE(dsMutex_lock(state->stateMutex));
		++state->concurrent;
		if (state->concurrent > state->maxConcurrent)
			state->maxConcurrent = state->concurrent;
		EXPECT_TRUE(dsMutex_unlock(state->stateMutex));

		dsThread_sleep(1, "Wait");

		EXPECT_TRUE(dsMutex_lock(state->stateMutex));
		--state->concurrent;
		if (++state->finishedCount == state->total)
		{
			EXPECT_TRUE(dsConditionVariable_notifyAll(state->finishCondition));
		}
		EXPECT_TRUE(dsMutex_unlock(state->stateMutex));
	};

	dsThreadTask tasks[taskCount];
	for (unsigned int i = 0; i < taskCount; ++i)
	{
		tasks[i].taskFunc = taskFunc;
		tasks[i].userData = &state;
	}

	EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueue, tasks, taskCount));
	EXPECT_TRUE(dsThreadPool_setThreadCount(threadPool, threadCount));

	EXPECT_TRUE(dsMutex_lock(state.stateMutex));
	while (state.finishedCount < state.total)
		dsConditionVariable_wait(state.finishCondition, state.stateMutex);
	EXPECT_TRUE(dsMutex_unlock(state.stateMutex));

	// Wait should guarantee that max is reached. May need to change to EXPECT_GE() if somehow the
	// thread limit isn't consistently reached.
	EXPECT_EQ(maxConcurrency, state.maxConcurrent);

	dsThreadTaskQueue_destroy(taskQueue);
	dsMutex_destroy(state.stateMutex);
	dsConditionVariable_destroy(state.finishCondition);
}

TEST_F(ThreadTaskQueueTest, AddTaskWithinTask)
{
	constexpr unsigned int taskCount = 5;
	dsThreadTaskQueue* taskQueue = dsThreadTaskQueue_create(allocator, threadPool, taskCount, 0);
	ASSERT_TRUE(taskQueue);

	struct TaskState
	{
		unsigned int index;
		unsigned int* finishedCounter;
		dsThreadTaskQueue* taskQueue;
		dsThreadTask* nextTask;
	};

	auto taskFunc = [](void* userData)
	{
		auto state = reinterpret_cast<TaskState*>(userData);
		EXPECT_EQ(state->index, *state->finishedCounter);
		++*state->finishedCounter;
		if (state->nextTask)
		{
			EXPECT_TRUE(dsThreadTaskQueue_addTasks(state->taskQueue, state->nextTask, 1));
		}
	};

	unsigned int finishedCounter = 0;
	TaskState states[taskCount];
	dsThreadTask tasks[taskCount];
	for (unsigned int i = 0; i < taskCount; ++i)
	{
		states[i].index = i;
		states[i].finishedCounter = &finishedCounter;
		states[i].taskQueue = taskQueue;
		states[i].nextTask = i < taskCount - 1 ? tasks + i + 1 : nullptr;
		tasks[i].taskFunc = taskFunc;
		tasks[i].userData = states + i;
	}

	EXPECT_TRUE(dsThreadTaskQueue_addTasks(taskQueue, tasks, 1));
	EXPECT_EQ(0U, finishedCounter);
	EXPECT_TRUE(dsThreadTaskQueue_waitForTasks(taskQueue));
	EXPECT_EQ(taskCount, finishedCounter);

	dsThreadTaskQueue_destroy(taskQueue);
}
