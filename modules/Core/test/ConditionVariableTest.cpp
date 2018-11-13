/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Timer.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsConditionVariable* condition;
	dsMutex* mutex;
	bool ready;
	uint32_t executed;
};

dsThreadReturnType threadFunc(void* data)
{
	ThreadData* threadData = (ThreadData*)data;

	EXPECT_TRUE(dsMutex_lock(threadData->mutex));
	while (!threadData->ready)
	{
		EXPECT_EQ(dsConditionVariableResult_Success, dsConditionVariable_wait(threadData->condition,
			threadData->mutex));
	}
	EXPECT_TRUE(dsMutex_unlock(threadData->mutex));

	DS_ATOMIC_FETCH_ADD32(&threadData->executed, 1);
	return 0;
}

} // namespace

TEST(ConditionVariable, CreateEmptyAllocator)
{
	dsAllocator allocator = {};
	EXPECT_NULL_ERRNO(EINVAL, dsConditionVariable_create(&allocator, nullptr));
}

TEST(ConditionVariable, CreateAllocator)
{
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	dsConditionVariable* condition = dsConditionVariable_create((dsAllocator*)&allocator, nullptr);
	EXPECT_NE(nullptr, condition);
	dsConditionVariable_destroy(condition);
}

TEST(ConditionVariable, CreateAllocatorNoFree)
{
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	((dsAllocator*)&allocator)->freeFunc = nullptr;
	dsConditionVariable* condition = dsConditionVariable_create((dsAllocator*)&allocator, nullptr);
	EXPECT_NE(nullptr, condition);
	dsConditionVariable_destroy(condition);
	dsSystemAllocator_free(&allocator, condition);
}

TEST(ConditionVariable, Null)
{
	EXPECT_EQ_ERRNO(EINVAL, dsConditionVariableResult_Error,
		dsConditionVariable_wait(nullptr, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsConditionVariable_notifyOne(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsConditionVariable_notifyAll(nullptr));
}

TEST(ConditionVariable, NotifyAll)
{
	ThreadData threadData;
	threadData.condition = dsConditionVariable_create(nullptr, nullptr);
	ASSERT_NE(nullptr, threadData.condition);
	threadData.mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, threadData.mutex);
	threadData.ready = false;
	threadData.executed = 0;

	const unsigned int threadCount = 10;
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	EXPECT_TRUE(dsMutex_lock(threadData.mutex));
	EXPECT_EQ(0U, threadData.executed);
	threadData.ready = true;
	EXPECT_TRUE(dsConditionVariable_notifyAll(threadData.condition));
	EXPECT_TRUE(dsMutex_unlock(threadData.mutex));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);

	dsConditionVariable_destroy(threadData.condition);
	dsMutex_destroy(threadData.mutex);
}

TEST(ConditionVariable, NotifyOne)
{
	ThreadData threadData;
	threadData.condition = dsConditionVariable_create(nullptr, nullptr);
	ASSERT_NE(nullptr, threadData.condition);
	threadData.mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, threadData.mutex);
	threadData.ready = false;
	threadData.executed = 0;

	const unsigned int threadCount = 5;
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	EXPECT_TRUE(dsMutex_lock(threadData.mutex));
	EXPECT_EQ(0U, threadData.executed);
	threadData.ready = true;
	EXPECT_TRUE(dsMutex_unlock(threadData.mutex));

	uint32_t executed;
	do
	{
		EXPECT_TRUE(dsMutex_lock(threadData.mutex));
		EXPECT_TRUE(dsConditionVariable_notifyOne(threadData.condition));
		executed = threadData.executed;
		EXPECT_TRUE(dsMutex_unlock(threadData.mutex));
		dsThread_sleep(1, nullptr);
	} while (executed != threadCount);

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);

	dsConditionVariable_destroy(threadData.condition);
	dsMutex_destroy(threadData.mutex);
}

TEST(ConditionVariable, TimedWait)
{
	dsConditionVariable* condition = dsConditionVariable_create(nullptr, nullptr);
	ASSERT_NE(nullptr, condition);
	dsMutex* mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, mutex);

	dsTimer timer = dsTimer_create();
	double startTime = dsTimer_time(timer);
	EXPECT_TRUE(dsMutex_lock(mutex));
	EXPECT_EQ(dsConditionVariableResult_Timeout, dsConditionVariable_timedWait(condition, mutex,
		1150));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	double endTime = dsTimer_time(timer);
	// Give a generous error due to scheduling quantums.
	EXPECT_NEAR(1150, (endTime - startTime)*1000, 20);

	dsConditionVariable_destroy(condition);
	dsMutex_destroy(mutex);
}
