/*
 * Copyright 2022-2026 Aaron Barany
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
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for performing matrix operations with SIMD.
 *
 * These will only be available if DS_HAS_SIMD is set to 1. dsMatrix44f functions will use the
 * fastest operations available at compile time, but these functions can be used directly when
 * checking for capabilities at runtime, preferably before a loop of many operations.
 *
 * Variations of the same function are only provided if there's a benefit. For example, there may
 * be no FMA version if the implementation would be the same as the SIMD version. This is also the
 * case for some Double4 variants that are heavy on swizzles, since swizzling support is highly
 * limited (due to the hardware treating each dsSIMD4d as two dsSIMD2d elements processed in
 * parallel) and would be slower to work around these limitations.
 */

#if DS_HAS_SIMD

/**
 * @brief Multiplies two matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44f_mulSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);

/**
 * @brief Multiplies two affine matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineMulSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);

/**
 * @brief Transforms a vector by a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44f_transformSIMD(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44f_transformTransposedSIMD(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44f_transposeSIMD(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline float dsMatrix44f_determinantSIMD(const dsMatrix44f* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_fastInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Inverts an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Inverts the upper 3x3 portion of an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineInvert33SIMD(
	dsVector4f result[3], const dsMatrix44f* a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_invertSIMD(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_inverseTransposeSIMD(
	dsVector4f result[3], const dsMatrix44f* a);

/**
 * @brief Decomposes the transform from a rigid transform matrix.
 *
 * This assumes that the matrix is an rigid transform. The matrix can be re-composed by applying the
 * scale, orientation, and position in that order.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] outPosition The position of the transform.
 * @param[out] outOrientation The orientation of the transform.
 * @param[out] outScale The scale of the transform.
 * @param matrix The matrix to extract the transform from.
 */
DS_MATH_EXPORT inline void dsMatrix44f_decomposeTransformSIMD(dsVector4f* outPosition,
	dsQuaternion4f* outOrientation, dsVector4f* outScale, const dsMatrix44f* matrix);

/**
 * @brief Composes a transform into a matrix.
 *
 * This applies the scale, followed by the orientation, followed by the position. As the matrix is
 * column-order, The multiplication order is reversed from the logical application of the
 * components.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The resulting matrix.
 * @param position The position of the transform. The z value should be 1.
 * @param orientation The orientation of the transform.
 * @param scale The scale of the transform.
 */
DS_MATH_EXPORT inline void dsMatrix44f_composeTransformSIMD(dsMatrix44f* result,
	const dsVector4f* position, const dsQuaternion4f* orientation, const dsVector3f* scale);

/**
 * @brief Linearly interpolates between two rigid transform matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The matrix for the result.
 * @param a The first transform matrix to enterpolate.
 * @param b The second transform matrix to interpolate.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsMatrix44f_rigidLerpSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b, float t);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44f_mulFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);

/**
 * @brief Multiplies two affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineMulFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);

/**
 * @brief Transforms a vector by a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44f_transformFMA(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);

/**
 * @brief Transforms a vector by a transposed matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44f_transformTransposedFMA(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline float dsMatrix44f_determinantFMA(const dsMatrix44f* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_fastInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Inverts an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineInvertFMA(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Inverts the upper 3x3 portion of an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_affineInvert33FMA(
	dsVector4f result[3], const dsMatrix44f* a);

/**
 * @brief Inverts a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_invertFMA(dsMatrix44f* result, const dsMatrix44f* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors
 *     using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44f_inverseTransposeFMA(
	dsVector4f result[3], const dsMatrix44f* a);

/**
 * @brief Linearly interpolates between two rigid transform matrices using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The matrix for the result.
 * @param a The first transform matrix to enterpolate.
 * @param b The second transform matrix to interpolate.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsMatrix44f_rigidLerpFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b, float t);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_mulSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);

/**
 * @brief Multiplies two affine matrices.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineMulSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);
/**
 * @brief Transforms a vector by a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformSIMD2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedSIMD2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transposeSIMD2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix44d_determinantSIMD2(const dsMatrix44d* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_fastInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Inverts an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Inverts the upper 3x3 portion of an affine matrix.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33SIMD2(
	dsVector4d result[3], const dsMatrix44d* a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_invertSIMD2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeSIMD2(
	dsVector4d result[3], const dsMatrix44d* a);

/**
 * @brief Decomposes the transform from a rigid transform matrix.
 *
 * This assumes that the matrix is an rigid transform. The matrix can be re-composed by applying the
 * scale, orientation, and position in that order.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] outPosition The position of the transform.
 * @param[out] outOrientation The orientation of the transform.
 * @param[out] outScale The scale of the transform.
 * @param matrix The matrix to extract the transform from.
 */
DS_MATH_EXPORT inline void dsMatrix44d_decomposeTransformSIMD2(dsVector4d* outPosition,
	dsQuaternion4d* outOrientation, dsVector4d* outScale, const dsMatrix44d* matrix);

/**
 * @brief Composes a transform into a matrix.
 *
 * This applies the scale, followed by the orientation, followed by the position. As the matrix is
 * column-order, The multiplication order is reversed from the logical application of the
 * components.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The resulting matrix.
 * @param position The position of the transform. The z value should be 1.
 * @param orientation The orientation of the transform.
 * @param scale The scale of the transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_composeTransformSIMD2(dsMatrix44d* result,
	const dsVector4d* position, const dsQuaternion4d* orientation, const dsVector3d* scale);

/**
 * @brief Linearly interpolates between two rigid transform matrices.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] result The matrix for the result.
 * @param a The first transform matrix to enterpolate.
 * @param b The second transform matrix to interpolate.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b, double t);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_mulFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);

/**
 * @brief Multiplies two affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineMulFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);

/**
 * @brief Transforms a vector by a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformFMA2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedFMA2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix44d_determinantFMA2(const dsMatrix44d* a);

/**
 * @brief Inverts a matrix containing only a rotation and translation using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_fastInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Inverts an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Inverts the upper 3x3 portion of an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33FMA2(
	dsVector4d result[3], const dsMatrix44d* a);

/**
 * @brief Inverts a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_invertFMA2(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors
 *     using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeFMA2(
	dsVector4d result[3], const dsMatrix44d* a);

/**
 * @brief Linearly interpolates between two rigid transform matrices using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA is available.
 * @param[out] result The matrix for the result.
 * @param a The first transform matrix to enterpolate.
 * @param b The second transform matrix to interpolate.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b, double t);

#endif // !DS_DETERMINISTIC_MATH

/**
 * @brief Multiplies two matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_mulSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b);

/**
 * @brief Multiplies two affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineMulSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b);

/**
 * @brief Transforms a vector by a matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformSIMD4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) mat, const dsVector4d* DS_ALIGN_PARAM(32) vec);

/**
 * @brief Transforms a vector by a transposed matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The transformed vector.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) mat,
	const dsVector4d* DS_ALIGN_PARAM(32) vec);

/**
 * @brief Transposes a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The transposed matrix.
 * @param a The matrix to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44d_transposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Computes the determinant of a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The matrix to compute the determinant from.
 * @return The determinant.
 */
DS_MATH_EXPORT inline double dsMatrix44d_determinantSIMD4(const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts a matrix containing only a rotation and translation using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_fastInvertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix. This must not be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts the upper 3x3 portion of an affine matrix using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33SIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Inverts a matrix.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] result The inverted matrix.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_invertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors
 *     using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The inverted matrix as 3 aligned dsVector4f elements.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a);

/**
 * @brief Decomposes the transform from a rigid transform matrix.
 *
 * This assumes that the matrix is an rigid transform. The matrix can be re-composed by applying the
 * scale, orientation, and position in that order.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] outPosition The position of the transform.
 * @param[out] outOrientation The orientation of the transform.
 * @param[out] outScale The scale of the transform.
 * @param matrix The matrix to extract the transform from.
 */
DS_MATH_EXPORT inline void dsMatrix44d_decomposeTransformSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) outPosition, dsQuaternion4d* outOrientation,
	dsVector4d* DS_ALIGN_PARAM(32) outScale, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix);

/**
 * @brief Composes a transform into a matrix.
 *
 * This applies the scale, followed by the orientation, followed by the position. As the matrix is
 * column-order, The multiplication order is reversed from the logical application of the
 * components.
 *
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The resulting matrix.
 * @param position The position of the transform. The z value should be 1.
 * @param orientation The orientation of the transform.
 * @param scale The scale of the transform.
 */
DS_MATH_EXPORT inline void dsMatrix44d_composeTransformSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsVector4d* DS_ALIGN_PARAM(32) position, const dsQuaternion4d* orientation,
	const dsVector3d* scale);

/**
 * @brief Linearly interpolates between two rigid transform matrices using fused multiply-add
 *     operations.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use FMA if not
 *     disabled through enabling determinisitic math.
 * @param[out] result The matrix for the result.
 * @param a The first transform matrix to enterpolate.
 * @param b The second transform matrix to interpolate.
 * @param t The interpolation value between a and b.
 */
DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b, double t);

DS_SIMD_START(DS_SIMD_FLOAT4)

#if DS_X86_32 || DS_X86_64
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD4f _temp0 = _mm_movelh_ps((elem0), (elem1)); \
		dsSIMD4f _temp1 = _mm_movehl_ps((elem1), (elem0)); \
		(elem0) = _mm_shuffle_ps(_temp0, (elem2), _MM_SHUFFLE(3, 0, 2, 0)); \
		(elem1) = _mm_shuffle_ps(_temp0, (elem2), _MM_SHUFFLE(3, 1, 3, 1)); \
		(elem2) = _mm_shuffle_ps(_temp1, (elem2), _MM_SHUFFLE(3, 2, 2, 0)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(2, 0, 2, 0)); \
		(second) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(3, 1, 3, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(1, 0, 1, 0)); \
		(second) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(3, 2, 3, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(1, 3, 1, 3)); \
		(second) = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(0, 2, 0, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(0, 0, 3, 3)); \
		(second) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(2, 2, 1, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0303_2121(first, second, a) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(3, 0, 3, 0)); \
		(second) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(1, 2, 1, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		(first) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(0, 3, 0, 3)); \
		(second) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(1, 2, 1, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		(result) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(2, 3, 0, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		(result) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(1, 0, 3, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		(result) = _mm_shuffle_ps((a), (a), _MM_SHUFFLE(3, 1, 2, 0)); \
	} while (0)
#elif DS_ARM_32 || DS_ARM_64
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

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		float32x2_t _lowA = vget_low_f32((a)); \
		float32x2_t _highA = vget_high_f32((a)); \
		float32x2_t _lowB = vget_low_f32((b)); \
		float32x2_t _highB = vget_high_f32((b)); \
		float32x4x2_t _evenOdd = vtrnq_f32(vcombine_f32(_lowA, _lowB), \
			vcombine_f32(_highA, _highB)); \
		(first) = _evenOdd.val[0]; \
		(second) = _evenOdd.val[1]; \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		float32x2_t _lowA = vget_low_f32((a)); \
		float32x2_t _highA = vget_high_f32((a)); \
		float32x2_t _lowB = vget_low_f32((b)); \
		float32x2_t _highB = vget_high_f32((b)); \
		(first) = vcombine_f32(_lowA, _lowB); \
		(second) = vcombine_f32(_highA, _highB); \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		float32x2_t _lowA = vget_low_f32((a)); \
		float32x2_t _highA = vget_high_f32((a)); \
		float32x2_t _lowB = vget_low_f32((b)); \
		float32x2_t _highB = vget_high_f32((b)); \
		float32x4x2_t _evenOdd = vtrnq_f32(vcombine_f32(_lowA, _lowB), \
			vcombine_f32(_highA, _highB)); \
		(first) = vrev64q_f32(_evenOdd.val[1]); \
		(second) = vrev64q_f32(_evenOdd.val[0]); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		float32x4x2_t _zipped = vzipq_f32((a), (a)); \
		(first) = vextq_f32(_zipped.val[1], _zipped.val[0], 2); \
		(second) = vextq_f32(_zipped.val[0], _zipped.val[1], 2); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0303_2121(first, second, a) \
	do \
	{ \
		dsSIMD4f _a0321 = vrev64q_f32(vextq_f32((a), (a), 3)); \
		float32x2_t _a03 = vget_low_f32(_a0321); \
		float32x2_t _a21 = vget_high_f32(_a0321); \
		(first) = vcombine_f32(_a03, _a03); \
		(second) = vcombine_f32(_a21, _a21); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		dsSIMD4f _a3021 = vextq_f32((a), (a), 3); \
		float32x2_t _a30 = vget_low_f32(_a3021); \
		float32x2_t _a21 = vrev64_f32(vget_high_f32(_a3021)); \
		(first) = vcombine_f32(_a30, _a30); \
		(second) = vcombine_f32(_a21, _a21); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		(result) = vrev64q_f32((a)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		(result) = vextq_f32((a), (a), 2); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		float32x2_t _low = vget_low_f32((a)); \
		float32x2_t _high = vget_high_f32((a)); \
		float32x2x2_t _transposed = vtrn_f32(_low, _high); \
		result = vcombine_f32(_transposed.val[0], _transposed.val[1]); \
	} while (0)
#else
#error Special matrix operations not implemented for this platform.
#endif

#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA1032, _tempB0303, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_0303_2121(_tempB0303, _tempB2121, (b)); \
		(result) = dsSIMD4f_add(dsSIMD4f_mul((a), _tempB0303), \
			dsSIMD4f_mul(_tempA1032, _tempB2121)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result) = dsSIMD4f_sub(dsSIMD4f_mul(_tempA3300, (b)), \
			dsSIMD4f_mul(_tempA1122, _tempB2301)); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result) = dsSIMD4f_sub(dsSIMD4f_mul((a), _tempB3030), \
			dsSIMD4f_mul(_tempA1032, _tempB2121)); \
	} while (0)

DS_MATH_EXPORT inline void dsMatrix44f_mulSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	dsSIMD4f mul2 = dsSIMD4f_set1FromVec(b->columns[0].simd, 2);
	dsSIMD4f mul3 = dsSIMD4f_set1FromVec(b->columns[0].simd, 3);
	result->columns[0].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, mul2), dsSIMD4f_mul(a->columns[3].simd, mul3)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[1].simd, 2);
	mul3 = dsSIMD4f_set1FromVec(b->columns[1].simd, 3);
	result->columns[1].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, mul2), dsSIMD4f_mul(a->columns[3].simd, mul3)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[2].simd, 2);
	mul3 = dsSIMD4f_set1FromVec(b->columns[2].simd, 3);
	result->columns[2].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, mul2), dsSIMD4f_mul(a->columns[3].simd, mul3)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[3].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[3].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[3].simd, 2);
	mul3 = dsSIMD4f_set1FromVec(b->columns[3].simd, 3);
	result->columns[3].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, mul2), dsSIMD4f_mul(a->columns[3].simd, mul3)));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineMulSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b)
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

	mul0 = dsSIMD4f_set1FromVec(b->columns[3].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[3].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[3].simd, 2);
	result->columns[3].simd = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, mul0), dsSIMD4f_mul(a->columns[1].simd, mul1)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, mul2), a->columns[3].simd));
}

