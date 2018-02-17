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
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>

namespace
{

dsThreadReturnType allocThreadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_alloc((dsAllocator*)data, 14);
	EXPECT_NE(nullptr, ptr);
	EXPECT_TRUE(dsAllocator_free((dsAllocator*)data, ptr));
	return 0;
}

dsThreadReturnType pauseAllocThreadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_alloc((dsAllocator*)data, 14);
	EXPECT_NE(nullptr, ptr);
	dsThread_sleep(1, nullptr);
	EXPECT_TRUE(dsAllocator_free((dsAllocator*)data, ptr));
	return 0;
}

dsThreadReturnType reallocThreadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_realloc((dsAllocator*)data, nullptr, 14);
	EXPECT_NE(nullptr, ptr);
	ptr = dsAllocator_realloc((dsAllocator*)data, ptr, 23);
	EXPECT_NE(nullptr, ptr);
	ptr = dsAllocator_realloc((dsAllocator*)data, ptr, 7);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(nullptr, dsAllocator_realloc((dsAllocator*)data, ptr, 0));
	return 0;
}

dsThreadReturnType pauseReallocThreadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	void* ptr = dsAllocator_realloc((dsAllocator*)data, nullptr, 14);
	EXPECT_NE(nullptr, ptr);
	dsThread_sleep(1, nullptr);
	ptr = dsAllocator_realloc((dsAllocator*)data, ptr, 23);
	EXPECT_NE(nullptr, ptr);
	dsThread_sleep(1, nullptr);
	ptr = dsAllocator_realloc((dsAllocator*)data, ptr, 7);
	EXPECT_NE(nullptr, ptr);
	dsThread_sleep(1, nullptr);
	EXPECT_EQ(nullptr, dsAllocator_realloc((dsAllocator*)data, ptr, 0));
	return 0;
}

} // namespace

TEST(SystemAllocator, Allocation)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsSystemAllocator_initialize(NULL, DS_ALLOCATOR_NO_LIMIT));

	dsSystemAllocator systemAllocator;
	ASSERT_FALSE(dsSystemAllocator_initialize(&systemAllocator, 0));
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	void* ptr1 = dsAllocator_alloc(allocator, 11);
	size_t size1 = allocator->size;
	EXPECT_NE(nullptr, ptr1);
	EXPECT_EQ(0, (uintptr_t)ptr1 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(10U, size1);
	EXPECT_EQ(1U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	void* ptr2 = dsAllocator_alloc(allocator, 101);
	size_t size2 = allocator->size;
	EXPECT_NE(nullptr, ptr2);
	EXPECT_EQ(0, (uintptr_t)ptr2 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(112U, size2);
	EXPECT_EQ(2U, allocator->totalAllocations);
	EXPECT_EQ(2U, allocator->currentAllocations);

	void* ptr3 = dsAllocator_alloc(allocator, 1003);
	size_t size3 = allocator->size;
	EXPECT_NE(nullptr, ptr3);
	EXPECT_EQ(0, (uintptr_t)ptr3 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(1113U, size3);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(3U, allocator->currentAllocations);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr3));
	EXPECT_EQ(size2, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(2U, allocator->currentAllocations);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr1));
	EXPECT_EQ(size2 - size1, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr2));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);
}

TEST(SystemAllocator, DirectAllocation)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	void* ptr1 = dsSystemAllocator_alloc(&systemAllocator, 11, DS_ALLOC_ALIGNMENT);
	size_t size1 = allocator->size;
	EXPECT_NE(nullptr, ptr1);
	EXPECT_EQ(0, (uintptr_t)ptr1 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(10U, size1);
	EXPECT_EQ(1U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	void* ptr2 = dsSystemAllocator_alloc(&systemAllocator, 101, DS_ALLOC_ALIGNMENT);
	size_t size2 = allocator->size;
	EXPECT_NE(nullptr, ptr2);
	EXPECT_EQ(0, (uintptr_t)ptr2 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(112U, size2);
	EXPECT_EQ(2U, allocator->totalAllocations);
	EXPECT_EQ(2U, allocator->currentAllocations);

	void* ptr3 = dsSystemAllocator_alloc(&systemAllocator, 1003, DS_ALLOC_ALIGNMENT);
	size_t size3 = allocator->size;
	EXPECT_NE(nullptr, ptr3);
	EXPECT_EQ(0, (uintptr_t)ptr3 % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(1113U, size3);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(3U, allocator->currentAllocations);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr3));
	EXPECT_EQ(size2, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(2U, allocator->currentAllocations);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr1));
	EXPECT_EQ(size2 - size1, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr2));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);
}

