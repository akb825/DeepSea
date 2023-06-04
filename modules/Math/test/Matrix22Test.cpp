/*
 * Copyright 2016-2023 Aaron Barany
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

#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Vector2.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Matrix22TypeSelector;

template <>
struct Matrix22TypeSelector<float>
{
	typedef dsMatrix22f MatrixType;
	typedef dsVector2f VectorType;
	static const float epsilon;
};

template <>
struct Matrix22TypeSelector<double>
{
	typedef dsMatrix22d MatrixType;
	typedef dsVector2d VectorType;
	static const double epsilon;
};

const float Matrix22TypeSelector<float>::epsilon = 1e-4f;
const double Matrix22TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Matrix22Test : public testing::Test
{
};

using Matrix22Types = testing::Types<float, double>;
TYPED_TEST_SUITE(Matrix22Test, Matrix22Types);

inline void dsMatrix22_invert(dsMatrix22f* result, const dsMatrix22f* a)
{
	dsMatrix22f_invert(result, a);
}

inline void dsMatrix22_invert(dsMatrix22d* result, const dsMatrix22d* a)
{
	dsMatrix22d_invert(result, a);
}

inline void dsMatrix22_makeRotate(dsMatrix22f* result, float angle)
{
	dsMatrix22f_makeRotate(result, angle);
}

inline void dsMatrix22_makeRotate(dsMatrix22d* result, double angle)
{
	dsMatrix22d_makeRotate(result, angle);
}

inline void dsMatrix22_makeScale(dsMatrix22f* result, float x, float y)
{
	dsMatrix22f_makeScale(result, x, y);
}

inline void dsMatrix22_makeScale(dsMatrix22d* result, double x, double y)
{
	dsMatrix22d_makeScale(result, x, y);
}

TYPED_TEST(Matrix22Test, Initialize)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	EXPECT_EQ((TypeParam)0.1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, matrix.values[0][1]);

	EXPECT_EQ((TypeParam)-4.5, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)6.7, matrix.values[1][1]);
}

TYPED_TEST(Matrix22Test, Identity)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;

	Matrix22Type matrix;
	dsMatrix22_identity(matrix);

	EXPECT_EQ((TypeParam)1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)1, matrix.values[1][1]);
}

TYPED_TEST(Matrix22Test, Copy)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix22Type copy;
	dsMatrix22_copy(copy, matrix);

	EXPECT_EQ(copy.values[0][0], matrix.values[0][0]);
	EXPECT_EQ(copy.values[0][1], matrix.values[0][1]);

	EXPECT_EQ(copy.values[1][0], matrix.values[1][0]);
	EXPECT_EQ(copy.values[1][1], matrix.values[1][1]);
}

TYPED_TEST(Matrix22Test, Multiply)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix1 =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix22Type matrix2 =
	{{
		{(TypeParam)-1.0, (TypeParam)3.2},
		{(TypeParam)-5.4, (TypeParam)7.6}
	}};

	Matrix22Type result;
	dsMatrix22_mul(result, matrix1, matrix2);

	EXPECT_NEAR((TypeParam)-14.5, result.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)23.74, result.values[0][1], epsilon);

	EXPECT_NEAR((TypeParam)-34.74, result.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)63.34, result.values[1][1], epsilon);
}

TYPED_TEST(Matrix22Test, Transform)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	typedef typename Matrix22TypeSelector<TypeParam>::VectorType Vector2Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-4.5},
		{(TypeParam)-2.3, (TypeParam)6.7}
	}};

	Vector2Type vector = {{(TypeParam)-1.0, (TypeParam)3.2}};
	Vector2Type result;

	dsMatrix22_transform(result, matrix, vector);

	EXPECT_NEAR((TypeParam)-7.46, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)25.94, result.values[1], epsilon);
}

TYPED_TEST(Matrix22Test, TransformTransposed)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	typedef typename Matrix22TypeSelector<TypeParam>::VectorType Vector2Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	Vector2Type vector = {{(TypeParam)-1.0, (TypeParam)3.2}};
	Vector2Type result;

	dsMatrix22_transformTransposed(result, matrix, vector);

	EXPECT_NEAR((TypeParam)-7.46, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)25.94, result.values[1], epsilon);
}

TYPED_TEST(Matrix22Test, Transpose)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix22Type result;
	dsMatrix22_transpose(result, matrix);

	EXPECT_EQ((TypeParam)0.1, result.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, result.values[1][0]);

	EXPECT_EQ((TypeParam)-4.5, result.values[0][1]);
	EXPECT_EQ((TypeParam)6.7, result.values[1][1]);
}

TYPED_TEST(Matrix22Test, Determinant)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	EXPECT_NEAR((TypeParam)-9.68, dsMatrix22_determinant(matrix), epsilon);
}

TYPED_TEST(Matrix22Test, Invert)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix22Type inverse;
	dsMatrix22_invert(&inverse, &matrix);

	Matrix22Type result;
	dsMatrix22_mul(result, inverse, matrix);

	EXPECT_NEAR((TypeParam)-0.69214876033058, inverse.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)-0.2376033057851, inverse.values[0][1], epsilon);

	EXPECT_NEAR((TypeParam)-0.464876033057851, inverse.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)-0.0103305785123967, inverse.values[1][1], epsilon);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
}

TYPED_TEST(Matrix22Test, MakeRotate)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;
	TypeParam epsilon = Matrix22TypeSelector<TypeParam>::epsilon;

	Matrix22Type matrix;
	dsMatrix22_makeRotate(&matrix, (TypeParam)dsDegreesToRadiansd(30));

	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, matrix.values[0][1], epsilon);

	EXPECT_NEAR((TypeParam)-0.5, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[1][1], epsilon);
}

TYPED_TEST(Matrix22Test, MakeScale)
{
	typedef typename Matrix22TypeSelector<TypeParam>::MatrixType Matrix22Type;

	Matrix22Type matrix;
	dsMatrix22_makeScale(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
}

TEST(Matrix22, MultiplyDouble)
{
	double epsilon = Matrix22TypeSelector<double>::epsilon;

	dsMatrix22d matrix1 =
	{{
		{0.1, -2.3},
		{-4.5, 6.7}
	}};

	dsMatrix22d matrix2 =
	{{
		{-1.0, 3.2},
		{-5.4, 7.6}
	}};

	dsMatrix22d result;
	dsMatrix22d_mul(&result, &matrix1, &matrix2);

	EXPECT_NEAR(-14.5, result.values[0][0], epsilon);
	EXPECT_NEAR(23.74, result.values[0][1], epsilon);

	EXPECT_NEAR(-34.74, result.values[1][0], epsilon);
	EXPECT_NEAR(63.34, result.values[1][1], epsilon);
}

TEST(Matrix22, TransformDouble)
{
	double epsilon = Matrix22TypeSelector<double>::epsilon;

	dsMatrix22d matrix =
	{{
		{0.1, -4.5},
		{-2.3, 6.7}
	}};

	dsVector2d vector = {{-1.0, 3.2}};
	dsVector2d result;

	dsMatrix22d_transform(&result, &matrix, &vector);

	EXPECT_NEAR(-7.46, result.values[0], epsilon);
	EXPECT_NEAR(25.94, result.values[1], epsilon);
}

TEST(Matrix22, TransformTransposedDouble)
{
	double epsilon = Matrix22TypeSelector<double>::epsilon;

	dsMatrix22d matrix =
	{{
		{0.1, -2.3},
		{-4.5, 6.7}
	}};

	dsVector2d vector = {{-1.0, 3.2}};
	dsVector2d result;

	dsMatrix22d_transformTransposed(&result, &matrix, &vector);

	EXPECT_NEAR(-7.46, result.values[0], epsilon);
	EXPECT_NEAR(25.94, result.values[1], epsilon);
}

TEST(Matrix22, TransposeDouble)
{
	dsMatrix22d matrix =
	{{
		{0.1, -2.3},
		{-4.5, 6.7}
	}};

	dsMatrix22d result;
	dsMatrix22d_transpose(&result, &matrix);

	EXPECT_EQ(0.1, result.values[0][0]);
	EXPECT_EQ(-2.3, result.values[1][0]);

	EXPECT_EQ(-4.5, result.values[0][1]);
	EXPECT_EQ(6.7, result.values[1][1]);
}

TEST(Matrix22, ConvertFloatToDouble)
{
	dsMatrix22f matrixf =
	{{
		{0.1f, -2.3f},
		{-4.5f, 6.7f}
	}};

	dsMatrix22d matrixd;
	dsConvertFloatToDouble(matrixd, matrixf);

	EXPECT_FLOAT_EQ(matrixf.values[0][0], (float)matrixd.values[0][0]);
	EXPECT_FLOAT_EQ(matrixf.values[0][1], (float)matrixd.values[0][1]);

	EXPECT_FLOAT_EQ(matrixf.values[1][0], (float)matrixd.values[1][0]);
	EXPECT_FLOAT_EQ(matrixf.values[1][1], (float)matrixd.values[1][1]);
}

TEST(Matrix22, ConvertDoubleToFloat)
{
	dsMatrix22d matrixd =
	{{
		{0.1, -2.3},
		{-4.5, 6.7}
	}};

	dsMatrix22f matrixf;
	dsConvertDoubleToFloat(matrixf, matrixd);

	EXPECT_FLOAT_EQ((float)matrixd.values[0][0], matrixf.values[0][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][1], matrixf.values[0][1]);

	EXPECT_FLOAT_EQ((float)matrixd.values[1][0], matrixf.values[1][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][1], matrixf.values[1][1]);
}
