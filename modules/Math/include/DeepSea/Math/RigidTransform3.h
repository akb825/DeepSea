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

#pragma once

#include <DeepSea/Core/Config.h>

#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3x.h>
#include <DeepSea/Math/Vector4.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating 3D rigid transforms.
 * @see dsRigidTransform3f dsRigidTransform3d
 */

/**
 * @brief Initializes a rigid transform with its components.
 * @param[out] result The rigid transform.
 * @param position The position of the transform. This may be NULL to set the position to 0.
 * @param orientation The orientation of the transform. This may be NULL to use the identity
 *     rotation. The orientation should be normalized if provided.
 * @param scale The scale of the transform. This may be NULL to have a scale of 1.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_initialize(dsRigidTransform3f* result,
	const dsVector3xf* position, const dsQuaternion4f* orientation, const dsVector3xf* scale);

/** @copydoc dsRigidTransform3f_initialize() */
DS_MATH_EXPORT inline void dsRigidTransform3d_initialize(dsRigidTransform3d* result,
	const dsVector3xd* position, const dsQuaternion4d* orientation, const dsVector3xd* scale);

/**
 * @brief Extracts the rigid transform from a matrix.
 * @param[out] result The rigid transform.
 * @param matrix The transform matrix. It is expected to be explicitly a rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_fromMatrix(
	dsRigidTransform3f* result, const dsMatrix44f* matrix);

/** @copydoc dsRigidTransform3f_fromMatrix() */
DS_MATH_EXPORT inline void dsRigidTransform3d_fromMatrix(
	dsRigidTransform3d* result, const dsMatrix44d* matrix);

