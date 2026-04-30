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

#include "Determinism.h"
#include <DeepSea/Core/Timer.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Trig.h>
#include <DeepSea/Math/Types.h>

#include <gtest/gtest.h>

#include <cmath>
#include <functional>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

// NOTE: For most accurate comparative performance test results on x86, build with architecture
// level 1 or 2.
#define DS_PERFORMANCE_TESTS 0

template <typename T>
struct TrigTypeSelector;

template <>
struct TrigTypeSelector<float>
{
	static const float epsilon;
};

template <>
struct TrigTypeSelector<double>
{
	static const double epsilon;
};

const float TrigTypeSelector<float>::epsilon = 3e-7f;
const double TrigTypeSelector<double>::epsilon = 4e-16;

template <typename T>
class TrigTest : public testing::Test
{
};

using TrigTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(TrigTest, TrigTypes);

inline float dsSin(float angle)
{
	return dsSinf(angle);
}

inline double dsSin(double angle)
{
	return dsSind(angle);
}

inline float dsCos(float angle)
{
	return dsCosf(angle);
}

inline double dsCos(double angle)
{
	return dsCosd(angle);
}

inline void dsSinCos(float& outSin, float& outCos, float angle)
{
	return dsSinCosf(&outSin, &outCos, angle);
}

inline void dsSinCos(double& outSin, double& outCos, double angle)
{
	return dsSinCosd(&outSin, &outCos, angle);
}

inline float dsTan(float angle)
{
	return dsTanf(angle);
}

inline double dsTan(double angle)
{
	return dsTand(angle);
}

inline float dsASin(float x)
{
	return dsASinf(x);
}

inline double dsASin(double x)
{
	return dsASind(x);
}

inline float dsACos(float x)
{
	return dsACosf(x);
}

inline double dsACos(double x)
{
	return dsACosd(x);
}

inline float dsATan(float x)
{
	return dsATanf(x);
}

inline double dsATan(double x)
{
	return dsATand(x);
}

inline float dsATan2(float y, float x)
{
	return dsATan2f(y, x);
}

inline double dsATan2(double y, double x)
{
	return dsATan2d(y, x);
}

inline float randomValue(dsRandom& random, float min, float max)
{
	return dsRandom_nextFloatRange(&random, min, max);
}

inline double randomValue(dsRandom& random, double min, double max)
{
	return dsRandom_nextDoubleRange(&random, min, max);
}

#if DS_HAS_SIMD
template <typename T>
T* alignPtr(T* ptr, unsigned int alignment)
{
	return reinterpret_cast<T*>(
		(reinterpret_cast<std::size_t>(ptr) + alignment - 1) & ~(std::size_t(alignment) - 1));
}
#endif

constexpr unsigned int sampleCount = 1000;

TYPED_TEST(TrigTest, Sin)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam angle = randomValue(random, TypeParam(-1000), TypeParam(1000));
		TypeParam stdSin = std::sin(angle);
		TypeParam customSin = dsSin(angle);
		EXPECT_RELATIVE_EQ(stdSin, customSin, TypeParam(0), epsilon) << angle;
	}
}

TYPED_TEST(TrigTest, Cos)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam angle = randomValue(random, TypeParam(-1000), TypeParam(1000));
		TypeParam stdCos = std::cos(angle);
		TypeParam customCos = dsCos(angle);
		EXPECT_RELATIVE_EQ(stdCos, customCos, TypeParam(0), epsilon) << angle;
	}
}

TYPED_TEST(TrigTest, SinCos)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam angle = randomValue(random, TypeParam(-1000), TypeParam(1000));
		TypeParam stdSin = std::sin(angle);
		TypeParam stdCos = std::cos(angle);
		TypeParam customSin, customCos;
		dsSinCos(customSin, customCos, angle);
		EXPECT_RELATIVE_EQ(stdSin, customSin, TypeParam(0), epsilon) << angle;
		EXPECT_RELATIVE_EQ(stdCos, customCos, TypeParam(0), epsilon) << angle;
	}
}

TYPED_TEST(TrigTest, Tan)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam angle = randomValue(random, TypeParam(-1000), TypeParam(1000));
		TypeParam stdTan = std::tan(angle);
		TypeParam customTan = dsTan(angle);
		EXPECT_RELATIVE_EQ(stdTan, customTan, TypeParam(0), epsilon) << angle;
	}
}

TYPED_TEST(TrigTest, ASin)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam x = randomValue(random, TypeParam(-1), TypeParam(1));
		TypeParam stdASin = std::asin(x);
		TypeParam customASin = dsASin(x);
		EXPECT_RELATIVE_EQ(stdASin, customASin, TypeParam(0), epsilon) << x;
	}
}

TYPED_TEST(TrigTest, ACos)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam x = randomValue(random, TypeParam(-1), TypeParam(1));
		TypeParam stdACos = std::acos(x);
		TypeParam customACos = dsACos(x);
		EXPECT_RELATIVE_EQ(stdACos, customACos, TypeParam(0), epsilon) << x;
	}
}

TYPED_TEST(TrigTest, ATan)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam x = randomValue(random, TypeParam(-10), TypeParam(10));
		TypeParam stdATan = std::atan(x);
		TypeParam customATan = dsATan(x);
		EXPECT_RELATIVE_EQ(stdATan, customATan, TypeParam(0), epsilon) << x;
	}
}

