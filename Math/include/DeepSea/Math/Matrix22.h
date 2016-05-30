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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Export.h>
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
 * implementation cannot be practically done within a macro.
 */

/**
 * @brief Sets a matrix to be identity.
 * @param result The matrix to hold
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
 * @brief Multiplies two matrices.
 * @param result The result of a*b. This may NOT be the same as a or b.
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
 * @param result The result of vec*mat. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix22_transform(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[0][1]*(vec).values[1]; \
		(result).values[1] = (mat).values[1][0]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1]; \
	} while (0)

/**
 * @brief Transforms a vector with a transposed matrix.
 * @param result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix22_transformTransposed(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[1][0]*(vec).values[1]; \
		(result).values[1] = (mat).values[0][1]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1]; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param result The transposed matrix. This may NOT be the same as a.
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
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix22f_invert(dsMatrix22f* result, dsMatrix22f* a);

/** @copydoc dsMatrix22f_invert() */
DS_MATH_EXPORT void dsMatrix22d_invert(dsMatrix22d* result, dsMatrix22d* a);

/**
 * @brief Makes a rotation matrix.
 * @param result The matrix for the result.
 * @param angle The angle to rotate by in radians.
 */
DS_MATH_EXPORT void dsMatrix22f_makeRotate(dsMatrix22f* result, float angle);

/** @copydoc dsMatrix22f_makeRotate() */
DS_MATH_EXPORT void dsMatrix22d_makeRotate(dsMatrix22d* result, double angle);

/**
 * @brief Makes a scale matrix.
 * @param result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 */
DS_MATH_EXPORT void dsMatrix22f_makeScale(dsMatrix22f* result, float x, float y);

/** @copydoc dsMatrix22f_makeScale() */
DS_MATH_EXPORT void dsMatrix22d_makeScale(dsMatrix22d* result, double x, double y);

#ifdef __cplusplus
}
#endif
