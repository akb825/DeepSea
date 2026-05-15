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
#include <DeepSea/Math/Vector3x.h>
#include <gtest/gtest.h>
#include <cmath>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Vector3xTypeSelector;

template <>
struct Vector3xTypeSelector<float>
{
	typedef dsVector3xf Type;
	static const float epsilon;
};

template <>
struct Vector3xTypeSelector<double>
{
	typedef dsVector3xd Type;
	static const double epsilon;
};

const float Vector3xTypeSelector<float>::epsilon = 2e-6f;
const double Vector3xTypeSelector<double>::epsilon = 1e-14f;

template <typename T>
class Vector3xTest : public testing::Test
{
};

using Vector3xTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(Vector3xTest, Vector3xTypes);

inline void dsVector3x_add(dsVector4f* result, const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_add(result, a, b);
}

inline void dsVector3x_add(dsVector4d* result, const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_add(result, a, b);
}

inline void dsVector3x_sub(dsVector4f* result, const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_sub(result, a, b);
}

inline void dsVector3x_sub(dsVector4d* result, const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_sub(result, a, b);
}

inline void dsVector3x_mul(dsVector4f* result, const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_mul(result, a, b);
}

inline void dsVector3x_mul(dsVector4d* result, const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_mul(result, a, b);
}

inline void dsVector3x_div(dsVector4f* result, const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_div(result, a, b);
}

inline void dsVector3x_div(dsVector4d* result, const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_div(result, a, b);
}

inline void dsVector3x_scale(dsVector4f* result, const dsVector4f* a, float s)
{
	return dsVector3xf_scale(result, a, s);
}

inline void dsVector3x_scale(dsVector4d* result, const dsVector4d* a, double s)
{
	return dsVector3xd_scale(result, a, s);
}

inline void dsVector3x_neg(dsVector4f* result, const dsVector4f* a)
{
	dsVector3xf_neg(result, a);
}

inline void dsVector3x_neg(dsVector4d* result, const dsVector4d* a)
{
	dsVector3xd_neg(result, a);
}

inline void dsVector3x_lerp(dsVector4f* result, const dsVector4f* a, const dsVector4f* b, float t)
{
	return dsVector3xf_lerp(result, a, b, t);
}

inline void dsVector3x_lerp(dsVector4d* result, const dsVector4d* a, const dsVector4d* b, double t)
{
	return dsVector3xd_lerp(result, a, b, t);
}

inline float dsVector3x_dot(const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_dot(a, b);
}

inline double dsVector3x_dot(const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_dot(a, b);
}

inline void dsVector3x_cross(dsVector4f* result, const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_cross(result, a, b);
}

inline void dsVector3x_cross(dsVector4d* result, const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_cross(result, a, b);
}

inline float dsVector3x_len2(const dsVector4f* a)
{
	return dsVector3xf_len2(a);
}

inline double dsVector3x_len2(const dsVector4d* a)
{
	return dsVector3xd_len2(a);
}

inline float dsVector3x_dist2(const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_dist2(a, b);
}

inline double dsVector3x_dist2(const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_dist2(a, b);
}

inline bool dsVector3x_equal(const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_equal(a, b);
}

inline bool dsVector3x_equal(const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_equal(a, b);
}

inline float dsVector3x_len(const dsVector4f* a)
{
	return dsVector3xf_len(a);
}

inline double dsVector3x_len(const dsVector4d* a)
{
	return dsVector3xd_len(a);
}

inline float dsVector3x_dist(const dsVector4f* a, const dsVector4f* b)
{
	return dsVector3xf_dist(a, b);
}

inline double dsVector3x_dist(const dsVector4d* a, const dsVector4d* b)
{
	return dsVector3xd_dist(a, b);
}

inline void dsVector3x_normalize(dsVector4f* result, const dsVector4f* a)
{
	dsVector3xf_normalize(result, a);
}

