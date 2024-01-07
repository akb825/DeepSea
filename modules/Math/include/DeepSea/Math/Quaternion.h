/*
 * Copyright 2020-2024 Aaron Barany
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

#include <DeepSea/Math/SIMD/SIMD.h>
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
 * The dsVector4f and dsVector4d functions may use SIMD operations when guaranteed to be available.
 *
 * @see dsQuaternion4f dsQuaternion4d
 */

/**
 * @brief Sets a quaternion to an identity rotation value.
 * @param[out] result The quaternion to set.
 */
#define dsQuaternion4_identityRotation(result) \
	do \
	{ \
		(result).values[0] = 0; \
		(result).values[1] = 0; \
		(result).values[2] = 0; \
		(result).values[3] = 1; \
	} while (0)

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
		(result).values[0] = (a).values[3]*(b).values[0] + (b).values[3]*(a).values[0] + \
			(a).values[1]*(b).values[2] - (a).values[2]*(b).values[1]; \
		(result).values[1] = (a).values[3]*(b).values[1] + (b).values[3]*(a).values[1] + \
			(a).values[2]*(b).values[0] - (a).values[0]*(b).values[2]; \
		(result).values[2] = (a).values[3]*(b).values[2] + (b).values[3]*(a).values[2] + \
			(a).values[0]*(b).values[1] - (a).values[1]*(b).values[0]; \
		(result).values[3] = (a).values[3]*(b).values[3] - (a).values[0]*(b).values[0] - \
			(a).values[1]*(b).values[1] - (a).values[2]*(b).values[2]; \
	} while (0)

/**
 * @brief Takes the conjugate of a quaternion.
 *
 * When a unit cquaternion, such as for a rotation, this is the same as the inverse.
 *
 * @param[out] result The conjugated result. This may be the same as a.
 * @param a The quaternion to conjugate.
 */
#define dsQuaternion4_conjugate(result, a) \
	do \
	{ \
		(result).values[0] = -(a).values[0]; \
		(result).values[1] = -(a).values[1]; \
		(result).values[2] = -(a).values[2]; \
		(result).values[3] = (a).values[3]; \
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

/** @copydoc dsQuaternion4f_normalize() */
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

/// @cond
#define dsQuaternion4_mulVecQuatConj(result, a, b) \
	do \
	{ \
		(result).values[0] = (b).values[3]*(a).values[0] - (a).values[1]*(b).values[2] + \
			(a).values[2]*(b).values[1]; \
		(result).values[1] = (b).values[3]*(a).values[1] - (a).values[2]*(b).values[0] + \
			(a).values[0]*(b).values[2]; \
		(result).values[2] = (b).values[3]*(a).values[2] - (a).values[0]*(b).values[1] + \
			(a).values[1]*(b).values[0]; \
		(result).values[3] = (a).values[0]*(b).values[0] + (a).values[1]*(b).values[1] + \
			(a).values[2]*(b).values[2]; \
	} while (0)

#define dsQuaternion4_mulToVector(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[3]*(b).values[0] + (b).values[3]*(a).values[0] + \
			(a).values[1]*(b).values[2] - (a).values[2]*(b).values[1]; \
		(result).values[1] = (a).values[3]*(b).values[1] + (b).values[3]*(a).values[1] + \
			(a).values[2]*(b).values[0] - (a).values[0]*(b).values[2]; \
		(result).values[2] = (a).values[3]*(b).values[2] + (b).values[3]*(a).values[2] + \
			(a).values[0]*(b).values[1] - (a).values[1]*(b).values[0]; \
	} while (0)

#if DS_HAS_SIMD && (DS_X86_32 || DS_X86_64)
// NOTE: Only SSE instructions have efficient shuffle. Without it, there aren't enough operations to
// offset the cost for manually shuffling the vectors with MOV operations. (e.g. on NEON)
#define dsQuaternion4f_shuffle_0120_1201_2012_3333(a0120, a1201, a2012, a3333, a) \
	do \
	{ \
		(a0120) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(0, 2, 1, 0)); \
		(a1201) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(1, 0, 2, 1)); \
		(a2012) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(2, 1, 0, 2)); \
		(a3333) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(3, 3, 3, 3)); \
	} while (0)

#define dsQuaternion4f_shuffle_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		(a3330) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(0, 3, 3, 3)); \
		(a2011) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(1, 1, 0, 2)); \
		(a1202) = _mm_shuffle_ps(((a).simd), ((a).simd), _MM_SHUFFLE(2, 0, 2, 1)); \
	} while (0)
#endif
/// @endcond

/** @copydoc dsQuaternion4_mul() */
DS_MATH_EXPORT inline void dsQuaternion4f_mul(dsQuaternion4f* result, const dsQuaternion4f* a,
	const dsQuaternion4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

#if DS_SIMD_ALWAYS_FLOAT4 && defined(dsQuaternion4f_shuffle_0120_1201_2012_3333)
	dsSIMD4f a0120, a1201, a2012, a3333;
	dsQuaternion4f_shuffle_0120_1201_2012_3333(a0120, a1201, a2012, a3333, *a);

	dsSIMD4f b3330, b2011, b1202;
	dsQuaternion4f_shuffle_3330_2011_1202(b3330, b2011, b1202, *b);

	dsSIMD4f t2 = dsSIMD4f_mul(a1201, b2011);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4f t12 = dsSIMD4f_fmadd(a0120, b3330, t2);
#else
	dsSIMD4f t1 = dsSIMD4f_mul(a0120, b3330);
	dsSIMD4f t12 = dsSIMD4f_add(t1, t2);
#endif
	t12 = dsSIMD4f_negComponents(t12, 0, 0, 0, 1);

	dsSIMD4f t3 = dsSIMD4f_mul(a2012, b1202);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4f t03 = dsSIMD4f_fmsub(a3333, b->simd, t3);
#else
	dsSIMD4f t0 = dsSIMD4f_mul(a3333, b->simd);
	dsSIMD4f t03 = dsSIMD4f_sub(t0, t3);
#endif

	result->simd = dsSIMD4f_add(t03, t12);
#else
	dsQuaternion4_mul(*result, *a, *b);
#endif
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

/** @copydoc dsQuaternion4_conjugate() */
DS_MATH_EXPORT inline void dsQuaternion4f_conjugate(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_negComponents(a->simd, 1, 1, 1, 0);
#else
	dsQuaternion4_conjugate(*result, *a);
#endif
}

/** @copydoc dsQuaternion4_conjugate() */
DS_MATH_EXPORT inline void dsQuaternion4d_conjugate(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_neg(a->simd2[0]);
	result->simd2[1] = dsSIMD2d_negComponents(a->simd2[1], 1, 0);
#else
	dsQuaternion4_conjugate(*result, *a);
#endif
}

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

	dsQuaternion4f tempQuat;
	dsQuaternion4_mulVecQuatConj(tempQuat, *v, *a);
	dsQuaternion4_mulToVector(*result, *a, tempQuat);
}

inline void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);

	dsQuaternion4d tempQuat;
	dsQuaternion4_mulVecQuatConj(tempQuat, *v, *a);
	dsQuaternion4_mulToVector(*result, *a, tempQuat);
}

#undef dsQuaternion4_mulVecQuatConj
#undef dsQuaternion4_mulToVector
#undef dsQuaternion4f_shuffle_0120_1201_2012_3333
#undef dsQuaternion4f_shuffle_3330_2011_1202

#ifdef __cplusplus
}
#endif
