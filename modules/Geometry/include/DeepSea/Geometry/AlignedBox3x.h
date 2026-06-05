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

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#include <DeepSea/Math/SIMD/SIMD.h>

#include <float.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating dsAlignedBox3x* structures.
 *
 * The functions have different versions for the supported dsAlignedBox3 types, and will utilize
 * SIMD operations where supported at compile-time.
 *
 * @see dsAlignedBox3xf dsAlignedBox3xd
 */

#if DS_HAS_SIMD

/**
 * @brief Converts the aligned box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_toMatrixSIMD(
	dsMatrix44f* result, const dsAlignedBox3xf* box);

/**
 * @brief Converts the aligned box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_toMatrixTransposeSIMD(
	dsMatrix44f* result, const dsAlignedBox3xf* box);

/**
 * @brief Converts the aligned box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrixSIMD2(
	dsMatrix44d* result, const dsAlignedBox3xd* box);

/**
 * @brief Converts the aligned box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrixTransposeSIMD2(
	dsMatrix44d* result, const dsAlignedBox3xd* box);

/**
 * @brief Converts the aligned box to a matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box);

/**
 * @brief Converts the aligned box to a transposed matrix representation using SIMD operations.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box);

#endif // DS_HAS_SIMD

/** @see dsAlignedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xf_isValid(const dsAlignedBox3xf* box)
{
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4f_cmple(box->min.simd, box->max.simd);
	return result.x && result.y && result.z;
#else
	return dsAlignedBox3_isValid(*box);
#endif
}

/** @see dsAlignedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xd_isValid(const dsAlignedBox3xd* box)
{
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector4l result;
	result.simd2[0] = dsSIMD2d_cmple(box->min.simd2[0], box->max.simd2[0]);
	result.simd2[1] = dsSIMD2d_cmple(box->min.simd2[1], box->max.simd2[1]);
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsAlignedBox3_isValid(*box);
#endif
}

/** @see dsAlignedBox3_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_addPoint(
	dsAlignedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_FLOAT4
	box->min.simd = dsSIMD4f_min(box->min.simd, point->simd);
	box->max.simd = dsSIMD4f_max(box->max.simd, point->simd);
#else
	dsAlignedBox3_addPoint(*box, *point);
#endif
}

/** @see dsAlignedBox3_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_addPoint(
	dsAlignedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_DOUBLE2
	box->min.simd2[0] = dsSIMD2d_min(box->min.simd2[0], point->simd2[0]);
	box->min.simd2[1] = dsSIMD2d_min(box->min.simd2[1], point->simd2[1]);
	box->max.simd2[0] = dsSIMD2d_max(box->max.simd2[0], point->simd2[0]);
	box->max.simd2[1] = dsSIMD2d_max(box->max.simd2[1], point->simd2[1]);
#else
	dsAlignedBox3_addPoint(*box, *point);
#endif
}

/** @see dsAlignedBox3_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_addBox(
	dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_FLOAT4
	box->min.simd = dsSIMD4f_min(box->min.simd, otherBox->min.simd);
	box->max.simd = dsSIMD4f_max(box->max.simd, otherBox->max.simd);
#else
	dsAlignedBox3_addBox(*box, *otherBox);
#endif
}

/** @see dsAlignedBox3_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_addBox(
	dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_DOUBLE2
	box->min.simd2[0] = dsSIMD2d_min(box->min.simd2[0], otherBox->min.simd2[0]);
	box->min.simd2[1] = dsSIMD2d_min(box->min.simd2[1], otherBox->min.simd2[1]);
	box->max.simd2[0] = dsSIMD2d_max(box->max.simd2[0], otherBox->max.simd2[0]);
	box->max.simd2[1] = dsSIMD2d_max(box->max.simd2[1], otherBox->max.simd2[1]);
#else
	dsAlignedBox3_addBox(*box, *otherBox);
#endif
}

/** @copydoc dsAlignedBox3_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xf_containsPoint(
	const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4fb_and(
		dsSIMD4f_cmpge(point->simd, box->min.simd), dsSIMD4f_cmple(point->simd, box->max.simd));
	return result.x && result.y && result.z;
#else
	return dsAlignedBox3_containsPoint(*box, *point);
#endif
}

/** @copydoc dsAlignedBox3_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xd_containsPoint(
	const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector4l result;
	result.simd2[0] = dsSIMD2db_and(dsSIMD2d_cmpge(point->simd2[0], box->min.simd2[0]),
		dsSIMD2d_cmple(point->simd2[0], box->max.simd2[0]));
	result.simd2[1] = dsSIMD2db_and(dsSIMD2d_cmpge(point->simd2[1], box->min.simd2[1]),
		dsSIMD2d_cmple(point->simd2[1], box->max.simd2[1]));
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsAlignedBox3_containsPoint(*box, *point);
#endif
}

/** @copydoc dsAlignedBox3_containsBox()  */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xf_containsBox(
	const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4fb_and(dsSIMD4f_cmple(box->min.simd, otherBox->min.simd),
		dsSIMD4f_cmpge(box->max.simd, otherBox->max.simd));
	return result.x && result.y && result.z;
#else
	return dsAlignedBox3_containsBox(*box, *otherBox);
#endif
}

