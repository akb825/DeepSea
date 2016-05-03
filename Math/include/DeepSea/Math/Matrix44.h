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
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsMatrix44* structures.
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
#define dsMatrix44_mul(result, a, b) \
	do \
	{ \
		(result).values[0][0] = (a).values[0][0]*(b).values[0][0] + \
								(a).values[1][0]*(b).values[0][1] + \
								(a).values[2][0]*(b).values[0][2] + \
								(a).values[3][0]*(b).values[0][3]; \
		(result).values[0][1] = (a).values[0][1]*(b).values[0][0] + \
								(a).values[1][1]*(b).values[0][1] + \
								(a).values[2][1]*(b).values[0][2] + \
								(a).values[3][1]*(b).values[0][3]; \
		(result).values[0][2] = (a).values[0][2]*(b).values[0][0] + \
								(a).values[1][2]*(b).values[0][1] + \
								(a).values[2][2]*(b).values[0][2] + \
								(a).values[3][2]*(b).values[0][3]; \
		(result).values[0][3] = (a).values[0][3]*(b).values[0][0] + \
								(a).values[1][3]*(b).values[0][1] + \
								(a).values[2][3]*(b).values[0][2]+ \
								(a).values[3][3]*(b).values[0][3]; \
		\
		(result).values[1][0] = (a).values[0][0]*(b).values[1][0] + \
								(a).values[1][0]*(b).values[1][1] + \
								(a).values[2][0]*(b).values[1][2] + \
								(a).values[3][0]*(b).values[1][3]; \
		(result).values[1][1] = (a).values[0][1]*(b).values[1][0] + \
								(a).values[1][1]*(b).values[1][1] + \
								(a).values[2][1]*(b).values[1][2] + \
								(a).values[3][1]*(b).values[1][3]; \
		(result).values[1][2] = (a).values[0][2]*(b).values[1][0] + \
								(a).values[1][2]*(b).values[1][1] + \
								(a).values[2][2]*(b).values[1][2] + \
								(a).values[3][2]*(b).values[1][3]; \
		(result).values[1][3] = (a).values[0][3]*(b).values[1][0] + \
								(a).values[1][3]*(b).values[1][1] + \
								(a).values[2][3]*(b).values[1][2] + \
								(a).values[3][3]*(b).values[1][3]; \
		\
		(result).values[2][0] = (a).values[0][0]*(b).values[2][0] + \
								(a).values[1][0]*(b).values[2][1] + \
								(a).values[2][0]*(b).values[2][2] + \
								(a).values[3][0]*(b).values[2][3]; \
		(result).values[2][1] = (a).values[0][1]*(b).values[2][0] + \
								(a).values[1][1]*(b).values[2][1] + \
								(a).values[2][1]*(b).values[2][2] + \
								(a).values[3][1]*(b).values[2][3]; \
		(result).values[2][2] = (a).values[0][2]*(b).values[2][0] + \
								(a).values[1][2]*(b).values[2][1] + \
								(a).values[2][2]*(b).values[2][2] + \
								(a).values[3][2]*(b).values[2][3]; \
		(result).values[2][3] = (a).values[0][3]*(b).values[2][0] + \
								(a).values[1][3]*(b).values[2][1] + \
								(a).values[2][3]*(b).values[2][2] + \
								(a).values[3][3]*(b).values[2][3]; \
		\
		(result).values[3][0] = (a).values[0][0]*(b).values[3][0] + \
								(a).values[1][0]*(b).values[3][1] + \
								(a).values[2][0]*(b).values[3][2] + \
								(a).values[3][0]*(b).values[3][3]; \
		(result).values[3][1] = (a).values[0][1]*(b).values[3][0] + \
								(a).values[1][1]*(b).values[3][1] + \
								(a).values[2][1]*(b).values[3][2] + \
								(a).values[3][1]*(b).values[3][3]; \
		(result).values[3][2] = (a).values[0][2]*(b).values[3][0] + \
								(a).values[1][2]*(b).values[3][1] + \
								(a).values[2][2]*(b).values[3][2] + \
								(a).values[3][2]*(b).values[3][3]; \
		(result).values[3][3] = (a).values[0][3]*(b).values[3][0] + \
								(a).values[1][3]*(b).values[3][1] + \
								(a).values[2][3]*(b).values[3][2] + \
								(a).values[3][3]*(b).values[3][3]; \
	} while (0)

/**
 * @brief Transforms a vector with a matrix.
 * @param result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix44_transform(result, mat, vec) \
	do \
	{ \
		(result).values[0] = (mat)[0][0]*(vec)[0] + \
							 (mat)[0][1]*(vec)[1] + \
							 (mat)[0][2]*(vec)[2] + \
							 (mat)[0][3]*(vec)[3]; \
		(result).values[1] = (mat)[1][0]*(vec)[0] + \
							 (mat)[1][1]*(vec)[1] + \
							 (mat)[1][2]*(vec)[2] + \
							 (mat)[1][3]*(vec)[3]; \
		(result).values[2] = (mat)[2][0]*(vec)[0] + \
							 (mat)[2][1]*(vec)[1] + \
							 (mat)[2][2]*(vec)[2] + \
							 (mat)[2][3]*(vec)[3]; \
		(result).values[3] = (mat)[2][0]*(vec)[0] + \
							 (mat)[2][1]*(vec)[1] + \
							 (mat)[2][2]*(vec)[2] + \
							 (mat)[2][3]*(vec)[3]; \
	} while (0)

/**
 * @brief Sets a matrix to be identity.
 * @param result The matrix to hold
 */
