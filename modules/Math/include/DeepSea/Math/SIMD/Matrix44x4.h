/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Math/SIMD/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulate four simultaneous 4x4 matrices.
 *
 * These will only be available if DS_HAS_SIMD is set to 1.
 */

#if DS_HAS_SIMD

/**
 * @brief Loads four matrices into a single structure.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The loaded matrices.
 * @param a The first matrix to load.
 * @param b The second matrix to load.
 * @param c The third matrix to load.
 * @param d The fourth matrix to load.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_load(dsMatrix44x4f* result, const dsMatrix44fSIMD* a,
	const dsMatrix44fSIMD* b, const dsMatrix44fSIMD* c, const dsMatrix44fSIMD* d);

/**
 * @brief Stores four matrices into separate values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] outA The first matrix to store into.
 * @param[out] outB The second matrix to store into.
 * @param[out] outC The third matrix to store into.
 * @param[out] outD The fourth matrix to store into.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_store(dsMatrix44fSIMD* outA, dsMatrix44fSIMD* outB,
	dsMatrix44fSIMD* outC, dsMatrix44fSIMD* outD, const dsMatrix44x4f* matrices);

/**
 * @brief Stores the upper 3x3 matrices into separate values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] outA The first matrix to store into as 3 vectors.
 * @param[out] outB The second matrix to store into as 3 vectors.
 * @param[out] outC The third matrix to store into as 3 vectors.
 * @param[out] outD The fourth matrix to store into.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_store33(dsVector4fSIMD* outA, dsVector4fSIMD* outB,
	dsVector4fSIMD* outC, dsVector4fSIMD* outD, const dsMatrix44x4f* matrices);

/**
 * @brief Multiplies two sets of four matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first set of matrices.
 * @param b The second set of matrices.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_mul(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b);

/**
 * @brief Multiplies two sets of four matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first set of matrices.
 * @param b The second set of matrices.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_mulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b);

/**
 * @brief Multiplies two sets of four affine matrices.
 *
 * This assumes that the last row of each matrix is [0, 0, 0, 1].
 *
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first set of matrices.
 * @param b The second set of matrices.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_affineMul(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b);

/**
 * @brief Multiplies two sets of four affine matrices using fused multiply-add operations.
 *
 * This assumes that the last row of each matrix is [0, 0, 0, 1].
 *
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the multiplication. This must not be the same as a or b.
 * @param a The first set of matrices.
 * @param b The second set of matrices.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_affineMulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b);

/**
 * @brief Transposes four matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the transpose. This must not be the same as a.
 * @param a The set of matrices to transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_transpose(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Inverts four matrices that only contains a rotation and translation.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_fastInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Inverts four matrices that only contains a rotation and translation using fused
 *     multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_fastInvertFMA(dsMatrix44x4f* result,
	const dsMatrix44x4f* a);

/**
 * @brief Inverts four affine matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_affineInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Inverts four affine matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_affineInvertFMA(dsMatrix44x4f* result,
	const dsMatrix44x4f* a);

/**
 * @brief Inverts four matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_invert(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Inverts four matrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_invertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the inverse-transpose. This must not be the same as a.
 * @param a The set of matrices to inverse-transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_inverseTranspose(dsMatrix44x4f* result,
	const dsMatrix44x4f* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix to transform direction vectors
 *     using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the inverse-transpose. This must not be the same as a.
 * @param a The set of matrices to inverse-transpose.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_inverseTransposeFMA(dsMatrix44x4f* result,
	const dsMatrix44x4f* a);

/**
 * @brief Inverts the upper 3x3 portion of four matrices.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_invert33(dsMatrix44x4f* result, const dsMatrix44x4f* a);

/**
 * @brief Inverts upper 3x3 portion of fourmatrices using fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param[out] result The result of the invert. This must not be the same as a.
 * @param a The set of matrices to invert.
 */
DS_MATH_EXPORT inline void dsMatrix44x4f_invert33FMA(dsMatrix44x4f* result, const dsMatrix44x4f* a);

DS_SIMD_START_FLOAT4()

/// @cond
#define dsMatrix33x4_determinantImpl(a, i0, i1, i2, j0, j1, j2) \
	dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(\
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i0][j0], (a).values[i1][j1]), (a).values[i2][j2]), \
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i1][j0], (a).values[i2][j1]), (a).values[i0][j2])), \
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i2][j0], (a).values[i0][j1]), (a).values[i1][j2])), \
	dsSIMD4f_add(dsSIMD4f_add( \
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i2][j0], (a).values[i1][j1]), (a).values[i0][j2]), \
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i1][j0], (a).values[i0][j1]), (a).values[i2][j2])), \
		dsSIMD4f_mul(dsSIMD4f_mul((a).values[i0][j0], (a).values[i2][j1]), (a).values[i1][j2])))