TEST(SystemAllocator, Reallocation)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	void* ptr = dsAllocator_realloc(allocator, nullptr, 100);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(100U, allocator->size);
	EXPECT_EQ(1U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	ptr = dsAllocator_realloc(allocator, ptr, 200);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(200U, allocator->size);
	EXPECT_EQ(2U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	ptr = dsAllocator_realloc(allocator, ptr, 50);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(50U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	EXPECT_EQ(nullptr, dsAllocator_realloc(allocator, ptr, 0));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);

	EXPECT_EQ(nullptr, dsAllocator_realloc(allocator, nullptr, 0));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);
}

TEST(SystemAllocator, AlignedReallocation)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	static unsigned int alignment = 64;
	void* ptr = dsSystemAllocator_realloc(&systemAllocator, nullptr, 100, alignment);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % alignment);
	EXPECT_LE(100U, allocator->size);
	EXPECT_EQ(1U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	ptr = dsSystemAllocator_realloc(&systemAllocator, ptr, 200, alignment);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % alignment);
	EXPECT_LE(200U, allocator->size);
	EXPECT_EQ(2U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	ptr = dsSystemAllocator_realloc(&systemAllocator, ptr, 50, alignment);
	EXPECT_NE(nullptr, ptr);
	EXPECT_EQ(0, (uintptr_t)ptr % DS_ALLOC_ALIGNMENT);
	EXPECT_LE(50U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(1U, allocator->currentAllocations);

	EXPECT_EQ(nullptr, dsSystemAllocator_realloc(&systemAllocator, ptr, 0, alignment));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);

	EXPECT_EQ(nullptr, dsSystemAllocator_realloc(&systemAllocator, nullptr, 0, alignment));
	EXPECT_EQ(0U, allocator->size);
	EXPECT_EQ(3U, allocator->totalAllocations);
	EXPECT_EQ(0U, allocator->currentAllocations);
}

TEST(SystemAllocator, Limit)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, 1024));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;

	void* ptr1 = dsAllocator_alloc(allocator, 512);
	EXPECT_NE(nullptr, ptr1);
	void* ptr2 = dsAllocator_alloc(allocator, 768);
	EXPECT_EQ(nullptr, ptr2);
	void* ptr3 = dsAllocator_alloc(allocator, 256);
	EXPECT_NE(nullptr, ptr3);
	void* ptr4 = dsAllocator_realloc(allocator, nullptr, 128);
	EXPECT_NE(nullptr, ptr4);
	EXPECT_EQ(nullptr, dsAllocator_realloc(allocator, ptr4, 768));
	ptr4 = dsAllocator_realloc(allocator, ptr4, 196);
	EXPECT_NE(nullptr, ptr4);

	dsAllocator_free(allocator, ptr1);
	dsAllocator_free(allocator, ptr2);
	dsAllocator_free(allocator, ptr3);
	dsAllocator_free(allocator, ptr4);
}

TEST(SystemAllocator, ThreadAlloc)
{
	const unsigned int threadCount = 100;
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &allocThreadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(threadCount, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(SystemAllocator, ThreadAllocWithPause)
{
	const unsigned int threadCount = 100;
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &pauseAllocThreadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(threadCount, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(SystemAllocator, ThreadRealloc)
{
	const unsigned int threadCount = 100;
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &reallocThreadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(threadCount*3, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(SystemAllocator, ThreadReallocWithPause)
{
	const unsigned int threadCount = 100;
	dsSystemAllocator allocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &pauseReallocThreadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(threadCount*3, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->currentAllocations);
}
