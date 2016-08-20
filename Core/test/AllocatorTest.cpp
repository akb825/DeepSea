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
#include <stdlib.h>

TEST(Allocator, Empty)
{
	dsAllocator allocator = {};
	EXPECT_EQ(nullptr, dsAllocator_alloc(&allocator, 100));
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