DS_MATH_EXPORT inline void dsMatrix44f_transformSIMD(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);
	dsSIMD4f w = dsSIMD4f_set1FromVec(vec->simd, 3);

	result->simd = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(mat->columns[0].simd, x), dsSIMD4f_mul(mat->columns[1].simd, y)),
		dsSIMD4f_add(dsSIMD4f_mul(mat->columns[2].simd, z), dsSIMD4f_mul(mat->columns[3].simd, w)));
}

DS_MATH_EXPORT inline void dsMatrix44f_transformTransposedSIMD(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4f row0 = mat->columns[0].simd;
	dsSIMD4f row1 = mat->columns[1].simd;
	dsSIMD4f row2 = mat->columns[2].simd;
	dsSIMD4f row3 = mat->columns[3].simd;
	dsSIMD4f_transpose(row0, row1, row2, row3);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);
	dsSIMD4f w = dsSIMD4f_set1FromVec(vec->simd, 3);

	result->simd = dsSIMD4f_add(dsSIMD4f_add(dsSIMD4f_mul(row0, x), dsSIMD4f_mul(row1, y)),
		dsSIMD4f_add(dsSIMD4f_mul(row2, z), dsSIMD4f_mul(row3, w)));
}

DS_MATH_EXPORT inline void dsMatrix44f_transposeSIMD(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	result->columns[3] = a->columns[3];
	dsSIMD4f_transpose(result->columns[0].simd, result->columns[1].simd,
		result->columns[2].simd, result->columns[3].simd);
}

DS_MATH_EXPORT inline float dsMatrix44f_determinantSIMD(const dsMatrix44f* a)
{
	DS_ASSERT(a);

	dsSIMD4f a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0].simd, a->columns[1].simd);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2].simd, a->columns[3].simd);

	dsSIMD4f detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0].simd, a->columns[2].simd);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1].simd, a->columns[3].simd);

	dsVector4f det;
	det.simd = dsSIMD4f_sub(dsSIMD4f_mul(detA, detB), dsSIMD4f_mul(detC, detD));
	float det44 = det.x*det.w + det.y*det.z;

	dsSIMD4f ab, dc;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);

	dsSIMD4f dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);

	dsVector4f tr;
	tr.simd = dsSIMD4f_mul(ab, dc0213);
	return det44 - ((tr.x + tr.y) + (tr.z + tr.w));
}

DS_MATH_EXPORT inline void dsMatrix44f_fastInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);

	result->columns[3].simd = dsSIMD4f_sub(dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(result->columns[0].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 0)),
			dsSIMD4f_mul(result->columns[1].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 1))),
			dsSIMD4f_mul(result->columns[2].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 2))));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineInvertSIMD(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f scale2 = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, a->columns[0].simd),
			dsSIMD4f_mul(a->columns[1].simd, a->columns[1].simd)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, a->columns[2].simd),
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)));
	dsSIMD4f invScale2 = dsSIMD4f_rcp(scale2);

	result->columns[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result->columns[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result->columns[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);

	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);

	result->columns[3].simd = dsSIMD4f_sub(dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(result->columns[0].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 0)),
			dsSIMD4f_mul(result->columns[1].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 1))),
			dsSIMD4f_mul(result->columns[2].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 2))));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineInvert33SIMD(
	dsVector4f result[3], const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f scale2 = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, a->columns[0].simd),
			dsSIMD4f_mul(a->columns[1].simd, a->columns[1].simd)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, a->columns[2].simd),
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)));
	dsSIMD4f invScale2 = dsSIMD4f_rcp(scale2);

	result[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);

	DS_SIMD_TRANSPOSE_33(result[0].simd, result[1].simd, result[2].simd);
}

DS_MATH_EXPORT inline void dsMatrix44f_invertSIMD(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	dsSIMD4f a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0].simd, a->columns[1].simd);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2].simd, a->columns[3].simd);

	dsSIMD4f detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0].simd, a->columns[2].simd);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1].simd, a->columns[3].simd);

	dsSIMD4f det = dsSIMD4f_sub(dsSIMD4f_mul(detA, detB), dsSIMD4f_mul(detC, detD));
	detA = dsSIMD4f_set1FromVec(det, 0);
	detB = dsSIMD4f_set1FromVec(det, 1);
	detC = dsSIMD4f_set1FromVec(det, 2);
	detD = dsSIMD4f_set1FromVec(det, 3);

	dsSIMD4f det44 = dsSIMD4f_add(dsSIMD4f_mul(detA, detD), dsSIMD4f_mul(detB, detC));

	dsSIMD4f ab, dc, bdc, cab;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);
	DS_MATRIX22_MUL(bdc, b22, dc);
	DS_MATRIX22_MUL(cab, c22, ab);

	dsSIMD4f x = dsSIMD4f_sub(dsSIMD4f_mul(detD, a22), bdc);
	dsSIMD4f w = dsSIMD4f_sub(dsSIMD4f_mul(detA, d22), cab);

	dsSIMD4f dab, adc;
	DS_MATRIX22_MUL_ADJ(dab, d22, ab);
	DS_MATRIX22_MUL_ADJ(adc, a22, dc);

	dsSIMD4f y = dsSIMD4f_sub(dsSIMD4f_mul(detB, c22), dab);
	dsSIMD4f z = dsSIMD4f_sub(dsSIMD4f_mul(detC, b22), adc);

	dsSIMD4f dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);
	dsSIMD4f tr = dsSIMD4f_mul(ab, dc0213);
#if DS_SIMD_ALWAYS_HADD
	tr = dsSIMD4f_hadd(tr, tr);
	tr = dsSIMD4f_hadd(tr, tr);
#else
	dsSIMD4f trX = dsSIMD4f_set1FromVec(tr, 0);
	dsSIMD4f trY = dsSIMD4f_set1FromVec(tr, 1);
	dsSIMD4f trZ = dsSIMD4f_set1FromVec(tr, 2);
	dsSIMD4f trW = dsSIMD4f_set1FromVec(tr, 3);
	tr = dsSIMD4f_add(dsSIMD4f_add(trX, trY), dsSIMD4f_add(trZ, trW));
#endif
	det44 = dsSIMD4f_sub(det44, tr);

	dsSIMD4f sign = dsSIMD4f_set4(1.0f, -1.0f, -1.0f, 1.0f);
	dsSIMD4f invDet44 = dsSIMD4f_div(sign, det44);

	x = dsSIMD4f_mul(invDet44, x);
	y = dsSIMD4f_mul(invDet44, y);
	z = dsSIMD4f_mul(invDet44, z);
	w = dsSIMD4f_mul(invDet44, w);

	DS_SIMD_SHUFFLE2_3131_2020(result->columns[0].simd, result->columns[1].simd, x, y);
	DS_SIMD_SHUFFLE2_3131_2020(result->columns[2].simd, result->columns[3].simd, z, w);
}

DS_MATH_EXPORT inline void dsMatrix44f_inverseTransposeSIMD(
	dsVector4f result[3], const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f scale2 = dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[0].simd, a->columns[0].simd),
			dsSIMD4f_mul(a->columns[1].simd, a->columns[1].simd)),
		dsSIMD4f_add(
			dsSIMD4f_mul(a->columns[2].simd, a->columns[2].simd),
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)));
	dsSIMD4f invScale2 = dsSIMD4f_rcp(scale2);

	result[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);
}

DS_MATH_EXPORT inline void dsMatrix44f_decomposeTransformSIMD(dsVector4f* outPosition,
	dsQuaternion4f* outOrientation, dsVector4f* outScale, const dsMatrix44f* matrix)
{
	DS_ASSERT(outPosition);
	DS_ASSERT(outOrientation);
	DS_ASSERT(outScale);
	DS_ASSERT(matrix);

	outPosition->simd = matrix->columns[3].simd;

	dsVector4f dot;
	dot.simd = dsSIMD4f_mul(matrix->columns[0].simd, matrix->columns[0].simd);
	float len2x = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(matrix->columns[1].simd, matrix->columns[1].simd);
	float len2y = dot.x + dot.y + dot.z;
	dot.simd = dsSIMD4f_mul(matrix->columns[2].simd, matrix->columns[2].simd);
	float len2z = dot.x + dot.y + dot.z;
	outScale->simd = dsSIMD4f_sqrt(dsSIMD4f_set4(len2x, len2y, len2z, 1.0f));

	dsVector4f invScale;
	invScale.simd = dsSIMD4f_rcp(outScale->simd);

	dsMatrix44f rotateMat;
	rotateMat.columns[0].simd = dsSIMD4f_mul(
		matrix->columns[0].simd, dsSIMD4f_set1FromVec(invScale.simd, 0));
	rotateMat.columns[1].simd = dsSIMD4f_mul(
		matrix->columns[1].simd, dsSIMD4f_set1FromVec(invScale.simd, 1));
	rotateMat.columns[2].simd = dsSIMD4f_mul(
		matrix->columns[2].simd, dsSIMD4f_set1FromVec(invScale.simd, 2));
	dsQuaternion4f_fromMatrix44(outOrientation, &rotateMat);
}

DS_MATH_EXPORT inline void dsMatrix44f_composeTransformSIMD(dsMatrix44f* result,
	const dsVector4f* position, const dsQuaternion4f* orientation, const dsVector3f* scale)
{
	DS_ASSERT(result);
	DS_ASSERT(position);
	DS_ASSERT(orientation);
	DS_ASSERT(scale);

	dsQuaternion4f_toMatrix44(result, orientation);
	result->columns[0].simd = dsSIMD4f_mul(result->columns[0].simd, dsSIMD4f_set1(scale->x));
	result->columns[1].simd = dsSIMD4f_mul(result->columns[1].simd, dsSIMD4f_set1(scale->y));
	result->columns[2].simd = dsSIMD4f_mul(result->columns[2].simd, dsSIMD4f_set1(scale->z));
	result->columns[3].simd = position->simd;
}

DS_MATH_EXPORT inline void dsMatrix44f_rigidLerpSIMD(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsVector4f positionA, scaleA, positionB, scaleB, positionInterp, scaleInterp;
	dsQuaternion4f orientationA, orientationB, orientationInterp;

	dsMatrix44f_decomposeTransformSIMD(&positionA, &orientationA, &scaleA, a);
	dsMatrix44f_decomposeTransformSIMD(&positionB, &orientationB, &scaleB, b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	positionInterp.simd = dsSIMD4f_add(positionA.simd,
		dsSIMD4f_mul(t4, dsSIMD4f_sub(positionB.simd, positionA.simd)));
	scaleInterp.simd = dsSIMD4f_add(scaleA.simd,
		dsSIMD4f_mul(t4, dsSIMD4f_sub(scaleB.simd, scaleA.simd)));
	dsQuaternion4f_slerp(&orientationInterp, &orientationA, &orientationB, t);

	dsMatrix44f_composeTransformSIMD(
		result, &positionInterp, &orientationInterp, (dsVector3f*)&scaleInterp);
}

#undef DS_MATRIX22_MUL
#undef DS_MATRIX22_ADJ_MUL
#undef DS_MATRIX22_MUL_ADJ

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA,DS_SIMD_HADD)

#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA1032, _tempB0303, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_0303_2121(_tempB0303, _tempB2121, (b)); \
		(result) = dsSIMD4f_fmadd((a), _tempB0303,  dsSIMD4f_mul(_tempA1032, _tempB2121)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result) = dsSIMD4f_fmsub(_tempA3300, (b), dsSIMD4f_mul(_tempA1122, _tempB2301)); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsSIMD4f _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result) = dsSIMD4f_fmsub((a), _tempB3030, dsSIMD4f_mul(_tempA1032, _tempB2121));\
	} while (0)

