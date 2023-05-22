/*
 * Copyright 2022-2023 Aaron Barany
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

#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Types.h>
#include <gtest/gtest.h>
#include <cmath>

#if DS_HAS_SIMD

DS_SIMD_START_FLOAT4();
static void SIMDTest_Float4()
{
	constexpr float epsilon = 5e-3f;
	float padding1; // Keep next value unalied.
	DS_UNUSED(padding1);
	float cpuA[4] = {1.2f, 3.4f, 5.6f, 7.8f};
	dsVector4f cpuB = {{-9.8f, -7.6f, -5.4f, -3.2f}};
	dsVector4f cpuResult;
	float padding2; // Keep next value unalied.
	DS_UNUSED(padding2);
	float unalginedCPUResult[4];

	dsSIMD4f a = dsSIMD4f_loadUnaligned(&cpuA);
	dsSIMD4f b = dsSIMD4f_load(&cpuB);

	dsSIMD4f result = dsSIMD4f_set1(0.1f);
	dsSIMD4f_storeUnaligned(&unalginedCPUResult, result);
	EXPECT_EQ(0.1f, unalginedCPUResult[0]);
	EXPECT_EQ(0.1f, unalginedCPUResult[1]);
	EXPECT_EQ(0.1f, unalginedCPUResult[2]);
	EXPECT_EQ(0.1f, unalginedCPUResult[3]);

	result = dsSIMD4f_set4(0.1f, 0.2f, 0.3f, 0.4f);
	EXPECT_EQ(0.1f, dsSIMD4f_get(result, 0));
	EXPECT_EQ(0.2f, dsSIMD4f_get(result, 1));
	EXPECT_EQ(0.3f, dsSIMD4f_get(result, 2));
	EXPECT_EQ(0.4f, dsSIMD4f_get(result, 3));

	result = dsSIMD4f_neg(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(-cpuA[0], cpuResult.x);
	EXPECT_EQ(-cpuA[1], cpuResult.y);
	EXPECT_EQ(-cpuA[2], cpuResult.z);
	EXPECT_EQ(-cpuA[3], cpuResult.w);

	result = dsSIMD4f_neg(b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
	EXPECT_EQ(-cpuB.z, cpuResult.z);
	EXPECT_EQ(-cpuB.w, cpuResult.w);

	result = dsSIMD4f_add(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] + cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] + cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2] + cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3] + cpuB.w, cpuResult.w);

	result = dsSIMD4f_sub(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] - cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] - cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2] - cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3] - cpuB.w, cpuResult.w);

	result = dsSIMD4f_mul(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0]*cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1]*cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2]*cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3]*cpuB.w, cpuResult.w);

	result = dsSIMD4f_div(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(cpuA[0]/cpuB.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA[1]/cpuB.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA[2]/cpuB.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA[3]/cpuB.w, cpuResult.w, epsilon);

	result = dsSIMD4f_rcp(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/cpuA[0], cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/cpuA[1], cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/cpuA[2], cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/cpuA[3], cpuResult.w, epsilon);

	result = dsSIMD4f_sqrt(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[1]), cpuResult.y, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[2]), cpuResult.z, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[3]), cpuResult.w, epsilon);

	result = dsSIMD4f_rsqrt(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[1]), cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[2]), cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[3]), cpuResult.w, epsilon);

	result = dsSIMD4f_abs(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0], cpuResult.x);
	EXPECT_EQ(cpuA[1], cpuResult.y);
	EXPECT_EQ(cpuA[2], cpuResult.z);
	EXPECT_EQ(cpuA[3], cpuResult.w);

	result = dsSIMD4f_abs(b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
	EXPECT_EQ(-cpuB.z, cpuResult.z);
	EXPECT_EQ(-cpuB.w, cpuResult.w);

	dsVector4f cpuC = {{7.8f, 5.6f, -3.4f, -1.2f}};
	dsVector4f cpuD = {{-3.2f, -5.4f, 7.6f, 9.8f}};
	dsSIMD4f c = dsSIMD4f_load(&cpuC);
	dsSIMD4f d = dsSIMD4f_load(&cpuD);

	dsSIMD4f_transpose(a, b, c, d);
	dsVector4f cpuAT, cpuBT, cpuCT, cpuDT;
	dsSIMD4f_store(&cpuAT, a);
	dsSIMD4f_store(&cpuBT, b);
	dsSIMD4f_store(&cpuCT, c);
	dsSIMD4f_store(&cpuDT, d);

	EXPECT_EQ(cpuA[0], cpuAT.x);
	EXPECT_EQ(cpuB.x, cpuAT.y);
	EXPECT_EQ(cpuC.x, cpuAT.z);
	EXPECT_EQ(cpuD.x, cpuAT.w);

	EXPECT_EQ(cpuA[1], cpuBT.x);
	EXPECT_EQ(cpuB.y, cpuBT.y);
	EXPECT_EQ(cpuC.y, cpuBT.z);
	EXPECT_EQ(cpuD.y, cpuBT.w);

	EXPECT_EQ(cpuA[2], cpuCT.x);
	EXPECT_EQ(cpuB.z, cpuCT.y);
	EXPECT_EQ(cpuC.z, cpuCT.z);
	EXPECT_EQ(cpuD.z, cpuCT.w);

	EXPECT_EQ(cpuA[3], cpuDT.x);
	EXPECT_EQ(cpuB.w, cpuDT.y);
	EXPECT_EQ(cpuC.w, cpuDT.z);
	EXPECT_EQ(cpuD.w, cpuDT.w);
}
DS_SIMD_END();

TEST(SIMDTest, Float4)
{
#if DS_SIMD_ALWAYS_FLOAT4
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Float4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DS_LOG_INFO("SIMDTest", "Enabling float4 SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping float4 SIMD tests.");
		return;
	}
#endif

	SIMDTest_Float4();
}

DS_SIMD_START_DOUBLE2();
static void SIMDTest_Double2()
{
	constexpr double epsilon = 5e-3;
	float padding1; // Keep next value unalied.
	DS_UNUSED(padding1);
	double cpuA[2] = {1.2, 3.4};
	dsVector2d cpuB = {{-9.8, -7.6}};
	dsVector2d cpuResult;
	float padding2; // Keep next value unalied.
	DS_UNUSED(padding2);
	double unalginedCPUResult[2];

	dsSIMD2d a = dsSIMD2d_loadUnaligned(&cpuA);
	dsSIMD2d b = dsSIMD2d_load(&cpuB);

	dsSIMD2d result = dsSIMD2d_set1(0.1f);
	dsSIMD2d_storeUnaligned(&unalginedCPUResult, result);
	EXPECT_EQ(0.1f, unalginedCPUResult[0]);
	EXPECT_EQ(0.1f, unalginedCPUResult[1]);

	result = dsSIMD2d_set2(0.1f, 0.2f);
	EXPECT_EQ(0.1f, dsSIMD2d_get(result, 0));
	EXPECT_EQ(0.2f, dsSIMD2d_get(result, 1));

	result = dsSIMD2d_neg(a);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(-cpuA[0], cpuResult.x);
	EXPECT_EQ(-cpuA[1], cpuResult.y);

	result = dsSIMD2d_neg(b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);

	result = dsSIMD2d_add(a, b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] + cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] + cpuB.y, cpuResult.y);

	result = dsSIMD2d_sub(a, b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] - cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] - cpuB.y, cpuResult.y);

	result = dsSIMD2d_mul(a, b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0]*cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1]*cpuB.y, cpuResult.y);

	result = dsSIMD2d_div(a, b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0]/cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1]/cpuB.y, cpuResult.y);

	result = dsSIMD2d_rcp(a);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/cpuA[0], cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/cpuA[1], cpuResult.y, epsilon);

	result = dsSIMD2d_sqrt(a);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[1]), cpuResult.y, epsilon);

	result = dsSIMD2d_rsqrt(a);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[1]), cpuResult.y, epsilon);

	result = dsSIMD2d_abs(a);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0], cpuResult.x);
	EXPECT_EQ(cpuA[1], cpuResult.y);

	result = dsSIMD2d_abs(b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
}
DS_SIMD_END();

TEST(SIMDTest, Double2)
{
#if DS_SIMD_ALWAYS_DOUBLE2
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Double2);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		DS_LOG_INFO("SIMDTest", "Enabling double2 SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double2 SIMD tests.");
		return;
	}
#endif

	SIMDTest_Double2();
}

DS_SIMD_START_DOUBLE4();
static void SIMDTest_Double4()
{
	constexpr double epsilon = 5e-3;
	double padding1; // Keep next value unalied.
	DS_UNUSED(padding1);
	double cpuA[4] = {1.2, 3.4, 5.6, 7.8};
	DS_ALIGN(32) dsVector4d cpuB = {{-9.8, -7.6, -5.4, -3.2}};
	DS_ALIGN(32) dsVector4d cpuResult;
	double padding2; // Keep next value unalied.
	DS_UNUSED(padding2);
	double unalginedCPUResult[4];

	dsSIMD4d a = dsSIMD4d_loadUnaligned(&cpuA);
	dsSIMD4d b = dsSIMD4d_load(&cpuB);

	dsSIMD4d result = dsSIMD4d_set1(0.1);
	dsSIMD4d_storeUnaligned(&unalginedCPUResult, result);
	EXPECT_EQ(0.1, unalginedCPUResult[0]);
	EXPECT_EQ(0.1, unalginedCPUResult[1]);
	EXPECT_EQ(0.1, unalginedCPUResult[2]);
	EXPECT_EQ(0.1, unalginedCPUResult[3]);

	result = dsSIMD4d_set4(0.1, 0.2, 0.3, 0.4);
	EXPECT_EQ(0.1, dsSIMD4d_get(result, 0));
	EXPECT_EQ(0.2, dsSIMD4d_get(result, 1));
	EXPECT_EQ(0.3, dsSIMD4d_get(result, 2));
	EXPECT_EQ(0.4, dsSIMD4d_get(result, 3));

	result = dsSIMD4d_neg(a);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(-cpuA[0], cpuResult.x);
	EXPECT_EQ(-cpuA[1], cpuResult.y);
	EXPECT_EQ(-cpuA[2], cpuResult.z);
	EXPECT_EQ(-cpuA[3], cpuResult.w);

	result = dsSIMD4d_neg(b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
	EXPECT_EQ(-cpuB.z, cpuResult.z);
	EXPECT_EQ(-cpuB.w, cpuResult.w);

	result = dsSIMD4d_add(a, b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] + cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] + cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2] + cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3] + cpuB.w, cpuResult.w);

	result = dsSIMD4d_sub(a, b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0] - cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1] - cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2] - cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3] - cpuB.w, cpuResult.w);

	result = dsSIMD4d_mul(a, b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0]*cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA[1]*cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA[2]*cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA[3]*cpuB.w, cpuResult.w);

	result = dsSIMD4d_div(a, b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(cpuA[0]/cpuB.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA[1]/cpuB.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA[2]/cpuB.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA[3]/cpuB.w, cpuResult.w, epsilon);

	result = dsSIMD4d_rcp(a);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/cpuA[0], cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/cpuA[1], cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/cpuA[2], cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/cpuA[3], cpuResult.w, epsilon);

	result = dsSIMD4d_sqrt(a);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[1]), cpuResult.y, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[2]), cpuResult.z, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA[3]), cpuResult.w, epsilon);

	result = dsSIMD4d_rsqrt(a);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[0]), cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[1]), cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[2]), cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA[3]), cpuResult.w, epsilon);

	result = dsSIMD4d_abs(a);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(cpuA[0], cpuResult.x);
	EXPECT_EQ(cpuA[1], cpuResult.y);
	EXPECT_EQ(cpuA[2], cpuResult.z);
	EXPECT_EQ(cpuA[3], cpuResult.w);

	result = dsSIMD4d_abs(b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
	EXPECT_EQ(-cpuB.z, cpuResult.z);
	EXPECT_EQ(-cpuB.w, cpuResult.w);

	DS_ALIGN(32) dsVector4d cpuC = {{7.8, 5.6, -3.4, -1.2}};
	DS_ALIGN(32) dsVector4d cpuD = {{-3.2, -5.4, 7.6, 9.8}};
	dsSIMD4d c = dsSIMD4d_load(&cpuC);
	dsSIMD4d d = dsSIMD4d_load(&cpuD);

	dsSIMD4d_transpose(a, b, c, d);
	DS_ALIGN(32) dsVector4d cpuAT, cpuBT, cpuCT, cpuDT;
	dsSIMD4d_store(&cpuAT, a);
	dsSIMD4d_store(&cpuBT, b);
	dsSIMD4d_store(&cpuCT, c);
	dsSIMD4d_store(&cpuDT, d);

	EXPECT_EQ(cpuA[0], cpuAT.x);
	EXPECT_EQ(cpuB.x, cpuAT.y);
	EXPECT_EQ(cpuC.x, cpuAT.z);
	EXPECT_EQ(cpuD.x, cpuAT.w);

	EXPECT_EQ(cpuA[1], cpuBT.x);
	EXPECT_EQ(cpuB.y, cpuBT.y);
	EXPECT_EQ(cpuC.y, cpuBT.z);
	EXPECT_EQ(cpuD.y, cpuBT.w);

	EXPECT_EQ(cpuA[2], cpuCT.x);
	EXPECT_EQ(cpuB.z, cpuCT.y);
	EXPECT_EQ(cpuC.z, cpuCT.z);
	EXPECT_EQ(cpuD.z, cpuCT.w);

	EXPECT_EQ(cpuA[3], cpuDT.x);
	EXPECT_EQ(cpuB.w, cpuDT.y);
	EXPECT_EQ(cpuC.w, cpuDT.z);
	EXPECT_EQ(cpuD.w, cpuDT.w);
}
DS_SIMD_END();

TEST(SIMDTest, Double4)
{
#if DS_SIMD_ALWAYS_DOUBLE4
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Double4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double4)
		DS_LOG_INFO("SIMDTest", "Enabling double4 SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double4 SIMD tests.");
		return;
	}
#endif

	SIMDTest_Double4();
}

DS_SIMD_START_FLOAT4();
static void SIMDTest_CompareLogicFloat4()
{
	dsVector4f cpuA = {{1.2f, 3.4f, 5.6f, 7.8f}};
	dsVector4f cpuB = {{1.1f, 3.5f, 5.6f, -7.8f}};
	dsVector4i cpuResult;

	dsSIMD4f a = dsSIMD4f_load(&cpuA);
	dsSIMD4f b = dsSIMD4f_load(&cpuB);

	dsVector4f cpuFPResult;
	dsSIMD4f fpResult = dsSIMD4f_min(a, b);
	dsSIMD4f_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1f, cpuFPResult.x);
	EXPECT_EQ(3.4f, cpuFPResult.y);
	EXPECT_EQ(5.6f, cpuFPResult.z);
	EXPECT_EQ(-7.8f, cpuFPResult.w);

	fpResult = dsSIMD4f_max(a, b);
	dsSIMD4f_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.2f, cpuFPResult.x);
	EXPECT_EQ(3.5f, cpuFPResult.y);
	EXPECT_EQ(5.6f, cpuFPResult.z);
	EXPECT_EQ(7.8f, cpuFPResult.w);

	dsSIMD4fb ab = dsSIMD4f_cmple(a, b);
	fpResult = dsSIMD4f_select(a, b, ab);
	dsSIMD4f_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1f, cpuFPResult.x);
	EXPECT_EQ(3.4f, cpuFPResult.y);
	EXPECT_EQ(5.6f, cpuFPResult.z);
	EXPECT_EQ(-7.8f, cpuFPResult.w);

	dsSIMD4fb result = dsSIMD4f_cmpeq(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmpne(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4f_cmplt(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmple(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmpgt(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4f_cmpge(a, b);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_true();
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_false();
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4fb_not(ab);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_and(ab, dsSIMD4fb_true());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4fb_and(ab, dsSIMD4fb_false());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4fb_andnot(ab, dsSIMD4fb_true());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_or(ab, dsSIMD4fb_true());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_or(ab, dsSIMD4fb_false());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4fb_ornot(dsSIMD4fb_false(), ab);
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4fb_xor(ab, dsSIMD4fb_false());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4fb_xor(ab, dsSIMD4fb_true());
	dsSIMD4fb_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);
}
DS_SIMD_END();

TEST(SIMDTest, CompareLogicFloat4)
{
#if DS_SIMD_ALWAYS_FLOAT4
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Float4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DS_LOG_INFO("SIMDTest", "Enabling float4 compare logic SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping float4 compare logic SIMD tests.");
		return;
	}
#endif

	SIMDTest_CompareLogicFloat4();
}

DS_SIMD_START_DOUBLE2();
static void SIMDTest_CompareLogicDouble2()
{
	dsVector2d cpuA = {{1.2, 3.4}};
	dsVector2d cpuB = {{1.1, 3.5}};
	dsVector2l cpuResult;

	dsSIMD2d a = dsSIMD2d_load(&cpuA);
	dsSIMD2d b = dsSIMD2d_load(&cpuB);

	dsVector2d cpuFPResult;
	dsSIMD2d fpResult = dsSIMD2d_min(a, b);
	dsSIMD2d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1, cpuFPResult.x);
	EXPECT_EQ(3.4, cpuFPResult.y);

	fpResult = dsSIMD2d_max(a, b);
	dsSIMD2d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.2, cpuFPResult.x);
	EXPECT_EQ(3.5, cpuFPResult.y);

	dsSIMD2db ab = dsSIMD2d_cmple(a, b);
	fpResult = dsSIMD2d_select(a, b, ab);
	dsSIMD2d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1, cpuFPResult.x);
	EXPECT_EQ(3.4, cpuFPResult.y);

	dsSIMD2db result = dsSIMD2d_cmpeq(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2d_cmpne(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2d_cmplt(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2d_cmple(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2d_cmpgt(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2d_cmpge(a, b);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_true();
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2db_false();
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_not(ab);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_and(ab, dsSIMD2db_true());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2db_and(ab, dsSIMD2db_false());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_andnot(ab, dsSIMD2db_true());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_or(ab, dsSIMD2db_true());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2db_or(ab, dsSIMD2db_false());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2db_ornot(dsSIMD2db_false(), ab);
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);

	result = dsSIMD2db_xor(ab, dsSIMD2db_false());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);

	result = dsSIMD2db_xor(ab, dsSIMD2db_true());
	dsSIMD2db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
}
DS_SIMD_END();

TEST(SIMDTest, CompareLogicDouble2)
{
#if DS_SIMD_ALWAYS_DOUBLE2
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Double2);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		DS_LOG_INFO("SIMDTest", "Enabling double2 compare logic SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double2 compare logic SIMD tests.");
		return;
	}
#endif

	SIMDTest_CompareLogicDouble2();
}

DS_SIMD_START_DOUBLE4();
static void SIMDTest_CompareLogicDouble4()
{
	DS_ALIGN(32) dsVector4d cpuA = {{1.2, 3.4, 5.6, 7.8}};
	DS_ALIGN(32) dsVector4d cpuB = {{1.1, 3.5, 5.6, -7.8}};
	DS_ALIGN(32) dsVector4l cpuResult;

	dsSIMD4d a = dsSIMD4d_load(&cpuA);
	dsSIMD4d b = dsSIMD4d_load(&cpuB);

	DS_ALIGN(32) dsVector4d cpuFPResult;
	dsSIMD4d fpResult = dsSIMD4d_min(a, b);
	dsSIMD4d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1, cpuFPResult.x);
	EXPECT_EQ(3.4, cpuFPResult.y);
	EXPECT_EQ(5.6, cpuFPResult.z);
	EXPECT_EQ(-7.8, cpuFPResult.w);

	fpResult = dsSIMD4d_max(a, b);
	dsSIMD4d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.2, cpuFPResult.x);
	EXPECT_EQ(3.5, cpuFPResult.y);
	EXPECT_EQ(5.6, cpuFPResult.z);
	EXPECT_EQ(7.8, cpuFPResult.w);

	dsSIMD4db ab = dsSIMD4d_cmple(a, b);
	fpResult = dsSIMD4d_select(a, b, ab);
	dsSIMD4d_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1, cpuFPResult.x);
	EXPECT_EQ(3.4, cpuFPResult.y);
	EXPECT_EQ(5.6, cpuFPResult.z);
	EXPECT_EQ(-7.8, cpuFPResult.w);

	dsSIMD4db result = dsSIMD4d_cmpeq(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4d_cmpne(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4d_cmplt(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4d_cmple(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4d_cmpgt(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4d_cmpge(a, b);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_true();
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_false();
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4db_not(ab);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_and(ab, dsSIMD4db_true());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4db_and(ab, dsSIMD4db_false());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4db_andnot(ab, dsSIMD4db_true());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_or(ab, dsSIMD4db_true());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_or(ab, dsSIMD4db_false());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4db_ornot(dsSIMD4db_false(), ab);
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4db_xor(ab, dsSIMD4db_false());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4db_xor(ab, dsSIMD4db_true());
	dsSIMD4db_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);
}
DS_SIMD_END();

TEST(SIMDTest, CompareLogicDouble4)
{
#if DS_SIMD_ALWAYS_DOUBLE4
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Double4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double4)
		DS_LOG_INFO("SIMDTest", "Enabling double4 compare logic SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double4 compare logic SIMD tests.");
		return;
	}
#endif

	SIMDTest_CompareLogicDouble4();
}

DS_SIMD_START_HADD();
static void SIMDTest_HAddFloat4()
{
	dsVector4f cpuA = {{1.2f, 3.4f, 5.6f, 7.8f}};
	dsVector4f cpuB = {{-9.8f, -7.6f, -5.4f, -3.2f}};
	dsVector4f cpuResult;

	dsSIMD4f a = dsSIMD4f_load(&cpuA);
	dsSIMD4f b = dsSIMD4f_load(&cpuB);
	dsSIMD4f result = dsSIMD4f_hadd(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuResult.x, cpuA.x + cpuA.y);
	EXPECT_EQ(cpuResult.y, cpuA.z + cpuA.w);
	EXPECT_EQ(cpuResult.z, cpuB.x + cpuB.y);
	EXPECT_EQ(cpuResult.w, cpuB.z + cpuB.w);
}
DS_SIMD_END();

TEST(SIMDTest, HAddFloat4)
{
#if DS_SIMD_ALWAYS_HADD
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_HAdd);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_HAdd)
		DS_LOG_INFO("SIMDTest", "Enabling float4 horizontal add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping float4 horizontal add SIMD tests.");
		return;
	}
#endif

	SIMDTest_HAddFloat4();
}

DS_SIMD_START_DOUBLE2();
DS_SIMD_START_HADD();
static void SIMDTest_HAddDouble2()
{
	dsVector2d cpuA = {{1.2, 3.2}};
	dsVector2d cpuB = {{-9.8, -7.6}};
	dsVector2d cpuResult;

	dsSIMD2d a = dsSIMD2d_load(&cpuA);
	dsSIMD2d b = dsSIMD2d_load(&cpuB);
	dsSIMD2d result = dsSIMD2d_hadd(a, b);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_EQ(cpuResult.x, cpuA.x + cpuA.y);
	EXPECT_EQ(cpuResult.y, cpuB.x + cpuB.y);
}
DS_SIMD_END();
DS_SIMD_END();

TEST(SIMDTest, HAddDouble2)
{
	dsSIMDFeatures features = dsSIMDFeatures_HAdd | dsSIMDFeatures_Double2;
#if DS_SIMD_ALWAYS_HADD && DS_SIMD_ALWAYS_DOUBLE2
	ASSERT_EQ(features, dsHostSIMDFeatures & features);
#else
	if ((dsHostSIMDFeatures & features) == features)
		DS_LOG_INFO("SIMDTest", "Enabling double2 horizontal add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double2 horizontal add SIMD tests.");
		return;
	}
#endif

	SIMDTest_HAddDouble2();
}

DS_SIMD_START_DOUBLE4();
DS_SIMD_START_HADD();
static void SIMDTest_HAddDouble4()
{
	DS_ALIGN(32) dsVector4d cpuA = {{1.2, 3.4, 5.6, 7.8}};
	DS_ALIGN(32) dsVector4d cpuB = {{-9.8, -7.6, -5.4, -3.2}};
	DS_ALIGN(32) dsVector4d cpuResult;

	dsSIMD4d a = dsSIMD4d_load(&cpuA);
	dsSIMD4d b = dsSIMD4d_load(&cpuB);
	dsSIMD4d result = dsSIMD4d_hadd(a, b);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_EQ(cpuResult.x, cpuA.x + cpuA.y);
	EXPECT_EQ(cpuResult.y, cpuB.x + cpuB.y);
	EXPECT_EQ(cpuResult.z, cpuA.z + cpuA.w);
	EXPECT_EQ(cpuResult.w, cpuB.z + cpuB.w);
}
DS_SIMD_END();
DS_SIMD_END();

TEST(SIMDTest, HAddDouble4)
{
	dsSIMDFeatures features = dsSIMDFeatures_HAdd | dsSIMDFeatures_Double4;
#if DS_SIMD_ALWAYS_HADD && DS_SIMD_ALWAYS_DOUBLE4
	ASSERT_EQ(features, dsHostSIMDFeatures & features);
#else
	if ((dsHostSIMDFeatures & features) == features)
		DS_LOG_INFO("SIMDTest", "Enabling double4 horizontal add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double4 horizontal add SIMD tests.");
		return;
	}
#endif

	SIMDTest_HAddDouble4();
}

DS_SIMD_START_FMA();
static void SIMDTest_FMAFloat4()
{
	constexpr float epsilon = 1e-6f;
	dsVector4f cpuA = {{1.2f, 3.4f, 5.6f, 7.8f}};
	dsVector4f cpuB = {{-9.8f, -7.6f, -5.4f, -3.2f}};
	dsVector4f cpuC = {{7.8f, 5.6f, -3.4f, -1.2f}};
	dsVector4f cpuResult;

	dsSIMD4f a = dsSIMD4f_load(&cpuA);
	dsSIMD4f b = dsSIMD4f_load(&cpuB);
	dsSIMD4f c = dsSIMD4f_load(&cpuC);

	dsSIMD4f result = dsSIMD4f_fmadd(a, b, c);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z*cpuB.z + cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w*cpuB.w + cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4f_fmsub(a, b, c);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z*cpuB.z - cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w*cpuB.w - cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4f_fnmadd(a, b, c);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(-cpuA.z*cpuB.z + cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(-cpuA.w*cpuB.w + cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4f_fnmsub(a, b, c);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(-cpuA.z*cpuB.z - cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(-cpuA.w*cpuB.w - cpuC.w, cpuResult.w, epsilon);
}
DS_SIMD_END();

TEST(SIMDTest, FMAFloat4)
{
#if DS_SIMD_ALWAYS_FMA
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_FMA);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_FMA)
		DS_LOG_INFO("SIMDTest", "Enabling float4 fused multiply-add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping float4 fused multiply-add SIMD tests.");
		return;
	}
#endif

	SIMDTest_FMAFloat4();
}

DS_SIMD_START_DOUBLE2();
DS_SIMD_START_FMA();
static void SIMDTest_FMADouble2()
{
	constexpr double epsilon = 1e-12;
	dsVector2d cpuA = {{1.2, 3.4}};
	dsVector2d cpuB = {{-9.8, -7.6}};
	dsVector2d cpuC = {{7.8, 5.6}};
	dsVector2d cpuResult;

	dsSIMD2d a = dsSIMD2d_load(&cpuA);
	dsSIMD2d b = dsSIMD2d_load(&cpuB);
	dsSIMD2d c = dsSIMD2d_load(&cpuC);

	dsSIMD2d result = dsSIMD2d_fmadd(a, b, c);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);

	result = dsSIMD2d_fmsub(a, b, c);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);

	result = dsSIMD2d_fnmadd(a, b, c);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);

	result = dsSIMD2d_fnmsub(a, b, c);
	dsSIMD2d_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);
}
DS_SIMD_END();
DS_SIMD_END();

TEST(SIMDTest, FMADouble2)
{
	dsSIMDFeatures features = dsSIMDFeatures_FMA | dsSIMDFeatures_Double2;
#if DS_SIMD_ALWAYS_FMA && DS_SIMD_ALWAYS_DOUBLE2
	ASSERT_EQ(features, dsHostSIMDFeatures & features);
#else
	if ((dsHostSIMDFeatures & features) == features)
		DS_LOG_INFO("SIMDTest", "Enabling double4 fused multiply-add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double4 fused multiply-add SIMD tests.");
		return;
	}
#endif

	SIMDTest_FMADouble2();
}

DS_SIMD_START_DOUBLE4();
DS_SIMD_START_FMA();
static void SIMDTest_FMADouble4()
{
	constexpr double epsilon = 1e-12;
	DS_ALIGN(32) dsVector4d cpuA = {{1.2, 3.4, 5.6, 7.8}};
	DS_ALIGN(32) dsVector4d cpuB = {{-9.8, -7.6, -5.4, -3.2}};
	DS_ALIGN(32) dsVector4d cpuC = {{7.8, 5.6, -3.4, -1.2}};
	DS_ALIGN(32) dsVector4d cpuResult;

	dsSIMD4d a = dsSIMD4d_load(&cpuA);
	dsSIMD4d b = dsSIMD4d_load(&cpuB);
	dsSIMD4d c = dsSIMD4d_load(&cpuC);

	dsSIMD4d result = dsSIMD4d_fmadd(a, b, c);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z*cpuB.z + cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w*cpuB.w + cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4d_fmsub(a, b, c);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z*cpuB.z - cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w*cpuB.w - cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4d_fnmadd(a, b, c);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x + cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y + cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(-cpuA.z*cpuB.z + cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(-cpuA.w*cpuB.w + cpuC.w, cpuResult.w, epsilon);

	result = dsSIMD4d_fnmsub(a, b, c);
	dsSIMD4d_store(&cpuResult, result);
	EXPECT_NEAR(-cpuA.x*cpuB.x - cpuC.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuA.y*cpuB.y - cpuC.y, cpuResult.y, epsilon);
	EXPECT_NEAR(-cpuA.z*cpuB.z - cpuC.z, cpuResult.z, epsilon);
	EXPECT_NEAR(-cpuA.w*cpuB.w - cpuC.w, cpuResult.w, epsilon);
}
DS_SIMD_END();
DS_SIMD_END();

TEST(SIMDTest, FMADouble4)
{
	dsSIMDFeatures features = dsSIMDFeatures_FMA | dsSIMDFeatures_Double4;
#if DS_SIMD_ALWAYS_FMA && DS_SIMD_ALWAYS_DOUBLE4
	ASSERT_EQ(features, dsHostSIMDFeatures & features);
#else
	if ((dsHostSIMDFeatures & features) == features)
		DS_LOG_INFO("SIMDTest", "Enabling double4 fused multiply-add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping double4 fused multiply-add SIMD tests.");
		return;
	}
#endif

	SIMDTest_FMADouble4();
}

DS_SIMD_START_HALF_FLOAT();
static void SIMDTest_HalfFloat()
{
	constexpr uint16_t unset = 0xFFFF;
	constexpr float epsilon = 1e-2f;
	dsVector4f cpuA = {{1.2f, 3.4f, 5.6f, 7.8f}};
	uint16_t padding; // Ensure unaligned.
	DS_UNUSED(padding);
	uint16_t cpuHalfFloat[4] = {unset, unset, unset, unset};
	dsVector4f cpuFullFloat;

	dsSIMD4f a = dsSIMD4f_load(&cpuA);

	dsSIMD4hf halfFloat = dsSIMD4hf_fromFloat(a);
	dsSIMD4hf_store1(cpuHalfFloat, halfFloat);
	EXPECT_NE(unset, cpuHalfFloat[0]);
	EXPECT_EQ(unset, cpuHalfFloat[1]);
	EXPECT_EQ(unset, cpuHalfFloat[2]);
	EXPECT_EQ(unset, cpuHalfFloat[3]);

	halfFloat = dsSIMD4hf_load1(cpuHalfFloat);
	dsSIMD4f fullFloat = dsSIMD4hf_toFloat(halfFloat);
	dsSIMD4f_store(&cpuFullFloat, fullFloat);
	EXPECT_NEAR(cpuA.x, cpuFullFloat.x, epsilon);

	halfFloat = dsSIMD4hf_fromFloat(a);
	dsSIMD4hf_store2(cpuHalfFloat, halfFloat);
	EXPECT_NE(unset, cpuHalfFloat[0]);
	EXPECT_NE(unset, cpuHalfFloat[1]);
	EXPECT_EQ(unset, cpuHalfFloat[2]);
	EXPECT_EQ(unset, cpuHalfFloat[3]);

	halfFloat = dsSIMD4hf_load2(cpuHalfFloat);
	fullFloat = dsSIMD4hf_toFloat(halfFloat);
	dsSIMD4f_store(&cpuFullFloat, fullFloat);
	EXPECT_NEAR(cpuA.x, cpuFullFloat.x, epsilon);
	EXPECT_NEAR(cpuA.y, cpuFullFloat.y, epsilon);

	halfFloat = dsSIMD4hf_fromFloat(a);
	dsSIMD4hf_store4(cpuHalfFloat, halfFloat);
	EXPECT_NE(unset, cpuHalfFloat[0]);
	EXPECT_NE(unset, cpuHalfFloat[1]);
	EXPECT_NE(unset, cpuHalfFloat[2]);
	EXPECT_NE(unset, cpuHalfFloat[3]);

	halfFloat = dsSIMD4hf_load4(cpuHalfFloat);
	fullFloat = dsSIMD4hf_toFloat(halfFloat);
	dsSIMD4f_store(&cpuFullFloat, fullFloat);
	EXPECT_NEAR(cpuA.x, cpuFullFloat.x, epsilon);
	EXPECT_NEAR(cpuA.y, cpuFullFloat.y, epsilon);
	EXPECT_NEAR(cpuA.z, cpuFullFloat.z, epsilon);
	EXPECT_NEAR(cpuA.w, cpuFullFloat.w, epsilon);
}
DS_SIMD_END();

TEST(SIMDTest, HalfFloat)
{
#if DS_SIMD_ALWAYS_HALF_FLOAT
	ASSERT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat)
		DS_LOG_INFO("SIMDTest", "Enabling half float SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping half float SIMD tests.");
		return;
	}
#endif

	SIMDTest_HalfFloat();
}

#endif
