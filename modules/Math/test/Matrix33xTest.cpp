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
#include <DeepSea/Math/Matrix33x.h>
#include <DeepSea/Math/Vector3x.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Matrix33xTypeSelector;

template <>
struct Matrix33xTypeSelector<float>
{
	typedef dsMatrix33xf Matrix33xType;
	typedef dsMatrix22f Matrix22Type;
	typedef dsVector3xf Vector3xType;
	typedef dsVector3f Vector3Type;
	static const float epsilon;
};

template <>
struct Matrix33xTypeSelector<double>
{
	typedef dsMatrix33xd Matrix33xType;
	typedef dsMatrix22d Matrix22Type;
	typedef dsVector3xd Vector3xType;
	typedef dsVector3d Vector3Type;
	static const double epsilon;
};

const float Matrix33xTypeSelector<float>::epsilon = 1e-4f;
const double Matrix33xTypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Matrix33xTest : public testing::Test
{
};

using Matrix33xTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(Matrix33xTest, Matrix33xTypes);

inline void dsMatrix33x_identity(dsMatrix33xf* result)
{
	dsMatrix33xf_identity(result);
}

inline void dsMatrix33x_identity(dsMatrix33xd* result)
{
	dsMatrix33xd_identity(result);
}

inline void dsMatrix33x_mul(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	dsMatrix33xf_mul(result, a, b);
}

inline void dsMatrix33x_mul(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	dsMatrix33xd_mul(result, a, b);
}

inline void dsMatrix33x_affineMul(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	dsMatrix33xf_affineMul(result, a, b);
}

inline void dsMatrix33x_affineMul(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	dsMatrix33xd_affineMul(result, a, b);
}

inline void dsMatrix33x_transform(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	dsMatrix33xf_transform(result, mat, vec);
}

inline void dsMatrix33x_transform(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	dsMatrix33xd_transform(result, mat, vec);
}

inline void dsMatrix33x_transformTransposed(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	dsMatrix33xf_transformTransposed(result, mat, vec);
}

inline void dsMatrix33x_transformTransposed(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	dsMatrix33xd_transformTransposed(result, mat, vec);
}

inline void dsMatrix33x_transpose(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	dsMatrix33xf_transpose(result, a);
}

inline void dsMatrix33x_transpose(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	dsMatrix33xd_transpose(result, a);
}

inline float dsMatrix33x_determinant(const dsMatrix33xf* a)
{
	return dsMatrix33xf_determinant(a);
}

inline double dsMatrix33x_determinant(const dsMatrix33xd* a)
{
	return dsMatrix33xd_determinant(a);
}

inline void dsMatrix33x_fastInvert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	dsMatrix33xf_fastInvert(result, a);
}

inline void dsMatrix33x_fastInvert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	dsMatrix33xd_fastInvert(result, a);
}

inline void dsMatrix33x_affineInvert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	dsMatrix33xf_affineInvert(result, a);
}

inline void dsMatrix33x_affineInvert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	dsMatrix33xd_affineInvert(result, a);
}

inline void dsMatrix33x_invert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	dsMatrix33xf_invert(result, a);
}

inline void dsMatrix33x_invert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	dsMatrix33xd_invert(result, a);
}

inline void dsMatrix33x_inverseTranspose(dsMatrix22f* result, const dsMatrix33xf* a)
{
	dsMatrix33xf_inverseTranspose(result, a);
}

inline void dsMatrix33x_inverseTranspose(dsMatrix22d* result, const dsMatrix33xd* a)
{
	dsMatrix33xd_inverseTranspose(result, a);
}

inline void dsMatrix33x_makeRotate(dsMatrix33xf* result, float angle)
{
	dsMatrix33xf_makeRotate(result, angle);
}

inline void dsMatrix33x_makeRotate(dsMatrix33xd* result, double angle)
{
	dsMatrix33xd_makeRotate(result, angle);
}

inline void dsMatrix33x_makeRotate3D(dsMatrix33xf* result, float x, float y, float z)
{
	dsMatrix33xf_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33x_makeRotate3D(dsMatrix33xd* result, double x, double y, double z)
{
	dsMatrix33xd_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33x_makeRotate3DAxisAngle(
	dsMatrix33xf* result, const dsVector3f* axis, float angle)
{
	dsMatrix33xf_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix33x_makeRotate3DAxisAngle(
	dsMatrix33xd* result, const dsVector3d* axis, double angle)
{
	dsMatrix33xd_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix33x_makeTranslate(dsMatrix33xf* result, float x, float y)
{
	dsMatrix33xf_makeTranslate(result, x, y);
}

inline void dsMatrix33x_makeTranslate(dsMatrix33xd* result, double x, double y)
{
	dsMatrix33xd_makeTranslate(result, x, y);
}

inline void dsMatrix33x_makeScale(dsMatrix33xf* result, float x, float y)
{
	dsMatrix33xf_makeScale(result, x, y);
}

inline void dsMatrix33x_makeScale(dsMatrix33xd* result, double x, double y)
{
	dsMatrix33xd_makeScale(result, x, y);
}

inline void dsMatrix33x_makeScale3D(dsMatrix33xf* result, float x, float y, float z)
{
	dsMatrix33xf_makeScale3D(result, x, y, z);
}

inline void dsMatrix33x_makeScale3D(dsMatrix33xd* result, double x, double y, double z)
{
	dsMatrix33xd_makeScale3D(result, x, y, z);
}

inline void dsVector3_normalize(dsVector3f* result, const dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, const dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

TYPED_TEST(Matrix33xTest, Identity)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;

	Matrix33xType matrix;
	dsMatrix33x_identity(&matrix);

	EXPECT_EQ((TypeParam)1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)1, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33xTest, Multiply)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix1 =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	Matrix33xType matrix2 =
	{{
		{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4, (TypeParam)-4},
		{(TypeParam)7.6, (TypeParam)-9.8, (TypeParam)1.0, (TypeParam)-5},
		{(TypeParam)-3.2, (TypeParam)5.4, (TypeParam)-7.6, (TypeParam)-6}
	}};

	Matrix33xType result;
	dsMatrix33x_mul(&result, &matrix1, &matrix2);

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

TYPED_TEST(Matrix33xTest, Transform)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	typedef typename Matrix33xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	Vector3xType vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4, (TypeParam)-4}};
	Vector3xType result;

	dsMatrix33x_transform(&result, &matrix, &vector);

	EXPECT_NEAR((TypeParam)-33.96, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)55.08, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)-41.0, result.values[2], epsilon);
}