#define dsMatrix44_identity(result) \
	do \
	{ \
		(result).value[0][0] = 1; \
		(result).value[0][1] = 0; \
		(result).value[0][2] = 0; \
		(result).value[0][3] = 0; \
		\
		(result).value[1][0] = 0; \
		(result).value[1][1] = 1; \
		(result).value[1][2] = 0; \
		(result).value[1][3] = 0; \
		\
		(result).value[2][0] = 0; \
		(result).value[2][1] = 0; \
		(result).value[2][2] = 1; \
		(result).value[2][3] = 0; \
		\
		(result).value[3][0] = 0; \
		(result).value[3][1] = 0; \
		(result).value[3][2] = 0; \
		(result).value[3][3] = 1; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param result The transposed matrix. This may NOT be the same as a.
 * @param a The matrix to transpose.
 */
#define dsMatrix44_transpose(result, a) \
	do \
	{ \
		(result).value[0][0] = (a)[0][0]; \
		(result).value[0][1] = (a)[1][0]; \
		(result).value[0][2] = (a)[2][0]; \
		(result).value[0][3] = (a)[3][0]; \
		\
		(result).value[1][0] = (a)[0][1]; \
		(result).value[1][1] = (a)[1][1]; \
		(result).value[1][2] = (a)[2][1]; \
		(result).value[1][3] = (a)[3][1]; \
		\
		(result).value[2][0] = (a)[0][2]; \
		(result).value[2][1] = (a)[1][2]; \
		(result).value[2][2] = (a)[2][2]; \
		(result).value[2][3] = (a)[3][2]; \
		\
		(result).value[3][0] = (a)[0][3]; \
		(result).value[3][1] = (a)[1][3]; \
		(result).value[3][2] = (a)[2][3]; \
		(result).value[3][3] = (a)[3][3]; \
	} while (0)

/**
 * @brief Gets the determinant of a matrix.
 * @param a The matrix to get the determinant of.
 * @return The determinant.
 */
#define dsMatrix44_determinant(a) \
	((a).values[0][0]*dsMatrix33_determinantImpl(a, 1, 2, 3, 1, 2, 3) - \
	 (a).values[1][0]*dsMatrix33_determinantImpl(a, 0, 2, 3, 1, 2, 3) + \
	 (a).values[2][0]*dsMatrix33_determinantImpl(a, 0, 1, 3, 1, 2, 3) - \
	 (a).values[3][0]*dsMatrix33_determinantImpl(a, 0, 1, 2, 1, 2, 3))

/**
 * @brief Inverts an matrix that only contains a rotation and translation.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
#define dsMatrix44_fastInvert(result, a) \
	do \
	{ \
		(result).value[0][0] = (a)[0][0]; \
		(result).value[0][1] = (a)[0][1]; \
		(result).value[0][2] = (a)[0][2]; \
		(result).value[0][3] = 0; \
		\
		(result).value[1][0] = (a)[1][0]; \
		(result).value[1][1] = (a)[1][1]; \
		(result).value[1][2] = (a)[1][2]; \
		(result).value[1][3] = 0; \
		\
		(result).value[2][0] = (a)[2][0]; \
		(result).value[2][1] = (a)[2][1]; \
		(result).value[2][2] = (a)[2][2]; \
		(result).value[2][3] = 0; \
		\
		(result).value[3][0] = -(a)[3][0]; \
		(result).value[3][1] = -(a)[3][1]; \
		(result).value[3][2] = -(a)[3][2]; \
		(result).value[3][3] = 1; \
	} \
	while (0)

/**
 * @brief Inverts an affine matrix.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix44f_affineInvert(dsMatrix44f* result, dsMatrix44f* a);

/** @copydoc dsMatrix44d_invert() */
DS_MATH_EXPORT void dsMatrix44d_affineInvert(dsMatrix44d* result, dsMatrix44d* a);

/**
 * @brief Inverts a matrix.
 * @param result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix44f_invert(dsMatrix44f* result, dsMatrix44f* a);

/** @copydoc dsMatrix44f_invert() */
DS_MATH_EXPORT void dsMatrix44d_invert(dsMatrix44d* result, dsMatrix44d* a);

/**
 * @brief Makes a rotation matrix.
 * @param result The matrix for the result.
 * @param x The angle around the x axis in radians.
 * @param y The angle around the y axis in radians.
 * @param z The angle around the z axis in radians.
 */
DS_MATH_EXPORT void dsMatrix44f_makeRotate(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeRotate() */
DS_MATH_EXPORT void dsMatrix44d_makeRotate(dsMatrix44d* result, double x, double y, double z);

/**
 * @brief Makes a rotation matrix.
 * @param result The matrix for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsMatrix44f_makeRotateAxisAngle(dsMatrix44f* result, const dsVector3f* axis,
	float angle);

/**
 * @brief Makes a rotation matrix.
 * @param result The matrix for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsMatrix44d_makeRotateAxisAngle(dsMatrix44d* result, const dsVector3d* axis,
	double angle);

/**
 * @brief Makes a translation matrix.
 * @param result The matrix for the result.
 * @param x The transition in the x axis.
 * @param y The transition in the y axis.
 * @param z The transition in the z axis.
 */
DS_MATH_EXPORT void dsMatrix44f_makeTranslate(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeTranslate() */
DS_MATH_EXPORT void dsMatrix44d_makeTranslate(dsMatrix44d* result, double x, double y, double z);

/**
 * @brief Makes a scale matrix.
 * @param result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 * @param z The scale in the z axis.
 */
DS_MATH_EXPORT void dsMatrix44f_makeScale(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeScale() */
DS_MATH_EXPORT void dsMatrix44d_makeScale(dsMatrix44d* result, double x, double y, double z);

#ifdef __cplusplus
}
#endif