/** @copydoc dsAlignedBox3_containsBox()  */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xd_containsBox(
	const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector4l result;
	result.simd2[0] = dsSIMD2db_and(dsSIMD2d_cmple(box->min.simd2[0], otherBox->min.simd2[0]),
		dsSIMD2d_cmpge(box->max.simd2[0], otherBox->max.simd2[0]));
	result.simd2[1] = dsSIMD2db_and(dsSIMD2d_cmple(box->min.simd2[1], otherBox->min.simd2[1]),
		dsSIMD2d_cmpge(box->max.simd2[1], otherBox->max.simd2[1]));
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsAlignedBox3_containsBox(*box, *otherBox);
#endif
}

/** @copydoc dsAlignedBox3_intersects()  */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xf_intersects(
	const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4fb_and(dsSIMD4f_cmple(box->min.simd, otherBox->max.simd),
		dsSIMD4f_cmpge(box->max.simd, otherBox->min.simd));
	return result.x && result.y && result.z;
#else
	return dsAlignedBox3_intersects(*box, *otherBox);
#endif
}

/** @copydoc dsAlignedBox3_intersects()  */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3xd_intersects(
	const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector4l result;
	result.simd2[0] = dsSIMD2db_and(dsSIMD2d_cmple(box->min.simd2[0], otherBox->max.simd2[0]),
		dsSIMD2d_cmpge(box->max.simd2[0], otherBox->min.simd2[0]));
	result.simd2[1] = dsSIMD2db_and(dsSIMD2d_cmple(box->min.simd2[1], otherBox->max.simd2[1]),
		dsSIMD2d_cmpge(box->max.simd2[1], otherBox->min.simd2[1]));
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsAlignedBox3_intersects(*box, *otherBox);
#endif
}