TYPED_TEST(TrigTest, ATan2)
{
	TypeParam epsilon = TrigTypeSelector<TypeParam>::epsilon;

	// Test identity values.
	EXPECT_EQ(TypeParam(M_PI), dsATan2(TypeParam(0.0), TypeParam(-1.0)));
	EXPECT_EQ(TypeParam(M_PI), dsATan2(TypeParam(0.0), TypeParam(-0.0)));
	EXPECT_EQ(TypeParam(0), dsATan2(TypeParam(-0.0), TypeParam(1.0)));
	EXPECT_EQ(TypeParam(0), dsATan2(TypeParam(-0.0), TypeParam(0.0)));
	EXPECT_EQ(TypeParam(-M_PI_2), dsATan2(TypeParam(-1.0), TypeParam(-0.0)));
	EXPECT_EQ(TypeParam(-M_PI_2), dsATan2(TypeParam(-1.0), TypeParam(0.0)));
	EXPECT_EQ(TypeParam(M_PI_2), dsATan2(TypeParam(1.0), TypeParam(-0.0)));
	EXPECT_EQ(TypeParam(M_PI_2), dsATan2(TypeParam(1.0), TypeParam(0.0)));

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		TypeParam y = randomValue(random, TypeParam(-10), TypeParam(10));
		TypeParam x = randomValue(random, TypeParam(-10), TypeParam(10));
		TypeParam stdATan = std::atan2(y, x);
		TypeParam customATan = dsATan2(y, x);
		EXPECT_RELATIVE_EQ(stdATan, customATan, TypeParam(0), epsilon) << x << ", " << y;
	}
}

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)

static void TrigFloatTest_SinSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f sinAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		sinAngles.simd = dsSinSIMD4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsSinf(angles.values[j]), sinAngles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	sinAngles.simd = dsSinSIMD4f(dsSIMD4f_set4(-12.34f, -0.1234f, 0.4321f, 2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.22444208f, sinAngles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.123087063f, sinAngles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.418778718f, sinAngles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.71497798f, sinAngles.w, 0.0f, epsilon);
}

static void TrigFloatTest_CosSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		cosAngles.simd = dsCosSIMD4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsCosf(angles.values[j]), cosAngles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	cosAngles.simd = dsCosSIMD4f(dsSIMD4f_set4(-12.34f, -0.1234f, 0.4321f, 2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.974487424f, cosAngles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.992395878f, cosAngles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.908088326f, cosAngles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.699147f, cosAngles.w, 0.0f, epsilon);
}

static void TrigFloatTest_SinCosSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f sinAngles, cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		dsSinCosSIMD4f(&sinAngles.simd, &cosAngles.simd, angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsSinf(angles.values[j]), sinAngles.values[j], 0.0f, epsilon) << angles.values[j];
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsCosf(angles.values[j]), cosAngles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	dsSinCosSIMD4f(
		&sinAngles.simd, &cosAngles.simd, dsSIMD4f_set4(-12.34f, -0.1234f, 0.4321f, 2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.22444208f, sinAngles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.123087063f, sinAngles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.418778718f, sinAngles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.71497798f, sinAngles.w, 0.0f, epsilon);

	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.974487424f, cosAngles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.992395878f, cosAngles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.908088326f, cosAngles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.699147f, cosAngles.w, 0.0f, epsilon);
}

static void TrigFloatTest_TanSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f tanAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		tanAngles.simd = dsTanSIMD4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsTanf(angles.values[j]), tanAngles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	tanAngles.simd = dsTanSIMD4f(dsSIMD4f_set4(-12.34f, -0.1234f, 0.4321f, 2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.230318084f, tanAngles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.124030203f, tanAngles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.46116516f, tanAngles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.02264333f, tanAngles.w, 0.0f, epsilon);
}

static void TrigFloatTest_ASinSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0f, 1.0f);
		angles.simd = dsASinSIMD4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsASinf(x.values[j]), angles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsASinSIMD4f(dsSIMD4f_set4(-0.9876f, -0.4321f, 0.1234f, 0.6789f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.41315317f, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.44682008f, angles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.123715349f, angles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.746263444f, angles.w, 0.0f, epsilon);
}

static void TrigFloatTest_ACosSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0f, 1.0f);
		angles.simd = dsACosSIMD4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsACosf(x.values[j]), angles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsACosSIMD4f(dsSIMD4f_set4(-0.9876f, -0.4321f, 0.1234f, 0.6789f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.98394966f, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.01761651f, angles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(1.44708097f, angles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.824532926f, angles.w, 0.0f, epsilon);
}

static void TrigFloatTest_ATanSIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -10.0f, 10.0f);
		angles.simd = dsATanSIMD4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsATanf(x.values[j]), angles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsATanSIMD4f(dsSIMD4f_set4(-12.34f, -0.1234f, 0.4321f, 2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.48993576f, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.12277931f, angles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.407869041f, angles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(1.16770732f, angles.w, 0.0f, epsilon);
}

static void TrigFloatTest_ATan2SIMD()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	// Test identity values.
	dsVector4f angles;
	angles.simd = dsATan2SIMD4f(
		dsSIMD4f_set4(0.0f, 0.0f, -0.0f, -0.0f), dsSIMD4f_set4(-1.0f, -0.0f, 1.0f, 0.0f));
	EXPECT_EQ(M_PIf, angles.x);
	EXPECT_EQ(M_PIf, angles.y);
	EXPECT_EQ(0.0f, angles.z);
	EXPECT_EQ(0.0f, angles.w);

	angles.simd = dsATan2SIMD4f(
		dsSIMD4f_set4(-1.0f, -1.0f, 1.0f, 1.0f), dsSIMD4f_set4(-0.0f, 0.0f, -0.0f, 0.0f));
	EXPECT_EQ(-M_PI_2f, angles.x);
	EXPECT_EQ(-M_PI_2f, angles.y);
	EXPECT_EQ(M_PI_2f, angles.z);
	EXPECT_EQ(M_PI_2f, angles.w);

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		dsVector4f x, y;
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			x.values[j] = randomValue(random, -10.0f, 10.0f);
			y.values[j] = randomValue(random, -10.0f, 10.0f);
		}
		angles.simd = dsATan2SIMD4f(y.simd, x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(dsATan2f(y.values[j], x.values[j]),
				angles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsATan2SIMD4f(dsSIMD4f_set4(-0.9876f, -0.4321f, 0.1234f, 0.6789f),
		dsSIMD4f_set4(-12.34f, 0.1234f, 0.4321f, -2.345f));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-3.06173062f, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.29261899f, angles.y, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.278177381f, angles.z, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.8597877f, angles.w, 0.0f, epsilon);
}

DS_SIMD_END()

