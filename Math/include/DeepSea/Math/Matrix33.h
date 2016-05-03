/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsMatrix33* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector2* type. All parameters are
 * by value.
 *
 * Unlike most of the vector macros, it is NOT safe to have the result be the same as one of the
 * parameters.
 *
 * The functions have different versions for the supported Vector2 types. These are used when
 * different underlying math functions are required based on the primitive type.
 */

/**
 * @brief Multiplies two matrices.
 * @param result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix33_mul(result, a, b) \
	do \
	{ \
		(result).values[0][0] = (a).values[0][0]*(b).values[0][0] + \
								(a).values[1][0]*(b).values[0][1] + \
								(a).values[2][0]*(b).values[0][2]; \
		(result).values[0][1] = (a).values[0][1]*(b).values[0][0] + \
								(a).values[1][1]*(b).values[0][1] + \
								(a).values[2][1]*(b).values[0][2]; \
		(result).values[0][2] = (a).values[0][2]*(b).values[0][0] + \
								(a).values[1][2]*(b).values[0][1] + \
								(a).values[2][2]*(b).values[0][2]; \
		\
		(result).values[1][0] = (a).values[0][0]*(b).values[1][0] + \
								(a).values[1][0]*(b).values[1][1] + \
								(a).values[2][0]*(b).values[1][2]; \
		(result).values[1][1] = (a).values[0][1]*(b).values[1][0] + \
								(a).values[1][1]*(b).values[1][1] + \
								(a).values[2][1]*(b).values[1][2]; \
		(result).values[1][2] = (a).values[0][2]*(b).values[1][0] + \
								(a).values[1][2]*(b).values[1][1] + \
								(a).values[2][2]*(b).values[1][2]; \
		\
		(result).values[2][0] = (a).values[0][0]*(b).values[2][0] + \
								(a).values[1][0]*(b).values[2][1] + \
								(a).values[2][0]*(b).values[2][2]; \
		(result).values[2][1] = (a).values[0][1]*(b).values[2][0] + \
								(a).values[1][1]*(b).values[2][1] + \
								(a).values[2][1]*(b).values[2][2]; \
		(result).values[2][2] = (a).values[0][2]*(b).values[2][0] + \
								(a).values[1][2]*(b).values[2][1] + \
								(a).values[2][2]*(b).values[2][2]; \
	} while (0)

/**
 * @brief Transforms a vector with a matrix.
 * @param result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix33_transform(result, mat, vec) \
	do \
	{ \
		(result).values[0] = (mat)[0][0]*(vec)[0] + \
							 (mat)[0][1]*(vec)[1] + \
							 (mat)[0][2]*(vec)[2]; \
		(result).values[1] = (mat)[1][0]*(vec)[0] + \
							 (mat)[1][1]*(vec)[1] + \
							 (mat)[1][2]*(vec)[2]; \
		(result).values[2] = (mat)[2][0]*(vec)[0] + \
							 (mat)[2][1]*(vec)[1] + \
							 (mat)[2][2]*(vec)[2]; \
	} while (0)

/**
 * @brief Sets a matrix to be identity.
 * @param result The matrix to hold
 */
