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

#include <DeepSea/Core/Config.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <stdint.h>

enum Bits
{
	Bit1 = 0x1,
	Bit2 = 0x2,
};

DS_ENUM_BITMASK_OPERATORS(Bits);

TEST(ConfigTest, IsBufferRangeValid)
{
	uint32_t offset, rangeSize, bufferSize;

	offset = 0;
	rangeSize = 8;
	bufferSize = 10;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 2;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 3;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = UINT_MAX - 10;
	rangeSize = 10;
	bufferSize = UINT_MAX;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset += 2;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 0;
	rangeSize = 0;
	bufferSize = 0;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 2;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));
}

TEST(ConfigTest, EncodeVersion)
{
	uint32_t version = DS_ENCODE_VERSION(1, 2, 3);
	uint32_t major, minor, patch;
	DS_DECODE_VERSION(major, minor, patch, version);
	EXPECT_EQ(1U, major);
	EXPECT_EQ(2U, minor);
	EXPECT_EQ(3U, patch);

	version = DS_ENCODE_VERSION(0xFFFFFE00, 0xFFFFFE00, 0xFFFFF800);
	DS_DECODE_VERSION(major, minor, patch, version);
	EXPECT_EQ(0x200U, major);
	EXPECT_EQ(0x200U, minor);
	EXPECT_EQ(0x800U, patch);

	EXPECT_LT(DS_ENCODE_VERSION(1, 2, 3), DS_ENCODE_VERSION(1, 2, 4));
	EXPECT_LT(DS_ENCODE_VERSION(1, 1, 3), DS_ENCODE_VERSION(1, 2, 4));
	EXPECT_LT(DS_ENCODE_VERSION(0, 3, 3), DS_ENCODE_VERSION(1, 2, 4));
}

TEST(ConfigTest, LibraryVersion)
{
	EXPECT_NE(0U, DS_VERSION);
}

TEST(ConfigTest, EnumBitmaskOperators)
{
	Bits value = Bit1 | Bit2;
	EXPECT_EQ(0x3U, (unsigned int)value);

	value = value & Bit2;
	EXPECT_EQ(0x2U, (unsigned int)value);

	value = value ^ (Bit1 | Bit2);
	EXPECT_EQ(0x1U, (unsigned int)value);

	value = ~value;
	EXPECT_EQ(0xFFFFFFFEU, (unsigned int)value);

	value = Bit1;
	value |= Bit2;
	EXPECT_EQ(0x3U, (unsigned int)value);

	value &= Bit2;
	EXPECT_EQ(0x2U, (unsigned int)value);

	value ^= (Bit1 | Bit2);
	EXPECT_EQ(0x1U, (unsigned int)value);
}