/** @copydoc dsAlignedBox3_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_intersect(
	dsAlignedBox3xf* result, const dsAlignedBox3xf* a, const dsAlignedBox3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	result->min.simd = dsSIMD4f_max(a->min.simd, b->min.simd);
	result->max.simd = dsSIMD4f_min(a->max.simd, b->max.simd);
#else
	dsAlignedBox3_intersect(*result, *a, *b);
#endif
}

/** @copydoc dsAlignedBox3_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_intersect(
	dsAlignedBox3xd* result, const dsAlignedBox3xd* a, const dsAlignedBox3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->min.simd2[0] = dsSIMD2d_max(a->min.simd2[0], b->min.simd2[0]);
	result->min.simd2[1] = dsSIMD2d_max(a->min.simd2[1], b->min.simd2[1]);
	result->max.simd2[0] = dsSIMD2d_min(a->max.simd2[0], b->max.simd2[0]);
	result->max.simd2[1] = dsSIMD2d_min(a->max.simd2[1], b->max.simd2[1]);
#else
	dsAlignedBox3_intersect(*result, *a, *b);
#endif
}

/** @copydoc dsAlignedBox3_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_center(
	dsVector3xf* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f half = dsSIMD4f_set1(0.5f);
	result->simd = dsSIMD4f_mul(dsSIMD4f_add(box->min.simd, box->max.simd), half);
#else
	dsAlignedBox3_center(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_center(
	dsVector3xd* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d half = dsSIMD2d_set1(0.5);
	result->simd2[0] = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[0], box->max.simd2[0]), half);
	result->simd2[1] = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[1], box->max.simd2[1]), half);
#else
	dsAlignedBox3_center(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_extents(
	dsVector3xf* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_sub(box->max.simd, box->min.simd);
#else
	dsAlignedBox3_extents(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_extents(
	dsVector3xd* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_sub(box->max.simd2[0], box->min.simd2[0]);
	result->simd2[1] = dsSIMD2d_sub(box->max.simd2[1], box->min.simd2[1]);
#else
	dsAlignedBox3_extents(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_toMatrix(
	dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsAlignedBox3xf_toMatrixSIMD(result, box);
#else
	dsAlignedBox3_toMatrix(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrix(
	dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsAlignedBox3xd_toMatrixSIMD2(result, box);
#else
	dsAlignedBox3_toMatrix(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_toMatrixTranspose(
	dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4
	dsAlignedBox3xf_toMatrixTransposeSIMD(result, box);
#else
	dsAlignedBox3_toMatrixTranspose(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_toMatrixTranspose(
	dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsAlignedBox3xd_toMatrixTransposeSIMD2(result, box);
#else
	dsAlignedBox3_toMatrixTranspose(*result, *box);
#endif
}

/** @copydoc dsAlignedBox3_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_corners(
	dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xf* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FLOAT4 && (DS_X86 || DS_ARM)
	corners[dsBox3Corner_xyz].simd = box->min.simd;
#if DS_X86
	dsSIMD4f minMaxLow = _mm_shuffle_ps(box->min.simd, box->max.simd, _MM_SHUFFLE(1, 0, 1, 0));
	corners[dsBox3Corner_xyZ].simd = _mm_shuffle_ps(
		box->min.simd, box->max.simd, _MM_SHUFFLE(3, 2, 1, 0));
	corners[dsBox3Corner_xYz].simd = _mm_shuffle_ps(
		minMaxLow, box->min.simd, _MM_SHUFFLE(3, 2, 3, 0));
	corners[dsBox3Corner_xYZ].simd = _mm_shuffle_ps(
		minMaxLow, box->max.simd, _MM_SHUFFLE(3, 2, 3, 0));
	corners[dsBox3Corner_Xyz].simd = _mm_shuffle_ps(
		minMaxLow, box->min.simd, _MM_SHUFFLE(3, 2, 1, 2));
	corners[dsBox3Corner_XyZ].simd = _mm_shuffle_ps(
		minMaxLow, box->max.simd, _MM_SHUFFLE(3, 2, 1, 2));
	corners[dsBox3Corner_XYz].simd = _mm_shuffle_ps(
		box->max.simd, box->min.simd, _MM_SHUFFLE(3, 2, 1, 0));
#elif DS_ARM_64
	corners[dsBox3Corner_xyZ].simd = vcopyq_laneq_f32(box->min.simd, 2, box->max.simd, 2);
	corners[dsBox3Corner_xYz].simd = vcopyq_laneq_f32(box->min.simd, 1, box->max.simd, 1);
	corners[dsBox3Corner_xYZ].simd = vcopyq_laneq_f32(box->max.simd, 0, box->min.simd, 0);
	corners[dsBox3Corner_Xyz].simd = vcopyq_laneq_f32(box->min.simd, 0, box->max.simd, 0);
	corners[dsBox3Corner_XyZ].simd = vcopyq_laneq_f32(box->max.simd, 1, box->min.simd, 1);
	corners[dsBox3Corner_XYz].simd = vcopyq_laneq_f32(box->max.simd, 2, box->min.simd, 2);
#elif DS_ARM_32
	float32x2_t minLow = vget_low_f32(box->min.simd);
	float32x2_t minHigh = vget_high_f32(box->min.simd);
	float32x2_t maxLow = vget_low_f32(box->max.simd);
	float32x2_t maxHigh = vget_high_f32(box->max.simd);
	float32x2x2_t lowMix0 = vtrn_f32(minLow, maxLow);
	float32x2x2_t lowMix = vtrn_f32(lowMix0.val[0], vrev64_f32(lowMix0.val[1]));
	corners[dsBox3Corner_xyZ].simd = vcombine_f32(minLow, maxHigh);
	corners[dsBox3Corner_xYz].simd = vcombine_f32(lowMix.val[0], minHigh);
	corners[dsBox3Corner_xYZ].simd = vcombine_f32(lowMix.val[0], maxHigh);
	corners[dsBox3Corner_Xyz].simd = vcombine_f32(lowMix.val[1], minHigh);
	corners[dsBox3Corner_XyZ].simd = vcombine_f32(lowMix.val[1], maxHigh);
	corners[dsBox3Corner_XYz].simd = vcombine_f32(maxLow, minHigh);
#else
#error Need to implement assigning of corners for this platform.
#endif
	corners[dsBox3Corner_XYZ].simd = box->max.simd;
#else
	dsAlignedBox3_corners(corners, *box);
#endif
}

/** @copydoc dsAlignedBox3_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_corners(
	dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xd* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_DOUBLE2 && (DS_X86 || DS_ARM)
#if DS_X86
	dsSIMD2d xY = _mm_shuffle_pd(box->min.simd2[0], box->max.simd2[0], _MM_SHUFFLE2(1, 0));
	dsSIMD2d Xy = _mm_shuffle_pd(box->max.simd2[0], box->min.simd2[0], _MM_SHUFFLE2(1, 0));
#elif DS_ARM_64
	dsSIMD2d xY = vcopyq_laneq_f64(box->min.simd2[0], 1, box->max.simd2[0], 1);
	dsSIMD2d Xy = vcopyq_laneq_f64(box->min.simd2[0], 0, box->max.simd2[0], 0);
#else
#error Need to implement assigning of corners for this platform.
#endif
	corners[dsBox3Corner_xyz].simd2[0] = box->min.simd2[0];
	corners[dsBox3Corner_xyz].simd2[1] = box->min.simd2[1];
	corners[dsBox3Corner_xyZ].simd2[0] = box->min.simd2[0];
	corners[dsBox3Corner_xyZ].simd2[1] = box->max.simd2[1];
	corners[dsBox3Corner_xYz].simd2[0] = xY;
	corners[dsBox3Corner_xYz].simd2[1] = box->min.simd2[1];
	corners[dsBox3Corner_xYZ].simd2[0] = xY;
	corners[dsBox3Corner_xYZ].simd2[1] = box->max.simd2[1];
	corners[dsBox3Corner_Xyz].simd2[0] = Xy;
	corners[dsBox3Corner_Xyz].simd2[1] = box->min.simd2[1];
	corners[dsBox3Corner_XyZ].simd2[0] = Xy;
	corners[dsBox3Corner_XyZ].simd2[1] = box->max.simd2[1];
	corners[dsBox3Corner_XYz].simd2[0] = box->max.simd2[0];
	corners[dsBox3Corner_XYz].simd2[1] = box->min.simd2[1];
	corners[dsBox3Corner_XYZ].simd2[0] = box->max.simd2[0];
	corners[dsBox3Corner_XYZ].simd2[1] = box->max.simd2[1];
#else
	dsAlignedBox3_corners(corners, *box);
#endif
}

/** @copydoc dsAlignedBox3_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_closestPoint(
	dsVector3xf* result, const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_FLOAT4
	if (dsAlignedBox3xf_isValid(box))
		result->simd = dsSIMD4f_min(box->max.simd, dsSIMD4f_max(box->min.simd, point->simd));
	else
		result->simd = point->simd;
#else
	dsAlignedBox3_closestPoint(*result, *box, *point);
#endif
}

/** @copydoc dsAlignedBox3_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_closestPoint(
	dsVector3xd* result, const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
#if DS_SIMD_ALWAYS_DOUBLE2
	if (dsAlignedBox3xd_isValid(box))
	{
		result->simd2[0] = dsSIMD2d_min(
			box->max.simd2[0], dsSIMD2d_max(box->min.simd2[0], point->simd2[0]));
		result->simd2[1] = dsSIMD2d_min(
			box->max.simd2[1], dsSIMD2d_max(box->min.simd2[1], point->simd2[1]));
	}
	else
	{
		result->simd2[0] = point->simd2[0];
		result->simd2[1] = point->simd2[1];
	}
#else
	dsAlignedBox3_closestPoint(*result, *box, *point);
#endif
}

/** @copydoc dsAlignedBox3f_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xf_makeInvalid(dsAlignedBox3xf* result)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->min.simd = dsSIMD4f_set1(FLT_MAX);
	result->max.simd = dsSIMD4f_set1(-FLT_MAX);
#else
	result->min.x = FLT_MAX;
	result->min.y = FLT_MAX;
	result->min.z = FLT_MAX;
	result->min.w = FLT_MAX;
	result->max.x = -FLT_MAX;
	result->max.y = -FLT_MAX;
	result->max.z = -FLT_MAX;
	result->max.w = -FLT_MAX;
#endif
}

/** @copydoc dsAlignedBox3f_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3xd_makeInvalid(dsAlignedBox3xd* result)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d minVal = dsSIMD2d_set1(DBL_MAX);
	dsSIMD2d maxVal = dsSIMD2d_set1(-DBL_MAX);
	result->min.simd2[0] = minVal;
	result->min.simd2[1] = minVal;
	result->max.simd2[0] = maxVal;
	result->max.simd2[1] = maxVal;
#else
	result->min.x = DBL_MAX;
	result->min.y = DBL_MAX;
	result->min.z = DBL_MAX;
	result->min.w = DBL_MAX;
	result->max.x = -DBL_MAX;
	result->max.y = -DBL_MAX;
	result->max.z = -DBL_MAX;
	result->max.w = -DBL_MAX;
#endif
}

/** @copydoc dsAlignedBox3f_dist2() */
DS_GEOMETRY_EXPORT float dsAlignedBox3xf_dist2(
	const dsAlignedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsAlignedBox3f_dist2() */
DS_GEOMETRY_EXPORT double dsAlignedBox3xd_dist2(
	const dsAlignedBox3xd* box, const dsVector3xd* point);

/** @copydoc dsAlignedBox3f_dist() */
DS_GEOMETRY_EXPORT float dsAlignedBox3xf_dist(
	const dsAlignedBox3xf* box, const dsVector3xf* point);

/** @copydoc dsAlignedBox3f_dist() */
DS_GEOMETRY_EXPORT double dsAlignedBox3xd_dist(
	const dsAlignedBox3xd* box, const dsVector3xd* point);

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4)

