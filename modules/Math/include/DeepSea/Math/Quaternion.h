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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/SIMD/Dot.h>
#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Sqrt.h>
#include <DeepSea/Math/Trig.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Math/Vector3x.h>

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
		/* Match the ordering for SIMD version for determinism. */ \
		(result).values[0] = ((a).values[0]*(b).values[3] + (a).values[1]*(b).values[2]) + \
			((a).values[3]*(b).values[0] - (a).values[2]*(b).values[1]); \
		(result).values[1] = ((a).values[1]*(b).values[3] + (a).values[2]*(b).values[0]) + \
			((a).values[3]*(b).values[1] - (a).values[0]*(b).values[2]); \
		(result).values[2] = ((a).values[2]*(b).values[3] + (a).values[0]*(b).values[1]) + \
			((a).values[3]*(b).values[2] - (a).values[1]*(b).values[0]); \
		(result).values[3] = -((a).values[0]*(b).values[0] + (a).values[1]*(b).values[1]) + \
			((a).values[3]*(b).values[3] - (a).values[2]*(b).values[2]); \
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
DS_MATH_EXPORT void dsQuaternion4f_fromEulerAngles(
	dsQuaternion4f* result, float x, float y, float z);

/** @copydoc dsQuaternion4f_fromEulerAngles() */
DS_MATH_EXPORT void dsQuaternion4d_fromEulerAngles(
	dsQuaternion4d* result, double x, double y, double z);

/**
 * @brief Makes a quaternion from an axis angle.
 * @param[out] result The quaternion for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromAxisAngle(
	dsQuaternion4f* result, const dsVector3f* axis, float angle);

/** @copydoc dsQuaternion4f_fromAxisAngle() */
DS_MATH_EXPORT void dsQuaternion4d_fromAxisAngle(
	dsQuaternion4d* result, const dsVector3d* axis, double angle);

/**
 * @brief Makes a quaternion from a 3x3 rotation matrix.
 * @param[out] result The quaternion for the result.
 * @param matrix The matrix to extract the rotation from.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromMatrix33(
	dsQuaternion4f* result, const dsMatrix33f* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_fromMatrix33(
	dsQuaternion4d* result, const dsMatrix33d* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4f_fromMatrix33x(
	dsQuaternion4f* result, const dsMatrix33xf* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_fromMatrix33x(
	dsQuaternion4d* result, const dsMatrix33xd* matrix);

/**
 * @brief Makes a quaternion from a 4x4 rotation matrix.
 * @param[out] result The quaternion for the result.
 * @param matrix The matrix to extract the rotation from.
 */
DS_MATH_EXPORT void dsQuaternion4f_fromMatrix44(
	dsQuaternion4f* result, const dsMatrix44f* matrix);

/** @copydoc dsQuaternion4f_fromMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_fromMatrix44(
	dsQuaternion4d* result, const dsMatrix44d* matrix);

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
DS_MATH_EXPORT inline void dsQuaternion4f_getRotationAxis(
	dsVector3f* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getRotationAxis() */
DS_MATH_EXPORT inline void dsQuaternion4d_getRotationAxis(
	dsVector3d* result, const dsQuaternion4d* a);