#define dsMatrix33_identity(result) \
	do \
	{ \
		(result).value[0][0] = 1; \
		(result).value[0][1] = 0; \
		(result).value[0][2] = 0; \
		\
		(result).value[1][0] = 0; \
		(result).value[1][1] = 1; \
		(result).value[1][2] = 0; \
		\
		(result).value[2][0] = 0; \
		(result).value[2][1] = 0; \
		(result).value[2][2] = 1; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param result The transposed matrix. This may NOT be the same as a.
 * @param a The matrix to transpose.
 */
#define dsMatrix33_transpose(result, a) \
	do \
	{ \
		(result).value[0][0] = (a)[0][0]; \
		(result).value[0][1] = (a)[1][0]; \
		(result).value[0][2] = (a)[2][0]; \
		\
		(result).value[1][0] = (a)[0][1]; \
		(result).value[1][1] = (a)[1][1]; \
		(result).value[1][2] = (a)[2][1]; \
		\
		(result).value[2][0] = (a)[0][2]; \
		(result).value[2][1] = (a)[1][2]; \
		(result).value[2][2] = (a)[2][2]; \
	} while (0)

/**
 * @brief Gets the determinant of a matrix.
 * @param a The matrix to get the determinant of.
 * @return The determinant.
 */
#define dsMatrix33_determinant(a) dsMatrix33_determinantImpl(a, 0, 1, 2, 0, 1, 2)

/**
 * @brief Inverts an matrix that only contains a rotation and translation.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
#define dsMatrix33_fastInvert(result, a) \
	do \
	{ \
		(result).value[0][0] = (a)[0][0]; \
		(result).value[0][1] = (a)[1][0]; \
		(result).value[0][2] = 0; \
		\
		(result).value[1][0] = (a)[0][1]; \
		(result).value[1][1] = (a)[1][1]; \
		(result).value[1][2] = 0; \
		\
		(result).value[2][0] = -(a)[2][0]; \
		(result).value[2][1] = -(a)[2][1]; \
		(result).value[2][2] = 1; \
	} \
	while (0)

/**
 * @brief Inverts an affine matrix.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix33f_affineInvert(dsMatrix33f* result, dsMatrix33f* a);

/** @copydoc dsMatrix33d_invert() */
DS_MATH_EXPORT void dsMatrix33d_affineInvert(dsMatrix33d* result, dsMatrix33d* a);

/**
 * @brief Inverts a matrix.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix33f_invert(dsMatrix33f* result, dsMatrix33f* a);

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT void dsMatrix33d_invert(dsMatrix33d* result, dsMatrix33d* a);

/**
 * @brief Makes a rotation matrix.
 * @param result The matrix for the result.
 * @param angle The angle to rotate by in radians.
 */
DS_MATH_EXPORT void dsMatrix33f_makeRotate(dsMatrix33f* result, float angle);

/** @copydoc dsMatrix33f_makeRotate() */
DS_MATH_EXPORT void dsMatrix33d_makeRotate(dsMatrix33d* result, double angle);

/**
 * @brief Makes a translation matrix.
 * @param result The matrix for the result.
 * @param x The transition in the x axis.
 * @param y The transition in the y axis.
 */
DS_MATH_EXPORT void dsMatrix33f_makeTranslate(dsMatrix33f* result, float x, float y);

/** @copydoc dsMatrix33f_makeTranslate() */
DS_MATH_EXPORT void dsMatrix33d_makeTranslate(dsMatrix33d* result, double x, double y);

/**
 * @brief Makes a scale matrix.
 * @param result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 */
DS_MATH_EXPORT void dsMatrix33f_makeScale(dsMatrix33f* result, float x, float y);

/** @copydoc dsMatrix33f_makeScale() */
DS_MATH_EXPORT void dsMatrix33d_makeScale(dsMatrix33d* result, double x, double y);

/** @cond */
#define dsMatrix33_determinantImpl(a, i0, i1, i2, j0, j1, j2) \
	((a).values[i0][j0]*(a).values[i1][j1]*(a).values[i2][j2] + \
	 (a).values[i1][j0]*(a).values[i2][j1]*(a).values[i0][j2] + \
	 (a).values[i2][j0]*(a).values[i0][j1]*(a).values[i1][j2] - \
	 (a).values[i2][j0]*(a).values[i1][j1]*(a).values[i0][j2] - \
	 (a).values[i1][j0]*(a).values[i0][j1]*(a).values[i2][j2] - \
	 (a).values[i0][j0]*(a).values[i2][j1]*(a).values[i1][j2])
/** @endcond */

#ifdef __cplusplus
}
#endif
