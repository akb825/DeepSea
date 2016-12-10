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

TEST(PackingTest, HalfFloat)
{
	EXPECT_EQ(0.0f, dsUnpackHalfFloat(dsPackHalfFloat(0.0f)));
	EXPECT_EQ(0.5f, dsUnpackHalfFloat(dsPackHalfFloat(0.5f)));
	EXPECT_EQ(-0.5f, dsUnpackHalfFloat(dsPackHalfFloat(-0.5f)));
	EXPECT_NEAR(1.5e3f, dsUnpackHalfFloat(dsPackHalfFloat(1.5e3f)), 1e-5f);
	EXPECT_NEAR(-1.5e-3f, dsUnpackHalfFloat(dsPackHalfFloat(-1.5e-3f)), 1e-5f);
}

TEST(PackingTest, Int32)
{
	EXPECT_EQ(0, dsPackInt32(0.0f));
	EXPECT_EQ((int32_t)0x80000001, dsPackInt32(-1.0f));
	EXPECT_EQ((int32_t)0x80000001, dsPackInt32(-2.0f));
	EXPECT_EQ((int32_t)0x7FFFFFFF, dsPackInt32(1.0f));
	EXPECT_EQ((int32_t)0x7FFFFFFF, dsPackInt32(2.0f));

	EXPECT_EQ(-0.3f, dsUnpackInt32(dsPackInt32(-0.3f)));
	EXPECT_EQ(0.3f, dsUnpackInt32(dsPackInt32(0.3f)));
}

TEST(PackingTest, UInt32)
{
	EXPECT_EQ(0U, dsPackUInt32(0.0f));
	EXPECT_EQ(0U, dsPackUInt32(-1.0f));
	EXPECT_EQ(0xFFFFFFFF, dsPackUInt32(1.0f));
	EXPECT_EQ(0xFFFFFFFF, dsPackUInt32(2.0f));

	EXPECT_EQ(0.3f, dsUnpackUInt32(dsPackUInt32(0.3f)));
	EXPECT_EQ(0.7f, dsUnpackUInt32(dsPackUInt32(0.7f)));
}

TEST(PackingTest, Int16)
{
	EXPECT_EQ(0, dsPackInt16(0.0f));
	EXPECT_EQ((int16_t)0x8001, dsPackInt16(-1.0f));
	EXPECT_EQ((int16_t)0x8001, dsPackInt16(-2.0f));
	EXPECT_EQ((int16_t)0x7FFF, dsPackInt16(1.0f));
	EXPECT_EQ((int16_t)0x7FFF, dsPackInt16(2.0f));

	EXPECT_NEAR(-0.3f, dsUnpackInt16(dsPackInt16(-0.3f)), 1e-5f);
	EXPECT_NEAR(0.3f, dsUnpackInt16(dsPackInt16(0.3f)), 1e-5f);
}

TEST(PackingTest, UInt16)
{
	EXPECT_EQ(0U, dsPackUInt16(0.0f));
	EXPECT_EQ(0U, dsPackUInt16(-1.0f));
	EXPECT_EQ(0xFFFF, dsPackUInt16(1.0f));
	EXPECT_EQ(0xFFFF, dsPackUInt16(2.0f));

	EXPECT_NEAR(0.3f, dsUnpackUInt16(dsPackUInt16(0.3f)), 1e-5f);
	EXPECT_NEAR(0.7f, dsUnpackUInt16(dsPackUInt16(0.7f)), 1e-5f);
}

TEST(PackingTest, Int8)
{
	EXPECT_EQ(0, dsPackInt8(0.0f));
	EXPECT_EQ((int8_t)0x81, dsPackInt8(-1.0f));
	EXPECT_EQ((int8_t)0x81, dsPackInt8(-2.0f));
	EXPECT_EQ((int8_t)0x7F, dsPackInt8(1.0f));
	EXPECT_EQ((int8_t)0x7F, dsPackInt8(2.0f));

	EXPECT_NEAR(-0.3f, dsUnpackInt8(dsPackInt8(-0.3f)), 1e-2f);
	EXPECT_NEAR(0.3f, dsUnpackInt8(dsPackInt8(0.3f)), 1e-2f);
}