/** @copydoc dsQuaternion4f_getRotationAxis() */
DS_MATH_EXPORT inline void dsQuaternion4f_getRotationAxis3x(
	dsVector3xf* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_getRotationAxis() */
DS_MATH_EXPORT inline void dsQuaternion4d_getRotationAxis3x(
	dsVector3xd* result, const dsQuaternion4d* a);

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
DS_MATH_EXPORT void dsQuaternion4f_toMatrix33x(dsMatrix33xf* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_toMatrix33x(dsMatrix33xd* result, const dsQuaternion4d* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4f_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_toMatrix33() */
DS_MATH_EXPORT void dsQuaternion4d_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a);

/**
 * @brief Normalizes a quaternion.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_normalize(
	dsQuaternion4f* result, const dsQuaternion4f* a);

/** @copydoc dsQuaternion4f_normalize() */
DS_MATH_EXPORT inline void dsQuaternion4d_normalize(
	dsQuaternion4d* result, const dsQuaternion4d* a);

/**
 * @brief Rotates a vector by a quaternion.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_rotate(
	dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v);

/** @copydoc dsQuaternion4f_rotate() */
DS_MATH_EXPORT inline void dsQuaternion4d_rotate(
	dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v);

/** @copydoc dsQuaternion4f_rotate() */
DS_MATH_EXPORT inline void dsQuaternion4f_rotate3x(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);

/** @copydoc dsQuaternion4f_rotate() */
DS_MATH_EXPORT inline void dsQuaternion4d_rotate3x(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);

/**
 * @brief Performs a spherical linear interpolation between two quaternions.
 * @param[out] result The interpolated result. This may be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 * @param t A value in the range [0, 1] to interpolate between a and b.
 */
DS_MATH_EXPORT void dsQuaternion4f_slerp(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t);

/** @copydoc dsQuaternion4f_slerp() */
DS_MATH_EXPORT void dsQuaternion4d_slerp(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t);

#if DS_HAS_SIMD

/**
 * @brief Multiplies two quaternions using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_mulSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);

/**
 * @brief Takes the conjugate of a quaternion using SIMD operations.
 *
 * When a unit cquaternion, such as for a rotation, this is the same as the inverse.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The conjugated result. This may be the same as a.
 * @param a The quaternion to conjugate.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_conjugateSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a);

/**
 * @brief Normalizes a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_normalizeSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a);

/**
 * @brief Rotates a vector by a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_rotateSIMD(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two quaternions using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_mulFMA(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);

/**
 * @brief Normalizes a quaternion using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_normalizeFMA(
	dsQuaternion4f* result, const dsQuaternion4f* a);

/**
 * @brief Rotates a vector by a quaternion using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4f_rotateFMA(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two quaternions using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_mulSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);

/**
 * @brief Takes the conjugate of a quaternion using SIMD operations.
 *
 * When a unit cquaternion, such as for a rotation, this is the same as the inverse.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The conjugated result. This may be the same as a.
 * @param a The quaternion to conjugate.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_conjugateSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a);

/**
 * @brief Normalizes a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_normalizeSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a);

/**
 * @brief Rotates a vector by a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_rotateSIMD2(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two quaternions using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_mulFMA2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);

/**
 * @brief Normalizes a quaternion using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_normalizeFMA2(
	dsQuaternion4d* result, const dsQuaternion4d* a);

/**
 * @brief Rotates a vector by a quaternion using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_rotateFMA2(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two quaternions using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @remark When combining rotations, multiplication order is reversed. In other words, b is applied
 *     before a.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first quaternion.
 * @param b The second quaternion.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_mulSIMD4(dsQuaternion4d* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsQuaternion4d* DS_ALIGN_PARAM(32) b);

/**
 * @brief Takes the conjugate of a quaternion using SIMD operations.
 *
 * When a unit cquaternion, such as for a rotation, this is the same as the inverse.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The conjugated result. This may be the same as a.
 * @param a The quaternion to conjugate.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_conjugateSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Normalizes a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param result The normalized result. This may be the same as a.
 * @param a The quaternion to normalize.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_normalizeSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Rotates a vector by a quaternion using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The rotated result as a Vector3 type. This may be the same as v.
 * @param a The quaternion to rotate by.
 * @param v The vector to rotate as a Vector3 type.
 */
DS_MATH_EXPORT inline void dsQuaternion4d_rotateSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsVector3xd* DS_ALIGN_PARAM(32) v);

#endif // DS_HAS_SIMD

/// @cond
#define dsQuaternion4_mulVecQuatConj(result, a, b) \
	do \
	{ \
		(result).values[0] = ((a).values[0]*(b).values[3] - (a).values[1]*(b).values[2]) + \
			(a).values[2]*(b).values[1]; \
		(result).values[1] = ((a).values[1]*(b).values[3] - (a).values[2]*(b).values[0]) + \
			(a).values[0]*(b).values[2]; \
		(result).values[2] = ((a).values[2]*(b).values[3] - (a).values[0]*(b).values[1]) + \
			(a).values[1]*(b).values[0]; \
		(result).values[3] = (a).values[0]*(b).values[0] + (a).values[1]*(b).values[1] + \
			(a).values[2]*(b).values[2]; \
	} while (0)

#define dsQuaternion4_mulToVector(result, a, b) \
	do \
	{ \
		(result).values[0] = ((a).values[0]*(b).values[3] + (a).values[1]*(b).values[2]) + \
			((a).values[3]*(b).values[0] - (a).values[2]*(b).values[1]); \
		(result).values[1] = ((a).values[1]*(b).values[3] + (a).values[3]*(b).values[1]) + \
			((a).values[2]*(b).values[0] - (a).values[0]*(b).values[2]); \
		(result).values[2] = ((a).values[2]*(b).values[3] + (a).values[3]*(b).values[2]) + \
			((a).values[0]*(b).values[1] - (a).values[1]*(b).values[0]); \
	} while (0)
/// @endcond

/** @copydoc dsQuaternion4_mul() */
DS_MATH_EXPORT inline void dsQuaternion4f_mul(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_ALWAYS_FMA
	dsQuaternion4f_mulFMA(result, a, b);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsQuaternion4f_mulSIMD(result, a, b);
#else
	dsQuaternion4_mul(*result, *a, *b);
#endif
}

/** @copydoc dsQuaternion4_mul() */
DS_MATH_EXPORT inline void dsQuaternion4d_mul(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsQuaternion4d_mulFMA2(result, a, b);
#else
	dsQuaternion4d_mulSIMD2(result, a, b);
#endif
#else
	dsQuaternion4_mul(*result, *a, *b);
#endif
}

/** @copydoc dsQuaternion4_conjugate() */
DS_MATH_EXPORT inline void dsQuaternion4f_conjugate(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	dsQuaternion4f_conjugateSIMD(result, a);
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
	dsQuaternion4d_conjugateSIMD2(result, a);
#else
	dsQuaternion4_conjugate(*result, *a);
#endif
}

inline float dsQuaternion4f_getXAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return dsATan2f(2*(a->r*a->i + a->j*a->k), 1 - 2*(dsPow2(a->i) + dsPow2(a->j)));
}