DS_MATH_EXPORT inline void dsMatrix44f_mulFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4f mul0 = dsSIMD4f_set1FromVec(b->columns[0].simd, 0);
	dsSIMD4f mul1 = dsSIMD4f_set1FromVec(b->columns[0].simd, 1);
	dsSIMD4f mul2 = dsSIMD4f_set1FromVec(b->columns[0].simd, 2);
	dsSIMD4f mul3 = dsSIMD4f_set1FromVec(b->columns[0].simd, 3);
	result->columns[0].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_fmadd(a->columns[2].simd, mul2,
		dsSIMD4f_mul(a->columns[3].simd, mul3))));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);;
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);;
	mul2 = dsSIMD4f_set1FromVec(b->columns[1].simd, 2);;
	mul3 = dsSIMD4f_set1FromVec(b->columns[1].simd, 3);;
	result->columns[1].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_fmadd(a->columns[2].simd, mul2,
		dsSIMD4f_mul(a->columns[3].simd, mul3))));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[2].simd, 2);
	mul3 = dsSIMD4f_set1FromVec(b->columns[2].simd, 3);
	result->columns[2].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_fmadd(a->columns[2].simd, mul2,
		dsSIMD4f_mul(a->columns[3].simd, mul3))));

	mul0 = dsSIMD4f_set1FromVec(b->columns[3].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[3].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[3].simd, 2);
	mul3 = dsSIMD4f_set1FromVec(b->columns[3].simd, 3);
	result->columns[3].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_fmadd(a->columns[2].simd, mul2,
		dsSIMD4f_mul(a->columns[3].simd, mul3))));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineMulFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b)
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
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_mul(a->columns[2].simd, mul2)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[1].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[1].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[1].simd, 2);
	result->columns[1].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_mul(a->columns[2].simd, mul2)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[2].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[2].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[2].simd, 2);
	result->columns[2].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_mul(a->columns[2].simd, mul2)));

	mul0 = dsSIMD4f_set1FromVec(b->columns[3].simd, 0);
	mul1 = dsSIMD4f_set1FromVec(b->columns[3].simd, 1);
	mul2 = dsSIMD4f_set1FromVec(b->columns[3].simd, 2);
	result->columns[3].simd = dsSIMD4f_fmadd(a->columns[0].simd, mul0,
		dsSIMD4f_fmadd(a->columns[1].simd, mul1,
		dsSIMD4f_fmadd(a->columns[2].simd, mul2, a->columns[3].simd)));
}

DS_MATH_EXPORT inline void dsMatrix44f_transformFMA(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);
	dsSIMD4f w = dsSIMD4f_set1FromVec(vec->simd, 3);

	result->simd = dsSIMD4f_fmadd(mat->columns[0].simd, x,
		dsSIMD4f_fmadd(mat->columns[1].simd, y, dsSIMD4f_fmadd(mat->columns[2].simd, z,
		dsSIMD4f_mul(mat->columns[3].simd, w))));
}

DS_MATH_EXPORT inline void dsMatrix44f_transformTransposedFMA(
	dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4f row0 = mat->columns[0].simd;
	dsSIMD4f row1 = mat->columns[1].simd;
	dsSIMD4f row2 = mat->columns[2].simd;
	dsSIMD4f row3 = mat->columns[3].simd;
	dsSIMD4f_transpose(row0, row1, row2, row3);

	dsSIMD4f x = dsSIMD4f_set1FromVec(vec->simd, 0);
	dsSIMD4f y = dsSIMD4f_set1FromVec(vec->simd, 1);
	dsSIMD4f z = dsSIMD4f_set1FromVec(vec->simd, 2);
	dsSIMD4f w = dsSIMD4f_set1FromVec(vec->simd, 3);

	result->simd = dsSIMD4f_fmadd(row0, x, dsSIMD4f_fmadd(row1, y,
		dsSIMD4f_fmadd(row2, z, dsSIMD4f_mul(row3, w))));
}

DS_MATH_EXPORT inline float dsMatrix44f_determinantFMA(const dsMatrix44f* a)
{
	DS_ASSERT(a);

	dsSIMD4f a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0].simd, a->columns[1].simd);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2].simd, a->columns[3].simd);

	dsSIMD4f detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0].simd, a->columns[2].simd);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1].simd, a->columns[3].simd);

	dsVector4f det;
	det.simd = dsSIMD4f_fmsub(detA, detB, dsSIMD4f_mul(detC, detD));
	float det44 = det.x*det.w + det.y*det.z;

	dsSIMD4f ab, dc;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);

	dsSIMD4f dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);

	dsVector4f tr;
	tr.simd = dsSIMD4f_mul(ab, dc0213);
	return det44 - ((tr.x + tr.y) + (tr.z + tr.w));
}

DS_MATH_EXPORT inline void dsMatrix44f_fastInvertFMA(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);

	result->columns[3].simd =
		dsSIMD4f_fnmsub(result->columns[0].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 0),
		dsSIMD4f_fmadd(result->columns[1].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 1),
		dsSIMD4f_fmadd(result->columns[2].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 2),
		dsSIMD4f_set4(0.0f, 0.0f, 0.0f, -1.0f))));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineInvertFMA(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f invScale2 = dsSIMD4f_rcp(
		dsSIMD4f_fmadd(a->columns[0].simd, a->columns[0].simd,
		dsSIMD4f_fmadd(a->columns[1].simd, a->columns[1].simd,
		dsSIMD4f_fmadd(a->columns[2].simd, a->columns[2].simd,
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)))));

	result->columns[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result->columns[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result->columns[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);

	DS_SIMD_TRANSPOSE_33(result->columns[0].simd, result->columns[1].simd, result->columns[2].simd);

	result->columns[3].simd =
		dsSIMD4f_fnmsub(result->columns[0].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 0),
		dsSIMD4f_fmadd(result->columns[1].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 1),
		dsSIMD4f_fmadd(result->columns[2].simd, dsSIMD4f_set1FromVec(a->columns[3].simd, 2),
		dsSIMD4f_set4(0.0f, 0.0f, 0.0f, -1.0f))));
}

DS_MATH_EXPORT inline void dsMatrix44f_affineInvert33FMA(dsVector4f result[3], const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f invScale2 = dsSIMD4f_rcp(
		dsSIMD4f_fmadd(a->columns[0].simd, a->columns[0].simd,
		dsSIMD4f_fmadd(a->columns[1].simd, a->columns[1].simd,
		dsSIMD4f_fmadd(a->columns[2].simd, a->columns[2].simd,
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)))));

	result[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);

	DS_SIMD_TRANSPOSE_33(result[0].simd, result[1].simd, result[2].simd);
}

DS_MATH_EXPORT inline void dsMatrix44f_invertFMA(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	dsSIMD4f a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0].simd, a->columns[1].simd);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2].simd, a->columns[3].simd);

	dsSIMD4f detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0].simd, a->columns[2].simd);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1].simd, a->columns[3].simd);

	dsSIMD4f det = dsSIMD4f_fmsub(detA, detB, dsSIMD4f_mul(detC, detD));
	detA = dsSIMD4f_set1FromVec(det, 0);
	detB = dsSIMD4f_set1FromVec(det, 1);
	detC = dsSIMD4f_set1FromVec(det, 2);
	detD = dsSIMD4f_set1FromVec(det, 3);

	dsSIMD4f det44 = dsSIMD4f_fmadd(detA, detD, dsSIMD4f_mul(detB, detC));

	dsSIMD4f ab, dc, bdc, cab;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);
	DS_MATRIX22_MUL(bdc, b22, dc);
	DS_MATRIX22_MUL(cab, c22, ab);

	dsSIMD4f x = dsSIMD4f_fmsub(detD, a22, bdc);
	dsSIMD4f w = dsSIMD4f_fmsub(detA, d22, cab);

	dsSIMD4f dab, adc;
	DS_MATRIX22_MUL_ADJ(dab, d22, ab);
	DS_MATRIX22_MUL_ADJ(adc, a22, dc);

	dsSIMD4f y = dsSIMD4f_fmsub(detB, c22, dab);
	dsSIMD4f z = dsSIMD4f_fmsub(detC, b22, adc);

	dsSIMD4f dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);
	dsSIMD4f tr = dsSIMD4f_mul(ab, dc0213);
	tr = dsSIMD4f_hadd(tr, tr);
	tr = dsSIMD4f_hadd(tr, tr);
	det44 = dsSIMD4f_sub(det44, tr);

	dsSIMD4f sign = dsSIMD4f_set4(1.0f, -1.0f, -1.0f, 1.0f);
	dsSIMD4f invDet44 = dsSIMD4f_div(sign, det44);

	x = dsSIMD4f_mul(invDet44, x);
	y = dsSIMD4f_mul(invDet44, y);
	z = dsSIMD4f_mul(invDet44, z);
	w = dsSIMD4f_mul(invDet44, w);

	DS_SIMD_SHUFFLE2_3131_2020(result->columns[0].simd, result->columns[1].simd, x, y);
	DS_SIMD_SHUFFLE2_3131_2020(result->columns[2].simd, result->columns[3].simd, z, w);
}

DS_MATH_EXPORT inline void dsMatrix44f_inverseTransposeFMA(
	dsVector4f result[3], const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4f invScale2 = dsSIMD4f_rcp(
		dsSIMD4f_fmadd(a->columns[0].simd, a->columns[0].simd,
		dsSIMD4f_fmadd(a->columns[1].simd, a->columns[1].simd,
		dsSIMD4f_fmadd(a->columns[2].simd, a->columns[2].simd,
			dsSIMD4f_set4(0.0f, 0.0f, 0.0f, 1.0f)))));

	result[0].simd = dsSIMD4f_mul(a->columns[0].simd, invScale2);
	result[1].simd = dsSIMD4f_mul(a->columns[1].simd, invScale2);
	result[2].simd = dsSIMD4f_mul(a->columns[2].simd, invScale2);
}

DS_MATH_EXPORT inline void dsMatrix44f_rigidLerpFMA(
	dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsVector4f positionA, scaleA, positionB, scaleB, positionInterp, scaleInterp;
	dsQuaternion4f orientationA, orientationB, orientationInterp;

	dsMatrix44f_decomposeTransformSIMD(&positionA, &orientationA, &scaleA, a);
	dsMatrix44f_decomposeTransformSIMD(&positionB, &orientationB, &scaleB, b);

	dsSIMD4f t4 = dsSIMD4f_set1(t);
	positionInterp.simd = dsSIMD4f_fmadd(
		t4, dsSIMD4f_sub(positionB.simd, positionA.simd), positionA.simd);
	scaleInterp.simd = dsSIMD4f_fmadd(t4, dsSIMD4f_sub(scaleB.simd, scaleA.simd), scaleA.simd);
	dsQuaternion4f_slerp(&orientationInterp, &orientationA, &orientationB, t);

	dsMatrix44f_composeTransformSIMD(
		result, &positionInterp, &orientationInterp, (dsVector3f*)&scaleInterp);
}

#undef DS_MATRIX22_MUL
#undef DS_MATRIX22_ADJ_MUL
#undef DS_MATRIX22_MUL_ADJ

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_SHUFFLE2_0202_1313
#undef DS_SIMD_SHUFFLE2_0101_2323
#undef DS_SIMD_SHUFFLE2_3131_2020
#undef DS_SIMD_SHUFFLE1_3300_1122
#undef DS_SIMD_SHUFFLE1_0303_2121
#undef DS_SIMD_SHUFFLE1_3030_2121
#undef DS_SIMD_SHUFFLE1_1032
#undef DS_SIMD_SHUFFLE1_2301
#undef DS_SIMD_SHUFFLE1_0213

DS_SIMD_START(DS_SIMD_DOUBLE2)

#if DS_X86_32 || DS_X86_64
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		dsSIMD2d_transpose((elem0).simd2[0], (elem1).simd2[0]); \
		dsSIMD2d _temp = _mm_unpacklo_pd((elem0).simd2[1], (elem1).simd2[1]); \
		(elem0).simd2[1] = _mm_unpacklo_pd((elem2).simd2[0], _mm_setzero_pd()); \
		(elem1).simd2[1] = _mm_unpackhi_pd((elem2).simd2[0], _mm_setzero_pd()); \
		(elem2).simd2[0] = _temp; \
	} while (0)

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = _mm_unpacklo_pd((a).simd2[0], (a).simd2[1]); \
		(first).simd2[1] = _mm_unpacklo_pd((b).simd2[0], (b).simd2[1]); \
		(second).simd2[0] = _mm_unpackhi_pd((a).simd2[0], (a).simd2[1]); \
		(second).simd2[1] = _mm_unpackhi_pd((b).simd2[0], (b).simd2[1]); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = (a).simd2[0]; \
		(first).simd2[1] = (b).simd2[0]; \
		(second).simd2[0] = (a).simd2[1]; \
		(second).simd2[1] = (b).simd2[1]; \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = _mm_unpackhi_pd((a).simd2[1], (a).simd2[0]); \
		(first).simd2[1] = _mm_unpackhi_pd((b).simd2[1], (b).simd2[0]); \
		(second).simd2[0] = _mm_unpacklo_pd((a).simd2[1], (a).simd2[0]); \
		(second).simd2[1] = _mm_unpacklo_pd((b).simd2[1], (b).simd2[0]); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		(first).simd2[0] = _mm_unpackhi_pd((a).simd2[1], (a).simd2[1]); \
		(first).simd2[1] = _mm_unpacklo_pd((a).simd2[0], (a).simd2[0]); \
		(second).simd2[0] = _mm_unpackhi_pd((a).simd2[0], (a).simd2[0]); \
		(second).simd2[1] = _mm_unpacklo_pd((a).simd2[1], (a).simd2[1]); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		(first).simd2[0] = (first).simd2[1] = \
			_mm_shuffle_pd((a).simd2[1], (a).simd2[0], 0x1); \
		(second).simd2[0] = (second).simd2[1] = \
			_mm_shuffle_pd((a).simd2[1], (a).simd2[0], 0x2); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		(result).simd2[0] = _mm_shuffle_pd((a).simd2[0], (a).simd2[0], 0x1); \
		(result).simd2[1] = _mm_shuffle_pd((a).simd2[1], (a).simd2[1], 0x1); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		(result).simd2[0] = (a).simd2[1]; \
		(result).simd2[1] = (a).simd2[0]; \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		(result).simd2[0] = _mm_unpacklo_pd((a).simd2[0], (a).simd2[1]); \
		(result).simd2[1] = _mm_unpackhi_pd((a).simd2[0], (a).simd2[1]); \
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

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = vtrn1q_f64((a).simd2[0], (a).simd2[1]); \
		(first).simd2[1] = vtrn1q_f64((b).simd2[0], (b).simd2[1]); \
		(second).simd2[0] = vtrn2q_f64((a).simd2[0], (a).simd2[1]); \
		(second).simd2[1] = vtrn2q_f64((b).simd2[0], (b).simd2[1]); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = (a).simd2[0]; \
		(first).simd2[1] = (b).simd2[0]; \
		(second).simd2[0] = (a).simd2[1]; \
		(second).simd2[1] = (b).simd2[1]; \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		(first).simd2[0] = vtrn2q_f64((a).simd2[1], (a).simd2[0]); \
		(first).simd2[1] = vtrn2q_f64((b).simd2[1], (b).simd2[0]); \
		(second).simd2[0] = vtrn1q_f64((a).simd2[1], (a).simd2[0]); \
		(second).simd2[1] = vtrn1q_f64((b).simd2[1], (b).simd2[0]); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		(first).simd2[0] = vtrn2q_f64((a).simd2[1], (a).simd2[1]); \
		(first).simd2[1] = vtrn1q_f64((a).simd2[0], (a).simd2[0]); \
		(second).simd2[0] = vtrn2q_f64((a).simd2[0], (a).simd2[0]); \
		(second).simd2[1] = vtrn1q_f64((a).simd2[1], (a).simd2[1]); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		(first).simd2[0] = (first).simd2[1] = vextq_f64((a).simd2[1], (a).simd2[0], 1); \
		(second).simd2[0] = vextq_f64((a).simd2[0], (a).simd2[1], 1); \
		(second).simd2[0] = (second).simd2[1] = \
			vextq_f64((second).simd2[0], (second).simd2[0], 1); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		(result).simd2[0] = vextq_f64((a).simd2[0], (a).simd2[0], 1); \
		(result).simd2[1] = vextq_f64((a).simd2[1], (a).simd2[1], 1); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		(result).simd2[0] = (a).simd2[1]; \
		(result).simd2[1] = (a).simd2[0]; \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		(result).simd2[0] = vtrn1q_f64((a).simd2[0], (a).simd2[1]); \
		(result).simd2[1] = vtrn2q_f64((a).simd2[0], (a).simd2[1]); \
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

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)
#endif

