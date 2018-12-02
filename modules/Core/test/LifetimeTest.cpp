/*
 * Copyright 2018 Aaron Barany
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

#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsLifetime* lifetime;
	dsMutex* mutex;
	dsConditionVariable* condition;
	bool acquired;
};

static dsThreadReturnType threadFunc(void* userData)
{
	auto threadData = reinterpret_cast<ThreadData*>(userData);
	EXPECT_TRUE(dsMutex_lock(threadData->mutex));
	EXPECT_TRUE(dsLifetime_acquire(threadData->lifetime));
	threadData->acquired = true;
	EXPECT_TRUE(dsConditionVariable_notifyAll(threadData->condition));
	EXPECT_TRUE(dsMutex_unlock(threadData->mutex));

	dsThread_sleep(1, NULL);

	dsLifetime_release(threadData->lifetime);
	dsLifetime_freeRef(threadData->lifetime);

	return 0;
}

} // namespace

TEST(Lifetime, AcquireRelease)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	EXPECT_FALSE(dsLifetime_create(NULL, &allocator));
	EXPECT_FALSE(dsLifetime_create((dsAllocator*)&allocator, NULL));

	dsLifetime* lifetime = dsLifetime_create((dsAllocator*)&allocator, &allocator);
	ASSERT_TRUE(lifetime);

	EXPECT_EQ(&allocator, dsLifetime_getObject(lifetime));

	EXPECT_EQ(&allocator, dsLifetime_acquire(lifetime));
	dsLifetime_release(lifetime);

	dsLifetime_addRef(lifetime);
	dsLifetime_destroy(lifetime);

	EXPECT_FALSE(dsLifetime_getObject(lifetime));
	EXPECT_FALSE(dsLifetime_acquire(lifetime));
	dsLifetime_freeRef(lifetime);

	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

TEST(Lifetime, AcquireReleaseThread)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsLifetime* lifetime = dsLifetime_create((dsAllocator*)&allocator, &allocator);
	ASSERT_TRUE(lifetime);
	dsMutex* mutex = dsMutex_create((dsAllocator*)&allocator, NULL);
	ASSERT_TRUE(mutex);
	dsConditionVariable* condition = dsConditionVariable_create((dsAllocator*)&allocator, NULL);
	ASSERT_TRUE(condition);

	dsLifetime_addRef(lifetime);
	ThreadData threadData = {lifetime, mutex, condition, false};
	dsThread thread;
	EXPECT_TRUE(dsThread_create(&thread, &threadFunc, &threadData, 0, NULL));

	EXPECT_TRUE(dsMutex_lock(mutex));
	while (!threadData.acquired)
		dsConditionVariable_wait(condition, mutex);
	EXPECT_TRUE(dsMutex_unlock(mutex));

	dsLifetime_destroy(lifetime);
	EXPECT_TRUE(dsThread_join(&thread, NULL));

	dsMutex_destroy(mutex);
	dsConditionVariable_destroy(condition);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}
