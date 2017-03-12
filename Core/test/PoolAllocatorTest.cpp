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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>

namespace
{

dsThreadReturnType threadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_alloc((dsAllocator*)data, 14);
	EXPECT_NE(nullptr, ptr);
	EXPECT_TRUE(dsAllocator_free((dsAllocator*)data, ptr));
	return 0;
}

dsThreadReturnType pauseThreadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_alloc((dsAllocator*)data, 14);
	EXPECT_NE(nullptr, ptr);
	dsThread_sleep(1, nullptr);
	EXPECT_TRUE(dsAllocator_free((dsAllocator*)data, ptr));
	return 0;
}

} // namespace

TEST(PoolAllocator, Initialize)
{
	const unsigned int chunkSize = 24;
	const unsigned int chunkCount = 4;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*chunkCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	EXPECT_EQ(bufferSize, dsPoolAllocator_bufferSize(chunkSize, chunkCount));

	dsPoolAllocator allocator;
	EXPECT_FALSE_ERRNO(EINVAL, dsPoolAllocator_initialize(nullptr, chunkSize, chunkCount, buffer,
		bufferSize));
	EXPECT_FALSE_ERRNO(EINVAL, dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, buffer,
		bufferSize - 1));
	EXPECT_FALSE_ERRNO(EINVAL, dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, NULL,
		bufferSize));
	EXPECT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, buffer, bufferSize));

	EXPECT_EQ(buffer, allocator.buffer);
	EXPECT_EQ(bufferSize, allocator.bufferSize);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), allocator.chunkSize);
	EXPECT_EQ(chunkCount, allocator.chunkCount);
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(chunkCount, allocator.freeCount);
	EXPECT_EQ(0U, allocator.initializedCount);

	dsPoolAllocator_destroy(&allocator);
}

TEST(PoolAllocator, AllocateFree)
{
	const unsigned int chunkSize = 24;
	const unsigned int chunkCount = 4;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*chunkCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	EXPECT_EQ(bufferSize, dsPoolAllocator_bufferSize(chunkSize, chunkCount));

	dsPoolAllocator allocator;
	EXPECT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, buffer, bufferSize));

	EXPECT_NULL_ERRNO(EINVAL, dsAllocator_alloc((dsAllocator*)&allocator, 0));
	EXPECT_NULL_ERRNO(EINVAL, dsAllocator_alloc((dsAllocator*)&allocator, 33));
	EXPECT_NULL_ERRNO(EINVAL, dsPoolAllocator_alloc(&allocator, chunkSize, 32));

	void* ptr1 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr1);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(1U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr2 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize - 1);
	EXPECT_EQ(buffer + DS_ALIGNED_SIZE(chunkSize), ptr2);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(2U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr3 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 2*DS_ALIGNED_SIZE(chunkSize), ptr3);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(3U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr1));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr3));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr4 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 2*DS_ALIGNED_SIZE(chunkSize), ptr4);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr5 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr5);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(3U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr6 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 3*DS_ALIGNED_SIZE(chunkSize), ptr6);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ((size_t)-1, allocator.head);
	EXPECT_EQ(0U, allocator.freeCount);
	EXPECT_EQ(4U, allocator.initializedCount);
	EXPECT_EQ(4*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_NULL_ERRNO(ENOMEM, dsAllocator_alloc((dsAllocator*)&allocator, chunkSize));

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr4));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(4U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr6));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(3U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(4U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr8 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 3*DS_ALIGNED_SIZE(chunkSize), ptr8);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(4U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	dsPoolAllocator_destroy(&allocator);
}

TEST(PoolAllocator, SingleChunk)
{
	const unsigned int chunkSize = 24;
	const unsigned int chunkCount = 1;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*chunkCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	EXPECT_EQ(bufferSize, dsPoolAllocator_bufferSize(chunkSize, chunkCount));

	dsPoolAllocator allocator;
	EXPECT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, buffer, bufferSize));

	void* ptr = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ((size_t)-1, allocator.head);
	EXPECT_EQ(0U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_FALSE_ERRNO(EINVAL, dsPoolAllocator_reset(nullptr));

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);

	ptr = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ((size_t)-1, allocator.head);
	EXPECT_EQ(0U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);
}

TEST(PoolAllocator, Reset)
{
	const unsigned int chunkSize = 24;
	const unsigned int chunkCount = 4;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*chunkCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	EXPECT_EQ(bufferSize, dsPoolAllocator_bufferSize(chunkSize, chunkCount));

	dsPoolAllocator allocator;
	EXPECT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, chunkCount, buffer, bufferSize));

	EXPECT_NULL_ERRNO(EINVAL, dsAllocator_alloc((dsAllocator*)&allocator, 0));
	EXPECT_NULL_ERRNO(EINVAL, dsAllocator_alloc((dsAllocator*)&allocator, 33));

	void* ptr1 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr1);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(1U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr2 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize - 1);
	EXPECT_EQ(buffer + DS_ALIGNED_SIZE(chunkSize), ptr2);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(2U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	void* ptr3 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 2*DS_ALIGNED_SIZE(chunkSize), ptr3);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(3U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr1));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr3));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_FALSE_ERRNO(EINVAL, dsPoolAllocator_reset(nullptr));
	EXPECT_TRUE(dsPoolAllocator_reset(&allocator));
	
	ptr1 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer, ptr1);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(1U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(1U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	ptr2 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize - 1);
	EXPECT_EQ(buffer + DS_ALIGNED_SIZE(chunkSize), ptr2);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(2U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	ptr3 = dsAllocator_alloc((dsAllocator*)&allocator, chunkSize);
	EXPECT_EQ(buffer + 2*DS_ALIGNED_SIZE(chunkSize), ptr3);
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(3U, allocator.head);
	EXPECT_EQ(1U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(3*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr1));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, allocator.head);
	EXPECT_EQ(2U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(2*DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, ptr3));
	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(2U, allocator.head);
	EXPECT_EQ(3U, allocator.freeCount);
	EXPECT_EQ(3U, allocator.initializedCount);
	EXPECT_EQ(DS_ALIGNED_SIZE(chunkSize), ((dsAllocator*)&allocator)->size);

	dsPoolAllocator_destroy(&allocator);
}

TEST(PoolAllocator, ThreadAlloc)
{
	const unsigned int threadCount = 100;
	const unsigned int chunkSize = 24;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*threadCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsPoolAllocator allocator;
	ASSERT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, threadCount, buffer, bufferSize));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	dsPoolAllocator_destroy(&allocator);
}

TEST(PoolAllocator, ThreadAllocWithPause)
{
	const unsigned int threadCount = 100;
	const unsigned int chunkSize = 24;
	const unsigned int bufferSize = DS_ALIGNED_SIZE(chunkSize)*threadCount;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsPoolAllocator allocator;
	ASSERT_TRUE(dsPoolAllocator_initialize(&allocator, chunkSize, threadCount, buffer, bufferSize));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &pauseThreadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_TRUE(dsPoolAllocator_validate(&allocator));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	dsPoolAllocator_destroy(&allocator);
}