inline double dsQuaternion4d_getXAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return dsATan2d(2*(a->r*a->i + a->j*a->k), 1 - 2*(dsPow2(a->i) + dsPow2(a->j)));
}

inline float dsQuaternion4f_getYAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return dsASinf(2*(a->r*a->j - a->k*a->i));
}

inline double dsQuaternion4d_getYAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return dsASind(2*(a->r*a->j - a->k*a->i));
}

inline float dsQuaternion4f_getZAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return dsATan2f(2*(a->r*a->k + a->i*a->j), 1 - 2*(dsPow2(a->j) + dsPow2(a->k)));
}

inline double dsQuaternion4d_getZAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return dsATan2d(2*(a->r*a->k + a->i*a->j), 1 - 2*(dsPow2(a->j) + dsPow2(a->k)));
}

inline void dsQuaternion4f_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	float len2 = dsVector3xf_len2((const dsVector3xf*)a);
	const float epsilon2 = 1e-12f;
	if (len2 < epsilon2)
	{
		result->x = result->y = 0.0f;
		result->z = 1.0f;
	}
	else
	{
		float invLen = 1.0f/dsSqrtf(len2);
		dsVector3_scale(*result, *a, invLen);
	}

	if (a->r < 0)
		dsVector3_neg(*result, *result);
}

inline void dsQuaternion4d_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	double len2 = dsVector3xd_len2((const dsVector3xd*)a);
	const double epsilon2 = 1e-32;
	if (len2 < epsilon2)
	{
		result->x = result->y = 0.0;
		result->z = 1.0;
	}
	else
	{
		double invLen = 1.0/dsSqrtd(len2);
		dsVector3_scale(*result, *a, invLen);
	}

	if (a->r < 0)
		dsVector3_neg(*result, *result);
}

inline void dsQuaternion4f_getRotationAxis3x(dsVector3xf* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	float len2 = dsVector3xf_len2((const dsVector3xf*)a);
	const float epsilon2 = 1e-12f;
	if (len2 < epsilon2)
	{
		result->x = result->y = 0.0f;
		result->z = 1.0f;
	}
	else
	{
		float invLen = 1.0f/dsSqrtf(len2);
		dsVector3xf_scale(result, (const dsVector3xf*)a, invLen);
	}

	if (a->r < 0)
		dsVector3xf_neg(result, result);
}

inline void dsQuaternion4d_getRotationAxis3x(dsVector3xd* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	double len2 = dsVector3xd_len2((const dsVector3xd*)a);
	const double epsilon2 = 1e-32;
	if (len2 < epsilon2)
	{
		result->x = result->y = 0.0;
		result->z = 1.0;
	}
	else
	{
		double invLen = 1.0/dsSqrtd(len2);
		dsVector3xd_scale(result, (const dsVector3xd*)a, invLen);
	}

	if (a->r < 0)
		dsVector3xd_neg(result, result);
}

inline float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a)
{
	DS_ASSERT(a);
	return dsACosf(fabsf(a->r))*2;
}

inline double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a)
{
	DS_ASSERT(a);
	return dsACosd(fabs(a->r))*2;
}

