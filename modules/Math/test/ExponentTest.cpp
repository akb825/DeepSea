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
#include <DeepSea/Math/Types.h>
#include <gtest/gtest.h>
#include <cmath>

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

	EXPECT_EQ(HUGE_VALF, dsSplitPow2f(&pow2, HUGE_VALF));
	EXPECT_EQ(-HUGE_VALF, dsSplitPow2f(&pow2, -HUGE_VALF));
	EXPECT_TRUE(std::isnan(dsSplitPow2f(&pow2, std::nanf(""))));
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

	// Shouldn't be able to escape infinity, and should also handle NaN.
	EXPECT_EQ(HUGE_VALF, dsMulPow2f(HUGE_VALF, -1));
	EXPECT_TRUE(std::isnan(dsMulPow2f(std::nanf(""), -1)));
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

	EXPECT_EQ(HUGE_VAL, dsSplitPow2d(&pow2, HUGE_VAL));
	EXPECT_EQ(-HUGE_VAL, dsSplitPow2d(&pow2, -HUGE_VAL));
	EXPECT_TRUE(std::isnan(dsSplitPow2d(&pow2, std::nan(""))));
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

	// Shouldn't be able to escape infinity, and should also handle NaN.
	EXPECT_EQ(HUGE_VAL, dsMulPow2d(HUGE_VAL, -1));
	EXPECT_TRUE(std::isnan(dsMulPow2d(std::nan(""), -1)));
}

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)

static void ExponentFloatTest_SplitPow2SIMD()
{
	dsVector4i pow2;
	dsVector4f result;
	result.simd = dsSplitPow2SIMD4f(&pow2.simd, dsSIMD4f_set4(0.0f, -0.0f, 0.5f, -0.5f));
	EXPECT_EQ(0.0f, result.x);
	EXPECT_EQ(-0.0f, result.y);
	EXPECT_EQ(0.5f, result.z);
	EXPECT_EQ(-0.5f, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);
	EXPECT_EQ(0, pow2.z);
	EXPECT_EQ(0, pow2.w);

	result.simd = dsSplitPow2SIMD4f(&pow2.simd, dsSIMD4f_set4(1.0f, -1.0f, 0.25f, -0.25f));
	EXPECT_EQ(0.5f, result.x);
	EXPECT_EQ(-0.5f, result.y);
	EXPECT_EQ(0.5f, result.z);
	EXPECT_EQ(-0.5f, result.w);
	EXPECT_EQ(1, pow2.x);
	EXPECT_EQ(1, pow2.y);
	EXPECT_EQ(-1, pow2.z);
	EXPECT_EQ(-1, pow2.w);

	result.simd = dsSplitPow2SIMD4f(
		&pow2.simd, dsSIMD4f_set4(0.75f, 2.0f, 1.1754943508e-38f, 3.4028234664e38f));
	EXPECT_EQ(0.75f, result.x);
	EXPECT_EQ(0.5f, result.y);
	EXPECT_EQ(0.5f, result.z);
	EXPECT_EQ(0.999999940f, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(2, pow2.y);
	EXPECT_EQ(-125, pow2.z);
	EXPECT_EQ(128, pow2.w);

	result.simd = dsSplitPow2SIMD4f(&pow2.simd,
		dsSIMD4f_set4(1.4012984643e-45f, -1.4012984643e-45f, 1.1754942107e-38f, HUGE_VALF));
	EXPECT_EQ(0.5f, result.x);
	EXPECT_EQ(-0.5f, result.y);
	EXPECT_EQ(0.999999881f, result.z);
	EXPECT_EQ(HUGE_VALF, result.w);
	EXPECT_EQ(-148, pow2.x);
	EXPECT_EQ(-148, pow2.y);
	EXPECT_EQ(-126, pow2.z);
	EXPECT_EQ(0, pow2.w);

	result.simd = dsSplitPow2SIMD4f(&pow2.simd,
		dsSIMD4f_set4(-HUGE_VALF, std::nanf(""), std::nanf("1"), 1.0f));
	EXPECT_EQ(-HUGE_VAL, result.x);
	EXPECT_TRUE(std::isnan(result.y));
	EXPECT_TRUE(std::isnan(result.z));
	EXPECT_EQ(0.5f, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);
	EXPECT_EQ(0, pow2.z);
	EXPECT_EQ(1, pow2.w);
}

static void ExponentFloatTest_MulPow2SIMD()
{
	dsVector4f result;
	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 0.0f), dsSIMD4fb_set4(0, 10, -10, 10000));
	EXPECT_EQ(0.0f, result.x);
	EXPECT_EQ(0.0f, result.y);
	EXPECT_EQ(0.0f, result.z);
	EXPECT_EQ(0.0f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(0.0f, -0.0f, -0.0f, -0.0f), dsSIMD4fb_set4(-10000, 0, 10, -10));
	EXPECT_EQ(0.0f, result.x);
	EXPECT_EQ(-0.0f, result.y);
	EXPECT_EQ(-0.0f, result.z);
	EXPECT_EQ(-0.0f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(-0.0f, -0.0f, 0.5f, -0.5f), dsSIMD4fb_set4(10000, -10000, 0, 0));
	EXPECT_EQ(-0.0f, result.x);
	EXPECT_EQ(-0.0f, result.y);
	EXPECT_EQ(0.5f, result.z);
	EXPECT_EQ(-0.5f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(0.75f, 0.5f, -0.5f, 0.5f), dsSIMD4fb_set4(0, 1, -1, -1));
	EXPECT_EQ(0.75f, result.x);
	EXPECT_EQ(1.0f, result.y);
	EXPECT_EQ(-0.25f, result.z);
	EXPECT_EQ(0.25f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(0.5f, 0.5f, 1.0f, 0.999999940f), dsSIMD4fb_set4(2, -125, -126, 128));
	EXPECT_EQ(2.0f, result.x);
	EXPECT_EQ(1.1754943508e-38f, result.y);
	EXPECT_EQ(1.1754943508e-38f, result.z);
	EXPECT_EQ(3.4028234664e38f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(0.5f, 1.0f, -0.5f, 0.999999881f), dsSIMD4fb_set4(-148, -149, -148, -126));
	EXPECT_EQ(1.4012984643e-45f, result.x);
	EXPECT_EQ(1.4012984643e-45f, result.y);
	EXPECT_EQ(-1.4012984643e-45f, result.z);
	EXPECT_EQ(1.1754942107e-38f, result.w);

	result.simd = dsMulPow2SIMD4f(
		dsSIMD4f_set4(1.0f, 1.0f, 1.4012984643e-45f, -0.5f), dsSIMD4fb_set4(-150, -1000, -1, -149));
	EXPECT_EQ(0.0f, result.x);
	EXPECT_EQ(0.0f, result.y);
	EXPECT_EQ(0.0f, result.z);
	EXPECT_EQ(-0.0f, result.w);

	result.simd = dsMulPow2SIMD4f(dsSIMD4f_set4(1.0f, 3.4028234664e38f, -HUGE_VALF, std::nanf("")),
		dsSIMD4fb_set4(1000, 1, -1, -1));
	EXPECT_EQ(HUGE_VALF, result.x);
	EXPECT_EQ(HUGE_VALF, result.y);
	EXPECT_EQ(-HUGE_VALF, result.z);
	EXPECT_TRUE(std::isnan(result.w));
}

