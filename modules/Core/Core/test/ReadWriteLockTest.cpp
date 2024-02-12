/*
 * Copyright 2024 Aaron Barany
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
#include <DeepSea/Core/Thread/ReadWriteLock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Atomic.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsReadWriteLock* lock;
	unsigned int counter;
	unsigned int executed;
};

dsThreadReturnType threadFunc(void* data)
{
	ThreadData* threadData = (ThreadData*)data;

	EXPECT_TRUE(dsReadWriteLock_lockWrite(threadData->lock));

	++threadData->executed;
	++threadData->counter;
	EXPECT_EQ(1U, threadData->counter);
	--threadData->counter;
	EXPECT_EQ(0U, threadData->counter);

	EXPECT_TRUE(dsReadWriteLock_unlockWrite(threadData->lock));
	return 0;
}

} // namespace

TEST(ReadWriteLock, Null)
{
	EXPECT_NULL_ERRNO(EINVAL, dsReadWriteLock_create(nullptr, nullptr, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_tryLockRead(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_lockRead(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_unlockRead(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_tryLockWrite(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_lockWrite(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteLock_unlockWrite(nullptr));
}

TEST(ReadWriteLock, Lock)
{
	dsSystemAllocator systemAllocator;
	auto allocator = reinterpret_cast<dsAllocator*>(&systemAllocator);
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsReadWriteLock* lock = dsReadWriteLock_create(allocator, nullptr, nullptr);
	ASSERT_TRUE(lock);

	EXPECT_TRUE(dsReadWriteLock_lockRead(lock));
	EXPECT_TRUE(dsReadWriteLock_lockRead(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockRead(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockRead(lock));
	EXPECT_FALSE_ERRNO(EPERM, dsReadWriteLock_unlockRead(lock));

	EXPECT_TRUE(dsReadWriteLock_lockWrite(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockWrite(lock));
	EXPECT_FALSE_ERRNO(EPERM, dsReadWriteLock_unlockWrite(lock));

	dsReadWriteLock_destroy(lock);
	EXPECT_EQ(0U, allocator->size);
}

TEST(ReadWriteLock, TryLock)
{
	dsSystemAllocator systemAllocator;
	auto allocator = reinterpret_cast<dsAllocator*>(&systemAllocator);
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsReadWriteLock* lock = dsReadWriteLock_create(allocator, nullptr, nullptr);
	ASSERT_TRUE(lock);

	EXPECT_TRUE(dsReadWriteLock_tryLockRead(lock));
	EXPECT_TRUE(dsReadWriteLock_tryLockRead(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockRead(lock));
	EXPECT_FALSE_ERRNO(EBUSY, dsReadWriteLock_tryLockWrite(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockRead(lock));
	EXPECT_FALSE_ERRNO(EPERM, dsReadWriteLock_unlockRead(lock));

	EXPECT_TRUE(dsReadWriteLock_tryLockWrite(lock));
	EXPECT_FALSE_ERRNO(EBUSY, dsReadWriteLock_tryLockRead(lock));
	EXPECT_FALSE_ERRNO(EBUSY, dsReadWriteLock_tryLockWrite(lock));
	EXPECT_TRUE(dsReadWriteLock_unlockWrite(lock));
	EXPECT_FALSE_ERRNO(EPERM, dsReadWriteLock_unlockWrite(lock));

	dsReadWriteLock_destroy(lock);
	EXPECT_EQ(0U, allocator->size);
}

TEST(ReadWriteLock, Contention)
{
	dsSystemAllocator systemAllocator;
	auto allocator = reinterpret_cast<dsAllocator*>(&systemAllocator);
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	ThreadData threadData;
	threadData.lock = dsReadWriteLock_create(allocator, nullptr, nullptr);
	ASSERT_TRUE(threadData.lock);
	threadData.counter = 0;
	threadData.executed = 0;

	EXPECT_TRUE(dsReadWriteLock_lockRead(threadData.lock));
	const unsigned int threadCount = 100;
	dsThread threads[100];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	dsThread_sleep(10, nullptr);
	EXPECT_EQ(0U, threadData.executed);
	EXPECT_TRUE(dsReadWriteLock_unlockRead(threadData.lock));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);
	dsReadWriteLock_destroy(threadData.lock);
	EXPECT_EQ(0U, allocator->size);
}