inline void dsQuaternion4f_normalize(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FMA
	dsQuaternion4f_normalizeSIMD(result, a);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsQuaternion4f_normalizeSIMD(result, a);
#else
	float len2 = dsPow2(a->i) + dsPow2(a->j) + dsPow2(a->k) + dsPow2(a->r);
	float invLen = 1.0f/dsSqrtf(len2);
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
#if DS_SIMD_ALWAYS_DOUBLE2
	dsQuaternion4d_normalizeSIMD2(result, a);
#else
	double len2 = dsPow2(a->i) + dsPow2(a->j) + dsPow2(a->k) + dsPow2(a->r);
	double invLen = 1.0/dsSqrtd(len2);
	result->i = a->i*invLen;
	result->j = a->j*invLen;
	result->k = a->k*invLen;
	result->r = a->r*invLen;
#endif
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

inline void dsQuaternion4f_rotate3x(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);
#if DS_SIMD_ALWAYS_FMA
	dsQuaternion4f_rotateFMA(result, a, v);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsQuaternion4f_rotateSIMD(result, a, v);
#else
	dsQuaternion4f tempQuat;
	dsQuaternion4_mulVecQuatConj(tempQuat, *v, *a);
	dsQuaternion4_mulToVector(*result, *a, tempQuat);
#endif
}

inline void dsQuaternion4d_rotate3x(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);
#if DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsQuaternion4d_rotateFMA2(result, a, v);
#else
	dsQuaternion4d_rotateSIMD2(result, a, v);
#endif
#else
	dsQuaternion4d tempQuat;
	dsQuaternion4_mulVecQuatConj(tempQuat, *v, *a);
	dsQuaternion4_mulToVector(*result, *a, tempQuat);
#endif
}

#undef dsQuaternion4_mulVecQuatConj
#undef dsQuaternion4_mulToVector

#if DS_HAS_SIMD

/// @cond
#if DS_X86
#define DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a) \
	do \
	{ \
		(a0120) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(0, 2, 1, 0)); \
		(a1201) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(1, 0, 2, 1)); \
		(a2012) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(2, 1, 0, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		(a3330) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(0, 3, 3, 3)); \
		(a2011) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(1, 1, 0, 2)); \
		(a1202) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(2, 0, 2, 1)); \
	} while (0)
#elif DS_ARM_32
#define DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a23 = vget_high_f32((a)); \
		float32x2x2_t _a2031 = vtrn_f32(_a23, _a01); \
		float32x2_t _a12 = vext_f32(_a01, _a23, 1); \
		(a0120) = vcombine_f32(_a01, _a2031.val[0]); \
		(a1201) = vcombine_f32(_a12, _a01); \
		(a2012) = vcombine_f32(_a2031.val[0], _a12); \
	} while (0)

#define DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a23 = vget_high_f32((a)); \
		float32x2x2_t _a2031 = vtrn_f32(_a23, _a01); \
		(a3330) = vcombine_f32(vdup_lane_f32(_a23, 1), vext_f32(_a23, _a01, 1)); \
		(a2011) = vcombine_f32(_a2031.val[0], vdup_lane_f32(_a01, 1)); \
		(a1202) = vcombine_f32(vext_f32(_a01, _a23, 1), vrev64_f32(_a2031.val[0])); \
	} while (0)
#elif DS_ARM_64
#define DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a23 = vget_high_f32((a)); \
		float32x2_t _a20 = vtrn1_f32(_a23, _a01); \
		float32x2_t _a12 = vext_f32(_a01, _a23, 1); \
		(a0120) = vcombine_f32(_a01, _a20); \
		(a1201) = vcombine_f32(_a12, _a01); \
		(a2012) = vcombine_f32(_a20, _a12); \
	} while (0)

#define DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a23 = vget_high_f32((a)); \
		dsSIMD4f _a3333 = vdupq_laneq_f32((a), 3); \
		(a3330) = vcopyq_laneq_f32(_a3333, 3, (a), 0); \
		(a2011) = vcombine_f32(vtrn1_f32(_a23, _a01), vdup_lane_f32(_a01, 1)); \
		(a1202) = vcombine_f32(vext_f32(_a01, _a23, 1), vtrn1_f32(_a01, _a23)); \
	} while (0)
#else
#error Need to provide quaternion shuffling implementations for this platform.
#endif
/// @endcond

DS_SIMD_START(DS_SIMD_FLOAT4)

inline void dsQuaternion4f_mulSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f a0120, a1201, a2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a->simd);

	dsSIMD4f b3330, b2011, b1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(b3330, b2011, b1202, b->simd);

	dsSIMD4f t1 = dsSIMD4f_mul(a0120, b3330);
	dsSIMD4f t2 = dsSIMD4f_mul(a1201, b2011);
	dsSIMD4f t12 = dsSIMD4f_add(t1, t2);
	t12 = dsSIMD4f_negComponents(t12, 0, 0, 0, 1);

	dsSIMD4f t0 = dsSIMD4f_mul(dsSIMD4f_set1FromVec(a->simd, 3), b->simd);
	dsSIMD4f t3 = dsSIMD4f_mul(a2012, b1202);
	dsSIMD4f t03 = dsSIMD4f_sub(t0, t3);
	result->simd = dsSIMD4f_add(t12, t03);
}