inline void dsAlignedBox3xf_toMatrixSIMD(dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4f half = dsSIMD4f_set1(0.5f);
	dsSIMD4f halfExtents = dsSIMD4f_mul(dsSIMD4f_sub(box->max.simd, box->min.simd), half);
	dsSIMD4f center = dsSIMD4f_mul(dsSIMD4f_add(box->min.simd, box->max.simd), half);
	dsSIMD4f zero = dsSIMD4f_set1(0.0f);
#if DS_X86
	dsSIMD4f one = dsSIMD4f_set1(1.0f);
	dsSIMD4f halfExtents0 = _mm_shuffle_ps(halfExtents, zero, _MM_SHUFFLE(0, 0, 1, 0));
	dsSIMD4f halfExtents1 = _mm_shuffle_ps(halfExtents, zero, _MM_SHUFFLE(0, 0, 0, 2));
	dsSIMD4f center1 = _mm_shuffle_ps(center, one, _MM_SHUFFLE(0, 0, 0, 2));
	result->columns[0].simd = _mm_shuffle_ps(halfExtents0, zero, _MM_SHUFFLE(0, 0, 3, 0));
	result->columns[1].simd = _mm_shuffle_ps(halfExtents0, zero, _MM_SHUFFLE(0, 0, 1, 3));
	result->columns[2].simd = _mm_shuffle_ps(zero, halfExtents1, _MM_SHUFFLE(3, 0, 0, 0));
	result->columns[3].simd = _mm_shuffle_ps(center, center1, _MM_SHUFFLE(3, 0, 1, 0));
#elif DS_ARM_64
	result->columns[0].simd = vcopyq_laneq_f32(zero, 0, halfExtents, 0);
	result->columns[1].simd = vcopyq_laneq_f32(zero, 1, halfExtents, 1);
	result->columns[2].simd = vcopyq_laneq_f32(zero, 2, halfExtents, 2);
	result->columns[3].simd = vsetq_lane_f32(1.0f, center, 3);
#elif DS_ARM_32
	dsVector4f scalarHalfExtents;
	scalarHalfExtents.simd = halfExtents;
	result->columns[0].simd = vsetq_lane_f32(scalarHalfExtents.x, zero, 0);
	result->columns[1].simd = vsetq_lane_f32(scalarHalfExtents.y, zero, 1);
	result->columns[2].simd = vsetq_lane_f32(scalarHalfExtents.z, zero, 2);
	result->columns[3].simd = vsetq_lane_f32(1.0f, center, 3);
#else
#error Need to implement assigning of matrix columns for this platform.
#endif
}

