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

#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>
#include <cmath>

template <typename T>
struct Vector3TypeSelector;

template <>
struct Vector3TypeSelector<float> {typedef dsVector3f Type;};

template <>
struct Vector3TypeSelector<double> {typedef dsVector3d Type;};

template <>
struct Vector3TypeSelector<int> {typedef dsVector3i Type;};

template <typename T>
class Vector3Test : public testing::Test
{
};

using Vector3Types = testing::Types<float, double, int>;
TYPED_TEST_CASE(Vector3Test, Vector3Types);

template <typename T>
class Vector3FloatTest : public Vector3Test<T>
{
};

using Vector3FloatTypes = testing::Types<float, double>;
TYPED_TEST_CASE(Vector3FloatTest, Vector3FloatTypes);

inline float dsVector3_len(dsVector3f* a)
{
	return dsVector3f_len(a);
}

inline double dsVector3_len(dsVector3d* a)
{
	return dsVector3d_len(a);
}

inline float dsVector3_dist(dsVector3f* a, dsVector3f* b)
{
	return dsVector3f_dist(a, b);
}

inline double dsVector3_dist(dsVector3d* a, dsVector3d* b)
{
	return dsVector3d_dist(a, b);
}

inline void dsVector3_normalize(dsVector3f* result, dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

TYPED_TEST(Vector3Test, Initialize)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};

	EXPECT_EQ((TypeParam)-2.3, a.x);
	EXPECT_EQ((TypeParam)4.5, a.y);
	EXPECT_EQ((TypeParam)-6.7, a.z);

	EXPECT_EQ((TypeParam)-2.3, a.s);
	EXPECT_EQ((TypeParam)4.5, a.t);
	EXPECT_EQ((TypeParam)-6.7, a.p);

	EXPECT_EQ((TypeParam)-2.3, a.r);
	EXPECT_EQ((TypeParam)4.5, a.g);
	EXPECT_EQ((TypeParam)-6.7, a.b);

	EXPECT_EQ((TypeParam)-2.3, a.values[0]);
	EXPECT_EQ((TypeParam)4.5, a.values[1]);
	EXPECT_EQ((TypeParam)-6.7, a.values[2]);
}

TYPED_TEST(Vector3Test, Add)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};
	Vector3Type result;

	dsVector3_add(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 + (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Subtract)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};
	Vector3Type result;

	dsVector3_sub(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 - (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Multiply)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};
	Vector3Type result;

	dsVector3_mul(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Divide)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};
	Vector3Type result;

	dsVector3_div(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 / (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Scale)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type result;

	dsVector3_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)3.2, result.z);
}

TYPED_TEST(Vector3Test, Dot)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4 +
			  (TypeParam)-6.7*(TypeParam)7.6,
			  dsVector3_dot(a, b));
}

TYPED_TEST(Vector3Test, Cross)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};
	Vector3Type result;

	dsVector3_cross(result, a, b);
	EXPECT_EQ((TypeParam)4.5*(TypeParam)7.6 - (TypeParam)-5.4*(TypeParam)-6.7, result.x);
	EXPECT_EQ((TypeParam)3.2*(TypeParam)-6.7 - (TypeParam)-2.3*(TypeParam)7.6, result.y);
	EXPECT_EQ((TypeParam)-2.3*(TypeParam)-5.4 - (TypeParam)4.5*(TypeParam)3.2, result.z);

	Vector3Type xAxis = {1, 0, 0};
	Vector3Type yAxis = {0, 1, 0};

	dsVector3_cross(result, xAxis, yAxis);
	EXPECT_EQ(0, result.x);
	EXPECT_EQ(0, result.y);
	EXPECT_EQ(1, result.z);
}

TYPED_TEST(Vector3FloatTest, Length)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};

	EXPECT_EQ(dsPow2((TypeParam)-2.3) +
			  dsPow2((TypeParam)4.5) +
			  dsPow2((TypeParam)-6.7),
			  dsVector3_len2(a));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3) +
						dsPow2((TypeParam)4.5) +
						dsPow2((TypeParam)-6.7)),
			  dsVector3_len(&a));
}

TYPED_TEST(Vector3FloatTest, Distance)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6};

	EXPECT_EQ(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
			  dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
			  dsPow2((TypeParam)-6.7 - (TypeParam)7.6),
			  dsVector3_dist2(a, b));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
						dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
						dsPow2((TypeParam)-6.7 - (TypeParam)7.6)),
			  dsVector3_dist(&a, &b));
}

TYPED_TEST(Vector3FloatTest, Normalize)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7};
	Vector3Type result;

	TypeParam length = dsVector3_len(&a);
	dsVector3_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
	EXPECT_EQ((TypeParam)-6.7*(1/length), result.z);
}
