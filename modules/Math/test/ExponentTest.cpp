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

#include <DeepSea/Math/Exponent.h>

#include <gtest/gtest.h>

TEST(ExponentFloatTest, SplitPow2)
{
	int pow2;
	EXPECT_EQ(0.0f, dsSplitPow2f(&pow2, 0.0f));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(-0.0f, dsSplitPow2f(&pow2, -0.0f));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 0.5f));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(-0.5f, dsSplitPow2f(&pow2, -0.5f));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 1.0f));
	EXPECT_EQ(1, pow2);

	EXPECT_EQ(-0.5f, dsSplitPow2f(&pow2, -1.0f));
	EXPECT_EQ(1, pow2);

	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 0.25f));
	EXPECT_EQ(-1, pow2);

	EXPECT_EQ(-0.5f, dsSplitPow2f(&pow2, -0.25f));
	EXPECT_EQ(-1, pow2);

	EXPECT_EQ(0.75f, dsSplitPow2f(&pow2, 0.75f));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 2.0f));
	EXPECT_EQ(2, pow2);

	// Smallest normal number. (2^-126)
	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 1.1754943508e-38f));
	EXPECT_EQ(-125, pow2);

	// Largest normal number. (1 - 2^-24)*2^128
	EXPECT_EQ(0.999999940f, dsSplitPow2f(&pow2, 3.4028234664e38f));
	EXPECT_EQ(128, pow2);

	// Smallest subnormal number. (2^-149)
	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, 1.4012984643e-45f));
	EXPECT_EQ(-148, pow2);

	EXPECT_EQ(-0.5f, dsSplitPow2f(&pow2, -1.4012984643e-45f));
	EXPECT_EQ(-148, pow2);

	// Largest subnormal number. (1 - 2^-23)*2^-126
	EXPECT_EQ(0.999999881f, dsSplitPow2f(&pow2, 1.1754942107e-38f));
	EXPECT_EQ(-126, pow2);

	EXPECT_EQ(0.5f, dsSplitPow2f(&pow2, HUGE_VALF));
	EXPECT_EQ(129, pow2);

	EXPECT_EQ(-0.5f, dsSplitPow2f(&pow2, -HUGE_VALF));
	EXPECT_EQ(129, pow2);
}