inline void dsAlignedBox3xf_toMatrixTransposeSIMD(dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4f half = dsSIMD4f_set1(0.5f);
	dsSIMD4f halfExtents = dsSIMD4f_mul(dsSIMD4f_sub(box->max.simd, box->min.simd), half);
	dsSIMD4f center = dsSIMD4f_mul(dsSIMD4f_add(box->min.simd, box->max.simd), half);
	dsSIMD4f lastCol = dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f);
#if DS_X86
	dsSIMD4f halfExtents0 = _mm_shuffle_ps(halfExtents, lastCol, _MM_SHUFFLE(0, 0, 1, 0));
	dsSIMD4f halfExtents1 = _mm_shuffle_ps(halfExtents, lastCol, _MM_SHUFFLE(0, 0, 0, 2));
	dsSIMD4f center0 = _mm_shuffle_ps(center, lastCol, _MM_SHUFFLE(0, 0, 1, 0));
	dsSIMD4f center1 = _mm_shuffle_ps(center, lastCol, _MM_SHUFFLE(0, 0, 0, 2));
	dsSIMD4f halfExtentsCenter1 = _mm_shuffle_ps(halfExtents1, center1, _MM_SHUFFLE(3, 0, 3, 0));
	result->columns[0].simd = _mm_shuffle_ps(halfExtents0, center0, _MM_SHUFFLE(0, 3, 3, 0));
	result->columns[1].simd = _mm_shuffle_ps(halfExtents0, center0, _MM_SHUFFLE(1, 3, 1, 3));
	result->columns[2].simd = _mm_shuffle_ps(lastCol, halfExtentsCenter1, _MM_SHUFFLE(2, 0, 0, 0));
	result->columns[3].simd = lastCol;
