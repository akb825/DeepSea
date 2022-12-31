/*
 * Copyright 2022 Aaron Barany
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
TEST(SIMDTest, Float4)
{
#if DS_SIMD_ALWAYS_FLOAT4
	EXPECT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Float4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DS_LOG_INFO("SIMDTest", "Enabling float4 SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping float4 SIMD tests.");
		return;
	}
#endif

	constexpr float epsilon = 5e-3f;
	float padding1; // Keep next value unalied.
	DS_UNUSED(padding1);
	dsVector4f cpuA = {{1.2f, 3.4f, 5.6f, 7.8f}};
	dsVector4f cpuB = {{-9.8f, -7.6f, -5.4f, -3.2f}};
	dsVector4f cpuResult;
	float padding2; // Keep next value unalied.
	DS_UNUSED(padding2);
	dsVector4f unalginedCPUResult;

	dsSIMD4f a = dsSIMD4f_loadUnaligned(&cpuA);
	dsSIMD4f b = dsSIMD4f_load(&cpuB);

	dsSIMD4f result = dsSIMD4f_set1(0.1f);
	dsSIMD4f_storeUnaligned(&unalginedCPUResult, result);
	EXPECT_EQ(0.1f, unalginedCPUResult.x);
	EXPECT_EQ(0.1f, unalginedCPUResult.y);
	EXPECT_EQ(0.1f, unalginedCPUResult.z);
	EXPECT_EQ(0.1f, unalginedCPUResult.w);

	result = dsSIMD4f_set4(0.1f, 0.2f, 0.3f, 0.4f);
	EXPECT_EQ(0.1f, dsSIMD4f_get(result, 0));
	EXPECT_EQ(0.2f, dsSIMD4f_get(result, 1));
	EXPECT_EQ(0.3f, dsSIMD4f_get(result, 2));
	EXPECT_EQ(0.4f, dsSIMD4f_get(result, 3));

	result = dsSIMD4f_neg(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(-cpuA.x, cpuResult.x);
	EXPECT_EQ(-cpuA.y, cpuResult.y);
	EXPECT_EQ(-cpuA.z, cpuResult.z);
	EXPECT_EQ(-cpuA.w, cpuResult.w);

	result = dsSIMD4f_neg(b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(-cpuB.x, cpuResult.x);
	EXPECT_EQ(-cpuB.y, cpuResult.y);
	EXPECT_EQ(-cpuB.z, cpuResult.z);
	EXPECT_EQ(-cpuB.w, cpuResult.w);

	result = dsSIMD4f_add(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA.x + cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA.y + cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA.z + cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA.w + cpuB.w, cpuResult.w);

	result = dsSIMD4f_sub(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA.x - cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA.y - cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA.z - cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA.w - cpuB.w, cpuResult.w);

	result = dsSIMD4f_mul(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_EQ(cpuA.x*cpuB.x, cpuResult.x);
	EXPECT_EQ(cpuA.y*cpuB.y, cpuResult.y);
	EXPECT_EQ(cpuA.z*cpuB.z, cpuResult.z);
	EXPECT_EQ(cpuA.w*cpuB.w, cpuResult.w);

	result = dsSIMD4f_div(a, b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x/cpuB.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y/cpuB.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z/cpuB.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w/cpuB.w, cpuResult.w, epsilon);

	result = dsSIMD4f_rcp(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/cpuA.x, cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/cpuA.y, cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/cpuA.z, cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/cpuA.w, cpuResult.w, epsilon);

	result = dsSIMD4f_sqrt(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(std::sqrt(cpuA.x), cpuResult.x, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA.y), cpuResult.y, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA.z), cpuResult.z, epsilon);
	EXPECT_NEAR(std::sqrt(cpuA.w), cpuResult.w, epsilon);

	result = dsSIMD4f_rsqrt(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA.x), cpuResult.x, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA.y), cpuResult.y, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA.z), cpuResult.z, epsilon);
	EXPECT_NEAR(1.0f/std::sqrt(cpuA.w), cpuResult.w, epsilon);

	result = dsSIMD4f_abs(a);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(cpuA.x, cpuResult.x, epsilon);
	EXPECT_NEAR(cpuA.y, cpuResult.y, epsilon);
	EXPECT_NEAR(cpuA.z, cpuResult.z, epsilon);
	EXPECT_NEAR(cpuA.w, cpuResult.w, epsilon);

	result = dsSIMD4f_abs(b);
	dsSIMD4f_store(&cpuResult, result);
	EXPECT_NEAR(-cpuB.x, cpuResult.x, epsilon);
	EXPECT_NEAR(-cpuB.y, cpuResult.y, epsilon);
	EXPECT_NEAR(-cpuB.z, cpuResult.z, epsilon);
	EXPECT_NEAR(-cpuB.w, cpuResult.w, epsilon);

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

	EXPECT_EQ(cpuA.x, cpuAT.x);
	EXPECT_EQ(cpuB.x, cpuAT.y);
	EXPECT_EQ(cpuC.x, cpuAT.z);
	EXPECT_EQ(cpuD.x, cpuAT.w);

	EXPECT_EQ(cpuA.y, cpuBT.x);
	EXPECT_EQ(cpuB.y, cpuBT.y);
	EXPECT_EQ(cpuC.y, cpuBT.z);
	EXPECT_EQ(cpuD.y, cpuBT.w);

	EXPECT_EQ(cpuA.z, cpuCT.x);
	EXPECT_EQ(cpuB.z, cpuCT.y);
	EXPECT_EQ(cpuC.z, cpuCT.z);
	EXPECT_EQ(cpuD.z, cpuCT.w);

	EXPECT_EQ(cpuA.w, cpuDT.x);
	EXPECT_EQ(cpuB.w, cpuDT.y);
	EXPECT_EQ(cpuC.w, cpuDT.z);
	EXPECT_EQ(cpuD.w, cpuDT.w);
}
DS_SIMD_END();

DS_SIMD_START_FLOAT4();
TEST(SIMDTest, CompareLogic)
{
#if DS_SIMD_ALWAYS_FLOAT4
	EXPECT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_Float4);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DS_LOG_INFO("SIMDTest", "Enabling compare logic SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping compare logic SIMD tests.");
		return;
	}
#endif

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

	dsSIMD4b ab = dsSIMD4f_cmple(a, b);
	fpResult = dsSIMD4f_select(a, b, ab);
	dsSIMD4f_store(&cpuFPResult, fpResult);
	EXPECT_EQ(1.1f, cpuFPResult.x);
	EXPECT_EQ(3.4f, cpuFPResult.y);
	EXPECT_EQ(5.6f, cpuFPResult.z);
	EXPECT_EQ(-7.8f, cpuFPResult.w);

	dsSIMD4b result = dsSIMD4f_cmpeq(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmpne(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4f_cmplt(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmple(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4f_cmpgt(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4f_cmpge(a, b);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_true();
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_false();
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4b_not(ab);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_and(ab, dsSIMD4b_true());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4b_and(ab, dsSIMD4b_false());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4b_andnot(ab, dsSIMD4b_true());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_or(ab, dsSIMD4b_true());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_or(ab, dsSIMD4b_false());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4b_ornot(dsSIMD4b_false(), ab);
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);

	result = dsSIMD4b_xor(ab, dsSIMD4b_false());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_FALSE(cpuResult.x);
	EXPECT_TRUE(cpuResult.y);
	EXPECT_TRUE(cpuResult.z);
	EXPECT_FALSE(cpuResult.w);

	result = dsSIMD4b_xor(ab, dsSIMD4b_true());
	dsSIMD4b_store(&cpuResult, result);
	EXPECT_TRUE(cpuResult.x);
	EXPECT_FALSE(cpuResult.y);
	EXPECT_FALSE(cpuResult.z);
	EXPECT_TRUE(cpuResult.w);
}
DS_SIMD_END();

DS_SIMD_START_HADD();
TEST(SIMDTest, HAdd)
{
#if DS_SIMD_ALWAYS_HADD
	EXPECT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_HAdd);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_HAdd)
		DS_LOG_INFO("SIMDTest", "Enabling horizontal add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping horizontal add SIMD tests.");
		return;
	}
#endif

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

DS_SIMD_START_FMA();
TEST(SIMDTest, FMA)
{
#if DS_SIMD_ALWAYS_FMA
	EXPECT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_FMA);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_FMA)
		DS_LOG_INFO("SIMDTest", "Enabling fused multiply-add SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping fused multiply-add SIMD tests.");
		return;
	}
#endif

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

DS_SIMD_START_HALF_FLOAT();
TEST(SIMDTest, HalfFloat)
{
#if DS_SIMD_ALWAYS_HALF_FLOAT
	EXPECT_TRUE(dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat);
#else
	if (dsHostSIMDFeatures & dsSIMDFeatures_HalfFloat)
		DS_LOG_INFO("SIMDTest", "Enabling half float SIMD at runtime.");
	else
	{
		DS_LOG_INFO("SIMDTest", "Skipping half float SIMD tests.");
		return;
	}
#endif

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

#endif
