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

#include <DeepSea/Math/SIMD/Matrix33xSIMD.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix33.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating 3x3 matrices aligned for SIMD usage.
 *
 * This allows for SIMD operations when supported, allowing for faster processing in the vast
 * majority of situations. The value of the last element is undefined for any results. Operations
 * are only provided for floating-point types, as integer types are more limited in the availability
 * of operations.
 *
 * @see dsMatrix33xf dsMatrix33xd
 */

/** @copydoc dsMatrix33_identity() */
DS_MATH_EXPORT inline void dsMatrix33xf_identity(dsMatrix33xf* result)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->columns[0].simd = dsSIMD4f_set4(1.0f, 0.0f, 0.0f, 0.0f);
	result->columns[1].simd = dsSIMD4f_set4(0.0f, 1.0f, 0.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
#else
	dsMatrix33_identity(*result);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_identity() */
DS_MATH_EXPORT inline void dsMatrix33xd_identity(dsMatrix33xd* result)
{
	DS_ASSERT(result);
#if DS_SIMD_PREFER_DOUBLE4
	result->columns[0].simd = dsSIMD4d_set4(1.0, 0.0, 0.0, 0.0);
	result->columns[1].simd = dsSIMD4d_set4(0.0, 1.0, 0.0, 0.0);
	result->columns[2].simd = dsSIMD4d_set4(0.0, 0.0, 1.0, 0.0);
#elif DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0].simd2[0] = dsSIMD2d_set2(1.0f, 0.0f);
	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0f);
	result->columns[1].simd2[0] = dsSIMD2d_set2(0.0f, 1.0f);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0f);
	result->columns[2].simd2[0] = dsSIMD2d_set1(0.0f);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0f, 0.0f);
#else
	dsMatrix33_identity(*result);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_mul() */
