/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>

TEST(ResizeableArray, Add)
{
	uint32_t* buffer = NULL;
	uint32_t count = 0;
	uint32_t capacity = 0;

	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	EXPECT_FALSE(dsResizeableArray_add(NULL, (void**)&buffer, &count, &capacity, sizeof(*buffer),
		1));
	EXPECT_FALSE(dsResizeableArray_add((dsAllocator*)&allocator, NULL, &count, &capacity,
		sizeof(*buffer), 1));
	EXPECT_FALSE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, NULL, &capacity,
		sizeof(*buffer), 1));
	EXPECT_FALSE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, NULL,
		sizeof(*buffer), 1));
	EXPECT_FALSE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		0, 1));

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 0));
	EXPECT_FALSE(buffer);
	EXPECT_EQ(0U, count);
	EXPECT_EQ(0U, capacity);

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 0));

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 1));
	const uint32_t minSize = 16;
	EXPECT_TRUE(buffer);
	EXPECT_EQ(1U, count);
	EXPECT_EQ(minSize, capacity);

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 50));
	EXPECT_TRUE(buffer);
	EXPECT_EQ(51U, count);
	EXPECT_EQ(51U, capacity);

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 1));
	EXPECT_TRUE(buffer);
	EXPECT_EQ(52U, count);
	EXPECT_EQ(102U, capacity);

	EXPECT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 4));
	EXPECT_TRUE(buffer);
	EXPECT_EQ(56U, count);
	EXPECT_EQ(102U, capacity);

	count = 103;
	EXPECT_FALSE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 4));
	EXPECT_EQ(103U, count);
	EXPECT_EQ(102U, capacity);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, buffer));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

TEST(ResizeableArray, Remove)
{
	uint32_t* buffer = NULL;
	uint32_t count = 0;
	uint32_t capacity = 0;

	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
	ASSERT_TRUE(dsResizeableArray_add((dsAllocator*)&allocator, (void**)&buffer, &count, &capacity,
		sizeof(*buffer), 100));

	ASSERT_TRUE(buffer);
	ASSERT_EQ(100U, count);
	for (uint32_t i = 0; i < count; ++i)
		buffer[i] = i;

	EXPECT_FALSE(dsResizeableArray_remove(buffer, &count, sizeof(*buffer), 95, 6));
	EXPECT_TRUE(dsResizeableArray_remove(buffer, &count, sizeof(*buffer), 95, 5));
	ASSERT_EQ(95U, count);
	for (uint32_t i = 0; i < count; ++i)
		EXPECT_EQ(i, buffer[i]);

	EXPECT_TRUE(dsResizeableArray_remove(buffer, &count, sizeof(*buffer), 30, 6));
	ASSERT_EQ(89U, count);
	for (uint32_t i = 0; i < 30; ++i)
		EXPECT_EQ(i, buffer[i]);
	for (uint32_t i = 30; i < count; ++i)
		EXPECT_EQ(i + 6, buffer[i]);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, buffer));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}