TEST(TrigFloatTest, SinSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_SinSIMD();
}

TEST(TrigFloatTest, CosSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_CosSIMD();
}

TEST(TrigFloatTest, SinCosSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_SinCosSIMD();
}

TEST(TrigFloatTest, TanSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_TanSIMD();
}

TEST(TrigFloatTest, ASinSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ASinSIMD();
}

TEST(TrigFloatTest, ACosSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ACosSIMD();
}

TEST(TrigFloatTest, ATanSIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ATanSIMD();
}

TEST(TrigFloatTest, ATan2SIMD)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ATan2SIMD();
}

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT,DS_SIMD_FMA)

static void TrigFloatTest_SinFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f sinAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		sinAngles.simd = dsSinFMA4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSinf(angles.values[j]), sinAngles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_CosFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		cosAngles.simd = dsCosFMA4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsCosf(angles.values[j]), cosAngles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_SinCosFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f sinAngles, cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		dsSinCosFMA4f(&sinAngles.simd, &cosAngles.simd, angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSinf(angles.values[j]), sinAngles.values[j], 0.0f, epsilon) <<
				angles.values[j];
			EXPECT_RELATIVE_EQ(dsCosf(angles.values[j]), cosAngles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_TanFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f tanAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0f, 1000.0f);
		tanAngles.simd = dsTanFMA4f(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsTanf(angles.values[j]), tanAngles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_ASinFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0f, 1.0f);
		angles.simd = dsASinFMA4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsASinf(x.values[j]), angles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_ACosFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0f, 1.0f);
		angles.simd = dsACosFMA4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsACosf(x.values[j]), angles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_ATanFMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector4f angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector4f x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -10.0f, 10.0f);
		angles.simd = dsATanFMA4f(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATanf(x.values[j]), angles.values[j], 0.0f, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigFloatTest_ATan2FMA()
{
	constexpr unsigned int vecSize = 4;
	float epsilon = TrigTypeSelector<float>::epsilon;

	// Test identity values.
	dsVector4f angles;
	angles.simd = dsATan2FMA4f(
		dsSIMD4f_set4(0.0f, 0.0f, -0.0f, -0.0f), dsSIMD4f_set4(-1.0f, -0.0f, 1.0f, 0.0f));
	EXPECT_EQ(M_PIf, angles.x);
	EXPECT_EQ(M_PIf, angles.y);
	EXPECT_EQ(0.0f, angles.z);
	EXPECT_EQ(0.0f, angles.w);

	angles.simd = dsATan2FMA4f(
		dsSIMD4f_set4(-1.0f, -1.0f, 1.0f, 1.0f), dsSIMD4f_set4(-0.0f, 0.0f, -0.0f, 0.0f));
	EXPECT_EQ(-M_PI_2f, angles.x);
	EXPECT_EQ(-M_PI_2f, angles.y);
	EXPECT_EQ(M_PI_2f, angles.z);
	EXPECT_EQ(M_PI_2f, angles.w);

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		dsVector4f x, y;
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			x.values[j] = randomValue(random, -10.0f, 10.0f);
			y.values[j] = randomValue(random, -10.0f, 10.0f);
		}
		angles.simd = dsATan2FMA4f(y.simd, x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATan2f(y.values[j], x.values[j]),
				angles.values[j], 0.0f, epsilon) << angles.values[j];
		}
	}
}

DS_SIMD_END()

TEST(TrigFloatTest, SinFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_SinFMA();
}

TEST(TrigFloatTest, CosFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_CosFMA();
}

TEST(TrigFloatTest, SinCosFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_SinCosFMA();
}

TEST(TrigFloatTest, TanFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_TanFMA();
}

TEST(TrigFloatTest, ASinFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ASinFMA();
}

TEST(TrigFloatTest, ACosFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ACosFMA();
}

TEST(TrigFloatTest, ATanFMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ATanFMA();
}

TEST(TrigFloatTest, ATan2FMA)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_ATan2FMA();
}
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)

static void TrigDoubleTest_SinSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d sinAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		sinAngles.simd = dsSinSIMD2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	sinAngles.simd = dsSinSIMD2d(dsSIMD2d_set2(-12.34, -0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.22444221895185537, sinAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.12308705821137626, sinAngles.y, 0.0, epsilon);
	sinAngles.simd = dsSinSIMD2d(dsSIMD2d_set2(0.4321, 2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.41877870990075816, sinAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.71497801013649265, sinAngles.y, 0.0, epsilon);
}

static void TrigDoubleTest_CosSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		cosAngles.simd = dsCosSIMD2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	cosAngles.simd = dsCosSIMD2d(dsSIMD2d_set2(-12.34, -0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.97448739876509816, cosAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.99239587670489104, cosAngles.y, 0.0, epsilon);
	cosAngles.simd = dsCosSIMD2d(dsSIMD2d_set2(0.4321, 2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.90808831736448226, cosAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.69914694093678287, cosAngles.y, 0.0, epsilon);
}

static void TrigDoubleTest_SinCosSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d sinAngles, cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSinCosSIMD2d(&sinAngles.simd, &cosAngles.simd, angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) << angles.values[j];
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	dsSinCosSIMD2d(&sinAngles.simd, &cosAngles.simd, dsSIMD2d_set2(-12.34, -0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.22444221895185537, sinAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.12308705821137626, sinAngles.y, 0.0, epsilon);

	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.97448739876509816, cosAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.99239587670489104, cosAngles.y, 0.0, epsilon);

	dsSinCosSIMD2d(&sinAngles.simd, &cosAngles.simd, dsSIMD2d_set2(0.4321, 2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.41877870990075816, sinAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.71497801013649265, sinAngles.y, 0.0, epsilon);

	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.90808831736448226, cosAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.69914694093678287, cosAngles.y, 0.0, epsilon);
}

static void TrigDoubleTest_TanSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d tanAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		tanAngles.simd = dsTanSIMD2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsTand(angles.values[j]), tanAngles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	tanAngles.simd = dsTanSIMD2d(dsSIMD2d_set2(-12.34, -0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.23031823627096235, tanAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.12403019913793806, tanAngles.y, 0.0, epsilon);
	tanAngles.simd = dsTanSIMD2d(dsSIMD2d_set2(0.4321, 2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.46116517732126228, tanAngles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.0226434076626265, tanAngles.y, 0.0, epsilon);
}

static void TrigDoubleTest_ASinSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		angles.simd = dsASinSIMD2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsASind(x.values[j]), angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsASinSIMD2d(dsSIMD2d_set2(-0.9876, -0.4321));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.4131529841206687, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.44682009105875037, angles.y, 0.0, epsilon);
	angles.simd = dsASinSIMD2d(dsSIMD2d_set2(0.1234, 0.6789));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.12371534584255098, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.74626342835318737, angles.y, 0.0, epsilon);
}