TYPED_TEST(Matrix33xTest, TransformTransposed)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	typedef typename Matrix33xTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	Vector3xType vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4, (TypeParam)-4}};
	Vector3xType result;

	dsMatrix33x_transformTransposed(&result, &matrix, &vector);

	EXPECT_NEAR((TypeParam)-31.76, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)35.72, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)-52.88, result.values[2], epsilon);
}

TYPED_TEST(Matrix33xTest, Transpose)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;

	Matrix33xType matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	Matrix33xType result;
	dsMatrix33x_transpose(&result, &matrix);

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

TYPED_TEST(Matrix33xTest, Determinant)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	EXPECT_NEAR((TypeParam)-53.24, dsMatrix33x_determinant(&matrix), epsilon);
}

TYPED_TEST(Matrix33xTest, Invert)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix =
	{{
		{(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5, (TypeParam)1},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2},
		{(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7, (TypeParam)3}
	}};

	Matrix33xType inverse;
	dsMatrix33x_invert(&inverse, &matrix);

	Matrix33xType result;
	dsMatrix33x_mul(&result, &inverse, &matrix);

	EXPECT_NEAR((TypeParam)-1.11157024793389, inverse.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909, inverse.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)0.74793388429752, inverse.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.83884297520661, inverse.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.181818181818182, inverse.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.56611570247934, inverse.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.18181818181818, inverse.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909091, inverse.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.272727272727273, inverse.values[2][2], epsilon);

	EXPECT_NEAR((TypeParam)1, result.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0, result.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)0, result.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)0, result.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)1, result.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0, result.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)0, result.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0, result.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33xTest, MakeRotate)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType matrix;
	dsMatrix33x_makeRotate(&matrix, (TypeParam)dsDegreesToRadiansd(30));

	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, matrix.values[0][1], epsilon);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);

	EXPECT_NEAR((TypeParam)-0.5, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[1][1], epsilon);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33xTest, MakeRotate3D)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType rotateX;
	dsMatrix33x_makeRotate3D(&rotateX, (TypeParam)dsDegreesToRadiansd(30), 0, 0);

	EXPECT_EQ((TypeParam)1, rotateX.values[0][0]);
	EXPECT_EQ((TypeParam)0, rotateX.values[0][1]);
	EXPECT_EQ((TypeParam)0, rotateX.values[0][2]);

	EXPECT_EQ((TypeParam)0, rotateX.values[1][0]);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateX.values[1][2], epsilon);

	EXPECT_EQ((TypeParam)0, rotateX.values[2][0]);
	EXPECT_NEAR((TypeParam)-0.5, rotateX.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[2][2], epsilon);

	Matrix33xType rotateY;
	dsMatrix33x_makeRotate3D(&rotateY, 0, (TypeParam)dsDegreesToRadiansd(-15), 0);

	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[0][0], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[0][1]);
	EXPECT_NEAR((TypeParam)0.2588190451025208, rotateY.values[0][2], epsilon);

	EXPECT_EQ((TypeParam)0, rotateY.values[1][0]);
	EXPECT_EQ((TypeParam)1, rotateY.values[1][1]);
	EXPECT_EQ((TypeParam)0, rotateY.values[1][2]);

	EXPECT_NEAR((TypeParam)-0.2588190451025208, rotateY.values[2][0], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[2][1]);
	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[2][2], epsilon);

	Matrix33xType rotateZ;
	dsMatrix33x_makeRotate3D(&rotateZ, 0, 0, (TypeParam)dsDegreesToRadiansd(60));

	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateZ.values[0][1], epsilon);
	EXPECT_EQ((TypeParam)0, rotateZ.values[0][2]);

	EXPECT_NEAR((TypeParam)-0.866025403784439, rotateZ.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[1][1], epsilon);
	EXPECT_EQ((TypeParam)0, rotateZ.values[1][2]);

	EXPECT_EQ((TypeParam)0, rotateZ.values[2][0]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[2][1]);
	EXPECT_EQ((TypeParam)1, rotateZ.values[2][2]);

	Matrix33xType temp, result;
	dsMatrix33x_mul(&temp, &rotateY, &rotateX);
	dsMatrix33x_mul(&result, &rotateZ, &temp);

	Matrix33xType rotateXYZ;
	dsMatrix33x_makeRotate3D(&rotateXYZ, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));

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

