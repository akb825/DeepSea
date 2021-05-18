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

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

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
TYPED_TEST_SUITE(Vector4Test, Vector4Types);

template <typename T>
class Vector4FloatTest : public Vector4Test<T>
{
};

using Vector4FloatTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(Vector4FloatTest, Vector4FloatTypes);

inline float dsVector4_len(dsVector4f* a)
{
	return dsVector4f_len(a);
}

inline double dsVector4_len(dsVector4d* a)
{
	return dsVector4d_len(a);
}

inline double dsVector4_len(dsVector4i* a)
{
	return dsVector4i_len(a);
}

inline float dsVector4_dist(dsVector4f* a, dsVector4f* b)
{
	return dsVector4f_dist(a, b);
}

inline double dsVector4_dist(dsVector4d* a, dsVector4d* b)
{
	return dsVector4d_dist(a, b);
}

inline double dsVector4_dist(dsVector4i* a, dsVector4i* b)
{
	return dsVector4i_dist(a, b);
}

inline void dsVector4_normalize(dsVector4f* result, dsVector4f* a)
{
	dsVector4f_normalize(result, a);
}

inline void dsVector4_normalize(dsVector4d* result, dsVector4d* a)
{
	dsVector4d_normalize(result, a);
}

inline bool dsVector4_epsilonEqual(const dsVector4f* a, const dsVector4f* b, float epsilon)
{
	return dsVector4f_epsilonEqual(a, b, epsilon);
}

inline bool dsVector4_epsilonEqual(const dsVector4d* a, const dsVector4d* b, double epsilon)
{
	return dsVector4d_epsilonEqual(a, b, epsilon);
}

inline bool dsVector4_relativeEpsilonEqual(const dsVector4f* a, const dsVector4f* b, float epsilon)
{
	return dsVector4f_relativeEpsilonEqual(a, b, epsilon);
}

inline bool dsVector4_relativeEpsilonEqual(const dsVector4d* a, const dsVector4d* b, double epsilon)
{
	return dsVector4d_relativeEpsilonEqual(a, b, epsilon);
}

TYPED_TEST(Vector4Test, Initialize)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};

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

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};
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

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};
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

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};
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

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};
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

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type result;

	dsVector4_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)3.2, result.z);
	EXPECT_EQ((TypeParam)8.9 * (TypeParam)3.2, result.w);
}

TYPED_TEST(Vector4Test, Neg)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type result;

	dsVector4_neg(result, a);
	EXPECT_EQ(-a.x, result.x);
	EXPECT_EQ(-a.y, result.y);
	EXPECT_EQ(-a.z, result.z);
	EXPECT_EQ(-a.w, result.w);
}

TYPED_TEST(Vector4Test, Dot)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4 +
			  (TypeParam)-6.7*(TypeParam)7.6 +
			  (TypeParam)8.9*(TypeParam)-9.8,
			  dsVector4_dot(a, b));
}

TYPED_TEST(Vector4Test, Length)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};

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

TYPED_TEST(Vector4Test, Distance)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};

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

TYPED_TEST(Vector4Test, Equal)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type c = {{(TypeParam)-2.3, (TypeParam)-4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type d = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)6.7, (TypeParam)8.9}};
	Vector4Type e = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9}};

	EXPECT_TRUE(dsVector4_equal(a, a));
	EXPECT_FALSE(dsVector4_equal(a, b));
	EXPECT_FALSE(dsVector4_equal(a, c));
	EXPECT_FALSE(dsVector4_equal(a, d));
	EXPECT_FALSE(dsVector4_equal(a, e));
}

TEST(Vector4IntTest, Lerp)
{
	dsVector4i a = {{-2, 4, -6, 8}};
	dsVector4i b = {{3, -5, 7, -9}};
	dsVector4i result;

	dsVector4i_lerp(&result, &a, &b, 0.3f);
	EXPECT_EQ(0, result.x);
	EXPECT_EQ(1, result.y);
	EXPECT_EQ(-2, result.z);
	EXPECT_EQ(2, result.w);
}

TYPED_TEST(Vector4FloatTest, Lerp)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6, (TypeParam)-9.8}};
	Vector4Type result;

	dsVector4_lerp(result, a, b, (TypeParam)0.3);
	EXPECT_EQ(dsLerp(a.x, b.x, (TypeParam)0.3), result.x);
	EXPECT_EQ(dsLerp(a.y, b.y, (TypeParam)0.3), result.y);
	EXPECT_EQ(dsLerp(a.z, b.z, (TypeParam)0.3), result.z);
	EXPECT_EQ(dsLerp(a.w, b.w, (TypeParam)0.3), result.w);
}

TYPED_TEST(Vector4FloatTest, Normalize)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type result;

	TypeParam length = dsVector4_len(&a);
	dsVector4_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
	EXPECT_EQ((TypeParam)-6.7*(1/length), result.z);
	EXPECT_EQ((TypeParam)8.9*(1/length), result.w);
}

