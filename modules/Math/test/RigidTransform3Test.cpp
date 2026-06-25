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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/RigidTransform3.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct RigidTransform3TypeSelector;

template <>
struct RigidTransform3TypeSelector<float>
{
	typedef dsRigidTransform3f RigidTransform3Type;
	typedef dsMatrix44f Matrix44Type;
	typedef dsVector3xf Vector3xType;
	typedef dsQuaternion4f Quaternion4Type;
	static const float epsilon;
};

template <>
struct RigidTransform3TypeSelector<double>
{
	typedef dsRigidTransform3d RigidTransform3Type;
	typedef dsMatrix44d Matrix44Type;
	typedef dsVector3xd Vector3xType;
	typedef dsQuaternion4d Quaternion4Type;
	static const double epsilon;
};

const float RigidTransform3TypeSelector<float>::epsilon = 1e-5f;
const double RigidTransform3TypeSelector<double>::epsilon = 1e-13;

template <typename T>
class RigidTransform3Test : public testing::Test
{
};

using RigidTransform3Types = testing::Types<float, double>;
TYPED_TEST_SUITE(RigidTransform3Test, RigidTransform3Types);

inline void dsRigidTransform3_initialize(dsRigidTransform3f* result,
	const dsVector3xf* position, const dsQuaternion4f* orientation, const dsVector3xf* scale)
{
	dsRigidTransform3f_initialize(result, position, orientation, scale);
}

inline void dsRigidTransform3_initialize(dsRigidTransform3d* result,
	const dsVector3xd* position, const dsQuaternion4d* orientation, const dsVector3xd* scale)
{
	dsRigidTransform3d_initialize(result, position, orientation, scale);
}

inline void dsRigidTransform3_fromMatrix(dsRigidTransform3f* result, const dsMatrix44f* matrix)
{
	dsRigidTransform3f_fromMatrix(result, matrix);
}

inline void dsRigidTransform3_fromMatrix(dsRigidTransform3d* result, const dsMatrix44d* matrix)
{
	dsRigidTransform3d_fromMatrix(result, matrix);
}

inline void dsRigidTransform3_toMatrix(dsMatrix44f* result, const dsRigidTransform3f* transform)
{
	dsRigidTransform3f_toMatrix(result, transform);
}

inline void dsRigidTransform3_toMatrix(dsMatrix44d* result, const dsRigidTransform3d* transform)
{
	dsRigidTransform3d_toMatrix(result, transform);
}

void dsRigidTransform3_makeOrientationConsistent(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation)
{
	dsRigidTransform3f_makeOrientationConsistent(transform, otherOrientation);
}

void dsRigidTransform3_makeOrientationConsistent(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation)
{
	dsRigidTransform3d_makeOrientationConsistent(transform, otherOrientation);
}

inline bool dsRigidTransform3_isMulValid(const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	return dsRigidTransform3f_isMulValid(a, b);
}

inline bool dsRigidTransform3_isMulValid(const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	return dsRigidTransform3d_isMulValid(a, b);
}

inline void dsRigidTransform3_mul(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	dsRigidTransform3f_mul(result, a, b);
}

inline void dsRigidTransform3_mul(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	dsRigidTransform3d_mul(result, a, b);
}

inline void dsRigidTransform3_lerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	dsRigidTransform3f_lerp(result, a, b, t);
}

inline void dsRigidTransform3_lerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	dsRigidTransform3d_lerp(result, a, b, t);
}

inline void dsRigidTransform3_nearLerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	dsRigidTransform3f_nearLerp(result, a, b, t);
}

inline void dsRigidTransform3_nearLerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	dsRigidTransform3d_nearLerp(result, a, b, t);
}

inline bool dsRigidTransform3_equal(const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	return dsRigidTransform3f_equal(a, b);
}

inline bool dsRigidTransform3_equal(const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	return dsRigidTransform3d_equal(a, b);
}

inline void dsMatrix44_makeTranslate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeScale(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeScale(result, x, y, z);
}

inline void dsQuaternion4_normalize(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_normalize(result, a);
}

inline void dsQuaternion4_normalize(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_normalize(result, a);
}

inline void dsQuaternion4_fromEulerAngles(dsQuaternion4f* result, float x, float y, float z)
{
	dsQuaternion4f_fromEulerAngles(result, x, y, z);
}

inline void dsQuaternion4_fromEulerAngles(dsQuaternion4d* result, double x, double y, double z)
{
	dsQuaternion4d_fromEulerAngles(result, x, y, z);
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

inline void dsQuaternion4_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a)
{
	dsQuaternion4f_toMatrix44(result, a);
}

inline void dsQuaternion4_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a)
{
	dsQuaternion4d_toMatrix44(result, a);
}

inline float dsRadiansToDegrees(float radians)
{
	return dsRadiansToDegreesf(radians);
}

inline double dsRadiansToDegrees(double radians)
{
	return dsRadiansToDegreesd(radians);
}

inline bool dsEpsilonEqual(float x, float y, float epsilon)
{
	return dsEpsilonEqualf(x, y, epsilon);
}

inline bool dsEpsilonEqual(double x, double y, double epsilon)
{
	return dsEpsilonEquald(x, y, epsilon);
}

TYPED_TEST(RigidTransform3Test, Initialize)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type
		RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;

	RigidTransform3Type transform;
	dsRigidTransform3_initialize(&transform, nullptr, nullptr, nullptr);

	EXPECT_EQ(0, transform.position.x);
	EXPECT_EQ(0, transform.position.y);
	EXPECT_EQ(0, transform.position.z);

	EXPECT_EQ(0, transform.orientation.i);
	EXPECT_EQ(0, transform.orientation.j);
	EXPECT_EQ(0, transform.orientation.k);
	EXPECT_EQ(1, transform.orientation.r);

	EXPECT_EQ(1, transform.scale.x);
	EXPECT_EQ(1, transform.scale.y);
	EXPECT_EQ(1, transform.scale.z);

	Vector3xType position = {{1, 2, 3}};
	Quaternion4Type orientation = {{-1, -2, 3, 4}};
	dsQuaternion4_normalize(&orientation, &orientation);
	Vector3xType scale = {{6, 5, 4}};
	dsRigidTransform3_initialize(&transform, &position, &orientation, &scale);

	EXPECT_EQ(position.x, transform.position.x);
	EXPECT_EQ(position.y, transform.position.y);
	EXPECT_EQ(position.z, transform.position.z);

	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	EXPECT_EQ(scale.x, transform.scale.x);
	EXPECT_EQ(scale.y, transform.scale.y);
	EXPECT_EQ(scale.z, transform.scale.z);

	dsRigidTransform3_initialize(&transform, nullptr, &orientation, &scale);

	EXPECT_EQ(0, transform.position.x);
	EXPECT_EQ(0, transform.position.y);
	EXPECT_EQ(0, transform.position.z);

	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	EXPECT_EQ(scale.x, transform.scale.x);
	EXPECT_EQ(scale.y, transform.scale.y);
	EXPECT_EQ(scale.z, transform.scale.z);

	dsRigidTransform3_initialize(&transform, &position, nullptr, &scale);

	EXPECT_EQ(position.x, transform.position.x);
	EXPECT_EQ(position.y, transform.position.y);
	EXPECT_EQ(position.z, transform.position.z);

	EXPECT_EQ(0, transform.orientation.i);
	EXPECT_EQ(0, transform.orientation.j);
	EXPECT_EQ(0, transform.orientation.k);
	EXPECT_EQ(1, transform.orientation.r);

	EXPECT_EQ(scale.x, transform.scale.x);
	EXPECT_EQ(scale.y, transform.scale.y);
	EXPECT_EQ(scale.z, transform.scale.z);

	dsRigidTransform3_initialize(&transform, &position, &orientation, nullptr);

	EXPECT_EQ(position.x, transform.position.x);
	EXPECT_EQ(position.y, transform.position.y);
	EXPECT_EQ(position.z, transform.position.z);

	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	EXPECT_EQ(1, transform.scale.x);
	EXPECT_EQ(1, transform.scale.y);
	EXPECT_EQ(1, transform.scale.z);
}

