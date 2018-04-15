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

#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector3.h>
#include <gtest/gtest.h>

template <typename T>
struct Matrix33TypeSelector;

template <>
struct Matrix33TypeSelector<float>
{
	typedef dsMatrix33f MatrixType;
	typedef dsVector3f VectorType;
	static const float epsilon;
};

template <>
struct Matrix33TypeSelector<double>
{
	typedef dsMatrix33d MatrixType;
	typedef dsVector3d VectorType;
	static const double epsilon;
};

const float Matrix33TypeSelector<float>::epsilon = 1e-4f;
const double Matrix33TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Matrix33Test : public testing::Test
{
};

using Matrix33Types = testing::Types<float, double>;
TYPED_TEST_CASE(Matrix33Test, Matrix33Types);

inline void dsMatrix33_affineInvert(dsMatrix33f* result, const dsMatrix33f* a)
{
	dsMatrix33f_affineInvert(result, a);
}

inline void dsMatrix33_affineInvert(dsMatrix33d* result, const dsMatrix33d* a)
{
	dsMatrix33d_affineInvert(result, a);
}

inline void dsMatrix33_invert(dsMatrix33f* result, const dsMatrix33f* a)
{
	dsMatrix33f_invert(result, a);
}

inline void dsMatrix33_invert(dsMatrix33d* result, const dsMatrix33d* a)
{
	dsMatrix33d_invert(result, a);
}

inline void dsMatrix33_inverseTranspose(dsMatrix33f* result, const dsMatrix33f* a)
{
	dsMatrix33f_inverseTranspose(result, a);
}

inline void dsMatrix33_inverseTranspose(dsMatrix33d* result, const dsMatrix33d* a)
{
	dsMatrix33d_inverseTranspose(result, a);
}

inline void dsMatrix33_makeRotate(dsMatrix33f* result, float angle)
{
	dsMatrix33f_makeRotate(result, angle);
}

inline void dsMatrix33_makeRotate(dsMatrix33d* result, double angle)
{
	dsMatrix33d_makeRotate(result, angle);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33f* result, float x, float y, float z)
{
	dsMatrix33f_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33d* result, double x, double y, double z)
{
	dsMatrix33d_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33_makeRotate3DAxisAngle(dsMatrix33f* result, const dsVector3f* axis,
	float angle)
{
	dsMatrix33f_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix33_makeRotate3DAxisAngle(dsMatrix33d* result, const dsVector3d* axis,
	double angle)
{
	dsMatrix33d_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix33_makeTranslate(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeTranslate(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeScale(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeScale(result, x, y);
}

inline void dsMatrix33_makeScale3D(dsMatrix33f* result, float x, float y, float z)
{
	dsMatrix33f_makeScale3D(result, x, y, z);
}

inline void dsMatrix33_makeScale3D(dsMatrix33d* result, double x, double y, double z)
{
	dsMatrix33d_makeScale3D(result, x, y, z);
}

inline void dsVector3_normalize(dsVector3f* result, const dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, const dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

TYPED_TEST(Matrix33Test, Initialize)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	EXPECT_EQ((TypeParam)0.1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)4.5, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)-6.7, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)8.9, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)-0.1, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)2.3, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)-4.5, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)6.7, matrix.values[2][2]);

	EXPECT_EQ((TypeParam)0.1, matrix.columns[0].values[0]);
	EXPECT_EQ((TypeParam)-2.3, matrix.columns[0].values[1]);
	EXPECT_EQ((TypeParam)4.5, matrix.columns[0].values[2]);

	EXPECT_EQ((TypeParam)-6.7, matrix.columns[1].values[0]);
	EXPECT_EQ((TypeParam)8.9, matrix.columns[1].values[1]);
	EXPECT_EQ((TypeParam)-0.1, matrix.columns[1].values[2]);

	EXPECT_EQ((TypeParam)2.3, matrix.columns[2].values[0]);
	EXPECT_EQ((TypeParam)-4.5, matrix.columns[2].values[1]);
	EXPECT_EQ((TypeParam)6.7, matrix.columns[2].values[2]);
}