inline void dsQuaternion4f_conjugateSIMD(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->simd = dsSIMD4f_negComponents(a->simd, 1, 1, 1, 0);
}

inline void dsQuaternion4f_normalizeSIMD(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f len2 = dsDot4SIMD4f(a->simd, a->simd);
#if DS_SIMD_EMULATED_DIV_SQRT
	dsSIMD4f invLen = dsSIMD4f_set1(1/dsSqrtf(dsSIMD4f_get(len2, 0)));
#else
	dsSIMD4f invLen = dsSIMD4f_rsqrt(len2);
#endif
	result->simd = dsSIMD4f_mul(a->simd, invLen);
}

inline void dsQuaternion4f_rotateSIMD(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);

	// v*a(conjugate), accounting for the fact that as a vector v.r is 0.
	dsSIMD4f v0120, v1201, v2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(v0120, v1201, v2012, v->simd);

	dsSIMD4f a3330, a2011, a1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a->simd);

	dsSIMD4f t1 = dsSIMD4f_mul(v0120, a3330);
	dsSIMD4f t2 = dsSIMD4f_mul(v1201, a2011);
	t2 = dsSIMD4f_negComponents(t2, 1, 1, 1, 0);
	dsSIMD4f t12 = dsSIMD4f_add(t1, t2);

	dsSIMD4f t3 = dsSIMD4f_mul(v2012, a1202);

	dsQuaternion4f va;
	va.simd = dsSIMD4f_add(t12, t3);

	dsQuaternion4f_mulSIMD((dsQuaternion4f*)result, a, &va);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

inline void dsQuaternion4f_mulFMA(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f a0120, a1201, a2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a->simd);

	dsSIMD4f b3330, b2011, b1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(b3330, b2011, b1202, b->simd);

	dsSIMD4f t12 = dsSIMD4f_fmadd(a0120, b3330, dsSIMD4f_mul(a1201, b2011));
	t12 = dsSIMD4f_negComponents(t12, 0, 0, 0, 1);

	dsSIMD4f t312 = dsSIMD4f_fnmadd(a2012, b1202, t12);
	result->simd = dsSIMD4f_fmadd(dsSIMD4f_set1FromVec(a->simd, 3), b->simd, t312);
}

inline void dsQuaternion4f_normalizeFMA(dsQuaternion4f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f len2 = dsDot4FMA4f(a->simd, a->simd);
#if DS_SIMD_EMULATED_DIV_SQRT
	dsSIMD4f invLen = dsSIMD4f_set1(1/dsSqrtf(dsSIMD4f_get(len2, 0)));
#else
	dsSIMD4f invLen = dsSIMD4f_rsqrt(len2);
#endif
	result->simd = dsSIMD4f_mul(a->simd, invLen);
}

inline void dsQuaternion4f_rotateFMA(
	dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);

	// v*a(conjugate), accounting for the fact that as a vector v.r is 0.
	dsSIMD4f v0120, v1201, v2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(v0120, v1201, v2012, v->simd);

	dsSIMD4f a3330, a2011, a1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a->simd);

	dsSIMD4f t2 = dsSIMD4f_mul(v1201, a2011);
	t2 = dsSIMD4f_negComponents(t2, 1, 1, 1, 0);
	dsSIMD4f t12 = dsSIMD4f_fmadd(v0120, a3330, t2);

	dsQuaternion4f va;
	va.simd = dsSIMD4f_fmadd(v2012, a1202, t12);

	dsQuaternion4f_mulFMA((dsQuaternion4f*)result, a, &va);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

#undef DS_SIMD_SHUFFLE_0120_1201_2012
#undef DS_SIMD_SHUFFLE_3330_2011_1202

