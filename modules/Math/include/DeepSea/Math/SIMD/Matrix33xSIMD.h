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

#include <DeepSea/Math/SIMD/Dot.h>
#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for performing 3x3 matrix operations with SIMD.
 *
 * These will only be available if DS_HAS_SIMD is set to 1. dsMatrix33x* functions will use the
 * fastest operations available at compile time, but these functions can be used directly when
 * checking for capabilities at runtime, preferably before a loop of many operations.
 *
 * Variations of the same function are only provided if there's a benefit. For example, there may
 * be no FMA version if the implementation would be the same as the SIMD version.
 */

#if DS_HAS_SIMD

/**
 * @brief Multiplies two matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_mulSIMD(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);

/**
 * @brief Multiplies two affine matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_affineMulSIMD(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);

/**
 * @brief Transforms a vector by a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_transformSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_transformTransposedSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_transposeSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline float dsMatrix33xf_determinantSIMD(const dsMatrix33xf* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_fastInvertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Inverts an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_affineInvertSIMD(
	dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_invertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector3xf elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_inverseTransposeSIMD(
	dsMatrix22f* result, const dsMatrix33xf* a);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_mulFMA(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);

/**
 * @brief Multiplies two affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_affineMulFMA(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);

/**
 * @brief Transforms a vector by a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_transformFMA(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);

/**
 * @brief Transforms a vector by a transposed matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_transformTransposedFMA(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);

/**
 * @brief Computes the determinant of a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline float dsMatrix33xf_determinantFMA(const dsMatrix33xf* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_fastInvertFMA(dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Inverts an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_affineInvertFMA(
	dsMatrix33xf* result, const dsMatrix33xf* a);

/**
 * @brief Inverts a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xf_invertFMA(dsMatrix33xf* result, const dsMatrix33xf* a);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_mulSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

/**
 * @brief Multiplies two affine matrices.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineMulSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

/**
 * @brief Transforms a vector by a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformTransposedSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transposeSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix33xd_determinantSIMD2(const dsMatrix33xd* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_fastInvertSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Inverts an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineInvertSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_invertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector3xd elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_inverseTransposeSIMD2(
	dsMatrix22d* result, const dsMatrix33xd* a);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_mulFMA2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

/**
 * @brief Multiplies two affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineMulFMA2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

/**
 * @brief Transforms a vector by a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

/**
 * @brief Transforms a vector by a transposed matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformTransposedFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

/**
 * @brief Computes the determinant of a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix33xd_determinantFMA2(const dsMatrix33xd* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_fastInvertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Inverts an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineInvertFMA2(
	dsMatrix33xd* result, const dsMatrix33xd* a);

/**
 * @brief Inverts a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_invertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_mulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b);

/**
 * @brief Multiplies two affine matrices.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineMulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b);

/**
 * @brief Transforms a vector by a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) mat, const dsVector3xd* DS_ALIGN_PARAM(32) vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transformTransposedSIMD4(
	dsVector3xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) mat,
	const dsVector3xd* DS_ALIGN_PARAM(32) vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_transposeSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix33xd_determinantSIMD4(
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts a matrix containing only a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_fastInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_affineInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_invertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector3xd elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix33xd_inverseTransposeSIMD4(
	dsMatrix22d* result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a);

DS_SIMD_START(DS_SIMD_FLOAT4)

#if DS_X86
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD4f _temp0 = _mm_movelh_ps((elem0), (elem1)); \
		dsSIMD4f _temp1 = _mm_movehl_ps((elem1), (elem0)); \
		(elem0) = _mm_shuffle_ps(_temp0, (elem2), _MM_SHUFFLE(3, 0, 2, 0)); \
		(elem1) = _mm_shuffle_ps(_temp0, (elem2), _MM_SHUFFLE(3, 1, 3, 1)); \
		(elem2) = _mm_shuffle_ps(_temp1, (elem2), _MM_SHUFFLE(3, 2, 2, 0)); \
	} while (0)

#define DS_SIMD_TRANSPOSE_22(elem0, elem1) \
	do \
	{ \
		dsSIMD4f _temp0 = _mm_movelh_ps((elem0), (elem1)); \
		(elem0) = _mm_shuffle_ps(_temp0, (elem0), _MM_SHUFFLE(3, 2, 2, 0)); \
		(elem1) = _mm_shuffle_ps(_temp0, (elem1), _MM_SHUFFLE(3, 2, 3, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(3, 0, 2, 1)); \
		(second) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(3, 1, 0, 2)); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col01, det, a, b) \
	do \
	{ \
		dsSIMD4f _tempCol01 = _mm_shuffle_ps((b), (a), _MM_SHUFFLE(0, 1, 0, 1)); \
		dsSIMD4f _upperDetMul = dsSIMD4f_mul((a), _tempCol01); \
		(col01) = _mm_xor_ps(_tempCol01, dsSIMD4f_set4(0.0f, -0.0f, -0.0f, 0.0f)); \
		(det) = dsSIMD4f_sub(\
			dsSIMD4f_set1FromVec(_upperDetMul, 0), dsSIMD4f_set1FromVec(_upperDetMul, 1)); \
	} while (0)

#define DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		dsSIMD4f _col01; \
		DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(_col01, (det), (a), (b)); \
		(col0) = _mm_shuffle_ps(_col01, (a), _MM_SHUFFLE(3, 2, 2, 0)); \
		(col1) = _mm_shuffle_ps(_col01, (b), _MM_SHUFFLE(3, 2, 3, 1)); \
	} while (0)
#elif DS_ARM
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		float32x4x2_t _tmpAB = vtrnq_f32((elem0), (elem1)); \
		float32x4x2_t _tmpCD = vtrnq_f32((elem2), dsSIMD4f_set1(0.0f)); \
		\
		(elem0) = vcombine_f32(vget_low_f32(_tmpAB.val[0]), vget_low_f32(_tmpCD.val[0])); \
		(elem1) = vcombine_f32(vget_low_f32(_tmpAB.val[1]), vget_low_f32(_tmpCD.val[1])); \
		(elem2) = vcombine_f32(vget_high_f32(_tmpAB.val[0]), vget_high_f32(_tmpCD.val[0])); \
	} while (0)

#define DS_SIMD_TRANSPOSE_22(elem0, elem1) \
	do \
	{ \
		float32x2_t _tmpA = vget_low_f32((elem0)); \
		float32x2_t _tmpB = vget_low_f32((elem1)); \
		float32x2x2_t _tmpAB = vtrn_f32(_tmpA, _tmpB); \
		\
		(elem0) = vcombine_f32(_tmpAB.val[0], vdup_n_f32(0.0f)); \
		(elem1) = vcombine_f32(_tmpAB.val[1], vdup_n_f32(0.0f)); \
	} while (0)

#if DS_ARM_64
#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		float32x2_t _tmp01 = vget_low_f32((a)); \
		float32x2_t _tmp23 = vget_high_f32((a)); \
		float32x2_t _tmp10 = vrev64_f32(_tmp01); \
		(first) = vcombine_f32(vzip1_f32(_tmp10, _tmp23), _tmp01); \
		(second) = vcombine_f32(vzip1_f32(_tmp23, _tmp01), _tmp10); \
	} while (0)
#else
#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		float32x2_t _tmp01 = vget_low_f32((a)); \
		float32x2_t _tmp23 = vget_high_f32((a)); \
		float32x2_t _tmp10 = vrev64_f32(_tmp01); \
		float32x2x2_t _tmp1203 = vzip_f32(_tmp10, _tmp23); \
		float32x2x2_t _tmp2031 = vzip_f32(_tmp23, _tmp01); \
		(first) = vcombine_f32(_tmp1203.val[0], _tmp01); \
		(second) = vcombine_f32(_tmp2031.val[0], _tmp10); \
	} while (0)
#endif

#define DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a10 = vrev64_f32(_a01); \
		float32x2_t _b10 = vrev64_f32(vget_low_f32((b))); \
		float32x2_t _upperDetMul = vmul_f32(_a01, _b10); \
		float32x2_t _upperDet = vsub_f32( \
			vdup_lane_f32(_upperDetMul, 0), vdup_lane_f32(_upperDetMul, 1)); \
		(det) = vcombine_f32(_upperDet, _upperDet); \
		\
		float32x2x2_t _cols = vtrn_f32(_b10, _a10); \
		uint32x2_t _col0Neg = {0, 0x80000000}; \
		uint32x2_t _col1Neg = {0x80000000, 0}; \
		(col0) = vcombine_f32( \
			vreinterpret_f32_u32(veor_u32(vreinterpret_u32_f32(_cols.val[0]), _col0Neg)), \
			vdup_n_f32(0.0f)); \
		(col1) = vcombine_f32( \
			vreinterpret_f32_u32(veor_u32(vreinterpret_u32_f32(_cols.val[1]), _col1Neg)), \
			vdup_n_f32(0.0f)); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col01, det, a, b) \
	do \
	{ \
		float32x2_t _a01 = vget_low_f32((a)); \
		float32x2_t _a10 = vrev64_f32(_a01); \
		float32x2_t _b10 = vrev64_f32(vget_low_f32((b))); \
		float32x2_t _upperDetMul = vmul_f32(_a01, _b10); \
		float32x2_t _upperDet = vsub_f32( \
			vdup_lane_f32(_upperDetMul, 0), vdup_lane_f32(_upperDetMul, 1)); \
		(det) = vcombine_f32(_upperDet, _upperDet); \
		\
		uint32x4_t _col01Neg = {0, 0x80000000, 0x80000000, 0}; \
		(col01) = vreinterpretq_f32_u32( \
			veorq_u32(vreinterpretq_u32_f32(vcombine_f32(_b10, _a10)), _col01Neg)); \
	} while (0)
#else
#error Special matrix operations not implemented for this platform.
#endif

inline void dsMatrix33xf_mulSIMD(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	dsSIMD4f mul2 = dsSIMD4f_set1FromVec(b->columns[0].simd, 2);
	result->columns[0].simd = dsSIMD4f_add(dsSIMD4f_add(
		dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_mul(a->columns[2].simd, mul2));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[1].simd, 2);
	result->columns[1].simd = dsSIMD4f_add(dsSIMD4f_add(
		dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_mul(a->columns[2].simd, mul2));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[2].simd, 2);
	result->columns[2].simd = dsSIMD4f_add(dsSIMD4f_add(
		dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_mul(a->columns[2].simd, mul2));
}

inline void dsMatrix33xf_affineMulSIMD(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	result->columns[0].simd = dsSIMD4f_add(
		dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	result->columns[1].simd = dsSIMD4f_add(
		dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	result->columns[2].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		a->columns[2].simd);
}

inline void dsMatrix33xf_transformSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);

	result->simd = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(mat->columns[0].simd, x), dsSIMD4f_mul(mat->columns[1].simd, y)),
		dsSIMD4f_mul(mat->columns[2].simd, z));
}

inline void dsMatrix33xf_transformTransposedSIMD(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD4f row0 = dsSIMD4f_mul(mat->columns[0].simd, vec->simd);
	dsSIMD4f row1 = dsSIMD4f_mul(mat->columns[1].simd, vec->simd);
	dsSIMD4f row2 = dsSIMD4f_mul(mat->columns[2].simd, vec->simd);
	DS_SIMD_TRANSPOSE_33(row0, row1, row2);

	result->simd = dsSIMD4f_add(dsSIMD4f_add(row0, row1), row2);
}

inline void dsMatrix33xf_transposeSIMD(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);
}

inline float dsMatrix33xf_determinantSIMD(const dsMatrix33xf* a)
{
	DS_ASSERT(a);

	dsSIMD4f a012, b120, b201, c120, c201;
	a012 = a->columns[0].simd;
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1].simd);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2].simd);

	dsSIMD4f detBC = dsSIMD4f_sub(dsSIMD4f_mul(b120, c201), dsSIMD4f_mul(b201, c120));
	dsSIMD4f det = dsDot3SIMD4f(a012, detBC);
	return dsSIMD4f_get(det, 0);
}

inline void dsMatrix33xf_fastInvertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	DS_SIMD_TRANSPOSE_22(result->columns[0].simd, result->columns[1].simd);

	dsSIMD4f x = dsSIMD4f_set1FromVec(a->columns[2].simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(a->columns[2].simd, 1);
	dsSIMD4f z = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_sub(z, dsSIMD4f_add(dsSIMD4f_mul(result->columns[0].simd, x),
		dsSIMD4f_mul(result->columns[1].simd, y)));
}

inline void dsMatrix33xf_affineInvertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f det;
	DS_SIMD_INVERT_22_COMPONENTS(result->columns[0].simd, result->columns[1].simd, det,
		a->columns[0].simd, a->columns[1].simd);

	dsSIMD4f invDet = dsSIMD4f_rcp(det);
	result->columns[0].simd = dsSIMD4f_mul(result->columns[0].simd, invDet);
	result->columns[1].simd = dsSIMD4f_mul(result->columns[1].simd, invDet);

	dsSIMD4f x = dsSIMD4f_set1FromVec(a->columns[2].simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(a->columns[2].simd, 1);
	dsSIMD4f z = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_sub(z, dsSIMD4f_add(dsSIMD4f_mul(result->columns[0].simd, x),
		dsSIMD4f_mul(result->columns[1].simd, y)));
}

inline void dsMatrix33xf_invertSIMD(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f a012, a120, a201, b120, b201, c120, c201;
	a012 = a->columns[0].simd;
	DS_SIMD_SHUFFLE1_120_201(a120, a201, a->columns[0].simd);
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1].simd);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2].simd);

	dsSIMD4f detBC = dsSIMD4f_sub(dsSIMD4f_mul(b120, c201), dsSIMD4f_mul(b201, c120));
	dsSIMD4f invDet = dsSIMD4f_rcp(dsDot3SIMD4f(a012, detBC));

	result->columns[0].simd = dsSIMD4f_mul(detBC, invDet);
	result->columns[1].simd = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_mul(a201, c120), dsSIMD4f_mul(a120, c201)), invDet);
	result->columns[2].simd = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_mul(a120, b201), dsSIMD4f_mul(a201, b120)), invDet);
	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);
}

inline void dsMatrix33xf_inverseTransposeSIMD(dsMatrix22f* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f simdResult;
	dsSIMD4f det;
	DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(simdResult, det, a->columns[0].simd, a->columns[1].simd);

	dsSIMD4f invDet = dsSIMD4f_rcp(det);
	simdResult = dsSIMD4f_mul(simdResult, invDet);
	dsSIMD4f_storeUnaligned(result, simdResult);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

inline void dsMatrix33xf_mulFMA(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	dsSIMD4f mul2 = dsSIMD4f_set1FromVec(b->columns[0].simd, 2);
	result->columns[0].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1, dsSIMD4f_mul(a->columns[2].simd, mul2)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[1].simd, 2);
	result->columns[1].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1, dsSIMD4f_mul(a->columns[2].simd, mul2)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[2].simd, 2);
	result->columns[2].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1, dsSIMD4f_mul(a->columns[2].simd, mul2)));
}

inline void dsMatrix33xf_affineMulFMA(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	result->columns[0].simd = dsSIMD4f_fmadd(
		a->columns[0].simd, mul0, dsSIMD4f_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	result->columns[1].simd = dsSIMD4f_fmadd(
		a->columns[0].simd, mul0, dsSIMD4f_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	result->columns[2].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1, a->columns[2].simd));
}

inline void dsMatrix33xf_transformFMA(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);

	result->simd = dsSIMD4f_fmadd(mat->columns[0].simd, x, dsSIMD4f_fmadd(mat->columns[1].simd, y,
		dsSIMD4f_mul(mat->columns[2].simd, z)));
}

inline void dsMatrix33xf_transformTransposedFMA(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD4f row0 = mat->columns[0].simd;
	dsSIMD4f row1 = mat->columns[1].simd;
	dsSIMD4f row2 = mat->columns[2].simd;
	DS_SIMD_TRANSPOSE_33(row0, row1, row2);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);

	result->simd = dsSIMD4f_fmadd(row0, x, dsSIMD4f_fmadd(row1, y, dsSIMD4f_mul(row2, z)));
}

inline float dsMatrix33xf_determinantFMA(const dsMatrix33xf* a)
{
	DS_ASSERT(a);

	dsSIMD4f a012, b120, b201, c120, c201;
	a012 = a->columns[0].simd;
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1].simd);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2].simd);

	// Use nmadd rather than msub to reduce instructions on more platforms.
	dsSIMD4f detBC = dsSIMD4f_fnmadd(b201, c120, dsSIMD4f_mul(b120, c201));
	dsSIMD4f det = dsDot3FMA4f(a012, detBC);
	return dsSIMD4f_get(det, 0);
}

inline void dsMatrix33xf_fastInvertFMA(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	DS_SIMD_TRANSPOSE_22(result->columns[0].simd, result->columns[1].simd);

	dsSIMD4f x = dsSIMD4f_set1FromVec(a->columns[2].simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(a->columns[2].simd, 1);
	dsSIMD4f z = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_fnmadd(result->columns[0].simd, x,
		dsSIMD4f_fnmadd(result->columns[1].simd, y, z));
}

inline void dsMatrix33xf_affineInvertFMA(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f det;
	DS_SIMD_INVERT_22_COMPONENTS(result->columns[0].simd, result->columns[1].simd, det,
		a->columns[0].simd, a->columns[1].simd);

	dsSIMD4f invDet = dsSIMD4f_rcp(det);
	result->columns[0].simd = dsSIMD4f_mul(result->columns[0].simd, invDet);
	result->columns[1].simd = dsSIMD4f_mul(result->columns[1].simd, invDet);

	dsSIMD4f x = dsSIMD4f_set1FromVec(a->columns[2].simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(a->columns[2].simd, 1);
	dsSIMD4f z = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_fnmadd(result->columns[0].simd, x,
		dsSIMD4f_fnmadd(result->columns[1].simd, y, z));
}

inline void dsMatrix33xf_invertFMA(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f a012, a120, a201, b120, b201, c120, c201;
	a012 = a->columns[0].simd;
	DS_SIMD_SHUFFLE1_120_201(a120, a201, a->columns[0].simd);
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1].simd);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2].simd);

	// Use nmadd rather than msub to reduce instructions on more platforms.
	dsSIMD4f detBC = dsSIMD4f_fnmadd(b201, c120, dsSIMD4f_mul(b120, c201));
	dsSIMD4f invDet = dsSIMD4f_rcp(dsDot3FMA4f(a012, detBC));

	result->columns[0].simd = dsSIMD4f_mul(detBC, invDet);
	result->columns[1].simd = dsSIMD4f_mul(
		dsSIMD4f_fnmadd(a120, c201, dsSIMD4f_mul(a201, c120)), invDet);
	result->columns[2].simd = dsSIMD4f_mul(
		dsSIMD4f_fnmadd(a201, b120, dsSIMD4f_mul(a120, b201)), invDet);
	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_TRANSPOSE_22
#undef DS_SIMD_SHUFFLE1_120_201
#undef DS_SIMD_INVERT_22_COMPONENTS
#undef DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS

DS_SIMD_START(DS_SIMD_DOUBLE2)

#if DS_X86
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD2d_transpose((elem0).simd2[0], (elem1).simd2[0]); \
		dsSIMD2d _temp = _mm_unpacklo_pd((elem0).simd2[1], (elem1).simd2[1]); \
		(elem0).simd2[1] = _mm_unpacklo_pd((elem2).simd2[0], _mm_setzero_pd()); \
		(elem1).simd2[1] = _mm_unpackhi_pd((elem2).simd2[0], _mm_setzero_pd()); \
		(elem2).simd2[0] = _temp; \
	} while (0)

#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		(first).simd2[0] = _mm_shuffle_pd((a).simd2[0], (a).simd2[1], _MM_SHUFFLE2(0, 1)); \
		(first).simd2[1] = (a).simd2[0]; \
		(second).simd2[0] = _mm_shuffle_pd((a).simd2[1], (a).simd2[0], _MM_SHUFFLE2(0, 0)); \
		(second).simd2[1] = _mm_shuffle_pd((a).simd2[0], (a).simd2[0], _MM_SHUFFLE2(0, 1)); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		dsSIMD2d _a10 = _mm_shuffle_pd((a), (a), _MM_SHUFFLE2(0, 1)); \
		dsSIMD2d _b10 = _mm_shuffle_pd((b), (b), _MM_SHUFFLE2(0, 1)); \
		dsSIMD2d _upperDetMul = dsSIMD2d_mul((a), _b10); \
		(det) = dsSIMD2d_sub( \
			dsSIMD2d_set1FromVec(_upperDetMul, 0), dsSIMD2d_set1FromVec(_upperDetMul, 1)); \
		\
		(col0) = _mm_xor_pd(_b10, dsSIMD2d_set2(0.0, -0.0)); \
		(col1) = _mm_xor_pd(_a10, dsSIMD2d_set2(-0.0, 0.0)); \
	} while (0)
#elif DS_ARM_64
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD2d_transpose((elem0).simd2[0], (elem1).simd2[0]); \
		dsSIMD2d _temp = vtrn1q_f64((elem0).simd2[1], (elem1).simd2[1]); \
		(elem0).simd2[1] = vtrn1q_f64((elem2).simd2[0], dsSIMD2d_set1(0.0)); \
		(elem1).simd2[1] = vtrn2q_f64((elem2).simd2[0], dsSIMD2d_set1(0.0)); \
		(elem2).simd2[0] = _temp; \
	} while (0)

#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		dsSIMD2d _tmp10 = vextq_f64((a).simd2[0], (a).simd2[0], 1); \
		(first).simd2[0] = vzip1q_f64(_tmp10, (a).simd2[1]); \
		(first).simd2[1] = (a).simd2[0]; \
		(second).simd2[0] = vzip1q_f64((a).simd2[1], (a).simd2[0]); \
		(second).simd2[1] = _tmp10; \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		dsSIMD2d _a10 = vextq_f64((a), (a), 1); \
		dsSIMD2d _b10 = vextq_f64((b), (b), 1); \
		dsSIMD2d _upperDetMul = vmulq_f64((a), _b10); \
		(det) = vsubq_f64( \
			vdupq_laneq_f64(_upperDetMul, 0), vdupq_laneq_f64(_upperDetMul, 1)); \
		\
		uint64x2_t _col0Neg = {0, 0x8000000000000000ULL}; \
		uint64x2_t _col1Neg = {0x8000000000000000ULL, 0}; \
		(col0) = vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(_b10), _col0Neg)); \
		(col1) = vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(_a10), _col1Neg)); \
	} while (0)
#else
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(elem0); \
		DS_UNUSED(elem1); \
		DS_UNUSED(elem2); \
	} while (0)

#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(col0); \
		DS_UNUSED(col1); \
		DS_UNUSED(det); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)
#endif

#define DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS((col0), (col1), (det), (a), (b)); \
		dsSIMD2d_transpose((col0), (col1)); \
	} while (0)

inline void dsMatrix33xd_mulSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	dsSIMD2d mul2 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 0);
	result->columns[0].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2));
	result->columns[0].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 0);
	result->columns[1].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2));
	result->columns[1].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 0);
	result->columns[2].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2));
	result->columns[2].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2));
}

inline void dsMatrix33xd_affineMulSIMD2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	result->columns[0].simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1));
	result->columns[0].simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	result->columns[1].simd2[0] = dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1));
	result->columns[1].simd2[1] = dsSIMD2d_add(
		dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		a->columns[2].simd2[0]);
	result->columns[2].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		a->columns[2].simd2[1]);
}

inline void dsMatrix33xd_transformSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);

	result->simd2[0] = dsSIMD2d_add(dsSIMD2d_add(dsSIMD2d_mul(mat->columns[0].simd2[0], x),
		dsSIMD2d_mul(mat->columns[1].simd2[0], y)), dsSIMD2d_mul(mat->columns[2].simd2[0], z));
	result->simd2[1] = dsSIMD2d_add(dsSIMD2d_add(dsSIMD2d_mul(mat->columns[0].simd2[1], x),
		dsSIMD2d_mul(mat->columns[1].simd2[1], y)), dsSIMD2d_mul(mat->columns[2].simd2[1], z));
}

inline void dsMatrix33xd_transformTransposedSIMD2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsVector4d row0, row1, row2;
	row0.simd2[0] = dsSIMD2d_mul(mat->columns[0].simd2[0], vec->simd2[0]);
	row0.simd2[1] = dsSIMD2d_mul(mat->columns[0].simd2[1], vec->simd2[1]);
	row1.simd2[0] = dsSIMD2d_mul(mat->columns[1].simd2[0], vec->simd2[0]);
	row1.simd2[1] = dsSIMD2d_mul(mat->columns[1].simd2[1], vec->simd2[1]);
	row2.simd2[0] = dsSIMD2d_mul(mat->columns[2].simd2[0], vec->simd2[0]);
	row2.simd2[1] = dsSIMD2d_mul(mat->columns[2].simd2[1], vec->simd2[1]);
	DS_SIMD_TRANSPOSE_33(row0, row1, row2);

	result->simd2[0] = dsSIMD2d_add(dsSIMD2d_add(row0.simd2[0], row1.simd2[0]), row2.simd2[0]);
	result->simd2[1] = dsSIMD2d_add(dsSIMD2d_add(row0.simd2[1], row1.simd2[1]), row2.simd2[1]);
}

inline void dsMatrix33xd_transposeSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);
}

inline double dsMatrix33xd_determinantSIMD2(const dsMatrix33xd* a)
{
	DS_ASSERT(a);

	dsVector4d a012, b120, b201, c120, c201;
	a012 = a->columns[0];
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1]);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2]);

	dsVector4d detBC;
	detBC.simd2[0] = dsSIMD2d_sub(
		dsSIMD2d_mul(b120.simd2[0], c201.simd2[0]), dsSIMD2d_mul(b201.simd2[0], c120.simd2[0]));
	detBC.simd2[1] = dsSIMD2d_sub(
		dsSIMD2d_mul(b120.simd2[1], c201.simd2[1]), dsSIMD2d_mul(b201.simd2[1], c120.simd2[1]));
	dsSIMD2d det = dsDot3SIMD2d(a012.simd2[0], a012.simd2[1], detBC.simd2[0], detBC.simd2[1]);
	return dsSIMD2d_get(det, 0);
}

inline void dsMatrix33xd_fastInvertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	result->columns[0].simd2[0] = a->columns[0].simd2[0];
	result->columns[1].simd2[0] = a->columns[1].simd2[0];
	dsSIMD2d_transpose(result->columns[0].simd2[0], result->columns[1].simd2[0]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_neg(dsSIMD2d_add(dsSIMD2d_mul(
		result->columns[0].simd2[0], x), dsSIMD2d_mul(result->columns[1].simd2[0], y)));

	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
}

inline void dsMatrix33xd_affineInvertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD2d det;
	DS_SIMD_INVERT_22_COMPONENTS(result->columns[0].simd2[0], result->columns[1].simd2[0], det,
		a->columns[0].simd2[0], a->columns[1].simd2[0]);

	dsSIMD2d invDet = dsSIMD2d_rcp(det);
	result->columns[0].simd2[0] = dsSIMD2d_mul(result->columns[0].simd2[0], invDet);
	result->columns[1].simd2[0] = dsSIMD2d_mul(result->columns[1].simd2[0], invDet);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_neg(dsSIMD2d_add(dsSIMD2d_mul(
		result->columns[0].simd2[0], x), dsSIMD2d_mul(result->columns[1].simd2[0], y)));

	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
}

inline void dsMatrix33xd_invertSIMD2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsVector4d a012, a120, a201, b120, b201, c120, c201;
	a012 = a->columns[0];
	DS_SIMD_SHUFFLE1_120_201(a120, a201, a->columns[0]);
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1]);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2]);

	dsVector4d detBC;
	detBC.simd2[0] = dsSIMD2d_sub(
		dsSIMD2d_mul(b120.simd2[0], c201.simd2[0]), dsSIMD2d_mul(b201.simd2[0], c120.simd2[0]));
	detBC.simd2[1] = dsSIMD2d_sub(
		dsSIMD2d_mul(b120.simd2[1], c201.simd2[1]), dsSIMD2d_mul(b201.simd2[1], c120.simd2[1]));
	dsSIMD2d invDet = dsSIMD2d_rcp(
		dsDot3SIMD2d(a012.simd2[0], a012.simd2[1], detBC.simd2[0], detBC.simd2[1]));

	result->columns[0].simd2[0] = dsSIMD2d_mul(detBC.simd2[0], invDet);
	result->columns[0].simd2[1] = dsSIMD2d_mul(detBC.simd2[1], invDet);
	result->columns[1].simd2[0] = dsSIMD2d_mul(dsSIMD2d_sub(dsSIMD2d_mul(
		a201.simd2[0], c120.simd2[0]), dsSIMD2d_mul(a120.simd2[0], c201.simd2[0])), invDet);
	result->columns[1].simd2[1] = dsSIMD2d_mul(dsSIMD2d_sub(dsSIMD2d_mul(
		a201.simd2[1], c120.simd2[1]), dsSIMD2d_mul(a120.simd2[1], c201.simd2[1])), invDet);
	result->columns[2].simd2[0] = dsSIMD2d_mul(dsSIMD2d_sub(dsSIMD2d_mul(
		a120.simd2[0], b201.simd2[0]), dsSIMD2d_mul(a201.simd2[0], b120.simd2[0])), invDet);
	result->columns[2].simd2[1] = dsSIMD2d_mul(dsSIMD2d_sub(dsSIMD2d_mul(
		a120.simd2[1], b201.simd2[1]), dsSIMD2d_mul(a201.simd2[1], b120.simd2[1])), invDet);
	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);
}

inline void dsMatrix33xd_inverseTransposeSIMD2(dsMatrix22d* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD2d det;
	DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(result->columns[0].simd, result->columns[1].simd, det,
		a->columns[0].simd2[0], a->columns[1].simd2[0]);

	dsSIMD2d invDet = dsSIMD2d_rcp(det);
	result->columns[0].simd = dsSIMD2d_mul(result->columns[0].simd, invDet);
	result->columns[1].simd = dsSIMD2d_mul(result->columns[1].simd, invDet);
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

inline void dsMatrix33xd_mulFMA2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	dsSIMD2d mul2 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 0);
	result->columns[0].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1, dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[0].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1, dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 0);
	result->columns[1].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1, dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[1].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1, dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 0);
	result->columns[2].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1, dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[2].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1, dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));
}

inline void dsMatrix33xd_affineMulFMA2(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	result->columns[0].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_mul(a->columns[1].simd2[0], mul1));
	result->columns[0].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_mul(a->columns[1].simd2[1], mul1));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	result->columns[1].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_mul(a->columns[1].simd2[0], mul1));
	result->columns[1].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_mul(a->columns[1].simd2[1], mul1));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1, a->columns[2].simd2[0]));
	result->columns[2].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1, a->columns[2].simd2[1]));
}

inline void dsMatrix33xd_transformFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);

	result->simd2[0] = dsSIMD2d_fmadd(mat->columns[0].simd2[0], x,
		dsSIMD2d_fmadd(mat->columns[1].simd2[0], y, dsSIMD2d_mul(mat->columns[2].simd2[0], z)));
	result->simd2[1] = dsSIMD2d_fmadd(mat->columns[0].simd2[1], x,
		dsSIMD2d_fmadd(mat->columns[1].simd2[1], y, dsSIMD2d_mul(mat->columns[2].simd2[1], z)));
}

inline void dsMatrix33xd_transformTransposedFMA2(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsVector4d row0 = mat->columns[0];
	dsVector4d row1 = mat->columns[1];
	dsVector4d row2 = mat->columns[2];
	DS_SIMD_TRANSPOSE_33(row0, row1, row2);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);

	result->simd2[0] = dsSIMD2d_fmadd(row0.simd2[0], x,
		dsSIMD2d_fmadd(row1.simd2[0], y, dsSIMD2d_mul(row2.simd2[0], z)));
	result->simd2[1] = dsSIMD2d_fmadd(row0.simd2[1], x,
		dsSIMD2d_fmadd(row1.simd2[1], y, dsSIMD2d_mul(row2.simd2[1], z)));
}

inline double dsMatrix33xd_determinantFMA2(const dsMatrix33xd* a)
{
	DS_ASSERT(a);

	dsVector4d a012, b120, b201, c120, c201;
	a012 = a->columns[0];
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1]);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2]);

	dsVector4d detBC;
	// Use nmadd rather than msub to reduce instructions on more platforms.
	detBC.simd2[0] = dsSIMD2d_fnmadd(b201.simd2[0], c120.simd2[0],
		dsSIMD2d_mul(b120.simd2[0], c201.simd2[0]));
	detBC.simd2[1] = dsSIMD2d_fnmadd(b201.simd2[1], c120.simd2[1],
		dsSIMD2d_mul(b120.simd2[1], c201.simd2[1]));
	dsSIMD2d det = dsDot3FMA2d(a012.simd2[0], a012.simd2[1], detBC.simd2[0], detBC.simd2[1]);
	return dsSIMD2d_get(det, 0);
}

inline void dsMatrix33xd_fastInvertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	result->columns[0].simd2[0] = a->columns[0].simd2[0];
	result->columns[1].simd2[0] = a->columns[1].simd2[0];
	dsSIMD2d_transpose(result->columns[0].simd2[0], result->columns[1].simd2[0]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_fnmsub(result->columns[0].simd2[0], x,
		dsSIMD2d_mul(result->columns[1].simd2[0], y));

	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
}

inline void dsMatrix33xd_affineInvertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD2d det;
	DS_SIMD_INVERT_22_COMPONENTS(result->columns[0].simd2[0], result->columns[1].simd2[0], det,
		a->columns[0].simd2[0], a->columns[1].simd2[0]);

	dsSIMD2d invDet = dsSIMD2d_rcp(det);
	result->columns[0].simd2[0] = dsSIMD2d_mul(result->columns[0].simd2[0], invDet);
	result->columns[1].simd2[0] = dsSIMD2d_mul(result->columns[1].simd2[0], invDet);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[2].simd2[0], 1);
	result->columns[2].simd2[0] = dsSIMD2d_fnmsub(result->columns[0].simd2[0], x,
		dsSIMD2d_mul(result->columns[1].simd2[0], y));

	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
}

inline void dsMatrix33xd_invertFMA2(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsVector4d a012, a120, a201, b120, b201, c120, c201;
	a012 = a->columns[0];
	DS_SIMD_SHUFFLE1_120_201(a120, a201, a->columns[0]);
	DS_SIMD_SHUFFLE1_120_201(b120, b201, a->columns[1]);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, a->columns[2]);

	dsVector4d detBC;
	// Use nmadd rather than msub to reduce instructions on more platforms.
	detBC.simd2[0] = dsSIMD2d_fnmadd(b201.simd2[0], c120.simd2[0],
		dsSIMD2d_mul(b120.simd2[0], c201.simd2[0]));
	detBC.simd2[1] = dsSIMD2d_fnmadd(b201.simd2[1], c120.simd2[1],
		dsSIMD2d_mul(b120.simd2[1], c201.simd2[1]));
	dsSIMD2d invDet = dsSIMD2d_rcp(
		dsDot3FMA2d(a012.simd2[0], a012.simd2[1], detBC.simd2[0], detBC.simd2[1]));

	result->columns[0].simd2[0] = dsSIMD2d_mul(detBC.simd2[0], invDet);
	result->columns[0].simd2[1] = dsSIMD2d_mul(detBC.simd2[1], invDet);
	result->columns[1].simd2[0] = dsSIMD2d_mul(dsSIMD2d_fnmadd(a120.simd2[0], c201.simd2[0],
		dsSIMD2d_mul(a201.simd2[0], c120.simd2[0])), invDet);
	result->columns[1].simd2[1] = dsSIMD2d_mul(dsSIMD2d_fnmadd(a120.simd2[1], c201.simd2[1],
		dsSIMD2d_mul(a201.simd2[1], c120.simd2[1])), invDet);
	result->columns[2].simd2[0] = dsSIMD2d_mul(dsSIMD2d_fnmadd(a201.simd2[0], b120.simd2[0],
		dsSIMD2d_mul(a120.simd2[0], b201.simd2[0])), invDet);
	result->columns[2].simd2[1] = dsSIMD2d_mul(dsSIMD2d_fnmadd(a201.simd2[1], b120.simd2[1],
		dsSIMD2d_mul(a120.simd2[1], b201.simd2[1])), invDet);
	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_SHUFFLE1_120_201
#undef DS_SIMD_INVERT_22_COMPONENTS
#undef DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS

DS_SIMD_START(DS_SIMD_DOUBLE4)

#if DS_X86
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD4d _zero = _mm256_setzero_pd(); \
		dsSIMD4d _tmp0 = _mm256_unpacklo_pd((elem0), (elem1)); \
		dsSIMD4d _tmp1 = _mm256_unpacklo_pd((elem2), (_zero)); \
		dsSIMD4d _tmp2 = _mm256_unpackhi_pd((elem0), (elem1)); \
		dsSIMD4d _tmp3 = _mm256_unpackhi_pd((elem2), (_zero)); \
		(elem0) = _mm256_permute2f128_pd(_tmp0, _tmp1, _MM_SHUFFLE(0, 2, 0, 0)); \
		(elem1) = _mm256_permute2f128_pd(_tmp2, _tmp3, _MM_SHUFFLE(0, 2, 0, 0)); \
		(elem2) = _mm256_permute2f128_pd(_tmp0, _tmp1, _MM_SHUFFLE(0, 3, 0, 1)); \
	} while (0)

#define DS_SIMD_TRANSPOSE_22(elem0, elem1) \
	do \
	{ \
		dsSIMD4d _col0 = _mm256_unpacklo_pd((elem0), (elem1)); \
		dsSIMD4d _col1 = _mm256_unpackhi_pd((elem0), (elem1)); \
		(elem0) = _col0; \
		(elem1) = _col1; \
	} while (0)

#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		(first) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(3, 0, 2, 1)); \
		(second) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(3, 1, 0, 2)); \
	} while (0)

#define DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		dsSIMD4d _col0 = _mm256_permute_pd((b), 0x9); \
		dsSIMD4d _col1 = _mm256_permute_pd((a), 0x9); \
		dsSIMD4d _upperDetMul = dsSIMD4d_mul((a), _col0); \
		(det) = dsSIMD4d_sub( \
			dsSIMD4d_set1FromVec(_upperDetMul, 0), dsSIMD4d_set1FromVec(_upperDetMul, 1)); \
		\
		_col0 = _mm256_xor_pd(_col0, dsSIMD4d_set4(0.0, -0.0, 0.0, 0.0)); \
		_col1 = _mm256_xor_pd(_col1, dsSIMD4d_set4(-0.0, 0.0, 0.0, 0.0)); \
		(col0) = _mm256_unpacklo_pd(_col0, _col1); \
		(col1) = _mm256_unpackhi_pd(_col0, _col1); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col01, det, a, b) \
	do \
	{ \
		dsSIMD4d _col01 = _mm256_permute2f128_pd((a), (b), _MM_SHUFFLE(0, 0, 0, 2)); \
		_col01 = _mm256_permute_pd(_col01, 0x5); \
		dsSIMD4d _upperDetMul = dsSIMD4d_mul((a), _col01); \
		(col01) = _mm256_xor_pd(_col01, dsSIMD4d_set4(0.0, -0.0, -0.0, 0.0)); \
		(det) = dsSIMD4d_sub( \
			dsSIMD4d_set1FromVec(_upperDetMul, 0), dsSIMD4d_set1FromVec(_upperDetMul, 1)); \
	} while (0)
#else
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(elem0); \
		DS_UNUSED(elem1); \
		DS_UNUSED(elem2); \
	} while (0)

#define DS_SIMD_TRANSPOSE_22(elem0, elem1) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(elem0); \
		DS_UNUSED(elem1); \
	} while (0)

#if DS_ARM_64
#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)
#else
#define DS_SIMD_SHUFFLE1_120_201(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)
#endif

#define DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(col0); \
		DS_UNUSED(col1); \
		DS_UNUSED(det); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col01, det, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(col01); \
		DS_UNUSED(det); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)
#endif

inline void dsMatrix33xd_mulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	dsSIMD4d mulCol = dsSIMD4d_load(b->columns);
	dsSIMD4d mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	dsSIMD4d mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	dsSIMD4d mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns,
		dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1, dsSIMD4d_mul(col2, mul2))));
#else
	dsSIMD4d_store(result->columns, dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)), dsSIMD4d_mul(col2, mul2)));
#endif

	mulCol = dsSIMD4d_load(b->columns + 1);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 1,
		dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1, dsSIMD4d_mul(col2, mul2))));
#else
	dsSIMD4d_store(result->columns + 1, dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)), dsSIMD4d_mul(col2, mul2)));
#endif

	mulCol = dsSIMD4d_load(b->columns + 2);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 2,
		dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1, dsSIMD4d_mul(col2, mul2))));
#else
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)), dsSIMD4d_mul(col2, mul2)));
#endif
}

inline void dsMatrix33xd_affineMulSIMD4(dsMatrix33xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) a, const dsMatrix33xd* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	dsSIMD4d mulCol = dsSIMD4d_load(b->columns);
	dsSIMD4d mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	dsSIMD4d mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_mul(col1, mul1)));
#else
	dsSIMD4d_store(
		result->columns, dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)));
#endif

	mulCol = dsSIMD4d_load(b->columns + 1);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 1, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_mul(col1, mul1)));
#else
	dsSIMD4d_store(
		result->columns + 1, dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)));
#endif

	mulCol = dsSIMD4d_load(b->columns + 2);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(
		result->columns + 2, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1, col2)));
#else
	dsSIMD4d_store(result->columns + 2,
		dsSIMD4d_add(dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)), col2));
#endif
}

inline void dsMatrix33xd_transformSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) mat, const dsVector3xd* DS_ALIGN_PARAM(32) vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	dsSIMD4d col0 = dsSIMD4d_load(mat->columns);
	dsSIMD4d col1 = dsSIMD4d_load(mat->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(mat->columns + 2);

	dsSIMD4d simdVec = dsSIMD4d_load(vec);
	dsSIMD4d x = dsSIMD4d_set1FromVec(simdVec, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(simdVec, 1);
	dsSIMD4d z = dsSIMD4d_set1FromVec(simdVec, 2);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result, dsSIMD4d_fmadd(col0, x, dsSIMD4d_fmadd(col1, y, dsSIMD4d_mul(col2, z))));
#else
	dsSIMD4d_store(result, dsSIMD4d_add(dsSIMD4d_add(dsSIMD4d_mul(col0, x),
		dsSIMD4d_mul(col1, y)), dsSIMD4d_mul(col2, z)));
#endif
}

inline void dsMatrix33xd_transformTransposedSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsMatrix33xd* DS_ALIGN_PARAM(32) mat, const dsVector3xd* DS_ALIGN_PARAM(32) vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);

	// NOTE: Always use the non-FMA version because shuffling is much slower in practice for 256-bit
	// SIMD registers.
	dsSIMD4d simdVec = dsSIMD4d_load(vec);
	dsSIMD4d row0 = dsSIMD4d_mul(dsSIMD4d_load(mat->columns), simdVec);
	dsSIMD4d row1 = dsSIMD4d_mul(dsSIMD4d_load(mat->columns + 1), simdVec);
	dsSIMD4d row2 = dsSIMD4d_mul(dsSIMD4d_load(mat->columns + 2), simdVec);
	DS_SIMD_TRANSPOSE_33(row0, row1, row2);

	dsSIMD4d_store(result, dsSIMD4d_add(dsSIMD4d_add(row0, row1), row2));
}

inline void dsMatrix33xd_transposeSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	DS_SIMD_TRANSPOSE_33(col0, col1, col2);
	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
}

inline double dsMatrix33xd_determinantSIMD4(const dsMatrix33xd* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	dsSIMD4d a012, b120, b201, c120, c201;
	a012 = col0;
	DS_SIMD_SHUFFLE1_120_201(b120, b201, col1);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, col2);

#if DS_SIMD_ALWAYS_FMA
	// Use nmadd rather than msub to reduce instructions on more platforms.
	dsSIMD4d detBC = dsSIMD4d_fnmadd(b201, c120, dsSIMD4d_mul(b120, c201));
#else
	dsSIMD4d detBC = dsSIMD4d_sub(dsSIMD4d_mul(b120, c201), dsSIMD4d_mul(b201, c120));
#endif
	dsSIMD4d det = dsDot3SIMD4d(a012, detBC);
	return dsSIMD4d_get(det, 0);
}

inline void dsMatrix33xd_fastInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	DS_SIMD_TRANSPOSE_22(col0, col1);
	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);

	dsSIMD4d x = dsSIMD4d_set1FromVec(col2, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(col2, 1);
	dsSIMD4d z = dsSIMD4d_set4(0.0, 0.0, 1.0, 0.0);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_fnmadd(col0, x, dsSIMD4d_fnmadd(col1, y, z)));
#else
	dsSIMD4d_store(result->columns + 2,
		dsSIMD4d_sub(z, dsSIMD4d_add(dsSIMD4d_mul(col0, x), dsSIMD4d_mul(col1, y))));
#endif
}

inline void dsMatrix33xd_affineInvertSIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsMatrix33xd* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	dsSIMD4d det;
	DS_SIMD_INVERT_22_COMPONENTS(col0, col1, det, col0, col1);

	dsSIMD4d invDet = dsSIMD4d_rcp(det);
	col0 = dsSIMD4d_mul(col0, invDet);
	col1 = dsSIMD4d_mul(col1, invDet);
	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);

	dsSIMD4d x = dsSIMD4d_set1FromVec(col2, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(col2, 1);
	dsSIMD4d z = dsSIMD4d_set4(0.0, 0.0, 1.0, 0.0);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_fnmadd(col0, x, dsSIMD4d_fnmadd(col1, y, z)));
#else
	dsSIMD4d_store(result->columns + 2,
		dsSIMD4d_sub(z, dsSIMD4d_add(dsSIMD4d_mul(col0, x), dsSIMD4d_mul(col1, y))));
#endif
}

inline void dsMatrix33xd_invertSIMD4(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

	dsSIMD4d a012, a120, a201, b120, b201, c120, c201;
	a012 = col0;
	DS_SIMD_SHUFFLE1_120_201(a120, a201, col0);
	DS_SIMD_SHUFFLE1_120_201(b120, b201, col1);
	DS_SIMD_SHUFFLE1_120_201(c120, c201, col2);

#if DS_SIMD_ALWAYS_FMA
	// Use nmadd rather than msub to reduce instructions on more platforms.
	dsSIMD4d detBC = dsSIMD4d_fnmadd(b201, c120, dsSIMD4d_mul(b120, c201));
#else
	dsSIMD4d detBC = dsSIMD4d_sub(dsSIMD4d_mul(b120, c201), dsSIMD4d_mul(b201, c120));
#endif
	dsSIMD4d invDet = dsSIMD4d_rcp(dsDot3SIMD4d(a012, detBC));
	col0 = dsSIMD4d_mul(detBC, invDet);

#if DS_SIMD_ALWAYS_FMA
	col1 = dsSIMD4d_mul(dsSIMD4d_fnmadd(a120, c201, dsSIMD4d_mul(a201, c120)), invDet);
	col2 = dsSIMD4d_mul(dsSIMD4d_fnmadd(a201, b120, dsSIMD4d_mul(a120, b201)), invDet);
#else
	col1 = dsSIMD4d_mul(dsSIMD4d_sub(dsSIMD4d_mul(a201, c120), dsSIMD4d_mul(a120, c201)), invDet);
	col2 = dsSIMD4d_mul(dsSIMD4d_sub(dsSIMD4d_mul(a120, b201), dsSIMD4d_mul(a201, b120)), invDet);
#endif

	DS_SIMD_TRANSPOSE_33(col0, col1, col2);
	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
}

inline void dsMatrix33xd_inverseTransposeSIMD4(dsMatrix22d* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);

	dsSIMD4d col01;
	dsSIMD4d det;
	DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS(col01, det, col0, col1);

	dsSIMD4d_storeUnaligned(result, dsSIMD4d_mul(col01, dsSIMD4d_rcp(det)));
}

DS_SIMD_END()

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_TRANSPOSE_22
#undef DS_SIMD_SHUFFLE1_120_201
#undef DS_SIMD_INVERT_22_COMPONENTS
#undef DS_SIMD_INVERSE_TRANSPOSE_22_COMPONENTS

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
