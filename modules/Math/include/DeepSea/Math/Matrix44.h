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
 * The macros are type-agnostic, allowing to be used with any dsMatrix44* type. All parameters are
 * by value. Unlike most of the vector macros, it is NOT safe to have the result be the same as one
 * of the parameters.
 *
 * The functions have different versions for the supported Matrix44 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * When using affine transforms (combinations of rotate, scale, and translate), it is faster to use
 * the affine functions and macros.
 *
 * @see dsMatrix44f dsMatrix44d
 */

/**
 * @brief Sets a matrix to be identity.
 * @param[out] result The matrix to hold the identity.
 */
#define dsMatrix44_identity(result) \
	do \
	{ \
		(result).values[0][0] = 1; \
		(result).values[0][1] = 0; \
		(result).values[0][2] = 0; \
		(result).values[0][3] = 0; \
		\
		(result).values[1][0] = 0; \
		(result).values[1][1] = 1; \
		(result).values[1][2] = 0; \
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = 0; \
		(result).values[2][1] = 0; \
		(result).values[2][2] = 1; \
		(result).values[2][3] = 0; \
		\
		(result).values[3][0] = 0; \
		(result).values[3][1] = 0; \
		(result).values[3][2] = 0; \
		(result).values[3][3] = 1; \
	} while (0)

/**
 * @brief Multiplies two matrices.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix44_mul(result, a, b) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		DS_ASSERT(&(result) != (const void*)&(b)); \
		\
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
 * @brief Multiplies two affine matrices.
 *
 * This assumes that the last row of both matrices is [0, 0, 0, 1].
 *
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix44_affineMul(result, a, b) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		DS_ASSERT(&(result) != (const void*)&(b)); \
		\
		(result).values[0][0] = (a).values[0][0]*(b).values[0][0] + \
								(a).values[1][0]*(b).values[0][1] + \
								(a).values[2][0]*(b).values[0][2]; \
		(result).values[0][1] = (a).values[0][1]*(b).values[0][0] + \
								(a).values[1][1]*(b).values[0][1] + \
								(a).values[2][1]*(b).values[0][2]; \
		(result).values[0][2] = (a).values[0][2]*(b).values[0][0] + \
								(a).values[1][2]*(b).values[0][1] + \
								(a).values[2][2]*(b).values[0][2]; \
		(result).values[0][3] = 0; \
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
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = (a).values[0][0]*(b).values[2][0] + \
								(a).values[1][0]*(b).values[2][1] + \
								(a).values[2][0]*(b).values[2][2] + \
								(a).values[3][0]*(b).values[2][3]; \
		(result).values[2][1] = (a).values[0][1]*(b).values[2][0] + \
								(a).values[1][1]*(b).values[2][1] + \
								(a).values[2][1]*(b).values[2][2]; \
		(result).values[2][2] = (a).values[0][2]*(b).values[2][0] + \
								(a).values[1][2]*(b).values[2][1] + \
								(a).values[2][2]*(b).values[2][2]; \
		(result).values[2][3] = 0; \
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
		(result).values[3][3] = 1; \
	} while (0)

/**
 * @brief Transforms a vector with a matrix.
 * @param[out] result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix44_transform(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[1][0]*(vec).values[1] + \
							 (mat).values[2][0]*(vec).values[2] + \
							 (mat).values[3][0]*(vec).values[3]; \
		(result).values[1] = (mat).values[0][1]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1] + \
							 (mat).values[2][1]*(vec).values[2] + \
							 (mat).values[3][1]*(vec).values[3]; \
		(result).values[2] = (mat).values[0][2]*(vec).values[0] + \
							 (mat).values[1][2]*(vec).values[1] + \
							 (mat).values[2][2]*(vec).values[2] + \
							 (mat).values[3][2]*(vec).values[3]; \
		(result).values[3] = (mat).values[0][3]*(vec).values[0] + \
							 (mat).values[1][3]*(vec).values[1] + \
							 (mat).values[2][3]*(vec).values[2] + \
							 (mat).values[3][3]*(vec).values[3]; \
	} while (0)

/**
 * @brief Transforms a vector with a transposed matrix.
 * @param[out] result The result of vec*mat. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix44_transformTransposed(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[0][1]*(vec).values[1] + \
							 (mat).values[0][2]*(vec).values[2] + \
							 (mat).values[0][3]*(vec).values[3]; \
		(result).values[1] = (mat).values[1][0]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1] + \
							 (mat).values[1][2]*(vec).values[2] + \
							 (mat).values[1][3]*(vec).values[3]; \
		(result).values[2] = (mat).values[2][0]*(vec).values[0] + \
							 (mat).values[2][1]*(vec).values[1] + \
							 (mat).values[2][2]*(vec).values[2] + \
							 (mat).values[2][3]*(vec).values[3]; \
		(result).values[3] = (mat).values[3][0]*(vec).values[0] + \
							 (mat).values[3][1]*(vec).values[1] + \
							 (mat).values[3][2]*(vec).values[2] + \
							 (mat).values[3][3]*(vec).values[3]; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param[out] result The transposed matrix. This may NOT be the same as a.
 * @param a The matrix to transpose.
 */