/// @cond
#if DS_X86
#define DS_SIMD_SHUFFLE_20_12(a20, a12, a) \
	do \
	{ \
		(a20) = _mm_shuffle_pd((a).simd2[1], (a).simd2[0], _MM_SHUFFLE2(0, 0)); \
		(a12) = _mm_shuffle_pd((a).simd2[0], (a).simd2[1], _MM_SHUFFLE2(0, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE_30_02(a30, a02, a) \
	do \
	{ \
		(a30) = _mm_shuffle_pd((a).simd2[1], (a).simd2[0], _MM_SHUFFLE2(0, 1)); \
		(a02) = _mm_shuffle_pd((a).simd2[0], (a).simd2[1], _MM_SHUFFLE2(0, 0)); \
	} while (0)
#elif DS_ARM_64
#define DS_SIMD_SHUFFLE_20_12(a20, a12, a) \
	do \
	{ \
		(a20) = vtrn1q_f64((a).simd2[1], (a).simd2[0]); \
		(a12) = vextq_f64((a).simd2[0], (a).simd2[1], 1); \
	} while (0)

#define DS_SIMD_SHUFFLE_30_02(a30, a02, a) \
	do \
	{ \
		(a30) = vextq_f64((a).simd2[1], (a).simd2[0], 1); \
		(a02) = vtrn1q_f64((a).simd2[0], (a).simd2[1]); \
	} while (0)
#else
#define DS_SIMD_SHUFFLE_20_12(a20, a12, a) \
	do \
	{ \
		(a20) = (a).simd2[0]; \
		(a12) = (a).simd2[1]; \
	} while (0)

#define DS_SIMD_SHUFFLE_30_02(a30, a02, a) \
	do \
	{ \
		(a30) = (a).simd2[0]; \
		(a02) = (a).simd2[1]; \
	} while (0)
#endif
/// @endcond

DS_SIMD_START(DS_SIMD_DOUBLE2)

inline void dsQuaternion4d_mulSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d a20, a12;
	DS_SIMD_SHUFFLE_20_12(a20, a12, *a);
	dsSIMD2d a33 = dsSIMD2d_set1FromVec(a->simd2[1], 1);

	dsSIMD2d b20, b12, b30, b02;
	DS_SIMD_SHUFFLE_20_12(b20, b12, *b);
	DS_SIMD_SHUFFLE_30_02(b30, b02, *b);

	dsSIMD2d t1_0 = dsSIMD2d_mul(a->simd2[0], dsSIMD2d_set1FromVec(b->simd2[1], 1));
	dsSIMD2d t1_1 = dsSIMD2d_mul(a20, b30);
	dsSIMD2d t2_0 = dsSIMD2d_mul(a12, b20);
	dsSIMD2d t2_1 = dsSIMD2d_mul(a->simd2[0], dsSIMD2d_set1FromVec(b->simd2[0], 1));
	dsSIMD2d t12_0 = dsSIMD2d_add(t1_0, t2_0);
	dsSIMD2d t12_1 = dsSIMD2d_add(t1_1, t2_1);
	t12_1 = dsSIMD2d_negComponents(t12_1, 0, 1);

	dsSIMD2d t0_0 = dsSIMD2d_mul(a33, b->simd2[0]);
	dsSIMD2d t0_1 = dsSIMD2d_mul(a33, b->simd2[1]);
	dsSIMD2d t3_0 = dsSIMD2d_mul(a20, b12);
	dsSIMD2d t3_1 = dsSIMD2d_mul(a12, b02);
	dsSIMD2d t03_0 = dsSIMD2d_sub(t0_0, t3_0);
	dsSIMD2d t03_1 = dsSIMD2d_sub(t0_1, t3_1);
	result->simd2[0] = dsSIMD2d_add(t12_0, t03_0);
	result->simd2[1] = dsSIMD2d_add(t12_1, t03_1);
}

inline void dsQuaternion4d_conjugateSIMD2(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->simd2[0] = dsSIMD2d_neg(a->simd2[0]);
	result->simd2[1] = dsSIMD2d_negComponents(a->simd2[1], 1, 0);
}

inline void dsQuaternion4d_normalizeSIMD2(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD2d len2 = dsDot4SIMD2d(a->simd2[0], a->simd2[1], a->simd2[0], a->simd2[1]);
	dsSIMD2d invLen = dsSIMD2d_rsqrt(len2);
	result->simd2[0] = dsSIMD2d_mul(a->simd2[0], invLen);
	result->simd2[1] = dsSIMD2d_mul(a->simd2[1], invLen);
}

inline void dsQuaternion4d_rotateSIMD2(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);

	// v*a(conjugate), accounting for the fact that as a vector v.r is 0.
	dsSIMD2d v20, v12;
	DS_SIMD_SHUFFLE_20_12(v20, v12, *v);

	dsSIMD2d a20, a12, a30, a02;
	DS_SIMD_SHUFFLE_20_12(a20, a12, *a);
	DS_SIMD_SHUFFLE_30_02(a30, a02, *a);

	dsSIMD2d t1_0 = dsSIMD2d_mul(v->simd2[0], dsSIMD2d_set1FromVec(a->simd2[1], 1));
	dsSIMD2d t1_1 = dsSIMD2d_mul(v20, a30);
	dsSIMD2d t2_0 = dsSIMD2d_mul(v12, a20);
	dsSIMD2d t2_1 = dsSIMD2d_mul(v->simd2[0], dsSIMD2d_set1FromVec(a->simd2[0], 1));
	t2_0 = dsSIMD2d_neg(t2_0);
	t2_1 = dsSIMD2d_negComponents(t2_1, 1, 0);
	dsSIMD2d t12_0 = dsSIMD2d_add(t1_0, t2_0);
	dsSIMD2d t12_1 = dsSIMD2d_add(t1_1, t2_1);

	dsSIMD2d t3_0 = dsSIMD2d_mul(v20, a12);
	dsSIMD2d t3_1 = dsSIMD2d_mul(v12, a02);

	dsQuaternion4d va;
	va.simd2[0] = dsSIMD2d_add(t12_0, t3_0);
	va.simd2[1] = dsSIMD2d_add(t12_1, t3_1);

	dsQuaternion4d_mulSIMD2((dsQuaternion4d*)result, a, &va);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

inline void dsQuaternion4d_mulFMA2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d a20, a12;
	DS_SIMD_SHUFFLE_20_12(a20, a12, *a);
	dsSIMD2d a33 = dsSIMD2d_set1FromVec(a->simd2[1], 1);

	dsSIMD2d b20, b12, b30, b02;
	DS_SIMD_SHUFFLE_20_12(b20, b12, *b);
	DS_SIMD_SHUFFLE_30_02(b30, b02, *b);

	dsSIMD2d t12_0 = dsSIMD2d_fmadd(
		a->simd2[0], dsSIMD2d_set1FromVec(b->simd2[1], 1), dsSIMD2d_mul(a12, b20));
	dsSIMD2d t12_1 = dsSIMD2d_fmadd(
		a20, b30, dsSIMD2d_mul(a->simd2[0], dsSIMD2d_set1FromVec(b->simd2[0], 1)));
	t12_1 = dsSIMD2d_negComponents(t12_1, 0, 1);

	dsSIMD2d t312_0 = dsSIMD2d_fnmadd(a20, b12, t12_0);
	dsSIMD2d t312_1 = dsSIMD2d_fnmadd(a12, b02, t12_1);
	result->simd2[0] = dsSIMD2d_fmadd(a33, b->simd2[0], t312_0);
	result->simd2[1] = dsSIMD2d_fmadd(a33, b->simd2[1], t312_1);
}

inline void dsQuaternion4d_normalizeFMA2(dsQuaternion4d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD2d len2 = dsDot4FMA2d(a->simd2[0], a->simd2[1], a->simd2[0], a->simd2[1]);
	dsSIMD2d invLen = dsSIMD2d_rsqrt(len2);
	result->simd2[0] = dsSIMD2d_mul(a->simd2[0], invLen);
	result->simd2[1] = dsSIMD2d_mul(a->simd2[1], invLen);
}

inline void dsQuaternion4d_rotateFMA2(
	dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);

	// v*a(conjugate), accounting for the fact that as a vector v.r is 0.
	dsSIMD2d v20, v12;
	DS_SIMD_SHUFFLE_20_12(v20, v12, *v);

	dsSIMD2d a20, a12, a30, a02;
	DS_SIMD_SHUFFLE_20_12(a20, a12, *a);
	DS_SIMD_SHUFFLE_30_02(a30, a02, *a);

	dsSIMD2d t2_0 = dsSIMD2d_mul(v12, a20);
	dsSIMD2d t2_1 = dsSIMD2d_mul(v->simd2[0], dsSIMD2d_set1FromVec(a->simd2[0], 1));
	t2_0 = dsSIMD2d_neg(t2_0);
	t2_1 = dsSIMD2d_negComponents(t2_1, 1, 0);
	dsSIMD2d t12_0 = dsSIMD2d_fmadd(v->simd2[0], dsSIMD2d_set1FromVec(a->simd2[1], 1), t2_0);
	dsSIMD2d t12_1 = dsSIMD2d_fmadd(v20, a30, t2_1);

	dsQuaternion4d va;
	va.simd2[0] = dsSIMD2d_fmadd(v20, a12, t12_0);
	va.simd2[1] = dsSIMD2d_fmadd(v12, a02, t12_1);

	dsQuaternion4d_mulFMA2((dsQuaternion4d*)result, a, &va);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

#undef DS_SIMD_SHUFFLE_20_12
#undef DS_SIMD_SHUFFLE_30_02

/// @cond
#if DS_X86
#define DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a) \
	do \
	{ \
		(a0120) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 2, 1, 0)); \
		(a1201) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(1, 0, 2, 1)); \
		(a2012) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(2, 1, 0, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		(a3330) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 3, 3, 3)); \
		(a2011) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(1, 1, 0, 2)); \
		(a1202) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(2, 0, 2, 1)); \
	} while (0)
