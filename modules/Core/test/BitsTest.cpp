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

#include <DeepSea/Core/Bits.h>
#include <gtest/gtest.h>

TEST(BitsTest, Clz)
{
	EXPECT_EQ(32U, dsClz(0));
	EXPECT_EQ(31U, dsClz(1));
	EXPECT_EQ(4U, dsClz(0x0FFFFFFF));
	EXPECT_EQ(0U, dsClz(0xFFFFFFF0));
}

TEST(BitsTest, Ctz)
{
	EXPECT_EQ(32U, dsCtz(0));
	EXPECT_EQ(0U, dsCtz(1));
	EXPECT_EQ(0U, dsCtz(0x0FFFFFFF));
	EXPECT_EQ(4U, dsCtz(0xFFFFFFF0));
}

TEST(BitsTest, BitmaskIterate)
{
	uint32_t bitmask = 0x39;
	EXPECT_EQ(0U, dsBitmaskIndex(bitmask));

	bitmask = dsRemoveLastBit(bitmask);
	EXPECT_EQ(0x38U, bitmask);
	EXPECT_EQ(3U, dsBitmaskIndex(bitmask));

	bitmask = dsRemoveLastBit(bitmask);
	EXPECT_EQ(0x30U, bitmask);
	EXPECT_EQ(4U, dsBitmaskIndex(bitmask));

	bitmask = dsRemoveLastBit(bitmask);
	EXPECT_EQ(0x20U, bitmask);
	EXPECT_EQ(5U, dsBitmaskIndex(bitmask));

	bitmask = dsRemoveLastBit(bitmask);
	EXPECT_EQ(0U, bitmask);
}

TEST(BitsTest, Count)
{
	EXPECT_EQ(0U, dsCountBits(0U));
	EXPECT_EQ(1U, dsCountBits(1U));
	EXPECT_EQ(1U, dsCountBits(0x010U));
	EXPECT_EQ(17U, dsCountBits(0xAB0CD0EFU));
	EXPECT_EQ(32U, dsCountBits(0xFFFFFFFF));
}
