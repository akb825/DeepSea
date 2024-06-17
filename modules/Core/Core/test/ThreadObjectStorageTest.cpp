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

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Thread/ThreadObjectStorage.h>
#include <DeepSea/Core/Atomic.h>

#include <gtest/gtest.h>

class ThreadObjectStorageTest : public testing::Test
{
public:
	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&m_allocator, DS_ALLOCATOR_NO_LIMIT));
		allocator = reinterpret_cast<dsAllocator*>(&m_allocator);
	}

	void TearDown() override
	{
		ASSERT_EQ(0U, allocator->size);
	}

	dsAllocator* allocator;

private:
	dsSystemAllocator m_allocator;
};

TEST_F(ThreadObjectStorageTest, Create)
{
	EXPECT_NULL_ERRNO(EINVAL, dsThreadObjectStorage_create(nullptr, [](void*) {}));
	EXPECT_NULL_ERRNO(EINVAL, dsThreadObjectStorage_create(allocator, nullptr));

	dsThreadObjectStorage* storage = dsThreadObjectStorage_create(allocator, [](void*) {});
	ASSERT_TRUE(storage);
	dsThreadObjectStorage_destroy(storage);
}

TEST_F(ThreadObjectStorageTest, GetSet)
{
	constexpr unsigned int threadCount = 3;
	uint32_t destroyCount = 0;

	struct TestData
	{
		dsThreadObjectStorage* storage;
		uint32_t* destroyCount;
	};

	dsThreadObjectStorage* storage = dsThreadObjectStorage_create(allocator,
		[](void* dataPtr)
		{
			ASSERT_TRUE(dataPtr);
			auto data = reinterpret_cast<TestData*>(dataPtr);
			DS_ATOMIC_FETCH_ADD32(data->destroyCount, 1);
		});
	ASSERT_TRUE(storage);

	TestData dataPool[threadCount];
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
	{
		dataPool[i].storage = storage;
		dataPool[i].destroyCount = &destroyCount;
		ASSERT_TRUE(dsThread_create(threads + i,
			[](void* userData)
			{
				auto testData = reinterpret_cast<TestData*>(userData);
				EXPECT_FALSE(dsThreadObjectStorage_get(testData->storage));
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
				EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				return dsThreadReturnType(0);
			}, dataPool + i, 0, nullptr));
	}

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount, destroyCount);
	dsThreadObjectStorage_destroy(storage);
}

TEST_F(ThreadObjectStorageTest, Take)
{
	constexpr unsigned int threadCount = 3;
	uint32_t destroyCount = 0;

	struct TestData
	{
		dsThreadObjectStorage* storage;
		uint32_t* destroyCount;
	};

	dsThreadObjectStorage* storage = dsThreadObjectStorage_create(allocator,
		[](void* dataPtr)
		{
			ASSERT_TRUE(dataPtr);
			auto data = reinterpret_cast<TestData*>(dataPtr);
			DS_ATOMIC_FETCH_ADD32(data->destroyCount, 1);
		});
	ASSERT_TRUE(storage);

	TestData dataPool[threadCount];
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
	{
		dataPool[i].storage = storage;
		dataPool[i].destroyCount = &destroyCount;
		ASSERT_TRUE(dsThread_create(threads + i,
			[](void* userData)
			{
				auto testData = reinterpret_cast<TestData*>(userData);
				EXPECT_FALSE(dsThreadObjectStorage_take(testData->storage));
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
				EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				EXPECT_EQ(testData, dsThreadObjectStorage_take(testData->storage));
				EXPECT_FALSE(dsThreadObjectStorage_get(testData->storage));
				return dsThreadReturnType(0);
			}, dataPool + i, 0, nullptr));
	}

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(0U, destroyCount);
	dsThreadObjectStorage_destroy(storage);
}