#else
#define DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, a) \
	do \
	{ \
		(a0120) = (a); \
		(a1201) = (a); \
		(a2012) = (a); \
	} while(0)

#define DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, a) \
	do \
	{ \
		(a3330) = (a); \
		(a2011) = (a); \
		(a1202) = (a); \
	} while (0)
#endif
/// @endcond

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA)

inline void dsQuaternion4d_mulSIMD4(dsQuaternion4d* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsQuaternion4d* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d simdA = dsSIMD4d_load(a);
	dsSIMD4d simdB = dsSIMD4d_load(b);

	dsSIMD4d a0120, a1201, a2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(a0120, a1201, a2012, simdA);

	dsSIMD4d b3330, b2011, b1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(b3330, b2011, b1202, simdB);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d t12 = dsSIMD4d_fmadd(a0120, b3330, dsSIMD4d_mul(a1201, b2011));
#else
	dsSIMD4d t1 = dsSIMD4d_mul(a0120, b3330);
	dsSIMD4d t2 = dsSIMD4d_mul(a1201, b2011);
	dsSIMD4d t12 = dsSIMD4d_add(t1, t2);
#endif
	t12 = dsSIMD4d_negComponents(t12, 0, 0, 0, 1);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d t312 = dsSIMD4d_fnmadd(a2012, b1202, t12);
	dsSIMD4d_store(result, dsSIMD4d_fmadd(dsSIMD4d_set1FromVec(simdA, 3), simdB, t312));
#else
	dsSIMD4d t0 = dsSIMD4d_mul(dsSIMD4d_set1FromVec(simdA, 3), simdB);
	dsSIMD4d t3 = dsSIMD4d_mul(a2012, b1202);
	dsSIMD4d t03 = dsSIMD4d_sub(t0, t3);
	dsSIMD4d_store(result, dsSIMD4d_add(t12, t03));
#endif
}