#elif DS_ARM
	float32x2_t zero = vdup_n_f32(0.0f);
	float32x2_t halfExtents0 = vget_low_f32(halfExtents);
	float32x2_t halfExtents1 = vget_high_f32(halfExtents);
	float32x2_t center0 = vget_low_f32(center);
	float32x2_t center1 = vget_high_f32(center);
	float32x2x2_t center0Trans = vtrn_f32(zero, center0);
#if DS_ARM_64
	result->columns[0].simd = vcombine_f32(vtrn1_f32(halfExtents0, zero), center0Trans.val[0]);
	result->columns[1].simd = vcombine_f32(vtrn2_f32(zero, halfExtents0), center0Trans.val[1]);
	result->columns[2].simd = vcombine_f32(zero, vtrn1_f32(halfExtents1, center1));
	result->columns[3].simd = lastCol;
#else
	float32x2x2_t halfExtents0Trans = vtrn_f32(halfExtents0, zero);
	float32x2x2_t halfExtentsCenter1Trans = vtrn_f32(halfExtents1, center1);
	result->columns[0].simd = vcombine_f32(halfExtents0Trans.val[0], center0Trans.val[0]);
	result->columns[1].simd = vcombine_f32(
		vrev64_f32(halfExtents0Trans.val[1]), center0Trans.val[1]);
	result->columns[2].simd = vcombine_f32(zero, halfExtentsCenter1Trans.val[0]);
	result->columns[3].simd = lastCol;
#endif
#else
#error Need to implement assigning of matrix columns for this platform.
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2)

inline void dsAlignedBox3xd_toMatrixSIMD2(dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD2d half = dsSIMD2d_set1(0.5);
	dsSIMD2d halfExtents0 = dsSIMD2d_mul(dsSIMD2d_sub(box->max.simd2[0], box->min.simd2[0]), half);
	dsSIMD2d halfExtents1 = dsSIMD2d_mul(dsSIMD2d_sub(box->max.simd2[1], box->min.simd2[1]), half);
	dsSIMD2d center0 = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[0], box->max.simd2[0]), half);
	dsSIMD2d center1 = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[1], box->max.simd2[1]), half);
	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsSIMD2d one = dsSIMD2d_set1(1.0);
