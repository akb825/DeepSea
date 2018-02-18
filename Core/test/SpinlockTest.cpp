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
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Atomic.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsSpinlock spinlock;
	unsigned int counter;
	unsigned int executed;
};

dsThreadReturnType threadFunc(void* data)
{
	ThreadData* threadData = (ThreadData*)data;
	dsThread_sleep(1, nullptr);

	EXPECT_TRUE(dsSpinlock_lock(&threadData->spinlock));

	++threadData->counter;
	EXPECT_EQ(1, threadData->counter);
	--threadData->counter;
	EXPECT_EQ(0, threadData->counter);
	++threadData->executed;

	EXPECT_TRUE(dsSpinlock_unlock(&threadData->spinlock));
	return 0;
}

} // namespace

TEST(Spinlock, Null)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsSpinlock_initialize(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsSpinlock_lock(nullptr));
	EXPECT_FALSE(dsSpinlock_tryLock(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsSpinlock_unlock(nullptr));
}

TEST(Spinlock, TryLock)
{
	dsSpinlock spinlock;
	EXPECT_TRUE(dsSpinlock_initialize(&spinlock));
	EXPECT_TRUE(dsSpinlock_tryLock(&spinlock));
	EXPECT_FALSE_ERRNO(EBUSY, dsSpinlock_tryLock(&spinlock));
	EXPECT_TRUE(dsSpinlock_unlock(&spinlock));
	EXPECT_TRUE(dsSpinlock_tryLock(&spinlock));
	EXPECT_TRUE(dsSpinlock_unlock(&spinlock));
	dsSpinlock_shutdown(&spinlock);
}

TEST(Spinlock, Contention)
{
	ThreadData threadData;
	EXPECT_TRUE(dsSpinlock_initialize(&threadData.spinlock));
	threadData.counter = 0;
	threadData.executed = 0;

	const unsigned int threadCount = 100;
	dsThread threads[100];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);
	dsSpinlock_shutdown(&threadData.spinlock);
}