#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD2d _mul0 = dsSIMD2d_set1FromVec((a).simd2[0], 0); \
		dsSIMD2d _mul1 = dsSIMD2d_set1FromVec((a).simd2[0], 1); \
		(result).simd2[0] = dsSIMD2d_add(dsSIMD2d_mul((b).simd2[0], _mul0), \
			dsSIMD2d_mul((b).simd2[1], _mul1)); \
		_mul0 = dsSIMD2d_set1FromVec((a).simd2[1], 0); \
		_mul1 = dsSIMD2d_set1FromVec((a).simd2[1], 1); \
		(result).simd2[1] = dsSIMD2d_add(dsSIMD2d_mul((b).simd2[0], _mul0), \
			dsSIMD2d_mul((b).simd2[1], _mul1)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsVector4d _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result).simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(_tempA3300.simd2[0], (b).simd2[0]), \
			dsSIMD2d_mul(_tempA1122.simd2[0], _tempB2301.simd2[0])); \
		(result).simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(_tempA3300.simd2[1], (b).simd2[1]), \
			dsSIMD2d_mul(_tempA1122.simd2[1], _tempB2301.simd2[1])); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsVector4d _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result).simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul((a).simd2[0], _tempB3030.simd2[0]), \
			dsSIMD2d_mul(_tempA1032.simd2[0], _tempB2121.simd2[0])); \
		(result).simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul((a).simd2[1], _tempB3030.simd2[1]), \
			dsSIMD2d_mul(_tempA1032.simd2[1], _tempB2121.simd2[1])); \
	} while (0)

DS_MATH_EXPORT inline void dsMatrix44d_mulSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	dsSIMD2d mul2 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 0);
	dsSIMD2d mul3 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 1);
	result->columns[0].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[0], mul2), dsSIMD2d_mul(a->columns[3].simd2[0], mul3)));
	result->columns[0].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], mul2), dsSIMD2d_mul(a->columns[3].simd2[1], mul3)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 1);
	result->columns[1].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[0], mul2), dsSIMD2d_mul(a->columns[3].simd2[0], mul3)));
	result->columns[1].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], mul2), dsSIMD2d_mul(a->columns[3].simd2[1], mul3)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 1);
	result->columns[2].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[0], mul2), dsSIMD2d_mul(a->columns[3].simd2[0], mul3)));
	result->columns[2].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], mul2), dsSIMD2d_mul(a->columns[3].simd2[1], mul3)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 1);
	result->columns[3].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[0], mul2), dsSIMD2d_mul(a->columns[3].simd2[0], mul3)));
	result->columns[3].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], mul2), dsSIMD2d_mul(a->columns[3].simd2[1], mul3)));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineMulSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b)
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

	mul0 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], mul0), dsSIMD2d_mul(a->columns[1].simd2[0], mul1)),
		dsSIMD2d_add(dsSIMD2d_mul(a->columns[2].simd2[0], mul2), a->columns[3].simd2[0]));
	result->columns[3].simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], mul0), dsSIMD2d_mul(a->columns[1].simd2[1], mul1)),
		dsSIMD2d_add(dsSIMD2d_mul(a->columns[2].simd2[1], mul2), a->columns[3].simd2[1]));
}

DS_MATH_EXPORT inline void dsMatrix44d_transformSIMD2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);
	dsSIMD2d w = dsSIMD2d_set1FromVec(vec->simd2[1], 1);

	result->simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(mat->columns[0].simd2[0], x), dsSIMD2d_mul(mat->columns[1].simd2[0], y)),
		dsSIMD2d_add(dsSIMD2d_mul(mat->columns[2].simd2[0], z),
			dsSIMD2d_mul(mat->columns[3].simd2[0], w)));
	result->simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(mat->columns[0].simd2[1], x), dsSIMD2d_mul(mat->columns[1].simd2[1], y)),
		dsSIMD2d_add(dsSIMD2d_mul(mat->columns[2].simd2[1], z),
			dsSIMD2d_mul(mat->columns[3].simd2[1], w)));
}

DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedSIMD2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);
	dsSIMD2d w = dsSIMD2d_set1FromVec(vec->simd2[1], 1);

	dsVector4d row0, row1, row2, row3;
	row0.simd2[0] = mat->columns[0].simd2[0];
	row1.simd2[0] = mat->columns[1].simd2[0];
	dsSIMD2d_transpose(row0.simd2[0], row1.simd2[0]);

	row0.simd2[1] = mat->columns[2].simd2[0];
	row1.simd2[1] = mat->columns[3].simd2[0];
	dsSIMD2d_transpose(row0.simd2[1], row1.simd2[1]);

	row2.simd2[0] = mat->columns[0].simd2[1];
	row3.simd2[0] = mat->columns[1].simd2[1];
	dsSIMD2d_transpose(row2.simd2[0], row3.simd2[0]);

	row2.simd2[1] = mat->columns[2].simd2[1];
	row3.simd2[1] = mat->columns[3].simd2[1];
	dsSIMD2d_transpose(row2.simd2[1], row3.simd2[1]);

	result->simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(row0.simd2[0], x), dsSIMD2d_mul(row1.simd2[0], y)),
		dsSIMD2d_add(dsSIMD2d_mul(row2.simd2[0], z), dsSIMD2d_mul(row3.simd2[0], w)));
	result->simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(row0.simd2[1], x), dsSIMD2d_mul(row1.simd2[1], y)),
		dsSIMD2d_add(dsSIMD2d_mul(row2.simd2[1], z), dsSIMD2d_mul(row3.simd2[1], w)));
}

DS_MATH_EXPORT inline void dsMatrix44d_transposeSIMD2(dsMatrix44d* result, const dsMatrix44d* a)
{
	result->columns[0].simd2[0] = a->columns[0].simd2[0];
	result->columns[1].simd2[0] = a->columns[1].simd2[0];
	dsSIMD2d_transpose(result->columns[0].simd2[0], result->columns[1].simd2[0]);

	result->columns[0].simd2[1] = a->columns[2].simd2[0];
	result->columns[1].simd2[1] = a->columns[3].simd2[0];
	dsSIMD2d_transpose(result->columns[0].simd2[1], result->columns[1].simd2[1]);

	result->columns[2].simd2[0] = a->columns[0].simd2[1];
	result->columns[3].simd2[0] = a->columns[1].simd2[1];
	dsSIMD2d_transpose(result->columns[2].simd2[0], result->columns[3].simd2[0]);

	result->columns[2].simd2[1] = a->columns[2].simd2[1];
	result->columns[3].simd2[1] = a->columns[3].simd2[1];
	dsSIMD2d_transpose(result->columns[2].simd2[1], result->columns[3].simd2[1]);
}

DS_MATH_EXPORT inline double dsMatrix44d_determinantSIMD2(const dsMatrix44d* a)
{
	DS_ASSERT(a);

	dsVector4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0], a->columns[1]);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2], a->columns[3]);

	dsVector4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0], a->columns[2]);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1], a->columns[3]);

	dsVector4d det;
	det.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detA.simd2[0], detB.simd2[0]),
		dsSIMD2d_mul(detC.simd2[0], detD.simd2[0]));
	det.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detA.simd2[1], detB.simd2[1]),
		dsSIMD2d_mul(detC.simd2[1], detD.simd2[1]));
	double det44 = det.x*det.w + det.y*det.z;

	dsVector4d ab, dc;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);

	dsVector4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);

	dsVector4d tr;
	tr.simd2[0] = dsSIMD2d_mul(ab.simd2[0], dc0213.simd2[0]);
	tr.simd2[1] = dsSIMD2d_mul(ab.simd2[1], dc0213.simd2[1]);
	return det44 - ((tr.x + tr.y) + (tr.z + tr.w));
}

DS_MATH_EXPORT inline void dsMatrix44d_fastInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(a->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_neg(dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(result->columns[0].simd2[0], x),
		dsSIMD2d_mul(result->columns[1].simd2[0], y)),
		dsSIMD2d_mul(result->columns[2].simd2[0], z)));
	result->columns[3].simd2[1] = dsSIMD2d_sub(dsSIMD2d_set2(0.0, 1.0), dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(result->columns[0].simd2[1], x),
		dsSIMD2d_mul(result->columns[1].simd2[1], y)),
		dsSIMD2d_mul(result->columns[2].simd2[1], z)));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvertSIMD2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], a->columns[0].simd2[0]),
			dsSIMD2d_mul(a->columns[1].simd2[0], a->columns[1].simd2[0])),
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0]));
	scale2.simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], a->columns[0].simd2[1]),
			dsSIMD2d_mul(a->columns[1].simd2[1], a->columns[1].simd2[1])),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], a->columns[2].simd2[1]), dsSIMD2d_set2(0.0, 1.0)));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result->columns[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result->columns[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result->columns[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result->columns[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result->columns[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result->columns[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);

	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(a->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_neg(dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(result->columns[0].simd2[0], x),
		dsSIMD2d_mul(result->columns[1].simd2[0], y)),
		dsSIMD2d_mul(result->columns[2].simd2[0], z)));
	result->columns[3].simd2[1] = dsSIMD2d_sub(dsSIMD2d_set2(0.0, 1.0), dsSIMD2d_add(dsSIMD2d_add(
		dsSIMD2d_mul(result->columns[0].simd2[1], x),
		dsSIMD2d_mul(result->columns[1].simd2[1], y)),
		dsSIMD2d_mul(result->columns[2].simd2[1], z)));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33SIMD2(
	dsVector4d result[3], const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], a->columns[0].simd2[0]),
			dsSIMD2d_mul(a->columns[1].simd2[0], a->columns[1].simd2[0])),
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0]));
	scale2.simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], a->columns[0].simd2[1]),
			dsSIMD2d_mul(a->columns[1].simd2[1], a->columns[1].simd2[1])),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], a->columns[2].simd2[1]), dsSIMD2d_set2(0.0, 1.0)));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);

	DS_SIMD_TRANSPOSE_33(result[0], result[1], result[2]);
}

DS_MATH_EXPORT inline void dsMatrix44d_invertSIMD2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	dsVector4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0], a->columns[1]);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2], a->columns[3]);

	dsVector4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0], a->columns[2]);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1], a->columns[3]);

	dsVector4d det;
	det.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detA.simd2[0], detB.simd2[0]),
		dsSIMD2d_mul(detC.simd2[0], detD.simd2[0]));
	det.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detA.simd2[1], detB.simd2[1]),
		dsSIMD2d_mul(detC.simd2[1], detD.simd2[1]));
	dsSIMD2d detA2 = dsSIMD2d_set1FromVec(det.simd2[0], 0);
	dsSIMD2d detB2 = dsSIMD2d_set1FromVec(det.simd2[0], 1);
	dsSIMD2d detC2 = dsSIMD2d_set1FromVec(det.simd2[1], 0);
	dsSIMD2d detD2 = dsSIMD2d_set1FromVec(det.simd2[1], 1);

	dsSIMD2d det44 = dsSIMD2d_add(dsSIMD2d_mul(detA2, detD2), dsSIMD2d_mul(detB2, detC2));

	dsVector4d ab, dc, bdc, cab;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);
	DS_MATRIX22_MUL(bdc, b22, dc);
	DS_MATRIX22_MUL(cab, c22, ab);

	dsVector4d x;
	x.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detD2, a22.simd2[0]), bdc.simd2[0]);
	x.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detD2, a22.simd2[1]), bdc.simd2[1]);
	dsVector4d w;
	w.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detA2, d22.simd2[0]), cab.simd2[0]);
	w.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detA2, d22.simd2[1]), cab.simd2[1]);

	dsVector4d dab, adc;
	DS_MATRIX22_MUL_ADJ(dab, d22, ab);
	DS_MATRIX22_MUL_ADJ(adc, a22, dc);

	dsVector4d y;
	y.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detB2, c22.simd2[0]), dab.simd2[0]);
	y.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detB2, c22.simd2[1]), dab.simd2[1]);
	dsVector4d z;
	z.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detC2, b22.simd2[0]), adc.simd2[0]);
	z.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detC2, b22.simd2[1]), adc.simd2[1]);

	dsVector4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);
	dsVector4d tr;
	tr.simd2[0] = dsSIMD2d_mul(ab.simd2[0], dc0213.simd2[0]);
	tr.simd2[1] = dsSIMD2d_mul(ab.simd2[1], dc0213.simd2[1]);