/**
 * @brief Converts the rigid transform to a matrix.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_toMatrix(
	dsMatrix44f* result, const dsRigidTransform3f* transform);

/** @copydoc dsRigidTransform3f_toMatrix() */
DS_MATH_EXPORT inline void dsRigidTransform3d_toMatrix(
	dsMatrix44d* result, const dsRigidTransform3d* transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_makeOrientationConsistent(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);

/** @copydoc dsRigidTransform3f_makeOrientationConsistent() */
DS_MATH_EXPORT inline void dsRigidTransform3d_makeOrientationConsistent(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);

/**
 * @brief Checks whether a multiply will result in a rigid transform.
 *
 * If this returns false, calling dsRigidTransform3f_mul() will give an inconsistent result compared
 * to a matrix multiply, where a matrix multiply would result in a shear.
 *
 * @param a The first transform.
 * @param b The second transform.
 * @return Whether a multiply will result in a rigid transform.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3f_isMulValid(
	const dsRigidTransform3f* a, const dsRigidTransform3f* b);

/**
 * @brief Checks whether a multiply will result in a rigid transform.
 *
 * If this returns false, calling dsRigidTransform3d_mul() will give an inconsistent result compared
 * to a matrix multiply, where a matrix multiply would result in a shear.
 *
 * @param a The first transform.
 * @param b The second transform.
 * @return Whether a multiply will result in a rigid transform.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3d_isMulValid(
	const dsRigidTransform3d* a, const dsRigidTransform3d* b);

/**
 * @brief Multiplies two rigid transforms.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *     a non-identity rotation, as this would result in a non-rigid transform.
 * @param[out] result The result of the multiplication. This may NOT be the same as a or b.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_mul(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);

/** @copydoc dsRigidTransform3f_mul() */
DS_MATH_EXPORT inline void dsRigidTransform3d_mul(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);

/**
 * @brief Transforms a point with a rigid transform.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_transform(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);

/** dsRigidTransform3f_transform() */
DS_MATH_EXPORT inline void dsRigidTransform3d_transform(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);

/**
 * @brief Interpolates between two rigid transforms.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_lerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

/** @copydoc dsRigidTransform3f_lerp() */
DS_MATH_EXPORT inline void dsRigidTransform3d_lerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3f_makeOrientationConsistent().
 *
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_nearLerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3d_makeOrientationConsistent().
 *
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_nearLerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

/**
 * @brief Checks if two rigid transforms are exactly equal.
 *
 * This will consider equivalent orientations (where one is negated from the other) to be equal.
 *
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @return True if a == b.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3f_equal(
	const dsRigidTransform3f* a, const dsRigidTransform3f* b);

/** @copydoc dsRigidTransform3f_equal() */
DS_MATH_EXPORT inline bool dsRigidTransform3d_equal(
	const dsRigidTransform3d* a, const dsRigidTransform3d* b);

#if DS_HAS_SIMD

/**
 * @brief Extracts the rigid transform from a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The rigid transform.
 * @param matrix The transform matrix. It is expected to be explicitly a rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_fromMatrixSIMD(
	dsRigidTransform3f* result, const dsMatrix44f* matrix);

/**
 * @brief Converts the rigid transform to a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_toMatrixSIMD(
	dsMatrix44f* result, const dsRigidTransform3f* transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation using
 * SIMD operations.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_makeOrientationConsistentSIMD(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);

/**
 * @brief Multiplies two rigid transforms using SIMD operations.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *    a non-identity rotation, as this would result in a non-rigid transform.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This may NOT be the same as a or b.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_mulSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);

/**
 * @brief Transforms a point with a rigid transform using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_transformSIMD(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);

/**
 * @brief Interpolates between two rigid transforms using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_lerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other using
 * SIMD operations.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3f_makeOrientationConsistent().
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_nearLerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

/**
 * @brief Checks if two rigid transforms are exactly equal using SIMD operations.
 *
 * This will consider equivalent orientations (where one is negated from the other) to be equal.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @return True if a == b.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3f_equalSIMD(
	const dsRigidTransform3f* a, const dsRigidTransform3f* b);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Converts the rigid transform to a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_toMatrixFMA(
	dsMatrix44f* result, const dsRigidTransform3f* transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation using
 * fused multiply-add operations.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_makeOrientationConsistentFMA(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation);

/**
 * @brief Multiplies two rigid transforms using fused multiply-add operations.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *     a non-identity rotation, as this would result in a non-rigid transform.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This may NOT be the same as a or b.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_mulFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b);

/**
 * @brief Transforms a point with a rigid transform using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_transformFMA(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point);

/**
 * @brief Interpolates between two rigid transforms using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_lerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other using
 * fused multply-add operations.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3f_makeOrientationConsistent().
 *
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3f_nearLerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Extracts the rigid transform from a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The rigid transform.
 * @param matrix The transform matrix. It is expected to be explicitly a rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_fromMatrixSIMD2(
	dsRigidTransform3d* result, const dsMatrix44d* matrix);

/**
 * @brief Converts the rigid transform to a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_toMatrixSIMD2(
	dsMatrix44d* result, const dsRigidTransform3d* transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation using
 * SIMD operations.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_makeOrientationConsistentSIMD2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);

/**
 * @brief Multiplies two rigid transforms using SIMD operations.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *     a non-identity rotation, as this would result in a non-rigid transform.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the multiplication. This may NOT be the same as a or b.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_mulSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);

/**
 * @brief Transforms a point with a rigid transform using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_transformSIMD2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);

/**
 * @brief Interpolates between two rigid transforms using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_lerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other using
 * SIMD operations.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3d_makeOrientationConsistent().
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_nearLerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

/**
 * @brief Checks if two rigid transforms are exactly equal using SIMD operations.
 *
 * This will consider equivalent orientations (where one is negated from the other) to be equal.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @return True if a == b.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3d_equalSIMD2(
	const dsRigidTransform3d* a, const dsRigidTransform3d* b);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Converts the rigid transform to a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_toMatrixFMA2(
	dsMatrix44d* result, const dsRigidTransform3d* transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation using
 * fused multiply-add operations.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_makeOrientationConsistentFMA2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation);

/**
 * @brief Multiplies two rigid transforms using fused multiply-add operations.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *     a non-identity rotation, as this would result in a non-rigid transform.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This may NOT be the same as a or b.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_mulFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b);

/**
 * @brief Transforms a point with a rigid transform using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_transformFMA2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point);

/**
 * @brief Interpolates between two rigid transforms using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_lerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other using
 * fused multply-add operations.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3d_makeOrientationConsistent().
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_nearLerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Extracts the rigid transform from a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The rigid transform.
 * @param matrix The transform matrix. It is expected to be explicitly a rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_fromMatrixSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix);

/**
 * @brief Converts the rigid transform to a matrix using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The matrix for the transform.
 * @param transform The rigid transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) transform);

/**
 * @brief Makes the orientation of a transform to be consistent with a reference orientation using
 * SIMD operations.
 *
 * The orientation will be equivalent, but it will ensure that using a lerp to interpolate between
 * two close orientations will use the shortest angular distance.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param transform The rigid transform to make consistent.
 * @param otherOrientation The other orientation to make the transform's orientation consistent
 *     with.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_makeOrientationConsistentSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) transform,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) otherOrientation);

/**
 * @brief Multiplies two rigid transforms using SIMD operations.
 *
 * This will use the same multiplication order as matrices, such that the logical transform order is
 * reversed from the multiplication order.
 *
 * @remark This will not produce correct results if a contains non-uniform scale and and b contains
 *     a non-identity rotation, as this would result in a non-rigid transform.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the multiplication.
 * @param a The first transform.
 * @param b The second transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_mulSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) a,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) b);

/**
 * @brief Transforms a point with a rigid transform using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The transformed point.
 * @param transform The transform to apply to the point.
 * @param point The point to transform.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_transformSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) transform,
	const dsVector3xd* DS_ALIGN_PARAM(32) point);

/**
 * @brief Interpolates between two rigid transforms using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_lerpSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) a,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) b, double t);

/**
 * @brief Interpolates between two rigid transforms that are expected to be near each-other using
 * SIMD operations.
 *
 * This will use a faster, but less accurate, interpolation for the orientation under the assumption
 * they will be close enough for that to not be an issue. For best results, the orientations should
 * be consistent as with dsRigidTransform3d_makeOrientationConsistent().
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the interpolation.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsRigidTransform3d_nearLerpSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) a,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) b, double t);

/**
 * @brief Checks if two rigid transforms are exactly equal using SIMD operations.
 *
 * This will consider equivalent orientations (where one is negated from the other) to be equal.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param a The first rigid transform.
 * @param b The second rigid transform.
 * @return True if a == b.
 */
DS_MATH_EXPORT inline bool dsRigidTransform3d_equalSIMD4(
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b);

#endif // DS_HAS_SIMD

inline void dsRigidTransform3f_initialize(dsRigidTransform3f* result,
	const dsVector3xf* position, const dsQuaternion4f* orientation, const dsVector3xf* scale)
{
	DS_ASSERT(result);

	if (position)
		result->position = *position;
	else
		result->position.x = result->position.y = result->position.z = result->position.w = 0.0f;

	if (orientation)
		result->orientation = *orientation;
	else
		dsQuaternion4_identityRotation(result->orientation);

	if (scale)
		result->scale = *scale;
	else
		result->scale.x = result->scale.y = result->scale.z = result->scale.w = 1.0f;
}

inline void dsRigidTransform3d_initialize(dsRigidTransform3d* result,
	const dsVector3xd* position, const dsQuaternion4d* orientation, const dsVector3xd* scale)
{
	DS_ASSERT(result);

	if (position)
		result->position = *position;
	else
		result->position.x = result->position.y = result->position.z = result->position.w = 0.0;

	if (orientation)
		result->orientation = *orientation;
	else
		dsQuaternion4_identityRotation(result->orientation);

	if (scale)
		result->scale = *scale;
	else
		result->scale.x = result->scale.y = result->scale.z = result->scale.w = 1.0;
}

inline void dsRigidTransform3f_fromMatrix(dsRigidTransform3f* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	// NOTE: This call already chooses the best SIMD implementation available at compile time.
	dsMatrix44f_decomposeTransform(&result->position, &result->orientation, &result->scale, matrix);
}

inline void dsRigidTransform3d_fromMatrix(dsRigidTransform3d* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	// NOTE: This call already chooses the best SIMD implementation available at compile time.
	dsMatrix44d_decomposeTransform(&result->position, &result->orientation, &result->scale, matrix);
}

inline void dsRigidTransform3f_toMatrix(dsMatrix44f* result, const dsRigidTransform3f* transform)
{
	DS_ASSERT(transform);
	// NOTE: This call already chooses the best SIMD implementation available at compile time.
	dsMatrix44f_composeTransform(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3d_toMatrix(dsMatrix44d* result, const dsRigidTransform3d* transform)
{
	DS_ASSERT(transform);
	// NOTE: This call already chooses the best SIMD implementation available at compile time.
	dsMatrix44d_composeTransform(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3f_makeOrientationConsistent(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3f_makeOrientationConsistentFMA(transform, otherOrientation);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsRigidTransform3f_makeOrientationConsistentSIMD(transform, otherOrientation);
#else
	dsVector4f* orientation4f = (dsVector4f*)&transform->orientation;
	if (dsVector4f_dot(orientation4f, (const dsVector4f*)otherOrientation) < 0.0f)
		dsVector4f_neg(orientation4f, orientation4f);
#endif
}

inline void dsRigidTransform3d_makeOrientationConsistent(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);
#if DS_SIMD_PREFER_DOUBLE4
	dsRigidTransform3d_makeOrientationConsistentSIMD4(transform, otherOrientation);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3d_makeOrientationConsistentFMA2(transform, otherOrientation);
#else
	dsRigidTransform3d_makeOrientationConsistentSIMD2(transform, otherOrientation);
#endif
#else
	dsVector4d* orientation4d = (dsVector4d*)&transform->orientation;
	if (dsVector4d_dot(orientation4d, (const dsVector4d*)otherOrientation) < 0.0)
		dsVector4d_neg(orientation4d, orientation4d);
#endif
}

inline bool dsRigidTransform3f_isMulValid(const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);

	bool aUniformScale = a->scale.x == a->scale.y && a->scale.x == a->scale.z;
	bool bIdentityRotation =
		b->orientation.i == 0 && b->orientation.j == 0 && b->orientation.k == 0;
	return aUniformScale || bIdentityRotation;
}

inline bool dsRigidTransform3d_isMulValid(const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);

	bool aUniformScale = a->scale.x == a->scale.y && a->scale.x == a->scale.z;
	bool bIdentityRotation =
		b->orientation.i == 0 && b->orientation.j == 0 && b->orientation.k == 0;
	return aUniformScale || bIdentityRotation;
}

inline void dsRigidTransform3f_mul(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3f_mulFMA(result, a, b);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsRigidTransform3f_mulSIMD(result, a, b);
#else
	dsVector3xf_mul(&result->position, &a->scale, &b->position);
	dsQuaternion4f_rotate3x(&result->position, &a->orientation, &result->position);
	dsVector3xf_add(&result->position, &a->position, &result->position);
	dsQuaternion4f_mul(&result->orientation, &a->orientation, &b->orientation);
	dsVector3xf_mul(&result->scale, &a->scale, &b->scale);
#endif
}

inline void dsRigidTransform3d_mul(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_PREFER_DOUBLE4
	dsRigidTransform3d_mulSIMD4(result, a, b);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3d_mulFMA2(result, a, b);
#else
	dsRigidTransform3d_mulSIMD2(result, a, b);
#endif
#else
	dsVector3xd_mul(&result->position, &a->scale, &b->position);
	dsQuaternion4d_rotate3x(&result->position, &a->orientation, &result->position);
	dsVector3xd_add(&result->position, &a->position, &result->position);
	dsQuaternion4d_mul(&result->orientation, &a->orientation, &b->orientation);
	dsVector3xd_mul(&result->scale, &a->scale, &b->scale);
#endif
}

inline void dsRigidTransform3f_transform(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3f_transformFMA(result, transform, point);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsRigidTransform3f_transformSIMD(result, transform, point);
#else
	dsVector3xf_mul(result, &transform->scale, point);
	dsQuaternion4f_rotate3x(result, &transform->orientation, result);
	dsVector3xf_add(result, &transform->position, result);
#endif
}

inline void dsRigidTransform3d_transform(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);
#if DS_SIMD_PREFER_DOUBLE4
	dsRigidTransform3d_transformSIMD4(result, transform, point);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3d_transformFMA2(result, transform, point);
#else
	dsRigidTransform3d_transformSIMD2(result, transform, point);
#endif
#else
	dsVector3xd_mul(result, &transform->scale, point);
	dsQuaternion4d_rotate3x(result, &transform->orientation, result);
	dsVector3xd_add(result, &transform->position, result);
#endif
}

inline void dsRigidTransform3f_lerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3f_lerpFMA(result, a, b, t);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsRigidTransform3f_lerpSIMD(result, a, b, t);
#else
	dsVector3xf_lerp(&result->position, &a->position, &b->position, t);
	dsQuaternion4f_slerp(&result->orientation, &a->orientation, &b->orientation, t);
	dsVector3xf_lerp(&result->scale, &a->scale, &b->scale, t);
#endif
}