TEST_F(ThreadObjectStorageTest, ReAssign)
{
	constexpr unsigned int threadCount = 3;
	uint32_t destroyCount = 0;

	struct TestData
	{
		dsThreadObjectStorage* storage;
		uint32_t* destroyCount;
		bool leaveNull;
	};

	dsThreadObjectStorage* storage = dsThreadObjectStorage_create(allocator,
		[](void* dataPtr)
		{
			ASSERT_TRUE(dataPtr);
			auto data = reinterpret_cast<TestData*>(dataPtr);
			DS_ATOMIC_FETCH_ADD32(data->destroyCount, 1);
		});
	ASSERT_TRUE(storage);

	TestData dataPool[threadCount];
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
	{
		dataPool[i].storage = storage;
		dataPool[i].destroyCount = &destroyCount;
		dataPool[i].leaveNull = i == 0;
		ASSERT_TRUE(dsThread_create(threads + i,
			[](void* userData)
			{
				auto testData = reinterpret_cast<TestData*>(userData);
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
				EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				// Should avoid deleting when setting same object.
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
				EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, nullptr));
				EXPECT_FALSE(dsThreadObjectStorage_get(testData->storage));
				if (!testData->leaveNull)
				{
					EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
					EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				}
				return dsThreadReturnType(0);
			}, dataPool + i, 0, nullptr));
	}

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	EXPECT_EQ(threadCount*2 - 1, destroyCount);
	dsThreadObjectStorage_destroy(storage);
}

TEST_F(ThreadObjectStorageTest, CleanupOnDestroy)
{
	constexpr unsigned int threadCount = 3;
	uint32_t destroyCount = 0;

	struct TestData
	{
		dsThreadObjectStorage* storage;
		uint32_t* destroyCount;
		unsigned int* state;
		dsMutex* mutex;
		dsConditionVariable* condition;
		bool leaveNull;
	};

	dsThreadObjectStorage* storage = dsThreadObjectStorage_create(allocator,
		[](void* dataPtr)
		{
			ASSERT_TRUE(dataPtr);
			auto data = reinterpret_cast<TestData*>(dataPtr);
			DS_ATOMIC_FETCH_ADD32(data->destroyCount, 1);
		});
	ASSERT_TRUE(storage);

	dsMutex* mutex = dsMutex_create(allocator, NULL);
	ASSERT_TRUE(mutex);
	dsConditionVariable* condition = dsConditionVariable_create(allocator, NULL);
	ASSERT_TRUE(condition);
	unsigned int state = 0;

	TestData dataPool[threadCount];
	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
	{
		dataPool[i].storage = storage;
		dataPool[i].destroyCount = &destroyCount;
		if (i == 0)
		{
			dataPool[i].state = &state;
			dataPool[i].mutex = mutex;
			dataPool[i].condition = condition;
		}
		else
		{
			dataPool[i].state = nullptr;
			dataPool[i].mutex = nullptr;
			dataPool[i].condition = nullptr;
		}
		dataPool[i].leaveNull = i == 1;
		ASSERT_TRUE(dsThread_create(threads + i,
			[](void* userData)
			{
				auto testData = reinterpret_cast<TestData*>(userData);
				EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, testData));
				EXPECT_EQ(testData, dsThreadObjectStorage_get(testData->storage));
				if (testData->leaveNull)
				{
					EXPECT_TRUE(dsThreadObjectStorage_set(testData->storage, nullptr));
				}

				if (testData->state)
				{
					EXPECT_TRUE(dsMutex_lock(testData->mutex));
					*testData->state = 1;
					dsConditionVariable_notifyAll(testData->condition);
					while (*testData->state != 2)
						dsConditionVariable_wait(testData->condition, testData->mutex);
					EXPECT_TRUE(dsMutex_unlock(testData->mutex));
				}
				return dsThreadReturnType(0);
			}, dataPool + i, 0, nullptr));
	}

	// Join all but the first thread.
	for (unsigned int i = 1; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, nullptr));

	// Wait until the first thread has set the value.
	EXPECT_TRUE(dsMutex_lock(mutex));
	while (state != 1)
		dsConditionVariable_wait(condition, mutex);
	EXPECT_TRUE(dsMutex_unlock(mutex));

	dsThreadObjectStorage_destroy(storage);
	EXPECT_EQ(threadCount, destroyCount);

	// Allow for the first thread to exit now that we've destroyed the storage.
	EXPECT_TRUE(dsMutex_lock(mutex));
	state = 2;
	dsConditionVariable_notifyAll(condition);
	EXPECT_TRUE(dsMutex_unlock(mutex));
	EXPECT_TRUE(dsThread_join(threads, nullptr));

	dsMutex_destroy(mutex);
	dsConditionVariable_destroy(condition);
}