#if DS_SIMD_ALWAYS_HADD
	tr.simd2[0] = dsSIMD2d_hadd(tr.simd2[0], tr.simd2[1]);
	tr.simd2[0] = dsSIMD2d_hadd(tr.simd2[0], tr.simd2[0]);
#else
	tr.simd2[0] = dsSIMD2d_add(
		dsSIMD2d_set1FromVec(tr.simd2[0], 0), dsSIMD2d_set1FromVec(tr.simd2[0], 1));
	tr.simd2[1] = dsSIMD2d_add(
		dsSIMD2d_set1FromVec(tr.simd2[1], 0), dsSIMD2d_set1FromVec(tr.simd2[1], 1));
	tr.simd2[0] = dsSIMD2d_add(tr.simd2[0], tr.simd2[1]);
#endif
	det44 = dsSIMD2d_sub(det44, tr.simd2[0]);

	dsSIMD2d sign = dsSIMD2d_set2(1.0, -1.0);
	dsVector4d invDet44;
	invDet44.simd2[0] = dsSIMD2d_div(sign, det44);
	invDet44.simd2[1] = dsSIMD2d_neg(invDet44.simd2[0]);

	x.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], x.simd2[0]);
	x.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], x.simd2[1]);
	y.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], y.simd2[0]);
	y.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], y.simd2[1]);
	z.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], z.simd2[0]);
	z.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], z.simd2[1]);
	w.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], w.simd2[0]);
	w.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], w.simd2[1]);

	DS_SIMD_SHUFFLE2_3131_2020(result->columns[0], result->columns[1], x, y);
	DS_SIMD_SHUFFLE2_3131_2020(result->columns[2], result->columns[3], z, w);
}

DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeSIMD2(
	dsVector4d result[3], const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[0], a->columns[0].simd2[0]),
			dsSIMD2d_mul(a->columns[1].simd2[0], a->columns[1].simd2[0])),
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0]));
	scale2.simd2[1] = dsSIMD2d_add(dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[0].simd2[1], a->columns[0].simd2[1]),
			dsSIMD2d_mul(a->columns[1].simd2[1], a->columns[1].simd2[1])),
		dsSIMD2d_add(
			dsSIMD2d_mul(a->columns[2].simd2[1], a->columns[2].simd2[1]), dsSIMD2d_set2(0.0, 1.0)));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);
}

DS_MATH_EXPORT inline void dsMatrix44d_decomposeTransformSIMD2(dsVector4d* outPosition,
	dsQuaternion4d* outOrientation, dsVector4d* outScale, const dsMatrix44d* matrix)
{
	DS_ASSERT(outPosition);
	DS_ASSERT(outOrientation);
	DS_ASSERT(outScale);
	DS_ASSERT(matrix);

	outPosition->simd2[0] = matrix->columns[3].simd2[0];
	outPosition->simd2[1] = matrix->columns[3].simd2[1];

	dsVector4d dot;
	dot.simd2[0] = dsSIMD2d_mul(matrix->columns[0].simd2[0], matrix->columns[0].simd2[0]);
	dot.simd2[1] = dsSIMD2d_mul(matrix->columns[0].simd2[1], matrix->columns[0].simd2[1]);
	double len2x = dot.x + dot.y + dot.z;
	dot.simd2[0] = dsSIMD2d_mul(matrix->columns[1].simd2[0], matrix->columns[1].simd2[0]);
	dot.simd2[1] = dsSIMD2d_mul(matrix->columns[1].simd2[1], matrix->columns[1].simd2[1]);
	double len2y = dot.x + dot.y + dot.z;
	dot.simd2[0] = dsSIMD2d_mul(matrix->columns[2].simd2[0], matrix->columns[2].simd2[0]);
	dot.simd2[1] = dsSIMD2d_mul(matrix->columns[2].simd2[1], matrix->columns[2].simd2[1]);
	double len2z = dot.x + dot.y + dot.z;
	outScale->simd2[0] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2x, len2y));
	outScale->simd2[1] = dsSIMD2d_sqrt(dsSIMD2d_set2(len2z, 1.0f));

	dsVector4d invScale;
	invScale.simd2[0] = dsSIMD2d_rcp(outScale->simd2[0]);
	invScale.simd2[1] = dsSIMD2d_rcp(outScale->simd2[1]);
	dsSIMD2d invScaleX = dsSIMD2d_set1FromVec(invScale.simd2[0], 0);
	dsSIMD2d invScaleY = dsSIMD2d_set1FromVec(invScale.simd2[0], 1);
	dsSIMD2d invScaleZ = dsSIMD2d_set1FromVec(invScale.simd2[1], 0);

	dsMatrix44d rotateMat;
	rotateMat.columns[0].simd2[0] = dsSIMD2d_mul(matrix->columns[0].simd2[0], invScaleX);
	rotateMat.columns[0].simd2[1] = dsSIMD2d_mul(matrix->columns[0].simd2[1], invScaleX);
	rotateMat.columns[1].simd2[0] = dsSIMD2d_mul(matrix->columns[1].simd2[0], invScaleY);
	rotateMat.columns[1].simd2[1] = dsSIMD2d_mul(matrix->columns[1].simd2[1], invScaleY);
	rotateMat.columns[2].simd2[0] = dsSIMD2d_mul(matrix->columns[2].simd2[0], invScaleZ);
	rotateMat.columns[2].simd2[1] = dsSIMD2d_mul(matrix->columns[2].simd2[1], invScaleZ);
	dsQuaternion4d_fromMatrix44(outOrientation, &rotateMat);
}

DS_MATH_EXPORT inline void dsMatrix44d_composeTransformSIMD2(dsMatrix44d* result,
	const dsVector4d* position, const dsQuaternion4d* orientation, const dsVector3d* scale)
{
	DS_ASSERT(result);
	DS_ASSERT(position);
	DS_ASSERT(orientation);
	DS_ASSERT(scale);

	dsQuaternion4d_toMatrix44(result, orientation);
	dsSIMD2d scale1 = dsSIMD2d_set1(scale->x);
	result->columns[0].simd2[0] = dsSIMD2d_mul(result->columns[0].simd2[0], scale1);
	result->columns[0].simd2[1] = dsSIMD2d_mul(result->columns[0].simd2[1], scale1);
	scale1 = dsSIMD2d_set1(scale->y);
	result->columns[1].simd2[0] = dsSIMD2d_mul(result->columns[1].simd2[0], scale1);
	result->columns[1].simd2[1] = dsSIMD2d_mul(result->columns[1].simd2[1], scale1);
	scale1 = dsSIMD2d_set1(scale->z);
	result->columns[2].simd2[0] = dsSIMD2d_mul(result->columns[2].simd2[0], scale1);
	result->columns[2].simd2[1] = dsSIMD2d_mul(result->columns[2].simd2[1], scale1);
	result->columns[3].simd2[0] = position->simd2[0];
	result->columns[3].simd2[1] = position->simd2[1];
}

DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpSIMD2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsVector4d positionA, scaleA, positionB, scaleB, positionInterp, scaleInterp;
	dsQuaternion4d orientationA, orientationB, orientationInterp;

	dsMatrix44d_decomposeTransformSIMD2(&positionA, &orientationA, &scaleA, a);
	dsMatrix44d_decomposeTransformSIMD2(&positionB, &orientationB, &scaleB, b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	positionInterp.simd2[0] = dsSIMD2d_add(positionA.simd2[0],
		dsSIMD2d_mul(t2, dsSIMD2d_sub(positionB.simd2[0], positionA.simd2[0])));
	positionInterp.simd2[1] = dsSIMD2d_add(positionA.simd2[1],
		dsSIMD2d_mul(t2, dsSIMD2d_sub(positionB.simd2[1], positionA.simd2[1])));
	scaleInterp.simd2[0] = dsSIMD2d_add(scaleA.simd2[0],
		dsSIMD2d_mul(t2, dsSIMD2d_sub(scaleB.simd2[0], scaleA.simd2[0])));
	scaleInterp.simd2[1] = dsSIMD2d_add(scaleA.simd2[1],
		dsSIMD2d_mul(t2, dsSIMD2d_sub(scaleB.simd2[1], scaleA.simd2[1])));
	dsQuaternion4d_slerp(&orientationInterp, &orientationA, &orientationB, t);

	dsMatrix44d_composeTransformSIMD2(
		result, &positionInterp, &orientationInterp, (dsVector3d*)&scaleInterp);
}

#undef DS_MATRIX22_MUL
#undef DS_MATRIX22_ADJ_MUL
#undef DS_MATRIX22_MUL_ADJ

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD2d _mul0 = dsSIMD2d_set1FromVec((a).simd2[0], 0); \
		dsSIMD2d _mul1 = dsSIMD2d_set1FromVec((a).simd2[0], 1); \
		(result).simd2[0] = dsSIMD2d_fmadd((b).simd2[0], _mul0, \
			dsSIMD2d_mul((b).simd2[1], _mul1)); \
		_mul0 = dsSIMD2d_set1FromVec((a).simd2[1], 0); \
		_mul1 = dsSIMD2d_set1FromVec((a).simd2[1], 1); \
		(result).simd2[1] = dsSIMD2d_fmadd((b).simd2[0], _mul0, \
			dsSIMD2d_mul((b).simd2[1], _mul1)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsVector4d _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result).simd2[0] = dsSIMD2d_fmsub(_tempA3300.simd2[0], (b).simd2[0], \
			dsSIMD2d_mul(_tempA1122.simd2[0], _tempB2301.simd2[0])); \
		(result).simd2[1] = dsSIMD2d_fmsub(_tempA3300.simd2[1], (b).simd2[1], \
			dsSIMD2d_mul(_tempA1122.simd2[1], _tempB2301.simd2[1])); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsVector4d _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result).simd2[0] = dsSIMD2d_fmsub((a).simd2[0], _tempB3030.simd2[0], \
			dsSIMD2d_mul(_tempA1032.simd2[0], _tempB2121.simd2[0])); \
		(result).simd2[1] = dsSIMD2d_fmsub((a).simd2[1], _tempB3030.simd2[1], \
			dsSIMD2d_mul(_tempA1032.simd2[1], _tempB2121.simd2[1])); \
	} while (0)

DS_MATH_EXPORT inline void dsMatrix44d_mulFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD2d mul0 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 0);
	dsSIMD2d mul1 = dsSIMD2d_set1FromVec(b->columns[0].simd2[0], 1);
	dsSIMD2d mul2 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 0);
	dsSIMD2d mul3 = dsSIMD2d_set1FromVec(b->columns[0].simd2[1], 1);
	result->columns[0].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[0], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[0], mul3))));
	result->columns[0].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[1], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[1], mul3))));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 1);
	result->columns[1].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[0], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[0], mul3))));
	result->columns[1].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[1], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[1], mul3))));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 1);
	result->columns[2].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[0], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[0], mul3))));
	result->columns[2].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[1], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[1], mul3))));

	mul0 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 0);
	mul3 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 1);
	result->columns[3].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[0], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[0], mul3))));
	result->columns[3].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[1], mul2,
		dsSIMD2d_mul(a->columns[3].simd2[1], mul3))));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineMulFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b)
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
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[0].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[1].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[1].simd2[1], 0);
	result->columns[1].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[1].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[2].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[2].simd2[1], 0);
	result->columns[2].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[0], mul2)));
	result->columns[2].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_mul(a->columns[2].simd2[1], mul2)));

	mul0 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 0);
	mul1 = dsSIMD2d_set1FromVec(b->columns[3].simd2[0], 1);
	mul2 = dsSIMD2d_set1FromVec(b->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[0], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[0], mul2, a->columns[3].simd2[0])));
	result->columns[3].simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], mul0,
		dsSIMD2d_fmadd(a->columns[1].simd2[1], mul1,
		dsSIMD2d_fmadd(a->columns[2].simd2[1], mul2, a->columns[3].simd2[1])));
}

DS_MATH_EXPORT inline void dsMatrix44d_transformFMA2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);
	dsSIMD2d w = dsSIMD2d_set1FromVec(vec->simd2[1], 1);

	result->simd2[0] = dsSIMD2d_fmadd(mat->columns[0].simd2[0], x,
		dsSIMD2d_fmadd(mat->columns[1].simd2[0], y, dsSIMD2d_fmadd(mat->columns[2].simd2[0], z,
			dsSIMD2d_mul(mat->columns[3].simd2[0], w))));
	result->simd2[1] = dsSIMD2d_fmadd(mat->columns[0].simd2[1], x,
		dsSIMD2d_fmadd(mat->columns[1].simd2[1], y, dsSIMD2d_fmadd(mat->columns[2].simd2[1], z,
			dsSIMD2d_mul(mat->columns[3].simd2[1], w))));
}

DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedFMA2(
	dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD2d x = dsSIMD2d_set1FromVec(vec->simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(vec->simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(vec->simd2[1], 0);
	dsSIMD2d w = dsSIMD2d_set1FromVec(vec->simd2[1], 1);

	dsVector4d row0, row1, row2, row3;
	row0.simd2[0] = mat->columns[0].simd2[0];
	row1.simd2[0] = mat->columns[1].simd2[0];
	dsSIMD2d_transpose(row0.simd2[0], row1.simd2[0]);

	row0.simd2[1] = mat->columns[2].simd2[0];
	row1.simd2[1] = mat->columns[3].simd2[0];
	dsSIMD2d_transpose(row0.simd2[1], row1.simd2[1]);

	row2.simd2[0] = mat->columns[0].simd2[1];
	row3.simd2[0] = mat->columns[1].simd2[1];
	dsSIMD2d_transpose(row2.simd2[0], row3.simd2[0]);

	row2.simd2[1] = mat->columns[2].simd2[1];
	row3.simd2[1] = mat->columns[3].simd2[1];
	dsSIMD2d_transpose(row2.simd2[1], row3.simd2[1]);

	result->simd2[0] = dsSIMD2d_fmadd(row0.simd2[0], x, dsSIMD2d_fmadd(row1.simd2[0], y,
		dsSIMD2d_fmadd(row2.simd2[0], z, dsSIMD2d_mul(row3.simd2[0], w))));
	result->simd2[1] = dsSIMD2d_fmadd(row0.simd2[1], x, dsSIMD2d_fmadd(row1.simd2[1], y,
		dsSIMD2d_fmadd(row2.simd2[1], z, dsSIMD2d_mul(row3.simd2[1], w))));
}

DS_MATH_EXPORT inline double dsMatrix44d_determinantFMA2(const dsMatrix44d* a)
{
	DS_ASSERT(a);

	dsVector4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0], a->columns[1]);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2], a->columns[3]);

	dsVector4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0], a->columns[2]);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1], a->columns[3]);

	dsVector4d det;
	det.simd2[0] = dsSIMD2d_fmsub(detA.simd2[0], detB.simd2[0],
		dsSIMD2d_mul(detC.simd2[0], detD.simd2[0]));
	det.simd2[1] = dsSIMD2d_fmsub(detA.simd2[1], detB.simd2[1],
		dsSIMD2d_mul(detC.simd2[1], detD.simd2[1]));
	double det44 = det.x*det.w + det.y*det.z;

	dsVector4d ab, dc;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);

	dsVector4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);

	dsVector4d tr;
	tr.simd2[0] = dsSIMD2d_mul(ab.simd2[0], dc0213.simd2[0]);
	tr.simd2[1] = dsSIMD2d_mul(ab.simd2[1], dc0213.simd2[1]);
	return det44 - ((tr.x + tr.y) + (tr.z + tr.w));
}

DS_MATH_EXPORT inline void dsMatrix44d_fastInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	result->columns[2] = a->columns[2];
	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(a->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_neg(dsSIMD2d_fmadd(result->columns[0].simd2[0], x,
			dsSIMD2d_fmadd(result->columns[1].simd2[0], y,
			dsSIMD2d_mul(result->columns[2].simd2[0], z))));
	result->columns[3].simd2[1] = dsSIMD2d_fnmsub(result->columns[0].simd2[1], x,
		dsSIMD2d_fmadd(result->columns[1].simd2[1], y,
		dsSIMD2d_fmadd(result->columns[2].simd2[1], z, dsSIMD2d_set2(0.0, -1.0))));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvertFMA2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], a->columns[0].simd2[0],
		dsSIMD2d_fmadd(a->columns[1].simd2[0], a->columns[1].simd2[0],
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0])));
	scale2.simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], a->columns[0].simd2[1],
		dsSIMD2d_fmadd(a->columns[1].simd2[1], a->columns[1].simd2[1],
		dsSIMD2d_fmadd(a->columns[2].simd2[1], a->columns[2].simd2[1], dsSIMD2d_set2(0.0, 1.0))));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result->columns[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result->columns[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result->columns[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result->columns[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result->columns[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result->columns[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);

	DS_SIMD_TRANSPOSE_33(result->columns[0], result->columns[1], result->columns[2]);

	dsSIMD2d x = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 0);
	dsSIMD2d y = dsSIMD2d_set1FromVec(a->columns[3].simd2[0], 1);
	dsSIMD2d z = dsSIMD2d_set1FromVec(a->columns[3].simd2[1], 0);
	result->columns[3].simd2[0] = dsSIMD2d_neg(
		dsSIMD2d_fmadd(result->columns[0].simd2[0], x,
		dsSIMD2d_fmadd(result->columns[1].simd2[0], y,
		dsSIMD2d_mul(result->columns[2].simd2[0], z))));
	result->columns[3].simd2[1] =
		dsSIMD2d_fnmsub(result->columns[0].simd2[1], x,
		dsSIMD2d_fmadd(result->columns[1].simd2[1], y,
		dsSIMD2d_fmadd(result->columns[2].simd2[1], z, dsSIMD2d_set2(0.0, -1.0))));
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33FMA2(
	dsVector4d result[3], const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], a->columns[0].simd2[0],
		dsSIMD2d_fmadd(a->columns[1].simd2[0], a->columns[1].simd2[0],
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0])));
	scale2.simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], a->columns[0].simd2[1],
		dsSIMD2d_fmadd(a->columns[1].simd2[1], a->columns[1].simd2[1],
		dsSIMD2d_fmadd(a->columns[2].simd2[1], a->columns[2].simd2[1], dsSIMD2d_set2(0.0, 1.0))));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);

	DS_SIMD_TRANSPOSE_33(result[0], result[1], result[2]);
}

DS_MATH_EXPORT inline void dsMatrix44d_invertFMA2(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	dsVector4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, a->columns[0], a->columns[1]);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, a->columns[2], a->columns[3]);

	dsVector4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, a->columns[0], a->columns[2]);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, a->columns[1], a->columns[3]);

	dsVector4d det;
	det.simd2[0] = dsSIMD2d_fmsub(detA.simd2[0], detB.simd2[0],
		dsSIMD2d_mul(detC.simd2[0], detD.simd2[0]));
	det.simd2[1] = dsSIMD2d_fmsub(detA.simd2[1], detB.simd2[1],
		dsSIMD2d_mul(detC.simd2[1], detD.simd2[1]));
	dsSIMD2d detA2 = dsSIMD2d_set1FromVec(det.simd2[0], 0);
	dsSIMD2d detB2 = dsSIMD2d_set1FromVec(det.simd2[0], 1);
	dsSIMD2d detC2 = dsSIMD2d_set1FromVec(det.simd2[1], 0);
	dsSIMD2d detD2 = dsSIMD2d_set1FromVec(det.simd2[1], 1);

	dsSIMD2d det44 = dsSIMD2d_fmadd(detA2, detD2, dsSIMD2d_mul(detB2, detC2));

	dsVector4d ab, dc, bdc, cab;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);
	DS_MATRIX22_MUL(bdc, b22, dc);
	DS_MATRIX22_MUL(cab, c22, ab);

	dsVector4d x;
	x.simd2[0] = dsSIMD2d_fmsub(detD2, a22.simd2[0], bdc.simd2[0]);
	x.simd2[1] = dsSIMD2d_fmsub(detD2, a22.simd2[1], bdc.simd2[1]);
	dsVector4d w;
	w.simd2[0] = dsSIMD2d_fmsub(detA2, d22.simd2[0], cab.simd2[0]);
	w.simd2[1] = dsSIMD2d_fmsub(detA2, d22.simd2[1], cab.simd2[1]);

	dsVector4d dab, adc;
	DS_MATRIX22_MUL_ADJ(dab, d22, ab);
	DS_MATRIX22_MUL_ADJ(adc, a22, dc);

	dsVector4d y;
	y.simd2[0] = dsSIMD2d_fmsub(detB2, c22.simd2[0], dab.simd2[0]);
	y.simd2[1] = dsSIMD2d_fmsub(detB2, c22.simd2[1], dab.simd2[1]);
	dsVector4d z;
	z.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(detC2, b22.simd2[0]), adc.simd2[0]);
	z.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(detC2, b22.simd2[1]), adc.simd2[1]);

	dsVector4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);
	dsVector4d tr;
	tr.simd2[0] = dsSIMD2d_mul(ab.simd2[0], dc0213.simd2[0]);
	tr.simd2[1] = dsSIMD2d_mul(ab.simd2[1], dc0213.simd2[1]);
	tr.simd2[0] = dsSIMD2d_hadd(tr.simd2[0], tr.simd2[1]);
	tr.simd2[0] = dsSIMD2d_hadd(tr.simd2[0], tr.simd2[0]);
	det44 = dsSIMD2d_sub(det44, tr.simd2[0]);

	dsSIMD2d sign = dsSIMD2d_set2(1.0, -1.0);
	dsVector4d invDet44;
	invDet44.simd2[0] = dsSIMD2d_div(sign, det44);
	invDet44.simd2[1] = dsSIMD2d_neg(invDet44.simd2[0]);

	x.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], x.simd2[0]);
	x.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], x.simd2[1]);
	y.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], y.simd2[0]);
	y.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], y.simd2[1]);
	z.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], z.simd2[0]);
	z.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], z.simd2[1]);
	w.simd2[0] = dsSIMD2d_mul(invDet44.simd2[0], w.simd2[0]);
	w.simd2[1] = dsSIMD2d_mul(invDet44.simd2[1], w.simd2[1]);

	DS_SIMD_SHUFFLE2_3131_2020(result->columns[0], result->columns[1], x, y);
	DS_SIMD_SHUFFLE2_3131_2020(result->columns[2], result->columns[3], z, w);
}

DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeFMA2(
	dsVector4d result[3], const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsVector4d scale2;
	scale2.simd2[0] = dsSIMD2d_fmadd(a->columns[0].simd2[0], a->columns[0].simd2[0],
		dsSIMD2d_fmadd(a->columns[1].simd2[0], a->columns[1].simd2[0],
		dsSIMD2d_mul(a->columns[2].simd2[0], a->columns[2].simd2[0])));
	scale2.simd2[1] = dsSIMD2d_fmadd(a->columns[0].simd2[1], a->columns[0].simd2[1],
		dsSIMD2d_fmadd(a->columns[1].simd2[1], a->columns[1].simd2[1],
		dsSIMD2d_fmadd(a->columns[2].simd2[1], a->columns[2].simd2[1], dsSIMD2d_set2(0.0, 1.0))));
	dsVector4d invScale2;
	invScale2.simd2[0] = dsSIMD2d_rcp(scale2.simd2[0]);
	invScale2.simd2[1] = dsSIMD2d_rcp(scale2.simd2[1]);

	result[0].simd2[0] = dsSIMD2d_mul(a->columns[0].simd2[0], invScale2.simd2[0]);
	result[0].simd2[1] = dsSIMD2d_mul(a->columns[0].simd2[1], invScale2.simd2[1]);
	result[1].simd2[0] = dsSIMD2d_mul(a->columns[1].simd2[0], invScale2.simd2[0]);
	result[1].simd2[1] = dsSIMD2d_mul(a->columns[1].simd2[1], invScale2.simd2[1]);
	result[2].simd2[0] = dsSIMD2d_mul(a->columns[2].simd2[0], invScale2.simd2[0]);
	result[2].simd2[1] = dsSIMD2d_mul(a->columns[2].simd2[1], invScale2.simd2[1]);
}

DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpFMA2(
	dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	dsVector4d positionA, scaleA, positionB, scaleB, positionInterp, scaleInterp;
	dsQuaternion4d orientationA, orientationB, orientationInterp;

	dsMatrix44d_decomposeTransformSIMD2(&positionA, &orientationA, &scaleA, a);
	dsMatrix44d_decomposeTransformSIMD2(&positionB, &orientationB, &scaleB, b);

	dsSIMD2d t2 = dsSIMD2d_set1(t);
	positionInterp.simd2[0] = dsSIMD2d_fmadd(
		t2, dsSIMD2d_sub(positionB.simd2[0], positionA.simd2[0]), positionA.simd2[0]);
	positionInterp.simd2[1] = dsSIMD2d_fmadd(
		t2, dsSIMD2d_sub(positionB.simd2[1], positionA.simd2[1]), positionA.simd2[1]);
	scaleInterp.simd2[0] = dsSIMD2d_fmadd(
		t2, dsSIMD2d_sub(scaleB.simd2[0], scaleA.simd2[0]), scaleA.simd2[0]);
	scaleInterp.simd2[1] = dsSIMD2d_fmadd(
		t2, dsSIMD2d_sub(scaleB.simd2[1], scaleA.simd2[1]), scaleA.simd2[1]);
	dsQuaternion4d_slerp(&orientationInterp, &orientationA, &orientationB, t);

	dsMatrix44d_composeTransformSIMD2(
		result, &positionInterp, &orientationInterp, (dsVector3d*)&scaleInterp);
}

#undef DS_MATRIX22_MUL
#undef DS_MATRIX22_ADJ_MUL
#undef DS_MATRIX22_MUL_ADJ

DS_SIMD_END()
#endif // DS_DETERMINISTIC_MATH

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_SHUFFLE2_0202_1313
#undef DS_SIMD_SHUFFLE2_0101_2323
#undef DS_SIMD_SHUFFLE2_3131_2020
#undef DS_SIMD_SHUFFLE1_3300_1122
#undef DS_SIMD_SHUFFLE1_0303_2121
#undef DS_SIMD_SHUFFLE1_3030_2121
#undef DS_SIMD_SHUFFLE1_1032
#undef DS_SIMD_SHUFFLE1_2301
#undef DS_SIMD_SHUFFLE1_0213

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA)