TYPED_TEST(Matrix33Test, Identity)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_identity(matrix);

	EXPECT_EQ(1, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ(1, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, Multiply)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix1 =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix33Type matrix2 =
	{{
		{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4},
		{(TypeParam)7.6, (TypeParam)-9.8, (TypeParam)1.0},
		{(TypeParam)-3.2, (TypeParam)5.4, (TypeParam)-7.6}
	}};

	Matrix33Type result;
	dsMatrix33_mul(result, matrix1, matrix2);

	EXPECT_NEAR((TypeParam)-33.96, result.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)55.08, result.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-41.0, result.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)68.72, result.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)-109.2, result.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)41.88, result.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-53.98, result.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)89.62, result.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)-65.86, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, Transform)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	typedef typename Matrix33TypeSelector<TypeParam>::VectorType Vector3Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-6.7, (TypeParam)2.3},
		{(TypeParam)-2.3, (TypeParam)8.9, (TypeParam)-4.5},
		{(TypeParam)4.5, (TypeParam)-0.1, (TypeParam)6.7}
	}};

	Vector3Type vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4}};
	Vector3Type result;

	dsMatrix33_transform(result, matrix, vector);

	EXPECT_NEAR((TypeParam)-31.76, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)35.72, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)-52.88, result.values[2], epsilon);
}

TYPED_TEST(Matrix33Test, TransformTransposed)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	typedef typename Matrix33TypeSelector<TypeParam>::VectorType Vector3Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	Vector3Type vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4}};
	Vector3Type result;

	dsMatrix33_transformTransposed(result, matrix, vector);

	EXPECT_NEAR((TypeParam)-31.76, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)35.72, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)-52.88, result.values[2], epsilon);
}

TYPED_TEST(Matrix33Test, Transpose)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix33Type result;
	dsMatrix33_transpose(result, matrix);

	EXPECT_EQ((TypeParam)0.1, result.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, result.values[1][0]);
	EXPECT_EQ((TypeParam)4.5, result.values[2][0]);

	EXPECT_EQ((TypeParam)-6.7, result.values[0][1]);
	EXPECT_EQ((TypeParam)8.9, result.values[1][1]);
	EXPECT_EQ((TypeParam)-0.1, result.values[2][1]);

	EXPECT_EQ((TypeParam)2.3, result.values[0][2]);
	EXPECT_EQ((TypeParam)-4.5, result.values[1][2]);
	EXPECT_EQ((TypeParam)6.7, result.values[2][2]);
}

TYPED_TEST(Matrix33Test, Determinant)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	EXPECT_NEAR((TypeParam)-53.24, dsMatrix33_determinant(matrix), epsilon);
}