DS_SIMD_END()

TEST(ExponentFloatTest, SplitPow2SIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentFloatTest_SplitPow2SIMD();
}

TEST(ExponentFloatTest, MulPow2SIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentFloatTest_MulPow2SIMD();
}

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)

static void ExponentDoubleTest_SplitPow2SIMD2()
{
	dsVector2l pow2;
	dsVector2d result;
	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(0.0, -0.0));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(-0.0, result.y);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(0.5, -0.5));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(1.0, -1.0));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(1, pow2.x);
	EXPECT_EQ(1, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(0.25, -0.25));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(-1, pow2.x);
	EXPECT_EQ(-1, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(0.75, 2.0));
	EXPECT_EQ(0.75, result.x);
	EXPECT_EQ(0.5, result.y);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(2, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd,
		dsSIMD2d_set2(2.2250738585072014e-308, 1.7976931348623157e308));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(0.99999999999999989, result.y);
	EXPECT_EQ(-1021, pow2.x);
	EXPECT_EQ(1024, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd,
		dsSIMD2d_set2(4.9406564584124654e-324, -4.9406564584124654e-324));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(-1073, pow2.x);
	EXPECT_EQ(-1073, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(2.2250738585072009e-308, HUGE_VAL));
	EXPECT_EQ(0.99999999999999978, result.x);
	EXPECT_EQ(HUGE_VAL, result.y);
	EXPECT_EQ(-1022, pow2.x);
	EXPECT_EQ(0, pow2.y);

	result.simd = dsSplitPow2SIMD2d(&pow2.simd, dsSIMD2d_set2(-HUGE_VAL, std::nan("")));
	EXPECT_EQ(-HUGE_VAL, result.x);
	EXPECT_TRUE(std::isnan(result.y));
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);
}