#define dsMatrix33x4_invertImpl(result, mat, invDet) \
	do \
	{ \
		(result).values[0][0] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[1][1], (mat).values[2][2]), \
			dsSIMD4f_mul((mat).values[1][2], (mat).values[2][1])), invDet); \
		(result).values[0][1] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[2][1]), \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[2][2])), invDet); \
		(result).values[0][2] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[1][2]), \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[1][1])), invDet); \
		\
		(result).values[1][0] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[1][2], (mat).values[2][0]), \
			dsSIMD4f_mul((mat).values[1][0], (mat).values[2][2])), invDet); \
		(result).values[1][1] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[2][2]), \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[2][0])), invDet); \
		(result).values[1][2] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[1][0]), \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[1][2])), invDet); \
		\
		(result).values[2][0] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[1][0], (mat).values[2][1]), \
			dsSIMD4f_mul((mat).values[1][1], (mat).values[2][0])), invDet); \
		(result).values[2][1] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[2][0]), \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[2][1])), invDet); \
		(result).values[2][2] = dsSIMD4f_mul(dsSIMD4f_sub( \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[1][1]), \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[1][0])), invDet); \
	} \
	while (0)
/// @endcond

inline void dsMatrix44x4f_load(dsMatrix44x4f* result, const dsMatrix44fSIMD* a,
	const dsMatrix44fSIMD* b, const dsMatrix44fSIMD* c, const dsMatrix44fSIMD* d)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(c);
	DS_ASSERT(d);

	result->values[0][0] = dsSIMD4f_load(a->columns);
	result->values[0][1] = dsSIMD4f_load(b->columns);
	result->values[0][2] = dsSIMD4f_load(c->columns);
	result->values[0][3] = dsSIMD4f_load(d->columns);
	dsSIMD4f_transpose(result->values[0][0], result->values[0][1], result->values[0][2],
		result->values[0][3]);

	result->values[1][0] = dsSIMD4f_load(a->columns + 1);
	result->values[1][1] = dsSIMD4f_load(b->columns + 1);
	result->values[1][2] = dsSIMD4f_load(c->columns + 1);
	result->values[1][3] = dsSIMD4f_load(d->columns + 1);
	dsSIMD4f_transpose(result->values[1][0], result->values[1][1], result->values[1][2],
		result->values[1][3]);

	result->values[2][0] = dsSIMD4f_load(a->columns + 2);
	result->values[2][1] = dsSIMD4f_load(b->columns + 2);
	result->values[2][2] = dsSIMD4f_load(c->columns + 2);
	result->values[2][3] = dsSIMD4f_load(d->columns + 2);
	dsSIMD4f_transpose(result->values[2][0], result->values[2][1], result->values[2][2],
		result->values[2][3]);

	result->values[3][0] = dsSIMD4f_load(a->columns + 3);
	result->values[3][1] = dsSIMD4f_load(b->columns + 3);
	result->values[3][2] = dsSIMD4f_load(c->columns + 3);
	result->values[3][3] = dsSIMD4f_load(d->columns + 3);
	dsSIMD4f_transpose(result->values[3][0], result->values[3][1], result->values[3][2],
		result->values[3][3]);
}

inline void dsMatrix44x4f_store(dsMatrix44fSIMD* outA, dsMatrix44fSIMD* outB, dsMatrix44fSIMD* outC,
	dsMatrix44fSIMD* outD, const dsMatrix44x4f* matrices)
{
	DS_ASSERT(outA);
	DS_ASSERT(outB);
	DS_ASSERT(outC);
	DS_ASSERT(outD);
	DS_ASSERT(matrices);

	dsSIMD4f a, b, c, d;
	a = matrices->values[0][0];
	b = matrices->values[0][1];
	c = matrices->values[0][2];
	d = matrices->values[0][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA->columns, a);
	dsSIMD4f_store(outB->columns, b);
	dsSIMD4f_store(outC->columns, c);
	dsSIMD4f_store(outD->columns, d);

	a = matrices->values[1][0];
	b = matrices->values[1][1];
	c = matrices->values[1][2];
	d = matrices->values[1][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA->columns + 1, a);
	dsSIMD4f_store(outB->columns + 1, b);
	dsSIMD4f_store(outC->columns + 1, c);
	dsSIMD4f_store(outD->columns + 1, d);

	a = matrices->values[2][0];
	b = matrices->values[2][1];
	c = matrices->values[2][2];
	d = matrices->values[2][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA->columns + 2, a);
	dsSIMD4f_store(outB->columns + 2, b);
	dsSIMD4f_store(outC->columns + 2, c);
	dsSIMD4f_store(outD->columns + 2, d);

	a = matrices->values[3][0];
	b = matrices->values[3][1];
	c = matrices->values[3][2];
	d = matrices->values[3][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA->columns + 3, a);
	dsSIMD4f_store(outB->columns + 3, b);
	dsSIMD4f_store(outC->columns + 3, c);
	dsSIMD4f_store(outD->columns + 3, d);
}

