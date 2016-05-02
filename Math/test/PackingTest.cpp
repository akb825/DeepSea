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

#include <DeepSea/Math/Packing.h>
#include <gtest/gtest.h>

TEST(Packing, HalfFloat)
{
	EXPECT_EQ(0.0f, dsUnpackHalfFloat(dsPackHalfFloat(0.0f)));
	EXPECT_EQ(0.5f, dsUnpackHalfFloat(dsPackHalfFloat(0.5f)));
	EXPECT_EQ(-0.5f, dsUnpackHalfFloat(dsPackHalfFloat(-0.5f)));
	EXPECT_NEAR(1.5e3f, dsUnpackHalfFloat(dsPackHalfFloat(1.5e3f)), 1e-5f);
	EXPECT_NEAR(-1.5e-3f, dsUnpackHalfFloat(dsPackHalfFloat(-1.5e-3f)), 1e-5f);
}

TEST(Packing, Int32)
{
	EXPECT_EQ(0, dsPackInt32(0.0f));
	EXPECT_EQ((int32_t)0x80000001, dsPackInt32(-1.0f));
	EXPECT_EQ((int32_t)0x80000001, dsPackInt32(-2.0f));
	EXPECT_EQ((int32_t)0x7FFFFFFF, dsPackInt32(1.0f));
	EXPECT_EQ((int32_t)0x7FFFFFFF, dsPackInt32(2.0f));

	EXPECT_EQ(-0.3f, dsUnpackInt32(dsPackInt32(-0.3f)));
	EXPECT_EQ(0.3f, dsUnpackInt32(dsPackInt32(0.3f)));
}

TEST(Packing, UInt32)
{
	EXPECT_EQ(0U, dsPackUInt32(0.0f));
	EXPECT_EQ(0U, dsPackUInt32(-1.0f));
	EXPECT_EQ(0xFFFFFFFF, dsPackUInt32(1.0f));
	EXPECT_EQ(0xFFFFFFFF, dsPackUInt32(2.0f));

	EXPECT_EQ(0.3f, dsUnpackUInt32(dsPackUInt32(0.3f)));
	EXPECT_EQ(0.7f, dsUnpackUInt32(dsPackUInt32(0.7f)));
}

TEST(Packing, Int16)
{
	EXPECT_EQ(0, dsPackInt16(0.0f));
	EXPECT_EQ((int16_t)0x8001, dsPackInt16(-1.0f));
	EXPECT_EQ((int16_t)0x8001, dsPackInt16(-2.0f));
	EXPECT_EQ((int16_t)0x7FFF, dsPackInt16(1.0f));
	EXPECT_EQ((int16_t)0x7FFF, dsPackInt16(2.0f));

	EXPECT_NEAR(-0.3f, dsUnpackInt16(dsPackInt16(-0.3f)), 1e-5f);
	EXPECT_NEAR(0.3f, dsUnpackInt16(dsPackInt16(0.3f)), 1e-5f);
}

TEST(Packing, UInt16)
{
	EXPECT_EQ(0U, dsPackUInt16(0.0f));
	EXPECT_EQ(0U, dsPackUInt16(-1.0f));
	EXPECT_EQ(0xFFFF, dsPackUInt16(1.0f));
	EXPECT_EQ(0xFFFF, dsPackUInt16(2.0f));

	EXPECT_NEAR(0.3f, dsUnpackUInt16(dsPackUInt16(0.3f)), 1e-5f);
	EXPECT_NEAR(0.7f, dsUnpackUInt16(dsPackUInt16(0.7f)), 1e-5f);
}

TEST(Packing, Int8)
{
	EXPECT_EQ(0, dsPackInt8(0.0f));
	EXPECT_EQ((int8_t)0x81, dsPackInt8(-1.0f));
	EXPECT_EQ((int8_t)0x81, dsPackInt8(-2.0f));
	EXPECT_EQ((int8_t)0x7F, dsPackInt8(1.0f));
	EXPECT_EQ((int8_t)0x7F, dsPackInt8(2.0f));

	EXPECT_NEAR(-0.3f, dsUnpackInt8(dsPackInt8(-0.3f)), 1e-2f);
	EXPECT_NEAR(0.3f, dsUnpackInt8(dsPackInt8(0.3f)), 1e-2f);
}

TEST(Packing, UInt8)
{
	EXPECT_EQ(0U, dsPackUInt8(0.0f));
	EXPECT_EQ(0U, dsPackUInt8(-1.0f));
	EXPECT_EQ(0xFF, dsPackUInt8(1.0f));
	EXPECT_EQ(0xFF, dsPackUInt8(2.0f));

	EXPECT_NEAR(0.3f, dsUnpackUInt8(dsPackUInt8(0.3f)), 1e-2f);
	EXPECT_NEAR(0.7f, dsUnpackUInt8(dsPackUInt8(0.7f)), 1e-2f);
}
