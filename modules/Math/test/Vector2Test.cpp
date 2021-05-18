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

#include <DeepSea/Math/Vector2.h>
#include <gtest/gtest.h>
#include <cmath>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Vector2TypeSelector;

template <>
struct Vector2TypeSelector<float> {typedef dsVector2f Type;};

template <>
struct Vector2TypeSelector<double> {typedef dsVector2d Type;};

template <>
struct Vector2TypeSelector<int> {typedef dsVector2i Type;};

template <typename T>
class Vector2Test : public testing::Test
{
};

using Vector2Types = testing::Types<float, double, int>;
TYPED_TEST_SUITE(Vector2Test, Vector2Types);

template <typename T>
class Vector2FloatTest : public Vector2Test<T>
{
};

using Vector2FloatTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(Vector2FloatTest, Vector2FloatTypes);

inline float dsVector2_len(const dsVector2f* a)
{
	return dsVector2f_len(a);
}

inline double dsVector2_len(const dsVector2d* a)
{
	return dsVector2d_len(a);
}

inline double dsVector2_len(const dsVector2i* a)
{
	return dsVector2i_len(a);
}

inline float dsVector2_dist(const dsVector2f* a, const dsVector2f* b)
{
	return dsVector2f_dist(a, b);
}

inline double dsVector2_dist(const dsVector2d* a, const dsVector2d* b)
{
	return dsVector2d_dist(a, b);
}

inline double dsVector2_dist(const dsVector2i* a, const dsVector2i* b)
{
	return dsVector2i_dist(a, b);
}

inline void dsVector2_normalize(dsVector2f* result, const dsVector2f* a)
{
	dsVector2f_normalize(result, a);
}

inline void dsVector2_normalize(dsVector2d* result, const dsVector2d* a)
{
	dsVector2d_normalize(result, a);
}

inline bool dsVector2_epsilonEqual(const dsVector2f* a, const dsVector2f* b, float epsilon)
{
	return dsVector2f_epsilonEqual(a, b, epsilon);
}

inline bool dsVector2_epsilonEqual(const dsVector2d* a, const dsVector2d* b, double epsilon)
{
	return dsVector2d_epsilonEqual(a, b, epsilon);
}

inline bool dsVector2_relativeEpsilonEqual(const dsVector2f* a, const dsVector2f* b, float epsilon)
{
	return dsVector2f_relativeEpsilonEqual(a, b, epsilon);
}

inline bool dsVector2_relativeEpsilonEqual(const dsVector2d* a, const dsVector2d* b, double epsilon)
{
	return dsVector2d_relativeEpsilonEqual(a, b, epsilon);
}

TYPED_TEST(Vector2Test, Initialize)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};

	EXPECT_EQ((TypeParam)-2.3, a.x);
	EXPECT_EQ((TypeParam)4.5, a.y);

	EXPECT_EQ((TypeParam)-2.3, a.s);
	EXPECT_EQ((TypeParam)4.5, a.t);

	EXPECT_EQ((TypeParam)-2.3, a.r);
	EXPECT_EQ((TypeParam)4.5, a.g);

	EXPECT_EQ((TypeParam)-2.3, a.values[0]);
	EXPECT_EQ((TypeParam)4.5, a.values[1]);
}

TYPED_TEST(Vector2Test, Add)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_add(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Subtract)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_sub(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Multiply)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_mul(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Divide)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_div(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Scale)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	dsVector2_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
}

TYPED_TEST(Vector2Test, Neg)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	dsVector2_neg(result, a);
	EXPECT_EQ(-a.x, result.x);
	EXPECT_EQ(-a.y, result.y);
}

TYPED_TEST(Vector2Test, Dot)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4,
			  dsVector2_dot(a, b));
}

TYPED_TEST(Vector2Test, Length)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3) +
			  dsPow2((TypeParam)4.5),
			  dsVector2_len2(a));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3) +
						dsPow2((TypeParam)4.5)),
			  dsVector2_len(&a));
}

TYPED_TEST(Vector2Test, Distance)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
			  dsPow2((TypeParam)4.5 - (TypeParam)-5.4),
			  dsVector2_dist2(a, b));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
						dsPow2((TypeParam)4.5 - (TypeParam)-5.4)),
			  dsVector2_dist(&a, &b));
}