inline void dsMatrix44x4f_store33(dsVector4fSIMD* outA, dsVector4fSIMD* outB, dsVector4fSIMD* outC,
	dsVector4fSIMD* outD, const dsMatrix44x4f* matrices)
{
	DS_ASSERT(outA);
	DS_ASSERT(outB);
	DS_ASSERT(outC);
	DS_ASSERT(outD);
	DS_ASSERT(matrices);

	dsSIMD4f a, b, c, d;
	a = matrices->values[0][0];
	b = matrices->values[0][1];
	c = matrices->values[0][2];
	d = matrices->values[0][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA, a);
	dsSIMD4f_store(outB, b);
	dsSIMD4f_store(outC, c);
	dsSIMD4f_store(outD, d);

	a = matrices->values[1][0];
	b = matrices->values[1][1];
	c = matrices->values[1][2];
	d = matrices->values[1][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA + 1, a);
	dsSIMD4f_store(outB + 1, b);
	dsSIMD4f_store(outC + 1, c);
	dsSIMD4f_store(outD + 1, d);

	a = matrices->values[2][0];
	b = matrices->values[2][1];
	c = matrices->values[2][2];
	d = matrices->values[2][3];
	dsSIMD4f_transpose(a, b, c, d);
	dsSIMD4f_store(outA + 2, a);
	dsSIMD4f_store(outB + 2, b);
	dsSIMD4f_store(outC + 2, c);
	dsSIMD4f_store(outD + 2, d);
}

inline void dsMatrix44x4f_mul(dsMatrix44x4f* result, const dsMatrix44x4f* a, const dsMatrix44x4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->values[0][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[0][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][0], b->values[0][2]),
			dsSIMD4f_mul(a->values[3][0], b->values[0][3])));
	result->values[0][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[0][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][1], b->values[0][2]),
			dsSIMD4f_mul(a->values[3][1], b->values[0][3])));
	result->values[0][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[0][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][2], b->values[0][2]),
			dsSIMD4f_mul(a->values[3][2], b->values[0][3])));
	result->values[0][3] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][3], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][3], b->values[0][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][3], b->values[0][2]),
			dsSIMD4f_mul(a->values[3][3], b->values[0][3])));

	result->values[1][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[1][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][0], b->values[1][2]),
			dsSIMD4f_mul(a->values[3][0], b->values[1][3])));
	result->values[1][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[1][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][1], b->values[1][2]),
			dsSIMD4f_mul(a->values[3][1], b->values[1][3])));
	result->values[1][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[1][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][2], b->values[1][2]),
			dsSIMD4f_mul(a->values[3][2], b->values[1][3])));
	result->values[1][3] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][3], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][3], b->values[1][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][3], b->values[1][2]),
			dsSIMD4f_mul(a->values[3][3], b->values[1][3])));

	result->values[2][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[2][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][0], b->values[2][2]),
			dsSIMD4f_mul(a->values[3][0], b->values[2][3])));
	result->values[2][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[2][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][1], b->values[2][2]),
			dsSIMD4f_mul(a->values[3][1], b->values[2][3])));
	result->values[2][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[2][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][2], b->values[2][2]),
			dsSIMD4f_mul(a->values[3][2], b->values[2][3])));
	result->values[2][3] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][3], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][3], b->values[2][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][3], b->values[2][2]),
			dsSIMD4f_mul(a->values[3][3], b->values[2][3])));

	result->values[3][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][0], b->values[3][2]),
			dsSIMD4f_mul(a->values[3][0], b->values[3][3])));
	result->values[3][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][1], b->values[3][2]),
			dsSIMD4f_mul(a->values[3][1], b->values[3][3])));
	result->values[3][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][2], b->values[3][2]),
			dsSIMD4f_mul(a->values[3][2], b->values[3][3])));
	result->values[3][3] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][3], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][3], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][3], b->values[3][2]),
			dsSIMD4f_mul(a->values[3][3], b->values[3][3])));
}

inline void dsMatrix44x4f_affineMul(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	result->values[0][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[0][1])),
		dsSIMD4f_mul(a->values[2][0], b->values[0][2]));
	result->values[0][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[0][1])),
		dsSIMD4f_mul(a->values[2][1], b->values[0][2]));
	result->values[0][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[0][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[0][1])),
		dsSIMD4f_mul(a->values[2][2], b->values[0][2]));
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[1][1])),
		dsSIMD4f_mul(a->values[2][0], b->values[1][2]));
	result->values[1][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[1][1])),
		dsSIMD4f_mul(a->values[2][1], b->values[1][2]));
	result->values[1][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[1][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[1][1])),
		dsSIMD4f_mul(a->values[2][2], b->values[1][2]));
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[2][1])),
		dsSIMD4f_mul(a->values[2][0], b->values[2][2]));
	result->values[2][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[2][1])),
		dsSIMD4f_mul(a->values[2][1], b->values[2][2]));
	result->values[2][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[2][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[2][1])),
		dsSIMD4f_mul(a->values[2][2], b->values[2][2]));
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][0], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][0], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][0], b->values[3][2]), a->values[3][0]));
	result->values[3][1] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][1], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][1], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][1], b->values[3][2]), a->values[3][1]));
	result->values[3][2] = dsSIMD4f_add(
		dsSIMD4f_add(dsSIMD4f_mul(a->values[0][2], b->values[3][0]),
			dsSIMD4f_mul(a->values[1][2], b->values[3][1])),
		dsSIMD4f_add(dsSIMD4f_mul(a->values[2][2], b->values[3][2]), a->values[3][2]));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_transpose(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	result->values[0][0] = a->values[0][0];
	result->values[0][1] = a->values[1][0];
	result->values[0][2] = a->values[2][0];
	result->values[0][3] = a->values[3][0];

	result->values[1][0] = a->values[0][1];
	result->values[1][1] = a->values[1][1];
	result->values[1][2] = a->values[2][1];
	result->values[1][3] = a->values[3][1];

	result->values[2][0] = a->values[0][2];
	result->values[2][1] = a->values[1][2];
	result->values[2][2] = a->values[2][2];
	result->values[2][3] = a->values[3][2];

	result->values[3][0] = a->values[0][3];
	result->values[3][1] = a->values[1][3];
	result->values[3][2] = a->values[2][3];
	result->values[3][3] = a->values[3][3];
}

