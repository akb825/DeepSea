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
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>
#include <stdlib.h>

TEST(Allocator, AlignedSize)
{
	EXPECT_EQ(16U, DS_ALIGNED_SIZE(1U));
	EXPECT_EQ(16U, DS_ALIGNED_SIZE(16U));
	EXPECT_EQ(32U, DS_ALIGNED_SIZE(17U));
}

TEST(Allocator, Empty)
{
	dsAllocator allocator = {};
	EXPECT_NULL_ERRNO(EINVAL, dsAllocator_alloc(&allocator, 100));
}

TEST(Allocator, NoFree)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	allocator->freeFunc = NULL;

	void* ptr = dsAllocator_alloc(allocator, 100);
	ASSERT_NE(nullptr, ptr);
	EXPECT_FALSE(dsAllocator_free(allocator, ptr));
	EXPECT_TRUE(dsSystemAllocator_free(&systemAllocator, ptr));
}

TEST(Allocator, ReallocWithFallbackUsingRealloc)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsAllocator* allocator = (dsAllocator*)&systemAllocator;

	int* data = (int*)dsAllocator_reallocWithFallback(allocator, nullptr, 0, 10*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 10; ++i)
		data[i] = i;

	data = (int*)dsAllocator_reallocWithFallback(allocator, data, 10*sizeof(int), 20*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 10; ++i)
		EXPECT_EQ(i, data[i]);

	data = (int*)dsAllocator_reallocWithFallback(allocator, data, 20*sizeof(int), 5*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 5; ++i)
		EXPECT_EQ(i, data[i]);

	EXPECT_EQ(nullptr, dsAllocator_reallocWithFallback(allocator, data, 5*sizeof(int), 0));
	EXPECT_EQ(0U, allocator->size);
}

TEST(Allocator, ReallocWithFallbackUsingFallback)
{
	dsSystemAllocator systemAllocator;
	ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsAllocator* allocator = (dsAllocator*)&systemAllocator;
	allocator->reallocFunc = nullptr;

	int* data = (int*)dsAllocator_reallocWithFallback(allocator, nullptr, 100, 10*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 10; ++i)
		data[i] = i;

	data = (int*)dsAllocator_reallocWithFallback(allocator, data, 10*sizeof(int), 20*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 10; ++i)
		EXPECT_EQ(i, data[i]);

	data = (int*)dsAllocator_reallocWithFallback(allocator, data, 20*sizeof(int), 5*sizeof(int));
	ASSERT_NE(nullptr, data);
	for (int i = 0; i < 5; ++i)
		EXPECT_EQ(i, data[i]);

	EXPECT_EQ(nullptr, dsAllocator_reallocWithFallback(allocator, data, 5*sizeof(int), 0));
	EXPECT_EQ(0U, allocator->size);
}
