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
#include <gtest/gtest.h>

class ThreadPoolTest : public testing::Test
{
public:
	ThreadPoolTest()
		: allocator(reinterpret_cast<dsAllocator*>(&systemAllocator))
	{
	}

	void SetUp() override
	{
		EXPECT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	}

	void TearDown() override
	{
		EXPECT_EQ(0U, allocator->size);
	}

	dsSystemAllocator systemAllocator;
	dsAllocator* allocator;
};

static uint64_t splitMix64(uint64_t& state)
{
	state += 0x9e3779b97f4a7c15ULL;
	uint64_t value = state;
	value = (value ^ (value >> 30))*0xbf58476d1ce4e5b9ULL;
	value = (value ^ (value >> 27))*0x94d049bb133111ebULL;
	return value ^ (value >> 31);
}

TEST_F(ThreadPoolTest, Create)
{
	EXPECT_NULL_ERRNO(EINVAL, dsThreadPool_create(NULL, 2, 0));
	EXPECT_NULL_ERRNO(EINVAL, dsThreadPool_create(allocator, DS_THREAD_POOL_MAX_THREADS + 1, 0));
	dsThreadPool* threadPool = dsThreadPool_create(allocator, 2, 0);
	EXPECT_TRUE(threadPool);
	EXPECT_TRUE(dsThreadPool_destroy(threadPool));
}

TEST_F(ThreadPoolTest, SetThreads)
{
	dsThreadPool* threadPool = dsThreadPool_create(allocator, 2, 0);
	ASSERT_TRUE(threadPool);
	EXPECT_EQ(2U, dsThreadPool_getThreadCount(threadPool));
	EXPECT_TRUE(dsThreadPool_setThreadCount(threadPool, 4));
	EXPECT_EQ(4U, dsThreadPool_getThreadCount(threadPool));
	EXPECT_TRUE(dsThreadPool_setThreadCount(threadPool, 1));
	EXPECT_EQ(1U, dsThreadPool_getThreadCount(threadPool));
	EXPECT_TRUE(dsThreadPool_destroy(threadPool));
}

TEST_F(ThreadPoolTest, StressTestSetThreads)
{
	constexpr unsigned int setThreadCount = 10;
	constexpr unsigned int setCount = 20;
	constexpr unsigned int maxThreadCount = 100;

	struct ThreadState
	{
		dsThreadPool* threadPool;
		dsMutex* startMutex;
		dsConditionVariable* startCondition;
		bool* start;
		const unsigned int* threadCounts;
		unsigned int totalCount = setCount;
	};

	dsMutex* startMutex = dsMutex_create(allocator, "Start");
	ASSERT_TRUE(startMutex);
	dsConditionVariable* startCondition = dsConditionVariable_create(allocator, "Start");
	ASSERT_TRUE(startCondition);
	bool start = false;

	dsThreadPool* threadPool = dsThreadPool_create(allocator, 0, 0);
	ASSERT_TRUE(threadPool);

	uint64_t randomState = 0;
	unsigned int threadCounts[setThreadCount][setCount];
	ThreadState threadStates[setThreadCount];
	for (unsigned int i = 0; i < setThreadCount; ++i)
	{
		threadStates[i].startMutex = startMutex;
		threadStates[i].startCondition = startCondition;
		threadStates[i].start = &start;
		threadStates[i].threadPool = threadPool;
		threadStates[i].threadCounts = threadCounts[i];
		for (unsigned int j = 0; j < setCount; ++j)
		{
			threadCounts[i][j] =
				static_cast<unsigned int>((splitMix64(randomState) % maxThreadCount));
		}
	}

	auto threadFunc = [](void* userData) -> dsThreadReturnType
	{
		auto threadState = reinterpret_cast<ThreadState*>(userData);
		EXPECT_TRUE(dsMutex_lock(threadState->startMutex));
		while (!*threadState->start)
			dsConditionVariable_wait(threadState->startCondition, threadState->startMutex);
		EXPECT_TRUE(dsMutex_unlock(threadState->startMutex));

		for (unsigned int i = 0; i < threadState->totalCount; ++i)
		{
			EXPECT_TRUE(dsThreadPool_setThreadCount(threadState->threadPool,
				threadState->threadCounts[i]));
		}
		return 0;
	};

	dsThread threads[setThreadCount];
	for (unsigned int i = 0; i < setThreadCount; ++i)
	{
		EXPECT_TRUE(dsThread_create(threads + i, threadFunc, threadStates + i, 0,
			"Set thread count"));
	}

	// Start all threads at the same time.
	EXPECT_TRUE(dsMutex_lock(startMutex));
	start = true;
	EXPECT_TRUE(dsConditionVariable_notifyAll(startCondition));
	EXPECT_TRUE(dsMutex_unlock(startMutex));

	for (unsigned int i = 0; i < setThreadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_TRUE(dsThreadPool_destroy(threadPool));
	dsMutex_destroy(startMutex);
	dsConditionVariable_destroy(startCondition);
}
