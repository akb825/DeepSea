/*
 * Copyright 2020 Aaron Barany
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
	typedef dsMatrix33f Matrix33Type;
	typedef dsMatrix44f Matrix44Type;
	static const float epsilon;
};

template <>
struct QuaternionTypeSelector<double>
{
	typedef dsQuaternion4d QuaternionType;
	typedef dsVector3d Vector3Type;
	typedef dsMatrix33d Matrix33Type;
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

inline void dsQuaternion4_fromAxisAngle(dsQuaternion4f* result, const dsVector3f* axis,
	float angle)
{
	dsQuaternion4f_fromAxisAngle(result, axis, angle);
}

inline void dsQuaternion4_fromAxisAngle(dsQuaternion4d* result, const dsVector3d* axis,
	double angle)
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

inline void dsQuaternion4_slerp(dsQuaternion4f* result, const dsQuaternion4f* a,
	const dsQuaternion4f* b, float t)
{
	dsQuaternion4f_slerp(result, a, b, t);
}

inline void dsQuaternion4_slerp(dsQuaternion4d* result, const dsQuaternion4d* a,
	const dsQuaternion4d* b, double t)
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

	EXPECT_NEAR(theta, dsQuaternion4_getAxisAngle(&q), epsilon);
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

TEST(Quaternion4fTest, MultiplySIMD)
{
	float epsilon = QuaternionTypeSelector<float>::epsilon;

	auto x = float(M_PI*3/4);
	auto y = float(-M_PI/3);
	auto z = float(-M_PI/5);

	dsVector3f axis = {{1.2f, -3.4f, 2.1f}};
	dsVector3_normalize(&axis, &axis);
	auto theta = float(M_PI/3);

	dsQuaternion4f qa, qb, qab, qabRef;
	dsQuaternion4_fromEulerAngles(&qa, x, y, z);
	dsQuaternion4_fromAxisAngle(&qb, &axis, theta);
	dsQuaternion4f_mul(&qab, &qa, &qb);
	dsQuaternion4_mul(qabRef, qa, qb);

	EXPECT_NEAR(qabRef.i, qab.i, epsilon);
	EXPECT_NEAR(qabRef.j, qab.j, epsilon);
	EXPECT_NEAR(qabRef.k, qab.k, epsilon);
	EXPECT_NEAR(qabRef.r, qab.r, epsilon);
}

TEST(Quaternion4fTest, ConjugateSIMD)
{
	float epsilon = QuaternionTypeSelector<float>::epsilon;

	auto x = float(M_PI*3/4);
	auto y = float(-M_PI/3);
	auto z = float(-M_PI/5);

	dsQuaternion4f q, invQ, ident;
	dsQuaternion4_fromEulerAngles(&q, x, y, z);
	dsQuaternion4f_conjugate(&invQ, &q);
	dsQuaternion4_mul(ident, q, invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}

TEST(Quaternion4dfTest, ConjugateSIMD)
{
	double epsilon = QuaternionTypeSelector<double>::epsilon;

	double x = M_PI*3/4;
	double y = -M_PI/3;
	double z = -M_PI/5;

	dsQuaternion4d q, invQ, ident;
	dsQuaternion4_fromEulerAngles(&q, x, y, z);
	dsQuaternion4d_conjugate(&invQ, &q);
	dsQuaternion4_mul(ident, q, invQ);

	EXPECT_NEAR(1, ident.r, epsilon);
	EXPECT_NEAR(0, ident.i, epsilon);
	EXPECT_NEAR(0, ident.j, epsilon);
	EXPECT_NEAR(0, ident.k, epsilon);
}