inline void dsRigidTransform3d_lerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_PREFER_DOUBLE4
	dsRigidTransform3d_lerpSIMD4(result, a, b, t);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3d_lerpFMA2(result, a, b, t);
#else
	dsRigidTransform3d_lerpSIMD2(result, a, b, t);
#endif
#else
	dsVector3xd_lerp(&result->position, &a->position, &b->position, t);
	dsVector3xd_lerp(&result->scale, &a->scale, &b->scale, t);
	dsQuaternion4d_slerp(&result->orientation, &a->orientation, &b->orientation, t);
#endif
}

inline void dsRigidTransform3f_nearLerp(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3f_nearLerpFMA(result, a, b, t);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsRigidTransform3f_nearLerpSIMD(result, a, b, t);
#else
	dsVector3xf_lerp(&result->position, &a->position, &b->position, t);
	dsVector3xf_lerp(&result->scale, &a->scale, &b->scale, t);
	dsVector4f_lerp((dsVector4f*)&result->orientation, (const dsVector4f*)&a->orientation,
		(const dsVector4f*)&b->orientation, t);
	dsQuaternion4f_normalize(&result->orientation, &result->orientation);
#endif
}

inline void dsRigidTransform3d_nearLerp(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_PREFER_DOUBLE4
	dsRigidTransform3d_nearLerpSIMD4(result, a, b, t);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsRigidTransform3d_nearLerpFMA2(result, a, b, t);
#else
	dsRigidTransform3d_nearLerpSIMD2(result, a, b, t);
#endif
#else
	dsVector3xd_lerp(&result->position, &a->position, &b->position, t);
	dsVector4d_lerp((dsVector4d*)&result->orientation, (const dsVector4d*)&a->orientation,
		(const dsVector4d*)&b->orientation, t);
	dsVector3xd_lerp(&result->scale, &a->scale, &b->scale, t);
	dsQuaternion4d_normalize(&result->orientation, &result->orientation);
#endif
}

