/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/Core/Memory/Memory.h>
#include <gtest/gtest.h>

TEST(MemoryTest, AlignedSize)
{
	EXPECT_EQ(32U, DS_ALIGNED_SIZE(1U, 32U));
	EXPECT_EQ(32U, DS_ALIGNED_SIZE(32U, 32U));
	EXPECT_EQ(64U, DS_ALIGNED_SIZE(33U, 32U));
}

TEST(MemoryTest, RealignedSize)
{
#if DS_ALLOC_ALIGNMENT == 16
	EXPECT_EQ(16U, DS_REALIGNED_SIZE(16U, 16U));
	EXPECT_EQ(32U, DS_REALIGNED_SIZE(17U, 16U));
	EXPECT_EQ(48U, DS_REALIGNED_SIZE(33U, 16U));

	EXPECT_EQ(48U, DS_REALIGNED_SIZE(16U, 32U));
	EXPECT_EQ(48U, DS_REALIGNED_SIZE(17U, 32U));
	EXPECT_EQ(80U, DS_REALIGNED_SIZE(33U, 32U));

	EXPECT_EQ(112U, DS_REALIGNED_SIZE(1U, 64U));
#elif DS_ALLOC_ALIGNMENT == 32
	EXPECT_EQ(32U, DS_REALIGNED_SIZE(32U, 32U));
	EXPECT_EQ(64U, DS_REALIGNED_SIZE(33U, 32U));
	EXPECT_EQ(96U, DS_REALIGNED_SIZE(65U, 32U));

	EXPECT_EQ(96U, DS_REALIGNED_SIZE(32U, 64U));
	EXPECT_EQ(96U, DS_REALIGNED_SIZE(33U, 64U));
	EXPECT_EQ(160U, DS_REALIGNED_SIZE(65U, 64U));

	EXPECT_EQ(224U, DS_REALIGNED_SIZE(1U, 128U));
#else
#error Need to populate test cases for current alignment.
#endif
}

TEST(MemoryTest, CanAddSizes)
{
	size_t halfSize = SIZE_MAX/2;
	EXPECT_TRUE(DS_CAN_ADD_SIZES(halfSize, halfSize));
	EXPECT_FALSE(DS_CAN_ADD_SIZES(halfSize + 10, halfSize));
	EXPECT_FALSE(DS_CAN_ADD_SIZES(halfSize, halfSize + 10));
	EXPECT_FALSE(DS_CAN_ADD_SIZES(SIZE_MAX, halfSize));
	EXPECT_FALSE(DS_CAN_ADD_SIZES(halfSize, SIZE_MAX));
	EXPECT_FALSE(DS_CAN_ADD_SIZES(SIZE_MAX, SIZE_MAX));
}

TEST(MemoryTest, ArraySizeValid)
{
	size_t maxIntCount = SIZE_MAX/sizeof(int);
	EXPECT_TRUE(DS_ARRAY_SIZE_VALID(sizeof(int), 0));
	EXPECT_TRUE(DS_ARRAY_SIZE_VALID(sizeof(int), 10));
	EXPECT_TRUE(DS_ARRAY_SIZE_VALID(sizeof(int), maxIntCount));
	EXPECT_FALSE(DS_ARRAY_SIZE_VALID(sizeof(int), maxIntCount + 1));
	EXPECT_FALSE(DS_ARRAY_SIZE_VALID(sizeof(int), SIZE_MAX));
}

TEST(MemoryTest, AddAlignedSize)
{
	size_t curSize = 13;
	EXPECT_FALSE_ERRNO(EINVAL, dsAddAlignedSize(nullptr, 100, 8));
	EXPECT_FALSE_ERRNO(EINVAL, dsAddAlignedSize(&curSize, 100, 9));
	EXPECT_TRUE(dsAddAlignedSize(&curSize, 100, 8));
	EXPECT_EQ(120U, curSize);
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedSize(&curSize, SIZE_MAX - 10, 8));

	// Will overflow when aligning the new size.
	curSize = 0;
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedSize(&curSize, SIZE_MAX, 8));

	// Will overflow when aligning the current size.
	curSize = SIZE_MAX;
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedSize(&curSize, 0, 8));
}

