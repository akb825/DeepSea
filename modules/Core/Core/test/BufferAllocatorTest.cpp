/*
 * Copyright 2016-2023 Aaron Barany
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
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>

namespace
{

dsThreadReturnType threadFunc(void* data)
{
	dsThread_sleep(1, nullptr);
	EXPECT_NE(nullptr, dsAllocator_alloc((dsAllocator*)data, DS_ALLOC_ALIGNMENT));
	return 0;
}

} // namespace

TEST(BufferAllocator, Initialize)
{
	const unsigned int bufferSize = 100;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsBufferAllocator allocator;
	EXPECT_FALSE_ERRNO(EINVAL, dsBufferAllocator_initialize(nullptr, buffer, bufferSize));
	EXPECT_FALSE_ERRNO(EINVAL, dsBufferAllocator_initialize(&allocator, buffer, 0));
	EXPECT_FALSE_ERRNO(EINVAL, dsBufferAllocator_initialize(&allocator, buffer + 1, bufferSize));
	EXPECT_TRUE(dsBufferAllocator_initialize(&allocator, buffer, bufferSize));
	EXPECT_EQ(buffer, allocator.buffer);
	EXPECT_EQ(bufferSize, allocator.bufferSize);
}

TEST(BufferAllocator, Allocate)
{
	const unsigned int bufferSize = 100;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsBufferAllocator allocator;
	ASSERT_TRUE(dsBufferAllocator_initialize(&allocator, buffer, bufferSize));

	void* ptr1 = dsAllocator_alloc((dsAllocator*)&allocator, 10);
	EXPECT_NE(nullptr, ptr1);
	EXPECT_EQ(10U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(1U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(1U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr2 = dsAllocator_alloc((dsAllocator*)&allocator, 30);
	EXPECT_EQ((uintptr_t)ptr1 + 16, (uintptr_t)ptr2);
	EXPECT_EQ(46U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(2U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(2U, ((dsAllocator*)&allocator)->currentAllocations);

	EXPECT_NULL_ERRNO(ENOMEM, dsAllocator_alloc((dsAllocator*)&allocator, 60));

	void* ptr3 = dsAllocator_alloc((dsAllocator*)&allocator, 40);
	EXPECT_EQ((uintptr_t)ptr1 + 48, (uintptr_t)ptr3);
	EXPECT_EQ(88U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(3U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(3U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr4 = dsAllocator_alloc((dsAllocator*)&allocator, 1);
	EXPECT_EQ((uintptr_t)ptr1 + 96, (uintptr_t)ptr4);
	EXPECT_EQ(97U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->currentAllocations);

	EXPECT_NULL_ERRNO(ENOMEM, dsAllocator_alloc((dsAllocator*)&allocator, 1));
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(BufferAllocator, AlignedAllocate)
{
	const unsigned int bufferSize = 192;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsBufferAllocator allocator;
	ASSERT_TRUE(dsBufferAllocator_initialize(&allocator, buffer, bufferSize));

	void* ptr1 = dsAllocator_alloc((dsAllocator*)&allocator, 10);
	EXPECT_NE(nullptr, ptr1);
	EXPECT_EQ(10U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(1U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(1U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr2 = dsAllocator_alloc((dsAllocator*)&allocator, 30);
	EXPECT_EQ((uintptr_t)ptr1 + 16, (uintptr_t)ptr2);
	EXPECT_EQ(46U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(2U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(2U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr3 = dsAllocator_alloc((dsAllocator*)&allocator, 40);
	EXPECT_EQ((uintptr_t)ptr1 + 48, (uintptr_t)ptr3);
	EXPECT_EQ(88U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(3U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(3U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr4 = dsAllocator_alloc((dsAllocator*)&allocator, 1);
	EXPECT_EQ((uintptr_t)ptr1 + 96, (uintptr_t)ptr4);
	EXPECT_EQ(97U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(4U, ((dsAllocator*)&allocator)->currentAllocations);

	void* ptr5 = dsAllocator_alignedAlloc((dsAllocator*)&allocator, 16, 64);
	EXPECT_NE(nullptr, ptr5);
	EXPECT_EQ(0U, (uintptr_t)ptr5 % 64);
	size_t size = ((dsAllocator*)&allocator)->size;
	EXPECT_TRUE(size == 144 || size == 160 || size == 176 || size == 192);
	EXPECT_EQ(5U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(5U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(BufferAllocator, Reset)
{
	const unsigned int bufferSize = 100;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];
	dsBufferAllocator allocator = {};

	EXPECT_FALSE_ERRNO(EINVAL, dsBufferAllocator_reset(nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsBufferAllocator_reset(&allocator));

	ASSERT_TRUE(dsBufferAllocator_initialize(&allocator, buffer, bufferSize));
	EXPECT_TRUE(dsAllocator_alloc((dsAllocator*)&allocator, 10));

	EXPECT_TRUE(dsBufferAllocator_reset(&allocator));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->currentAllocations);
}

TEST(BufferAllocator, ThreadAlloc)
{
	const unsigned int threadCount = 100;
	const unsigned int bufferSize = threadCount*DS_ALLOC_ALIGNMENT;
	DS_ALIGN(DS_ALLOC_ALIGNMENT) uint8_t buffer[bufferSize];

	dsBufferAllocator allocator;
	ASSERT_TRUE(dsBufferAllocator_initialize(&allocator, buffer, bufferSize));

	dsThread threads[threadCount];
	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_create(threads + i, &threadFunc, &allocator, 0, nullptr));

	for (unsigned int i = 0; i < threadCount; ++i)
		EXPECT_TRUE(dsThread_join(threads + i, NULL));

	EXPECT_EQ(bufferSize, ((dsAllocator*)&allocator)->size);
	EXPECT_EQ(threadCount, ((dsAllocator*)&allocator)->totalAllocations);
	EXPECT_EQ(threadCount, ((dsAllocator*)&allocator)->currentAllocations);
}