inline bool dsRigidTransform3f_equal(const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	return dsRigidTransform3f_equalSIMD(a, b);
#else
	const dsVector4f* aOrientation = (const dsVector4f*)&a->orientation;
	const dsVector4f* bOrientation = (const dsVector4f*)&b->orientation;
	dsVector4f bOrientationNeg;
	dsVector4f_neg(&bOrientationNeg, bOrientation);
	return dsVector3xf_equal(&a->position, &b->position) &&
		dsVector3xf_equal(&a->scale, &b->scale) && (dsVector4f_equal(aOrientation, bOrientation) ||
		dsVector4f_equal(aOrientation, &bOrientationNeg));
#endif
}

inline bool dsRigidTransform3d_equal(const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_PREFER_DOUBLE4
	return dsRigidTransform3d_equalSIMD4(a, b);
#elif DS_SIMD_ALWAYS_DOUBLE2
	return dsRigidTransform3d_equalSIMD2(a, b);
#else
	const dsVector4d* aOrientation = (const dsVector4d*)&a->orientation;
	const dsVector4d* bOrientation = (const dsVector4d*)&b->orientation;
	dsVector4d bOrientationNeg;
	dsVector4d_neg(&bOrientationNeg, bOrientation);
	return dsVector3xd_equal(&a->position, &b->position) &&
		dsVector3xd_equal(&a->scale, &b->scale) && (dsVector4d_equal(aOrientation, bOrientation) ||
		dsVector4d_equal(aOrientation, &bOrientationNeg));
#endif
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)

inline void dsRigidTransform3f_fromMatrixSIMD(dsRigidTransform3f* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	dsMatrix44f_decomposeTransformSIMD(
		&result->position, &result->orientation, &result->scale, matrix);
}

inline void dsRigidTransform3f_toMatrixSIMD(
	dsMatrix44f* result, const dsRigidTransform3f* transform)
{
	DS_ASSERT(transform);
	dsMatrix44f_composeTransformSIMD(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3f_makeOrientationConsistentSIMD(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);

	dsSIMD4f dot = dsDot4SIMD4f(transform->orientation.simd, otherOrientation->simd);
	dsSIMD4fb dotNeg = dsSIMD4f_cmplt(dot, dsSIMD4f_set1(0.0f));
	transform->orientation.simd = dsSIMD4f_select(
		dotNeg, dsSIMD4f_neg(transform->orientation.simd), transform->orientation.simd);
}

inline void dsRigidTransform3f_mulSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->position.simd = dsSIMD4f_mul(a->scale.simd, b->position.simd);
	dsQuaternion4f_rotateSIMD(&result->position, &a->orientation, &result->position);
	result->position.simd = dsSIMD4f_add(a->position.simd, result->position.simd);
	dsQuaternion4f_mulSIMD(&result->orientation, &a->orientation, &b->orientation);
	result->scale.simd = dsSIMD4f_mul(a->scale.simd, b->scale.simd);
}

inline void dsRigidTransform3f_transformSIMD(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);

	result->simd = dsSIMD4f_mul(transform->scale.simd, point->simd);
	dsQuaternion4f_rotateSIMD(result, &transform->orientation, result);
	result->simd = dsSIMD4f_add(transform->position.simd, result->simd);
}

inline void dsRigidTransform3f_lerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsSIMD4f invT4 = dsSIMD4f_set1(1.0f - t);
	result->position.simd = dsSIMD4f_add(
		dsSIMD4f_mul(invT4, a->position.simd), dsSIMD4f_mul(t4, b->position.simd));
	result->scale.simd = dsSIMD4f_add(
		dsSIMD4f_mul(invT4, a->scale.simd), dsSIMD4f_mul(t4, b->scale.simd));
	dsQuaternion4f_slerp(&result->orientation, &a->orientation, &b->orientation, t);
}

inline void dsRigidTransform3f_nearLerpSIMD(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsSIMD4f invT4 = dsSIMD4f_set1(1.0f - t);
	result->position.simd = dsSIMD4f_add(
		dsSIMD4f_mul(invT4, a->position.simd), dsSIMD4f_mul(t4, b->position.simd));
	result->orientation.simd = dsSIMD4f_add(
		dsSIMD4f_mul(invT4, a->orientation.simd), dsSIMD4f_mul(t4, b->orientation.simd));
	result->scale.simd = dsSIMD4f_add(
		dsSIMD4f_mul(invT4, a->scale.simd), dsSIMD4f_mul(t4, b->scale.simd));

	dsSIMD4f len2 = dsDot4SIMD4f(result->orientation.simd, result->orientation.simd);
#if DS_SIMD_EMULATED_DIV_SQRT
	dsSIMD4f invLen = dsSIMD4f_set1(1/dsSqrtf(dsSIMD4f_get(len2, 0)));
#else
	dsSIMD4f invLen = dsSIMD4f_rsqrt(len2);
#endif
	result->orientation.simd = dsSIMD4f_mul(result->orientation.simd, invLen);
}

