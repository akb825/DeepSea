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

#include <DeepSea/Math/Vector4.h>
#include <gtest/gtest.h>
#include <cmath>

template <typename T>
struct Vector4TypeSelector;

template <>
struct Vector4TypeSelector<float> {typedef dsVector4f Type;};

template <>
struct Vector4TypeSelector<double> {typedef dsVector4d Type;};

template <>
struct Vector4TypeSelector<int> {typedef dsVector4i Type;};

template <typename T>
class Vector4Test : public testing::Test
{
};

using Vector4Types = testing::Types<float, double, int>;
TYPED_TEST_CASE(Vector4Test, Vector4Types);

template <typename T>
class Vector4FloatTest : public Vector4Test<T>
{
};

using Vector4FloatTypes = testing::Types<float, double>;
TYPED_TEST_CASE(Vector4FloatTest, Vector4FloatTypes);

float dsVector4_len(dsVector4f* a)
{
	return dsVector4f_len(a);
}

double dsVector4_len(dsVector4d* a)
{
	return dsVector4d_len(a);
}

float dsVector4_dist(dsVector4f* a, dsVector4f* b)
{
	return dsVector4f_dist(a, b);
}

double dsVector4_dist(dsVector4d* a, dsVector4d* b)
{
	return dsVector4d_dist(a, b);
}

void dsVector4_normalize(dsVector4f* result, dsVector4f* a)
{
	dsVector4f_normalize(result, a);
}

void dsVector4_normalize(dsVector4d* result, dsVector4d* a)
{
	dsVector4d_normalize(result, a);
}

TYPED_TEST(Vector4Test, Initialize)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};

	EXPECT_EQ((TypeParam)-2.3, a.x);
	EXPECT_EQ((TypeParam)4.5, a.y);
	EXPECT_EQ((TypeParam)-6.7, a.z);
	EXPECT_EQ((TypeParam)8.9, a.w);

	EXPECT_EQ((TypeParam)-2.3, a.s);
	EXPECT_EQ((TypeParam)4.5, a.t);
	EXPECT_EQ((TypeParam)-6.7, a.p);
	EXPECT_EQ((TypeParam)8.9, a.q);

	EXPECT_EQ((TypeParam)-2.3, a.r);
	EXPECT_EQ((TypeParam)4.5, a.g);
	EXPECT_EQ((TypeParam)-6.7, a.b);
	EXPECT_EQ((TypeParam)8.9, a.w);

	EXPECT_EQ((TypeParam)-2.3, a.values[0]);
	EXPECT_EQ((TypeParam)4.5, a.values[1]);
	EXPECT_EQ((TypeParam)-6.7, a.values[2]);
	EXPECT_EQ((TypeParam)8.9, a.values[3]);
}

TYPED_TEST(Vector4Test, Add)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};
	Vector4Type result;

	dsVector4_add(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 + (TypeParam)7.6, result.z);
	EXPECT_EQ((TypeParam)8.9 + (TypeParam)-9.8, result.w);
}

TYPED_TEST(Vector4Test, Subtract)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};
	Vector4Type result;

	dsVector4_sub(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 - (TypeParam)7.6, result.z);
	EXPECT_EQ((TypeParam)8.9 - (TypeParam)-9.8, result.w);
}

TYPED_TEST(Vector4Test, Multiply)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};
	Vector4Type result;

	dsVector4_mul(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)7.6, result.z);
	EXPECT_EQ((TypeParam)8.9 * (TypeParam)-9.8, result.w);
}

TYPED_TEST(Vector4Test, Divide)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};
	Vector4Type result;

	dsVector4_div(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 / (TypeParam)7.6, result.z);
	EXPECT_EQ((TypeParam)8.9 / (TypeParam)-9.8, result.w);
}

TYPED_TEST(Vector4Test, Scale)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type result;

	dsVector4_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)3.2, result.z);
	EXPECT_EQ((TypeParam)8.9 * (TypeParam)3.2, result.w);
}

TYPED_TEST(Vector4Test, Dot)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4 +
			  (TypeParam)-6.7*(TypeParam)7.6 +
			  (TypeParam)8.9*(TypeParam)-9.8,
			  dsVector4_dot(a, b));
}

TYPED_TEST(Vector4FloatTest, Length)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};

	EXPECT_EQ(dsPow2((TypeParam)-2.3) +
			  dsPow2((TypeParam)4.5) +
			  dsPow2((TypeParam)-6.7) +
			  dsPow2((TypeParam)8.9),
			  dsVector4_len2(a));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3) +
						dsPow2((TypeParam)4.5) +
						dsPow2((TypeParam)-6.7) +
						dsPow2((TypeParam)8.9)),
			  dsVector4_len(&a));
}

TYPED_TEST(Vector4FloatTest, Distance)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type b = {(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8};

	EXPECT_EQ(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
			  dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
			  dsPow2((TypeParam)-6.7 - (TypeParam)7.6) +
			  dsPow2((TypeParam)8.9 - (TypeParam)-9.8),
			  dsVector4_dist2(a, b));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
						dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
						dsPow2((TypeParam)-6.7 - (TypeParam)7.6) +
						dsPow2((TypeParam)8.9 - (TypeParam)-9.8)),
			  dsVector4_dist(&a, &b));
}

TYPED_TEST(Vector4FloatTest, Normalize)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9};
	Vector4Type result;

	TypeParam length = dsVector4_len(&a);
	dsVector4_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
	EXPECT_EQ((TypeParam)-6.7*(1/length), result.z);
	EXPECT_EQ((TypeParam)8.9*(1/length), result.w);
}
