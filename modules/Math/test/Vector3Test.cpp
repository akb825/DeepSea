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

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

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
TYPED_TEST_SUITE(Vector3Test, Vector3Types);

template <typename T>
class Vector3FloatTest : public Vector3Test<T>
{
};

using Vector3FloatTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(Vector3FloatTest, Vector3FloatTypes);

inline float dsVector3_len(dsVector3f* a)
{
	return dsVector3f_len(a);
}

inline double dsVector3_len(dsVector3d* a)
{
	return dsVector3d_len(a);
}

inline double dsVector3_len(dsVector3i* a)
{
	return dsVector3i_len(a);
}

inline float dsVector3_dist(dsVector3f* a, dsVector3f* b)
{
	return dsVector3f_dist(a, b);
}

inline double dsVector3_dist(dsVector3d* a, dsVector3d* b)
{
	return dsVector3d_dist(a, b);
}

inline double dsVector3_dist(dsVector3i* a, dsVector3i* b)
{
	return dsVector3i_dist(a, b);
}

inline void dsVector3_normalize(dsVector3f* result, dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

inline bool dsVector3_epsilonEqual(const dsVector3f* a, const dsVector3f* b, float epsilon)
{
	return dsVector3f_epsilonEqual(a, b, epsilon);
}

inline bool dsVector3_epsilonEqual(const dsVector3d* a, const dsVector3d* b, double epsilon)
{
	return dsVector3d_epsilonEqual(a, b, epsilon);
}

TYPED_TEST(Vector3Test, Initialize)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};

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

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_add(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 + (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Subtract)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_sub(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 - (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Multiply)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_mul(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Divide)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_div(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
	EXPECT_EQ((TypeParam)-6.7 / (TypeParam)7.6, result.z);
}

TYPED_TEST(Vector3Test, Scale)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type result;

	dsVector3_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
	EXPECT_EQ((TypeParam)-6.7 * (TypeParam)3.2, result.z);
}

TYPED_TEST(Vector3Test, Neg)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type result;

	dsVector3_neg(result, a);
	EXPECT_EQ(-a.x, result.x);
	EXPECT_EQ(-a.y, result.y);
	EXPECT_EQ(-a.z, result.z);
}

TYPED_TEST(Vector3Test, Dot)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4 +
			  (TypeParam)-6.7*(TypeParam)7.6,
			  dsVector3_dot(a, b));
}

TYPED_TEST(Vector3Test, Cross)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_cross(result, a, b);
	EXPECT_EQ((TypeParam)4.5*(TypeParam)7.6 - (TypeParam)-5.4*(TypeParam)-6.7, result.x);
	EXPECT_EQ((TypeParam)3.2*(TypeParam)-6.7 - (TypeParam)-2.3*(TypeParam)7.6, result.y);
	EXPECT_EQ((TypeParam)-2.3*(TypeParam)-5.4 - (TypeParam)4.5*(TypeParam)3.2, result.z);

	Vector3Type xAxis = {{1, 0, 0}};
	Vector3Type yAxis = {{0, 1, 0}};

	dsVector3_cross(result, xAxis, yAxis);
	EXPECT_EQ((TypeParam)0, result.x);
	EXPECT_EQ((TypeParam)0, result.y);
	EXPECT_EQ((TypeParam)1, result.z);
}

TYPED_TEST(Vector3Test, Length)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3) +
			  dsPow2((TypeParam)4.5) +
			  dsPow2((TypeParam)-6.7),
			  dsVector3_len2(a));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3) +
						dsPow2((TypeParam)4.5) +
						dsPow2((TypeParam)-6.7)),
			  dsVector3_len(&a));
}

TYPED_TEST(Vector3Test, Distance)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
			  dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
			  dsPow2((TypeParam)-6.7 - (TypeParam)7.6),
			  dsVector3_dist2(a, b));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
						dsPow2((TypeParam)4.5 - (TypeParam)-5.4) +
						dsPow2((TypeParam)-6.7 - (TypeParam)7.6)),
			  dsVector3_dist(&a, &b));
}

TYPED_TEST(Vector3Test, Equal)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type c = {{(TypeParam)-2.3, (TypeParam)-4.5, (TypeParam)-6.7}};
	Vector3Type d = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)6.7}};

	EXPECT_TRUE(dsVector3_equal(a, a));
	EXPECT_FALSE(dsVector3_equal(a, b));
	EXPECT_FALSE(dsVector3_equal(a, c));
	EXPECT_FALSE(dsVector3_equal(a, d));
}