#define dsMatrix44_transpose(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[1][0]; \
		(result).values[0][2] = (a).values[2][0]; \
		(result).values[0][3] = (a).values[3][0]; \
		\
		(result).values[1][0] = (a).values[0][1]; \
		(result).values[1][1] = (a).values[1][1]; \
		(result).values[1][2] = (a).values[2][1]; \
		(result).values[1][3] = (a).values[3][1]; \
		\
		(result).values[2][0] = (a).values[0][2]; \
		(result).values[2][1] = (a).values[1][2]; \
		(result).values[2][2] = (a).values[2][2]; \
		(result).values[2][3] = (a).values[3][2]; \
		\
		(result).values[3][0] = (a).values[0][3]; \
		(result).values[3][1] = (a).values[1][3]; \
		(result).values[3][2] = (a).values[2][3]; \
		(result).values[3][3] = (a).values[3][3]; \
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
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
#define dsMatrix44_fastInvert(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[1][0]; \
		(result).values[0][2] = (a).values[2][0]; \
		(result).values[0][3] = 0; \
		\
		(result).values[1][0] = (a).values[0][1]; \
		(result).values[1][1] = (a).values[1][1]; \
		(result).values[1][2] = (a).values[2][1]; \
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = (a).values[0][2]; \
		(result).values[2][1] = (a).values[1][2]; \
		(result).values[2][2] = (a).values[2][2]; \
		(result).values[2][3] = 0; \
		\
		(result).values[3][0] = -(a).values[3][0]*(result).values[0][0] - \
			(a).values[3][1]*(result).values[1][0] - (a).values[3][2]*(result).values[2][0]; \
		(result).values[3][1] = -(a).values[3][0]*(result).values[0][1] - \
			(a).values[3][1]*(result).values[1][1] - (a).values[3][2]*(result).values[2][1]; \
		(result).values[3][2] = -(a).values[3][0]*(result).values[0][2] - \
			(a).values[3][1]*(result).values[1][2] - (a).values[3][2]*(result).values[2][2]; \
		(result).values[3][3] = 1; \
	} \
	while (0)