TYPED_TEST(Matrix33xTest, MakeRotateAxisAngle)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	typedef typename Matrix33xTypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Vector3Type axis = {{(TypeParam)-0.289967871131, (TypeParam)0.0171578621971,
		(TypeParam)0.51473586591302}};
	dsVector3_normalize(&axis, &axis);
	Matrix33xType matrix;
	dsMatrix33x_makeRotate3DAxisAngle(&matrix, &axis,
		(TypeParam)dsDegreesToRadiansd(17.188733853924894));

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

TYPED_TEST(Matrix33xTest, MakeTranslate)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;

	Matrix33xType matrix;
	dsMatrix33x_makeTranslate(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ((TypeParam)1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)1, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)1.2, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33xTest, MakeScale)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;

	Matrix33xType matrix;
	dsMatrix33x_makeScale(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33xTest, MakeScale3D)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;

	Matrix33xType matrix;
	dsMatrix33x_makeScale3D(&matrix, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)5.6, matrix.values[2][2]);
}

TYPED_TEST(Matrix33xTest, FastInvert)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType rotate;
	dsMatrix33x_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30));

	Matrix33xType translate;
	dsMatrix33x_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33xType matrix;
	dsMatrix33x_mul(&matrix, &translate, &rotate);

	Matrix33xType inverse;
	dsMatrix33x_fastInvert(&inverse, &matrix);

	Matrix33xType result;
	dsMatrix33x_mul(&result, &inverse, &matrix);

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

TYPED_TEST(Matrix33xTest, AffineInvert)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType rotate;
	dsMatrix33x_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30));

	Matrix33xType translate;
	dsMatrix33x_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33xType scale;
	dsMatrix33x_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3);

	Matrix33xType temp;
	dsMatrix33x_affineMul(&temp, &scale, &rotate);

	Matrix33xType matrix;
	dsMatrix33x_affineMul(&matrix, &translate, &temp);

	Matrix33xType inverse;
	dsMatrix33x_affineInvert(&inverse, &matrix);

	Matrix33xType result;
	dsMatrix33x_affineMul(&result, &inverse, &matrix);

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

TYPED_TEST(Matrix33xTest, InverseTranspose)
{
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	typedef typename Matrix33xTypeSelector<TypeParam>::Matrix22Type Matrix22Type;
	TypeParam epsilon = Matrix33xTypeSelector<TypeParam>::epsilon;

	Matrix33xType rotate;
	dsMatrix33x_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30));

	Matrix33xType translate;
	dsMatrix33x_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33xType scale;
	dsMatrix33x_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3);

	Matrix33xType temp;
	dsMatrix33x_mul(&temp, &scale, &rotate);

	Matrix33xType matrix;
	dsMatrix33x_mul(&matrix, &translate, &temp);

	Matrix22Type inverseTranspose;
	dsMatrix33x_inverseTranspose(&inverseTranspose, &matrix);

	Matrix33xType inverse, inverseTransposeCheck;
	dsMatrix33x_invert(&inverse, &matrix);
	dsMatrix33_transpose(inverseTransposeCheck, inverse);

	EXPECT_NEAR(inverseTransposeCheck.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][1], inverseTranspose.values[0][1], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][1], inverseTranspose.values[1][1], epsilon);
}

#if DS_HAS_SIMD