static void TrigDoubleTest_ACosSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		angles.simd = dsACosSIMD2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsACosd(x.values[j]), angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsACosSIMD2d(dsSIMD2d_set2(-0.9876, -0.4321));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.9839493109155653, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.0176164178536471, angles.y, 0.0, epsilon);
	angles.simd = dsACosSIMD2d(dsSIMD2d_set2(0.1234, 0.6789));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(1.4470809809523457, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.82453289844170918, angles.y, 0.0, epsilon);
}

static void TrigDoubleTest_ATanSIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -10.0, 10.0);
		angles.simd = dsATanSIMD2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(
				dsATand(x.values[j]), angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsATanSIMD2d(dsSIMD2d_set2(-12.34, -0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.4899357456343294, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-0.12277930094473836, angles.y, 0.0, epsilon);
	angles.simd = dsATanSIMD2d(dsSIMD2d_set2(0.4321, 2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.40786900830223771, angles.x, 0.0, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(1.1677072684050145, angles.y, 0.0, epsilon);
}

static void TrigDoubleTest_ATan2SIMD2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	// Test identity values.
	dsVector2d angles;
	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(0.0, 0.0), dsSIMD2d_set2(-1.0, -0.0));
	EXPECT_EQ(M_PI, angles.x);
	EXPECT_EQ(M_PI, angles.y);
	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(-0.0, -0.0), dsSIMD2d_set2(1.0, 0.0));
	EXPECT_EQ(0.0, angles.x);
	EXPECT_EQ(0.0, angles.y);

	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(-1.0, -1.0), dsSIMD2d_set2(-0.0, 0.0));
	EXPECT_EQ(-M_PI_2, angles.x);
	EXPECT_EQ(-M_PI_2, angles.y);
	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(1.0, 1.0), dsSIMD2d_set2(-0.0, 0.0));
	EXPECT_EQ(M_PI_2, angles.x);
	EXPECT_EQ(M_PI_2, angles.y);

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		dsVector2d x, y;
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			x.values[j] = randomValue(random, -10.0, 10.0);
			y.values[j] = randomValue(random, -10.0, 10.0);
		}
		angles.simd = dsATan2SIMD2d(y.simd, x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ_DETERMINISTIC(dsATan2d(y.values[j], x.values[j]),
				angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(-0.9876, -0.4321), dsSIMD2d_set2(-12.34, 0.1234));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-3.0617304591858581, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(-1.2926189358619253, angles.y, 0.0f, epsilon);
	angles.simd = dsATan2SIMD2d(dsSIMD2d_set2(0.1234, 0.6789), dsSIMD2d_set2(0.4321, -2.345));
	EXPECT_RELATIVE_EQ_DETERMINISTIC(0.27817739093297111, angles.x, 0.0f, epsilon);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(2.8597876524421402, angles.y, 0.0f, epsilon);
}

DS_SIMD_END()

TEST(TrigDoubleTest, SinSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinSIMD2();
}

TEST(TrigDoubleTest, CosSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_CosSIMD2();
}

TEST(TrigDoubleTest, SinCosSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinCosSIMD2();
}

TEST(TrigDoubleTest, TanSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_TanSIMD2();
}

TEST(TrigDoubleTest, ASinSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ASinSIMD2();
}

TEST(TrigDoubleTest, ACosSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ACosSIMD2();
}

TEST(TrigDoubleTest, ATanSIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATanSIMD2();
}

TEST(TrigDoubleTest, ATan2SIMD2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATan2SIMD2();
}
#if !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT,DS_SIMD_FMA)