#if DS_X86
	result->columns[0].simd2[0] = _mm_shuffle_pd(halfExtents0, zero, _MM_SHUFFLE2(0, 0));
	result->columns[0].simd2[1] = zero;
	result->columns[1].simd2[0] = _mm_shuffle_pd(zero, halfExtents0, _MM_SHUFFLE2(1, 0));
	result->columns[1].simd2[1] = zero;
	result->columns[2].simd2[0] = zero;
	result->columns[2].simd2[1] = _mm_shuffle_pd(halfExtents1, zero, _MM_SHUFFLE2(0, 0));
	result->columns[3].simd2[0] = center0;
	result->columns[3].simd2[1] = _mm_shuffle_pd(center1, one, _MM_SHUFFLE2(0, 0));
#elif DS_ARM_64
	result->columns[0].simd2[0] = vtrn1q_f64(halfExtents0, zero);
	result->columns[0].simd2[1] = zero;
	result->columns[1].simd2[0] = vtrn2q_f64(zero, halfExtents0);
	result->columns[1].simd2[1] = zero;
	result->columns[2].simd2[0] = zero;
	result->columns[2].simd2[1] = vtrn1q_f64(halfExtents1, zero);
	result->columns[3].simd2[0] = center0;
	result->columns[3].simd2[1] = vtrn1q_f64(center1, one);
#else
	// Not all platforms have double2 support.
	DS_UNUSED(result);
	DS_UNUSED(half);
	DS_UNUSED(halfExtents0);
	DS_UNUSED(halfExtents1);
	DS_UNUSED(center0);
	DS_UNUSED(center1);
	DS_UNUSED(zero);
	DS_UNUSED(one);
	DS_ASSERT(false);
#endif
}

inline void dsAlignedBox3xd_toMatrixTransposeSIMD2(dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD2d half = dsSIMD2d_set1(0.5);
	dsSIMD2d halfExtents0 = dsSIMD2d_mul(dsSIMD2d_sub(box->max.simd2[0], box->min.simd2[0]), half);
	dsSIMD2d halfExtents1 = dsSIMD2d_mul(dsSIMD2d_sub(box->max.simd2[1], box->min.simd2[1]), half);
	dsSIMD2d center0 = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[0], box->max.simd2[0]), half);
	dsSIMD2d center1 = dsSIMD2d_mul(dsSIMD2d_add(box->min.simd2[1], box->max.simd2[1]), half);
	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsSIMD2d lastCol1 = dsSIMD2d_set2(0.0, 1.0);
#if DS_X86
	result->columns[0].simd2[0] = _mm_shuffle_pd(halfExtents0, zero, _MM_SHUFFLE2(0, 0));
	result->columns[0].simd2[1] = _mm_shuffle_pd(zero, center0, _MM_SHUFFLE2(0, 0));
	result->columns[1].simd2[0] = _mm_shuffle_pd(zero, halfExtents0, _MM_SHUFFLE2(1, 0));
	result->columns[1].simd2[1] = _mm_shuffle_pd(zero, center0, _MM_SHUFFLE2(1, 0));
	result->columns[2].simd2[0] = zero;
	result->columns[2].simd2[1] = _mm_shuffle_pd(halfExtents1, center1, _MM_SHUFFLE2(0, 0));
	result->columns[3].simd2[0] = zero;
	result->columns[3].simd2[1] = lastCol1;
#elif DS_ARM_64
	result->columns[0].simd2[0] = vtrn1q_f64(halfExtents0, zero);
	result->columns[0].simd2[1] = vtrn1q_f64(zero, center0);
	result->columns[1].simd2[0] = vtrn2q_f64(zero, halfExtents0);
	result->columns[1].simd2[1] = vtrn2q_f64(zero, center0);
	result->columns[2].simd2[0] = zero;
	result->columns[2].simd2[1] = vtrn1q_f64(halfExtents1, center1);
	result->columns[3].simd2[0] = zero;
	result->columns[3].simd2[1] = lastCol1;