inline bool dsRigidTransform3f_equalSIMD(const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);

#if DS_SIMD_ALWAYS_INT
	dsSIMD4fb lastTrue = dsSIMD4fb_set4(0, 0, 0, 0xFFFFFFFF);
#else
	static const DS_ALIGN(16) uint32_t lastTrueVal[4] = {0, 0, 0, 0xFFFFFFFF};
	dsSIMD4fb lastTrue = dsSIMD4fb_load(lastTrueVal);
#endif

	dsSIMD4fb positionScaleEqual = dsSIMD4fb_or(dsSIMD4fb_and(
		dsSIMD4f_cmpeq(a->position.simd, b->position.simd),
		dsSIMD4f_cmpeq(a->scale.simd, b->scale.simd)), lastTrue);
	dsSIMD4fb orientationEqual = dsSIMD4fb_or(
		dsSIMD4f_cmpeq(a->orientation.simd, b->orientation.simd),
		dsSIMD4f_cmpeq(a->orientation.simd, dsSIMD4f_neg(b->orientation.simd)));
	dsSIMD4fb equal = dsSIMD4fb_and(positionScaleEqual, orientationEqual);
#if DS_SIMD_ALWAYS_INT
	return dsSIMD4fb_all(equal) != 0;
#else
	dsVector4i equalVal;
	equalVal.simd = equal;
	return equalVal.x && equalVal.y && equalVal.z && equalVal.w;
#endif
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

inline void dsRigidTransform3f_toMatrixFMA(
	dsMatrix44f* result, const dsRigidTransform3f* transform)
{
	DS_ASSERT(transform);
	dsMatrix44f_composeTransformFMA(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3f_makeOrientationConsistentFMA(
	dsRigidTransform3f* transform, const dsQuaternion4f* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);

	dsSIMD4f dot = dsDot4FMA4f(transform->orientation.simd, otherOrientation->simd);
	dsSIMD4fb dotNeg = dsSIMD4f_cmplt(dot, dsSIMD4f_set1(0.0f));
	transform->orientation.simd = dsSIMD4f_select(
		dotNeg, dsSIMD4f_neg(transform->orientation.simd), transform->orientation.simd);
}

inline void dsRigidTransform3f_mulFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->position.simd = dsSIMD4f_mul(a->scale.simd, b->position.simd);
	dsQuaternion4f_rotateFMA(&result->position, &a->orientation, &result->position);
	result->position.simd = dsSIMD4f_add(a->position.simd, result->position.simd);
	dsQuaternion4f_mulFMA(&result->orientation, &a->orientation, &b->orientation);
	result->scale.simd = dsSIMD4f_mul(a->scale.simd, b->scale.simd);
}

inline void dsRigidTransform3f_transformFMA(
	dsVector3xf* result, const dsRigidTransform3f* transform, const dsVector3xf* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);

	result->simd = dsSIMD4f_mul(transform->scale.simd, point->simd);
	dsQuaternion4f_rotateFMA(result, &transform->orientation, result);
	result->simd = dsSIMD4f_add(transform->position.simd, result->simd);
}

inline void dsRigidTransform3f_lerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsSIMD4f invT4 = dsSIMD4f_set1(1.0f - t);
	result->position.simd = dsSIMD4f_fmadd(
		invT4, a->position.simd, dsSIMD4f_mul(t4, b->position.simd));
	result->scale.simd = dsSIMD4f_fmadd(invT4, a->scale.simd, dsSIMD4f_mul(t4, b->scale.simd));
	dsQuaternion4f_slerp(&result->orientation, &a->orientation, &b->orientation, t);
}

inline void dsRigidTransform3f_nearLerpFMA(
	dsRigidTransform3f* result, const dsRigidTransform3f* a, const dsRigidTransform3f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	dsSIMD4f invT4 = dsSIMD4f_set1(1.0f - t);
	result->position.simd = dsSIMD4f_fmadd(
		invT4, a->position.simd, dsSIMD4f_mul(t4, b->position.simd));
	result->orientation.simd = dsSIMD4f_fmadd(
		invT4, a->orientation.simd, dsSIMD4f_mul(t4, b->orientation.simd));
	result->scale.simd = dsSIMD4f_fmadd(invT4, a->scale.simd, dsSIMD4f_mul(t4, b->scale.simd));

	dsSIMD4f len2 = dsDot4FMA4f(result->orientation.simd, result->orientation.simd);
	dsSIMD4f invLen = dsSIMD4f_rsqrt(len2);
	result->orientation.simd = dsSIMD4f_mul(result->orientation.simd, invLen);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2)

inline void dsRigidTransform3d_fromMatrixSIMD2(
	dsRigidTransform3d* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	dsMatrix44d_decomposeTransformSIMD2(
		&result->position, &result->orientation, &result->scale, matrix);
}

inline void dsRigidTransform3d_toMatrixSIMD2(
	dsMatrix44d* result, const dsRigidTransform3d* transform)
{
	DS_ASSERT(transform);
	dsMatrix44d_composeTransformSIMD2(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3d_makeOrientationConsistentSIMD2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);

	dsSIMD2d dot = dsDot4SIMD2d(transform->orientation.simd2[0], transform->orientation.simd2[1],
		otherOrientation->simd2[0], otherOrientation->simd2[1]);
	dsSIMD2db dotNeg = dsSIMD2d_cmplt(dot, dsSIMD2d_set1(0.0));
	transform->orientation.simd2[0] = dsSIMD2d_select(
		dotNeg, dsSIMD2d_neg(transform->orientation.simd2[0]), transform->orientation.simd2[0]);
	transform->orientation.simd2[1] = dsSIMD2d_select(
		dotNeg, dsSIMD2d_neg(transform->orientation.simd2[1]), transform->orientation.simd2[1]);
}

