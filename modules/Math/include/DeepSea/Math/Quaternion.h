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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Math/Vector3.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsQuaternion4* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector3* type. All parameters are
 * by value. See the individual macros for whether or not the result can be the same variable as one
 * of the parameters.
 *
 * The functions have different versions for the supported Vector4 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsQuaternion4f dsQuaternion4d
 */

/**
 * @brief Multiplies two quaternions.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
#define dsQuaternion4_mul(result, a, b) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		DS_ASSERT(&(result) != (const void*)&(b)); \
		\
		(result).values[0] = (a).values[0]*(b).values[0] - (a).values[1]*(b).values[1] - \
			(a).values[2]*(b).values[2] - (a).values[3]*(b).values[3]; \
		(result).values[1] = (a).values[0]*(b).values[1] + (b).values[0]*(a).values[1] + \
			(a).values[2]*(b).values[3] - (a).values[3]*(b).values[2]; \
		(result).values[2] = (a).values[0]*(b).values[2] + (b).values[0]*(a).values[2] + \
			(a).values[3]*(b).values[1] - (a).values[1]*(b).values[3]; \
		(result).values[3] = (a).values[0]*(b).values[3] + (b).values[0]*(a).values[3] + \
			(a).values[1]*(b).values[2] - (a).values[2]*(b).values[1]; \
	} while (0)

/**
 * @brief Inverts a quaternion.
 * @param[out] result The inverted result. This may be the same as a.
 * @param a The quaternion to invert. It is assumed that a is a unit quaternion.
 */
#define dsQuaternion4_invert(result, a) \
	do \
	{ \
		(result).values[0] = (a).values[0]; \
		(result).values[1] = -(a).values[1]; \
		(result).values[2] = -(a).values[2]; \
		(result).values[3] = -(a).values[3]; \
	} while (0)

/**
 * @brief Makes a quaternion from Euler angles.
 * @param[out] result The quaternion for the result.
 * @param x The angle around the x axis in radians.
 * @param y The angle around the y axis in radians.
 * @param z The angle around the z axis in radians.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromEulerAngles(dsQuaternion4f* result, float x, float y,
	float z);

/** @copydoc dsQuaternion4f_fromEulerAngles() */
DS_MATH_EXPORT void dsQuaternion4d_fromEulerAngles(dsQuaternion4d* result, double x, double y,
	double z);

/**
 * @brief Makes a quaternion from an axis angle.
 * @param[out] result The quaternion for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromAxisAngle(dsQuaternion4f* result,
	const dsVector3f* axis, float angle);

/** @copydoc dsQuaternion4f_fromAxisAngle() */
DS_MATH_EXPORT void dsQuaternion4d_fromAxisAngle(dsQuaternion4d* result,
	const dsVector3d* axis, double angle);

/**
 * @brief Makes a quaternion from a 3x3 rotation matrix.
 * @param[out] result The quaternion for the result.
 * @param matrix The matrix to extract the rotation from.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromMatrix33(dsQuaternion4f* result,
	const dsMatrix33f* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_fromMatrix33(dsQuaternion4d* result,
	const dsMatrix33d* matrix);

/**
 * @brief Makes a quaternion from a 4x4 rotation matrix.
 * @param[out] result The quaternion for the result.
 * @param matrix The matrix to extract the rotation from.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromMatrix44(dsQuaternion4f* result,
	const dsMatrix44f* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_fromMatrix44(dsQuaternion4d* result,
	const dsMatrix44d* matrix);

/**
 * @brief Gets the X angle from a quaternion.
 * @param a The quaternion.
 * @return The X angle in radians.
 */