inline void dsMatrix44x4f_fastInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	result->values[0][0] = a->values[0][0];
	result->values[0][1] = a->values[1][0];
	result->values[0][2] = a->values[2][0];
	result->values[0][3] = zero;

	result->values[1][0] = a->values[0][1];
	result->values[1][1] = a->values[1][1];
	result->values[1][2] = a->values[2][1];
	result->values[1][3] = zero;

	result->values[2][0] = a->values[0][2];
	result->values[2][1] = a->values[1][2];
	result->values[2][2] = a->values[2][2];
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][0])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][0])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][0]));
	result->values[3][1] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][1])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][1])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][1]));
	result->values[3][2] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][2])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][2])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][2]));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_affineInvert(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));
	dsMatrix33x4_invertImpl(*result, *a, invUpperDet);

	result->values[0][3] = zero;
	result->values[1][3] = zero;
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][0])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][0])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][0]));
	result->values[3][1] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][1])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][1])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][1]));
	result->values[3][2] = dsSIMD4f_sub(dsSIMD4f_sub(dsSIMD4f_neg(
		dsSIMD4f_mul(a->values[3][0], result->values[0][2])),
		dsSIMD4f_mul(a->values[3][1], result->values[1][2])),
		dsSIMD4f_mul(a->values[3][2], result->values[2][2]));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_invert(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f det0 = dsMatrix33x4_determinantImpl(*a, 1, 2, 3, 1, 2, 3);
	dsSIMD4f det1 = dsMatrix33x4_determinantImpl(*a, 0, 2, 3, 1, 2, 3);
	dsSIMD4f det2 = dsMatrix33x4_determinantImpl(*a, 0, 1, 3, 1, 2, 3);
	dsSIMD4f det3 = dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 1, 2, 3);
	dsSIMD4f det = dsSIMD4f_add(
		dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][0], det0), dsSIMD4f_mul(a->values[1][0], det1)),
		dsSIMD4f_sub(dsSIMD4f_mul(a->values[2][0], det2), dsSIMD4f_mul(a->values[3][0], det3)));

	// Prefer more accurate divide.
	dsSIMD4f invDet = dsSIMD4f_div(dsSIMD4f_set1(1.0f), det);

	result->values[0][0] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[2][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[3][2]), a->values[1][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[1][2]), a->values[2][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[3][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[1][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[2][2]), a->values[1][3]))),
		invDet);
	result->values[0][1] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[3][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[0][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[2][2]), a->values[0][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[2][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[3][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[0][2]), a->values[2][3]))),
		invDet);
	result->values[0][2] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[1][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[3][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[0][2]), a->values[1][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[3][2]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[0][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[1][2]), a->values[0][3]))),
		invDet);
	result->values[0][3] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[2][2]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[0][2]), a->values[2][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[1][2]), a->values[0][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][1], a->values[1][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][1], a->values[2][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[0][2]), a->values[1][3]))),
		invDet);

	result->values[1][0] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][2]), a->values[1][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][2]), a->values[1][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][2]), a->values[2][3]))),
		invDet);
	result->values[1][1] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][2]), a->values[2][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][2]), a->values[0][3]))),
		invDet);
	result->values[1][2] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][2]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][2]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][2]), a->values[0][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][2]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][2]), a->values[1][3]))),
		invDet);
	result->values[1][3] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][2]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][2]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][2]), a->values[1][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][2]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][2]), a->values[2][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][2]), a->values[0][3]))),
		invDet);

	result->values[2][0] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[1][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[2][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[1][3]))),
		invDet);
	result->values[2][1] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[0][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[2][3]))),
		invDet);
	result->values[2][2] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[3][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[1][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[3][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[0][3]))),
		invDet);
	result->values[2][3] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[1][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[2][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[0][3])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[2][3]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[0][3])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[1][3]))),
		invDet);

	result->values[3][0] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[2][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[3][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[1][2])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[3][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[1][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[2][2]))),
		invDet);
	result->values[3][1] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[3][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[0][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[2][2])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[2][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[3][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[0][2]))),
		invDet);
	result->values[3][2] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[1][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[3][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[0][2])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[3][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[0][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[1][2]))),
		invDet);
	result->values[3][3] = dsSIMD4f_mul(
		dsSIMD4f_sub(dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[2][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[0][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[1][2])),
		dsSIMD4f_add(dsSIMD4f_add(
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[1][2]),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[2][2])),
			dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[0][2]))),
		invDet);
}