static void TrigDoubleTest_SinFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d sinAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		sinAngles.simd = dsSinFMA2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_CosFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		cosAngles.simd = dsCosFMA2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_SinCosFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d sinAngles, cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSinCosFMA2d(&sinAngles.simd, &cosAngles.simd, angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
			EXPECT_RELATIVE_EQ(dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_TanFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d tanAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		tanAngles.simd = dsTanFMA2d(angles.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsTand(angles.values[j]), tanAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_ASinFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		angles.simd = dsASinFMA2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsASind(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_ACosFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		angles.simd = dsACosFMA2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsACosd(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_ATanFMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	dsVector2d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		dsVector2d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -10.0, 10.0);
		angles.simd = dsATanFMA2d(x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATand(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}
}

static void TrigDoubleTest_ATan2FMA2()
{
	constexpr unsigned int vecSize = 2;
	double epsilon = TrigTypeSelector<double>::epsilon;

	// Test identity values.
	dsVector2d angles;
	angles.simd = dsATan2FMA2d(dsSIMD2d_set2(0.0, 0.0), dsSIMD2d_set2(-1.0, -0.0));
	EXPECT_EQ(M_PI, angles.x);
	EXPECT_EQ(M_PI, angles.y);
	angles.simd = dsATan2FMA2d(dsSIMD2d_set2(-0.0, -0.0), dsSIMD2d_set2(1.0, 0.0));
	EXPECT_EQ(0.0, angles.x);
	EXPECT_EQ(0.0, angles.y);

	angles.simd = dsATan2FMA2d(dsSIMD2d_set2(-1.0, -1.0), dsSIMD2d_set2(-0.0, 0.0));
	EXPECT_EQ(-M_PI_2, angles.x);
	EXPECT_EQ(-M_PI_2, angles.y);
	angles.simd = dsATan2FMA2d(dsSIMD2d_set2(1.0, 1.0), dsSIMD2d_set2(-0.0, 0.0));
	EXPECT_EQ(M_PI_2, angles.x);
	EXPECT_EQ(M_PI_2, angles.y);

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		dsVector2d x, y;
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			x.values[j] = randomValue(random, -10.0, 10.0);
			y.values[j] = randomValue(random, -10.0, 10.0);
		}
		angles.simd = dsATan2FMA2d(y.simd, x.simd);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATan2d(y.values[j], x.values[j]),
				angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}
}

DS_SIMD_END()

TEST(TrigDoubleTest, SinFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinFMA2();
}

TEST(TrigDoubleTest, CosFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_CosFMA2();
}

TEST(TrigDoubleTest, SinCosFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinCosFMA2();
}

TEST(TrigDoubleTest, TanFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_TanFMA2();
}

TEST(TrigDoubleTest, ASinFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ASinFMA2();
}

TEST(TrigDoubleTest, ACosFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ACosFMA2();
}

TEST(TrigDoubleTest, ATanFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATanFMA2();
}

TEST(TrigDoubleTest, ATan2FMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATan2FMA2();
}

#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT,DS_SIMD_FMA)

static void TrigDoubleTest_SinSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d sinAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSIMD4d_store(&sinAngles, dsSinSIMD4d(dsSIMD4d_load(&angles)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&sinAngles, dsSinSIMD4d(dsSIMD4d_set4(-12.34, -0.1234, 0.4321, 2.345)));
	EXPECT_EQ(0.22444221895185537, sinAngles.x);
	EXPECT_EQ(-0.12308705821137626, sinAngles.y);
	EXPECT_EQ(0.41877870990075816, sinAngles.z);
	EXPECT_EQ(0.71497801013649265, sinAngles.w);
#endif
}

static void TrigDoubleTest_CosSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d cosAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSIMD4d_store(&cosAngles, dsCosSIMD4d(dsSIMD4d_load(&angles)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&cosAngles, dsCosSIMD4d(dsSIMD4d_set4(-12.34, -0.1234, 0.4321, 2.345)));
	EXPECT_EQ(0.97448739876509816, cosAngles.x);
	EXPECT_EQ(0.99239587670489104, cosAngles.y);
	EXPECT_EQ(0.90808831736448226, cosAngles.z);
	EXPECT_EQ(-0.69914694093678287, cosAngles.w);
#endif
}

static void TrigDoubleTest_SinCosSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d sinAngles, cosAngles;
	dsSIMD4d simdSin, simdCos;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSinCosSIMD4d(&simdSin, &simdCos, dsSIMD4d_load(&angles));
		dsSIMD4d_store(&sinAngles, simdSin);
		dsSIMD4d_store(&cosAngles, simdCos);
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsSind(angles.values[j]), sinAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
			EXPECT_RELATIVE_EQ(dsCosd(angles.values[j]), cosAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSinCosSIMD4d(&simdSin, &simdCos, dsSIMD4d_set4(-12.34, -0.1234, 0.4321, 2.345));
	dsSIMD4d_store(&sinAngles, simdSin);
	dsSIMD4d_store(&cosAngles, simdCos);
	EXPECT_EQ(0.22444221895185537, sinAngles.x);
	EXPECT_EQ(-0.12308705821137626, sinAngles.y);
	EXPECT_EQ(0.41877870990075816, sinAngles.z);
	EXPECT_EQ(0.71497801013649265, sinAngles.w);

	EXPECT_EQ(0.97448739876509816, cosAngles.x);
	EXPECT_EQ(0.99239587670489104, cosAngles.y);
	EXPECT_EQ(0.90808831736448226, cosAngles.z);
	EXPECT_EQ(-0.69914694093678287, cosAngles.w);
#endif
}

static void TrigDoubleTest_TanSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d tanAngles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d angles;
		for (unsigned int j = 0; j < vecSize; ++j)
			angles.values[j] = randomValue(random, -1000.0, 1000.0);
		dsSIMD4d_store(&tanAngles, dsTanSIMD4d(dsSIMD4d_load(&angles)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsTand(angles.values[j]), tanAngles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&tanAngles, dsTanSIMD4d(dsSIMD4d_set4(-12.34, -0.1234, 0.4321, 2.345)));
	EXPECT_EQ(0.23031823627096235, tanAngles.x);
	EXPECT_EQ(-0.12403019913793806, tanAngles.y);
	EXPECT_EQ(0.46116517732126228, tanAngles.z);
	EXPECT_EQ(-1.0226434076626265, tanAngles.w);
#endif
}

static void TrigDoubleTest_ASinSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		dsSIMD4d_store(&angles, dsASinSIMD4d(dsSIMD4d_load(&x)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsASind(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&angles, dsASinSIMD4d(dsSIMD4d_set4(-0.9876, -0.4321, 0.1234, 0.6789)));
	EXPECT_EQ(-1.4131529841206687, angles.x);
	EXPECT_EQ(-0.44682009105875037, angles.y);
	EXPECT_EQ(0.12371534584255098, angles.z);
	EXPECT_EQ(0.74626342835318737, angles.w);
#endif
}

static void TrigDoubleTest_ACosSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		dsSIMD4d_store(&angles, dsACosSIMD4d(dsSIMD4d_load(&x)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsACosd(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&angles, dsACosSIMD4d(dsSIMD4d_set4(-0.9876, -0.4321, 0.1234, 0.6789)));
	EXPECT_EQ(2.9839493109155653, angles.x);
	EXPECT_EQ(2.0176164178536471, angles.y);
	EXPECT_EQ(1.4470809809523457, angles.z);
	EXPECT_EQ(0.82453289844170918, angles.w);
#endif
}

static void TrigDoubleTest_ATanSIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	dsRandom random;
	dsRandom_seed(&random, 0);

	DS_ALIGN(32) dsVector4d angles;
	for (unsigned int i = 0; i < sampleCount; i += vecSize)
	{
		DS_ALIGN(32) dsVector4d x;
		for (unsigned int j = 0; j < vecSize; ++j)
			x.values[j] = randomValue(random, -1.0, 1.0);
		dsSIMD4d_store(&angles, dsATanSIMD4d(dsSIMD4d_load(&x)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATand(x.values[j]), angles.values[j], 0.0, epsilon) <<
				angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&angles, dsATanSIMD4d(dsSIMD4d_set4(-12.34, -0.1234, 0.4321, 2.345)));
	EXPECT_EQ(-1.4899357456343294, angles.x);
	EXPECT_EQ(-0.12277930094473836, angles.y);
	EXPECT_EQ(0.40786900830223771, angles.z);
	EXPECT_EQ(1.1677072684050145, angles.w);
#endif
}

static void TrigDoubleTest_ATan2SIMD4()
{
	constexpr unsigned int vecSize = 4;
	double epsilon = TrigTypeSelector<double>::epsilon;

	// Test identity values.
	DS_ALIGN(32) dsVector4d angles;
	dsSIMD4d_store(&angles,
		dsATan2SIMD4d(dsSIMD4d_set4(0.0, 0.0, -0.0, -0.0), dsSIMD4d_set4(-1.0, -0.0, 1.0, 0.0)));
	EXPECT_EQ(M_PI, angles.x);
	EXPECT_EQ(M_PI, angles.y);
	EXPECT_EQ(0.0, angles.z);
	EXPECT_EQ(0.0, angles.w);

	dsSIMD4d_store(&angles,
		dsATan2SIMD4d(dsSIMD4d_set4(-1.0, -1.0, 1.0, 1.0), dsSIMD4d_set4(-0.0, 0.0, -0.0, 0.0)));
	EXPECT_EQ(-M_PI_2, angles.x);
	EXPECT_EQ(-M_PI_2, angles.y);
	EXPECT_EQ(M_PI_2, angles.z);
	EXPECT_EQ(M_PI_2, angles.w);

	dsRandom random;
	dsRandom_seed(&random, 0);

	for (unsigned int i = 0; i < sampleCount; ++i)
	{
		DS_ALIGN(32) dsVector4d x, y;
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			x.values[j] = randomValue(random, -10.0, 10.0);
			y.values[j] = randomValue(random, -10.0, 10.0);
		}
		dsSIMD4d_store(&angles, dsATan2SIMD4d(dsSIMD4d_load(&y), dsSIMD4d_load(&x)));
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			EXPECT_RELATIVE_EQ(dsATan2d(y.values[j], x.values[j]),
				angles.values[j], 0.0, epsilon) << angles.values[j];
		}
	}

#if DS_DETERMINISTIC_MATH
	dsSIMD4d_store(&angles, dsATan2SIMD4d(dsSIMD4d_set4(-0.9876, -0.4321, 0.1234, 0.6789),
		dsSIMD4d_set4(-12.34, 0.1234, 0.4321, -2.345)));
	EXPECT_EQ(-3.0617304591858581, angles.x);
	EXPECT_EQ(-1.2926189358619253, angles.y);
	EXPECT_EQ(0.27817739093297111, angles.z);
	EXPECT_EQ(2.8597876524421402, angles.w);
#endif
}

DS_SIMD_END()

TEST(TrigDoubleTest, SinSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinSIMD4();
}

TEST(TrigDoubleTest, CosSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_CosSIMD4();
}

TEST(TrigDoubleTest, SinCosSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_SinCosSIMD4();
}

TEST(TrigDoubleTest, TanSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_TanSIMD4();
}

TEST(TrigDoubleTest, ASinSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ASinSIMD4();
}

TEST(TrigDoubleTest, ACosSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ACosSIMD4();
}

TEST(TrigDoubleTest, ATanSIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATanSIMD4();
}

TEST(TrigDoubleTest, ATan2SIMD4)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigDoubleTest_ATan2SIMD4();
}

#endif // DS_HAS_SIMD

#if DS_PERFORMANCE_TESTS

#if DS_GCC
#pragma GCC push_options
#pragma GCC optimize("no-tree-vectorize")
#endif

#if DS_CLANG
#define DS_NO_VECTORIZE _Pragma("clang loop vectorize(disable)")
#elif DS_MSC
#define DS_NO_VECTORIZE _Pragma("loop(no_vector)")
#else
#define DS_NO_VECTORIZE
#endif

constexpr unsigned int performanceCount = 10000000;

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)
static void TrigFloatTest_PerformanceSIMD4f(std::vector<float>& results,
	std::vector<float>& otherResults, const std::vector<float>& angles, std::size_t& hashValue,
	dsTimer timer)
{
	std::hash<float> hasher;
	printf("\n");

	int64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsSinSIMD4f(simdAngles));
	}
	uint64_t end = dsTimer_currentTicks();
	printf("custom sin SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsCosSIMD4f(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom cos SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f simdSin, simdCos;
		dsSinCosSIMD4f(&simdSin, &simdCos, simdAngles);
		dsSIMD4f_store(results.data() + i, simdSin);
		dsSIMD4f_store(otherResults.data() + i, simdCos);
	}
	end = dsTimer_currentTicks();
	printf("custom sincos SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsTanSIMD4f(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom tan SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(otherResults.data() + i);
		dsSIMD4f_store(results.data() + i, dsASinSIMD4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(otherResults.data() + i);
		dsSIMD4f_store(results.data() + i, dsACosSIMD4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsATanSIMD4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdY = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f simdX = dsSIMD4f_load(angles.data() + performanceCount - i - 4);
		dsSIMD4f_store(results.data() + i, dsATan2SIMD4f(simdY, simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan2 SIMD4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
}
DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT,DS_SIMD_FMA)
static void TrigFloatTest_PerformanceFMA4f(std::vector<float>& results,
	std::vector<float>& otherResults, const std::vector<float>& angles, std::size_t& hashValue,
	dsTimer timer)
{
	std::hash<float> hasher;
	printf("\n");

	int64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsSinFMA4f(simdAngles));
	}
	uint64_t end = dsTimer_currentTicks();
	printf("custom sin FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsCosFMA4f(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom cos FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f simdSin, simdCos;
		dsSinCosFMA4f(&simdSin, &simdCos, simdAngles);
		dsSIMD4f_store(results.data() + i, simdSin);
		dsSIMD4f_store(otherResults.data() + i, simdCos);
	}
	end = dsTimer_currentTicks();
	printf("custom sincos FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdAngles = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsTanFMA4f(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom tan FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(otherResults.data() + i);
		dsSIMD4f_store(results.data() + i, dsASinFMA4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(otherResults.data() + i);
		dsSIMD4f_store(results.data() + i, dsACosFMA4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdX = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f_store(results.data() + i, dsATanFMA4f(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4f simdY = dsSIMD4f_load(angles.data() + i);
		dsSIMD4f simdX = dsSIMD4f_load(angles.data() + performanceCount - i - 4);
		dsSIMD4f_store(results.data() + i, dsATan2FMA4f(simdY, simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan2 FMA4f time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
}
DS_SIMD_END()

#endif // !DS_DETERMINISTIC_MATH
#endif // DS_HAS_SIMD

static void TrigFloatTest_Performance()
{
	std::vector<float> results(performanceCount);
	std::vector<float> otherResults(performanceCount);
	std::vector<float> angles(performanceCount);

	dsRandom random;
	dsRandom_seed(&random, 0);
	for (unsigned int i = 0; i < performanceCount; ++i)
		angles[i] = randomValue(random, -10.0f, 10.0f);

	// Keep a running hash to avoid the optimizer stripping out the loops.
	std::hash<float> hasher;
	std::size_t hashValue = 0;

	dsTimer timer = dsTimer_create();
	uint64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::sin(angles[i]);
	uint64_t end = dsTimer_currentTicks();
	printf("std sin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::cos(angles[i]);
	end = dsTimer_currentTicks();
	printf("std cos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
	{
		results[i] = std::sin(angles[i]);
		otherResults[i] = std::cos(angles[i]);
	}
	end = dsTimer_currentTicks();
	printf("std sincos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::tan(angles[i]);
	end = dsTimer_currentTicks();
	printf("std tan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::asin(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("std asin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::acos(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("std acos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::atan(angles[i]);
	end = dsTimer_currentTicks();
	printf("std atan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::atan2(angles[i], angles[performanceCount - i - 1]);
	end = dsTimer_currentTicks();
	printf("std atan2 time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	printf("\n");

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsSin(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom sin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsCos(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom cos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		dsSinCos(results[i], otherResults[i], angles[i]);
	end = dsTimer_currentTicks();
	printf("custom sincos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsTan(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom tan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsASin(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("custom asin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsACos(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("custom acos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsATan(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom atan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsATan2(angles[i], angles[performanceCount - i - 1]);
	end = dsTimer_currentTicks();
	printf("custom atan2 time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

#if DS_HAS_SIMD

	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_PerformanceSIMD4f(results, otherResults, angles, hashValue, timer);

#if !DS_DETERMINISTIC_MATH

	features |= dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_PerformanceFMA4f(results, otherResults, angles, hashValue, timer);

#endif // !DS_DETERMINISTIC_MATH
#endif // DS_HAS_SIMD

	printf("\nValue to avoid optimizing out loops: 0x%X\n", static_cast<unsigned int>(hashValue));
}

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)
static void TrigFloatTest_PerformanceSIMD2d(std::vector<double>& results,
	std::vector<double>& otherResults, const std::vector<double>& angles, std::size_t& hashValue,
	dsTimer timer)
{
	std::hash<double> hasher;
	printf("\n");

	int64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsSinSIMD2d(simdAngles));
	}
	uint64_t end = dsTimer_currentTicks();
	printf("custom sin SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsCosSIMD2d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom cos SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d simdSin, simdCos;
		dsSinCosSIMD2d(&simdSin, &simdCos, simdAngles);
		dsSIMD2d_store(results.data() + i, simdSin);
		dsSIMD2d_store(otherResults.data() + i, simdCos);
	}
	end = dsTimer_currentTicks();
	printf("custom sincos SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsTanSIMD2d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom tan SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(otherResults.data() + i);
		dsSIMD2d_store(results.data() + i, dsASinSIMD2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(otherResults.data() + i);
		dsSIMD2d_store(results.data() + i, dsACosSIMD2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsATanSIMD2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdY = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d simdX = dsSIMD2d_load(angles.data() + performanceCount - i - 2);
		dsSIMD2d_store(results.data() + i, dsATan2SIMD2d(simdY, simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan2 SIMD2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
}
DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT,DS_SIMD_FMA)
static void TrigFloatTest_PerformanceFMA2d(std::vector<double>& results,
	std::vector<double>& otherResults, const std::vector<double>& angles, std::size_t& hashValue,
	dsTimer timer)
{
	std::hash<double> hasher;
	printf("\n");

	int64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsSinFMA2d(simdAngles));
	}
	uint64_t end = dsTimer_currentTicks();
	printf("custom sin FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsCosFMA2d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom cos FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d simdSin, simdCos;
		dsSinCosFMA2d(&simdSin, &simdCos, simdAngles);
		dsSIMD2d_store(results.data() + i, simdSin);
		dsSIMD2d_store(otherResults.data() + i, simdCos);
	}
	end = dsTimer_currentTicks();
	printf("custom sincos FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdAngles = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsTanFMA2d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom tan FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(otherResults.data() + i);
		dsSIMD2d_store(results.data() + i, dsASinFMA2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(otherResults.data() + i);
		dsSIMD2d_store(results.data() + i, dsACosFMA2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(otherResults.data() + i);
		dsSIMD2d_store(results.data() + i, dsASinFMA2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdX = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d_store(results.data() + i, dsATanFMA2d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 2)
	{
		dsSIMD2d simdY = dsSIMD2d_load(angles.data() + i);
		dsSIMD2d simdX = dsSIMD2d_load(angles.data() + performanceCount - i - 2);
		dsSIMD2d_store(results.data() + i, dsATan2FMA2d(simdY, simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan2 FMA2d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
}
DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT,DS_SIMD_FMA)
static void TrigFloatTest_PerformanceSIMD4d(std::vector<double>& results,
	std::vector<double>& otherResults, const std::vector<double>& angles, std::size_t& hashValue,
	dsTimer timer)
{
	double* resultsData = alignPtr(results.data(), 32);
	double* otherResultsData = alignPtr(otherResults.data(), 32);
	const double* anglesData = alignPtr(angles.data(), 32);

	std::hash<double> hasher;
	printf("\n");

	int64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdAngles = dsSIMD4d_load(anglesData + i);
		dsSIMD4d_store(resultsData + i, dsSinSIMD4d(simdAngles));
	}
	uint64_t end = dsTimer_currentTicks();
	printf("custom sin SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdAngles = dsSIMD4d_load(anglesData + i);
		dsSIMD4d_store(resultsData + i, dsCosSIMD4d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom cos SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdAngles = dsSIMD4d_load(anglesData + i);
		dsSIMD4d simdSin, simdCos;
		dsSinCosSIMD4d(&simdSin, &simdCos, simdAngles);
		dsSIMD4d_store(resultsData + i, simdSin);
		dsSIMD4d_store(otherResultsData + i, simdCos);
	}
	end = dsTimer_currentTicks();
	printf("custom sincos SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdAngles = dsSIMD4d_load(anglesData + i);
		dsSIMD4d_store(resultsData + i, dsTanSIMD4d(simdAngles));
	}
	end = dsTimer_currentTicks();
	printf("custom tan SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdX = dsSIMD4d_load(otherResultsData + i);
		dsSIMD4d_store(resultsData + i, dsASinSIMD4d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom asin SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdX = dsSIMD4d_load(otherResultsData + i);
		dsSIMD4d_store(resultsData + i, dsACosSIMD4d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom acos SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdX = dsSIMD4d_load(anglesData + i);
		dsSIMD4d_store(resultsData + i, dsATanSIMD4d(simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; i += 4)
	{
		dsSIMD4d simdY = dsSIMD4d_load(anglesData + i);
		dsSIMD4d simdX = dsSIMD4d_load(anglesData + performanceCount - i - 4);
		dsSIMD4d_store(resultsData + i, dsATan2SIMD4d(simdY, simdX));
	}
	end = dsTimer_currentTicks();
	printf("custom atan2 SIMD4d time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
}
DS_SIMD_END()
#endif // DS_HAS_SIMD

static void TrigDoubleTest_Performance()
{
	// Add some padding to ensure we can get aligned results.
	unsigned int performanceCountAligned = performanceCount + 3;
	std::vector<double> results(performanceCountAligned);
	std::vector<double> otherResults(performanceCountAligned);
	std::vector<double> angles(performanceCountAligned);

	dsRandom random;
	dsRandom_seed(&random, 0);
	for (unsigned int i = 0; i < performanceCountAligned; ++i)
		angles[i] = randomValue(random, -10.0, 10.0);

	// Keep a running hash to avoid the optimizer stripping out the loops.
	std::hash<double> hasher;
	std::size_t hashValue = 0;

	dsTimer timer = dsTimer_create();
	uint64_t start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::sin(angles[i]);
	uint64_t end = dsTimer_currentTicks();
	printf("std sin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::cos(angles[i]);
	end = dsTimer_currentTicks();
	printf("std cos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
	{
		results[i] = std::sin(angles[i]);
		otherResults[i] = std::cos(angles[i]);
	}
	end = dsTimer_currentTicks();
	printf("std sincos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);
	hashValue += hasher(otherResults[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::tan(angles[i]);
	end = dsTimer_currentTicks();
	printf("std tan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::asin(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("std asin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::acos(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("std acos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::atan(angles[i]);
	end = dsTimer_currentTicks();
	printf("std atan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = std::atan2(angles[i], angles[performanceCount - i - 1]);
	end = dsTimer_currentTicks();
	printf("std atan2 time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	printf("\n");

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsSin(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom sin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsCos(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom cos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		dsSinCos(results[i], otherResults[i], angles[i]);
	end = dsTimer_currentTicks();
	printf("custom sincos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsTan(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom tan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsASin(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("custom asin time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsACos(otherResults[i]);
	end = dsTimer_currentTicks();
	printf("custom acos time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsATan(angles[i]);
	end = dsTimer_currentTicks();
	printf("custom atan time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

	start = dsTimer_currentTicks();
	DS_NO_VECTORIZE
	for (unsigned int i = 0; i < performanceCount; ++i)
		results[i] = dsATan2(angles[i], angles[performanceCount - i - 1]);
	end = dsTimer_currentTicks();
	printf("custom atan2 time: %f s\n", dsTimer_ticksToSeconds(timer, end - start));
	hashValue += hasher(results[size_t(end % performanceCount)]);

#if DS_HAS_SIMD

	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_PerformanceSIMD2d(results, otherResults, angles, hashValue, timer);

#if !DS_DETERMINISTIC_MATH

	features |= dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_PerformanceFMA2d(results, otherResults, angles, hashValue, timer);

#endif // !DS_DETERMINISTIC_MATH

	features = dsSIMDFeatures_Double4 | dsSIMDFeatures_Int;
	if ((dsHostSIMDFeatures & features) == features)
		TrigFloatTest_PerformanceSIMD4d(results, otherResults, angles, hashValue, timer);

#endif // DS_HAS_SIMD

	printf("\nValue to avoid optimizing out loops: 0x%X\n", static_cast<unsigned int>(hashValue));
}

TEST(TrigFloatTest, Performance)
{
	TrigFloatTest_Performance();
}

TEST(TrigDoubleTest, Performance)
{
	TrigDoubleTest_Performance();
}

#if DS_GCC
#pragma GCC pop_options
#endif

#endif // DS_PERFORMANCE_TESTS