TEST(MemoryTest, AddAlignedArraySize)
{
	size_t curSize = 13;
	EXPECT_FALSE_ERRNO(EINVAL, dsAddAlignedArraySize(nullptr, 4, 13, 8));
	EXPECT_FALSE_ERRNO(EINVAL, dsAddAlignedArraySize(&curSize, 0, 13, 8));
	EXPECT_FALSE_ERRNO(EINVAL, dsAddAlignedArraySize(&curSize, 4, 13, 9));
	EXPECT_TRUE(dsAddAlignedArraySize(&curSize, 4, 13, 8));
	EXPECT_EQ(72U, curSize);

	// Will overflow the array size.
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedArraySize(&curSize, 4, SIZE_MAX/2, 8));

	// Will overflow adding the sizes.
	curSize = SIZE_MAX/2;
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedArraySize(&curSize, 4, SIZE_MAX/4, 8));

	// Will overflow when aligning the array size.
	curSize = 0;
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedArraySize(&curSize, 1, SIZE_MAX, 8));

	// Will overflow when aligning the current size.
	curSize = SIZE_MAX;
	EXPECT_FALSE_ERRNO(ERANGE, dsAddAlignedArraySize(&curSize, 1, 0, 8));
}

TEST(MemoryTest, AccumulateAlignedSizes)
{
	size_t curSize = 13;
	dsMemorySize sizes[] =
	{
		{sizeof(int), 4},
		{sizeof(double), 5},
		{sizeof(short), 0},
		{0, 0},
		{sizeof(char), 6}
	};

	EXPECT_FALSE_ERRNO(EINVAL, dsAccumulateAlignedSizes(nullptr, sizes, DS_ARRAY_SIZE(sizes), 8));
	EXPECT_FALSE_ERRNO(
		EINVAL, dsAccumulateAlignedSizes(&curSize, nullptr, DS_ARRAY_SIZE(sizes), 8));
	EXPECT_FALSE_ERRNO(EINVAL, dsAccumulateAlignedSizes(&curSize, sizes, DS_ARRAY_SIZE(sizes), 9));
	EXPECT_TRUE(dsAccumulateAlignedSizes(&curSize, sizes, DS_ARRAY_SIZE(sizes), 8));
	EXPECT_EQ(80U, curSize);

	// Will overflow the array size.
	sizes[1].count = SIZE_MAX/2;
	EXPECT_FALSE_ERRNO(ERANGE, dsAccumulateAlignedSizes(&curSize, sizes, DS_ARRAY_SIZE(sizes), 8));

	// Will overflow adding the sizes.
	sizes[1].count = 5;
	curSize = SIZE_MAX - 20;
	EXPECT_FALSE_ERRNO(ERANGE, dsAccumulateAlignedSizes(&curSize, sizes, DS_ARRAY_SIZE(sizes), 8));

	// Will overflow when aligning the array size.
	dsMemorySize singleSize = {1, SIZE_MAX};
	curSize = 0;
	EXPECT_FALSE_ERRNO(ERANGE, dsAccumulateAlignedSizes(&curSize, &singleSize, 1, 8));

	// Will overflow when aligning the current size.
	curSize = SIZE_MAX;
	EXPECT_FALSE_ERRNO(ERANGE, dsAccumulateAlignedSizes(&curSize, nullptr, 0, 8));

	// Will consider this as a previous call to get the size having failed. Should keep errno the
	// same, assuming it was previously set.
	errno = EINVAL;
	singleSize.elementSize = 0;
	singleSize.count = 1;
	curSize = 0;
	EXPECT_FALSE(dsAccumulateAlignedSizes(&curSize, &singleSize, 1, 8));
	EXPECT_EQ(EINVAL, errno);
}
