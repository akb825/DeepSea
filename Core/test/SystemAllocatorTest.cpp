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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>

TEST(SystemAllocator, Allocation)
{
	EXPECT_FALSE(dsSystemAllocator_initialize(NULL));

	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	void* ptr1 = dsAllocator_alloc(allocator, 11);
	size_t size1 = allocator->size;
	EXPECT_NE(nullptr, ptr1);
	EXPECT_LE(10U, size1);

	void* ptr2 = dsAllocator_alloc(allocator, 101);
	size_t size2 = allocator->size;
	EXPECT_NE(nullptr, ptr2);
	EXPECT_LE(112U, size2);

	void* ptr3 = dsAllocator_alloc(allocator, 1003);
	size_t size3 = allocator->size;
	EXPECT_NE(nullptr, ptr3);
	EXPECT_LE(1113U, size3);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr3));
	EXPECT_EQ(size2, allocator->size);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr1));
	EXPECT_EQ(size2 - size1, allocator->size);

	EXPECT_TRUE(dsAllocator_free(allocator, ptr2));
	EXPECT_EQ(0U, allocator->size);
}

TEST(SystemAllocator, DirectAllocation)
{
	EXPECT_FALSE(dsSystemAllocator_initialize(NULL));

	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator));
	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	EXPECT_EQ(0, allocator->size);

	void* ptr1 = dsSystemAllocator_alloc(&systemAllocator, 11);
	size_t size1 = allocator->size;
	EXPECT_NE(nullptr, ptr1);
	EXPECT_LE(10U, size1);

	void* ptr2 = dsSystemAllocator_alloc(&systemAllocator, 101);
	size_t size2 = allocator->size;
	EXPECT_NE(nullptr, ptr2);
	EXPECT_LE(112U, size2);

	void* ptr3 = dsSystemAllocator_alloc(&systemAllocator, 1003);
	size_t size3 = allocator->size;
	EXPECT_NE(nullptr, ptr3);
	EXPECT_LE(1113U, size3);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr3));
	EXPECT_EQ(size2, allocator->size);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr1));
	EXPECT_EQ(size2 - size1, allocator->size);

	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr2));
	EXPECT_EQ(0U, allocator->size);
}