inline void dsRigidTransform3d_mulSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->position.simd2[0] = dsSIMD2d_mul(a->scale.simd2[0], b->position.simd2[0]);
	result->position.simd2[1] = dsSIMD2d_mul(a->scale.simd2[1], b->position.simd2[1]);
	dsQuaternion4d_rotateSIMD2(&result->position, &a->orientation, &result->position);
	result->position.simd2[0] = dsSIMD2d_add(a->position.simd2[0], result->position.simd2[0]);
	result->position.simd2[1] = dsSIMD2d_add(a->position.simd2[1], result->position.simd2[1]);
	dsQuaternion4d_mulSIMD2(&result->orientation, &a->orientation, &b->orientation);
	result->scale.simd2[0] = dsSIMD2d_mul(a->scale.simd2[0], b->scale.simd2[0]);
	result->scale.simd2[1] = dsSIMD2d_mul(a->scale.simd2[1], b->scale.simd2[1]);
}

inline void dsRigidTransform3d_transformSIMD2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);

	result->simd2[0] = dsSIMD2d_mul(transform->scale.simd2[0], point->simd2[0]);
	result->simd2[1] = dsSIMD2d_mul(transform->scale.simd2[1], point->simd2[1]);
	dsQuaternion4d_rotateSIMD2(result, &transform->orientation, result);
	result->simd2[0] = dsSIMD2d_add(transform->position.simd2[0], result->simd2[0]);
	result->simd2[1] = dsSIMD2d_add(transform->position.simd2[1], result->simd2[1]);
}

inline void dsRigidTransform3d_lerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsSIMD2d invT2 = dsSIMD2d_set1(1.0 - t);
	result->position.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->position.simd2[0]),  dsSIMD2d_mul(t2, b->position.simd2[0]));
	result->position.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->position.simd2[1]),  dsSIMD2d_mul(t2, b->position.simd2[1]));
	result->scale.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->scale.simd2[0]),  dsSIMD2d_mul(t2, b->scale.simd2[0]));
	result->scale.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->scale.simd2[1]),  dsSIMD2d_mul(t2, b->scale.simd2[1]));
	dsQuaternion4d_slerp(&result->orientation, &a->orientation, &b->orientation, t);
}

inline void dsRigidTransform3d_nearLerpSIMD2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsSIMD2d invT2 = dsSIMD2d_set1(1.0 - t);
	result->position.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->position.simd2[0]),  dsSIMD2d_mul(t2, b->position.simd2[0]));
	result->position.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->position.simd2[1]),  dsSIMD2d_mul(t2, b->position.simd2[1]));
	result->orientation.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->orientation.simd2[0]),  dsSIMD2d_mul(t2, b->orientation.simd2[0]));
	result->orientation.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->orientation.simd2[1]),  dsSIMD2d_mul(t2, b->orientation.simd2[1]));
	result->scale.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->scale.simd2[0]),  dsSIMD2d_mul(t2, b->scale.simd2[0]));
	result->scale.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(invT2, a->scale.simd2[1]),  dsSIMD2d_mul(t2, b->scale.simd2[1]));

	dsSIMD2d len2 = dsDot4SIMD2d(result->orientation.simd2[0], result->orientation.simd2[1],
		result->orientation.simd2[0], result->orientation.simd2[1]);
	dsSIMD2d invLen = dsSIMD2d_rsqrt(len2);
	result->orientation.simd2[0] = dsSIMD2d_mul(result->orientation.simd2[0], invLen);
	result->orientation.simd2[1] = dsSIMD2d_mul(result->orientation.simd2[1], invLen);
}