inline void dsMatrix44x4f_invert33(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));

	result->values[0][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][1], a->values[2][2]),
		dsSIMD4f_mul(a->values[1][2], a->values[2][1])), invUpperDet);
	result->values[0][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][2], a->values[2][1]),
		dsSIMD4f_mul(a->values[0][1], a->values[2][2])), invUpperDet);
	result->values[0][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][1], a->values[1][2]),
		dsSIMD4f_mul(a->values[0][2], a->values[1][1])), invUpperDet);
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][2], a->values[2][0]),
		dsSIMD4f_mul(a->values[1][0], a->values[2][2])), invUpperDet);
	result->values[1][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][0], a->values[2][2]),
		dsSIMD4f_mul(a->values[0][2], a->values[2][0])), invUpperDet);
	result->values[1][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][2], a->values[1][0]),
		dsSIMD4f_mul(a->values[0][0], a->values[1][2])), invUpperDet);
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][0], a->values[2][1]),
		dsSIMD4f_mul(a->values[1][1], a->values[2][0])), invUpperDet);
	result->values[2][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][1], a->values[2][0]),
		dsSIMD4f_mul(a->values[0][0], a->values[2][1])), invUpperDet);
	result->values[2][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][0], a->values[1][1]),
		dsSIMD4f_mul(a->values[0][1], a->values[1][0])), invUpperDet);
	result->values[2][3] = zero;

	result->values[3][0] = zero;
	result->values[3][1] = zero;
	result->values[3][2] = zero;
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_inverseTranspose(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));

	result->values[0][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][1], a->values[2][2]),
		dsSIMD4f_mul(a->values[1][2], a->values[2][1])), invUpperDet);
	result->values[0][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][2], a->values[2][0]),
		dsSIMD4f_mul(a->values[1][0], a->values[2][2])), invUpperDet);
	result->values[0][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[1][0], a->values[2][1]),
		dsSIMD4f_mul(a->values[1][1], a->values[2][0])), invUpperDet);
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][2], a->values[2][1]),
		dsSIMD4f_mul(a->values[0][1], a->values[2][2])), invUpperDet);
	result->values[1][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][0], a->values[2][2]),
		dsSIMD4f_mul(a->values[0][2], a->values[2][0])), invUpperDet);
	result->values[1][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][1], a->values[2][0]),
		dsSIMD4f_mul(a->values[0][0], a->values[2][1])), invUpperDet);
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][1], a->values[1][2]),
		dsSIMD4f_mul(a->values[0][2], a->values[1][1])), invUpperDet);
	result->values[2][1] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][2], a->values[1][0]),
		dsSIMD4f_mul(a->values[0][0], a->values[1][2])), invUpperDet);
	result->values[2][2] = dsSIMD4f_mul(dsSIMD4f_sub(dsSIMD4f_mul(a->values[0][0], a->values[1][1]),
		dsSIMD4f_mul(a->values[0][1], a->values[1][0])), invUpperDet);
	result->values[2][3] = zero;

	result->values[3][0] = zero;
	result->values[3][1] = zero;
	result->values[3][2] = zero;
	result->values[3][3] = one;
}

#undef dsMatrix33x4_determinantImpl
#undef dsMatrix33x4_invertImpl

DS_SIMD_END()

DS_SIMD_START_FMA()

/// @cond
#define dsMatrix33x4_determinantImpl(a, i0, i1, i2, j0, j1, j2) \
	dsSIMD4f_fmadd(dsSIMD4f_mul((a).values[i0][j0], (a).values[i1][j1]), (a).values[i2][j2], \
	dsSIMD4f_fmadd(dsSIMD4f_mul((a).values[i1][j0], (a).values[i2][j1]), (a).values[i0][j2], \
	dsSIMD4f_fmsub(dsSIMD4f_mul((a).values[i2][j0], (a).values[i0][j1]), (a).values[i1][j2], \
	dsSIMD4f_fmadd(dsSIMD4f_mul((a).values[i2][j0], (a).values[i1][j1]), (a).values[i0][j2], \
	dsSIMD4f_fmadd(dsSIMD4f_mul((a).values[i1][j0], (a).values[i0][j1]), (a).values[i2][j2], \
	dsSIMD4f_mul(dsSIMD4f_mul((a).values[i0][j0], (a).values[i2][j1]), (a).values[i1][j2]))))))