inline void dsVector3x_normalize(dsVector4d* result, const dsVector4d* a)
{
	dsVector3xd_normalize(result, a);
}

inline bool dsVector3x_epsilonEqual(const dsVector4f* a, const dsVector4f* b, float epsilon)
{
	return dsVector3xf_epsilonEqual(a, b, epsilon);
}

inline bool dsVector3x_epsilonEqual(const dsVector4d* a, const dsVector4d* b, double epsilon)
{
	return dsVector3xd_epsilonEqual(a, b, epsilon);
}

inline bool dsVector3x_relativeEpsilonEqual(
	const dsVector4f* a, const dsVector4f* b, float absoluteEps, float relativeEps)
{
	return dsVector3xf_relativeEpsilonEqual(a, b, absoluteEps, relativeEps);
}

inline bool dsVector3x_relativeEpsilonEqual(
	const dsVector4d* a, const dsVector4d* b, double absoluteEps, double relativeEps)
{
	return dsVector3xd_relativeEpsilonEqual(a, b, absoluteEps, relativeEps);
}

TYPED_TEST(Vector3xTest, Add)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_add(&result, &a, &b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 + (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3xTest, Subtract)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_sub(&result, &a, &b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 - (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3xTest, Multiply)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_mul(&result, &a, &b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3xTest, Divide)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_div(&result, &a, &b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 / (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3xTest, Scale)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType result;

	dsVector3x_scale(&result, &a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)3.2, result.z);
}

TYPED_TEST(Vector3xTest, Neg)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType result;

	dsVector3x_neg(&result, &a);
	EXPECT_EQ(-a.x, result.x);
	EXPECT_EQ(-a.y, result.y);
	EXPECT_EQ(-a.z, result.z);
}

TYPED_TEST(Vector3xTest, Dot)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 + (TypeParam)4.5*(TypeParam)-5.4 +
		(TypeParam)-6.7*(TypeParam)7.6, dsVector3x_dot(&a, &b));
}

TYPED_TEST(Vector3xTest, Cross)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;
	TypeParam epsilon = Vector3xTypeSelector<TypeParam>::epsilon;
	DS_UNUSED(epsilon);

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_cross(&result, &a, &b);
	EXPECT_EQ_DETERMINISTIC(a.y*b.z - b.y*a.z, result.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(b.x*a.z - a.x*b.z, result.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(a.x*b.y - b.x*a.y, result.z, epsilon);

	Vector3xType xAxis = {{1, 0, 0, 1}};
	Vector3xType yAxis = {{0, 1, 0, 2}};

	dsVector3x_cross(&result, &xAxis, &yAxis);
	EXPECT_EQ((TypeParam)0, result.x);
	EXPECT_EQ((TypeParam)0, result.y);
	EXPECT_EQ((TypeParam)1, result.z);
}

TYPED_TEST(Vector3xTest, Length)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;
	TypeParam epsilon = Vector3xTypeSelector<TypeParam>::epsilon;
	DS_UNUSED(epsilon);

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};

	EXPECT_EQ(dsPow2(a.x) + dsPow2(a.y) + dsPow2(a.z), dsVector3_len2(a));
	EXPECT_EQ_DETERMINISTIC(
		std::sqrt(dsPow2(a.x) + dsPow2(a.y) + dsPow2(a.z)), dsVector3x_len(&a), epsilon);
}

TYPED_TEST(Vector3xTest, Distance)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};

	EXPECT_EQ(dsPow2(a.x - b.x) + dsPow2(a.y - b.y) + dsPow2(a.z - b.z), dsVector3_dist2(a, b));
	EXPECT_DOUBLE_EQ(std::sqrt(dsPow2(a.x - b.x) + dsPow2(a.y - b.y) + dsPow2(a.z - b.z)),
		dsVector3x_dist(&a, &b));
}

