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
#include <DeepSea/Math/SIMD/Dot.h>
#include <DeepSea/Math/Types.h>
#include <gtest/gtest.h>

#if DS_HAS_SIMD

#if !DS_DETERMINISTIC_MATH
static constexpr float epsilonf = 1e-6f;
static constexpr double epsilond = 1e-15;
#endif

DS_SIMD_START(DS_SIMD_FLOAT4)

static void DotTest_Dot4SIMD4f()
{
	dsVector4f a = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector4f b = {{0.98f, 76.54f, -3.21f, -123.456f}};
	dsVector4f result;

	result.simd = dsDot4SIMD4f(a.simd, b.simd);
	float resultScalar = (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0f, epsilonf);
}

static void DotTest_Dot3SIMD4f()
{
	dsVector4f a = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector4f b = {{0.98f, 76.54f, -3.21f, -123.456f}};
	dsVector4f result;

	result.simd = dsDot3SIMD4f(a.simd, b.simd);
	float resultScalar = (a.x*b.x + a.y*b.y) + a.z*b.z;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0f, epsilonf);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

static void DotTest_Dot4FMA4f()
{
	dsVector4f a = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector4f b = {{0.98f, 76.54f, -3.21f, -123.456f}};
	dsVector4f result;

	result.simd = dsDot4FMA4f(a.simd, b.simd);
	float resultScalar = (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0f, epsilonf);
}

static void DotTest_Dot3FMA4f()
{
	dsVector4f a = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector4f b = {{0.98f, 76.54f, -3.21f, -123.456f}};
	dsVector4f result;

	result.simd = dsDot3FMA4f(a.simd, b.simd);
	float resultScalar = (a.x*b.x + a.y*b.y) + a.z*b.z;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0f, epsilonf);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0f, epsilonf);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2)

static void DotTest_Dot4SIMD2d()
{
	dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	dsVector2d result;

	result.simd = dsDot4SIMD2d(a.simd2[0], a.simd2[1], b.simd2[0], b.simd2[1]);
	double resultScalar = (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

static void DotTest_Dot3SIMD2d()
{
	dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	dsVector2d result;

	result.simd = dsDot3SIMD2d(a.simd2[0], a.simd2[1], b.simd2[0], b.simd2[1]);
	double resultScalar = (a.x*b.x + a.y*b.y) + a.z*b.z;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

static void DotTest_Dot2SIMD2d()
{
	dsVector2d a = {{1.2, -3.4}};
	dsVector2d b = {{0.98, 76.54}};
	dsVector2d result;

	result.simd = dsDot2SIMD2d(a.simd, b.simd);
	double resultScalar = a.x*b.x + a.y*b.y;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

static void DotTest_Dot4FMA2d()
{
	dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	dsVector2d result;

	result.simd = dsDot4FMA2d(a.simd2[0], a.simd2[1], b.simd2[0], b.simd2[1]);
	double resultScalar = (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

static void DotTest_Dot3FMA2d()
{
	dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	dsVector2d result;

	result.simd = dsDot3FMA2d(a.simd2[0], a.simd2[1], b.simd2[0], b.simd2[1]);
	double resultScalar = (a.x*b.x + a.y*b.y) + a.z*b.z;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

static void DotTest_Dot2FMA2d()
{
	dsVector2d a = {{1.2, -3.4}};
	dsVector2d b = {{0.98, 76.54}};
	dsVector2d result;

	result.simd = dsDot2FMA2d(a.simd, b.simd);
	double resultScalar = a.x*b.x + a.y*b.y;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4)

static void DotTest_Dot4SIMD4d()
{
	DS_ALIGN(32) dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	DS_ALIGN(32) dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	DS_ALIGN(32) dsVector4d result;

	dsSIMD4d_store(&result, dsDot4SIMD4d(dsSIMD4d_load(&a), dsSIMD4d_load(&b)));
	double resultScalar = (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0, epsilond);
}

static void DotTest_Dot3SIMD4d()
{
	DS_ALIGN(32) dsVector4d a = {{1.2, -3.4, 5.6, -7.8}};
	DS_ALIGN(32) dsVector4d b = {{0.98, 76.54, -3.21, -123.456}};
	DS_ALIGN(32) dsVector4d result;

	dsSIMD4d_store(&result, dsDot3SIMD4d(dsSIMD4d_load(&a), dsSIMD4d_load(&b)));
	double resultScalar = (a.x*b.x + a.y*b.y) + a.z*b.z;
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.x, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.y, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.z, 0.0, epsilond);
	EXPECT_RELATIVE_EQ_DETERMINISTIC(resultScalar, result.w, 0.0, epsilond);
}

DS_SIMD_END()

TEST(DotTest, Dot4SIMD4f)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DotTest_Dot4SIMD4f();
}

TEST(DotTest, Dot3SIMD4f)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		DotTest_Dot3SIMD4f();
}

TEST(DotTest, Dot4FMA4f)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		DotTest_Dot4FMA4f();
}

TEST(DotTest, Dot3FMA4f)
{
	dsSIMDFeatures features = dsSIMDFeatures_Float4 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		DotTest_Dot3FMA4f();
}

TEST(DotTest, Dot4SIMD2d)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		DotTest_Dot4SIMD2d();
}

TEST(DotTest, Dot3SIMD2d)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		DotTest_Dot3SIMD2d();
}

TEST(DotTest, Dot2SIMD2d)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		DotTest_Dot2SIMD2d();
}

TEST(DotTest, Dot4FMA2d)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		DotTest_Dot4FMA2d();
}

TEST(DotTest, Dot3FMA2d)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		DotTest_Dot3FMA2d();
}

TEST(DotTest, Dot2FMA2d)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) == features)
		DotTest_Dot2FMA2d();
}

TEST(DotTest, Dot4SIMD4d)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double4)
		DotTest_Dot4SIMD4d();
}

TEST(DotTest, Dot3SIMD4d)
{
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double4)
		DotTest_Dot3SIMD4d();
}

#endif // DS_HAS_SIMD