#define dsMatrix33x4_invertImpl(result, mat, invDet) \
	do \
	{ \
		(result).values[0][0] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[1][1], (mat).values[2][2], \
			dsSIMD4f_mul((mat).values[1][2], (mat).values[2][1])), invDet); \
		(result).values[0][1] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][2], (mat).values[2][1], \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[2][2])), invDet); \
		(result).values[0][2] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][1], (mat).values[1][2], \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[1][1])), invDet); \
		\
		(result).values[1][0] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[1][2], (mat).values[2][0], \
			dsSIMD4f_mul((mat).values[1][0], (mat).values[2][2])), invDet); \
		(result).values[1][1] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][0], (mat).values[2][2], \
			dsSIMD4f_mul((mat).values[0][2], (mat).values[2][0])), invDet); \
		(result).values[1][2] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][2], (mat).values[1][0], \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[1][2])), invDet); \
		\
		(result).values[2][0] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[1][0], (mat).values[2][1], \
			dsSIMD4f_mul((mat).values[1][1], (mat).values[2][0])), invDet); \
		(result).values[2][1] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][1], (mat).values[2][0], \
			dsSIMD4f_mul((mat).values[0][0], (mat).values[2][1])), invDet); \
		(result).values[2][2] = dsSIMD4f_mul( \
			dsSIMD4f_fmsub((mat).values[0][0], (mat).values[1][1], \
			dsSIMD4f_mul((mat).values[0][1], (mat).values[1][0])), invDet); \
	} \
	while (0)
/// @endcond

inline void dsMatrix44x4f_mulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	result->values[0][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[0][1],
		dsSIMD4f_fmadd(a->values[2][0], b->values[0][2],
		dsSIMD4f_mul(a->values[3][0], b->values[0][3]))));
	result->values[0][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[0][1],
		dsSIMD4f_fmadd(a->values[2][1], b->values[0][2],
		dsSIMD4f_mul(a->values[3][1], b->values[0][3]))));
	result->values[0][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[0][1],
		dsSIMD4f_fmadd(a->values[2][2], b->values[0][2],
		dsSIMD4f_mul(a->values[3][2], b->values[0][3]))));
	result->values[0][3] = dsSIMD4f_fmadd(a->values[0][3], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][3], b->values[0][1],
		dsSIMD4f_fmadd(a->values[2][3], b->values[0][2],
		dsSIMD4f_mul(a->values[3][3], b->values[0][3]))));

	result->values[1][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[1][1],
		dsSIMD4f_fmadd(a->values[2][0], b->values[1][2],
		dsSIMD4f_mul(a->values[3][0], b->values[1][3]))));
	result->values[1][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[1][1],
		dsSIMD4f_fmadd(a->values[2][1], b->values[1][2],
		dsSIMD4f_mul(a->values[3][1], b->values[1][3]))));
	result->values[1][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[1][1],
		dsSIMD4f_fmadd(a->values[2][2], b->values[1][2],
		dsSIMD4f_mul(a->values[3][2], b->values[1][3]))));
	result->values[1][3] = dsSIMD4f_fmadd(a->values[0][3], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][3], b->values[1][1],
		dsSIMD4f_fmadd(a->values[2][3], b->values[1][2],
		dsSIMD4f_mul(a->values[3][3], b->values[1][3]))));

	result->values[2][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[2][1],
		dsSIMD4f_fmadd(a->values[2][0], b->values[2][2],
		dsSIMD4f_mul(a->values[3][0], b->values[2][3]))));
	result->values[2][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[2][1],
		dsSIMD4f_fmadd(a->values[2][1], b->values[2][2],
		dsSIMD4f_mul(a->values[3][1], b->values[2][3]))));
	result->values[2][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[2][1],
		dsSIMD4f_fmadd(a->values[2][2], b->values[2][2],
		dsSIMD4f_mul(a->values[3][2], b->values[2][3]))));
	result->values[2][3] = dsSIMD4f_fmadd(a->values[0][3], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][3], b->values[2][1],
		dsSIMD4f_fmadd(a->values[2][3], b->values[2][2],
		dsSIMD4f_mul(a->values[3][3], b->values[2][3]))));

	result->values[3][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][0], b->values[3][2],
		dsSIMD4f_mul(a->values[3][0], b->values[3][3]))));
	result->values[3][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][1], b->values[3][2],
		dsSIMD4f_mul(a->values[3][1], b->values[3][3]))));
	result->values[3][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][2], b->values[3][2],
		dsSIMD4f_mul(a->values[3][2], b->values[3][3]))));
	result->values[3][3] = dsSIMD4f_fmadd(a->values[0][3], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][3], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][3], b->values[3][2],
		dsSIMD4f_mul(a->values[3][3], b->values[3][3]))));
}

