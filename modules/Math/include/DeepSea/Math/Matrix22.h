/*
 * Copyright 2016-2023 Aaron Barany
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
#include <DeepSea/Math/JacobiEigenvalues.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsMatrix22* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsMatrix33* type. All parameters are
 * by value. Unlike most of the vector macros, it is NOT safe to have the result be the same as one
 * of the parameters.
 *
 * The functions have different versions for the supported dsMatrix22 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * The dsMatrix22d functions may use SIMD operations when guaranteed to be available.
 *
 * @see dsMatrix22f dsMatrix22d
 */

/**
 * @brief Sets a matrix to be identity.
 * @param[out] result The matrix to hold the identity.
 */
#define dsMatrix22_identity(result) \
	do \
	{ \
		(result).values[0][0] = 1; \
		(result).values[0][1] = 0; \
		\
		(result).values[1][0] = 0; \
		(result).values[1][1] = 1; \
	} while (0)

/**
 * @brief Copies a 2x2 matrix.
 *
 * This can be used to populate a 2x2 matrix with a 4x4 or 3x3 matrix.
 *
 * @param[out] result The matrix to hold the copy.
 * @param a The matrix to copy.
 */
#define dsMatrix22_copy(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[0][1]; \
		\
		(result).values[1][0] = (a).values[1][0]; \
		(result).values[1][1] = (a).values[1][1]; \
	} while (0)

/**
 * @brief Multiplies two matrices.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix22_mul(result, a, b) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		DS_ASSERT(&(result) != (const void*)&(b)); \
		\
		(result).values[0][0] = (a).values[0][0]*(b).values[0][0] + \
								(a).values[1][0]*(b).values[0][1]; \
		(result).values[0][1] = (a).values[0][1]*(b).values[0][0] + \
								(a).values[1][1]*(b).values[0][1]; \
		\
		(result).values[1][0] = (a).values[0][0]*(b).values[1][0] + \
								(a).values[1][0]*(b).values[1][1]; \
		(result).values[1][1] = (a).values[0][1]*(b).values[1][0] + \
								(a).values[1][1]*(b).values[1][1]; \
	} while (0)

/**
 * @brief Transforms a vector with a matrix.
 * @param[out] result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix22_transform(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[1][0]*(vec).values[1]; \
		(result).values[1] = (mat).values[0][1]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1]; \
	} while (0)

/**
 * @brief Transforms a vector with a transposed matrix.
 * @param[out] result The result of vec*mat. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix22_transformTransposed(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[0][1]*(vec).values[1]; \
		(result).values[1] = (mat).values[1][0]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1]; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param[out] result The transposed matrix. This may NOT be the same as a.
 * @param a The matrix to transpose.
 */
#define dsMatrix22_transpose(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[1][0]; \
		\
		(result).values[1][0] = (a).values[0][1]; \
		(result).values[1][1] = (a).values[1][1]; \
	} while (0)

/**
 * @brief Gets the determinant of a matrix.
 * @param a The matrix to get the determinant of.
 * @return The determinant.
 */
#define dsMatrix22_determinant(a) \
	((a).values[0][0]*(a).values[1][1] - (a).values[0][1]*(a).values[1][0])

/**
 * @brief Inverts a matrix.
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix22f_invert(dsMatrix22f* result, const dsMatrix22f* a);

/** @copydoc dsMatrix22f_invert() */
DS_MATH_EXPORT void dsMatrix22d_invert(dsMatrix22d* result, const dsMatrix22d* a);

/**
 * @brief Makes a rotation matrix.
 * @param[out] result The matrix for the result.
 * @param angle The angle to rotate by in radians.
 */
DS_MATH_EXPORT void dsMatrix22f_makeRotate(dsMatrix22f* result, float angle);

/** @copydoc dsMatrix22f_makeRotate() */
DS_MATH_EXPORT void dsMatrix22d_makeRotate(dsMatrix22d* result, double angle);

/**
 * @brief Makes a scale matrix.
 * @param[out] result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 */
DS_MATH_EXPORT void dsMatrix22f_makeScale(dsMatrix22f* result, float x, float y);