inline void dsQuaternion4d_conjugateSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d simdA = dsSIMD4d_load(a);
	dsSIMD4d_store(result, dsSIMD4d_negComponents(simdA, 1, 1, 1, 0));
}

inline void dsQuaternion4d_normalizeSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d simdA = dsSIMD4d_load(a);
	dsSIMD4d len2 = dsDot4SIMD4d(simdA, simdA);
	dsSIMD4d invLen = dsSIMD4d_rsqrt(len2);
	dsSIMD4d_store(result, dsSIMD4d_mul(simdA, invLen));
}

inline void dsQuaternion4d_rotateSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsVector3xd* DS_ALIGN_PARAM(32) v)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(v);
	DS_ASSERT(result != v);

	dsSIMD4d simdA = dsSIMD4d_load(a);
	dsSIMD4d simdV = dsSIMD4d_load(v);

	// v*a(conjugate), accounting for the fact that as a vector v.r is 0.
	dsSIMD4d v0120, v1201, v2012;
	DS_SIMD_SHUFFLE_0120_1201_2012(v0120, v1201, v2012, simdV);

	dsSIMD4d a3330, a2011, a1202;
	DS_SIMD_SHUFFLE_3330_2011_1202(a3330, a2011, a1202, simdA);

	dsSIMD4d t2 = dsSIMD4d_mul(v1201, a2011);
	t2 = dsSIMD4d_negComponents(t2, 1, 1, 1, 0);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d t12 = dsSIMD4d_fmadd(v0120, a3330, t2);
#else
	dsSIMD4d t12 = dsSIMD4d_add(dsSIMD4d_mul(v0120, a3330), t2);
#endif

	DS_ALIGN(32) dsQuaternion4d va;
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(&va, dsSIMD4d_fmadd(v2012, a1202, t12));
#else
	dsSIMD4d_store(&va, dsSIMD4d_add(t12, dsSIMD4d_mul(v2012, a1202)));
#endif

	dsQuaternion4d_mulSIMD4((dsQuaternion4d*)result, a, &va);
}

DS_SIMD_END()

#undef DS_SIMD_SHUFFLE_0120_1201_2012
#undef DS_SIMD_SHUFFLE_3330_2011_1202

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