inline void dsMatrix44x4f_affineMulFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a,
	const dsMatrix44x4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	result->values[0][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[0][1],
		dsSIMD4f_mul(a->values[2][0], b->values[0][2])));
	result->values[0][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[0][1],
		dsSIMD4f_mul(a->values[2][1], b->values[0][2])));
	result->values[0][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[0][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[0][1],
		dsSIMD4f_mul(a->values[2][2], b->values[0][2])));
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[1][1],
		dsSIMD4f_mul(a->values[2][0], b->values[1][2])));
	result->values[1][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[1][1],
		dsSIMD4f_mul(a->values[2][1], b->values[1][2])));
	result->values[1][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[1][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[1][1],
		dsSIMD4f_mul(a->values[2][2], b->values[1][2])));
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[2][1],
		dsSIMD4f_mul(a->values[2][0], b->values[2][2])));
	result->values[2][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[2][1],
		dsSIMD4f_mul(a->values[2][1], b->values[2][2])));
	result->values[2][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[2][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[2][1],
		dsSIMD4f_mul(a->values[2][2], b->values[2][2])));
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_fmadd(a->values[0][0], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][0], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][0], b->values[3][2], a->values[3][0])));
	result->values[3][1] = dsSIMD4f_fmadd(a->values[0][1], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][1], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][1], b->values[3][2], a->values[3][1])));
	result->values[3][2] = dsSIMD4f_fmadd(a->values[0][2], b->values[3][0],
		dsSIMD4f_fmadd(a->values[1][2], b->values[3][1],
		dsSIMD4f_fmadd(a->values[2][2], b->values[3][2], a->values[3][2])));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_fastInvertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	result->values[0][0] = a->values[0][0];
	result->values[0][1] = a->values[1][0];
	result->values[0][2] = a->values[2][0];
	result->values[0][3] = zero;

	result->values[1][0] = a->values[0][1];
	result->values[1][1] = a->values[1][1];
	result->values[1][2] = a->values[2][1];
	result->values[1][3] = zero;

	result->values[2][0] = a->values[0][2];
	result->values[2][1] = a->values[1][2];
	result->values[2][2] = a->values[2][2];
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][0],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][0],
		dsSIMD4f_mul(a->values[3][2], result->values[2][0])));
	result->values[3][1] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][1],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][1],
		dsSIMD4f_mul(a->values[3][2], result->values[2][1])));
	result->values[3][2] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][2],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][2],
		dsSIMD4f_mul(a->values[3][2], result->values[2][2])));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_affineInvertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));
	dsMatrix33x4_invertImpl(*result, *a, invUpperDet);

	result->values[0][3] = zero;
	result->values[1][3] = zero;
	result->values[2][3] = zero;

	result->values[3][0] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][0],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][0],
		dsSIMD4f_mul(a->values[3][2], result->values[2][0])));
	result->values[3][1] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][1],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][1],
		dsSIMD4f_mul(a->values[3][2], result->values[2][1])));
	result->values[3][2] = dsSIMD4f_fnmsub(a->values[3][0], result->values[0][2],
		dsSIMD4f_fmadd(a->values[3][1], result->values[1][2],
		dsSIMD4f_mul(a->values[3][2], result->values[2][2])));
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_invertFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsSIMD4f det0 = dsMatrix33x4_determinantImpl(*a, 1, 2, 3, 1, 2, 3);
	dsSIMD4f det1 = dsMatrix33x4_determinantImpl(*a, 0, 2, 3, 1, 2, 3);
	dsSIMD4f det2 = dsMatrix33x4_determinantImpl(*a, 0, 1, 3, 1, 2, 3);
	dsSIMD4f det3 = dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 1, 2, 3);
	dsSIMD4f det = dsSIMD4f_sub(
		dsSIMD4f_fmadd(a->values[0][0], det0, dsSIMD4f_mul(a->values[2][0], det2)),
		dsSIMD4f_fmadd(a->values[1][0], det1, dsSIMD4f_mul(a->values[3][0], det3)));

	// Prefer more accurate divide.
	dsSIMD4f invDet = dsSIMD4f_div(dsSIMD4f_set1(1.0f), det);

	result->values[0][0] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[2][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][1], a->values[3][2]), a->values[1][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][1], a->values[1][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[3][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][1], a->values[1][2]), a->values[3][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[2][2]), a->values[1][3])))))),
		invDet);
	result->values[0][1] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[3][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][1], a->values[0][2]), a->values[3][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][1], a->values[2][2]), a->values[0][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[2][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][1], a->values[3][2]), a->values[0][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[0][2]), a->values[2][3])))))),
		invDet);
	result->values[0][2] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[1][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[3][2]), a->values[0][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][1], a->values[0][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[3][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[0][2]), a->values[3][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][1], a->values[1][2]), a->values[0][3])))))),
		invDet);
	result->values[0][3] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[2][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[0][2]), a->values[2][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[2][1], a->values[1][2]), a->values[0][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][1], a->values[1][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][1], a->values[2][2]), a->values[0][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][1], a->values[0][2]), a->values[1][3])))))),
		invDet);

	result->values[1][0] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[1][2]), a->values[3][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[2][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][2]), a->values[1][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][2]), a->values[2][3])))))),
		invDet);
	result->values[1][1] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][2]), a->values[0][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[0][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[0][2]), a->values[3][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][2]), a->values[0][3])))))),
		invDet);
	result->values[1][2] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][2]), a->values[3][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[1][2]), a->values[0][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][2]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][2]), a->values[0][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][2]), a->values[1][3])))))),
		invDet);
	result->values[1][3] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][2]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][2]), a->values[0][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[2][0], a->values[0][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][2]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][2]), a->values[2][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][2]), a->values[0][3])))))),
		invDet);

	result->values[2][0] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[1][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[3][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[1][3])))))),
		invDet);
	result->values[2][1] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[3][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[0][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[0][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[2][3])))))),
		invDet);
	result->values[2][2] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[3][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[0][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[3][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[0][3])))))),
		invDet);
	result->values[2][3] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[1][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[2][3],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[0][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[2][3],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[0][3],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[1][3])))))),
		invDet);

	result->values[3][0] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[2][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[3][2],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[1][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[3][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[1][2],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[2][2])))))),
		invDet);
	result->values[3][1] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[3][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[3][1]), a->values[0][2],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[2][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[2][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[3][2],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[2][1]), a->values[0][2])))))),
		invDet);
	result->values[3][2] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[3][1]), a->values[1][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[3][2],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[3][0], a->values[1][1]), a->values[0][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[3][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[3][1]), a->values[0][2],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[3][0], a->values[0][1]), a->values[1][2])))))),
		invDet);
	result->values[3][3] = dsSIMD4f_mul(
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[1][1]), a->values[2][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[2][1]), a->values[0][2],
		dsSIMD4f_fmsub(dsSIMD4f_mul(a->values[2][0], a->values[0][1]), a->values[1][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[0][0], a->values[2][1]), a->values[1][2],
		dsSIMD4f_fmadd(dsSIMD4f_mul(a->values[1][0], a->values[0][1]), a->values[2][2],
		dsSIMD4f_mul(dsSIMD4f_mul(a->values[2][0], a->values[1][1]), a->values[0][2])))))),
		invDet);
}