static void ExponentDoubleTest_MulPow2SIMD2()
{
	dsVector2d result;
	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.0, 0.0), dsSIMD2db_set2(0, 10));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.0, 0.0), dsSIMD2db_set2(-10, 10000));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.0, -0.0), dsSIMD2db_set2(-10000, 0));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(-0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.0, -0.0), dsSIMD2db_set2(10, -10));
	EXPECT_EQ(-0.0, result.x);
	EXPECT_EQ(-0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.0, -0.0), dsSIMD2db_set2(10000, -10000));
	EXPECT_EQ(-0.0, result.x);
	EXPECT_EQ(-0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.5, -0.5), dsSIMD2db_set2(0, 0));
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.75, 0.5), dsSIMD2db_set2(0, 1));
	EXPECT_EQ(0.75, result.x);
	EXPECT_EQ(1.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.5, 0.5), dsSIMD2db_set2(1, -1));
	EXPECT_EQ(-1.0, result.x);
	EXPECT_EQ(0.25, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.5, 0.5), dsSIMD2db_set2(-1, 2));
	EXPECT_EQ(-0.25, result.x);
	EXPECT_EQ(2.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.5, 1.0), dsSIMD2db_set2(-1021, -1022));
	EXPECT_EQ(2.2250738585072014e-308, result.x);
	EXPECT_EQ(2.2250738585072014e-308, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(0.99999999999999989, 0.5),
		dsSIMD2db_set2(1024, -1073));
	EXPECT_EQ(1.7976931348623157e308, result.x);
	EXPECT_EQ(4.9406564584124654e-324, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.5, 0.99999999999999978),
		dsSIMD2db_set2(-1073, -1022));
	EXPECT_EQ(-4.9406564584124654e-324, result.x);
	EXPECT_EQ(2.2250738585072009e-308, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(4.9406564584124654e-324, 0.5),
		dsSIMD2db_set2(52, 1025));
	EXPECT_EQ(2.2250738585072014e-308, result.x);
	EXPECT_EQ(HUGE_VAL, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-0.5, 0.5), dsSIMD2db_set2(1025, -1074));
	EXPECT_EQ(-HUGE_VAL, result.x);
	EXPECT_EQ(0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(1.0, 1.0), dsSIMD2db_set2(-1075, -100000));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(4.9406564584124654e-324, -0.5),
		dsSIMD2db_set2(-1, -1074));
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(-0.0, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(1.0, 1.7976931348623157e308),
		dsSIMD2db_set2(100000, 1));
	EXPECT_EQ(HUGE_VAL, result.x);
	EXPECT_EQ(HUGE_VAL, result.y);

	result.simd = dsMulPow2SIMD2d(dsSIMD2d_set2(-HUGE_VAL, std::nan("")), dsSIMD2db_set2(-1, -1));
	EXPECT_EQ(-HUGE_VAL, result.x);
	EXPECT_TRUE(std::isnan(result.y));
}

DS_SIMD_END()

TEST(ExponentDoubleTest, SplitPow2SIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentDoubleTest_SplitPow2SIMD2();
}

TEST(ExponentDoubleTest, MulPow2SIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentDoubleTest_MulPow2SIMD2();
}

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT)

static void ExponentDoubleTest_SplitPow2SIMD4()
{
	DS_ALIGN(32) dsVector4l pow2;
	DS_ALIGN(32) dsVector4d result;
	dsSIMD4db simdPow2;
	dsSIMD4d simdResult =  dsSplitPow2SIMD4d(&simdPow2, dsSIMD4d_set4(0.0, -0.0, 0.5, -0.5));
	dsSIMD4db_store(&pow2, simdPow2);
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(-0.0, result.y);
	EXPECT_EQ(0.5, result.z);
	EXPECT_EQ(-0.5, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);
	EXPECT_EQ(0, pow2.z);
	EXPECT_EQ(0, pow2.w);

	simdResult = dsSplitPow2SIMD4d(&simdPow2, dsSIMD4d_set4(1.0, -1.0, 0.25, -0.25));
	dsSIMD4db_store(&pow2, simdPow2);
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(0.5, result.z);
	EXPECT_EQ(-0.5, result.w);
	EXPECT_EQ(1, pow2.x);
	EXPECT_EQ(1, pow2.y);
	EXPECT_EQ(-1, pow2.z);
	EXPECT_EQ(-1, pow2.w);

	simdResult = dsSplitPow2SIMD4d(&simdPow2,
		dsSIMD4d_set4(0.75, 2.0, 2.2250738585072014e-308, 1.7976931348623157e308));
	dsSIMD4db_store(&pow2, simdPow2);
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.75, result.x);
	EXPECT_EQ(0.5, result.y);
	EXPECT_EQ(0.5, result.z);
	EXPECT_EQ(0.99999999999999989, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(2, pow2.y);
	EXPECT_EQ(-1021, pow2.z);
	EXPECT_EQ(1024, pow2.w);

	simdResult = dsSplitPow2SIMD4d(&simdPow2, dsSIMD4d_set4(
		4.9406564584124654e-324, -4.9406564584124654e-324, 2.2250738585072009e-308, HUGE_VAL));
	dsSIMD4db_store(&pow2, simdPow2);
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.5, result.x);
	EXPECT_EQ(-0.5, result.y);
	EXPECT_EQ(0.99999999999999978, result.z);
	EXPECT_EQ(HUGE_VAL, result.w);
	EXPECT_EQ(-1073, pow2.x);
	EXPECT_EQ(-1073, pow2.y);
	EXPECT_EQ(-1022, pow2.z);
	EXPECT_EQ(0, pow2.w);

	simdResult = dsSplitPow2SIMD4d(&simdPow2,
		dsSIMD4d_set4(-HUGE_VAL, std::nan(""), std::nan("1"), 1.0));
	dsSIMD4db_store(&pow2, simdPow2);
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(-HUGE_VAL, result.x);
	EXPECT_TRUE(std::isnan(result.y));
	EXPECT_TRUE(std::isnan(result.z));
	EXPECT_EQ(0.5, result.w);
	EXPECT_EQ(0, pow2.x);
	EXPECT_EQ(0, pow2.y);
	EXPECT_EQ(0, pow2.z);
	EXPECT_EQ(1, pow2.w);
}

