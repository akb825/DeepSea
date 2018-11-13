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
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Atomic.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsMutex* mutex;
	unsigned int counter;
	unsigned int executed;
};

dsThreadReturnType threadFunc(void* data)
{
	ThreadData* threadData = (ThreadData*)data;
	dsThread_sleep(1, nullptr);

	EXPECT_TRUE(dsMutex_lock(threadData->mutex));

	++threadData->counter;
	EXPECT_EQ(1U, threadData->counter);
	--threadData->counter;
	EXPECT_EQ(0U, threadData->counter);
	++threadData->executed;

	EXPECT_TRUE(dsMutex_unlock(threadData->mutex));
	return 0;
}

} // namespace

TEST(Mutex, CreateEmptyAllocator)
{
	dsAllocator allocator = {};
	EXPECT_NULL_ERRNO(EINVAL, dsMutex_create(&allocator, nullptr));
}

TEST(Mutex, CreateAllocator)
{
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	dsMutex* mutex = dsMutex_create((dsAllocator*)&allocator, nullptr);
	EXPECT_NE(nullptr, mutex);
	dsMutex_destroy(mutex);
}

TEST(Mutex, CreateAllocatorNoFree)
{
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	((dsAllocator*)&allocator)->freeFunc = nullptr;
	dsMutex* mutex = dsMutex_create((dsAllocator*)&allocator, nullptr);
	EXPECT_NE(nullptr, mutex);
	dsMutex_destroy(mutex);
	dsSystemAllocator_free(&allocator, mutex);
}

TEST(Mutex, Null)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsMutex_lock(nullptr));
	EXPECT_FALSE(dsMutex_tryLock(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsMutex_unlock(nullptr));
}

TEST(Mutex, TryLock)
{
	dsMutex* mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, mutex);
	EXPECT_TRUE(dsMutex_tryLock(mutex));
	// NOTE: On Windows, this will return true if the current thread owns the lock.
	dsMutex_tryLock(mutex);
	EXPECT_TRUE(dsMutex_unlock(mutex));
	EXPECT_TRUE(dsMutex_tryLock(mutex));
	EXPECT_TRUE(dsMutex_unlock(mutex));
	dsMutex_destroy(mutex);
}

TEST(Mutex, Contention)
{
	ThreadData threadData;
	threadData.mutex = dsMutex_create(nullptr, nullptr);
	ASSERT_NE(nullptr, threadData.mutex);
	threadData.counter = 0;
	threadData.executed = 0;

	const unsigned int threadCount = 100;
	dsThread threads[100];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);
	dsMutex_destroy(threadData.mutex);
}