inline bool dsRigidTransform3d_equalSIMD2(const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD2db lastTrue = dsSIMD2db_set2(0, 0xFFFFFFFFFFFFFFFFULL);

	dsSIMD2db positionScaleEqual0 = dsSIMD2db_and(
		dsSIMD2d_cmpeq(a->position.simd2[0], b->position.simd2[0]),
		dsSIMD2d_cmpeq(a->scale.simd2[0], b->scale.simd2[0]));
	dsSIMD2db positionScaleEqual1 = dsSIMD2db_or(dsSIMD2db_and(
		dsSIMD2d_cmpeq(a->position.simd2[1], b->position.simd2[1]),
		dsSIMD2d_cmpeq(a->scale.simd2[1], b->scale.simd2[1])), lastTrue);
	dsSIMD2db orientationEqual0 = dsSIMD2db_or(
		dsSIMD2d_cmpeq(a->orientation.simd2[0], b->orientation.simd2[0]),
		dsSIMD2d_cmpeq(a->orientation.simd2[0], dsSIMD2d_neg(b->orientation.simd2[0])));
	dsSIMD2db orientationEqual1 = dsSIMD2db_or(
		dsSIMD2d_cmpeq(a->orientation.simd2[1], b->orientation.simd2[1]),
		dsSIMD2d_cmpeq(a->orientation.simd2[1], dsSIMD2d_neg(b->orientation.simd2[1])));
	dsSIMD2db equal = dsSIMD2db_and(
		dsSIMD2db_and(positionScaleEqual0, positionScaleEqual1),
		dsSIMD2db_and(orientationEqual0, orientationEqual1));
	return dsSIMD2db_all(equal) != 0;
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

inline void dsRigidTransform3d_toMatrixFMA2(
	dsMatrix44d* result, const dsRigidTransform3d* transform)
{
	DS_ASSERT(transform);
	dsMatrix44d_composeTransformFMA2(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3d_makeOrientationConsistentFMA2(
	dsRigidTransform3d* transform, const dsQuaternion4d* otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);

	dsSIMD2d dot = dsDot4FMA2d(transform->orientation.simd2[0], transform->orientation.simd2[1],
		otherOrientation->simd2[0], otherOrientation->simd2[1]);
	dsSIMD2db dotNeg = dsSIMD2d_cmplt(dot, dsSIMD2d_set1(0.0));
	transform->orientation.simd2[0] = dsSIMD2d_select(
		dotNeg, dsSIMD2d_neg(transform->orientation.simd2[0]), transform->orientation.simd2[0]);
	transform->orientation.simd2[1] = dsSIMD2d_select(
		dotNeg, dsSIMD2d_neg(transform->orientation.simd2[1]), transform->orientation.simd2[1]);
}

inline void dsRigidTransform3d_mulFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->position.simd2[0] = dsSIMD2d_mul(a->scale.simd2[0], b->position.simd2[0]);
	result->position.simd2[1] = dsSIMD2d_mul(a->scale.simd2[1], b->position.simd2[1]);
	dsQuaternion4d_rotateFMA2(&result->position, &a->orientation, &result->position);
	result->position.simd2[0] = dsSIMD2d_add(a->position.simd2[0], result->position.simd2[0]);
	result->position.simd2[1] = dsSIMD2d_add(a->position.simd2[1], result->position.simd2[1]);
	dsQuaternion4d_mulFMA2(&result->orientation, &a->orientation, &b->orientation);
	result->scale.simd2[0] = dsSIMD2d_mul(a->scale.simd2[0], b->scale.simd2[0]);
	result->scale.simd2[1] = dsSIMD2d_mul(a->scale.simd2[1], b->scale.simd2[1]);
}

inline void dsRigidTransform3d_transformFMA2(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);

	result->simd2[0] = dsSIMD2d_mul(transform->scale.simd2[0], point->simd2[0]);
	result->simd2[1] = dsSIMD2d_mul(transform->scale.simd2[1], point->simd2[1]);
	dsQuaternion4d_rotateFMA2(result, &transform->orientation, result);
	result->simd2[0] = dsSIMD2d_add(transform->position.simd2[0], result->simd2[0]);
	result->simd2[1] = dsSIMD2d_add(transform->position.simd2[1], result->simd2[1]);
}

inline void dsRigidTransform3d_lerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsSIMD2d invT2 = dsSIMD2d_set1(1.0 - t);
	result->position.simd2[0] = dsSIMD2d_fmadd(
		invT2, a->position.simd2[0], dsSIMD2d_mul(t2, b->position.simd2[0]));
	result->position.simd2[1] = dsSIMD2d_fmadd(
		invT2, a->position.simd2[1], dsSIMD2d_mul(t2, b->position.simd2[1]));
	result->scale.simd2[0] = dsSIMD2d_fmadd(
		invT2, a->scale.simd2[0], dsSIMD2d_mul(t2, b->scale.simd2[0]));
	result->scale.simd2[1] = dsSIMD2d_fmadd(
		invT2, a->scale.simd2[1], dsSIMD2d_mul(t2, b->scale.simd2[1]));
	dsQuaternion4d_slerp(&result->orientation, &a->orientation, &b->orientation, t);
}

inline void dsRigidTransform3d_nearLerpFMA2(
	dsRigidTransform3d* result, const dsRigidTransform3d* a, const dsRigidTransform3d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	dsSIMD2d invT2 = dsSIMD2d_set1(1.0 - t);
	result->position.simd2[0] = dsSIMD2d_fmadd(
		invT2, a->position.simd2[0], dsSIMD2d_mul(t2, b->position.simd2[0]));
	result->position.simd2[1] = dsSIMD2d_fmadd(
		invT2, a->position.simd2[1], dsSIMD2d_mul(t2, b->position.simd2[1]));
	result->orientation.simd2[0] = dsSIMD2d_fmadd(
		invT2, a->orientation.simd2[0], dsSIMD2d_mul(t2, b->orientation.simd2[0]));
	result->orientation.simd2[1] = dsSIMD2d_fmadd(
		invT2, a->orientation.simd2[1], dsSIMD2d_mul(t2, b->orientation.simd2[1]));
	result->scale.simd2[0] = dsSIMD2d_fmadd(
		invT2, a->scale.simd2[0], dsSIMD2d_mul(t2, b->scale.simd2[0]));
	result->scale.simd2[1] = dsSIMD2d_fmadd(
		invT2, a->scale.simd2[1], dsSIMD2d_mul(t2, b->scale.simd2[1]));

	dsSIMD2d len2 = dsDot4FMA2d(result->orientation.simd2[0], result->orientation.simd2[1],
		result->orientation.simd2[0], result->orientation.simd2[1]);
	dsSIMD2d invLen = dsSIMD2d_rsqrt(len2);
	result->orientation.simd2[0] = dsSIMD2d_mul(result->orientation.simd2[0], invLen);
	result->orientation.simd2[1] = dsSIMD2d_mul(result->orientation.simd2[1], invLen);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA)

inline void dsRigidTransform3d_fromMatrixSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix)
{
	DS_ASSERT(result);
	dsMatrix44d_decomposeTransformSIMD4(
		&result->position, &result->orientation, &result->scale, matrix);
}

inline void dsRigidTransform3d_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsRigidTransform3d* DS_ALIGN_PARAM(32) transform)
{
	DS_ASSERT(transform);
	dsMatrix44d_composeTransformSIMD4(
		result, &transform->position, &transform->orientation, &transform->scale);
}

