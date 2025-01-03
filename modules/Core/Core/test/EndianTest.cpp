/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Core/Streams/Endian.h>
#include <gtest/gtest.h>

TEST(EndianTest, Macros)
{
	const uint8_t bytes[] = {0x12, 0x34, 0x56, 0x78};
	uint32_t value = *reinterpret_cast<const uint32_t*>(bytes);
#if DS_BIG_ENDIAN
	EXPECT_EQ(0x12345678, value);
#else
	EXPECT_EQ(0x78563412, value);
#endif
}

TEST(EndianTest, ByteSwap)
{
	EXPECT_EQ(0x3412, dsEndian_swapUInt16(0x1234));
	EXPECT_EQ(static_cast<int16_t>(0x3412), dsEndian_swapInt16(static_cast<int16_t>(0x1234)));
	EXPECT_EQ(0x78563412, dsEndian_swapUInt32(0x12345678));
	EXPECT_EQ(static_cast<int32_t>(0x78563412),
		dsEndian_swapInt32(static_cast<int32_t>(0x12345678)));
	EXPECT_EQ(0xEFCDAB9078563412ULL, dsEndian_swapUInt64(0x1234567890ABCDEFULL));
	EXPECT_EQ(0xEFCDAB9078563412LL, dsEndian_swapInt64(0x1234567890ABCDEFLL));
	EXPECT_NE(1.234f, dsEndian_swapFloat(1.234f));
	EXPECT_EQ(1.234f, dsEndian_swapFloat(dsEndian_swapFloat(1.234f)));
	EXPECT_NE(1.23456789, dsEndian_swapDouble(23456789));
	EXPECT_EQ(1.23456789, dsEndian_swapDouble(dsEndian_swapDouble(1.23456789)));
}

TEST(EndianTest, ByteSwapOnBig)
{
#if DS_BIG_ENDIAN
	EXPECT_EQ(0x3412, dsEndian_swapUInt16OnBig(0x1234));
	EXPECT_EQ(static_cast<int16_t>(0x3412), dsEndian_swapInt16OnBig(static_cast<int16_t>(0x1234)));
	EXPECT_EQ(0x78563412, dsEndian_swapUInt32OnBig(0x12345678));
	EXPECT_EQ(static_cast<int32_t>(0x78563412),
		dsEndian_swapInt32OnBig(static_cast<int32_t>(0x12345678)));
	EXPECT_EQ(0xEFCDAB9078563412ULL, dsEndian_swapUInt64OnBig(0x1234567890ABCDEFULL));
	EXPECT_EQ(0xEFCDAB9078563412LL, dsEndian_swapInt64OnBig(0x1234567890ABCDEFLL));
	EXPECT_NE(1.234f, dsEndian_swapFloatOnBig(1.234f));
	EXPECT_EQ(1.234f, dsEndian_swapFloatOnBig(dsEndian_swapFloatOnBig(1.234f)));
	EXPECT_NE(1.23456789, dsEndian_swapDoubleOnBig(23456789));
	EXPECT_EQ(1.23456789, dsEndian_swapDoubleOnBig(dsEndian_swapDoubleOnBig(1.23456789)));
#else
	EXPECT_EQ(0x1234, dsEndian_swapUInt16OnBig(0x1234));
	EXPECT_EQ(static_cast<int16_t>(0x1234), dsEndian_swapInt16OnBig(static_cast<int16_t>(0x1234)));
	EXPECT_EQ(0x12345678, dsEndian_swapUInt32OnBig(0x12345678));
	EXPECT_EQ(static_cast<int32_t>(0x12345678),
		dsEndian_swapInt32OnBig(static_cast<int32_t>(0x12345678)));
	EXPECT_EQ(0x1234567890ABCDEFULL, dsEndian_swapUInt64OnBig(0x1234567890ABCDEFULL));
	EXPECT_EQ(0x1234567890ABCDEFLL, dsEndian_swapInt64OnBig(0x1234567890ABCDEFLL));
	EXPECT_EQ(1.234f, dsEndian_swapFloatOnBig(1.234f));
	EXPECT_EQ(1.23456789, dsEndian_swapDoubleOnBig(1.23456789));
#endif
}

TEST(EndianTest, ByteSwapOnLittle)
{
#if DS_LITTLE_ENDIAN
	EXPECT_EQ(0x3412, dsEndian_swapUInt16OnLittle(0x1234));
	EXPECT_EQ(static_cast<int16_t>(0x3412),
		dsEndian_swapInt16OnLittle(static_cast<int16_t>(0x1234)));
	EXPECT_EQ(0x78563412, dsEndian_swapUInt32OnLittle(0x12345678));
	EXPECT_EQ(static_cast<int32_t>(0x78563412),
		dsEndian_swapInt32OnLittle(static_cast<int32_t>(0x12345678)));
	EXPECT_EQ(0xEFCDAB9078563412ULL, dsEndian_swapUInt64OnLittle(0x1234567890ABCDEFULL));
	EXPECT_EQ(0xEFCDAB9078563412LL, dsEndian_swapInt64OnLittle(0x1234567890ABCDEFLL));
	EXPECT_NE(1.234f, dsEndian_swapFloatOnLittle(1.234f));
	EXPECT_EQ(1.234f, dsEndian_swapFloatOnLittle(dsEndian_swapFloatOnLittle(1.234f)));
	EXPECT_NE(1.23456789, dsEndian_swapDoubleOnLittle(23456789));
	EXPECT_EQ(1.23456789, dsEndian_swapDoubleOnLittle(dsEndian_swapDoubleOnLittle(1.23456789)));
#else
	EXPECT_EQ(0x1234, dsEndian_swapUInt16OnLittle(0x1234));
	EXPECT_EQ(static_cast<int16_t>(0x1234),
		dsEndian_swapInt16OnLittle(static_cast<int16_t>(0x1234)));
	EXPECT_EQ(0x12345678, dsEndian_swapUInt32OnLittle(0x12345678));
	EXPECT_EQ(static_cast<int32_t>(0x12345678),
		dsEndian_swapInt32OnLittle(static_cast<int32_t>(0x12345678)));
	EXPECT_EQ(0x1234567890ABCDEFULL, dsEndian_swapUInt64OnLittle(0x1234567890ABCDEFULL));
	EXPECT_EQ(0x1234567890ABCDEFLL, dsEndian_swapInt64OnLittle(0x1234567890ABCDEFLL));
	EXPECT_EQ(1.234f, dsEndian_swapFloatOnLittle(1.234f));
	EXPECT_EQ(1.23456789, dsEndian_swapDoubleOnLittle(23456789));
#endif
}
