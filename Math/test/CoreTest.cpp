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

#include <DeepSea/Math/Core.h>
#include <gtest/gtest.h>

TEST(Core, MinInt)
{
	EXPECT_EQ(0, dsMin(0, 1));
	EXPECT_EQ(0, dsMin(1, 0));
}

TEST(Core, MinFloat)
{
	EXPECT_EQ(-0.5f, dsMin(-0.5f, 3.2f));
	EXPECT_EQ(-0.5f, dsMin(3.2f, -0.5f));
}

TEST(Core, MaxInt)
{
	EXPECT_EQ(1, dsMax(0, 1));
	EXPECT_EQ(1, dsMax(1, 0));
}

TEST(Core, MaxFloat)
{
	EXPECT_EQ(3.2f, dsMax(-0.5f, 3.2f));
	EXPECT_EQ(3.2f, dsMax(3.2f, -0.5f));
}

TEST(Core, Pow2)
{
	EXPECT_EQ(3.2f*3.2f, dsPow2(3.2f));
}

TEST(Core, Pow3)
{
	EXPECT_EQ(3.2f*3.2f*3.2f, dsPow3(3.2f));
}

TEST(Core, Clamp)
{
	EXPECT_EQ(-0.5f, dsClamp(-1.0f, -0.5f, 3.2f));
	EXPECT_EQ(3.2f, dsClamp(4.0f, -0.5f, 3.2f));
	EXPECT_EQ(2.3f, dsClamp(2.3f, -0.5f, 3.2f));
}

TEST(Core, Lerp)
{
	EXPECT_EQ(-0.5f, dsLerp(-0.5f, 3.2f, 0.0f));
	EXPECT_EQ(3.2f, dsLerp(-0.5f, 3.2f, 1.0f));
	EXPECT_EQ(0.61f, dsLerp(-0.5f, 3.2f, 0.3f));
}

TEST(Core, DegreesRadians)
{
	EXPECT_DOUBLE_EQ(M_PI, dsDegreesToRadians(180));
	EXPECT_DOUBLE_EQ(180, dsRadiansToDegrees(M_PI));
}

TEST(Core, NextPowerOfTwo)
{
	EXPECT_EQ(1U, dsNextPowerOf2(1));
	EXPECT_EQ(8U, dsNextPowerOf2(7));
	EXPECT_EQ(8U, dsNextPowerOf2(8));
	EXPECT_EQ(16U, dsNextPowerOf2(9));
}

TEST(Core, Wrapi)
{
	EXPECT_EQ(3, dsWrapi(3, 3, 12));
	EXPECT_EQ(11, dsWrapi(11, 3, 12));
	EXPECT_EQ(3, dsWrapi(12, 3, 12));
	EXPECT_EQ(4, dsWrapi(-5, 3, 12));
	EXPECT_EQ(4, dsWrapi(-14, 3, 12));
	EXPECT_EQ(5, dsWrapi(23, 3, 12));
	EXPECT_EQ(5, dsWrapi(32, 3, 12));
}

TEST(Core, Wrapf)
{
	EXPECT_EQ(3.0f, dsWrapf(3.0f, 3.0f, 12.0f));
	EXPECT_EQ(11.0f, dsWrapf(11.0f, 3.0f, 12.0f));
	EXPECT_EQ(3.0f, dsWrapf(12.0f, 3.0f, 12.0f));
	EXPECT_EQ(4.0f, dsWrapf(-5.0f, 3.0f, 12.0f));
	EXPECT_EQ(4.0f, dsWrapf(-14.0f, 3.0f, 12.0f));
	EXPECT_EQ(5.0f, dsWrapf(23.0f, 3.0f, 12.0f));
	EXPECT_EQ(5.0f, dsWrapf(32.0f, 3.0f, 12.0f));
}

TEST(Core, Wrapd)
{
	EXPECT_EQ(3.0, dsWrapd(3.0, 3.0, 12.0));
	EXPECT_EQ(11.0, dsWrapd(11.0, 3.0, 12.0));
	EXPECT_EQ(3.0, dsWrapd(12.0, 3.0, 12.0));
	EXPECT_EQ(4.0, dsWrapd(-5.0, 3.0, 12.0));
	EXPECT_EQ(4.0, dsWrapd(-14.0, 3.0, 12.0));
	EXPECT_EQ(5.0, dsWrapd(23.0, 3.0, 12.0));
	EXPECT_EQ(5.0, dsWrapd(32.0, 3.0, 12.0));
}

TEST(Core, EpsilonEqualf)
{
	EXPECT_TRUE(dsEpsilonEqualf(0.0f, 0.0f, 1e-3f));

	EXPECT_TRUE(dsEpsilonEqualf(2.345f, 2.3456f, 1e-3f));
	EXPECT_TRUE(dsEpsilonEqualf(2.345f, 2.3448f, 1e-3f));
	EXPECT_FALSE(dsEpsilonEqualf(2.345f, 2.347f, 1e-3f));
	EXPECT_FALSE(dsEpsilonEqualf(2.345f, 2.343f, 1e-3f));

	EXPECT_TRUE(dsEpsilonEqualf(-2.345f, -2.3456f, 1e-3f));
	EXPECT_TRUE(dsEpsilonEqualf(-2.345f, -2.3448f, 1e-3f));
	EXPECT_FALSE(dsEpsilonEqualf(-2.345f, -2.347f, 1e-3f));
	EXPECT_FALSE(dsEpsilonEqualf(-2.345f, -2.343f, 1e-3f));

	EXPECT_FALSE(dsEpsilonEqualf(-2.345f, 2.345f, 1e-3f));
}

TEST(Core, EpsilonEquald)
{
	EXPECT_TRUE(dsEpsilonEquald(0.0, 0.0, 1e-3));

	EXPECT_TRUE(dsEpsilonEquald(2.345, 2.3456, 1e-3));
	EXPECT_TRUE(dsEpsilonEquald(2.345, 2.3448, 1e-3));
	EXPECT_FALSE(dsEpsilonEquald(2.345, 2.347, 1e-3));
	EXPECT_FALSE(dsEpsilonEquald(2.345, 2.343, 1e-3));

	EXPECT_TRUE(dsEpsilonEquald(-2.345, -2.3456, 1e-3));
	EXPECT_TRUE(dsEpsilonEquald(-2.345, -2.3448, 1e-3));
	EXPECT_FALSE(dsEpsilonEquald(-2.345, -2.347, 1e-3));
	EXPECT_FALSE(dsEpsilonEquald(-2.345, -2.343, 1e-3));

	EXPECT_FALSE(dsEpsilonEquald(-2.345, 2.345, 1e-3));
}