TYPED_TEST(Vector4FloatTest, EpsilonEqual)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector4Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type b = {{(TypeParam)-2.3001, (TypeParam)4.5001, (TypeParam)-6.7001,
		(TypeParam)8.9001}};
	Vector4Type c = {{(TypeParam)-2.31, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type d = {{(TypeParam)-2.3, (TypeParam)4.51, (TypeParam)-6.7, (TypeParam)8.9}};
	Vector4Type e = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.71, (TypeParam)8.9}};
	Vector4Type f = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7, (TypeParam)8.91}};

	EXPECT_TRUE(dsVector4_epsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector4_epsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector4_epsilonEqual(&a, &d, epsilon));
	EXPECT_FALSE(dsVector4_epsilonEqual(&a, &e, epsilon));
	EXPECT_FALSE(dsVector4_epsilonEqual(&a, &f, epsilon));
}

TYPED_TEST(Vector4FloatTest, RelativeEpsilonEqual)
{
	typedef typename Vector4TypeSelector<TypeParam>::Type Vector4Type;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector4Type a = {{(TypeParam)-23.0, (TypeParam)45.0, (TypeParam)-67.0, (TypeParam)89.0}};
	Vector4Type b = {{(TypeParam)-23.001, (TypeParam)45.001, (TypeParam)-67.001,
		(TypeParam)89.001}};
	Vector4Type c = {{(TypeParam)-23.1, (TypeParam)45.0, (TypeParam)-67.0, (TypeParam)89.0}};
	Vector4Type d = {{(TypeParam)-23.0, (TypeParam)45.1, (TypeParam)-67.0, (TypeParam)89.0}};
	Vector4Type e = {{(TypeParam)-23.0, (TypeParam)45.0, (TypeParam)-67.1, (TypeParam)89.0}};
	Vector4Type f = {{(TypeParam)-23.0, (TypeParam)45.0, (TypeParam)-67.0, (TypeParam)89.1}};

	EXPECT_TRUE(dsVector4_relativeEpsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector4_relativeEpsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector4_relativeEpsilonEqual(&a, &d, epsilon));
	EXPECT_FALSE(dsVector4_relativeEpsilonEqual(&a, &e, epsilon));
	EXPECT_FALSE(dsVector4_relativeEpsilonEqual(&a, &f, epsilon));
}

TEST(Vector4, ConvertFloatToDouble)
{
	dsVector4f vectorf = {{-2.3f, 4.5f, -6.7f, 8.9f}};

	dsVector4d vectord;
	dsConvertFloatToDouble(vectord, vectorf);

	EXPECT_FLOAT_EQ(vectorf.x, (float)vectord.x);
	EXPECT_FLOAT_EQ(vectorf.y, (float)vectord.y);
	EXPECT_FLOAT_EQ(vectorf.z, (float)vectord.z);
	EXPECT_FLOAT_EQ(vectorf.w, (float)vectord.w);
}

TEST(Vector4, ConvertDoubleToFloat)
{
	dsVector4d vectord = {{-2.3, 4.5, -6.7, 8.9}};

	dsVector4f vectorf;
	dsConvertDoubleToFloat(vectorf, vectord);

	EXPECT_FLOAT_EQ((float)vectord.x, vectorf.x);
	EXPECT_FLOAT_EQ((float)vectord.y, vectorf.y);
	EXPECT_FLOAT_EQ((float)vectord.z, vectorf.z);
	EXPECT_FLOAT_EQ((float)vectord.w, vectorf.w);
}

TEST(Vector4, ConvertFloatToInt)
{
	dsVector4f vectorf = {{-2, 3, -4, 5}};

	dsVector4i vectori;
	dsConvertFloatToInt(vectori, vectorf);

	EXPECT_EQ(vectorf.x, (float)vectori.x);
	EXPECT_EQ(vectorf.y, (float)vectori.y);
	EXPECT_EQ(vectorf.z, (float)vectori.z);
	EXPECT_EQ(vectorf.w, (float)vectori.w);
}

TEST(Vector4, ConvertIntToFloat)
{
	dsVector4i vectori = {{-2, 3, -4, 5}};

	dsVector4f vectorf;
	dsConvertIntToFloat(vectorf, vectori);

	EXPECT_EQ(vectori.x, (int)vectorf.x);
	EXPECT_EQ(vectori.y, (int)vectorf.y);
	EXPECT_EQ(vectori.z, (int)vectorf.z);
	EXPECT_EQ(vectori.w, (int)vectorf.w);
}

TEST(Vector4, ConvertDoubleToInt)
{
	dsVector4d vectord = {{-2, 3, -4, 5}};

	dsVector4i vectori;
	dsConvertDoubleToInt(vectori, vectord);

	EXPECT_EQ(vectord.x, vectori.x);
	EXPECT_EQ(vectord.y, vectori.y);
	EXPECT_EQ(vectord.z, vectori.z);
	EXPECT_EQ(vectord.w, vectori.w);
}

TEST(Vector4, ConvertIntToDouble)
{
	dsVector4i vectori = {{-2, 3, -4, 5}};

	dsVector4d vectord;
	dsConvertIntToDouble(vectord, vectori);

	EXPECT_EQ(vectori.x, vectord.x);
	EXPECT_EQ(vectori.y, vectord.y);
	EXPECT_EQ(vectori.z, vectord.z);
	EXPECT_EQ(vectori.w, vectord.w);
}