#else
	// Not all platforms have double2 support.
	DS_UNUSED(result);
	DS_UNUSED(half);
	DS_UNUSED(halfExtents0);
	DS_UNUSED(halfExtents1);
	DS_UNUSED(center0);
	DS_UNUSED(center1);
	DS_UNUSED(zero);
	DS_UNUSED(lastCol1);
	DS_ASSERT(false);
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4)

inline void dsAlignedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4d min = dsSIMD4d_load(&box->min);
	dsSIMD4d max = dsSIMD4d_load(&box->max);
	dsSIMD4d half = dsSIMD4d_set1(0.5);
	dsSIMD4d halfExtents = dsSIMD4d_mul(dsSIMD4d_sub(max, min), half);
	dsSIMD4d center = dsSIMD4d_mul(dsSIMD4d_add(min, max), half);
	dsSIMD4d zero = dsSIMD4d_set1(0.0);
#if DS_X86
	dsSIMD4d one = dsSIMD4d_set1(1.0);
	dsSIMD4d halfExtents0 = _mm256_permute2f128_pd(halfExtents, zero, _MM_SHUFFLE(0, 2, 0, 0));
	dsSIMD4d halfExtents1 = _mm256_permute2f128_pd(zero, halfExtents, _MM_SHUFFLE(0, 3, 0, 0));
	dsSIMD4d center0 = _mm256_permute2f128_pd(center, one, _MM_SHUFFLE(0, 2, 0, 0));
	dsSIMD4d_store(result->columns, _mm256_shuffle_pd(halfExtents0, zero, 0));
	dsSIMD4d_store(
		result->columns + 1, _mm256_shuffle_pd(zero, halfExtents0, 0x2));
	dsSIMD4d_store(
		result->columns + 2, _mm256_shuffle_pd(halfExtents1, zero, 0));
	dsSIMD4d_store(
		result->columns + 3, _mm256_shuffle_pd(center, center0, 0x2));
#else
	// Not all platforms have double4 support.
	DS_UNUSED(result);
	DS_UNUSED(min);
	DS_UNUSED(max);
	DS_UNUSED(half);
	DS_UNUSED(halfExtents);
	DS_UNUSED(center);
	DS_UNUSED(zero);
	DS_ASSERT(false);
#endif
}

inline void dsAlignedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);

	dsSIMD4d min = dsSIMD4d_load(&box->min);
	dsSIMD4d max = dsSIMD4d_load(&box->max);
	dsSIMD4d half = dsSIMD4d_set1(0.5);
	dsSIMD4d halfExtents = dsSIMD4d_mul(dsSIMD4d_sub(max, min), half);
	dsSIMD4d center = dsSIMD4d_mul(dsSIMD4d_add(min, max), half);
	dsSIMD4d zero = dsSIMD4d_set1(0.0);
#if DS_X86
	dsSIMD4d halfExtents0 = _mm256_unpacklo_pd(halfExtents, zero);
	dsSIMD4d halfExtents1 = _mm256_unpackhi_pd(zero, halfExtents);
	dsSIMD4d center0 = _mm256_unpacklo_pd(zero, center);
	dsSIMD4d center1 = _mm256_unpackhi_pd(zero, center);
	dsSIMD4d halfExtentsCenter2 = _mm256_unpacklo_pd(halfExtents, center);
	dsSIMD4d_store(result->columns,
		_mm256_permute2f128_pd(halfExtents0, center0, _MM_SHUFFLE(0, 2, 0, 0)));
	dsSIMD4d_store(result->columns + 1,
		_mm256_permute2f128_pd(halfExtents1, center1, _MM_SHUFFLE(0, 2, 0, 0)));
	dsSIMD4d_store(result->columns + 2,
		_mm256_permute2f128_pd(zero, halfExtentsCenter2, _MM_SHUFFLE(0, 3, 0, 0)));
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0));
#else
	// Not all platforms have double4 support.
	DS_UNUSED(result);
	DS_UNUSED(min);
	DS_UNUSED(max);
	DS_UNUSED(half);
	DS_UNUSED(halfExtents);
	DS_UNUSED(center);
	DS_UNUSED(zero);
	DS_ASSERT(false);
#endif
}

DS_SIMD_END()

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