TEST(PackingTest, UInt8)
{
	EXPECT_EQ(0U, dsPackUInt8(0.0f));
	EXPECT_EQ(0U, dsPackUInt8(-1.0f));
	EXPECT_EQ(0xFF, dsPackUInt8(1.0f));
	EXPECT_EQ(0xFF, dsPackUInt8(2.0f));

	EXPECT_NEAR(0.3f, dsUnpackUInt8(dsPackUInt8(0.3f)), 1e-2f);
	EXPECT_NEAR(0.7f, dsUnpackUInt8(dsPackUInt8(0.7f)), 1e-2f);
}

TEST(PackingTest, IntX4Y4)
{
	dsVector2f value = {{-1.0f, 1.0f}};
	EXPECT_EQ(0x97, dsPackIntX4Y4(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	dsVector2f result;
	dsUnpackIntX4Y4(&result, dsPackIntX4Y4(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
}

TEST(PackingTest, UIntX4Y4)
{
	dsVector2f value = {{0.0f, 1.0f}};
	EXPECT_EQ(0x0F, dsPackUIntX4Y4(&value));

	value.x = 0.3f;
	value.y = 0.7f;
	dsVector2f result;
	dsUnpackUIntX4Y4(&result, dsPackUIntX4Y4(&value));
	EXPECT_NEAR(0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.7f, result.y, 1e-1f);
}

TEST(PackingTest, IntY4X4)
{
	dsVector2f value = {{-1.0f, 1.0f}};
	EXPECT_EQ(0x79, dsPackIntY4X4(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	dsVector2f result;
	dsUnpackIntY4X4(&result, dsPackIntY4X4(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
}

TEST(PackingTest, UIntY4X4)
{
	dsVector2f value = {{0.0f, 1.0f}};
	EXPECT_EQ(0xF0, dsPackUIntY4X4(&value));

	value.x = 0.3f;
	value.y = 0.7f;
	dsVector2f result;
	dsUnpackUIntY4X4(&result, dsPackUIntY4X4(&value));
	EXPECT_NEAR(0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.7f, result.y, 1e-1f);
}

TEST(PackingTest, IntX4Y4Z4W4)
{
	dsVector4f value = {{-1.0f, 1.0f, -1.0f, 1.0f}};
	EXPECT_EQ(0x9797, dsPackIntX4Y4Z4W4(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = 0.7f;
	dsVector4f result;
	dsUnpackIntX4Y4Z4W4(&result, dsPackIntX4Y4Z4W4(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
	EXPECT_NEAR(-0.7f, result.z, 1e-1f);
	EXPECT_NEAR(0.7f, result.w, 1e-1f);
}

TEST(PackingTest, UIntX4Y4Z4W4)
{
	dsVector4f value = {{0.0f, 1.0f, 0.0f, 1.0f}};
	EXPECT_EQ(0x0F0F, dsPackUIntX4Y4Z4W4(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 1.0f;
	dsVector4f result;
	dsUnpackIntX4Y4Z4W4(&result, dsPackIntX4Y4Z4W4(&value));
	EXPECT_NEAR(0.0f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
	EXPECT_NEAR(0.7f, result.z, 1e-1f);
	EXPECT_NEAR(1.0f, result.w, 1e-1f);
}

TEST(PackingTest, IntW4Z4Y4X4)
{
	dsVector4f value = {{-1.0f, 1.0f, -1.0f, 1.0f}};
	EXPECT_EQ(0x7979, dsPackIntW4Z4Y4X4(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = 0.7f;
	dsVector4f result;
	dsUnpackIntW4Z4Y4X4(&result, dsPackIntW4Z4Y4X4(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
	EXPECT_NEAR(-0.7f, result.z, 1e-1f);
	EXPECT_NEAR(0.7f, result.w, 1e-1f);
}

TEST(PackingTest, UIntW4Z4Y4X4)
{
	dsVector4f value = {{0.0f, 1.0f, 0.0f, 1.0f}};
	EXPECT_EQ(0xF0F0, dsPackUIntW4Z4Y4X4(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 1.0f;
	dsVector4f result;
	dsUnpackIntW4Z4Y4X4(&result, dsPackIntW4Z4Y4X4(&value));
	EXPECT_NEAR(0.0f, result.x, 1e-1f);
	EXPECT_NEAR(0.3f, result.y, 1e-1f);
	EXPECT_NEAR(0.7f, result.z, 1e-1f);
	EXPECT_NEAR(1.0f, result.w, 1e-1f);
}

TEST(PackingTest, IntX5Y6Z5)
{
	dsVector3f value = {{-1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0x8BEF, dsPackIntX5Y6Z5(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	dsVector3f result;
	dsUnpackIntX5Y6Z5(&result, dsPackIntX5Y6Z5(&value));
	EXPECT_NEAR(-0.3f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(-0.7f, result.z, 5e-2f);
}

TEST(PackingTest, UIntXX5Y6Z5)
{
	dsVector3f value = {{0.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0x07FF, dsPackUIntX5Y6Z5(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	dsVector3f result;
	dsUnpackUIntX5Y6Z5(&result, dsPackUIntX5Y6Z5(&value));
	EXPECT_NEAR(0.0f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(0.7f, result.z, 5e-2f);
}

TEST(PackingTest, IntZ5Y6X5)
{
	dsVector3f value = {{-1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0x7BF1, dsPackIntZ5Y6X5(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	dsVector3f result;
	dsUnpackIntZ5Y6X5(&result, dsPackIntZ5Y6X5(&value));
	EXPECT_NEAR(-0.3f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(-0.7f, result.z, 5e-2f);
}

TEST(PackingTest, UIntZ5Y6X5)
{
	dsVector3f value = {{0.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0xFFE0, dsPackUIntZ5Y6X5(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	dsVector3f result;
	dsUnpackUIntZ5Y6X5(&result, dsPackUIntZ5Y6X5(&value));
	EXPECT_NEAR(0.0f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(0.7f, result.z, 5e-2f);
}

TEST(PackingTest, IntX5Y5Z5W1)
{
	dsVector4f value = {{-1.0f, 1.0f, 1.0f, 0.0f}};
	EXPECT_EQ(0x8BDE, dsPackIntX5Y5Z5W1(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = 0.7f;
	dsVector4f result;
	dsUnpackIntX5Y5Z5W1(&result, dsPackIntX5Y5Z5W1(&value));
	EXPECT_NEAR(-0.3f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(-0.7f, result.z, 5e-2f);
	EXPECT_EQ(1.0f, result.w);
}

TEST(PackingTest, UIntX5Y5Z5W1)
{
	dsVector4f value = {{0.0f, 1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0x07FF, dsPackUIntX5Y5Z5W1(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 0.3f;
	dsVector4f result;
	dsUnpackUIntX5Y5Z5W1(&result, dsPackUIntX5Y5Z5W1(&value));
	EXPECT_NEAR(0.0f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(0.7f, result.z, 5e-2f);
	EXPECT_EQ(0.0f, result.w);
}

TEST(PackingTest, IntZ5Y5X5W1)
{
	dsVector4f value = {{-1.0f, 1.0f, 1.0f, 0.0f}};
	EXPECT_EQ(0x7BE2, dsPackIntZ5Y5X5W1(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = 0.7f;
	dsVector4f result;
	dsUnpackIntZ5Y5X5W1(&result, dsPackIntZ5Y5X5W1(&value));
	EXPECT_NEAR(-0.3f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(-0.7f, result.z, 5e-2f);
	EXPECT_EQ(1.0f, result.w);
}

TEST(PackingTest, UIntXZ5Y5X5W1)
{
	dsVector4f value = {{0.0f, 1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0xFFC1, dsPackUIntZ5Y5X5W1(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 0.3f;
	dsVector4f result;
	dsUnpackUIntZ5Y5X5W1(&result, dsPackUIntZ5Y5X5W1(&value));
	EXPECT_NEAR(0.0f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(0.7f, result.z, 5e-2f);
	EXPECT_EQ(0.0f, result.w);
}

TEST(PackingTest, IntW1X5Y5Z5)
{
	dsVector4f value = {{-1.0f, 1.0f, 1.0f, 0.0f}};
	EXPECT_EQ(0x45EF, dsPackIntW1X5Y5Z5(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = 0.7f;
	dsVector4f result;
	dsUnpackIntW1X5Y5Z5(&result, dsPackIntW1X5Y5Z5(&value));
	EXPECT_NEAR(-0.3f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(-0.7f, result.z, 5e-2f);
	EXPECT_EQ(1.0f, result.w);
}

TEST(PackingTest, UIntW1X5Y5Z5)
{
	dsVector4f value = {{0.0f, 1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0x83FF, dsPackUIntW1X5Y5Z5(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 0.3f;
	dsVector4f result;
	dsUnpackUIntW1X5Y5Z5(&result, dsPackUIntW1X5Y5Z5(&value));
	EXPECT_NEAR(0.0f, result.x, 5e-2f);
	EXPECT_NEAR(0.3f, result.y, 5e-2f);
	EXPECT_NEAR(0.7f, result.z, 5e-2f);
	EXPECT_EQ(0.0f, result.w);
}

TEST(PackingTest, IntW2X10Y10Z10)
{
	dsVector4f value = {{-1.0f, 1.0f, 1.0f, 0.0f}};
	EXPECT_EQ(0x2017FDFF, dsPackIntW2X10Y10Z10(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = -0.7f;
	dsVector4f result;
	dsUnpackIntW2X10Y10Z10(&result, dsPackIntW2X10Y10Z10(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-3f);
	EXPECT_NEAR(0.3f, result.y, 1e-3f);
	EXPECT_NEAR(-0.7f, result.z, 1e-3f);
	EXPECT_EQ(-1.0f, result.w);
}

TEST(PackingTest, UIntW2X10Y10Z10)
{
	dsVector4f value = {{0.0f, 1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0xC00FFFFF, dsPackUIntW2X10Y10Z10(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 0.5f;
	dsVector4f result;
	dsUnpackUIntW2X10Y10Z10(&result, dsPackUIntW2X10Y10Z10(&value));
	EXPECT_NEAR(0.0f, result.x, 1e-3f);
	EXPECT_NEAR(0.3f, result.y, 1e-3f);
	EXPECT_NEAR(0.7f, result.z, 1e-3f);
	EXPECT_NEAR(0.66f, result.w, 1e-2f);
}

TEST(PackingTest, IntW2Z10Y10X10)
{
	dsVector4f value = {{-1.0f, 1.0f, 1.0f, 0.0f}};
	EXPECT_EQ(0x1FF7FE01, dsPackIntW2Z10Y10X10(&value));

	value.x = -0.3f;
	value.y = 0.3f;
	value.z = -0.7f;
	value.w = -0.7f;
	dsVector4f result;
	dsUnpackIntW2Z10Y10X10(&result, dsPackIntW2Z10Y10X10(&value));
	EXPECT_NEAR(-0.3f, result.x, 1e-3f);
	EXPECT_NEAR(0.3f, result.y, 1e-3f);
	EXPECT_NEAR(-0.7f, result.z, 1e-3f);
	EXPECT_EQ(-1.0f, result.w);
}

TEST(PackingTest, UIntW2Z10Y10X10)
{
	dsVector4f value = {{0.0f, 1.0f, 1.0f, 1.0f}};
	EXPECT_EQ(0xFFFFFC00, dsPackUIntW2Z10Y10X10(&value));

	value.x = 0.0f;
	value.y = 0.3f;
	value.z = 0.7f;
	value.w = 0.5f;
	dsVector4f result;
	dsUnpackUIntW2Z10Y10X10(&result, dsPackUIntW2Z10Y10X10(&value));
	EXPECT_NEAR(0.0f, result.x, 1e-3f);
	EXPECT_NEAR(0.3f, result.y, 1e-3f);
	EXPECT_NEAR(0.7f, result.z, 1e-3f);
	EXPECT_NEAR(0.66f, result.w, 1e-2f);
}