/** @copydoc dsMatrix22f_makeScale() */
DS_MATH_EXPORT void dsMatrix22d_makeScale(dsMatrix22d* result, double x, double y);

/**
 * @brief Extracts eigenvalues for a symmetric matrix using Jacobi iteration.
 * @param[out] outEigenvectors The resulting eigenvectors.
 * @param[out] outEigenvalues The resulting eigenvalues.
 * @param a The matrix to extract the eigenvalues for.
 * @return False if the eigenvalues couldn't be extracted.
 */
DS_MATH_EXPORT inline bool dsMatrix22f_jacobiEigenvalues(dsMatrix22f* outEigenvectors,
	dsVector2f* outEigenvalues, const dsMatrix22f* a);

/** @copydoc dsMatrix22f_jacobiEigenvalues() */
DS_MATH_EXPORT inline bool dsMatrix22d_jacobiEigenvalues(dsMatrix22d* outEigenvectors,
	dsVector2d* outEigenvalues, const dsMatrix22d* a);

/**
 * @brief Sorts eigenvalues and corresponding eigenvectors from largest to smallest.
 * @param[inout] eigenvectors The eigenvectors to sort.
 * @param[inout] eigenvalues The eigenvalues to sort.
 */
DS_MATH_EXPORT inline void dsMatrix22f_sortEigenvalues(dsMatrix22f* eigenvectors,
	dsVector2f* eigenvalues);

/** @copydoc dsMatrix22f_sortEigenvalues() */
DS_MATH_EXPORT inline void dsMatrix22d_sortEigenvalues(dsMatrix22d* eigenvectors,
	dsVector2d* eigenvalues);

/** @copydoc dsMatrix22_identity() */
DS_MATH_EXPORT inline void dsMatrix22f_identity(dsMatrix22f* result)
{
	DS_ASSERT(result);
	dsMatrix22_identity(*result);
}

/** @copydoc dsMatrix22_identity() */
DS_MATH_EXPORT inline void dsMatrix22d_identity(dsMatrix22d* result)
{
	DS_ASSERT(result);
	dsMatrix22_identity(*result);
}

/** @copydoc dsMatrix22_mul() */
DS_MATH_EXPORT inline void dsMatrix22f_mul(dsMatrix22f* result, const dsMatrix22f* a,
	const dsMatrix22f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix22_mul(*result, *a, *b);
}