TEST(Matrix33xfTest, MultiplySIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix1 =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xf matrix2 =
	{{
		{-1.0f, 3.2f, -5.4f, -4.0f},
		{7.6f, -9.8f, 1.0f, -5.0f},
		{-3.2f, 5.4f, -7.6f, -6.0f}
	}};

	dsMatrix33xf scalarResult, result;
	dsMatrix33_mul(scalarResult, matrix1, matrix2);
	dsMatrix33xf_mulSIMD(&result, &matrix1, &matrix2);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][0], result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][1], result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][2], result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][0], result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][1], result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][2], result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][0], result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][1], result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][2], result.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96f, result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.08f, result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.0f, result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(68.72f, result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-109.2f, result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(41.88f, result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-53.98f, result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(89.6199951f, result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-65.86f, result.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, MultiplyFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf matrix1 =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xf matrix2 =
	{{
		{-1.0f, 3.2f, -5.4f, -4.0f},
		{7.6f, -9.8f, 1.0f, -5.0f},
		{-3.2f, 5.4f, -7.6f, -6.0f}
	}};

	dsMatrix33xf result;
	dsMatrix33xf_mulFMA(&result, &matrix1, &matrix2);

	EXPECT_NEAR(-33.96f, result.values[0][0], epsilon);
	EXPECT_NEAR(55.08f, result.values[0][1], epsilon);
	EXPECT_NEAR(-41.0f, result.values[0][2], epsilon);

	EXPECT_NEAR(68.72f, result.values[1][0], epsilon);
	EXPECT_NEAR(-109.2f, result.values[1][1], epsilon);
	EXPECT_NEAR(41.88f, result.values[1][2], epsilon);

	EXPECT_NEAR(-53.98f, result.values[2][0], epsilon);
	EXPECT_NEAR(89.6199951f, result.values[2][1], epsilon);
	EXPECT_NEAR(-65.86f, result.values[2][2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, MultiplySIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd matrix1 =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xd matrix2 =
	{{
		{-1.0, 3.2, -5.4, -4.0},
		{7.6, -9.8, 1.0, -5.0},
		{-3.2, 5.4, -7.6, -6.0}
	}};

	dsMatrix33xd scalarResult, result;
	dsMatrix33_mul(scalarResult, matrix1, matrix2);
	dsMatrix33xd_mulSIMD2(&result, &matrix1, &matrix2);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][0], result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][1], result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][2], result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][0], result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][1], result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][2], result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][0], result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][1], result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][2], result.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96, result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.080000000000005, result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.000000000000007, result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(68.720000000000013, result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-109.20000000000002, result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(41.879999999999995, result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-53.980000000000004, result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(89.62, result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-65.86, result.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, MultiplyFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd matrix1 =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xd matrix2 =
	{{
		{-1.0, 3.2, -5.4, -4.0},
		{7.6, -9.8, 1.0, -5.0},
		{-3.2, 5.4, -7.6, -6.0}
	}};

	dsMatrix33xd result;
	dsMatrix33xd_mulFMA2(&result, &matrix1, &matrix2);

	EXPECT_NEAR(-33.96, result.values[0][0], epsilon);
	EXPECT_NEAR(55.080000000000005, result.values[0][1], epsilon);
	EXPECT_NEAR(-41.000000000000007, result.values[0][2], epsilon);

	EXPECT_NEAR(68.720000000000013, result.values[1][0], epsilon);
	EXPECT_NEAR(-109.20000000000002, result.values[1][1], epsilon);
	EXPECT_NEAR(41.879999999999995, result.values[1][2], epsilon);

	EXPECT_NEAR(-53.980000000000004, result.values[2][0], epsilon);
	EXPECT_NEAR(89.62, result.values[2][1], epsilon);
	EXPECT_NEAR(-65.86, result.values[2][2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, MultiplySIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd matrix1 =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	DS_ALIGN(32) dsMatrix33xd matrix2 =
	{{
		{-1.0, 3.2, -5.4, -4.0},
		{7.6, -9.8, 1.0, -5.0},
		{-3.2, 5.4, -7.6, -6.0}
	}};

	DS_ALIGN(32) dsMatrix33xd scalarResult, result;
	dsMatrix33_mul(scalarResult, matrix1, matrix2);
	dsMatrix33xd_mulSIMD4(&result, &matrix1, &matrix2);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][0], result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][1], result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0][2], result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][0], result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][1], result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1][2], result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][0], result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][1], result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2][2], result.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96, result.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.080000000000005, result.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.000000000000007, result.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(68.720000000000013, result.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-109.20000000000002, result.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(41.879999999999995, result.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-53.980000000000004, result.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(89.62, result.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-65.86, result.values[2][2], epsilon);
}

TEST(Matrix33xfTest, TransformSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsVector3xf vector = {{-1.0f, 3.2f, -5.4f, -4.0f}};
	dsVector3xf scalarResult, result;

	dsMatrix33_transform(scalarResult, matrix, vector);
	dsMatrix33xf_transformSIMD(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96f, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.08f, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.0f, result.values[2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, TransformFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsVector3xf vector = {{-1.0f, 3.2f, -5.4f, -4.0f}};
	dsVector3xf result;
	dsMatrix33xf_transformFMA(&result, &matrix, &vector);

	EXPECT_NEAR(-33.96f, result.values[0], epsilon);
	EXPECT_NEAR(55.08f, result.values[1], epsilon);
	EXPECT_NEAR(-41.0f, result.values[2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, TransformSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	dsVector3xd scalarResult, result;

	dsMatrix33_transform(scalarResult, matrix, vector);
	dsMatrix33xd_transformSIMD2(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.080000000000005, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.000000000000007, result.values[2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, TransformFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	dsVector3xd result;
	dsMatrix33xd_transformFMA2(&result, &matrix, &vector);

	EXPECT_NEAR(-33.96, result.values[0], epsilon);
	EXPECT_NEAR(55.080000000000005, result.values[1], epsilon);
	EXPECT_NEAR(-41.000000000000007, result.values[2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, TransformSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	DS_ALIGN(32) dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	DS_ALIGN(32) dsVector3xd scalarResult, result;

	dsMatrix33_transform(scalarResult, matrix, vector);
	dsMatrix33xd_transformSIMD4(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-33.96, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(55.080000000000005, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-41.000000000000007, result.values[2], epsilon);
}

TEST(Matrix33xfTest, TransformTransposedSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsVector3xf vector = {{-1.0f, 3.2f, -5.4f, -4.0f}};
	dsVector3xf scalarResult, result;

	dsMatrix33_transformTransposed(scalarResult, matrix, vector);
	dsMatrix33xf_transformTransposedSIMD(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-31.7600021f, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(35.72f, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-52.88f, result.values[2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, TransformTransposedFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsVector3xf vector = {{-1.0f, 3.2f, -5.4f, -4.0f}};
	dsVector3xf result;
	dsMatrix33xf_transformTransposedFMA(&result, &matrix, &vector);

	EXPECT_NEAR(-31.7600021f, result.values[0], epsilon);
	EXPECT_NEAR(35.72f, result.values[1], epsilon);
	EXPECT_NEAR(-52.88f, result.values[2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, TransformTransposedSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	dsVector3xd scalarResult, result;

	dsMatrix33_transformTransposed(scalarResult, matrix, vector);
	dsMatrix33xd_transformTransposedSIMD2(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-31.759999999999998, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(35.720000000000006, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-52.88000000000001, result.values[2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, TransformTransposedFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	dsVector3xd result;
	dsMatrix33xd_transformTransposedFMA2(&result, &matrix, &vector);

	EXPECT_NEAR(-31.759999999999998, result.values[0], epsilon);
	EXPECT_NEAR(35.720000000000006, result.values[1], epsilon);
	EXPECT_NEAR(-52.88000000000001, result.values[2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, TransformTransposedSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	DS_ALIGN(32) dsVector3xd vector = {{-1.0, 3.2, -5.4, -4.0}};
	DS_ALIGN(32) dsVector3xd scalarResult, result;

	dsMatrix33_transformTransposed(scalarResult, matrix, vector);
	dsMatrix33xd_transformTransposedSIMD4(&result, &matrix, &vector);

	EXPECT_EQ_DETERMINISTIC(scalarResult.values[0], result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[1], result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarResult.values[2], result.values[2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-31.759999999999998, result.values[0], epsilon);
	EXPECT_EQ_DETERMINISTIC(35.720000000000006, result.values[1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-52.88000000000001, result.values[2], epsilon);
}

TEST(Matrix33xfTest, TransposeSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xf result;
	dsMatrix33xf_transposeSIMD(&result, &matrix);

	EXPECT_EQ(0.1f, result.values[0][0]);
	EXPECT_EQ(-2.3f, result.values[1][0]);
	EXPECT_EQ(4.5f, result.values[2][0]);

	EXPECT_EQ(-6.7f, result.values[0][1]);
	EXPECT_EQ(8.9f, result.values[1][1]);
	EXPECT_EQ(-0.1f, result.values[2][1]);

	EXPECT_EQ(2.3f, result.values[0][2]);
	EXPECT_EQ(-4.5f, result.values[1][2]);
	EXPECT_EQ(6.7f, result.values[2][2]);
}

TEST(Matrix33xdTest, TransposeSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xd result;
	dsMatrix33xd_transposeSIMD2(&result, &matrix);

	EXPECT_EQ(0.1, result.values[0][0]);
	EXPECT_EQ(-2.3, result.values[1][0]);
	EXPECT_EQ(4.5, result.values[2][0]);

	EXPECT_EQ(-6.7, result.values[0][1]);
	EXPECT_EQ(8.9, result.values[1][1]);
	EXPECT_EQ(-0.1, result.values[2][1]);

	EXPECT_EQ(2.3, result.values[0][2]);
	EXPECT_EQ(-4.5, result.values[1][2]);
	EXPECT_EQ(6.7, result.values[2][2]);
}

TEST(Matrix33xdTest, TransposeSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	DS_ALIGN(32) dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	DS_ALIGN(32) dsMatrix33xd result;
	dsMatrix33xd_transposeSIMD4(&result, &matrix);

	EXPECT_EQ(0.1, result.values[0][0]);
	EXPECT_EQ(-2.3, result.values[1][0]);
	EXPECT_EQ(4.5, result.values[2][0]);

	EXPECT_EQ(-6.7, result.values[0][1]);
	EXPECT_EQ(8.9, result.values[1][1]);
	EXPECT_EQ(-0.1, result.values[2][1]);

	EXPECT_EQ(2.3, result.values[0][2]);
	EXPECT_EQ(-4.5, result.values[1][2]);
	EXPECT_EQ(6.7, result.values[2][2]);
}

TEST(Matrix33xfTest, DeterminantSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	float scalarResult = dsMatrix33_determinant(matrix);
	float result = dsMatrix33xf_determinantSIMD(&matrix);

	EXPECT_EQ_DETERMINISTIC(scalarResult, result, epsilon);
	EXPECT_EQ_DETERMINISTIC(-53.2399864f, result, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, DeterminantFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	float result = dsMatrix33xf_determinantFMA(&matrix);
	EXPECT_NEAR(-53.2399864f, result, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, DeterminantSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	double scalarResult = dsMatrix33_determinant(matrix);
	double result = dsMatrix33xd_determinantSIMD2(&matrix);

	EXPECT_EQ_DETERMINISTIC(scalarResult, result, epsilon);
	EXPECT_EQ_DETERMINISTIC(-53.239999999999981, result, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, DeterminantFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	double result = dsMatrix33xd_determinantFMA2(&matrix);

	EXPECT_NEAR(-53.239999999999981, result, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, DeterminantSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	double scalarResult = dsMatrix33_determinant(matrix);
	double result = dsMatrix33xd_determinantSIMD4(&matrix);

	EXPECT_EQ_DETERMINISTIC(scalarResult, result, epsilon);
	EXPECT_EQ_DETERMINISTIC(-53.239999999999981, result, epsilon);
}

TEST(Matrix33xfTest, FastInvertSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf rotate;
	dsMatrix33xf_makeRotate(&rotate, dsDegreesToRadiansf(30.0f));

	dsMatrix33xf translate;
	dsMatrix33xf_makeTranslate(&translate, 1.2f, -3.4f);

	// Also verify determinism of affine multiply.
	dsMatrix33xf scalarMatrix, matrix;
	dsMatrix33xf_affineMul(&scalarMatrix, &translate, &rotate);
	dsMatrix33xf_affineMulSIMD(&matrix, &translate, &rotate);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	dsMatrix33xf scalarInverse, inverse;
	dsMatrix33_fastInvert(scalarInverse, scalarMatrix);
	dsMatrix33xf_fastInvertSIMD(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.866025388f, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.5f, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0f, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.5f, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.866025388f, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0f, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.660769582f, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(3.54448652f, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0f, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, FastInvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf rotate;
	dsMatrix33xf_makeRotate(&rotate, dsDegreesToRadiansf(30.0f));

	dsMatrix33xf translate;
	dsMatrix33xf_makeTranslate(&translate, 1.2f, -3.4f);

	dsMatrix33xf matrix;
	dsMatrix33xf_mulFMA(&matrix, &translate, &rotate);

	dsMatrix33xf inverse;
	dsMatrix33xf_fastInvertFMA(&inverse, &matrix);

	dsMatrix33xf result;
	dsMatrix33xf_mulFMA(&result, &inverse, &matrix);

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
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, FastInvertSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	// Also verify determinism of affine multiply.
	dsMatrix33xd scalarMatrix, matrix;
	dsMatrix33xd_affineMul(&scalarMatrix, &translate, &rotate);
	dsMatrix33xd_affineMulSIMD2(&matrix, &translate, &rotate);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	dsMatrix33xd scalarInverse, inverse;
	dsMatrix33_fastInvert(scalarInverse, scalarMatrix);
	dsMatrix33xd_fastInvertSIMD2(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.86602540378443871, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.49999999999999994, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.49999999999999994, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443871, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.66076951545867324, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(3.544486372867091, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, FastInvertFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	dsMatrix33xd matrix;
	dsMatrix33xd_affineMulFMA2(&matrix, &translate, &rotate);

	dsMatrix33xd inverse;
	dsMatrix33xd_fastInvertFMA2(&inverse, &matrix);

	EXPECT_NEAR(0.86602540378443871, inverse.values[0][0], epsilon);
	EXPECT_NEAR(-0.49999999999999994, inverse.values[0][1], epsilon);
	EXPECT_NEAR(0.0, inverse.values[0][2], epsilon);

	EXPECT_NEAR(0.49999999999999994, inverse.values[1][0], epsilon);
	EXPECT_NEAR(0.86602540378443871, inverse.values[1][1], epsilon);
	EXPECT_NEAR(0.0, inverse.values[1][2], epsilon);

	EXPECT_NEAR(0.66076951545867324, inverse.values[2][0], epsilon);
	EXPECT_NEAR(3.544486372867091, inverse.values[2][1], epsilon);
	EXPECT_NEAR(1.0, inverse.values[2][2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, FastInvertSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	DS_ALIGN(32) dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	// Also verify determinism of affine multiply.
	DS_ALIGN(32) dsMatrix33xd scalarMatrix, matrix;
	dsMatrix33xd_affineMul(&scalarMatrix, &translate, &rotate);
	dsMatrix33xd_affineMulSIMD4(&matrix, &translate, &rotate);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	DS_ALIGN(32) dsMatrix33xd scalarInverse, inverse;
	dsMatrix33_fastInvert(scalarInverse, scalarMatrix);
	dsMatrix33xd_fastInvertSIMD4(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.86602540378443871, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.49999999999999994, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.49999999999999994, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443871, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.66076951545867324, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(3.544486372867091, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0, inverse.values[2][2], epsilon);
}

TEST(Matrix33xfTest, AffineInvertSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf rotate;
	dsMatrix33xf_makeRotate(&rotate, dsDegreesToRadiansf(30.0f));

	dsMatrix33xf translate;
	dsMatrix33xf_makeTranslate(&translate, 1.2f, -3.4f);

	dsMatrix33xf scale;
	dsMatrix33xf_makeScale(&scale, -2.1f, 4.3f);

	// Also verify determinism of affine multiply.
	dsMatrix33xf temp, scalarMatrix, matrix;
	dsMatrix33xf_affineMul(&temp, &scale, &rotate);
	dsMatrix33xf_affineMul(&scalarMatrix, &translate, &temp);

	dsMatrix33xf_affineMulSIMD(&temp, &scale, &rotate);
	dsMatrix33xf_affineMulSIMD(&matrix, &translate, &temp);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	dsMatrix33xf scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xf_affineInvert(&scalarInverse, &scalarMatrix);
	dsMatrix33xf_affineInvertSIMD(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.412393063f, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.238095254f, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0f, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.116279066f, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.201401249f, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0f, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.890220523f, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.399049938f, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0f, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, AffineInvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf rotate;
	dsMatrix33xf_makeRotate(&rotate, dsDegreesToRadiansf(30.0f));

	dsMatrix33xf translate;
	dsMatrix33xf_makeTranslate(&translate, 1.2f, -3.4f);

	dsMatrix33xf scale;
	dsMatrix33xf_makeScale(&scale, -2.1f, 4.3f);

	dsMatrix33xf temp;
	dsMatrix33xf_affineMulFMA(&temp, &scale, &rotate);

	dsMatrix33xf matrix;
	dsMatrix33xf_affineMulFMA(&matrix, &translate, &temp);

	dsMatrix33xf inverse;
	dsMatrix33xf_affineInvertFMA(&inverse, &matrix);

	dsMatrix33xf result;
	dsMatrix33xf_affineMulFMA(&result, &inverse, &matrix);

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
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, AffineInvertSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	dsMatrix33xd scale;
	dsMatrix33xd_makeScale(&scale, -2.1, 4.3);

	// Also verify determinism of affine multiply.
	dsMatrix33xd temp, scalarMatrix, matrix;
	dsMatrix33xd_affineMul(&temp, &scale, &rotate);
	dsMatrix33xd_affineMul(&scalarMatrix, &translate, &temp);

	dsMatrix33xd_affineMulSIMD2(&temp, &scale, &rotate);
	dsMatrix33xd_affineMulSIMD2(&matrix, &translate, &temp);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	dsMatrix33xd scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_affineInvert(&scalarInverse, &scalarMatrix);
	dsMatrix33xd_affineInvertSIMD2(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.41239304942116128, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.23809523809523805, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.11627906976744184, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20140125669405554, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.89022049651469581, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.39904998704550315, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, AffineInvertFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;

	dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	dsMatrix33xd scale;
	dsMatrix33xd_makeScale(&scale, -2.1, 4.3);

	dsMatrix33xd temp, matrix;
	dsMatrix33xd_affineMulFMA2(&temp, &scale, &rotate);
	dsMatrix33xd_affineMulFMA2(&matrix, &translate, &temp);

	dsMatrix33xd inverse;
	dsMatrix33xd_affineInvertFMA2(&inverse, &matrix);

	EXPECT_NEAR(-0.41239304942116128, inverse.values[0][0], epsilon);
	EXPECT_NEAR(0.23809523809523805, inverse.values[0][1], epsilon);
	EXPECT_NEAR(0.0, inverse.values[0][2], epsilon);

	EXPECT_NEAR(0.11627906976744184, inverse.values[1][0], epsilon);
	EXPECT_NEAR(0.20140125669405554, inverse.values[1][1], epsilon);
	EXPECT_NEAR(0.0, inverse.values[1][2], epsilon);

	EXPECT_NEAR(0.89022049651469581, inverse.values[2][0], epsilon);
	EXPECT_NEAR(0.39904998704550315, inverse.values[2][1], epsilon);
	EXPECT_NEAR(1.0, inverse.values[2][2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, AffineInvertSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	DS_ALIGN(32) dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	DS_ALIGN(32) dsMatrix33xd scale;
	dsMatrix33xd_makeScale(&scale, -2.1, 4.3);

	// Also verify determinism of affine multiply.
	DS_ALIGN(32) dsMatrix33xd temp, scalarMatrix, matrix;
	dsMatrix33xd_affineMul(&temp, &scale, &rotate);
	dsMatrix33xd_affineMul(&scalarMatrix, &translate, &temp);

	dsMatrix33xd_affineMulSIMD4(&temp, &scale, &rotate);
	dsMatrix33xd_affineMulSIMD4(&matrix, &translate, &temp);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);

	DS_ALIGN(32) dsMatrix33xd scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_affineInvert(&scalarInverse, &scalarMatrix);
	dsMatrix33xd_affineInvertSIMD4(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.41239304942116128, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.23809523809523805, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.11627906976744184, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20140125669405554, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.89022049651469581, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.39904998704550315, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(1.0, inverse.values[2][2], epsilon);
}

TEST(Matrix33xfTest, InvertSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xf scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xf_invert(&scalarInverse, &matrix);
	dsMatrix33xf_invertSIMD(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-1.11157048f, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0909091309f, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.747934043f, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.838843107f, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.181818217f, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.566115856f, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.181818232f, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.090909116f, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.272727311f, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xfTest, InvertFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xf matrix =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xf inverse;
	dsMatrix33xf_invertFMA(&inverse, &matrix);

	dsMatrix33xf result;
	dsMatrix33xf_mulFMA(&result, &inverse, &matrix);

	EXPECT_NEAR(-1.11157024793389f, inverse.values[0][0], epsilon);
	EXPECT_NEAR(0.090909090909f, inverse.values[0][1], epsilon);
	EXPECT_NEAR(0.74793388429752f, inverse.values[0][2], epsilon);

	EXPECT_NEAR(-0.83884297520661f, inverse.values[1][0], epsilon);
	EXPECT_NEAR(0.181818181818182f, inverse.values[1][1], epsilon);
	EXPECT_NEAR(0.56611570247934f, inverse.values[1][2], epsilon);

	EXPECT_NEAR(-0.18181818181818f, inverse.values[2][0], epsilon);
	EXPECT_NEAR(0.090909090909091f, inverse.values[2][1], epsilon);
	EXPECT_NEAR(0.272727272727273f, inverse.values[2][2], epsilon);

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
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, InvertSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xd scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_invert(&scalarInverse, &matrix);
	dsMatrix33xd_invertSIMD2(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-1.1115702479338847, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.090909090909090967, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.74793388429752106, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.83884297520661188, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.18181818181818188, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.56611570247933907, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.18181818181818193, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.090909090909090912, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.27272727272727276, inverse.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Matrix33xdTest, InvertFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;

	dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xd inverse;
	dsMatrix33xd_invertFMA2(&inverse, &matrix);

	EXPECT_NEAR(-1.1115702479338847, inverse.values[0][0], epsilon);
	EXPECT_NEAR(0.090909090909090967, inverse.values[0][1], epsilon);
	EXPECT_NEAR(0.74793388429752106, inverse.values[0][2], epsilon);

	EXPECT_NEAR(-0.83884297520661188, inverse.values[1][0], epsilon);
	EXPECT_NEAR(0.18181818181818188, inverse.values[1][1], epsilon);
	EXPECT_NEAR(0.56611570247933907, inverse.values[1][2], epsilon);

	EXPECT_NEAR(-0.18181818181818193, inverse.values[2][0], epsilon);
	EXPECT_NEAR(0.090909090909090912, inverse.values[2][1], epsilon);
	EXPECT_NEAR(0.27272727272727276, inverse.values[2][2], epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Matrix33xdTest, InvertSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd matrix =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	DS_ALIGN(32) dsMatrix33xd scalarInverse, inverse;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_invert(&scalarInverse, &matrix);
	dsMatrix33xd_invertSIMD4(&inverse, &matrix);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][0], inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][1], inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[0][2], inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][0], inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][1], inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[1][2], inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][0], inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][1], inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarInverse.values[2][2], inverse.values[2][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-1.1115702479338847, inverse.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.090909090909090967, inverse.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.74793388429752106, inverse.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.83884297520661188, inverse.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.18181818181818188, inverse.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.56611570247933907, inverse.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.18181818181818193, inverse.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.090909090909090912, inverse.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.27272727272727276, inverse.values[2][2], epsilon);
}

TEST(Matrix33xfTest, InverseTransposeSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	const float epsilon = Matrix33xTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xf rotate;
	dsMatrix33xf_makeRotate(&rotate, dsDegreesToRadiansf(30.0f));

	dsMatrix33xf translate;
	dsMatrix33xf_makeTranslate(&translate, 1.2f, -3.4f);

	dsMatrix33xf scale;
	dsMatrix33xf_makeScale(&scale, -2.1f, 4.3f);

	dsMatrix33xf temp;
	dsMatrix33xf_mulSIMD(&temp, &scale, &rotate);

	dsMatrix33xf matrix;
	dsMatrix33xf_mulSIMD(&matrix, &translate, &temp);

	dsMatrix22f scalarInverseTranspose, inverseTranspose;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xf_inverseTranspose(&scalarInverseTranspose, &matrix);
	dsMatrix33xf_inverseTransposeSIMD(&inverseTranspose, &matrix);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][1], inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][1], inverseTranspose.values[1][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.412393063f, inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.116279066f, inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.238095254f, inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.201401249f, inverseTranspose.values[1][1], epsilon);
}

TEST(Matrix33xdTest, InverseTransposeSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	dsMatrix33xd scale;
	dsMatrix33xd_makeScale(&scale, -2.1, 4.3);

	dsMatrix33xd temp;
	dsMatrix33xd_mulSIMD2(&temp, &scale, &rotate);

	dsMatrix33xd matrix;
	dsMatrix33xd_mulSIMD2(&matrix, &translate, &temp);

	dsMatrix22d scalarInverseTranspose, inverseTranspose;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_inverseTranspose(&scalarInverseTranspose, &matrix);
	dsMatrix33xd_inverseTransposeSIMD2(&inverseTranspose, &matrix);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][1], inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][1], inverseTranspose.values[1][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.41239304942116128, inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.11627906976744184, inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.23809523809523805, inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20140125669405554, inverseTranspose.values[1][1], epsilon);
}

TEST(Matrix33xdTest, InverseTransposeSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	const double epsilon = Matrix33xTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	DS_ALIGN(32) dsMatrix33xd rotate;
	dsMatrix33xd_makeRotate(&rotate, dsDegreesToRadiansd(30.0));

	DS_ALIGN(32) dsMatrix33xd translate;
	dsMatrix33xd_makeTranslate(&translate, 1.2, -3.4);

	DS_ALIGN(32) dsMatrix33xd scale;
	dsMatrix33xd_makeScale(&scale, -2.1, 4.3);

	DS_ALIGN(32) dsMatrix33xd temp;
	dsMatrix33xd_mulSIMD2(&temp, &scale, &rotate);

	DS_ALIGN(32) dsMatrix33xd matrix;
	dsMatrix33xd_mulSIMD2(&matrix, &translate, &temp);

	dsMatrix22d scalarInverseTranspose, inverseTranspose;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsMatrix33xd_inverseTranspose(&scalarInverseTranspose, &matrix);
	dsMatrix33xd_inverseTransposeSIMD4(&inverseTranspose, &matrix);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[0][1], inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarInverseTranspose.values[1][1], inverseTranspose.values[1][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.41239304942116128, inverseTranspose.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.11627906976744184, inverseTranspose.values[0][1], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.23809523809523805, inverseTranspose.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20140125669405554, inverseTranspose.values[1][1], epsilon);
}

#endif // DS_HAS_SIMD

TEST(Matrix33x, ConvertFloatToDouble)
{
	dsMatrix33xf matrixf =
	{{
		{0.1f, -2.3f, 4.5f, 1.0f},
		{-6.7f, 8.9f, -0.1f, 2.0f},
		{2.3f, -4.5f, 6.7f, 3.0f}
	}};

	dsMatrix33xd matrixd;
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

TEST(Matrix33x, ConvertDoubleToFloat)
{
	dsMatrix33xd matrixd =
	{{
		{0.1, -2.3, 4.5, 1.0},
		{-6.7, 8.9, -0.1, 2.0},
		{2.3, -4.5, 6.7, 3.0}
	}};

	dsMatrix33xf matrixf;
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
