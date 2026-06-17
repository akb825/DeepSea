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
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Geometry/AlignedBox3x.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Matrix33x.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3x.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating dsOrientedBox3x* structures.
 *
 * The functions have different versions for the supported dsOrientedBox3 types, and will utilize
 * SIMD operations where supported at compile-time.
 *
 * @see dsOrientedBox3xf dsOrientedBox3xd
 */

#if DS_HAS_SIMD

/**
 * @brief Converts the oriented box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_toMatrixSIMD(
	dsMatrix44f* result, const dsOrientedBox3xf* box);

/**
 * @brief Converts the oriented box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_toMatrixTransposeSIMD(
	dsMatrix44f* result, const dsOrientedBox3xf* box);

/**
 * @brief Creates an oriented box from a matrix representation using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_fromMatrixSIMD(
	dsOrientedBox3xf* result, const dsMatrix44f* matrix);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Creates an oriented box from a matrix representation using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_fromMatrixFMA(
	dsOrientedBox3xf* result, const dsMatrix44f* matrix);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Converts the oriented box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrixSIMD2(
	dsMatrix44d* result, const dsOrientedBox3xd* box);

/**
 * @brief Converts the oriented box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrixTransposeSIMD2(
	dsMatrix44d* result, const dsOrientedBox3xd* box);

/**
 * @brief Creates an oriented box from a matrix representation using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_fromMatrixSIMD2(
	dsOrientedBox3xd* result, const dsMatrix44d* matrix);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Creates an oriented box from a matrix representation using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_fromMatrixFMA2(
	dsOrientedBox3xd* result, const dsMatrix44d* matrix);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Converts the oriented box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box);

/**
 * @brief Converts the oriented box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box);

/**
 * @brief Creates an oriented box from a matrix representation using SIMD operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_fromMatrixSIMD4(
	dsOrientedBox3xd* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix);

#endif // DS_HAS_SIMD

/** @copydoc dsOrientedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox3xf_isValid(const dsOrientedBox3xf* box)
{
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4f_cmpge(box->halfExtents.simd, dsSIMD4f_set1(0.0f));
	return result.x && result.y && result.z;
#else
	return dsOrientedBox3_isValid(*box);
#endif
}

/** @copydoc dsOrientedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox3xd_isValid(const dsOrientedBox3xd* box)
{
	DS_ASSERT(box);
#if DS_SIMD_PREFER_DOUBLE4
	dsVector4l result;
	result.simd = dsSIMD4d_cmpge(box->halfExtents.simd, dsSIMD4d_set1(0.0));
	return result.x && result.y && result.z;
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsVector4l result;
	result.simd2[0] = dsSIMD2d_cmpge(box->halfExtents.simd2[0], zero);
	result.simd2[1] = dsSIMD2d_cmpge(box->halfExtents.simd2[1], zero);
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsOrientedBox3_isValid(*box);
#endif
}

/** @copydoc dsOrientedBox3_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_fromAlignedBox(
	dsOrientedBox3xf* result, const dsAlignedBox3xf* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);

	dsMatrix33xf_identity(&result->orientation);
	dsAlignedBox3xf_center(&result->center, alignedBox);
	dsAlignedBox3xf_extents(&result->halfExtents, alignedBox);
	dsVector3xf_scale(&result->halfExtents, &result->halfExtents, 0.5f);
}

/** @copydoc dsOrientedBox3_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_fromAlignedBox(
	dsOrientedBox3xd* result, const dsAlignedBox3xd* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);

	dsMatrix33xd_identity(&result->orientation);
	dsAlignedBox3xd_center(&result->center, alignedBox);
	dsAlignedBox3xd_extents(&result->halfExtents, alignedBox);
	dsVector3xd_scale(&result->halfExtents, &result->halfExtents, 0.5);
}

/** @copydoc dsOrientedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_toMatrix(
	dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsOrientedBox3xf_toMatrixSIMD(result, box);
#else
	dsOrientedBox3_toMatrix(*result, *box);
#endif
}

/** @copydoc dsOrientedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrix(
	dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_PREFER_DOUBLE4
	dsOrientedBox3xd_toMatrixSIMD4(result, box);
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsOrientedBox3xd_toMatrixSIMD2(result, box);
#else
	dsOrientedBox3_toMatrix(*result, *box);
#endif
}

/** @copydoc dsOrientedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_toMatrixTranspose(
	dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsOrientedBox3xf_toMatrixTransposeSIMD(result, box);
#else
	dsOrientedBox3_toMatrixTranspose(*result, *box);
#endif
}

/** @copydoc dsOrientedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_toMatrixTranspose(
	dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_PREFER_DOUBLE4
	dsOrientedBox3xd_toMatrixTransposeSIMD4(result, box);
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsOrientedBox3xd_toMatrixTransposeSIMD2(result, box);
#else
	dsOrientedBox3_toMatrixTranspose(*result, *box);
#endif
}

/** @copydoc dsOrientedBox3_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_makeInvalid(dsOrientedBox3xf* result)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->halfExtents.simd = dsSIMD4f_set1(-1.0f);
#else
	dsOrientedBox3_makeInvalid(*result);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->halfExtents.w = 0;
#endif
#endif
}

/** @copydoc dsOrientedBox3_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_makeInvalid(dsOrientedBox3xd* result)
{
	DS_ASSERT(result);
#if DS_SIMD_PREFER_DOUBLE4
	result->halfExtents.simd = dsSIMD4d_set1(-1.0);
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d negOne = dsSIMD2d_set1(-1.0);
	result->halfExtents.simd2[0] = negOne;
	result->halfExtents.simd2[1] = negOne;
#else
	dsOrientedBox3_makeInvalid(*result);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->halfExtents.w = 0;
#endif
#endif
}

/** @copydoc dsOrientedBox3f_fromMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xf_fromMatrix(
	dsOrientedBox3xf* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);
#if DS_SIMD_ALWAYS_FMA
	dsOrientedBox3xf_fromMatrixFMA(result, matrix);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsOrientedBox3xf_fromMatrixSIMD(result, matrix);
#else
	result->halfExtents.x = dsVector3xf_len(matrix->columns);
	result->halfExtents.y = dsVector3xf_len(matrix->columns + 1);
	result->halfExtents.z = dsVector3xf_len(matrix->columns + 2);

	float invLen = 1/result->halfExtents.x;
	dsVector3_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector3_scale(result->orientation.columns[1], matrix->columns[1], invLen);
	invLen = 1/result->halfExtents.z;
	dsVector3_scale(result->orientation.columns[2], matrix->columns[2], invLen);

	result->center = matrix->columns[3];

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->halfExtents.w = 0;
	result->center.w = 0;
	result->orientation.columns[0].w = 0;
	result->orientation.columns[1].w = 0;
	result->orientation.columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsOrientedBox3d_fromMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3xd_fromMatrix(
	dsOrientedBox3xd* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);
#if DS_SIMD_PREFER_DOUBLE4
	dsOrientedBox3xd_fromMatrixSIMD4(result, matrix);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsOrientedBox3xd_fromMatrixFMA2(result, matrix);
#else
	dsOrientedBox3xd_fromMatrixSIMD2(result, matrix);
#endif
#else
	result->halfExtents.x = dsVector3xd_len(matrix->columns);
	result->halfExtents.y = dsVector3xd_len(matrix->columns + 1);
	result->halfExtents.z = dsVector3xd_len(matrix->columns + 2);

	double invLen = 1/result->halfExtents.x;
	dsVector3_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector3_scale(result->orientation.columns[1], matrix->columns[1], invLen);
	invLen = 1/result->halfExtents.z;
	dsVector3_scale(result->orientation.columns[2], matrix->columns[2], invLen);

	result->center = matrix->columns[3];

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->halfExtents.w = 0;
	result->center.w = 0;
	result->orientation.columns[0].w = 0;
	result->orientation.columns[1].w = 0;
	result->orientation.columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsOrientedBox3f_transform() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_transform(
	dsOrientedBox3xf* box, const dsMatrix44f* transform);

/** @copydoc dsOrientedBox3f_transform() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_transform(
	dsOrientedBox3xd* box, const dsMatrix44d* transform);

/** @copydoc dsOrientedBox3f_addPoint() */
DS_GEOMETRY_EXPORT void dsOrientedBox3xf_addPoint(dsOrientedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsOrientedBox3f_addPoint() */
DS_GEOMETRY_EXPORT void dsOrientedBox3xd_addPoint(dsOrientedBox3xd* box, const dsVector3xd* point);

/** @copydoc dsOrientedBox3f_addBox() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_addBox(
	dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox);

/** @copydoc dsOrientedBox3f_addBox() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_addBox(
	dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox);

/** @copydoc dsOrientedBox3f_corners() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_corners(
	dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xf* box);

/** @copydoc dsOrientedBox3f_corners() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_corners(
	dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xd* box);

/** @copydoc dsOrientedBox3f_intersects() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_intersects(
	const dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox);

/** @copydoc dsOrientedBox3f_intersects() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_intersects(
	const dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox);

/** @copydoc dsOrientedBox3f_containsPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_containsPoint(
	const dsOrientedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsOrientedBox3f_containsPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_containsPoint(
	const dsOrientedBox3xd* box, const dsVector3xd* point);

/** @copydoc dsOrientedBox2f_closestPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xf_closestPoint(
	dsVector3xf* result, const dsOrientedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsOrientedBox2f_closestPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3xd_closestPoint(
	dsVector3xd* result, const dsOrientedBox3xd* box, const dsVector3xd* point);

/** @copydoc dsOrientedBox3f_dist2() */
DS_GEOMETRY_EXPORT float dsOrientedBox3xf_dist2(
	const dsOrientedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsOrientedBox3f_dist2() */
DS_GEOMETRY_EXPORT double dsOrientedBox3xd_dist2(
	const dsOrientedBox3xd* box, const dsVector3xd* point);

/** @copydoc dsOrientedBox3f_dist() */
DS_GEOMETRY_EXPORT float dsOrientedBox3xf_dist(
	const dsOrientedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsOrientedBox3f_dist() */
DS_GEOMETRY_EXPORT double dsOrientedBox3xd_dist(
	const dsOrientedBox3xd* box, const dsVector3xd* point);

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4)

inline void dsOrientedBox3xf_toMatrixSIMD(dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4f x = dsSIMD4f_set1FromVec(box->halfExtents.simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(box->halfExtents.simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(box->halfExtents.simd, 2);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);
#if DS_SIMD_ALWAYS_INT
	dsSIMD4fb mask = dsSIMD4fb_set4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0);
#else
	DS_ALIGN(16) uint32_t maskData[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0};
	dsSIMD4fb mask = dsSIMD4fb_load(maskData);
#endif

	result->columns[0].simd = dsSIMD4f_mul(box->orientation.columns[0].simd, x);
	result->columns[0].simd = dsSIMD4fb_toFloatBitfield(
		dsSIMD4fb_and(dsSIMD4fb_fromFloatBitfield(result->columns[0].simd), mask));
	result->columns[1].simd = dsSIMD4f_mul(box->orientation.columns[1].simd, y);
	result->columns[1].simd = dsSIMD4fb_toFloatBitfield(
		dsSIMD4fb_and(dsSIMD4fb_fromFloatBitfield(result->columns[1].simd), mask));
	result->columns[2].simd = dsSIMD4f_mul(box->orientation.columns[2].simd, z);
	result->columns[2].simd = dsSIMD4fb_toFloatBitfield(
		dsSIMD4fb_and(dsSIMD4fb_fromFloatBitfield(result->columns[2].simd), mask));
	result->columns[3].simd = dsSIMD4f_select(mask, box->center.simd, one);
}

inline void dsOrientedBox3xf_toMatrixTransposeSIMD(dsMatrix44f* result, const dsOrientedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsMatrix44f matrix;
	dsOrientedBox3xf_toMatrixSIMD(&matrix, box);
	dsMatrix44f_transposeSIMD(result, &matrix);
}

inline void dsOrientedBox3xf_fromMatrixSIMD(dsOrientedBox3xf* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	dsMatrix33xf rotationTrans;
	dsMatrix33xf_transposeSIMD(&rotationTrans, (const dsMatrix33xf*)matrix);

	dsSIMD4f halfExtents2 = dsSIMD4f_add(dsSIMD4f_add(
		dsSIMD4f_mul(rotationTrans.columns[0].simd, rotationTrans.columns[0].simd),
		dsSIMD4f_mul(rotationTrans.columns[1].simd, rotationTrans.columns[1].simd)),
		dsSIMD4f_mul(rotationTrans.columns[2].simd, rotationTrans.columns[2].simd));

#if DS_SIMD_APPROXIMATE_DIV_SQRT
	dsSIMD4f halfExtentsInv = dsSIMD4f_rsqrt(halfExtents2);
	result->halfExtents.simd = dsSIMD4f_mul(halfExtents2, halfExtentsInv);
#else
	result->halfExtents.simd = dsSIMD4f_sqrt(halfExtents2);
	dsSIMD4f halfExtentsInv = dsSIMD4f_rcp(result->halfExtents.simd);
#endif

	dsSIMD4f halfExtentsInvX = dsSIMD4f_set1FromVec(halfExtentsInv, 0);
	dsSIMD4f halfExtentsInvY = dsSIMD4f_set1FromVec(halfExtentsInv, 1);
	dsSIMD4f halfExtentsInvZ = dsSIMD4f_set1FromVec(halfExtentsInv, 2);

	result->orientation.columns[0].simd = dsSIMD4f_mul(matrix->columns[0].simd, halfExtentsInvX);
	result->orientation.columns[1].simd = dsSIMD4f_mul(matrix->columns[1].simd, halfExtentsInvY);
	result->orientation.columns[2].simd = dsSIMD4f_mul(matrix->columns[2].simd, halfExtentsInvZ);

	result->center.simd = matrix->columns[3].simd;
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

inline void dsOrientedBox3xf_fromMatrixFMA(dsOrientedBox3xf* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	dsMatrix33xf rotationTrans;
	dsMatrix33xf_transposeSIMD(&rotationTrans, (const dsMatrix33xf*)matrix);

	dsSIMD4f halfExtents2 = dsSIMD4f_fmadd(
		rotationTrans.columns[0].simd, rotationTrans.columns[0].simd,
		dsSIMD4f_fmadd(rotationTrans.columns[1].simd, rotationTrans.columns[1].simd,
		dsSIMD4f_mul(rotationTrans.columns[2].simd, rotationTrans.columns[2].simd)));

	result->halfExtents.simd = dsSIMD4f_sqrt(halfExtents2);
	dsSIMD4f halfExtentsInv = dsSIMD4f_rcp(result->halfExtents.simd);

	dsSIMD4f halfExtentsInvX = dsSIMD4f_set1FromVec(halfExtentsInv, 0);
	dsSIMD4f halfExtentsInvY = dsSIMD4f_set1FromVec(halfExtentsInv, 1);
	dsSIMD4f halfExtentsInvZ = dsSIMD4f_set1FromVec(halfExtentsInv, 2);

	result->orientation.columns[0].simd = dsSIMD4f_mul(matrix->columns[0].simd, halfExtentsInvX);
	result->orientation.columns[1].simd = dsSIMD4f_mul(matrix->columns[1].simd, halfExtentsInvY);
	result->orientation.columns[2].simd = dsSIMD4f_mul(matrix->columns[2].simd, halfExtentsInvZ);

	result->center.simd = matrix->columns[3].simd;
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2)

inline void dsOrientedBox3xd_toMatrixSIMD2(dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD2d x = dsSIMD2d_set1FromVec(box->halfExtents.simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(box->halfExtents.simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(box->halfExtents.simd2[1], 0);
	dsSIMD2d one = dsSIMD2d_set1(1.0);
	dsSIMD2db mask = dsSIMD2db_set2(0xFFFFFFFFFFFFFFFFULL, 0);

	result->columns[0].simd2[0] = dsSIMD2d_mul(box->orientation.columns[0].simd2[0], x);
	result->columns[0].simd2[1] = dsSIMD2d_mul(box->orientation.columns[0].simd2[1], x);
	result->columns[0].simd2[1] = dsSIMD2db_toDoubleBitfield(
		dsSIMD2db_and(dsSIMD2db_fromDoubleBitfield(result->columns[0].simd2[1]), mask));
	result->columns[1].simd2[0] = dsSIMD2d_mul(box->orientation.columns[1].simd2[0], y);
	result->columns[1].simd2[1] = dsSIMD2d_mul(box->orientation.columns[1].simd2[1], y);
	result->columns[1].simd2[1] = dsSIMD2db_toDoubleBitfield(
		dsSIMD2db_and(dsSIMD2db_fromDoubleBitfield(result->columns[1].simd2[1]), mask));
	result->columns[2].simd2[0] = dsSIMD2d_mul(box->orientation.columns[2].simd2[0], z);
	result->columns[2].simd2[1] = dsSIMD2d_mul(box->orientation.columns[2].simd2[1], z);
	result->columns[2].simd2[1] = dsSIMD2db_toDoubleBitfield(
		dsSIMD2db_and(dsSIMD2db_fromDoubleBitfield(result->columns[2].simd2[1]), mask));
	result->columns[3].simd2[0] = box->center.simd2[0];
	result->columns[3].simd2[1] = dsSIMD2d_select(mask, box->center.simd2[1], one);
}

inline void dsOrientedBox3xd_toMatrixTransposeSIMD2(
	dsMatrix44d* result, const dsOrientedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsMatrix44d matrix;
	dsOrientedBox3xd_toMatrixSIMD2(&matrix, box);
	dsMatrix44d_transposeSIMD2(result, &matrix);
}

inline void dsOrientedBox3xd_fromMatrixSIMD2(dsOrientedBox3xd* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	dsMatrix33xd rotationTrans;
	dsMatrix33xd_transposeSIMD2(&rotationTrans, (const dsMatrix33xd*)matrix);

	dsSIMD2d halfExtents20 = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(rotationTrans.columns[0].simd2[0], rotationTrans.columns[0].simd2[0]),
		dsSIMD2d_mul(rotationTrans.columns[1].simd2[0], rotationTrans.columns[1].simd2[0])),
		dsSIMD2d_mul(rotationTrans.columns[2].simd2[0], rotationTrans.columns[2].simd2[0]));
	dsSIMD2d halfExtents21 = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(rotationTrans.columns[0].simd2[1], rotationTrans.columns[0].simd2[1]),
		dsSIMD2d_mul(rotationTrans.columns[1].simd2[1], rotationTrans.columns[1].simd2[1])),
		dsSIMD2d_mul(rotationTrans.columns[2].simd2[1], rotationTrans.columns[2].simd2[1]));

	result->halfExtents.simd2[0] = dsSIMD2d_sqrt(halfExtents20);
	result->halfExtents.simd2[1] = dsSIMD2d_sqrt(halfExtents21);
	dsSIMD2d halfExtentsInv0 = dsSIMD2d_rcp(result->halfExtents.simd2[0]);
	dsSIMD2d halfExtentsInv1 = dsSIMD2d_rcp(result->halfExtents.simd2[1]);

	dsSIMD2d halfExtentsInvX = dsSIMD2d_set1FromVec(halfExtentsInv0, 0);
	dsSIMD2d halfExtentsInvY = dsSIMD2d_set1FromVec(halfExtentsInv0, 1);
	dsSIMD2d halfExtentsInvZ = dsSIMD2d_set1FromVec(halfExtentsInv1, 0);

	result->orientation.columns[0].simd2[0] = dsSIMD2d_mul(
		matrix->columns[0].simd2[0], halfExtentsInvX);
	result->orientation.columns[0].simd2[1] = dsSIMD2d_mul(
		matrix->columns[0].simd2[1], halfExtentsInvX);
	result->orientation.columns[1].simd2[0] = dsSIMD2d_mul(
		matrix->columns[1].simd2[0], halfExtentsInvY);
	result->orientation.columns[1].simd2[1] = dsSIMD2d_mul(
		matrix->columns[1].simd2[1], halfExtentsInvY);
	result->orientation.columns[2].simd2[0] = dsSIMD2d_mul(
		matrix->columns[2].simd2[0], halfExtentsInvZ);
	result->orientation.columns[2].simd2[1] = dsSIMD2d_mul(
		matrix->columns[2].simd2[1], halfExtentsInvZ);

	result->center.simd2[0] = matrix->columns[3].simd2[0];
	result->center.simd2[1] = matrix->columns[3].simd2[1];
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

inline void dsOrientedBox3xd_fromMatrixFMA2(dsOrientedBox3xd* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	dsMatrix33xd rotationTrans;
	dsMatrix33xd_transposeSIMD2(&rotationTrans, (const dsMatrix33xd*)matrix);

	dsSIMD2d halfExtents20 = dsSIMD2d_fmadd(
		rotationTrans.columns[0].simd2[0], rotationTrans.columns[0].simd2[0],
		dsSIMD2d_fmadd(rotationTrans.columns[1].simd2[0], rotationTrans.columns[1].simd2[0],
		dsSIMD2d_mul(rotationTrans.columns[2].simd2[0], rotationTrans.columns[2].simd2[0])));
	dsSIMD2d halfExtents21 = dsSIMD2d_fmadd(
		rotationTrans.columns[0].simd2[1], rotationTrans.columns[0].simd2[1],
		dsSIMD2d_fmadd(rotationTrans.columns[1].simd2[1], rotationTrans.columns[1].simd2[1],
		dsSIMD2d_mul(rotationTrans.columns[2].simd2[1], rotationTrans.columns[2].simd2[1])));

	result->halfExtents.simd2[0] = dsSIMD2d_sqrt(halfExtents20);
	result->halfExtents.simd2[1] = dsSIMD2d_sqrt(halfExtents21);
	dsSIMD2d halfExtentsInv0 = dsSIMD2d_rcp(result->halfExtents.simd2[0]);
	dsSIMD2d halfExtentsInv1 = dsSIMD2d_rcp(result->halfExtents.simd2[1]);

	dsSIMD2d halfExtentsInvX = dsSIMD2d_set1FromVec(halfExtentsInv0, 0);
	dsSIMD2d halfExtentsInvY = dsSIMD2d_set1FromVec(halfExtentsInv0, 1);
	dsSIMD2d halfExtentsInvZ = dsSIMD2d_set1FromVec(halfExtentsInv1, 0);

	result->orientation.columns[0].simd2[0] = dsSIMD2d_mul(
		matrix->columns[0].simd2[0], halfExtentsInvX);
	result->orientation.columns[0].simd2[1] = dsSIMD2d_mul(
		matrix->columns[0].simd2[1], halfExtentsInvX);
	result->orientation.columns[1].simd2[0] = dsSIMD2d_mul(
		matrix->columns[1].simd2[0], halfExtentsInvY);
	result->orientation.columns[1].simd2[1] = dsSIMD2d_mul(
		matrix->columns[1].simd2[1], halfExtentsInvY);
	result->orientation.columns[2].simd2[0] = dsSIMD2d_mul(
		matrix->columns[2].simd2[0], halfExtentsInvZ);
	result->orientation.columns[2].simd2[1] = dsSIMD2d_mul(
		matrix->columns[2].simd2[1], halfExtentsInvZ);

	result->center.simd2[0] = matrix->columns[3].simd2[0];
	result->center.simd2[1] = matrix->columns[3].simd2[1];
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4)

inline void dsOrientedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4d orientation0 = dsSIMD4d_load(box->orientation.columns);
	dsSIMD4d orientation1 = dsSIMD4d_load(box->orientation.columns + 1);
	dsSIMD4d orientation2 = dsSIMD4d_load(box->orientation.columns + 2);
	dsSIMD4d halfExtents = dsSIMD4d_load(&box->halfExtents);
	dsSIMD4d center = dsSIMD4d_load(&box->center);

	dsSIMD4d x = dsSIMD4d_set1FromVec(halfExtents, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(halfExtents, 1);
	dsSIMD4d z = dsSIMD4d_set1FromVec(halfExtents, 2);
	dsSIMD4d one = dsSIMD4d_set1(1.0);
	dsSIMD4db mask = dsSIMD4db_set4(
		0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0);

	orientation0 = dsSIMD4d_mul(orientation0, x);
	dsSIMD4d_store(result->columns, dsSIMD4db_toDoubleBitfield(
		dsSIMD4db_and(dsSIMD4db_fromDoubleBitfield(orientation0), mask)));
	orientation1 = dsSIMD4d_mul(orientation1, y);
	dsSIMD4d_store(result->columns + 1, dsSIMD4db_toDoubleBitfield(
		dsSIMD4db_and(dsSIMD4db_fromDoubleBitfield(orientation1), mask)));
	orientation2 = dsSIMD4d_mul(orientation2, z);
	dsSIMD4d_store(result->columns + 2, dsSIMD4db_toDoubleBitfield(
		dsSIMD4db_and(dsSIMD4db_fromDoubleBitfield(orientation2), mask)));
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_select(mask, center, one));
}

inline void dsOrientedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	DS_ALIGN(32) dsMatrix44d matrix;
	dsOrientedBox3xd_toMatrixSIMD4(&matrix, box);
	dsMatrix44d_transposeSIMD4(result, &matrix);
}