static void ExponentDoubleTest_MulPow2SIMD4()
{
	DS_ALIGN(32) dsVector4d result;
	dsSIMD4d simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(0.0, 0.0, 0.0, 0.0), dsSIMD4db_set4(0, 10, -10, 10000));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(0.0, result.y);
	EXPECT_EQ(0.0, result.z);
	EXPECT_EQ(0.0, result.w);

	simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(0.0, -0.0, -0.0, -0.0), dsSIMD4db_set4(-10000, 0, 10, -10));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(-0.0, result.y);
	EXPECT_EQ(-0.0, result.z);
	EXPECT_EQ(-0.0, result.w);

	simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(-0.0, -0.0, 0.5, -0.5), dsSIMD4db_set4(10000, -10000, 0, 0));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(-0.0, result.x);
	EXPECT_EQ(-0.0, result.y);
	EXPECT_EQ(0.5, result.z);
	EXPECT_EQ(-0.5, result.w);

	simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(0.75, 0.5, -0.5, 0.5), dsSIMD4db_set4(0, 1, 1, -1));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.75, result.x);
	EXPECT_EQ(1.0, result.y);
	EXPECT_EQ(-1.0, result.z);
	EXPECT_EQ(0.25, result.w);

	simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(-0.5, 0.5, 0.5, 1.0), dsSIMD4db_set4(-1, 2, -1021, -1022));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(-0.25, result.x);
	EXPECT_EQ(2.0, result.y);
	EXPECT_EQ(2.2250738585072014e-308, result.z);
	EXPECT_EQ(2.2250738585072014e-308, result.w);

	simdResult = dsMulPow2SIMD4d(dsSIMD4d_set4(0.99999999999999989, 0.5, -0.5, 0.99999999999999978),
		dsSIMD4db_set4(1024, -1073, -1073, -1022));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(1.7976931348623157e308, result.x);
	EXPECT_EQ(4.9406564584124654e-324, result.y);
	EXPECT_EQ(-4.9406564584124654e-324, result.z);
	EXPECT_EQ(2.2250738585072009e-308, result.w);

	simdResult = dsMulPow2SIMD4d(dsSIMD4d_set4(4.9406564584124654e-324, 0.5, -0.5, 0.5),
		dsSIMD4db_set4(52, 1025, 1025, -1074));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(2.2250738585072014e-308, result.x);
	EXPECT_EQ(HUGE_VAL, result.y);
	EXPECT_EQ(-HUGE_VAL, result.z);
	EXPECT_EQ(0.0, result.w);

	simdResult = dsMulPow2SIMD4d(dsSIMD4d_set4(1.0, 1.0, 4.9406564584124654e-324, -0.5),
		dsSIMD4db_set4(-1075, -100000, -1, -1074));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(0.0, result.x);
	EXPECT_EQ(0.0, result.y);
	EXPECT_EQ(0.0, result.z);
	EXPECT_EQ(-0.0, result.w);

	simdResult = dsMulPow2SIMD4d(
		dsSIMD4d_set4(1.0, 1.7976931348623157e308, -HUGE_VAL, std::nan("")),
		dsSIMD4db_set4(100000, 1, -1, -1));
	dsSIMD4d_store(&result, simdResult);
	EXPECT_EQ(HUGE_VAL, result.x);
	EXPECT_EQ(HUGE_VAL, result.y);
	EXPECT_EQ(-HUGE_VAL, result.z);
	EXPECT_TRUE(std::isnan(result.w));
}

DS_SIMD_END()

TEST(ExponentDoubleTest, SplitPow2SIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentDoubleTest_SplitPow2SIMD4();
}

TEST(ExponentDoubleTest, MulPow2SIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		ExponentDoubleTest_MulPow2SIMD4();
}

#endif // DS_HAS_SIMD