DS_MATH_EXPORT inline void dsMatrix33xf_mul(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_mulFMA(result, a, b);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_mulSIMD(result, a, b);
#else
	dsMatrix33_mul(*result, *a, *b);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_mul() */
DS_MATH_EXPORT inline void dsMatrix33xd_mul(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_mulSIMD4(result, a, b);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_mulFMA2(result, a, b);
#else
	dsMatrix33xd_mulSIMD2(result, a, b);
#endif
#else
	dsMatrix33_mul(*result, *a, *b);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_affineMul() */
DS_MATH_EXPORT inline void dsMatrix33xf_affineMul(
	dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_affineMulFMA(result, a, b);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_affineMulSIMD(result, a, b);
#else
	dsMatrix33_affineMul(*result, *a, *b);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_mul() */
DS_MATH_EXPORT inline void dsMatrix33xd_affineMul(
	dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_affineMulSIMD4(result, a, b);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_affineMulFMA2(result, a, b);
#else
	dsMatrix33xd_affineMulSIMD2(result, a, b);
#endif
#else
	dsMatrix33_affineMul(*result, *a, *b);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transform() */
DS_MATH_EXPORT inline void dsMatrix33xf_transform(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_transformFMA(result, mat, vec);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_transformSIMD(result, mat, vec);
#else
	dsMatrix33_transform(*result, *mat, *vec);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transform() */
DS_MATH_EXPORT inline void dsMatrix33xd_transform(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_transformSIMD4(result, mat, vec);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_transformFMA2(result, mat, vec);
#else
	dsMatrix33xd_transformSIMD2(result, mat, vec);
#endif
#else
	dsMatrix33_transform(*result, *mat, *vec);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transform() */
DS_MATH_EXPORT inline void dsMatrix33xf_transformTransposed(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_transformTransposedFMA(result, mat, vec);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_transformTransposedSIMD(result, mat, vec);
#else
	dsMatrix33_transformTransposed(*result, *mat, *vec);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transform() */
DS_MATH_EXPORT inline void dsMatrix33xd_transformTransposed(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	DS_ASSERT(result != vec);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_transformTransposedSIMD4(result, mat, vec);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_transformTransposedFMA2(result, mat, vec);
#else
	dsMatrix33xd_transformTransposedSIMD2(result, mat, vec);
#endif
#else
	dsMatrix33_transformTransposed(*result, *mat, *vec);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transpose() */
DS_MATH_EXPORT inline void dsMatrix33xf_transpose(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_transposeSIMD(result, a);
#else
	dsMatrix33_transpose(*result, *a);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_transpose() */
DS_MATH_EXPORT inline void dsMatrix33xd_transpose(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_transposeSIMD4(result, a);
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsMatrix33xd_transposeSIMD2(result, a);
#else
	dsMatrix33_transpose(*result, *a);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33_determinant() */
DS_MATH_EXPORT inline float dsMatrix33xf_determinant(const dsMatrix33xf* a)
{
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FMA
	return dsMatrix33xf_determinantFMA(a);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsMatrix33xf_determinantSIMD(a);
#else
	return dsMatrix33_determinant(*a);
#endif
}

/** @copydoc dsMatrix33_determinant() */
DS_MATH_EXPORT inline double dsMatrix33xd_determinant(const dsMatrix33xd* a)
{
	DS_ASSERT(a);
#if DS_SIMD_PREFER_DOUBLE4
	return dsMatrix33xd_determinantSIMD4(a);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	return dsMatrix33xd_determinantFMA2(a);
#else
	return dsMatrix33xd_determinantSIMD2(a);
#endif
#else
	return dsMatrix33_determinant(*a);
#endif
}

/** @copydoc dsMatrix33_fastInvert() */
DS_MATH_EXPORT inline void dsMatrix33xf_fastInvert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_fastInvertFMA(result, a);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_fastInvertSIMD(result, a);
#else
	dsMatrix33_fastInvert(*result, *a);
#endif
}

/** @copydoc dsMatrix33_fastInvert() */
DS_MATH_EXPORT inline void dsMatrix33xd_fastInvert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_fastInvertSIMD4(result, a);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_fastInvertFMA2(result, a);
#else
	dsMatrix33xd_fastInvertSIMD2(result, a);
#endif
#else
	dsMatrix33_fastInvert(*result, *a);
#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33f_affineInvert() */
DS_MATH_EXPORT inline void dsMatrix33xf_affineInvert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_affineInvertFMA(result, a);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_affineInvertSIMD(result, a);
#else
	float upperDet = a->values[0][0]*a->values[1][1] - a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	float invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[0][1]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[1][0]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = -a->values[2][0]*result->values[0][0] -
		a->values[2][1]*result->values[1][0];
	result->values[2][1] = -a->values[2][0]*result->values[0][1] -
		a->values[2][1]*result->values[1][1];
	result->values[2][2] = 1;

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33f_affineInvert() */
DS_MATH_EXPORT inline void dsMatrix33xd_affineInvert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_affineInvertSIMD4(result, a);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_affineInvertFMA2(result, a);
#else
	dsMatrix33xd_affineInvertSIMD2(result, a);
#endif
#else
	double upperDet = a->values[0][0]*a->values[1][1] - a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	double invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[0][1]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[1][0]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = -a->values[2][0]*result->values[0][0] -
		a->values[2][1]*result->values[1][0];
	result->values[2][1] = -a->values[2][0]*result->values[0][1] -
		a->values[2][1]*result->values[1][1];
	result->values[2][2] = 1;

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT inline void dsMatrix33xf_invert(dsMatrix33xf* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xf_invertFMA(result, a);
#elif DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_invertSIMD(result, a);
#else
	float det = dsMatrix33_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;
	dsMatrix33_invertImpl(*result, *a, invDet);

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT inline void dsMatrix33xd_invert(dsMatrix33xd* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_invertSIMD4(result, a);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsMatrix33xd_invertFMA2(result, a);
#else
	dsMatrix33xd_invertSIMD2(result, a);
#endif
#else
	double det = dsMatrix33_determinant(*a);
	DS_ASSERT(det != 0);
	double invDet = 1/det;
	dsMatrix33_invertImpl(*result, *a, invDet);

#if DS_HAS_SIMD
	// Avoid potential subnormal values with uninitialized memory if used by SIMD later.
	result->columns[0].w = 0;
	result->columns[1].w = 0;
	result->columns[2].w = 0;
#endif
#endif
}

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT inline void dsMatrix33xf_inverseTranspose(dsMatrix22f* result, const dsMatrix33xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	dsMatrix33xf_inverseTransposeSIMD(result, a);
#else
	dsMatrix22f temp, inverse;
	dsMatrix22_copy(temp, *a);
	dsMatrix22f_invert(&inverse, &temp);
	dsMatrix22_transpose(*result, inverse);
#endif
}

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT inline void dsMatrix33xd_inverseTranspose(dsMatrix22d* result, const dsMatrix33xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_PREFER_DOUBLE4
	dsMatrix33xd_inverseTransposeSIMD4(result, a);
#elif DS_SIMD_ALWAYS_DOUBLE2
	dsMatrix33xd_inverseTransposeSIMD2(result, a);
#else
	dsMatrix22d temp, inverse;
	dsMatrix22_copy(temp, *a);
	dsMatrix22d_invert(&inverse, &temp);
	dsMatrix22_transpose(*result, inverse);
#endif
}

/** @copydoc dsMatrix33f_makeRotate() */
DS_MATH_EXPORT void dsMatrix33xf_makeRotate(dsMatrix33xf* result, float angle);

/** @copydoc dsMatrix33f_makeRotate() */
DS_MATH_EXPORT void dsMatrix33xd_makeRotate(dsMatrix33xd* result, double angle);

/** @copydoc dsMatrix33f_makeRotate3D() */
DS_MATH_EXPORT void dsMatrix33xf_makeRotate3D(dsMatrix33xf* result, float x, float y, float z);

/** @copydoc dsMatrix33f_makeRotate3D() */
DS_MATH_EXPORT void dsMatrix33xd_makeRotate3D(dsMatrix33xd* result, double x, double y, double z);

/** @copydoc dsMatrix33f_makeRotate3DAxisAngle() */
DS_MATH_EXPORT void dsMatrix33xf_makeRotate3DAxisAngle(
	dsMatrix33xf* result, const dsVector3f* axis, float angle);

/** @copydoc dsMatrix33f_makeRotate3DAxisAngle() */
DS_MATH_EXPORT void dsMatrix33xd_makeRotate3DAxisAngle(
	dsMatrix33xd* result, const dsVector3d* axis, double angle);

/** @copydoc dsMatrix33f_makeTranslate() */
DS_MATH_EXPORT inline void dsMatrix33xf_makeTranslate(dsMatrix33xf* result, float x, float y)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->columns[0].simd = dsSIMD4f_set4(1.0f, 0.0f, 0.0f, 0.0f);
	result->columns[1].simd = dsSIMD4f_set4(0.0f, 1.0f, 0.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_set4(x, y, 1.0f, 0.0f);
#else
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = x;
	result->values[2][1] = y;
	result->values[2][2] = 1;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_makeTranslate() */
DS_MATH_EXPORT inline void dsMatrix33xd_makeTranslate(dsMatrix33xd* result, double x, double y)
{
	DS_ASSERT(result);
#if DS_SIMD_PREFER_DOUBLE4
	result->columns[0].simd = dsSIMD4d_set4(1.0, 0.0, 0.0, 0.0);
	result->columns[1].simd = dsSIMD4d_set4(0.0, 1.0, 0.0, 0.0);
	result->columns[2].simd = dsSIMD4d_set4(x, y, 1.0, 0.0);
#elif DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0].simd2[0] = dsSIMD2d_set2(1.0, 0.0);
	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[0] = dsSIMD2d_set2(0.0, 1.0);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[0] = dsSIMD2d_set2(x, y);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
#else
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = x;
	result->values[2][1] = y;
	result->values[2][2] = 1;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_makeScale() */
DS_MATH_EXPORT inline void dsMatrix33xf_makeScale(dsMatrix33xf* result, float x, float y)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->columns[0].simd = dsSIMD4f_set4(x, 0.0f, 0.0f, 0.0f);
	result->columns[1].simd = dsSIMD4f_set4(0.0f, y, 0.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
#else
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_makeScale() */
DS_MATH_EXPORT inline void dsMatrix33xd_makeScale(dsMatrix33xd* result, double x, double y)
{
	DS_ASSERT(result);
#if DS_SIMD_PREFER_DOUBLE4
	result->columns[0].simd = dsSIMD4d_set4(x, 0.0, 0.0, 0.0);
	result->columns[1].simd = dsSIMD4d_set4(0.0, y, 0.0, 0.0);
	result->columns[2].simd = dsSIMD4d_set4(0.0, 0.0, 1.0, 0.0);
#elif DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0].simd2[0] = dsSIMD2d_set2(x, 0.0);
	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[0] = dsSIMD2d_set2(0.0, y);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[0] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
#else
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_makeScale3D() */
DS_MATH_EXPORT inline void dsMatrix33xf_makeScale3D(dsMatrix33xf* result, float x, float y, float z)
{
	DS_ASSERT(result);
#if DS_SIMD_ALWAYS_FLOAT4
	result->columns[0].simd = dsSIMD4f_set4(x, 0.0f, 0.0f, 0.0f);
	result->columns[1].simd = dsSIMD4f_set4(0.0f, y, 0.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_set4(0.0f, 0.0f, z, 0.0f);
#else
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_makeScale3D() */
DS_MATH_EXPORT inline void dsMatrix33xd_makeScale3D(
	dsMatrix33xd* result, double x, double y, double z)
{
	DS_ASSERT(result);
#if DS_SIMD_PREFER_DOUBLE4
	result->columns[0].simd = dsSIMD4d_set4(x, 0.0, 0.0, 0.0);
	result->columns[1].simd = dsSIMD4d_set4(0.0, y, 0.0, 0.0);
	result->columns[2].simd = dsSIMD4d_set4(0.0, 0.0, z, 0.0);
#elif DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0].simd2[0] = dsSIMD2d_set2(x, 0.0);
	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[0] = dsSIMD2d_set2(0.0, y);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[0] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(z, 0.0);
#else
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
	result->values[2][3] = 0;
#endif
}

/** @copydoc dsMatrix33f_jacobiEigenvalues() */
DS_MATH_EXPORT inline bool dsMatrix33xf_jacobiEigenvalues(
	dsMatrix33xf* outEigenvectors, dsVector3xf* outEigenvalues, const dsMatrix33xf* a)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(a);

	bool result = dsJacobiEigenvaluesClassicf(
		(float*)outEigenvectors, (float*)outEigenvalues, (const float*)a, 3, 1, 6);

	// Avoid potential subnormal values with uninitialized memory.
	outEigenvectors->columns[0].w = 0;
	outEigenvectors->columns[1].w = 0;
	outEigenvectors->columns[2].w = 0;
	outEigenvalues->w = 0;
	return result;
}

/** @copydoc dsMatrix33f_jacobiEigenvalues() */
DS_MATH_EXPORT inline bool dsMatrix33xd_jacobiEigenvalues(
	dsMatrix33xd* outEigenvectors, dsVector3xd* outEigenvalues, const dsMatrix33xd* a)
{
	DS_ASSERT(outEigenvectors);
	DS_ASSERT(outEigenvalues);
	DS_ASSERT(a);

	bool result = dsJacobiEigenvaluesClassicd(
		(double*)outEigenvectors, (double*)outEigenvalues, (const double*)a, 3, 1, 12);

	// Avoid potential subnormal values with uninitialized memory.
	outEigenvectors->columns[0].w = 0;
	outEigenvectors->columns[1].w = 0;
	outEigenvectors->columns[2].w = 0;
	outEigenvalues->w = 0;
	return result;
}

/** @copydoc dsMatrix33f_sortEigenvalues() */
DS_MATH_EXPORT inline void dsMatrix33xf_sortEigenvalues(
	dsMatrix33xf* eigenvectors, dsVector3xf* eigenvalues)
{
	dsSortEigenvaluesf((float*)eigenvectors, (float*)eigenvalues, 3, 1);
}

/** @copydoc dsMatrix33f_sortEigenvalues() */
DS_MATH_EXPORT inline void dsMatrix33xd_sortEigenvalues(
	dsMatrix33xd* eigenvectors, dsVector3xd* eigenvalues)
{
	dsSortEigenvaluesd((double*)eigenvectors, (double*)eigenvalues, 3, 1);
}

#ifdef __cplusplus
}
#endif