inline void dsOrientedBox3xd_fromMatrixSIMD4(
	dsOrientedBox3xd* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	DS_ALIGN(32) dsMatrix33xd rotationTrans;
	dsMatrix33xd_transposeSIMD4(&rotationTrans, (const dsMatrix33xd*)matrix);

	dsSIMD4d rotationTrans0 = dsSIMD4d_load(rotationTrans.columns);
	dsSIMD4d rotationTrans1 = dsSIMD4d_load(rotationTrans.columns + 1);
	dsSIMD4d rotationTrans2 = dsSIMD4d_load(rotationTrans.columns + 2);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d halfExtents2 = dsSIMD4d_fmadd(rotationTrans0, rotationTrans0,
		dsSIMD4d_fmadd(rotationTrans1, rotationTrans1,
		dsSIMD4d_mul(rotationTrans2, rotationTrans2)));
#else
	dsSIMD4d halfExtents2 = dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_mul(rotationTrans0, rotationTrans0),
		dsSIMD4d_mul(rotationTrans1, rotationTrans1)),
		dsSIMD4d_mul(rotationTrans2, rotationTrans2));
#endif

	dsSIMD4d halfExtents = dsSIMD4d_sqrt(halfExtents2);
	dsSIMD4d halfExtentsInv = dsSIMD4d_rcp(halfExtents);

	dsSIMD4d halfExtentsInvX = dsSIMD4d_set1FromVec(halfExtentsInv, 0);
	dsSIMD4d halfExtentsInvY = dsSIMD4d_set1FromVec(halfExtentsInv, 1);
	dsSIMD4d halfExtentsInvZ = dsSIMD4d_set1FromVec(halfExtentsInv, 2);

	dsSIMD4d_store(result->orientation.columns,
		dsSIMD4d_mul(dsSIMD4d_load(matrix->columns), halfExtentsInvX));
	dsSIMD4d_store(result->orientation.columns + 1,
		dsSIMD4d_mul(dsSIMD4d_load(matrix->columns + 1), halfExtentsInvY));
	dsSIMD4d_store(result->orientation.columns + 2,
		dsSIMD4d_mul(dsSIMD4d_load(matrix->columns + 2), halfExtentsInvZ));

	dsSIMD4d_store(&result->halfExtents, halfExtents);
	dsSIMD4d_store(&result->center, dsSIMD4d_load(matrix->columns + 3));
}

DS_SIMD_END()

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