TYPED_TEST(Matrix33Test, Invert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7}
	}};

	Matrix33Type inverse;
	dsMatrix33_invert(&inverse, &matrix);

	Matrix33Type result;
	dsMatrix33_mul(result, inverse, matrix);

	EXPECT_NEAR((TypeParam)-1.11157024793389, inverse.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909, inverse.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)0.74793388429752, inverse.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.83884297520661, inverse.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.181818181818182, inverse.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.56611570247934, inverse.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.18181818181818, inverse.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909091, inverse.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.272727272727273, inverse.values[2][2], epsilon);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, MakeRotate)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix;
	dsMatrix33_makeRotate(&matrix, (TypeParam)dsDegreesToRadians(30));

	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, matrix.values[0][1], epsilon);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_NEAR((TypeParam)-0.5, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[1][1], epsilon);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, MakeRotate3D)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotateX;
	dsMatrix33_makeRotate3D(&rotateX, (TypeParam)dsDegreesToRadians(30), 0, 0);

	EXPECT_EQ(1, rotateX.values[0][0]);
	EXPECT_EQ(0, rotateX.values[0][1]);
	EXPECT_EQ(0, rotateX.values[0][2]);

	EXPECT_EQ(0, rotateX.values[1][0]);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateX.values[1][2], epsilon);

	EXPECT_EQ(0, rotateX.values[2][0]);
	EXPECT_NEAR((TypeParam)-0.5, rotateX.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[2][2], epsilon);

	Matrix33Type rotateY;
	dsMatrix33_makeRotate3D(&rotateY, 0, (TypeParam)dsDegreesToRadians(-15), 0);

	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[0][0], epsilon);
	EXPECT_EQ(0, rotateY.values[0][1]);
	EXPECT_NEAR((TypeParam)0.2588190451025208, rotateY.values[0][2], epsilon);

	EXPECT_EQ(0, rotateY.values[1][0]);
	EXPECT_EQ(1, rotateY.values[1][1]);
	EXPECT_EQ(0, rotateY.values[1][2]);

	EXPECT_NEAR((TypeParam)-0.2588190451025208, rotateY.values[2][0], epsilon);
	EXPECT_EQ(0, rotateY.values[2][1]);
	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[2][2], epsilon);

	Matrix33Type rotateZ;
	dsMatrix33_makeRotate3D(&rotateZ, 0, 0, (TypeParam)dsDegreesToRadians(60));

	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateZ.values[0][1], epsilon);
	EXPECT_EQ(0, rotateZ.values[0][2]);

	EXPECT_NEAR((TypeParam)-0.866025403784439, rotateZ.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[1][1], epsilon);
	EXPECT_EQ(0, rotateZ.values[1][2]);

	EXPECT_EQ(0, rotateZ.values[2][0]);
	EXPECT_EQ(0, rotateZ.values[2][1]);
	EXPECT_EQ(1, rotateZ.values[2][2]);

	Matrix33Type temp, result;
	dsMatrix33_mul(temp, rotateY, rotateX);
	dsMatrix33_mul(result, rotateZ, temp);

	Matrix33Type rotateXYZ;
	dsMatrix33_makeRotate3D(&rotateXYZ, (TypeParam)dsDegreesToRadians(30),
		(TypeParam)dsDegreesToRadians(-15), (TypeParam)dsDegreesToRadians(60));

	EXPECT_NEAR(result.values[0][0], rotateXYZ.values[0][0], epsilon);
	EXPECT_NEAR(result.values[0][1], rotateXYZ.values[0][1], epsilon);
	EXPECT_NEAR(result.values[0][2], rotateXYZ.values[0][2], epsilon);

	EXPECT_NEAR(result.values[1][0], rotateXYZ.values[1][0], epsilon);
	EXPECT_NEAR(result.values[1][1], rotateXYZ.values[1][1], epsilon);
	EXPECT_NEAR(result.values[1][2], rotateXYZ.values[1][2], epsilon);

	EXPECT_NEAR(result.values[2][0], rotateXYZ.values[2][0], epsilon);
	EXPECT_NEAR(result.values[2][1], rotateXYZ.values[2][1], epsilon);
	EXPECT_NEAR(result.values[2][2], rotateXYZ.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, MakeRotateAxisAngle)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	typedef typename Matrix33TypeSelector<TypeParam>::VectorType Vector3Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Vector3Type axis = {{(TypeParam)-0.289967871131, (TypeParam)0.0171578621971,
		(TypeParam)0.51473586591302}};
	dsVector3_normalize(&axis, &axis);
	Matrix33Type matrix;
	dsMatrix33_makeRotate3DAxisAngle(&matrix, &axis,
		(TypeParam)dsDegreesToRadians(17.188733853924894));

	EXPECT_NEAR((TypeParam)0.96608673169969, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.25673182392846, matrix.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.02766220194012, matrix.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.25800404198456, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.95537412871306, matrix.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.14385474794174, matrix.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.01050433974302, matrix.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0.14611312318926, matrix.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.98921211783846, matrix.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, MakeTranslate)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_makeTranslate(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ(1, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ(1, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)1.2, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, MakeScale)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_makeScale(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, MakeScale3D)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_makeScale3D(&matrix, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)5.6, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, FastInvert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));

	Matrix33Type translate;
	dsMatrix33_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33Type matrix;
	dsMatrix33_mul(matrix, translate, rotate);

	Matrix33Type inverse;
	dsMatrix33_fastInvert(inverse, matrix);

	Matrix33Type result;
	dsMatrix33_mul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, AffineInvert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));

	Matrix33Type translate;
	dsMatrix33_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33Type scale;
	dsMatrix33_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3);

	Matrix33Type temp;
	dsMatrix33_affineMul(temp, scale, rotate);

	Matrix33Type matrix;
	dsMatrix33_affineMul(matrix, translate, temp);

	Matrix33Type inverse;
	dsMatrix33_affineInvert(&inverse, &matrix);

	Matrix33Type result;
	dsMatrix33_affineMul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, InverseTranspose)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));

	Matrix33Type translate;
	dsMatrix33_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33Type scale;
	dsMatrix33_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3);

	Matrix33Type temp;
	dsMatrix33_mul(temp, scale, rotate);

	Matrix33Type matrix;
	dsMatrix33_mul(matrix, translate, temp);

	Matrix33Type inverseTranspose;
	dsMatrix33_inverseTranspose(&inverseTranspose, &matrix);

	Matrix33Type inverse, inverseTransposeCheck;
	dsMatrix33_invert(&inverse, &matrix);
	dsMatrix33_transpose(inverseTransposeCheck, inverse);

	EXPECT_NEAR(inverseTransposeCheck.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][1], inverseTranspose.values[0][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][2], inverseTranspose.values[0][2], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][1], inverseTranspose.values[1][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][1], inverseTranspose.values[1][1], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[2][0], inverseTranspose.values[2][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[2][1], inverseTranspose.values[2][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[2][2], inverseTranspose.values[2][2], epsilon);
}