/** @copydoc dsMatrix22_mul() */
DS_MATH_EXPORT inline void dsMatrix22d_mul(dsMatrix22d* result, const dsMatrix22d* a,
	const dsMatrix22d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	dsSIMD2d mul0 = dsSIMD2d_set1(b->columns[0].x);
	dsSIMD2d mul1 = dsSIMD2d_set1(b->columns[0].y);
	result->columns[0].simd = dsSIMD2d_fmadd(a->columns[0].simd, mul0,
		dsSIMD2d_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD2d_set1(b->columns[1].x);
	mul1 = dsSIMD2d_set1(b->columns[1].y);
	result->columns[1].simd = dsSIMD2d_fmadd(a->columns[0].simd, mul0,
		dsSIMD2d_mul(a->columns[1].simd, mul1));
#else
	dsSIMD2d mul0 = dsSIMD2d_set1(b->columns[0].x);
	dsSIMD2d mul1 = dsSIMD2d_set1(b->columns[0].y);
	result->columns[0].simd = dsSIMD2d_add(dsSIMD2d_mul(a->columns[0].simd, mul0),
		dsSIMD2d_mul(a->columns[1].simd, mul1));

	mul0 = dsSIMD2d_set1(b->columns[1].x);
	mul1 = dsSIMD2d_set1(b->columns[1].y);
	result->columns[1].simd = dsSIMD2d_add(dsSIMD2d_mul(a->columns[0].simd, mul0),
		dsSIMD2d_mul(a->columns[1].simd, mul1));
#endif
#else
	dsMatrix22_mul(*result, *a, *b);
#endif
}

/** @copydoc dsMatrix22_transform() */
DS_MATH_EXPORT inline void dsMatrix22f_transform(dsVector2f* result, const dsMatrix22f* mat,
	const dsVector2f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix22_transform(*result, *mat, *vec);
}

/** @copydoc dsMatrix22_transform() */
DS_MATH_EXPORT inline void dsMatrix22d_transform(dsVector2d* result, const dsMatrix22d* mat,
	const dsVector2d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d x = dsSIMD2d_set1(vec->x);
	dsSIMD2d y = dsSIMD2d_set1(vec->y);

#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD2d_fmadd(mat->columns[0].simd, x, dsSIMD2d_mul(mat->columns[1].simd, y));
#else
	result->simd = dsSIMD2d_add(dsSIMD2d_mul(mat->columns[0].simd, x),
		dsSIMD2d_mul(mat->columns[1].simd, y));
#endif
#else
	dsMatrix22_transform(*result, *mat, *vec);
#endif
}

/** @copydoc dsMatrix22_transformTransposed() */
DS_MATH_EXPORT inline void dsMatrix22f_transformTransposed(dsVector2f* result,
	const dsMatrix22f* mat, const dsVector2f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix22_transformTransposed(*result, *mat, *vec);
}

/** @copydoc dsMatrix22_transformTransposed() */
DS_MATH_EXPORT inline void dsMatrix22d_transformTransposed(dsVector2d* result,
	const dsMatrix22d* mat, const dsVector2d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d x = dsSIMD2d_set1(vec->x);
	dsSIMD2d y = dsSIMD2d_set1(vec->y);

	dsSIMD2d col0 = mat->columns[0].simd;
	dsSIMD2d col1 = mat->columns[1].simd;
	dsSIMD2d_transpose(col0, col1);

#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD2d_fmadd(col0, x, dsSIMD2d_mul(col1, y));
#else
	result->simd = dsSIMD2d_add(dsSIMD2d_mul(col0, x), dsSIMD2d_mul(col1, y));
#endif
#else
	dsMatrix22_transform(*result, *mat, *vec);
#endif
}

/** @copydoc dsMatrix22_transpose() */
DS_MATH_EXPORT inline void dsMatrix22f_transpose(dsMatrix22f* result, const dsMatrix22f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix22_transpose(*result, *a);
}

/** @copydoc dsMatrix22_transpose() */
DS_MATH_EXPORT inline void dsMatrix22d_transpose(dsMatrix22d* result, const dsMatrix22d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0] = a->columns[0];
	result->columns[1] = a->columns[1];
	dsSIMD2d_transpose(result->columns[0].simd, result->columns[1].simd);
#else
	dsMatrix22_transpose(*result, *a);
#endif
}

/** @copydoc dsMatrix22_determinant() */
DS_MATH_EXPORT inline float dsMatrix22f_determinant(const dsMatrix22f* a)
{
	DS_ASSERT(a);
	return dsMatrix22_determinant(*a);
}

/** @copydoc dsMatrix22_determinant() */
DS_MATH_EXPORT inline double dsMatrix22d_determinant(const dsMatrix22d* a)
{
	DS_ASSERT(a);
	return dsMatrix22_determinant(*a);
}

inline bool dsMatrix22f_jacobiEigenvalues(dsMatrix22f* outEigenvectors, dsVector2f* outEigenvalues,
	const dsMatrix22f* a)
{
	return dsJacobiEigenvaluesClassicf((float*)outEigenvectors, (float*)outEigenvalues,
		(const float*)a, 2, 1);
}

inline bool dsMatrix22d_jacobiEigenvalues(dsMatrix22d* outEigenvectors, dsVector2d* outEigenvalues,
	const dsMatrix22d* a)
{
	return dsJacobiEigenvaluesClassicd((double*)outEigenvectors, (double*)outEigenvalues,
		(const double*)a, 2, 1);
}

inline void dsMatrix22f_sortEigenvalues(dsMatrix22f* eigenvectors, dsVector2f* eigenvalues)
{
	dsSortEigenvaluesf((float*)eigenvectors, (float*)eigenvalues, 2);
}

inline void dsMatrix22d_sortEigenvalues(dsMatrix22d* eigenvectors, dsVector2d* eigenvalues)
{
	dsSortEigenvaluesd((double*)eigenvectors, (double*)eigenvalues, 2);
}

#ifdef __cplusplus
}
#endif