#if DS_X86_32 || DS_X86_64
#define DS_SIMD_TRANSPOSE_33(elem0, elem1, elem2) \
	do \
	{ \
		__m256d _zero = _mm256_setzero_pd(); \
		__m256d _tmp0 = _mm256_unpacklo_pd((elem0), (elem1)); \
		__m256d _tmp1 = _mm256_unpacklo_pd((elem2), _zero); \
		__m256d _tmp2 = _mm256_unpackhi_pd((elem0), (elem1)); \
		__m256d _tmp3 = _mm256_unpackhi_pd((elem2), _zero); \
		(a) = _mm256_permute2f128_pd(_tmpA, _tmpC, _MM_SHUFFLE(0, 2, 0, 0)); \
		(b) = _mm256_permute2f128_pd(_tmpB, _tmpD, _MM_SHUFFLE(0, 2, 0, 0)); \
		(c) = _mm256_permute2f128_pd(_tmpC, _tmpA, _MM_SHUFFLE(0, 1, 0, 3)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		__m256d _a0213 = _mm256_permute4x64_pd((a), _MM_SHUFFLE(3, 1, 2, 0)); \
		__m256d _b0213 = _mm256_permute4x64_pd((b), _MM_SHUFFLE(3, 1, 2, 0)); \
		(first) = _mm256_permute2f128_pd((_a0213), (_b0213), _MM_SHUFFLE(0, 2, 0, 0)); \
		(second) = _mm256_permute2f128_pd((_a0213), (_b0213), _MM_SHUFFLE(0, 3, 0, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		(first) = _mm256_permute2f128_pd((a), (b), _MM_SHUFFLE(0, 2, 0, 0)); \
		(second) = _mm256_permute2f128_pd((a), (b), _MM_SHUFFLE(0, 3, 0, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		__m256d _a3120 = _mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 2, 1, 3)); \
		__m256d _b3120 = _mm256_permute4x64_pd((b), _MM_SHUFFLE(0, 2, 1, 3)); \
		(first) = _mm256_permute2f128_pd((_a3120), (_b3120), _MM_SHUFFLE(0, 2, 0, 0)); \
		(second) = _mm256_permute2f128_pd((_a3120), (_b3120), _MM_SHUFFLE(0, 3, 0, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		(first) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 0, 3, 3)); \
		(second) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(2, 2, 1, 1)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0303_2121(first, second, a) \
	do \
	{ \
		(first) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(3, 0, 3, 0)); \
		(second) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(1, 2, 1, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		(first) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 3, 0, 3)); \
		(second) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(1, 2, 1, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		(result) = _mm256_permute_pd((a), 0x5); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		(result) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(1, 0, 3, 2)); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		(result) = _mm256_permute4x64_pd((a), _MM_SHUFFLE(3, 1, 2, 0)); \
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

#define DS_SIMD_SHUFFLE2_0202_1313(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE2_0101_2323(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE2_3131_2020(first, second, a, b) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
		DS_UNUSED(b); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3300_1122(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0303_2121(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_3030_2121(first, second, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(first); \
		DS_UNUSED(second); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_1032(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_2301(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)

#define DS_SIMD_SHUFFLE1_0213(result, a) \
	do \
	{ \
		DS_ASSERT(false); \
		DS_UNUSED(result); \
		DS_UNUSED(a); \
	} while (0)
#endif

#if DS_SIMD_ALWAYS_FMA
#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA1032, _tempB0303, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_0303_2121(_tempB0303, _tempB2121, (b)); \
		(result) = dsSIMD4d_fmadd((a), _tempB0303,  dsSIMD4d_mul(_tempA1032, _tempB2121)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result) = dsSIMD4d_fmsub(_tempA3300, (b), dsSIMD4d_mul(_tempA1122, _tempB2301)); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result) = dsSIMD4d_fmsub((a), _tempB3030, dsSIMD4d_mul(_tempA1032, _tempB2121));\
	} while (0)
#else
#define DS_MATRIX22_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA1032, _tempB0303, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_0303_2121(_tempB0303, _tempB2121, (b)); \
		(result) = dsSIMD4d_add(dsSIMD4d_mul((a), _tempB0303), \
			dsSIMD4d_mul(_tempA1032, _tempB2121)); \
	} while (0)

#define DS_MATRIX22_ADJ_MUL(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA3300, _tempA1122, _tempB2301; \
		DS_SIMD_SHUFFLE1_3300_1122(_tempA3300, _tempA1122, (a)); \
		DS_SIMD_SHUFFLE1_2301(_tempB2301, (b)); \
		(result) = dsSIMD4d_sub(dsSIMD4d_mul(_tempA3300, (b)), \
			dsSIMD4d_mul(_tempA1122, _tempB2301)); \
	} while (0)

#define DS_MATRIX22_MUL_ADJ(result, a, b) \
	do \
	{ \
		dsSIMD4d _tempA1032, _tempB3030, _tempB2121; \
		DS_SIMD_SHUFFLE1_1032(_tempA1032, (a)); \
		DS_SIMD_SHUFFLE1_3030_2121(_tempB3030, _tempB2121, (b)); \
		(result) = dsSIMD4d_sub(dsSIMD4d_mul((a), _tempB3030), \
			dsSIMD4d_mul(_tempA1032, _tempB2121)); \
	} while (0)
#endif

DS_MATH_EXPORT inline void dsMatrix44d_mulSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(a->columns + 3);

	dsSIMD4d mulCol = dsSIMD4d_load(b->columns);
	dsSIMD4d mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	dsSIMD4d mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	dsSIMD4d mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
	dsSIMD4d mul3 = dsSIMD4d_set1FromVec(mulCol, 3);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1,
		dsSIMD4d_fmadd(col2, mul2, dsSIMD4d_mul(col3, mul3)))));
#else
	dsSIMD4d_store(result->columns, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, mul2), dsSIMD4d_mul(col3, mul3))));
#endif

	mulCol = dsSIMD4d_load(b->columns + 1);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
	mul3 = dsSIMD4d_set1FromVec(mulCol, 3);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 1, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1,
		dsSIMD4d_fmadd(col2, mul2, dsSIMD4d_mul(col3, mul3)))));
#else
	dsSIMD4d_store(result->columns + 1, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, mul2), dsSIMD4d_mul(col3, mul3))));
#endif

	mulCol = dsSIMD4d_load(b->columns + 2);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
	mul3 = dsSIMD4d_set1FromVec(mulCol, 3);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1,
		dsSIMD4d_fmadd(col2, mul2, dsSIMD4d_mul(col3, mul3)))));
#else
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, mul2), dsSIMD4d_mul(col3, mul3))));
#endif

	mulCol = dsSIMD4d_load(b->columns + 3);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
	mul3 = dsSIMD4d_set1FromVec(mulCol, 3);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1,
		dsSIMD4d_fmadd(col2, mul2, dsSIMD4d_mul(col3, mul3)))));
#else
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, mul2), dsSIMD4d_mul(col3, mul3))));
#endif
}

DS_MATH_EXPORT inline void dsMatrix44d_affineMulSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(a->columns + 3);

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
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1,
		dsSIMD4d_mul(col2, mul2))));
#else
	dsSIMD4d_store(result->columns + 2, dsSIMD4d_add(dsSIMD4d_add(
		dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)), dsSIMD4d_mul(col2, mul2)));
#endif

	mulCol = dsSIMD4d_load(b->columns + 3);
	mul0 = dsSIMD4d_set1FromVec(mulCol, 0);
	mul1 = dsSIMD4d_set1FromVec(mulCol, 1);
	mul2 = dsSIMD4d_set1FromVec(mulCol, 2);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result->columns + 3,
		dsSIMD4d_fmadd(col0, mul0, dsSIMD4d_fmadd(col1, mul1, dsSIMD4d_fmadd(col2, mul2, col3))));
#else
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, mul0), dsSIMD4d_mul(col1, mul1)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, mul2), col3)));
#endif
}

DS_MATH_EXPORT inline void dsMatrix44d_transformSIMD4(dsVector4d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) mat, const dsVector4d* DS_ALIGN_PARAM(32) vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4d col0 = dsSIMD4d_load(mat->columns);
	dsSIMD4d col1 = dsSIMD4d_load(mat->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(mat->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(mat->columns + 3);

	dsSIMD4d simdVec = dsSIMD4d_load(vec);
	dsSIMD4d x = dsSIMD4d_set1FromVec(simdVec, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(simdVec, 1);
	dsSIMD4d z = dsSIMD4d_set1FromVec(simdVec, 2);
	dsSIMD4d w = dsSIMD4d_set1FromVec(simdVec, 3);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result, dsSIMD4d_fmadd(col0, x, dsSIMD4d_fmadd(col1, y,
		dsSIMD4d_fmadd(col2, z, dsSIMD4d_mul(col3, w)))));
#else
	dsSIMD4d_store(result, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(col0, x), dsSIMD4d_mul(col1, y)),
		dsSIMD4d_add(dsSIMD4d_mul(col2, z), dsSIMD4d_mul(col3, w))));
#endif
}

DS_MATH_EXPORT inline void dsMatrix44d_transformTransposedSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) mat,
	const dsVector4d* DS_ALIGN_PARAM(32) vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);

	dsSIMD4d row0 = dsSIMD4d_load(mat->columns);
	dsSIMD4d row1 = dsSIMD4d_load(mat->columns + 1);
	dsSIMD4d row2 = dsSIMD4d_load(mat->columns + 2);
	dsSIMD4d row3 = dsSIMD4d_load(mat->columns + 3);
	dsSIMD4d_transpose(row0, row1, row2, row3);

	dsSIMD4d simdVec = dsSIMD4d_load(vec);
	dsSIMD4d x = dsSIMD4d_set1FromVec(simdVec, 0);
	dsSIMD4d y = dsSIMD4d_set1FromVec(simdVec, 1);
	dsSIMD4d z = dsSIMD4d_set1FromVec(simdVec, 2);
	dsSIMD4d w = dsSIMD4d_set1FromVec(simdVec, 3);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(result, dsSIMD4d_fmadd(row0, x, dsSIMD4d_fmadd(row1, y,
		dsSIMD4d_fmadd(row2, z, dsSIMD4d_mul(row3, w)))));
#else
	dsSIMD4d_store(result, dsSIMD4d_add(
		dsSIMD4d_add(dsSIMD4d_mul(row0, x), dsSIMD4d_mul(row1, y)),
		dsSIMD4d_add(dsSIMD4d_mul(row2, z), dsSIMD4d_mul(row3, w))));
#endif
}

DS_MATH_EXPORT inline void dsMatrix44d_transposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(a->columns + 3);
	dsSIMD4d_transpose(col0, col1, col2, col3);
	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
	dsSIMD4d_store(result->columns + 3, col3);
}

DS_MATH_EXPORT inline double dsMatrix44d_determinantSIMD4(const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(a->columns + 3);

	dsSIMD4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, col0, col1);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, col2, col3);

	dsSIMD4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, col0, col2);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, col1, col3);

	dsSIMD4d simdDet;
#if DS_SIMD_ALWAYS_FMA
	simdDet = dsSIMD4d_fmsub(detA, detB, dsSIMD4d_mul(detC, detD));
#else
	simdDet = dsSIMD4d_sub(dsSIMD4d_mul(detA, detB), dsSIMD4d_mul(detC, detD));
#endif
	DS_ALIGN(32) dsVector4d det;
	dsSIMD4d_store(&det, simdDet);
	double det44 = det.x*det.w + det.y*det.z;

	dsSIMD4d ab, dc;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);

	dsSIMD4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);

	DS_ALIGN(32) dsVector4d tr;
	dsSIMD4d_store(&tr, dsSIMD4d_mul(ab, dc0213));
	return det44 - ((tr.x + tr.y) + (tr.z + tr.w));
}

DS_MATH_EXPORT inline void dsMatrix44d_fastInvertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_set1(0.0);
	dsSIMD4d_transpose(col0, col1, col2, col3);

	col3 = dsSIMD4d_load(a->columns + 3);
#if DS_SIMD_ALWAYS_FMA
	col3 = dsSIMD4d_fnmsub(col0, dsSIMD4d_set1FromVec(col3, 0),
		dsSIMD4d_fmadd(col1, dsSIMD4d_set1FromVec(col3, 1),
		dsSIMD4d_fmadd(col2, dsSIMD4d_set1FromVec(col3, 2),
		dsSIMD4d_set4(0.0, 0.0, 0.0, -1.0))));
#else
	col3 = dsSIMD4d_sub(dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0),
		dsSIMD4d_add(dsSIMD4d_add(
			dsSIMD4d_mul(col0, dsSIMD4d_set1FromVec(col3, 0)),
			dsSIMD4d_mul(col1, dsSIMD4d_set1FromVec(col3, 1))),
			dsSIMD4d_mul(col2, dsSIMD4d_set1FromVec(col3, 2))));
#endif

	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
	dsSIMD4d_store(result->columns + 3, col3);
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 =dsSIMD4d_set1(0.0);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d invScale2 = dsSIMD4d_fmadd(col0, col0, dsSIMD4d_fmadd(
		col1, col1, dsSIMD4d_fmadd(col2, col2, dsSIMD4d_set4(0.0, 0.0f, 0.0, 1.0))));
#else
	dsSIMD4d invScale2 = dsSIMD4d_add(dsSIMD4d_add(
			dsSIMD4d_mul(col0, col0),
			dsSIMD4d_mul(col1, col1)),
		dsSIMD4d_add(
			dsSIMD4d_mul(col2, col2),
			dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0)));
#endif
	invScale2 = dsSIMD4d_rcp(invScale2);

	col0 = dsSIMD4d_mul(col0, invScale2);
	col1 = dsSIMD4d_mul(col1, invScale2);
	col2 = dsSIMD4d_mul(col2, invScale2);

	dsSIMD4d_transpose(col0, col1, col2, col3);

	col3 = dsSIMD4d_load(a->columns + 3);
#if DS_SIMD_ALWAYS_FMA
	col3 = dsSIMD4d_fnmsub(col0, dsSIMD4d_set1FromVec(col3, 0),
		dsSIMD4d_fmadd(col1, dsSIMD4d_set1FromVec(col3, 1),
		dsSIMD4d_fmadd(col2, dsSIMD4d_set1FromVec(col3, 2),
		dsSIMD4d_set4(0.0, 0.0, 0.0, -1.0))));
