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
#include <DeepSea/Core/Thread/ReadWriteSpinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Atomic.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsReadWriteSpinlock lock;
	unsigned int counter;
	unsigned int executed;
};

dsThreadReturnType threadFunc(void* data)
{
	ThreadData* threadData = (ThreadData*)data;

	EXPECT_TRUE(dsReadWriteSpinlock_lockWrite(&threadData->lock));

	++threadData->executed;
	++threadData->counter;
	EXPECT_EQ(1U, threadData->counter);
	--threadData->counter;
	EXPECT_EQ(0U, threadData->counter);

	EXPECT_TRUE(dsReadWriteSpinlock_unlockWrite(&threadData->lock));
	return 0;
}

} // namespace

TEST(ReadWriteSpinlock, Null)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteSpinlock_initialize(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteSpinlock_lockRead(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteSpinlock_unlockRead(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteSpinlock_lockWrite(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsReadWriteSpinlock_unlockWrite(nullptr));
}

TEST(ReadWriteSpinlock, Lock)
{
	dsReadWriteSpinlock lock;
	EXPECT_TRUE(dsReadWriteSpinlock_initialize(&lock));

	EXPECT_TRUE(dsReadWriteSpinlock_lockRead(&lock));
	EXPECT_TRUE(dsReadWriteSpinlock_lockRead(&lock));
	EXPECT_TRUE(dsReadWriteSpinlock_unlockRead(&lock));
	EXPECT_TRUE(dsReadWriteSpinlock_unlockRead(&lock));
	EXPECT_FALSE(dsReadWriteSpinlock_unlockRead(&lock));

	EXPECT_TRUE(dsReadWriteSpinlock_initialize(&lock));
	EXPECT_TRUE(dsReadWriteSpinlock_lockWrite(&lock));;
	EXPECT_TRUE(dsReadWriteSpinlock_unlockWrite(&lock));

	dsReadWriteSpinlock_shutdown(&lock);
}

TEST(ReadWriteSpinlock, Contention)
{
	ThreadData threadData;
	EXPECT_TRUE(dsReadWriteSpinlock_initialize(&threadData.lock));
	threadData.counter = 0;
	threadData.executed = 0;

	EXPECT_TRUE(dsReadWriteSpinlock_lockRead(&threadData.lock));
	const unsigned int threadCount = 100;
	dsThread threads[100];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &threadData, 0, nullptr));

	dsThread_sleep(10, nullptr);
	EXPECT_EQ(0U, threadData.executed);
	EXPECT_TRUE(dsReadWriteSpinlock_unlockRead(&threadData.lock));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, threadData.executed);
	dsReadWriteSpinlock_shutdown(&threadData.lock);
}