TYPED_TEST(Vector3xTest, Equal)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3xType b = {{(TypeParam)2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3xType c = {{(TypeParam)-2.3, (TypeParam)-4.5, (TypeParam)-6.7}};
	Vector3xType d = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)6.7}};

	EXPECT_TRUE(dsVector3x_equal(&a, &a));
	EXPECT_FALSE(dsVector3x_equal(&a, &b));
	EXPECT_FALSE(dsVector3x_equal(&a, &c));
	EXPECT_FALSE(dsVector3x_equal(&a, &d));
}

TYPED_TEST(Vector3xTest, Lerp)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;
	TypeParam epsilon = Vector3xTypeSelector<TypeParam>::epsilon;
	DS_UNUSED(epsilon);

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, 2}};
	Vector3xType result;

	dsVector3x_lerp(&result, &a, &b, (TypeParam)0.3);
	EXPECT_EQ_DETERMINISTIC(dsLerp(a.x, b.x, (TypeParam)0.3), result.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(dsLerp(a.y, b.y, (TypeParam)0.3), result.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(dsLerp(a.z, b.z, (TypeParam)0.3), result.z, epsilon);
}

TYPED_TEST(Vector3xTest, Normalize)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType result;

	TypeParam length = dsVector3x_len(&a);
	dsVector3x_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
	EXPECT_EQ((TypeParam)-6.7*(1/length), result.z);
}

TYPED_TEST(Vector3xTest, EpsilonEqual)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector3xType a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, 1}};
	Vector3xType b = {{(TypeParam)-2.3001, (TypeParam)4.5001, (TypeParam)-6.7001, 2}};
	Vector3xType c = {{(TypeParam)-2.31, (TypeParam)4.5, (TypeParam)-6.7, 3}};
	Vector3xType d = {{(TypeParam)-2.3, (TypeParam)4.51, (TypeParam)-6.7, 4}};
	Vector3xType e = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.71, 5}};

	EXPECT_TRUE(dsVector3x_epsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector3x_epsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector3x_epsilonEqual(&a, &d, epsilon));
	EXPECT_FALSE(dsVector3x_epsilonEqual(&a, &e, epsilon));
}

TYPED_TEST(Vector3xTest, RelativeEpsilonEqual)
{
	typedef typename Vector3xTypeSelector<TypeParam>::Type Vector3xType;
	TypeParam smallEpsilon = (TypeParam)1e-5;
	TypeParam mediumEpsilon = (TypeParam)1e-4;
	TypeParam largeEpsilon = (TypeParam)1.001e-3;

	Vector3xType a = {{(TypeParam)-23.0, (TypeParam)45.0, (TypeParam)-67.0, 1}};
	Vector3xType b = {{(TypeParam)-23.001, (TypeParam)45.001, (TypeParam)-67.001, 2}};
	Vector3xType c = {{(TypeParam)-23.1, (TypeParam)45.0, (TypeParam)-67.0, 3}};
	Vector3xType d = {{(TypeParam)-23.0, (TypeParam)45.1, (TypeParam)-67.0, 4}};
	Vector3xType e = {{(TypeParam)-23.0, (TypeParam)45.0, (TypeParam)-67.1, 5}};

	EXPECT_FALSE(dsVector3x_relativeEpsilonEqual(&a, &b, smallEpsilon, smallEpsilon));
	EXPECT_TRUE(dsVector3x_relativeEpsilonEqual(&a, &b, largeEpsilon, smallEpsilon));
	EXPECT_TRUE(dsVector3x_relativeEpsilonEqual(&a, &b, smallEpsilon, mediumEpsilon));
	EXPECT_FALSE(dsVector3x_relativeEpsilonEqual(&a, &c, largeEpsilon, largeEpsilon));
	EXPECT_FALSE(dsVector3x_relativeEpsilonEqual(&a, &d, largeEpsilon, largeEpsilon));
	EXPECT_FALSE(dsVector3x_relativeEpsilonEqual(&a, &e, largeEpsilon, largeEpsilon));
}