DS_MATH_EXPORT inline float dsQuaternion4f_getXAngle(const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getXAngle() */
DS_MATH_EXPORT inline double dsQuaternion4d_getXAngle(const dsQuaternion4d* a);

/**
 * @brief Gets the Y angle from a quaternion.
 * @param a The quaternion.
 * @return The Y angle in radians.
 */
DS_MATH_EXPORT inline float dsQuaternion4f_getYAngle(const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getYAngle() */
DS_MATH_EXPORT inline double dsQuaternion4d_getYAngle(const dsQuaternion4d* a);

/**
 * @brief Gets the Z angle from a quaternion.
 * @param a The quaternion.
 * @return The Z angle in radians.
 */
DS_MATH_EXPORT inline float dsQuaternion4f_getZAngle(const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getZAngle() */
DS_MATH_EXPORT inline double dsQuaternion4d_getZAngle(const dsQuaternion4d* a);

/**
 * @brief Gets the axis that's rotated around from a quaternion.
 * @param[out] result The axis as a unit vector.
 * @param a The quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_getRotationAxis(dsVector3f* result,
	const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getRotationAxis() */
DS_MATH_EXPORT inline void dsQuaternion4d_getRotationAxis(dsVector3d* result,
	const dsQuaternion4d* a);

/**
 * @brief Gets the angle around the rotation axis from a quaternion.
 * @param a The quaternion.
 * @return The angle in radians. This will be in the range [0, pi]. The axis is flipped instead when
 *     rotated outside of this range.
 */
DS_MATH_EXPORT inline float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getAxisAngle() */
DS_MATH_EXPORT inline double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a);

/**
 * @brief Makes a rotation matrix from a quaternion.
 * @param[out] result The matrix for the result.
 * @param a The quaternion to extract the rotation from.
 */
DS_MATH_EXPORT void dsQuaternion4f_toMatrix33(dsMatrix33f* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_toMatrix33(dsMatrix33d* result, const dsQuaternion4d* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4f_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a);

/**
 * @brief Normalizes a quaternion.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_normalize(dsQuaternion4f* result,
	const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_normalize */
DS_MATH_EXPORT inline void dsQuaternion4d_normalize(dsQuaternion4d* result,
	const dsQuaternion4d* a);

/**
 * @brief Rotates a vector by a quaternion.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_rotate(dsVector3f* result, const dsQuaternion4f* a,
	const dsVector3f* v);

/** @copydoc dsQuaternion4f_rotate() */
DS_MATH_EXPORT inline void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a,
	const dsVector3d* v);

/**
 * @brief Performs a spherical linear interpolation between two quaternions.
 * @param[out] result The interpolated result. This may be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 * @param t A value in the range [0, 1] to interpolate between a and b.
 */
DS_MATH_EXPORT void dsQuaternion4f_slerp(dsQuaternion4f* result, const dsQuaternion4f* a,
	const dsQuaternion4f* b, float t);

/** @copydoc dsQuaternion4f_slerp() */
DS_MATH_EXPORT void dsQuaternion4d_slerp(dsQuaternion4d* result, const dsQuaternion4d* a,
	const dsQuaternion4d* b, double t);

/** @copydoc dsQuaternion4_mul() */
DS_MATH_EXPORT inline void dsQuaternion4f_mul(dsQuaternion4f* result, const dsQuaternion4f* a,
	const dsQuaternion4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsQuaternion4_mul(*result, *a, *b);
}

/** @copydoc dsQuaternion4_mul() */
DS_MATH_EXPORT inline void dsQuaternion4d_mul(dsQuaternion4d* result, const dsQuaternion4d* a,
	const dsQuaternion4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsQuaternion4_mul(*result, *a, *b);
}

/** @copydoc dsQuaternion4_invert() */
DS_MATH_EXPORT inline void dsQuaternion4f_invert(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_invert(*result, *a);
}

/** @copydoc dsQuaternion4_invert() */
DS_MATH_EXPORT inline void dsQuaternion4d_invert(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_invert(*result, *a);
}

/// @cond
#define dsQuaternion4_mulToVector(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(b).values[1] + (b).values[0]*(a).values[1] + \
			(a).values[2]*(b).values[3] - (a).values[3]*(b).values[2]; \
		(result).values[1] = (a).values[0]*(b).values[2] + (b).values[0]*(a).values[2] + \
			(a).values[3]*(b).values[1] - (a).values[1]*(b).values[3]; \
		(result).values[2] = (a).values[0]*(b).values[3] + (b).values[0]*(a).values[3] + \
			(a).values[1]*(b).values[2] - (a).values[2]*(b).values[1]; \
	} while (0)
/// @endcond

inline float dsQuaternion4f_getXAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return atan2f(2*(a->r*a->i + a->j*a->k), 1 - 2*(dsPow2(a->i) + dsPow2(a->j)));
}

inline double dsQuaternion4d_getXAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return atan2(2*(a->r*a->i + a->j*a->k), 1 - 2*(dsPow2(a->i) + dsPow2(a->j)));
}

inline float dsQuaternion4f_getYAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return asinf(2*(a->r*a->j - a->k*a->i));
}

inline double dsQuaternion4d_getYAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return asin(2*(a->r*a->j - a->k*a->i));
}

inline float dsQuaternion4f_getZAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return atan2f(2*(a->r*a->k + a->i*a->j), 1 - 2*(dsPow2(a->j) + dsPow2(a->k)));
}

inline double dsQuaternion4d_getZAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return atan2(2*(a->r*a->k + a->i*a->j), 1 - 2*(dsPow2(a->j) + dsPow2(a->k)));
}

inline void dsQuaternion4f_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->x = a->i;
	result->y = a->j;
	result->z = a->k;
	dsVector3f_normalize(result, result);
}

inline void dsQuaternion4d_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->x = a->i;
	result->y = a->j;
	result->z = a->k;
	dsVector3d_normalize(result, result);
}

inline float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return acosf(a->r)*2;
}

inline double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return acos(a->r)*2;
}

inline void dsQuaternion4f_normalize(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4f temp;
	temp.simd = dsSIMD4f_mul(a->simd, a->simd);
	float len2 = temp.x + temp.y + temp.z + temp.w;
	float invLen = 1.0f/sqrtf(len2);
	result->simd = dsSIMD4f_mul(a->simd, dsSIMD4f_set1(invLen));
#else
	float len2 = dsPow2(a->i) + dsPow2(a->j) + dsPow2(a->k) + dsPow2(a->r);
	float invLen = 1.0f/sqrtf(len2);
	result->i = a->i*invLen;
	result->j = a->j*invLen;
	result->k = a->k*invLen;
	result->r = a->r*invLen;
#endif
}

inline void dsQuaternion4d_normalize(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	double len2 = dsPow2(a->i) + dsPow2(a->j) + dsPow2(a->k) + dsPow2(a->r);
	double invLen = 1.0/sqrt(len2);
	result->i = a->i*invLen;
	result->j = a->j*invLen;
	result->k = a->k*invLen;
	result->r = a->r*invLen;
}

inline void dsQuaternion4f_rotate(dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);

	dsQuaternion4f quatV = {{0, v->values[0], v->values[1], v->values[2]}};
	dsQuaternion4f invA, tempQuat;
	dsQuaternion4_invert(invA, *a);
	dsQuaternion4_mul(tempQuat, *a, quatV);
	dsQuaternion4_mulToVector(*result, tempQuat, invA);
}

inline void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);

	dsQuaternion4d quatV = {{0, v->values[0], v->values[1], v->values[2]}};
	dsQuaternion4d invA, tempQuat;
	dsQuaternion4_invert(invA, *a);
	dsQuaternion4_mul(tempQuat, *a, quatV);
	dsQuaternion4_mulToVector(*result, tempQuat, invA);
}

#ifdef __cplusplus
}
#endif