#else
	col3 = dsSIMD4d_sub(dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0),
		dsSIMD4d_add(dsSIMD4d_add(
			dsSIMD4d_mul(col0, dsSIMD4d_set1FromVec(col3, 0)),
			dsSIMD4d_mul(col1, dsSIMD4d_set1FromVec(col3, 1))),
			dsSIMD4d_mul(col2, dsSIMD4d_set1FromVec(col3, 2))));
#endif

	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
	dsSIMD4d_store(result->columns + 3, col3);
}

DS_MATH_EXPORT inline void dsMatrix44d_affineInvert33SIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_set1(0.0);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d invScale2 = dsSIMD4d_fmadd(col0, col0,
		dsSIMD4d_fmadd(col1, col1, dsSIMD4d_fmadd(col2, col2, dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0))));
#else
	dsSIMD4d invScale2 = dsSIMD4d_add(dsSIMD4d_add(
			dsSIMD4d_mul(col0, col0),
			dsSIMD4d_mul(col1, col1)),
		dsSIMD4d_add(
			dsSIMD4d_mul(col2, col2),
			dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0)));
#endif
	invScale2 = dsSIMD4d_rcp(invScale2);

	col0 = dsSIMD4d_mul(col0, invScale2);
	col1 = dsSIMD4d_mul(col1, invScale2);
	col2 = dsSIMD4d_mul(col2, invScale2);

	dsSIMD4d_transpose(col0, col1, col2, col3);

	dsSIMD4d_store(result, col0);
	dsSIMD4d_store(result + 1, col1);
	dsSIMD4d_store(result + 2, col2);
}

DS_MATH_EXPORT inline void dsMatrix44d_invertSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	// https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(a->columns + 3);

	dsSIMD4d a22, b22, c22, d22;
	DS_SIMD_SHUFFLE2_0101_2323(a22, b22, col0, col1);
	DS_SIMD_SHUFFLE2_0101_2323(c22, d22, col2, col3);

	dsSIMD4d detA, detB, detC, detD;
	DS_SIMD_SHUFFLE2_0202_1313(detA, detC, col0, col2);
	DS_SIMD_SHUFFLE2_0202_1313(detD, detB, col1, col3);

	dsSIMD4d det;
#if DS_SIMD_ALWAYS_FMA
	det = dsSIMD4d_fmsub(detA, detB, dsSIMD4d_mul(detC, detD));
#else
	det = dsSIMD4d_sub(dsSIMD4d_mul(detA, detB), dsSIMD4d_mul(detC, detD));
#endif
	detA = dsSIMD4d_set1FromVec(det, 0);
	detB = dsSIMD4d_set1FromVec(det, 1);
	detC = dsSIMD4d_set1FromVec(det, 2);
	detD = dsSIMD4d_set1FromVec(det, 3);

	dsSIMD4d det44;
#if DS_SIMD_ALWAYS_FMA
	det44 = dsSIMD4d_fmadd(detA, detD, dsSIMD4d_mul(detB, detC));
#else
	det44 = dsSIMD4d_add(dsSIMD4d_mul(detA, detD), dsSIMD4d_mul(detB, detC));
#endif

	dsSIMD4d ab, dc, bdc, cab;
	DS_MATRIX22_ADJ_MUL(ab, a22, b22);
	DS_MATRIX22_ADJ_MUL(dc, d22, c22);
	DS_MATRIX22_MUL(bdc, b22, dc);
	DS_MATRIX22_MUL(cab, c22, ab);

	dsSIMD4d x, w;
#if DS_SIMD_ALWAYS_FMA
	x = dsSIMD4d_fmsub(detD, a22, bdc);
	w = dsSIMD4d_fmsub(detA, d22, cab);
#else
	x = dsSIMD4d_sub(dsSIMD4d_mul(detD, a22), bdc);
	w = dsSIMD4d_sub(dsSIMD4d_mul(detA, d22), cab);
#endif

	dsSIMD4d dab, adc;
	DS_MATRIX22_MUL_ADJ(dab, d22, ab);
	DS_MATRIX22_MUL_ADJ(adc, a22, dc);

	dsSIMD4d y, z;
#if DS_SIMD_ALWAYS_FMA
	y = dsSIMD4d_fmsub(detB, c22, dab);
	z = dsSIMD4d_fmsub(detC, b22, adc);
#else
	y = dsSIMD4d_sub(dsSIMD4d_mul(detB, c22), dab);
	z = dsSIMD4d_sub(dsSIMD4d_mul(detC, b22), adc);
#endif

	dsSIMD4d dc0213;
	DS_SIMD_SHUFFLE1_0213(dc0213, dc);
	dsSIMD4d tr = dsSIMD4d_mul(ab, dc0213);
	// hadd for double 4 doesn't cross 128 bit boundaries.
	tr = dsSIMD4d_hadd(tr, tr);
	tr = dsSIMD4d_add(dsSIMD4d_set1FromVec(tr, 0), dsSIMD4d_set1FromVec(tr, 2));
	det44 = dsSIMD4d_sub(det44, tr);

	dsSIMD4d sign = dsSIMD4d_set4(1.0, -1.0, -1.0, 1.0);
	dsSIMD4d invDet44 = dsSIMD4d_div(sign, det44);

	x = dsSIMD4d_mul(invDet44, x);
	y = dsSIMD4d_mul(invDet44, y);
	z = dsSIMD4d_mul(invDet44, z);
	w = dsSIMD4d_mul(invDet44, w);

	DS_SIMD_SHUFFLE2_3131_2020(col0, col1, x, y);
	DS_SIMD_SHUFFLE2_3131_2020(col2, col3, z, w);

	dsSIMD4d_store(result->columns, col0);
	dsSIMD4d_store(result->columns + 1, col1);
	dsSIMD4d_store(result->columns + 2, col2);
	dsSIMD4d_store(result->columns + 3, col3);
}

DS_MATH_EXPORT inline void dsMatrix44d_inverseTransposeSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsSIMD4d col0 = dsSIMD4d_load(a->columns);
	dsSIMD4d col1 = dsSIMD4d_load(a->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(a->columns + 2);

#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d invScale2 = dsSIMD4d_fmadd(col0, col0,
		dsSIMD4d_fmadd(col1, col1, dsSIMD4d_fmadd(col2, col2, dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0))));
#else
	dsSIMD4d invScale2 = dsSIMD4d_add(dsSIMD4d_add(
			dsSIMD4d_mul(col0, col0),
			dsSIMD4d_mul(col1, col1)),
		dsSIMD4d_add(
			dsSIMD4d_mul(col2, col2),
			dsSIMD4d_set4(0.0, 0.0, 0.0, 1.0)));
#endif
	invScale2 = dsSIMD4d_rcp(invScale2);

	col0 = dsSIMD4d_mul(col0, invScale2);
	col1 = dsSIMD4d_mul(col1, invScale2);
	col2 = dsSIMD4d_mul(col2, invScale2);

	dsSIMD4d_store(result, col0);
	dsSIMD4d_store(result + 1, col1);
	dsSIMD4d_store(result + 2, col2);
}

DS_MATH_EXPORT inline void dsMatrix44d_decomposeTransformSIMD4(
	dsVector4d* DS_ALIGN_PARAM(32) outPosition, dsQuaternion4d* DS_ALIGN_PARAM(32) outOrientation,
	dsVector4d* DS_ALIGN_PARAM(32) outScale, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix)
{
	DS_ASSERT(outPosition);
	DS_ASSERT(outOrientation);
	DS_ASSERT(outScale);
	DS_ASSERT(matrix);

	dsSIMD4d col0 = dsSIMD4d_load(matrix->columns);
	dsSIMD4d col1 = dsSIMD4d_load(matrix->columns + 1);
	dsSIMD4d col2 = dsSIMD4d_load(matrix->columns + 2);
	dsSIMD4d col3 = dsSIMD4d_load(matrix->columns + 3);

	dsSIMD4d_store(outPosition, col3);

	dsSIMD4d dot;
	dot = dsSIMD4d_mul(col0, col0);
	double len2x = dsSIMD4d_get(dot, 0) + dsSIMD4d_get(dot, 1) + dsSIMD4d_get(dot, 2);
	dot = dsSIMD4d_mul(col1, col1);
	double len2y = dsSIMD4d_get(dot, 0) + dsSIMD4d_get(dot, 1) + dsSIMD4d_get(dot, 2);
	dot = dsSIMD4d_mul(col2, col2);
	double len2z = dsSIMD4d_get(dot, 0) + dsSIMD4d_get(dot, 1) + dsSIMD4d_get(dot, 2);
	dsSIMD4d scale = dsSIMD4d_sqrt(dsSIMD4d_set4(len2x, len2y, len2z, 1.0));
	dsSIMD4d_store(outScale, scale);

	dsSIMD4d invScale = dsSIMD4d_rcp(scale);
	dsSIMD4d invScaleX = dsSIMD4d_set1FromVec(invScale, 0);
	dsSIMD4d invScaleY = dsSIMD4d_set1FromVec(invScale, 1);
	dsSIMD4d invScaleZ = dsSIMD4d_set1FromVec(invScale, 2);

	DS_ALIGN(32) dsMatrix44d rotateMat;
	dsSIMD4d_store(rotateMat.columns, dsSIMD4d_mul(col0, invScaleX));
	dsSIMD4d_store(rotateMat.columns + 1, dsSIMD4d_mul(col1, invScaleY));
	dsSIMD4d_store(rotateMat.columns + 2, dsSIMD4d_mul(col2, invScaleZ));
	dsQuaternion4d_fromMatrix44(outOrientation, &rotateMat);
}

DS_MATH_EXPORT inline void dsMatrix44d_composeTransformSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsVector4d* DS_ALIGN_PARAM(32) position, const dsQuaternion4d* orientation,
	const dsVector3d* scale)
{
	DS_ASSERT(result);
	DS_ASSERT(position);
	DS_ASSERT(orientation);
	DS_ASSERT(scale);

	dsQuaternion4d_toMatrix44(result, orientation);
	dsSIMD4d_store(result->columns,
		dsSIMD4d_mul(dsSIMD4d_load(result->columns), dsSIMD4d_set1(scale->x)));
	dsSIMD4d_store(result->columns + 1,
		dsSIMD4d_mul(dsSIMD4d_load(result->columns + 1), dsSIMD4d_set1(scale->y)));
	dsSIMD4d_store(result->columns + 2,
		dsSIMD4d_mul(dsSIMD4d_load(result->columns + 2), dsSIMD4d_set1(scale->z)));
	dsSIMD4d_store(result->columns + 3, dsSIMD4d_load(position));
}

DS_MATH_EXPORT inline void dsMatrix44d_rigidLerpSIMD4(dsMatrix44d* DS_ALIGN_PARAM(32) result,
	const dsMatrix44d* DS_ALIGN_PARAM(32) a, const dsMatrix44d* DS_ALIGN_PARAM(32) b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	DS_ALIGN_PARAM(32) dsVector4d positionA, scaleA, positionB, scaleB, positionInterp, scaleInterp;
	DS_ALIGN_PARAM(32) dsQuaternion4d orientationA, orientationB, orientationInterp;

	dsMatrix44d_decomposeTransformSIMD4(&positionA, &orientationA, &scaleA, a);
	dsMatrix44d_decomposeTransformSIMD4(&positionB, &orientationB, &scaleB, b);

	dsSIMD4d t4 = dsSIMD4d_set1(t);
	dsSIMD4d positionASIMD = dsSIMD4d_load(&positionA);
	dsSIMD4d positionBSIMD = dsSIMD4d_load(&positionB);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(&positionInterp, dsSIMD4d_fmadd(
		t4, dsSIMD4d_sub(positionBSIMD, positionASIMD), positionASIMD));
#else
	dsSIMD4d_store(&positionInterp, dsSIMD4d_add(positionASIMD,
		dsSIMD4d_mul(t4, dsSIMD4d_sub(positionBSIMD, positionASIMD))));
#endif

	dsSIMD4d scaleASIMD = dsSIMD4d_load(&scaleA);
	dsSIMD4d scaleBSIMD = dsSIMD4d_load(&scaleB);
#if DS_SIMD_ALWAYS_FMA
	dsSIMD4d_store(&scaleInterp, dsSIMD4d_fmadd(
		t4, dsSIMD4d_sub(scaleBSIMD, scaleASIMD), scaleASIMD));
#else
	dsSIMD4d_store(&scaleInterp, dsSIMD4d_add(scaleASIMD,
		dsSIMD4d_mul(t4, dsSIMD4d_sub(scaleBSIMD, scaleASIMD))));
#endif

	dsQuaternion4d_slerp(&orientationInterp, &orientationA, &orientationB, t);

	dsMatrix44d_composeTransformSIMD4(
		result, &positionInterp, &orientationInterp, (dsVector3d*)&scaleInterp);
}

#undef DS_MATRIX22_MUL
#undef DS_MATRIX22_ADJ_MUL
#undef DS_MATRIX22_MUL_ADJ

#undef DS_SIMD_TRANSPOSE_33
#undef DS_SIMD_SHUFFLE2_0202_1313
#undef DS_SIMD_SHUFFLE2_0101_2323
#undef DS_SIMD_SHUFFLE2_3131_2020
#undef DS_SIMD_SHUFFLE1_3300_1122
#undef DS_SIMD_SHUFFLE1_0303_2121
#undef DS_SIMD_SHUFFLE1_3030_2121
#undef DS_SIMD_SHUFFLE1_1032
#undef DS_SIMD_SHUFFLE1_2301
#undef DS_SIMD_SHUFFLE1_0213

DS_SIMD_END()

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