inline void dsRigidTransform3d_makeOrientationConsistentSIMD4(
	dsRigidTransform3d* DS_ALIGN_PARAM(32) transform,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) otherOrientation)
{
	DS_ASSERT(transform);
	DS_ASSERT(otherOrientation);

	dsSIMD4d simdOrientation = dsSIMD4d_load(&transform->orientation);
	dsSIMD4d simdOtherOrientation = dsSIMD4d_load(otherOrientation);
	dsSIMD4d dot = dsDot4SIMD4d(simdOrientation, simdOtherOrientation);
	dsSIMD4db dotNeg = dsSIMD4d_cmplt(dot, dsSIMD4d_set1(0.0));
	dsSIMD4d_store(&transform->orientation, dsSIMD4d_select(
		dotNeg, dsSIMD4d_neg(simdOrientation), simdOrientation));
}

inline void dsRigidTransform3d_mulSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d_store(
		&result->position, dsSIMD4d_mul(dsSIMD4d_load(&a->scale), dsSIMD4d_load(&b->position)));
	dsQuaternion4d_rotateSIMD4(&result->position, &a->orientation, &result->position);
	dsSIMD4d_store(&result->position,
		dsSIMD4d_add(dsSIMD4d_load(&a->position), dsSIMD4d_load(&result->position)));
	dsQuaternion4d_mulSIMD4(&result->orientation, &a->orientation, &b->orientation);
	dsSIMD4d_store(
		&result->scale, dsSIMD4d_mul(dsSIMD4d_load(&a->scale), dsSIMD4d_load(&b->scale)));
}

inline void dsRigidTransform3d_transformSIMD4(
	dsVector3xd* result, const dsRigidTransform3d* transform, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(transform);
	DS_ASSERT(point);

	dsSIMD4d_store(result, dsSIMD4d_mul(dsSIMD4d_load(&transform->scale), dsSIMD4d_load(point)));
	dsQuaternion4d_rotateSIMD4(result, &transform->orientation, result);
	dsSIMD4d_store(
		result, dsSIMD4d_add(dsSIMD4d_load(&transform->position), dsSIMD4d_load(result)));
}

inline void dsRigidTransform3d_lerpSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b,
	double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4d t4 = dsSIMD4d_set1(t);
	dsSIMD4d invT4 = dsSIMD4d_set1(1.0 - t);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(&result->position, dsSIMD4d_fmadd(
		invT4, dsSIMD4d_load(&a->position), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->position))));
	dsSIMD4d_store(&result->scale, dsSIMD4d_fmadd(
		invT4, dsSIMD4d_load(&a->scale), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->scale))));
#else
	dsSIMD4d_store(&result->position, dsSIMD4d_add(dsSIMD4d_mul(invT4, dsSIMD4d_load(&a->position)),
		dsSIMD4d_mul(t4, dsSIMD4d_load(&b->position))));
	dsSIMD4d_store(&result->scale, dsSIMD4d_add(
		dsSIMD4d_mul(invT4, dsSIMD4d_load(&a->scale)), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->scale))));
#endif
	dsQuaternion4d_slerp(&result->orientation, &a->orientation, &b->orientation, t);
}

inline void dsRigidTransform3d_nearLerpSIMD4(dsRigidTransform3d* DS_ALIGN_PARAM(32) result,
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b,
	double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4d t4 = dsSIMD4d_set1(t);
	dsSIMD4d invT4 = dsSIMD4d_set1(1.0 - t);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(&result->position, dsSIMD4d_fmadd(
		invT4, dsSIMD4d_load(&a->position), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->position))));
	dsSIMD4d_store(&result->orientation, dsSIMD4d_fmadd(
		invT4, dsSIMD4d_load(&a->orientation), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->orientation))));
	dsSIMD4d_store(&result->scale, dsSIMD4d_fmadd(
		invT4, dsSIMD4d_load(&a->scale), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->scale))));
#else
	dsSIMD4d_store(&result->position, dsSIMD4d_add(dsSIMD4d_mul(invT4, dsSIMD4d_load(&a->position)),
		dsSIMD4d_mul(t4, dsSIMD4d_load(&b->position))));
	dsSIMD4d_store(&result->orientation, dsSIMD4d_add(
		dsSIMD4d_mul(invT4, dsSIMD4d_load(&a->orientation)),
		dsSIMD4d_mul(t4, dsSIMD4d_load(&b->orientation))));
	dsSIMD4d_store(&result->scale, dsSIMD4d_add(
		dsSIMD4d_mul(invT4, dsSIMD4d_load(&a->scale)), dsSIMD4d_mul(t4, dsSIMD4d_load(&b->scale))));
#endif

	dsSIMD4d orientation = dsSIMD4d_load(&result->orientation);
	dsSIMD4d len2 = dsDot4SIMD4d(orientation, orientation);
	dsSIMD4d invLen = dsSIMD4d_rsqrt(len2);
	dsSIMD4d_store(&result->orientation, dsSIMD4d_mul(orientation, invLen));
}

inline bool dsRigidTransform3d_equalSIMD4(
	const dsRigidTransform3d* DS_ALIGN_PARAM(32) a, const dsRigidTransform3d* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsSIMD4db lastTrue = dsSIMD4db_set4(0, 0, 0, 0xFFFFFFFFFFFFFFFFULL);

	dsSIMD4db positionScaleEqual = dsSIMD4db_or(dsSIMD4db_and(
		dsSIMD4d_cmpeq(dsSIMD4d_load(&a->position), dsSIMD4d_load(&b->position)),
		dsSIMD4d_cmpeq(dsSIMD4d_load(&a->scale), dsSIMD4d_load(&b->scale))), lastTrue);
	dsSIMD4d bOrientation = dsSIMD4d_load(&b->orientation);
	dsSIMD4db orientationEqual = dsSIMD4db_or(
		dsSIMD4d_cmpeq(dsSIMD4d_load(&a->orientation), bOrientation),
		dsSIMD4d_cmpeq(dsSIMD4d_load(&a->orientation), dsSIMD4d_neg(bOrientation)));
	dsSIMD4db equal = dsSIMD4db_and(positionScaleEqual, orientationEqual);
	return dsSIMD4db_all(equal) != 0;
}

DS_SIMD_END()
#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