inline void dsMatrix44x4f_inverseTransposeFMA(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));

	result->values[0][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][1], a->values[2][2],
		dsSIMD4f_mul(a->values[1][2], a->values[2][1])), invUpperDet);
	result->values[0][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][2], a->values[2][0],
		dsSIMD4f_mul(a->values[1][0], a->values[2][2])), invUpperDet);
	result->values[0][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][0], a->values[2][1],
		dsSIMD4f_mul(a->values[1][1], a->values[2][0])), invUpperDet);
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][2], a->values[2][1],
		dsSIMD4f_mul(a->values[0][1], a->values[2][2])), invUpperDet);
	result->values[1][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][0], a->values[2][2],
		dsSIMD4f_mul(a->values[0][2], a->values[2][0])), invUpperDet);
	result->values[1][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][1], a->values[2][0],
		dsSIMD4f_mul(a->values[0][0], a->values[2][1])), invUpperDet);
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][1], a->values[1][2],
		dsSIMD4f_mul(a->values[0][2], a->values[1][1])), invUpperDet);
	result->values[2][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][2], a->values[1][0],
		dsSIMD4f_mul(a->values[0][0], a->values[1][2])), invUpperDet);
	result->values[2][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][0], a->values[1][1],
		dsSIMD4f_mul(a->values[0][1], a->values[1][0])), invUpperDet);
	result->values[2][3] = zero;

	result->values[3][0] = zero;
	result->values[3][1] = zero;
	result->values[3][2] = zero;
	result->values[3][3] = one;
}

inline void dsMatrix44x4f_invert33FMA(dsMatrix44x4f* result, const dsMatrix44x4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	const dsSIMD4f zero = dsSIMD4f_set1(0.0f);
	const dsSIMD4f one = dsSIMD4f_set1(1.0f);

	// Prefer more accurate divide.
	dsSIMD4f invUpperDet = dsSIMD4f_div(one, dsMatrix33x4_determinantImpl(*a, 0, 1, 2, 0, 1, 2));

	result->values[0][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][1], a->values[2][2],
		dsSIMD4f_mul(a->values[1][2], a->values[2][1])), invUpperDet);
	result->values[0][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][2], a->values[2][1],
		dsSIMD4f_mul(a->values[0][1], a->values[2][2])), invUpperDet);
	result->values[0][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][1], a->values[1][2],
		dsSIMD4f_mul(a->values[0][2], a->values[1][1])), invUpperDet);
	result->values[0][3] = zero;

	result->values[1][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][2], a->values[2][0],
		dsSIMD4f_mul(a->values[1][0], a->values[2][2])), invUpperDet);
	result->values[1][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][0], a->values[2][2],
		dsSIMD4f_mul(a->values[0][2], a->values[2][0])), invUpperDet);
	result->values[1][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][2], a->values[1][0],
		dsSIMD4f_mul(a->values[0][0], a->values[1][2])), invUpperDet);
	result->values[1][3] = zero;

	result->values[2][0] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[1][0], a->values[2][1],
		dsSIMD4f_mul(a->values[1][1], a->values[2][0])), invUpperDet);
	result->values[2][1] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][1], a->values[2][0],
		dsSIMD4f_mul(a->values[0][0], a->values[2][1])), invUpperDet);
	result->values[2][2] = dsSIMD4f_mul(dsSIMD4f_fmsub(a->values[0][0], a->values[1][1],
		dsSIMD4f_mul(a->values[0][1], a->values[1][0])), invUpperDet);
	result->values[2][3] = zero;

	result->values[3][0] = zero;
	result->values[3][1] = zero;
	result->values[3][2] = zero;
	result->values[3][3] = one;
}

#undef dsMatrix33x4_determinantImpl
#undef dsMatrix33x4_invertImpl

DS_SIMD_END()

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