TEST(Vector3IntTest, Lerp)
{
	dsVector3i a = {{-2, 4, -6}};
	dsVector3i b = {{3, -5, 7}};
	dsVector3i result;

	dsVector3i_lerp(&result, &a, &b, 0.3f);
	EXPECT_EQ(0, result.x);
	EXPECT_EQ(1, result.y);
	EXPECT_EQ(-2, result.z);
}

TYPED_TEST(Vector3FloatTest, Lerp)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector3Type result;

	dsVector3_lerp(result, a, b, (TypeParam)0.3);
	EXPECT_EQ(dsLerp(a.x, b.x, (TypeParam)0.3), result.x);
	EXPECT_EQ(dsLerp(a.y, b.y, (TypeParam)0.3), result.y);
	EXPECT_EQ(dsLerp(a.z, b.z, (TypeParam)0.3), result.z);
}

TYPED_TEST(Vector3FloatTest, Normalize)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type result;

	TypeParam length = dsVector3_len(&a);
	dsVector3_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
	EXPECT_EQ((TypeParam)-6.7*(1/length), result.z);
}

TYPED_TEST(Vector3FloatTest, EpsilonEqual)
{
	typedef typename Vector3TypeSelector<TypeParam>::Type Vector3Type;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector3Type a = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type b = {{(TypeParam)-2.3001, (TypeParam)4.5001, (TypeParam)-6.7001}};
	Vector3Type c = {{(TypeParam)-2.31, (TypeParam)4.5, (TypeParam)-6.7}};
	Vector3Type d = {{(TypeParam)-2.3, (TypeParam)4.51, (TypeParam)-6.7}};
	Vector3Type e = {{(TypeParam)-2.3, (TypeParam)4.5, (TypeParam)-6.71}};

	EXPECT_TRUE(dsVector3_epsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector3_epsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector3_epsilonEqual(&a, &d, epsilon));
	EXPECT_FALSE(dsVector3_epsilonEqual(&a, &e, epsilon));
}

TEST(Vector3, ConvertFloatToDouble)
{
	dsVector3f vectorf = {{-2.3f, 4.5f, -6.7f}};

	dsVector3d vectord;
	dsConvertFloatToDouble(vectord, vectorf);

	EXPECT_FLOAT_EQ(vectorf.x, (float)vectord.x);
	EXPECT_FLOAT_EQ(vectorf.y, (float)vectord.y);
	EXPECT_FLOAT_EQ(vectorf.z, (float)vectord.z);
}

TEST(Vector3, ConvertDoubleToFloat)
{
	dsVector3d vectord = {{-2.3, 4.5, -6.7}};

	dsVector3f vectorf;
	dsConvertDoubleToFloat(vectorf, vectord);

	EXPECT_FLOAT_EQ((float)vectord.x, vectorf.x);
	EXPECT_FLOAT_EQ((float)vectord.y, vectorf.y);
	EXPECT_FLOAT_EQ((float)vectord.z, vectorf.z);
}

TEST(Vector3, ConvertFloatToInt)
{
	dsVector3f vectorf = {{-2, 3, -4}};

	dsVector3i vectori;
	dsConvertFloatToInt(vectori, vectorf);

	EXPECT_EQ(vectorf.x, (float)vectori.x);
	EXPECT_EQ(vectorf.y, (float)vectori.y);
	EXPECT_EQ(vectorf.z, (float)vectori.z);
}

TEST(Vector3, ConvertIntToFloat)
{
	dsVector3i vectori = {{-2, 3, -4}};

	dsVector3f vectorf;
	dsConvertIntToFloat(vectorf, vectori);

	EXPECT_EQ(vectori.x, (int)vectorf.x);
	EXPECT_EQ(vectori.y, (int)vectorf.y);
	EXPECT_EQ(vectori.z, (int)vectorf.z);
}

TEST(Vector3, ConvertDoubleToInt)
{
	dsVector3d vectord = {{-2, 3, -4}};

	dsVector3i vectori;
	dsConvertDoubleToInt(vectori, vectord);

	EXPECT_EQ(vectord.x, vectori.x);
	EXPECT_EQ(vectord.y, vectori.y);
	EXPECT_EQ(vectord.z, vectori.z);
}

TEST(Vector3, ConvertIntToDouble)
{
	dsVector3i vectori = {{-2, 3, -4}};

	dsVector3d vectord;
	dsConvertIntToDouble(vectord, vectori);

	EXPECT_EQ(vectori.x, vectord.x);
	EXPECT_EQ(vectori.y, vectord.y);
	EXPECT_EQ(vectori.z, vectord.z);
}