TEST(ExponentFloatTest, MulPow2)
{
	EXPECT_EQ(0.0f, dsMulPow2f(0.0f, 0));
	EXPECT_EQ(0.0f, dsMulPow2f(0.0f, 10));
	EXPECT_EQ(0.0f, dsMulPow2f(0.0f, -10));
	EXPECT_EQ(0.0f, dsMulPow2f(0.0f, 10000));
	EXPECT_EQ(0.0f, dsMulPow2f(0.0f, -10000));

	EXPECT_EQ(-0.0f, dsMulPow2f(-0.0f, 0));
	EXPECT_EQ(-0.0f, dsMulPow2f(-0.0f, 10));
	EXPECT_EQ(-0.0f, dsMulPow2f(-0.0f, -10));
	EXPECT_EQ(-0.0f, dsMulPow2f(-0.0f, 10000));
	EXPECT_EQ(-0.0f, dsMulPow2f(-0.0f, -10000));

	EXPECT_EQ(0.5f, dsMulPow2f(0.5f, 0));
	EXPECT_EQ(-0.5f, dsMulPow2f(-0.5f, 0));
	EXPECT_EQ(0.75f, dsMulPow2f(0.75f, 0));
	EXPECT_EQ(1.0f, dsMulPow2f(0.5f, 1));
	EXPECT_EQ(-1.0f, dsMulPow2f(-0.5f, 1));
	EXPECT_EQ(0.25f, dsMulPow2f(0.5f, -1));
	EXPECT_EQ(-0.25f, dsMulPow2f(-0.5f, -1));
	EXPECT_EQ(2.0f, dsMulPow2f(0.5f, 2));

	// Smallest normal number. (2^-126)
	EXPECT_EQ(1.1754943508e-38f, dsMulPow2f(0.5f, -125));
	EXPECT_EQ(1.1754943508e-38f, dsMulPow2f(1.0f, -126));
	// Largest normal number. (1 - 2^-24)*2^128
	EXPECT_EQ(3.4028234664e38f, dsMulPow2f(0.999999940f, 128));
	// Smallest subnormal number. (2^-149)
	EXPECT_EQ(1.4012984643e-45f, dsMulPow2f(0.5f, -148));
	EXPECT_EQ(1.4012984643e-45f, dsMulPow2f(1.0f, -149));
	EXPECT_EQ(-1.4012984643e-45f, dsMulPow2f(-0.5f, -148));
	// Largest subnormal number. (1 - 2^-23)*2^-126
	EXPECT_EQ(1.1754942107e-38f, dsMulPow2f(0.999999881f, -126));

	// Smallest subnormal to smallest normal number.
	EXPECT_EQ(1.1754943508e-38f, dsMulPow2f(1.4012984643e-45f, 23));

	// Exactly +-infinity.
	EXPECT_EQ(HUGE_VALF, dsMulPow2f(0.5, 129));
	EXPECT_EQ(-HUGE_VALF, dsMulPow2f(-0.5, 129));

	// Underflow to zero.
	EXPECT_EQ(0.0, dsMulPow2f(0.5f, -149));
	EXPECT_EQ(0.0, dsMulPow2f(1.0f, -150));
	EXPECT_EQ(0.0, dsMulPow2f(1.0f, -1000));
	EXPECT_EQ(0.0, dsMulPow2f(1.4012984643e-45f, -1));
	EXPECT_EQ(-0.0, dsMulPow2f(-0.5f, -149));

	// Overflow to infinity.
	EXPECT_EQ(HUGE_VALF, dsMulPow2f(1.0f, 1000));
	EXPECT_EQ(HUGE_VALF, dsMulPow2f(3.4028234664e38f, 1));
	EXPECT_EQ(-HUGE_VALF, dsMulPow2f(-1.0f, 1000));
}

TEST(ExponentDoubleTest, SplitPow2)
{
	int pow2;
	EXPECT_EQ(0.0, dsSplitPow2d(&pow2, 0.0));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(-0.0, dsSplitPow2d(&pow2, -0.0));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 0.5));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(-0.5, dsSplitPow2d(&pow2, -0.5));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 1.0));
	EXPECT_EQ(1, pow2);

	EXPECT_EQ(-0.5, dsSplitPow2d(&pow2, -1.0));
	EXPECT_EQ(1, pow2);

	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 0.25));
	EXPECT_EQ(-1, pow2);

	EXPECT_EQ(-0.5, dsSplitPow2d(&pow2, -0.25));
	EXPECT_EQ(-1, pow2);

	EXPECT_EQ(0.75, dsSplitPow2d(&pow2, 0.75));
	EXPECT_EQ(0, pow2);

	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 2.0));
	EXPECT_EQ(2, pow2);

	// Smallest normal number. (2^-1022)
	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 2.2250738585072014e-308));
	EXPECT_EQ(-1021, pow2);

	// Largest normal number. (1 - 2^-53)*2^1024
	EXPECT_EQ(0.99999999999999989, dsSplitPow2d(&pow2, 1.7976931348623157e308));
	EXPECT_EQ(1024, pow2);

	// Smallest subnormal number. (2^-1074)
	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, 4.9406564584124654e-324));
	EXPECT_EQ(-1073, pow2);

	EXPECT_EQ(-0.5, dsSplitPow2d(&pow2, -4.9406564584124654e-324));
	EXPECT_EQ(-1073, pow2);

	// Largest subnormal number. (1 - 2^-52)*2^-1022
	EXPECT_EQ(0.99999999999999978, dsSplitPow2d(&pow2, 2.2250738585072009e-308));
	EXPECT_EQ(-1022, pow2);

	EXPECT_EQ(0.5, dsSplitPow2d(&pow2, HUGE_VAL));
	EXPECT_EQ(1025, pow2);

	EXPECT_EQ(-0.5, dsSplitPow2d(&pow2, -HUGE_VAL));
	EXPECT_EQ(1025, pow2);
}