TYPED_TEST(RigidTransform3Test, ToFromMatrix)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type
		RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;
	TypeParam epsilon = RigidTransform3TypeSelector<TypeParam>::epsilon;

	Vector3xType position = {{(TypeParam)-10, (TypeParam)20, (TypeParam)-30, (TypeParam)1}};
	Quaternion4Type orientation;
	dsQuaternion4_fromEulerAngles(&orientation, dsRadiansToDegrees((TypeParam)-10),
		dsRadiansToDegrees((TypeParam)15), dsRadiansToDegrees((TypeParam)-20));
	Vector3xType scale = {{(TypeParam)0.1, (TypeParam)0.2, (TypeParam)0.3, (TypeParam)-2}};

	RigidTransform3Type transform;
	dsRigidTransform3_initialize(&transform, &position, &orientation, &scale);

	Matrix44Type scaleMat, rotateMat, translateMat, tempMat;
	Matrix44Type expectedMatrix;
	dsMatrix44_makeScale(&scaleMat, scale.x, scale.y, scale.z);
	dsQuaternion4_toMatrix44(&rotateMat, &orientation);
	dsMatrix44_makeTranslate(&translateMat, position.x, position.y, position.z);
	dsMatrix44_affineMul(tempMat, rotateMat, scaleMat);
	dsMatrix44_affineMul(expectedMatrix, translateMat, tempMat);

	Matrix44Type matrix;
	dsRigidTransform3_toMatrix(&matrix, &transform);

	EXPECT_NEAR(expectedMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][3], matrix.values[3][3], epsilon);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3_fromMatrix(&transform, &expectedMatrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);
}