TEST(Matrix33, ConvertFloatToDouble)
{
	dsMatrix33f matrixf =
	{{
		{0.1f, -2.3f, 4.5f},
		{-6.7f, 8.9f, -0.1f},
		{2.3f, -4.5f, 6.7f}
	}};

	dsMatrix33d matrixd;
	dsConvertFloatToDouble(matrixd, matrixf);

	EXPECT_FLOAT_EQ(matrixf.values[0][0], (float)matrixd.values[0][0]);
	EXPECT_FLOAT_EQ(matrixf.values[0][1], (float)matrixd.values[0][1]);
	EXPECT_FLOAT_EQ(matrixf.values[0][2], (float)matrixd.values[0][2]);

	EXPECT_FLOAT_EQ(matrixf.values[1][0], (float)matrixd.values[1][0]);
	EXPECT_FLOAT_EQ(matrixf.values[1][1], (float)matrixd.values[1][1]);
	EXPECT_FLOAT_EQ(matrixf.values[1][2], (float)matrixd.values[1][2]);

	EXPECT_FLOAT_EQ(matrixf.values[2][0], (float)matrixd.values[2][0]);
	EXPECT_FLOAT_EQ(matrixf.values[2][1], (float)matrixd.values[2][1]);
	EXPECT_FLOAT_EQ(matrixf.values[2][2], (float)matrixd.values[2][2]);
}

TEST(Matrix33, ConvertDoubleToFloat)
{
	dsMatrix33d matrixd =
	{{
		{0.1, -2.3, 4.5},
		{-6.7, 8.9, -0.1},
		{2.3, -4.5, 6.7}
	}};

	dsMatrix33f matrixf;
	dsConvertDoubleToFloat(matrixf, matrixd);

	EXPECT_FLOAT_EQ((float)matrixd.values[0][0], matrixf.values[0][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][1], matrixf.values[0][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][2], matrixf.values[0][2]);

	EXPECT_FLOAT_EQ((float)matrixd.values[1][0], matrixf.values[1][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][1], matrixf.values[1][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][2], matrixf.values[1][2]);

	EXPECT_FLOAT_EQ((float)matrixd.values[2][0], matrixf.values[2][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[2][1], matrixf.values[2][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[2][2], matrixf.values[2][2]);
}