/**
 * @brief Inverts an affine matrix.
 *
 * An affine matrix will be a 3D transformation matrix that preserves parallel planes.
 *
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix44f_affineInvert(dsMatrix44f* result, const dsMatrix44f* a);

/** @copydoc dsMatrix44d_invert() */
DS_MATH_EXPORT void dsMatrix44d_affineInvert(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Inverts a matrix.
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix44f_invert(dsMatrix44f* result, const dsMatrix44f* a);

/** @copydoc dsMatrix44f_invert() */
DS_MATH_EXPORT void dsMatrix44d_invert(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix.
 * @param[out] result The inverse-transposed matrix. This may NOT be the same as a.
 * @param a The matrix to inverse-transpose.
 */
DS_MATH_EXPORT void dsMatrix44f_inverseTranspose(dsMatrix44f* result, const dsMatrix44f* a);

/** @copydoc dsMatrix44d_invert() */
DS_MATH_EXPORT void dsMatrix44d_inverseTranspose(dsMatrix44d* result, const dsMatrix44d* a);

/**
 * @brief Makes a rotation matrix.
 * @param[out] result The matrix for the result.
 * @param x The angle around the x axis in radians.
 * @param y The angle around the y axis in radians.
 * @param z The angle around the z axis in radians.
 */
DS_MATH_EXPORT void dsMatrix44f_makeRotate(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeRotate() */
DS_MATH_EXPORT void dsMatrix44d_makeRotate(dsMatrix44d* result, double x, double y, double z);

/**
 * @brief Makes a rotation matrix.
 * @param[out] result The matrix for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsMatrix44f_makeRotateAxisAngle(dsMatrix44f* result, const dsVector3f* axis,
	float angle);

/** @copydoc dsMatrix44f_makeRotateAxisAngle() */
DS_MATH_EXPORT void dsMatrix44d_makeRotateAxisAngle(dsMatrix44d* result, const dsVector3d* axis,
	double angle);

/**
 * @brief Makes a translation matrix.
 * @param[out] result The matrix for the result.
 * @param x The transition in the x axis.
 * @param y The transition in the y axis.
 * @param z The transition in the z axis.
 */
DS_MATH_EXPORT void dsMatrix44f_makeTranslate(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeTranslate() */
DS_MATH_EXPORT void dsMatrix44d_makeTranslate(dsMatrix44d* result, double x, double y, double z);

/**
 * @brief Makes a scale matrix.
 * @param[out] result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 * @param z The scale in the z axis.
 */
DS_MATH_EXPORT void dsMatrix44f_makeScale(dsMatrix44f* result, float x, float y, float z);

/** @copydoc dsMatrix44f_makeScale() */
DS_MATH_EXPORT void dsMatrix44d_makeScale(dsMatrix44d* result, double x, double y, double z);

/**
 * @brief Makes a matrix that looks at a position.
 * @param[out] result The matrix for the result.
 * @param eyePos The eye position for the center of the transform.
 * @param lookAtPos The position to look at.
 * @param upDir The reference up direction.
 */
DS_MATH_EXPORT void dsMatrix44f_lookAt(dsMatrix44f* result, const dsVector3f* eyePos,
	const dsVector3f* lookAtPos, const dsVector3f* upDir);

/** @copydoc dsMatrix44f_lookAt() */
DS_MATH_EXPORT void dsMatrix44d_lookAt(dsMatrix44d* result, const dsVector3d* eyePos,
	const dsVector3d* lookAtPos, const dsVector3d* upDir);

/**
 * @brief Makes an orthographic projection matrix.
 *
 * The matrix is generated assuming looking down the -Z axis. As a result, the near and far plane
 * distances are negated compared to world space.
 *
 * @param[out] result The matrix for the result.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane.
 * @param halfDepth True if the projected depth is in the range [0, 1], false if in the range
 *     [-1, 1]. Examples where this is true include Direct3D, Metal, or Vulkan.
 * @param invertY True to invert the Y coordinate. An example where this is used is Vulkan.
 */
DS_MATH_EXPORT void dsMatrix44f_makeOrtho(dsMatrix44f* result, float left, float right,
	float bottom, float top, float near, float far, bool halfDepth, bool invertY);

/** @copydoc dsMatrix44f_makeOrtho() */
DS_MATH_EXPORT void dsMatrix44d_makeOrtho(dsMatrix44d* result, double left, double right,
	double bottom, double top, double near, double far, bool halfDepth, bool invertY);

/**
 * @brief Makes a projection matrix for a frustum.
 * @param[out] result The matrix for the result.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane. This may be INFINITY.
 * @param halfDepth True if the projected depth is in the range [0, 1], false if in the range
 *     [-1, 1]. Examples where this is true include Direct3D, Metal, or Vulkan.
 * @param invertY True to invert the Y coordinate. An example where this is used is Vulkan.
 */
DS_MATH_EXPORT void dsMatrix44f_makeFrustum(dsMatrix44f* result, float left, float right,
	float bottom, float top, float near, float far, bool halfDepth, bool invertY);

/** @copydoc dsMatrix44f_makeFrustum() */
DS_MATH_EXPORT void dsMatrix44d_makeFrustum(dsMatrix44d* result, double left, double right,
	double bottom, double top, double near, double far, bool halfDepth, bool invertY);

/**
 * @brief Makes a perspective projection matrix.
 * @param[out] result The matrix for the result.
 * @param fovy The field of view in the Y direction in radians.
 * @param aspect The aspect ratio as X/Y.
 * @param near The near plane.
 * @param far The far plane. This may be INFINITY.
 * @param halfDepth True if the projected depth is in the range [0, 1], false if in the range
 *     [-1, 1]. Examples where this is true include Direct3D, Metal, or Vulkan.
 * @param invertY True to invert the Y coordinate. An example where this is used is Vulkan.
 */
DS_MATH_EXPORT void dsMatrix44f_makePerspective(dsMatrix44f* result, float fovy, float aspect,
	float near, float far, bool halfDepth, bool invertY);

/** @copydoc dsMatrix44f_makePerspective() */
DS_MATH_EXPORT void dsMatrix44d_makePerspective(dsMatrix44d* result, double fovy, double aspect,
	double near, double far, bool halfDepth, bool invertY);

/** @copydoc dsMatrix44_identity() */
DS_MATH_EXPORT inline void dsMatrix44f_identity(dsMatrix44f* result)
{
	DS_ASSERT(result);
	dsMatrix44_identity(*result);
}

/** @copydoc dsMatrix44_identity() */
DS_MATH_EXPORT inline void dsMatrix44d_identity(dsMatrix44d* result)
{
	DS_ASSERT(result);
	dsMatrix44_identity(*result);
}

/** @copydoc dsMatrix44_mul() */
DS_MATH_EXPORT inline void dsMatrix44f_mul(dsMatrix44f* result, const dsMatrix44f* a,
	const dsMatrix44f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix44_mul(*result, *a, *b);
}

/** @copydoc dsMatrix44_mul() */
DS_MATH_EXPORT inline void dsMatrix44d_mul(dsMatrix44d* result, const dsMatrix44d* a,
	const dsMatrix44d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix44_mul(*result, *a, *b);
}

/** @copydoc dsMatrix44_affineMul() */
DS_MATH_EXPORT inline void dsMatrix44f_affineMul(dsMatrix44f* result, const dsMatrix44f* a,
	const dsMatrix44f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix44_affineMul(*result, *a, *b);
}

/** @copydoc dsMatrix44_affineMul() */
DS_MATH_EXPORT inline void dsMatrix44d_affineMul(dsMatrix44d* result, const dsMatrix44d* a,
	const dsMatrix44d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix44_affineMul(*result, *a, *b);
}

/** @copydoc dsMatrix44_transform() */
DS_MATH_EXPORT inline void dsMatrix44f_transform(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix44_transform(*result, *mat, *vec);
}

/** @copydoc dsMatrix44_transform() */
DS_MATH_EXPORT inline void dsMatrix44d_transform(dsVector4d* result, const dsMatrix44d* mat,
	const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix44_transform(*result, *mat, *vec);
}

/** @copydoc dsMatrix44_transformTransposed() */
DS_MATH_EXPORT inline void dsMatrix44f_transformTransposed(dsVector4f* result,
	const dsMatrix44f* mat, const dsVector4f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix44_transformTransposed(*result, *mat, *vec);
}

/** @copydoc dsMatrix44_transformTransposed() */
DS_MATH_EXPORT inline void dsMatrix44d_transformTransposed(dsVector4d* result,
	const dsMatrix44d* mat, const dsVector4d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix44_transformTransposed(*result, *mat, *vec);
}

/** @copydoc dsMatrix44_transpose() */
DS_MATH_EXPORT inline void dsMatrix44f_transpose(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix44_transpose(*result, *a);
}

/** @copydoc dsMatrix44_transpose() */
DS_MATH_EXPORT inline void dsMatrix44d_transpose(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix44_transpose(*result, *a);
}

/** @copydoc dsMatrix44_determinant() */
DS_MATH_EXPORT inline float dsMatrix44f_determinant(dsMatrix44f* a)
{
	DS_ASSERT(a);
	return dsMatrix44_determinant(*a);
}

/** @copydoc dsMatrix44_determinant() */
DS_MATH_EXPORT inline double dsMatrix44d_determinant(dsMatrix44d* a)
{
	DS_ASSERT(a);
	return dsMatrix44_determinant(*a);
}

/** @copydoc dsMatrix44_fastInvert() */
DS_MATH_EXPORT inline void dsMatrix44f_fastInvert(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix44_fastInvert(*result, *a);
}

/** @copydoc dsMatrix44_fastInvert() */
DS_MATH_EXPORT inline void dsMatrix44d_fastInvert(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix44_fastInvert(*result, *a);
}

#ifdef __cplusplus
}
#endif