TYPED_TEST(RigidTransform3Test, MakeOrientationConsistent)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type
		RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;

	Quaternion4Type referenceOrientation, orientation;
	dsQuaternion4_fromEulerAngles(
		&referenceOrientation, (TypeParam)0.1, (TypeParam)0, (TypeParam)0);
	dsQuaternion4_fromEulerAngles(&orientation, (TypeParam)6.3, (TypeParam)0, (TypeParam)0);

	RigidTransform3Type transform;
	dsRigidTransform3_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3_makeOrientationConsistent(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3_makeOrientationConsistent(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}

TYPED_TEST(RigidTransform3Test, Multiply)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type
		RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;
	TypeParam epsilon = RigidTransform3TypeSelector<TypeParam>::epsilon;

	Vector3xType firstPosition = {{(TypeParam)-10, (TypeParam)20, (TypeParam)-30, (TypeParam)1}};
	Quaternion4Type firstOrientation;
	dsQuaternion4_fromEulerAngles(&firstOrientation, dsRadiansToDegrees((TypeParam)-10),
		dsRadiansToDegrees((TypeParam)15), dsRadiansToDegrees((TypeParam)-20));
	Vector3xType firstScaleUniform =
		{{(TypeParam)0.3, (TypeParam)0.3, (TypeParam)0.3, (TypeParam)-2}};
	Vector3xType firstScale = {{(TypeParam)0.1, (TypeParam)0.2, (TypeParam)0.3, (TypeParam)-2}};

	Vector3xType secondPosition = {{(TypeParam)31, (TypeParam)-24, (TypeParam)15, (TypeParam)-3}};
	Quaternion4Type secondOrientation;
	dsQuaternion4_fromEulerAngles(&secondOrientation, dsRadiansToDegrees((TypeParam)24),
		dsRadiansToDegrees((TypeParam)-62), dsRadiansToDegrees((TypeParam)35));
	Vector3xType secondScale = {{(TypeParam)1.2, (TypeParam)1.3, (TypeParam)1.4, (TypeParam)4}};

	// First check: uniform scale for first transform, rotation for second transform.
	RigidTransform3Type firstTransform;
	dsRigidTransform3_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	Matrix44Type scaleMat, rotateMat, translateMat, tempMat;
	Matrix44Type firstMatrix;
	dsMatrix44_makeScale(&scaleMat, firstScaleUniform.x, firstScaleUniform.y, firstScaleUniform.z);
	dsQuaternion4_toMatrix44(&rotateMat, &firstOrientation);
	dsMatrix44_makeTranslate(&translateMat, firstPosition.x, firstPosition.y, firstPosition.z);
	dsMatrix44_affineMul(tempMat, rotateMat, scaleMat);
	dsMatrix44_affineMul(firstMatrix, translateMat, tempMat);

	RigidTransform3Type secondTransform;
	dsRigidTransform3_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	Matrix44Type secondMatrix;
	dsMatrix44_makeScale(&scaleMat, secondScale.x, secondScale.y, secondScale.z);
	dsQuaternion4_toMatrix44(&rotateMat, &secondOrientation);
	dsMatrix44_makeTranslate(&translateMat, secondPosition.x, secondPosition.y, secondPosition.z);
	dsMatrix44_affineMul(tempMat, rotateMat, scaleMat);
	dsMatrix44_affineMul(secondMatrix, translateMat, tempMat);

	Matrix44Type expectedMatrix, matrix;
	dsMatrix44_affineMul(expectedMatrix, firstMatrix, secondMatrix);

	RigidTransform3Type transform;
	EXPECT_TRUE(dsRigidTransform3_isMulValid(&firstTransform, &secondTransform));
	EXPECT_FALSE(dsRigidTransform3_isMulValid(&secondTransform, &firstTransform));
	dsRigidTransform3_mul(&transform, &firstTransform, &secondTransform);
	dsRigidTransform3_toMatrix(&matrix, &transform);

	EXPECT_NEAR(expectedMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][3], matrix.values[3][3], epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);

	dsMatrix44_makeScale(&scaleMat, firstScale.x, firstScale.y, firstScale.z);
	dsQuaternion4_toMatrix44(&rotateMat, &firstOrientation);
	dsMatrix44_makeTranslate(&translateMat, firstPosition.x, firstPosition.y, firstPosition.z);
	dsMatrix44_affineMul(tempMat, rotateMat, scaleMat);
	dsMatrix44_affineMul(firstMatrix, translateMat, tempMat);

	dsRigidTransform3_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsMatrix44_makeScale(&scaleMat, secondScale.x, secondScale.y, secondScale.z);
	dsMatrix44_makeTranslate(&translateMat, secondPosition.x, secondPosition.y, secondPosition.z);
	dsMatrix44_affineMul(secondMatrix, translateMat, scaleMat);

	dsMatrix44_affineMul(expectedMatrix, firstMatrix, secondMatrix);

	EXPECT_TRUE(dsRigidTransform3_isMulValid(&firstTransform, &secondTransform));
	EXPECT_FALSE(dsRigidTransform3_isMulValid(&secondTransform, &firstTransform));
	dsRigidTransform3_mul(&transform, &firstTransform, &secondTransform);
	dsRigidTransform3_toMatrix(&matrix, &transform);

	EXPECT_NEAR(expectedMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_NEAR(expectedMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_NEAR(expectedMatrix.values[3][3], matrix.values[3][3], epsilon);
}

TYPED_TEST(RigidTransform3Test, Lerp)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;
	TypeParam epsilon = RigidTransform3TypeSelector<TypeParam>::epsilon;

	Vector3xType positionA = {{(TypeParam)-10, (TypeParam)20, (TypeParam)-30}};
	Quaternion4Type orientationA;
	dsQuaternion4_fromEulerAngles(&orientationA, dsRadiansToDegrees((TypeParam)-10),
		dsRadiansToDegrees((TypeParam)15), dsRadiansToDegrees((TypeParam)-20));
	Vector3xType scaleA = {{(TypeParam)0.1, (TypeParam)0.2, (TypeParam)0.3}};
	RigidTransform3Type transformA;
	dsRigidTransform3_initialize(&transformA, &positionA, &orientationA, &scaleA);

	Vector3xType positionB = {{(TypeParam)20, (TypeParam)-15, (TypeParam)10}};
	Quaternion4Type orientationB;
	dsQuaternion4_fromEulerAngles(&orientationB, dsRadiansToDegrees((TypeParam)30),
		dsRadiansToDegrees((TypeParam)-20), dsRadiansToDegrees((TypeParam)10));
	Vector3xType scaleB = {{(TypeParam)3.0, (TypeParam)2.0, (TypeParam)1.0}};
	RigidTransform3Type transformB;
	dsRigidTransform3_initialize(&transformB, &positionB, &orientationB, &scaleB);

	TypeParam t = (TypeParam)0.375;
	Vector3xType positionInterp;
	Quaternion4Type orientationInterp;
	Vector3xType scaleInterp;
	dsVector3_lerp(positionInterp, positionA, positionB, t);
	dsQuaternion4_slerp(&orientationInterp, &orientationA, &orientationB, t);
	dsVector3_lerp(scaleInterp, scaleA, scaleB, t);

	RigidTransform3Type transformInterp;
	dsRigidTransform3_lerp(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(positionInterp.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(positionInterp.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(positionInterp.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(orientationInterp.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(orientationInterp.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(orientationInterp.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(orientationInterp.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scaleInterp.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scaleInterp.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scaleInterp.z, transformInterp.scale.z, epsilon);

	t = (TypeParam)1.0;
	dsRigidTransform3_lerp(&transformInterp, &transformA, &transformB, t);
	dsRigidTransform3_makeOrientationConsistent(&transformInterp, &orientationB);

	EXPECT_NEAR(positionB.x, transformInterp.position.x, epsilon);
	EXPECT_NEAR(positionB.y, transformInterp.position.y, epsilon);
	EXPECT_NEAR(positionB.z, transformInterp.position.z, epsilon);

	EXPECT_NEAR(orientationB.i, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(orientationB.j, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(orientationB.k, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(orientationB.r, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(scaleB.x, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(scaleB.y, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(scaleB.z, transformInterp.scale.z, epsilon);
}

TYPED_TEST(RigidTransform3Test, NearLerp)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;
	TypeParam epsilon = RigidTransform3TypeSelector<TypeParam>::epsilon;
	constexpr auto slerpEpsilon = TypeParam(1e-3);

	Vector3xType positionA = {{(TypeParam)-10, (TypeParam)20, (TypeParam)-30}};
	Quaternion4Type orientationA;
	dsQuaternion4_fromEulerAngles(&orientationA, dsRadiansToDegrees((TypeParam)-10),
		dsRadiansToDegrees((TypeParam)15), dsRadiansToDegrees((TypeParam)-20));
	Vector3xType scaleA = {{(TypeParam)0.1, (TypeParam)0.2, (TypeParam)0.3}};
	RigidTransform3Type transformA;
	dsRigidTransform3_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way.
	Vector3xType positionB = {{(TypeParam)20, (TypeParam)-15, (TypeParam)10}};
	Quaternion4Type orientationB;
	dsQuaternion4_fromEulerAngles(&orientationB, dsRadiansToDegrees((TypeParam)130),
		dsRadiansToDegrees((TypeParam)-120), dsRadiansToDegrees((TypeParam)110));
	Vector3xType scaleB = {{(TypeParam)3.0, (TypeParam)2.0, (TypeParam)1.0}};
	RigidTransform3Type transformB;
	dsRigidTransform3_initialize(&transformB, &positionB, &orientationB, &scaleB);

	TypeParam t = (TypeParam)0.375;
	Vector3xType positionInterp;
	Quaternion4Type orientationInterp;
	Vector3xType scaleInterp;
	dsVector3_lerp(positionInterp, positionA, positionB, t);
	dsQuaternion4_slerp(&orientationInterp, &orientationA, &orientationB, t);
	dsVector3_lerp(scaleInterp, scaleA, scaleB, t);

	// Naive approach without keeping orientation consistent should have unexpected rotation.
	RigidTransform3Type transformInterp;
	dsRigidTransform3_nearLerp(&transformInterp, &transformA, &transformB, t);
	EXPECT_FALSE(dsEpsilonEqual(orientationInterp.i, transformInterp.orientation.i, slerpEpsilon));
	EXPECT_FALSE(dsEpsilonEqual(orientationInterp.j, transformInterp.orientation.j, slerpEpsilon));
	EXPECT_FALSE(dsEpsilonEqual(orientationInterp.k, transformInterp.orientation.k, slerpEpsilon));
	EXPECT_FALSE(dsEpsilonEqual(orientationInterp.r, transformInterp.orientation.r, slerpEpsilon));

	dsRigidTransform3_makeOrientationConsistent(&transformB, &orientationA);
	EXPECT_EQ(-orientationB.i, transformB.orientation.i);
	dsRigidTransform3_nearLerp(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(positionInterp.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(positionInterp.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(positionInterp.z, transformInterp.position.z, epsilon);

	EXPECT_NEAR(orientationInterp.i, transformInterp.orientation.i, slerpEpsilon);
	EXPECT_NEAR(orientationInterp.j, transformInterp.orientation.j, slerpEpsilon);
	EXPECT_NEAR(orientationInterp.k, transformInterp.orientation.k, slerpEpsilon);
	EXPECT_NEAR(orientationInterp.r, transformInterp.orientation.r, slerpEpsilon);

	EXPECT_EQ_DETERMINISTIC(scaleInterp.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scaleInterp.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scaleInterp.z, transformInterp.scale.z, epsilon);

	t = (TypeParam)1.0;
	dsRigidTransform3_lerp(&transformInterp, &transformA, &transformB, t);
	dsRigidTransform3_makeOrientationConsistent(&transformInterp, &orientationB);

	EXPECT_NEAR(positionB.x, transformInterp.position.x, epsilon);
	EXPECT_NEAR(positionB.y, transformInterp.position.y, epsilon);
	EXPECT_NEAR(positionB.z, transformInterp.position.z, epsilon);

	EXPECT_NEAR(orientationB.i, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(orientationB.j, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(orientationB.k, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(orientationB.r, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(scaleB.x, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(scaleB.y, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(scaleB.z, transformInterp.scale.z, epsilon);
}

TYPED_TEST(RigidTransform3Test, Equal)
{
	typedef typename RigidTransform3TypeSelector<TypeParam>::RigidTransform3Type
		RigidTransform3Type;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Vector3xType Vector3xType;
	typedef typename RigidTransform3TypeSelector<TypeParam>::Quaternion4Type Quaternion4Type;

	Vector3xType position = {{1, 2, 3}};
	Quaternion4Type orientation = {{-1, -2, 3, 4}};
	dsQuaternion4_normalize(&orientation, &orientation);
	Vector3xType scale = {{6, 5, 4}};
	RigidTransform3Type transform;
	dsRigidTransform3_initialize(&transform, &position, &orientation, &scale);

	RigidTransform3Type otherTransform = transform;
	otherTransform.position.w = 5;
	otherTransform.scale.w = -3;
	EXPECT_TRUE(dsRigidTransform3_equal(&transform, &otherTransform));

	dsVector4_neg(otherTransform.orientation, otherTransform.orientation);
	EXPECT_TRUE(dsRigidTransform3_equal(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.position.x = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.y = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.z = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.orientation.i = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.j = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.k = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.r = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.scale.x = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.y = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.z = 20;
	EXPECT_FALSE(dsRigidTransform3_equal(&transform, &otherTransform));
}

#if DS_HAS_SIMD

TEST(RigidTransform3fTest, ToFromMatrixSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;

	dsVector3xf position = {{-10.0f, 20.0f, -30.0f, 1.0f}};
	dsQuaternion4f orientation;
	dsQuaternion4f_fromEulerAngles(&orientation, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scale = {{0.1f, 0.2f, 0.3f, -2.0f}};

	dsRigidTransform3f transform;
	dsRigidTransform3f_initialize(&transform, &position, &orientation, &scale);

	dsMatrix44f scalarMatrix, matrix;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3f_toMatrix(&scalarMatrix, &transform);
	dsRigidTransform3f_toMatrixSIMD(&matrix, &transform);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][3], matrix.values[3][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.0151008489f, matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.0145200016f, matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0977810472f, matrix.values[0][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.0789697766f, matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.179592133f, matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.0388642699f, matrix.values[1][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.271875292f, matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.124629475f, matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.0234803092f, matrix.values[2][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[2][3]);

	EXPECT_EQ_DETERMINISTIC(-10.0f, matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0f, matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0f, matrix.values[3][2], epsilon);
	EXPECT_EQ(1.0f, matrix.values[3][3]);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3f_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3f_fromMatrixSIMD(&transform, &matrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-10.0f, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0f, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0f, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.6458866f, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.209033087f, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.729251087f, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.085584f, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.1f, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.199999973f, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.3f, transform.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3fTest, ToFromMatrixFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;

	dsVector3xf position = {{-10.0f, 20.0f, -30.0f, 1.0f}};
	dsQuaternion4f orientation;
	dsQuaternion4f_fromEulerAngles(&orientation, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scale = {{0.1f, 0.2f, 0.3f, -2.0f}};

	dsRigidTransform3f transform;
	dsRigidTransform3f_initialize(&transform, &position, &orientation, &scale);

	dsMatrix44f matrix;
	dsRigidTransform3f_toMatrixFMA(&matrix, &transform);

	EXPECT_NEAR(-0.0151008489f, matrix.values[0][0], epsilon);
	EXPECT_NEAR(-0.0145200016f, matrix.values[0][1], epsilon);
	EXPECT_NEAR(0.0977810472f, matrix.values[0][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[0][3]);

	EXPECT_NEAR(-0.0789697766f, matrix.values[1][0], epsilon);
	EXPECT_NEAR(-0.179592133f, matrix.values[1][1], epsilon);
	EXPECT_NEAR(-0.0388642699f, matrix.values[1][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[1][3]);

	EXPECT_NEAR(0.271875292f, matrix.values[2][0], epsilon);
	EXPECT_NEAR(-0.124629475f, matrix.values[2][1], epsilon);
	EXPECT_NEAR(0.0234803092f, matrix.values[2][2], epsilon);
	EXPECT_EQ(0.0f, matrix.values[2][3]);

	EXPECT_NEAR(-10.0f, matrix.values[3][0], epsilon);
	EXPECT_NEAR(20.0f, matrix.values[3][1], epsilon);
	EXPECT_NEAR(-30.0f, matrix.values[3][2], epsilon);
	EXPECT_EQ(1.0f, matrix.values[3][3]);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3f_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3f_fromMatrixSIMD(&transform, &matrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);
}
#endif // DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, ToFromMatrixSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd position = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d orientation;
	dsQuaternion4d_fromEulerAngles(&orientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesf(15.0), dsRadiansToDegreesf(-20.0));
	dsVector3xd scale = {{0.1, 0.2, 0.3, -2.0}};

	dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, &position, &orientation, &scale);

	dsMatrix44d scalarMatrix, matrix;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_toMatrix(&scalarMatrix, &transform);
	dsRigidTransform3d_toMatrixSIMD2(&matrix, &transform);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][3], matrix.values[3][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.015100853575494422, matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.014520004549609262, matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.097781049744675996, matrix.values[0][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.078975502407083856, matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.17958953043887621, matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.038864772947910312, matrix.values[1][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.27187154234234928, matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.12463798162513615, matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.023478458228861087, matrix.values[2][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[2][3]);

	EXPECT_EQ_DETERMINISTIC(-10.0, matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0, matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0, matrix.values[3][2], epsilon);
	EXPECT_EQ(1.0, matrix.values[3][3]);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3d_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3d_fromMatrixSIMD2(&transform, &matrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-10.0, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.64588652041397232, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20904506459793729, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.72924922272478832, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.085594011881982027, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.099999999999999992, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.19999999999999996, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.29999999999999993, transform.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3dTest, ToFromMatrixFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd position = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d orientation;
	dsQuaternion4d_fromEulerAngles(&orientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesf(15.0), dsRadiansToDegreesf(-20.0));
	dsVector3xd scale = {{0.1, 0.2, 0.3, -2.0}};

	dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, &position, &orientation, &scale);

	dsMatrix44d matrix;
	dsRigidTransform3d_toMatrixFMA2(&matrix, &transform);

	EXPECT_NEAR(-0.015100853575494422, matrix.values[0][0], epsilon);
	EXPECT_NEAR(-0.014520004549609262, matrix.values[0][1], epsilon);
	EXPECT_NEAR(0.097781049744675996, matrix.values[0][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[0][3]);

	EXPECT_NEAR(-0.078975502407083856, matrix.values[1][0], epsilon);
	EXPECT_NEAR(-0.17958953043887621, matrix.values[1][1], epsilon);
	EXPECT_NEAR(-0.038864772947910312, matrix.values[1][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[1][3]);

	EXPECT_NEAR(0.27187154234234928, matrix.values[2][0], epsilon);
	EXPECT_NEAR(-0.12463798162513615, matrix.values[2][1], epsilon);
	EXPECT_NEAR(0.023478458228861087, matrix.values[2][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[2][3]);

	EXPECT_NEAR(-10.0, matrix.values[3][0], epsilon);
	EXPECT_NEAR(20.0, matrix.values[3][1], epsilon);
	EXPECT_NEAR(-30.0, matrix.values[3][2], epsilon);
	EXPECT_EQ(1.0, matrix.values[3][3]);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3d_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3d_fromMatrixSIMD2(&transform, &matrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, ToFromMatrixSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd position = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d orientation;
	dsQuaternion4d_fromEulerAngles(&orientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesf(15.0), dsRadiansToDegreesf(-20.0));
	dsVector3xd scale = {{0.1, 0.2, 0.3, -2.0}};

	DS_ALIGN(32) dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, &position, &orientation, &scale);

	DS_ALIGN(32) dsMatrix44d scalarMatrix, matrix;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_toMatrix(&scalarMatrix, &transform);
	dsRigidTransform3d_toMatrixSIMD4(&matrix, &transform);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarMatrix.values[3][3], matrix.values[3][3], epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.015100853575494422, matrix.values[0][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.014520004549609262, matrix.values[0][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.097781049744675996, matrix.values[0][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[0][3]);

	EXPECT_EQ_DETERMINISTIC(-0.078975502407083856, matrix.values[1][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.17958953043887621, matrix.values[1][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.038864772947910312, matrix.values[1][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[1][3]);

	EXPECT_EQ_DETERMINISTIC(0.27187154234234928, matrix.values[2][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.12463798162513615, matrix.values[2][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(0.023478458228861087, matrix.values[2][2], epsilon);
	EXPECT_EQ(0.0, matrix.values[2][3]);

	EXPECT_EQ_DETERMINISTIC(-10.0, matrix.values[3][0], epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0, matrix.values[3][1], epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0, matrix.values[3][2], epsilon);
	EXPECT_EQ(1.0, matrix.values[3][3]);

	// Clear out the transform to ensure it doesn't keep the original values around.
	dsRigidTransform3d_initialize(&transform, nullptr, nullptr, nullptr);
	dsRigidTransform3d_fromMatrixSIMD4(&transform, &matrix);

	EXPECT_NEAR(position.x, transform.position.x, epsilon);
	EXPECT_NEAR(position.y, transform.position.y, epsilon);
	EXPECT_NEAR(position.z, transform.position.z, epsilon);

	// May be opposite sign.
	if ((transform.orientation.i < 0) != (orientation.i < 0))
		dsVector4_neg(transform.orientation, transform.orientation);
	EXPECT_NEAR(orientation.i, transform.orientation.i, epsilon);
	EXPECT_NEAR(orientation.j, transform.orientation.j, epsilon);
	EXPECT_NEAR(orientation.k, transform.orientation.k, epsilon);
	EXPECT_NEAR(orientation.r, transform.orientation.r, epsilon);

	EXPECT_NEAR(scale.x, transform.scale.x, epsilon);
	EXPECT_NEAR(scale.y, transform.scale.y, epsilon);
	EXPECT_NEAR(scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-10.0, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(20.0, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-30.0, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.64588652041397232, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20904506459793729, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.72924922272478832, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.085594011881982027, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.099999999999999992, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.19999999999999996, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.29999999999999993, transform.scale.z, epsilon);
}

TEST(RigidTransform3fTest, MakeOrientationConsistentSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsQuaternion4f referenceOrientation, orientation;
	dsQuaternion4f_fromEulerAngles(&referenceOrientation, 0.1f, 0.0f, 0.0f);
	dsQuaternion4f_fromEulerAngles(&orientation, 6.3f, 0.0f, 0.0f);

	dsRigidTransform3f transform;
	dsRigidTransform3f_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3f_makeOrientationConsistentSIMD(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3f_makeOrientationConsistentSIMD(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3fTest, MakeOrientationConsistentFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	dsQuaternion4f referenceOrientation, orientation;
	dsQuaternion4f_fromEulerAngles(&referenceOrientation, 0.1f, 0.0f, 0.0f);
	dsQuaternion4f_fromEulerAngles(&orientation, 6.3f, 0.0f, 0.0f);

	dsRigidTransform3f transform;
	dsRigidTransform3f_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3f_makeOrientationConsistentFMA(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3f_makeOrientationConsistentFMA(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, MakeOrientationConsistentSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	dsQuaternion4d referenceOrientation, orientation;
	dsQuaternion4d_fromEulerAngles(&referenceOrientation, 0.1, 0.0, 0.0);
	dsQuaternion4d_fromEulerAngles(&orientation, 6.3, 0.0, 0.0);

	dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3d_makeOrientationConsistentSIMD2(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3d_makeOrientationConsistentSIMD2(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3dTest, MakeOrientationConsistentFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	dsQuaternion4d referenceOrientation, orientation;
	dsQuaternion4d_fromEulerAngles(&referenceOrientation, 0.1, 0.0, 0.0);
	dsQuaternion4d_fromEulerAngles(&orientation, 6.3, 0.0, 0.0);

	dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3d_makeOrientationConsistentFMA2(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3d_makeOrientationConsistentFMA2(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, MakeOrientationConsistentSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	DS_ALIGN(32) dsQuaternion4d referenceOrientation, orientation;
	dsQuaternion4d_fromEulerAngles(&referenceOrientation, 0.1, 0.0, 0.0);
	dsQuaternion4d_fromEulerAngles(&orientation, 6.3, 0.0, 0.0);

	DS_ALIGN(32) dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, nullptr, &orientation, nullptr);

	dsRigidTransform3d_makeOrientationConsistentSIMD4(&transform, &orientation);
	EXPECT_EQ(orientation.i, transform.orientation.i);
	EXPECT_EQ(orientation.j, transform.orientation.j);
	EXPECT_EQ(orientation.k, transform.orientation.k);
	EXPECT_EQ(orientation.r, transform.orientation.r);

	dsRigidTransform3d_makeOrientationConsistentSIMD4(&transform, &referenceOrientation);
	EXPECT_EQ(-orientation.i, transform.orientation.i);
	EXPECT_EQ(-orientation.j, transform.orientation.j);
	EXPECT_EQ(-orientation.k, transform.orientation.k);
	EXPECT_EQ(-orientation.r, transform.orientation.r);
}

TEST(RigidTransform3fTest, MultiplySIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xf firstPosition = {{-10.0f, 20.0f, -30.0f, 1.0f}};
	dsQuaternion4f firstOrientation;
	dsQuaternion4f_fromEulerAngles(&firstOrientation, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf firstScaleUniform = {{0.3f, 0.3f, 0.3f, -2.0f}};
	dsVector3xf firstScale = {{0.1f, 0.2f, 0.3f, -2.0f}};

	dsVector3xf secondPosition = {{31.0f, -24.0f, 15.0f, -3.0f}};
	dsQuaternion4f secondOrientation;
	dsQuaternion4f_fromEulerAngles(&secondOrientation, dsRadiansToDegreesf(24.0f),
		dsRadiansToDegreesf(-62.0f), dsRadiansToDegreesf(35.0f));
	dsVector3xf secondScale = {{1.2f, 1.3f, 1.4f, 4.0f}};

	// First check: uniform scale for first transform, rotation for second transform.
	dsRigidTransform3f firstTransform;
	dsRigidTransform3f_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	dsRigidTransform3f secondTransform;
	dsRigidTransform3f_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	dsRigidTransform3f scalarTransform, transform;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3f_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3f_mulSIMD(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.48333836f, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(23.2455158f, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-19.1550446f, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.973394632f, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.128100008f, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.115697697f, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.150689334f, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.36f, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.390000015f, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.420000017f, transform.scale.z, epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3f_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);
	dsRigidTransform3f_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsRigidTransform3f_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3f_mulSIMD(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.49472237f, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(21.9906502f, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-25.6838417f, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.645887852f, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.209033534f, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.729252517f, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.0855838209f, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.120000005f, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.26f, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.420000017f, transform.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3fTest, MultiplyFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;

	dsVector3xf firstPosition = {{-10.0f, 20.0f, -30.0f, 1.0f}};
	dsQuaternion4f firstOrientation;
	dsQuaternion4f_fromEulerAngles(&firstOrientation, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf firstScaleUniform = {{0.3f, 0.3f, 0.3f, -2.0f}};
	dsVector3xf firstScale = {{0.1f, 0.2f, 0.3f, -2.0f}};

	dsVector3xf secondPosition = {{31.0f, -24.0f, 15.0f, -3.0f}};
	dsQuaternion4f secondOrientation;
	dsQuaternion4f_fromEulerAngles(&secondOrientation, dsRadiansToDegreesf(24.0f),
		dsRadiansToDegreesf(-62.0f), dsRadiansToDegreesf(35.0f));
	dsVector3xf secondScale = {{1.2f, 1.3f, 1.4f, 4.0f}};

	// First check: uniform scale for first transform, rotation for second transform.
	dsRigidTransform3f firstTransform;
	dsRigidTransform3f_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	dsRigidTransform3f secondTransform;
	dsRigidTransform3f_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	dsRigidTransform3f transform;
	dsRigidTransform3f_mulFMA(&transform, &firstTransform, &secondTransform);

	EXPECT_NEAR(-4.48333836f, transform.position.x, epsilon);
	EXPECT_NEAR(23.2455158f, transform.position.y, epsilon);
	EXPECT_NEAR(-19.1550446f, transform.position.z, epsilon);

	EXPECT_NEAR(0.973394632f, transform.orientation.i, epsilon);
	EXPECT_NEAR(0.128100008f, transform.orientation.j, epsilon);
	EXPECT_NEAR(-0.115697697f, transform.orientation.k, epsilon);
	EXPECT_NEAR(-0.150689334f, transform.orientation.r, epsilon);

	EXPECT_NEAR(0.36f, transform.scale.x, epsilon);
	EXPECT_NEAR(0.390000015f, transform.scale.y, epsilon);
	EXPECT_NEAR(0.420000017f, transform.scale.z, epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3f_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);
	dsRigidTransform3f_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsRigidTransform3f_mulFMA(&transform, &firstTransform, &secondTransform);

	EXPECT_NEAR(-4.49472237f, transform.position.x, epsilon);
	EXPECT_NEAR(21.9906502f, transform.position.y, epsilon);
	EXPECT_NEAR(-25.6838417f, transform.position.z, epsilon);

	EXPECT_NEAR(-0.645887852f, transform.orientation.i, epsilon);
	EXPECT_NEAR(0.209033534f, transform.orientation.j, epsilon);
	EXPECT_NEAR(-0.729252517f, transform.orientation.k, epsilon);
	EXPECT_NEAR(-0.0855838209f, transform.orientation.r, epsilon);

	EXPECT_NEAR(0.120000005f, transform.scale.x, epsilon);
	EXPECT_NEAR(0.26f, transform.scale.y, epsilon);
	EXPECT_NEAR(0.420000017f, transform.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, MultiplySIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd firstPosition = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d firstOrientation;
	dsQuaternion4d_fromEulerAngles(&firstOrientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd firstScaleUniform = {{0.3, 0.3, 0.3, -2.0}};
	dsVector3xd firstScale = {{0.1, 0.2, 0.3, -2.0}};

	dsVector3xd secondPosition = {{31.0, -24.0, 15.0, -3.0}};
	dsQuaternion4d secondOrientation;
	dsQuaternion4d_fromEulerAngles(&secondOrientation, dsRadiansToDegreesd(24.0),
		dsRadiansToDegreesd(-62.0), dsRadiansToDegreesd(35.0));
	dsVector3xd secondScale = {{1.2, 1.3, 1.4, 4.0}};

	// First check: uniform scale for first transform, rotation for second transform.
	dsRigidTransform3d firstTransform;
	dsRigidTransform3d_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	dsRigidTransform3d secondTransform;
	dsRigidTransform3d_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	dsRigidTransform3d scalarTransform, transform;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3d_mulSIMD2(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.4833529031101014, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(23.244591152212397, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-19.154759889086769, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.9733716341619123, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.12815217858273431, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.11569664883451933, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.15079445076636339, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.36, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.39, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.42, transform.scale.z, epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3d_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);
	dsRigidTransform3d_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsRigidTransform3d_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3d_mulSIMD2(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.4946628702787637, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(21.989971314080751, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-25.683603672361809, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.64586894018621044, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20907876439232123, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.72925409624943316, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.085602835737748073, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.12, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.26, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.42, transform.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3dTest, MultiplyFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd firstPosition = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d firstOrientation;
	dsQuaternion4d_fromEulerAngles(&firstOrientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd firstScaleUniform = {{0.3, 0.3, 0.3, -2.0}};
	dsVector3xd firstScale = {{0.1, 0.2, 0.3, -2.0}};

	dsVector3xd secondPosition = {{31.0, -24.0, 15.0, -3.0}};
	dsQuaternion4d secondOrientation;
	dsQuaternion4d_fromEulerAngles(&secondOrientation, dsRadiansToDegreesd(24.0),
		dsRadiansToDegreesd(-62.0), dsRadiansToDegreesd(35.0));
	dsVector3xd secondScale = {{1.2, 1.3, 1.4, 4.0}};

	// First check: uniform scale for first transform, rotation for second transform.
	dsRigidTransform3d firstTransform;
	dsRigidTransform3d_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	dsRigidTransform3d secondTransform;
	dsRigidTransform3d_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	dsRigidTransform3d transform;
	dsRigidTransform3d_mulFMA2(&transform, &firstTransform, &secondTransform);

	EXPECT_NEAR(-4.4833529031101014, transform.position.x, epsilon);
	EXPECT_NEAR(23.244591152212397, transform.position.y, epsilon);
	EXPECT_NEAR(-19.154759889086769, transform.position.z, epsilon);

	EXPECT_NEAR(0.9733716341619123, transform.orientation.i, epsilon);
	EXPECT_NEAR(0.12815217858273431, transform.orientation.j, epsilon);
	EXPECT_NEAR(-0.11569664883451933, transform.orientation.k, epsilon);
	EXPECT_NEAR(-0.15079445076636339, transform.orientation.r, epsilon);

	EXPECT_NEAR(0.36, transform.scale.x, epsilon);
	EXPECT_NEAR(0.39, transform.scale.y, epsilon);
	EXPECT_NEAR(0.42, transform.scale.z, epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3d_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);
	dsRigidTransform3d_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsRigidTransform3d_mulFMA2(&transform, &firstTransform, &secondTransform);

	EXPECT_NEAR(-4.4946628702787637, transform.position.x, epsilon);
	EXPECT_NEAR(21.989971314080751, transform.position.y, epsilon);
	EXPECT_NEAR(-25.683603672361809, transform.position.z, epsilon);

	EXPECT_NEAR(-0.64586894018621044, transform.orientation.i, epsilon);
	EXPECT_NEAR(0.20907876439232123, transform.orientation.j, epsilon);
	EXPECT_NEAR(-0.72925409624943316, transform.orientation.k, epsilon);
	EXPECT_NEAR(-0.085602835737748073, transform.orientation.r, epsilon);

	EXPECT_NEAR(0.12, transform.scale.x, epsilon);
	EXPECT_NEAR(0.26, transform.scale.y, epsilon);
	EXPECT_NEAR(0.42, transform.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, MultiplySIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd firstPosition = {{-10.0, 20.0, -30.0, 1.0}};
	dsQuaternion4d firstOrientation;
	dsQuaternion4d_fromEulerAngles(&firstOrientation, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd firstScaleUniform = {{0.3, 0.3, 0.3, -2.0}};
	dsVector3xd firstScale = {{0.1, 0.2, 0.3, -2.0}};

	dsVector3xd secondPosition = {{31.0, -24.0, 15.0, -3.0}};
	dsQuaternion4d secondOrientation;
	dsQuaternion4d_fromEulerAngles(&secondOrientation, dsRadiansToDegreesd(24.0),
		dsRadiansToDegreesd(-62.0), dsRadiansToDegreesd(35.0));
	dsVector3xd secondScale = {{1.2, 1.3, 1.4, 4.0}};

	// First check: uniform scale for first transform, rotation for second transform.
	DS_ALIGN(32) dsRigidTransform3d firstTransform;
	dsRigidTransform3d_initialize(
		&firstTransform, &firstPosition, &firstOrientation, &firstScaleUniform);

	DS_ALIGN(32) dsRigidTransform3d secondTransform;
	dsRigidTransform3d_initialize(
		&secondTransform, &secondPosition, &secondOrientation, &secondScale);

	DS_ALIGN(32) dsRigidTransform3d scalarTransform, transform;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3d_mulSIMD4(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.4833529031101014, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(23.244591152212397, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-19.154759889086769, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.9733716341619123, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.12815217858273431, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.11569664883451933, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.15079445076636339, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.36, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.39, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.42, transform.scale.z, epsilon);

	// Second check: non-uniform scale for first transform, no rotation for second transform.
	dsRigidTransform3d_initialize(&firstTransform, &firstPosition, &firstOrientation, &firstScale);
	dsRigidTransform3d_initialize(&secondTransform, &secondPosition, nullptr, &secondScale);

	dsRigidTransform3d_mul(&scalarTransform, &firstTransform, &secondTransform);
	dsRigidTransform3d_mulSIMD4(&transform, &firstTransform, &secondTransform);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.x, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.y, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.position.z, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.i, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.j, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.k, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.orientation.r, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.x, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.y, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransform.scale.z, transform.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-4.4946628702787637, transform.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(21.989971314080751, transform.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-25.683603672361809, transform.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.64586894018621044, transform.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.20907876439232123, transform.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.72925409624943316, transform.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.085602835737748073, transform.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(0.12, transform.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.26, transform.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.42, transform.scale.z, epsilon);
}

TEST(RigidTransform3fTest, LerpSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xf positionA = {{-10.0f, 20.0f, -30.0f}};
	dsQuaternion4f orientationA;
	dsQuaternion4f_fromEulerAngles(&orientationA, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scaleA = {{0.1f, 0.2f, 0.3f}};
	dsRigidTransform3f transformA;
	dsRigidTransform3f_initialize(&transformA, &positionA, &orientationA, &scaleA);

	dsVector3xf positionB = {{20.0f, -15.0f, 10.0f}};
	dsQuaternion4f orientationB;
	dsQuaternion4f_fromEulerAngles(&orientationB, dsRadiansToDegreesf(30.0f),
		dsRadiansToDegreesf(-20.0f), dsRadiansToDegreesf(10.0f));
	dsVector3xf scaleB = {{3.0f, 2.0f, 1.0f}};
	dsRigidTransform3f transformB;
	dsRigidTransform3f_initialize(&transformB, &positionB, &orientationB, &scaleB);

	float t = 0.375f;
	dsRigidTransform3f scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3f_lerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3f_lerpSIMD(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25f, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875f, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0f, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.537236691f, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.0142315179f, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.821273148f, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.191532031f, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875f, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875f, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625f, transformInterp.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3fTest, LerpFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;

	dsVector3xf positionA = {{-10.0f, 20.0f, -30.0f}};
	dsQuaternion4f orientationA;
	dsQuaternion4f_fromEulerAngles(&orientationA, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scaleA = {{0.1f, 0.2f, 0.3f}};
	dsRigidTransform3f transformA;
	dsRigidTransform3f_initialize(&transformA, &positionA, &orientationA, &scaleA);

	dsVector3xf positionB = {{20.0f, -15.0f, 10.0f}};
	dsQuaternion4f orientationB;
	dsQuaternion4f_fromEulerAngles(&orientationB, dsRadiansToDegreesf(30.0f),
		dsRadiansToDegreesf(-20.0f), dsRadiansToDegreesf(10.0f));
	dsVector3xf scaleB = {{3.0f, 2.0f, 1.0f}};
	dsRigidTransform3f transformB;
	dsRigidTransform3f_initialize(&transformB, &positionB, &orientationB, &scaleB);

	float t = 0.375f;
	dsRigidTransform3f transformInterp;
	dsRigidTransform3f_lerpFMA(&transformInterp, &transformA, &transformB, t);

	EXPECT_NEAR(1.25f, transformInterp.position.x, epsilon);
	EXPECT_NEAR(6.875f, transformInterp.position.y, epsilon);
	EXPECT_NEAR(-15.0f, transformInterp.position.z, epsilon);

	EXPECT_NEAR(-0.537236691f, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(-0.0142315179f, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(-0.821273148f, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(0.191532031f, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(1.1875f, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(0.875f, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(0.5625f, transformInterp.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, LerpSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(30.0),
		dsRadiansToDegreesd(-20.0), dsRadiansToDegreesd(10.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);

	double t = 0.375;
	dsRigidTransform3d scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_lerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3d_lerpSIMD2(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.53721097411129237, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.014211490228619311, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.8212897344396517, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.19153478781653371, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625, transformInterp.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3dTest, LerpFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(30.0),
		dsRadiansToDegreesd(-20.0), dsRadiansToDegreesd(10.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);

	double t = 0.375;
	dsRigidTransform3d transformInterp;
	dsRigidTransform3d_lerpFMA2(&transformInterp, &transformA, &transformB, t);

	EXPECT_NEAR(1.25, transformInterp.position.x, epsilon);
	EXPECT_NEAR(6.875, transformInterp.position.y, epsilon);
	EXPECT_NEAR(-15.0, transformInterp.position.z, epsilon);

	EXPECT_NEAR(-0.53721097411129237, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(-0.014211490228619311, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(-0.8212897344396517, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(0.19153478781653371, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(0.875, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(0.5625, transformInterp.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, LerpSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	DS_ALIGN(32) dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(30.0),
		dsRadiansToDegreesd(-20.0), dsRadiansToDegreesd(10.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	DS_ALIGN(32) dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);

	double t = 0.375;
	DS_ALIGN(32) dsRigidTransform3d scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_lerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3d_lerpSIMD4(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.53721097411129237, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.014211490228619311, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.8212897344396517, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.19153478781653371, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625, transformInterp.scale.z, epsilon);
}

TEST(RigidTransform3fTest, NearLerpSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xf positionA = {{-10.0f, 20.0f, -30.0f}};
	dsQuaternion4f orientationA;
	dsQuaternion4f_fromEulerAngles(&orientationA, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scaleA = {{0.1f, 0.2f, 0.3f}};
	dsRigidTransform3f transformA;
	dsRigidTransform3f_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way
	dsVector3xf positionB = {{20.0f, -15.0f, 10.0f}};
	dsQuaternion4f orientationB;
	dsQuaternion4f_fromEulerAngles(&orientationB, dsRadiansToDegreesf(130.0f),
		dsRadiansToDegreesf(-120.0f), dsRadiansToDegreesf(110.0f));
	dsVector3xf scaleB = {{3.0f, 2.0f, 1.0f}};
	dsRigidTransform3f transformB;
	dsRigidTransform3f_initialize(&transformB, &positionB, &orientationB, &scaleB);
	dsRigidTransform3f_makeOrientationConsistent(&transformB, &orientationA);

	float t = 0.375f;
	dsRigidTransform3f scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3f_nearLerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3f_nearLerpSIMD(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25f, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875f, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0f, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.660715282f, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.106790721f, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.742810905f, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.016820224f, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875f, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875f, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625f, transformInterp.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3fTest, NearLerpFMA)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		return;

	float epsilon = RigidTransform3TypeSelector<float>::epsilon;

	dsVector3xf positionA = {{-10.0f, 20.0f, -30.0f}};
	dsQuaternion4f orientationA;
	dsQuaternion4f_fromEulerAngles(&orientationA, dsRadiansToDegreesf(-10.0f),
		dsRadiansToDegreesf(15.0f), dsRadiansToDegreesf(-20.0f));
	dsVector3xf scaleA = {{0.1f, 0.2f, 0.3f}};
	dsRigidTransform3f transformA;
	dsRigidTransform3f_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way
	dsVector3xf positionB = {{20.0f, -15.0f, 10.0f}};
	dsQuaternion4f orientationB;
	dsQuaternion4f_fromEulerAngles(&orientationB, dsRadiansToDegreesf(130.0f),
		dsRadiansToDegreesf(-120.0f), dsRadiansToDegreesf(110.0f));
	dsVector3xf scaleB = {{3.0f, 2.0f, 1.0f}};
	dsRigidTransform3f transformB;
	dsRigidTransform3f_initialize(&transformB, &positionB, &orientationB, &scaleB);
	dsRigidTransform3f_makeOrientationConsistent(&transformB, &orientationA);

	float t = 0.375f;
	dsRigidTransform3f transformInterp;
	dsRigidTransform3f_nearLerpFMA(&transformInterp, &transformA, &transformB, t);

	EXPECT_NEAR(1.25f, transformInterp.position.x, epsilon);
	EXPECT_NEAR(6.875f, transformInterp.position.y, epsilon);
	EXPECT_NEAR(-15.0f, transformInterp.position.z, epsilon);

	EXPECT_NEAR(-0.660715282f, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(0.106790721f, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(-0.742810905f, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(-0.016820224f, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(1.1875f, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(0.875f, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(0.5625f, transformInterp.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, NearLerpSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way
	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(130.0),
		dsRadiansToDegreesd(-120.0), dsRadiansToDegreesd(110.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);
	dsRigidTransform3d_makeOrientationConsistent(&transformB, &orientationA);

	double t = 0.375;
	dsRigidTransform3d scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_nearLerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3d_nearLerpSIMD2(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.66066352072076651, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.10674456069643107, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.74286611721458073, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.016710566869509218, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625, transformInterp.scale.z, epsilon);
}

#if !DS_DETERMINISTIC_MATH
TEST(RigidTransform3dTest, NearLerpFMA2)
{
	dsSIMDFeatures features = dsSIMDFeatures_Double2 | dsSIMDFeatures_FMA;
	if ((dsHostSIMDFeatures & features) != features)
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way
	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(130.0),
		dsRadiansToDegreesd(-120.0), dsRadiansToDegreesd(110.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);
	dsRigidTransform3d_makeOrientationConsistent(&transformB, &orientationA);

	double t = 0.375;
	dsRigidTransform3d transformInterp;
	dsRigidTransform3d_nearLerpFMA2(&transformInterp, &transformA, &transformB, t);

	EXPECT_NEAR(1.25, transformInterp.position.x, epsilon);
	EXPECT_NEAR(6.875, transformInterp.position.y, epsilon);
	EXPECT_NEAR(-15.0, transformInterp.position.z, epsilon);

	EXPECT_NEAR(-0.66066352072076651, transformInterp.orientation.i, epsilon);
	EXPECT_NEAR(0.10674456069643107, transformInterp.orientation.j, epsilon);
	EXPECT_NEAR(-0.74286611721458073, transformInterp.orientation.k, epsilon);
	EXPECT_NEAR(-0.016710566869509218, transformInterp.orientation.r, epsilon);

	EXPECT_NEAR(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_NEAR(0.875, transformInterp.scale.y, epsilon);
	EXPECT_NEAR(0.5625, transformInterp.scale.z, epsilon);
}
#endif // !DS_DETERMINISTIC_MATH

TEST(RigidTransform3dTest, NearLerpSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	double epsilon = RigidTransform3TypeSelector<double>::epsilon;
	DS_UNUSED(epsilon);

	dsVector3xd positionA = {{-10.0, 20.0, -30.0}};
	dsQuaternion4d orientationA;
	dsQuaternion4d_fromEulerAngles(&orientationA, dsRadiansToDegreesd(-10.0),
		dsRadiansToDegreesd(15.0), dsRadiansToDegreesd(-20.0));
	dsVector3xd scaleA = {{0.1, 0.2, 0.3}};
	DS_ALIGN(32) dsRigidTransform3d transformA;
	dsRigidTransform3d_initialize(&transformA, &positionA, &orientationA, &scaleA);

	// Choose a rotation that requires making the orientation consistent to ensure that the lerp
	// goes the correct way
	dsVector3xd positionB = {{20.0, -15.0, 10.0}};
	dsQuaternion4d orientationB;
	dsQuaternion4d_fromEulerAngles(&orientationB, dsRadiansToDegreesd(130.0),
		dsRadiansToDegreesd(-120.0), dsRadiansToDegreesd(110.0));
	dsVector3xd scaleB = {{3.0, 2.0, 1.0}};
	DS_ALIGN(32) dsRigidTransform3d transformB;
	dsRigidTransform3d_initialize(&transformB, &positionB, &orientationB, &scaleB);
	dsRigidTransform3d_makeOrientationConsistent(&transformB, &orientationA);

	double t = 0.375;
	DS_ALIGN(32) dsRigidTransform3d scalarTransformInterp, transformInterp;
	// Not guaranteed to be scalar, but still may be different depending on the compiler settings.
	dsRigidTransform3d_nearLerp(&scalarTransformInterp, &transformA, &transformB, t);
	dsRigidTransform3d_nearLerpSIMD4(&transformInterp, &transformA, &transformB, t);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.x, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.y, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.position.z, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.i, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.j, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.k, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(
		scalarTransformInterp.orientation.r, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.x, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.y, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(scalarTransformInterp.scale.z, transformInterp.scale.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.25, transformInterp.position.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(6.875, transformInterp.position.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(-15.0, transformInterp.position.z, epsilon);

	EXPECT_EQ_DETERMINISTIC(-0.66066352072076651, transformInterp.orientation.i, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.10674456069643107, transformInterp.orientation.j, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.74286611721458073, transformInterp.orientation.k, epsilon);
	EXPECT_EQ_DETERMINISTIC(-0.016710566869509218, transformInterp.orientation.r, epsilon);

	EXPECT_EQ_DETERMINISTIC(1.1875, transformInterp.scale.x, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.875, transformInterp.scale.y, epsilon);
	EXPECT_EQ_DETERMINISTIC(0.5625, transformInterp.scale.z, epsilon);
}

TEST(RigidTransform3fTest, EqualSIMD)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		return;

	dsVector3xf position = {{1.0f, 2.0f, 3.0f}};
	dsQuaternion4f orientation = {{-1.0f, -2.0f, 3.0f, 4.0f}};
	dsQuaternion4f_normalize(&orientation, &orientation);
	dsVector3xf scale = {{6.0f, 5.0f, 4.0f}};
	dsRigidTransform3f transform;
	dsRigidTransform3f_initialize(&transform, &position, &orientation, &scale);

	dsRigidTransform3f otherTransform = transform;
	otherTransform.position.w = 5.0f;
	otherTransform.scale.w = -3.0f;
	EXPECT_TRUE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));

	dsVector4_neg(otherTransform.orientation, otherTransform.orientation);
	EXPECT_TRUE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.position.x = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.y = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.z = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.orientation.i = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.j = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.k = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.r = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.scale.x = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.y = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.z = 20.0f;
	EXPECT_FALSE(dsRigidTransform3f_equalSIMD(&transform, &otherTransform));
}

TEST(RigidTransform3dTest, EqualSIMD2)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double2))
		return;

	dsVector3xd position = {{1.0, 2.0, 3.0}};
	dsQuaternion4d orientation = {{-1.0, -2.0, 3.0, 4.0}};
	dsQuaternion4d_normalize(&orientation, &orientation);
	dsVector3xd scale = {{6.0, 5.0, 4.0}};
	dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, &position, &orientation, &scale);

	dsRigidTransform3d otherTransform = transform;
	otherTransform.position.w = 5.0;
	otherTransform.scale.w = -3.0;
	EXPECT_TRUE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));

	dsVector4_neg(otherTransform.orientation, otherTransform.orientation);
	EXPECT_TRUE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.position.x = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.y = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.z = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.orientation.i = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.j = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.k = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.r = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.scale.x = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.y = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.z = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD2(&transform, &otherTransform));
}

TEST(RigidTransform3dTest, EqualSIMD4)
{
	if (!(dsHostSIMDFeatures & dsSIMDFeatures_Double4))
		return;

	dsVector3xd position = {{1.0, 2.0, 3.0}};
	dsQuaternion4d orientation = {{-1.0, -2.0, 3.0, 4.0}};
	dsQuaternion4d_normalize(&orientation, &orientation);
	dsVector3xd scale = {{6.0, 5.0, 4.0}};
	DS_ALIGN(32) dsRigidTransform3d transform;
	dsRigidTransform3d_initialize(&transform, &position, &orientation, &scale);

	DS_ALIGN(32) dsRigidTransform3d otherTransform = transform;
	otherTransform.position.w = 5.0;
	otherTransform.scale.w = -3.0;
	EXPECT_TRUE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));

	dsVector4_neg(otherTransform.orientation, otherTransform.orientation);
	EXPECT_TRUE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.position.x = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.y = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.position.z = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.orientation.i = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.j = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.k = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.orientation.r = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));

	otherTransform = transform;
	otherTransform.scale.x = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.y = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
	otherTransform = transform;
	otherTransform.scale.z = 20.0;
	EXPECT_FALSE(dsRigidTransform3d_equalSIMD4(&transform, &otherTransform));
}

#endif // DS_HAS_SIMD