TEST(ExponentDoubleTest, MulPow2)
{
	EXPECT_EQ(0.0, dsMulPow2d(0.0, 0));
	EXPECT_EQ(0.0, dsMulPow2d(0.0, 10));
	EXPECT_EQ(0.0, dsMulPow2d(0.0, -10));
	EXPECT_EQ(0.0, dsMulPow2d(0.0, 10000));
	EXPECT_EQ(0.0, dsMulPow2d(0.0, -10000));

	EXPECT_EQ(-0.0, dsMulPow2d(-0.0, 0));
	EXPECT_EQ(-0.0, dsMulPow2d(-0.0, 10));
	EXPECT_EQ(-0.0, dsMulPow2d(-0.0, -10));
	EXPECT_EQ(-0.0, dsMulPow2d(-0.0, 10000));
	EXPECT_EQ(-0.0, dsMulPow2d(-0.0, -10000));

	EXPECT_EQ(0.5, dsMulPow2d(0.5, 0));
	EXPECT_EQ(-0.5, dsMulPow2d(-0.5, 0));
	EXPECT_EQ(0.75, dsMulPow2d(0.75, 0));
	EXPECT_EQ(1.0, dsMulPow2d(0.5, 1));
	EXPECT_EQ(-1.0, dsMulPow2d(-0.5, 1));
	EXPECT_EQ(0.25, dsMulPow2d(0.5, -1));
	EXPECT_EQ(-0.25, dsMulPow2d(-0.5, -1));
	EXPECT_EQ(2.0, dsMulPow2d(0.5, 2));

	// Smallest normal number. (2^-1022)
	EXPECT_EQ(2.2250738585072014e-308, dsMulPow2d(0.5, -1021));
	EXPECT_EQ(2.2250738585072014e-308, dsMulPow2d(1.0, -1022));
	// Largest normal number. (1 - 2^-53)*2^1024
	EXPECT_EQ(1.7976931348623157e308, dsMulPow2d(0.99999999999999989, 1024));
	// Smallest subnormal number. (2^-149)
	EXPECT_EQ(4.9406564584124654e-324, dsMulPow2d(0.5, -1073));
	EXPECT_EQ(4.9406564584124654e-324, dsMulPow2d(1.0, -1074));
	EXPECT_EQ(-4.9406564584124654e-324, dsMulPow2d(-0.5, -1073));
	// Largest subnormal number. (1 - 2^-52)*2^-1022
	EXPECT_EQ(2.2250738585072009e-308, dsMulPow2d(0.99999999999999978, -1022));

	// Smallest subnormal to smallest normal number.
	EXPECT_EQ(2.2250738585072014e-308, dsMulPow2d(4.9406564584124654e-324, 52));

	// Exactly +-infinity.
	EXPECT_EQ(HUGE_VAL, dsMulPow2d(0.5, 1025));
	EXPECT_EQ(-HUGE_VAL, dsMulPow2d(-0.5, 1025));

	// Underflow to zero.
	EXPECT_EQ(0.0, dsMulPow2d(0.5, -1074));
	EXPECT_EQ(0.0, dsMulPow2d(1.0, -1075));
	EXPECT_EQ(0.0, dsMulPow2d(1.0, -100000));
	EXPECT_EQ(0.0, dsMulPow2d(4.9406564584124654e-324, -1));
	EXPECT_EQ(-0.0, dsMulPow2d(-0.5, -1074));

	// Overflow to infinity.
	EXPECT_EQ(HUGE_VAL, dsMulPow2d(1.0, 100000));
	EXPECT_EQ(HUGE_VAL, dsMulPow2d(1.7976931348623157e308, 1));
	EXPECT_EQ(-HUGE_VAL, dsMulPow2d(-1.0, 100000));
}