TYPED_TEST(Vector2Test, Equal)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)2.3, (TypeParam)4.5}};
	Vector2Type c = {{(TypeParam)-2.3, (TypeParam)-4.5}};

	EXPECT_TRUE(dsVector2_equal(a, a));
	EXPECT_FALSE(dsVector2_equal(a, b));
	EXPECT_FALSE(dsVector2_equal(a, c));
}

TEST(Vector2IntTest, Lerp)
{
	dsVector2i a = {{-2, 4}};
	dsVector2i b = {{3, -5}};
	dsVector2i result;

	dsVector2i_lerp(&result, &a, &b, 0.3f);
	EXPECT_EQ(0, result.x);
	EXPECT_EQ(1, result.y);
}

TYPED_TEST(Vector2FloatTest, Lerp)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_lerp(result, a, b, (TypeParam)0.3);
	EXPECT_EQ(dsLerp(a.x, b.x, (TypeParam)0.3), result.x);
	EXPECT_EQ(dsLerp(a.y, b.y, (TypeParam)0.3), result.y);
}

TYPED_TEST(Vector2FloatTest, Normalize)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	TypeParam length = dsVector2_len(&a);
	dsVector2_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
}

TYPED_TEST(Vector2FloatTest, EpsilonEqual)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)-2.3001, (TypeParam)4.5001}};
	Vector2Type c = {{(TypeParam)-2.31, (TypeParam)4.5}};
	Vector2Type d = {{(TypeParam)-2.3, (TypeParam)4.51}};

	EXPECT_TRUE(dsVector2_epsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector2_epsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector2_epsilonEqual(&a, &d, epsilon));
}

TYPED_TEST(Vector2FloatTest, RelativeEpsilonEqual)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;
	TypeParam epsilon = (TypeParam)1e-3;

	Vector2Type a = {{(TypeParam)-23.0, (TypeParam)45.0}};
	Vector2Type b = {{(TypeParam)-23.001, (TypeParam)45.001}};
	Vector2Type c = {{(TypeParam)-23.1, (TypeParam)45.0}};
	Vector2Type d = {{(TypeParam)-23.0, (TypeParam)45.1}};

	EXPECT_TRUE(dsVector2_relativeEpsilonEqual(&a, &b, epsilon));
	EXPECT_FALSE(dsVector2_relativeEpsilonEqual(&a, &c, epsilon));
	EXPECT_FALSE(dsVector2_relativeEpsilonEqual(&a, &d, epsilon));
}

TEST(Vector2, ConvertFloatToDouble)
{
	dsVector2f vectorf = {{-2.3f, 4.5f}};

	dsVector2d vectord;
	dsConvertFloatToDouble(vectord, vectorf);

	EXPECT_FLOAT_EQ(vectorf.x, (float)vectord.x);
	EXPECT_FLOAT_EQ(vectorf.y, (float)vectord.y);
}

TEST(Vector2, ConvertDoubleToFloat)
{
	dsVector2d vectord = {{-2.3, 4.5}};

	dsVector2f vectorf;
	dsConvertDoubleToFloat(vectorf, vectord);

	EXPECT_FLOAT_EQ((float)vectord.x, vectorf.x);
	EXPECT_FLOAT_EQ((float)vectord.y, vectorf.y);
}

TEST(Vector2, ConvertFloatToInt)
{
	dsVector2f vectorf = {{-2, 3}};

	dsVector2i vectori;
	dsConvertFloatToInt(vectori, vectorf);

	EXPECT_EQ(vectorf.x, (float)vectori.x);
	EXPECT_EQ(vectorf.y, (float)vectori.y);
}

TEST(Vector2, ConvertIntToFloat)
{
	dsVector2i vectori = {{-2, 3}};

	dsVector2f vectorf;
	dsConvertIntToFloat(vectorf, vectori);

	EXPECT_EQ(vectori.x, (int)vectorf.x);
	EXPECT_EQ(vectori.y, (int)vectorf.y);
}

TEST(Vector2, ConvertDoubleToInt)
{
	dsVector2d vectord = {{-2, 3}};

	dsVector2i vectori;
	dsConvertDoubleToInt(vectori, vectord);

	EXPECT_EQ(vectord.x, vectori.x);
	EXPECT_EQ(vectord.y, vectori.y);
}

TEST(Vector2, ConvertIntToDouble)
{
	dsVector2i vectori = {{-2, 3}};

	dsVector2d vectord;
	dsConvertIntToDouble(vectord, vectori);

	EXPECT_EQ(vectori.x, (int)vectord.x);
	EXPECT_EQ(vectori.y, (int)vectord.y);
}
