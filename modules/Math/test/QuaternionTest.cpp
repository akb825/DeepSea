/*
 * Copyright 2020-2026 Aaron Barany
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
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct QuaternionTypeSelector;

template <>
struct QuaternionTypeSelector<float>
{
	typedef dsQuaternion4f QuaternionType;
	typedef dsVector3f Vector3Type;
	typedef dsVector3xf Vector3xType;
	typedef dsMatrix33f Matrix33Type;
	typedef dsMatrix33xf Matrix33xType;
	typedef dsMatrix44f Matrix44Type;
	static const float epsilon;
};

template <>
struct QuaternionTypeSelector<double>
{
	typedef dsQuaternion4d QuaternionType;
	typedef dsVector3d Vector3Type;
	typedef dsVector3xd Vector3xType;
	typedef dsMatrix33d Matrix33Type;
	typedef dsMatrix33xd Matrix33xType;
	typedef dsMatrix44d Matrix44Type;
	static const double epsilon;
};


const float QuaternionTypeSelector<float>::epsilon = 1e-5f;
const double QuaternionTypeSelector<double>::epsilon = 1e-13;

template <typename T>
class QuaternionTest : public testing::Test
{
};

using QuaternionTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(QuaternionTest, QuaternionTypes);

inline void dsQuaternion4_fromEulerAngles(dsQuaternion4f* result, float x, float y, float z)
{
	dsQuaternion4f_fromEulerAngles(result, x, y, z);
}

inline void dsQuaternion4_fromEulerAngles(dsQuaternion4d* result, double x, double y, double z)
{
	dsQuaternion4d_fromEulerAngles(result, x, y, z);
}

inline void dsQuaternion4_fromAxisAngle(
	dsQuaternion4f* result, const dsVector3f* axis, float angle)
{
	dsQuaternion4f_fromAxisAngle(result, axis, angle);
}

inline void dsQuaternion4_fromAxisAngle(
	dsQuaternion4d* result, const dsVector3d* axis, double angle)
{
	dsQuaternion4d_fromAxisAngle(result, axis, angle);
}

inline void dsQuaternion4_fromMatrix33(dsQuaternion4f* result, const dsMatrix33f* matrix)
{
	dsQuaternion4f_fromMatrix33(result, matrix);
}

inline void dsQuaternion4_fromMatrix33(dsQuaternion4d* result, const dsMatrix33d* matrix)
{
	dsQuaternion4d_fromMatrix33(result, matrix);
}

inline void dsQuaternion4_fromMatrix33x(dsQuaternion4f* result, const dsMatrix33xf* matrix)
{
	dsQuaternion4f_fromMatrix33x(result, matrix);
}

inline void dsQuaternion4_fromMatrix33x(dsQuaternion4d* result, const dsMatrix33xd* matrix)
{
	dsQuaternion4d_fromMatrix33x(result, matrix);
}

inline void dsQuaternion4_fromMatrix44(dsQuaternion4f* result, const dsMatrix44f* matrix)
{
	dsQuaternion4f_fromMatrix44(result, matrix);
}

inline void dsQuaternion4_fromMatrix44(dsQuaternion4d* result, const dsMatrix44d* matrix)
{
	dsQuaternion4d_fromMatrix44(result, matrix);
}

inline float dsQuaternion4_getXAngle(const dsQuaternion4f* a)
{
	return dsQuaternion4f_getXAngle(a);
}

inline double dsQuaternion4_getXAngle(const dsQuaternion4d* a)
{
	return dsQuaternion4d_getXAngle(a);
}

inline float dsQuaternion4_getYAngle(const dsQuaternion4f* a)
{
	return dsQuaternion4f_getYAngle(a);
}

inline double dsQuaternion4_getYAngle(const dsQuaternion4d* a)
{
	return dsQuaternion4d_getYAngle(a);
}

inline float dsQuaternion4_getZAngle(const dsQuaternion4f* a)
{
	return dsQuaternion4f_getZAngle(a);
}

inline double dsQuaternion4_getZAngle(const dsQuaternion4d* a)
{
	return dsQuaternion4d_getZAngle(a);
}

inline void dsQuaternion4_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a)
{
	return dsQuaternion4f_getRotationAxis(result, a);
}

inline void dsQuaternion4_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a)
{
	return dsQuaternion4d_getRotationAxis(result, a);
}

inline void dsQuaternion4_getRotationAxis3x(dsVector3xf* result, const dsQuaternion4f* a)
{
	return dsQuaternion4f_getRotationAxis3x(result, a);
}

inline void dsQuaternion4_getRotationAxis3x(dsVector3xd* result, const dsQuaternion4d* a)
{
	return dsQuaternion4d_getRotationAxis3x(result, a);
}

inline float dsQuaternion4_getAxisAngle(const dsQuaternion4f* a)
{
	return dsQuaternion4f_getAxisAngle(a);
}

inline double dsQuaternion4_getAxisAngle(const dsQuaternion4d* a)
{
	return dsQuaternion4d_getAxisAngle(a);
}

inline void dsQuaternion4_toMatrix33(dsMatrix33f* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_toMatrix33(result, a);
}

inline void dsQuaternion4_toMatrix33(dsMatrix33d* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_toMatrix33(result, a);
}

inline void dsQuaternion4_toMatrix33x(dsMatrix33xf* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_toMatrix33x(result, a);
}

inline void dsQuaternion4_toMatrix33x(dsMatrix33xd* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_toMatrix33x(result, a);
}

inline void dsQuaternion4_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_toMatrix44(result, a);
}

inline void dsQuaternion4_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_toMatrix44(result, a);
}

inline void dsQuaternion4_normalize(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_normalize(result, a);
}

inline void dsQuaternion4_normalize(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_normalize(result, a);
}

inline void dsQuaternion4_rotate(dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v)
{
	dsQuaternion4f_rotate(result, a, v);
}

inline void dsQuaternion4_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v)
{
	dsQuaternion4d_rotate(result, a, v);
}

inline void dsQuaternion4_rotate3x(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v)
{
	dsQuaternion4f_rotate3x(result, a, v);
}

inline void dsQuaternion4_rotate3x(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v)
{
	dsQuaternion4d_rotate3x(result, a, v);
}

inline void dsQuaternion4_slerp(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t)
{
	dsQuaternion4f_slerp(result, a, b, t);
}

inline void dsQuaternion4_slerp(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t)
{
	dsQuaternion4d_slerp(result, a, b, t);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33f* result, float x, float y, float z)
{
	dsMatrix33f_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33d* result, double x, double y, double z)
{
	dsMatrix33d_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33_makeRotate3DAxisAngle(
	dsMatrix33f* result, const dsVector3f* axis, float angle)
{
	dsMatrix33f_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix33_makeRotate3DAxisAngle(
	dsMatrix33d* result, const dsVector3d* axis, double angle)
{
	dsMatrix33d_makeRotate3DAxisAngle(result, axis, angle);
}

inline void dsMatrix44_makeRotate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeRotate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeRotate(result, x, y, z);
}

inline void dsVector3_normalize(dsVector3f* result, const dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, const dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

TYPED_TEST(QuaternionTest, EulerAngles)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	QuaternionType q;
	dsQuaternion4_fromEulerAngles(&q, x, y, z);

	EXPECT_NEAR(x, dsQuaternion4_getXAngle(&q), epsilon);
	EXPECT_NEAR(y, dsQuaternion4_getYAngle(&q), epsilon);
	EXPECT_NEAR(z, dsQuaternion4_getZAngle(&q), epsilon);
}

TYPED_TEST(QuaternionTest, AxisAngle)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3xType Vector3xType;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	Vector3Type axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3_normalize(&axis, &axis);
	auto theta = TypeParam(M_PI/3);

	QuaternionType q;
	dsQuaternion4_fromAxisAngle(&q, &axis, theta);

	Vector3Type qAxis;
	dsQuaternion4_getRotationAxis(&qAxis, &q);
	EXPECT_NEAR(axis.x, qAxis.x, epsilon);
	EXPECT_NEAR(axis.y, qAxis.y, epsilon);
	EXPECT_NEAR(axis.z, qAxis.z, epsilon);

	Vector3xType qAxis3x;
	dsQuaternion4_getRotationAxis3x(&qAxis3x, &q);
	EXPECT_EQ(qAxis.x, qAxis3x.x);
	EXPECT_EQ(qAxis.y, qAxis3x.y);
	EXPECT_EQ(qAxis.z, qAxis3x.z);

	EXPECT_NEAR(theta, dsQuaternion4_getAxisAngle(&q), epsilon);

	dsQuaternion4_identityRotation(q);
	dsQuaternion4_getRotationAxis(&qAxis, &q);
	EXPECT_EQ(0, qAxis.x);
	EXPECT_EQ(0, qAxis.y);
	EXPECT_EQ(1, qAxis.z);

	EXPECT_NEAR(0, dsQuaternion4_getAxisAngle(&q), epsilon);

	q.r = -q.r;
	dsQuaternion4_getRotationAxis(&qAxis, &q);
	EXPECT_EQ(0, qAxis.x);
	EXPECT_EQ(0, qAxis.y);
	EXPECT_EQ(-1, qAxis.z);

	EXPECT_NEAR(0, dsQuaternion4_getAxisAngle(&q), epsilon);
}

TYPED_TEST(QuaternionTest, Matrix33)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Matrix33Type m;
	dsMatrix33_makeRotate3D(&m, x, y, z);

	QuaternionType q;
	dsQuaternion4_fromMatrix33(&q, &m);

	Matrix33Type qm;
	dsQuaternion4_toMatrix33(&qm, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(m.values[i][j], qm.values[i][j], epsilon);
	}
}

TYPED_TEST(QuaternionTest, Matrix33x)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33xType Matrix33xType;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Matrix33Type m;
	dsMatrix33_makeRotate3D(&m, x, y, z);

	Matrix33xType m3x;
	dsMatrix33_copy(m3x, m);

	QuaternionType q;
	dsQuaternion4_fromMatrix33x(&q, &m3x);

	Matrix33xType qm;
	dsQuaternion4_toMatrix33x(&qm, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(m.values[i][j], qm.values[i][j], epsilon);
	}
}

TYPED_TEST(QuaternionTest, Matrix44)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Matrix44Type m;
	dsMatrix44_makeRotate(&m, x, y, z);

	QuaternionType q;
	dsQuaternion4_fromMatrix44(&q, &m);

	Matrix44Type qm;
	dsQuaternion4_toMatrix44(&qm, &q);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_NEAR(m.values[i][j], qm.values[i][j], epsilon);
	}
}

TYPED_TEST(QuaternionTest, Multiply)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Vector3Type axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3_normalize(&axis, &axis);
	auto theta = TypeParam(M_PI/3);

	Matrix33Type ma, mb, mab;
	dsMatrix33_makeRotate3D(&ma, x, y, z);
	dsMatrix33_makeRotate3DAxisAngle(&mb, &axis, theta);
	dsMatrix33_mul(mab, ma, mb);

	QuaternionType qa, qb, qab;
	dsQuaternion4_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4_mul(qab, qa, qb);

	Matrix33Type qm;
	dsQuaternion4_toMatrix33(&qm, &qab);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(mab.values[i][j], qm.values[i][j], epsilon);
	}
}

TYPED_TEST(QuaternionTest, Conjugate)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	QuaternionType q, invQ, ident;
	dsQuaternion4_fromEulerAngles(&q, x, y, z);
	dsQuaternion4_conjugate(invQ, q);
	dsQuaternion4_mul(ident, q, invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}

TYPED_TEST(QuaternionTest, Rotate)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Matrix33Type m;
	dsMatrix33_makeRotate3D(&m, x, y, z);

	QuaternionType q;
	dsQuaternion4_fromMatrix33(&q, &m);

	Vector3Type v = {{TypeParam(1.2), TypeParam(-3.4), TypeParam(5.6)}};
	Vector3Type vm, vq;
	dsQuaternion4_rotate(&vq, &q, &v);
	dsMatrix33_transform(vm, m, v);

	EXPECT_NEAR(vm.x, vq.x, epsilon);
	EXPECT_NEAR(vm.y, vq.y, epsilon);
	EXPECT_NEAR(vm.z, vq.z, epsilon);
}

TYPED_TEST(QuaternionTest, Rotate3x)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename QuaternionTypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	auto x = TypeParam(M_PI*3/4);
	auto y = TypeParam(-M_PI/3);
	auto z = TypeParam(-M_PI/5);

	Matrix33Type m;
	dsMatrix33_makeRotate3D(&m, x, y, z);

	QuaternionType q;
	dsQuaternion4_fromMatrix33(&q, &m);

	Vector3xType v = {{TypeParam(1.2), TypeParam(-3.4), TypeParam(5.6), 7}};
	Vector3xType vm, vq;
	dsQuaternion4_rotate3x(&vq, &q, &v);
	dsMatrix33_transform(vm, m, v);

	EXPECT_NEAR(vm.x, vq.x, epsilon);
	EXPECT_NEAR(vm.y, vq.y, epsilon);
	EXPECT_NEAR(vm.z, vq.z, epsilon);
}

TYPED_TEST(QuaternionTest, Slerp)
{
	typedef typename QuaternionTypeSelector<TypeParam>::QuaternionType QuaternionType;
	typedef typename QuaternionTypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = QuaternionTypeSelector<TypeParam>::epsilon;

	Vector3Type axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3_normalize(&axis, &axis);
	auto theta0 = TypeParam(-M_PI/3);
	auto theta1 = TypeParam(M_PI/2);
	auto t = TypeParam(0.37);

	QuaternionType q0, q1, q01, sq01;
	dsQuaternion4_fromAxisAngle(&q0, &axis, theta0);
	dsQuaternion4_fromAxisAngle(&q1, &axis, theta1);
	dsQuaternion4_fromAxisAngle(&q01, &axis, dsLerp(theta0, theta1, t));
	dsQuaternion4_slerp(&sq01, &q0, &q1, t);

	EXPECT_NEAR(q01.r, sq01.r, epsilon);
	EXPECT_NEAR(q01.i, sq01.i, epsilon);
	EXPECT_NEAR(q01.j, sq01.j, epsilon);
	EXPECT_NEAR(q01.k, sq01.k, epsilon);

	theta0 = TypeParam(M_PI*4/3);
	theta1 = TypeParam(-M_PI);

	dsQuaternion4_fromAxisAngle(&q0, &axis, theta0);
	dsQuaternion4_fromAxisAngle(&q1, &axis, theta1);
	theta1 = 2*TypeParam(M_PI) + theta1; // Wraps around the other way.
	dsQuaternion4_fromAxisAngle(&q01, &axis, dsLerp(theta0, theta1, t));
	dsQuaternion4_slerp(&sq01, &q0, &q1, t);

	EXPECT_NEAR(q01.r, sq01.r, epsilon);
	EXPECT_NEAR(q01.i, sq01.i, epsilon);
	EXPECT_NEAR(q01.j, sq01.j, epsilon);
	EXPECT_NEAR(q01.k, sq01.k, epsilon);
}

#if DS_HAS_SIMD

TEST(Quaternion4fTest, MultiplySIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsVector3f axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3f_normalize(&axis, &axis);
	float theta = M_PIf/3;

	dsQuaternion4f qa, qb, qab, qabRef;
	dsQuaternion4f_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4f_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4f_mulSIMD(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_EQ_DETERMINISTIC(qabRef.i, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.j, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.k, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.r, qab.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.70289826f, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.686455429f, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.18280144f, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0360077024f, qab.r, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4fTest, MultiplyFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsVector3f axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3f_normalize(&axis, &axis);
	float theta = M_PIf/3;

	dsQuaternion4f qa, qb, qab, qabRef;
	dsQuaternion4f_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4f_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4f_mulFMA(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_NEAR(qabRef.i, qab.i, epsilon);
	EXPECT_NEAR(qabRef.j, qab.j, epsilon);
	EXPECT_NEAR(qabRef.k, qab.k, epsilon);
	EXPECT_NEAR(qabRef.r, qab.r, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, MultiplySIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsVector3d axis = {{1.2, -3.4, 2.1}};
	dsVector3d_normalize(&axis, &axis);
	double theta = M_PI/3;

	dsQuaternion4d qa, qb, qab, qabRef;
	dsQuaternion4d_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4d_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4d_mulSIMD2(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_EQ_DETERMINISTIC(qabRef.i, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.j, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.k, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.r, qab.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.70289830380604112, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.6864554188984564, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.1828014693872537, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.036007709749387995, qab.r, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4dTest, MultiplyFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsVector3d axis = {{1.2, -3.4, 2.1}};
	dsVector3d_normalize(&axis, &axis);
	double theta = M_PI/3;

	dsQuaternion4d qa, qb, qab, qabRef;
	dsQuaternion4d_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4d_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4d_mulFMA2(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_NEAR(qabRef.i, qab.i, epsilon);
	EXPECT_NEAR(qabRef.j, qab.j, epsilon);
	EXPECT_NEAR(qabRef.k, qab.k, epsilon);
	EXPECT_NEAR(qabRef.r, qab.r, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, MultiplySIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsVector3d axis = {{1.2, -3.4, 2.1}};
	dsVector3d_normalize(&axis, &axis);
	double theta = M_PI/3;

	DS_ALIGN(32) dsQuaternion4d qa, qb, qab, qabRef;
	dsQuaternion4d_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4d_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4d_mulSIMD4(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_EQ_DETERMINISTIC(qabRef.i, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.j, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.k, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(qabRef.r, qab.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.70289830380604112, qab.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.6864554188984564, qab.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.1828014693872537, qab.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.036007709749387995, qab.r, epsilon);
}

TEST(Quaternion4fTest, ConjugateSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q, invQ, ident;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);
	dsQuaternion4f_conjugateSIMD(&invQ, &q);
	dsQuaternion4f_mulSIMD(&ident, &q, &invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}

TEST(Quaternion4dTest, ConjugateSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q, invQ, ident;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);
	dsQuaternion4d_conjugateSIMD2(&invQ, &q);
	dsQuaternion4d_mulSIMD2(&ident, &q, &invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}

TEST(Quaternion4dTest, ConjugateSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	DS_ALIGN(32) dsQuaternion4d q, invQ, ident;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);
	dsQuaternion4d_conjugateSIMD4(&invQ, &q);
	dsQuaternion4d_mulSIMD4(&ident, &q, &invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}

TEST(Quaternion4fTest, ToMatrix33SIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsMatrix33xf qm;
	dsQuaternion4f_toMatrix33SIMD(&qm, &q);

	dsMatrix33f qmScalar;
	dsQuaternion4f_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}

	EXPECT_EQ_DETERMINISTIC(0.404508471f, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.293892652f, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.866025329f, qm.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.911046624f, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.212117672f, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.353553385f, qm.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.0797926486f, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.932004809f, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.353553295f, qm.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4fTest, ToMatrix33FMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsMatrix33xf qm;
	dsQuaternion4f_toMatrix33FMA(&qm, &q);

	dsMatrix33f qmScalar;
	dsQuaternion4f_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, ToMatrix33SIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsMatrix33xd qm;
	dsQuaternion4d_toMatrix33SIMD2(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon) << i << ", " << j;
	}

	EXPECT_EQ_DETERMINISTIC(0.40450849718747384, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.29389262614623657, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443849, qm.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.91104664514213074, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.21211791620527531, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.35355339059327384, qm.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.079792769587223922, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.93200488943009308, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.35355339059327373, qm.values[2][2], epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4dTest, ToMatrix33FMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsMatrix33xd qm;
	dsQuaternion4d_toMatrix33SIMD2(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, ToMatrix33SIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	DS_ALIGN(32) dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	DS_ALIGN(32) dsMatrix33xd qm;
	dsQuaternion4d_toMatrix33SIMD4(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon) << i << ", " << j;
	}

	EXPECT_EQ_DETERMINISTIC(0.40450849718747384, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.29389262614623657, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443849, qm.values[0][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.91104664514213074, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.21211791620527531, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.35355339059327384, qm.values[1][2], epsilon);

	EXPECT_EQ_DETERMINISTIC(0.079792769587223922, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.93200488943009308, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.35355339059327373, qm.values[2][2], epsilon);
}

TEST(Quaternion4fTest, ToMatrix44SIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsMatrix44f qm;
	dsQuaternion4f_toMatrix44SIMD(&qm, &q);

	dsMatrix33f qmScalar;
	dsQuaternion4f_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}

	EXPECT_EQ_DETERMINISTIC(0.404508471f, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.293892652f, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.866025329f, qm.values[0][2], epsilon);
	EXPECT_EQ(0.0f, qm.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.911046624f, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.212117672f, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.353553385f, qm.values[1][2], epsilon);
	EXPECT_EQ(0.0f, qm.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.0797926486f, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.932004809f, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.353553295f, qm.values[2][2], epsilon);
	EXPECT_EQ(0.0f, qm.values[2][3]);

	EXPECT_EQ(0.0f, qm.values[3][0]);
	EXPECT_EQ(0.0f, qm.values[3][1]);
	EXPECT_EQ(0.0f, qm.values[3][2]);
	EXPECT_EQ(1.0f, qm.values[3][3]);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4fTest, ToMatrix44FMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsMatrix44f qm;
	dsQuaternion4f_toMatrix44FMA(&qm, &q);

	dsMatrix33f qmScalar;
	dsQuaternion4f_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}

	EXPECT_EQ(0.0f, qm.values[0][3]);
	EXPECT_EQ(0.0f, qm.values[1][3]);
	EXPECT_EQ(0.0f, qm.values[2][3]);

	EXPECT_EQ(0.0f, qm.values[3][0]);
	EXPECT_EQ(0.0f, qm.values[3][1]);
	EXPECT_EQ(0.0f, qm.values[3][2]);
	EXPECT_EQ(1.0f, qm.values[3][3]);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, ToMatrix44SIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsMatrix44d qm;
	dsQuaternion4d_toMatrix44SIMD2(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon) << i << ", " << j;
	}

	EXPECT_EQ_DETERMINISTIC(0.40450849718747384, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.29389262614623657, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443849, qm.values[0][2], epsilon);
	EXPECT_EQ(0.0, qm.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.91104664514213074, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.21211791620527531, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.35355339059327384, qm.values[1][2], epsilon);
	EXPECT_EQ(0.0, qm.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.079792769587223922, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.93200488943009308, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.35355339059327373, qm.values[2][2], epsilon);
	EXPECT_EQ(0.0, qm.values[2][3]);

	EXPECT_EQ(0.0, qm.values[3][0]);
	EXPECT_EQ(0.0, qm.values[3][1]);
	EXPECT_EQ(0.0, qm.values[3][2]);
	EXPECT_EQ(1.0, qm.values[3][3]);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4dTest, ToMatrix44FMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsMatrix44d qm;
	dsQuaternion4d_toMatrix44SIMD2(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_NEAR(qmScalar.values[i][j], qm.values[i][j], epsilon);
	}

	EXPECT_EQ(0.0, qm.values[0][3]);
	EXPECT_EQ(0.0, qm.values[1][3]);
	EXPECT_EQ(0.0, qm.values[2][3]);

	EXPECT_EQ(0.0, qm.values[3][0]);
	EXPECT_EQ(0.0, qm.values[3][1]);
	EXPECT_EQ(0.0, qm.values[3][2]);
	EXPECT_EQ(1.0, qm.values[3][3]);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, ToMatrix44SIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	DS_ALIGN(32) dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	DS_ALIGN(32) dsMatrix44d qm;
	dsQuaternion4d_toMatrix44SIMD4(&qm, &q);

	dsMatrix33d qmScalar;
	dsQuaternion4d_toMatrix33(&qmScalar, &q);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ_DETERMINISTIC(qmScalar.values[i][j], qm.values[i][j], epsilon) << i << ", " << j;
	}

	EXPECT_EQ_DETERMINISTIC(0.40450849718747384, qm.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.29389262614623657, qm.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.86602540378443849, qm.values[0][2], epsilon);
	EXPECT_EQ(0.0, qm.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.91104664514213074, qm.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.21211791620527531, qm.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.35355339059327384, qm.values[1][2], epsilon);
	EXPECT_EQ(0.0, qm.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.079792769587223922, qm.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.93200488943009308, qm.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.35355339059327373, qm.values[2][2], epsilon);
	EXPECT_EQ(0.0, qm.values[2][3]);

	EXPECT_EQ(0.0, qm.values[3][0]);
	EXPECT_EQ(0.0, qm.values[3][1]);
	EXPECT_EQ(0.0, qm.values[3][2]);
	EXPECT_EQ(1.0, qm.values[3][3]);
}

TEST(Quaternion4fTest, RotateSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsVector3xf v = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector3xf vq;
	dsQuaternion4f_rotateSIMD(&vq, &q, &v);

	dsQuaternion4f vQuat = {{v.x, v.y, v.z, 0.0}};
	dsQuaternion4f vqRef, temp;
	dsQuaternion4f qConj;
	dsQuaternion4_conjugate(qConj, q);
	dsQuaternion4_mul(temp, vQuat, qConj);
	dsQuaternion4_mul(vqRef, q, temp);

	EXPECT_EQ_DETERMINISTIC(vqRef.i, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.j, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.k, vq.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(4.02980804f, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(-4.85069752f, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-2.14274979f, vq.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4fTest, RotateFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = QuaternionTypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	float x = M_PIf*3/4;
	float y = -M_PIf/3;
	float z = -M_PIf/5;

	dsQuaternion4f q;
	dsQuaternion4f_fromEulerAngles(&q, x, y, z);

	dsVector3xf v = {{1.2f, -3.4f, 5.6f, -7.8f}};
	dsVector3xf vq;
	dsQuaternion4f_rotateFMA(&vq, &q, &v);

	dsQuaternion4f vQuat = {{v.x, v.y, v.z, 0.0}};
	dsQuaternion4f vqRef, temp;
	dsQuaternion4f qConj;
	dsQuaternion4_conjugate(qConj, q);
	dsQuaternion4_mul(temp, vQuat, qConj);
	dsQuaternion4_mul(vqRef, q, temp);

	EXPECT_NEAR(vqRef.i, vq.x, epsilon);
	EXPECT_NEAR(vqRef.j, vq.y, epsilon);
	EXPECT_NEAR(vqRef.k, vq.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, RotateSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsVector3xd v = {{1.2, -3.4, 5.6, -7.8}};
	dsVector3xd vq;
	dsQuaternion4d_rotateSIMD2(&vq, &q, &v);

	dsQuaternion4d vQuat = {{v.x, v.y, v.z, 0.0}};
	dsQuaternion4d vqRef, temp;
	dsQuaternion4d qConj;
	dsQuaternion4_conjugate(qConj, q);
	dsQuaternion4_mul(temp, vQuat, qConj);
	dsQuaternion4_mul(vqRef, q, temp);

	EXPECT_EQ_DETERMINISTIC(vqRef.i, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.j, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.k, vq.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(4.0298082997966667, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(-4.8506976170860687, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-2.1427500307981391, vq.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(Quaternion4dTest, RotateFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	dsVector3xd v = {{1.2, -3.4, 5.6, -7.8}};
	dsVector3xd vq;
	dsQuaternion4d_rotateFMA2(&vq, &q, &v);

	dsQuaternion4d vQuat = {{v.x, v.y, v.z, 0.0}};
	dsQuaternion4d vqRef, temp;
	dsQuaternion4d qConj;
	dsQuaternion4_conjugate(qConj, q);
	dsQuaternion4_mul(temp, vQuat, qConj);
	dsQuaternion4_mul(vqRef, q, temp);

	EXPECT_NEAR(vqRef.i, vq.x, epsilon);
	EXPECT_NEAR(vqRef.j, vq.y, epsilon);
	EXPECT_NEAR(vqRef.k, vq.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(Quaternion4dTest, RotateSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = QuaternionTypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	DS_ALIGN(32) dsQuaternion4d q;
	dsQuaternion4d_fromEulerAngles(&q, x, y, z);

	DS_ALIGN(32) dsVector3xd v = {{1.2, -3.4, 5.6, -7.8}};
	DS_ALIGN(32) dsVector3xd vq;
	dsQuaternion4d_rotateSIMD4(&vq, &q, &v);

	dsQuaternion4d vQuat = {{v.x, v.y, v.z, 0.0}};
	dsQuaternion4d vqRef, temp;
	dsQuaternion4d qConj;
	dsQuaternion4_conjugate(qConj, q);
	dsQuaternion4_mul(temp, vQuat, qConj);
	dsQuaternion4_mul(vqRef, q, temp);

	EXPECT_EQ_DETERMINISTIC(vqRef.i, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.j, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(vqRef.k, vq.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(4.0298082997966667, vq.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(-4.8506976170860687, vq.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-2.1427500307981391, vq.z, epsilon);
}

#endif // DS_HAS_SIMD
