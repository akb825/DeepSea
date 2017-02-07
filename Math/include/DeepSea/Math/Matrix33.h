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
 * @brief Macros and functions for manipulating dsMatrix33* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsMatrix33* type. All parameters are
 * by value. Unlike most of the vector macros, it is NOT safe to have the result be the same as one
 * of the parameters.
 *
 * The functions have different versions for the supported dsMatrix33 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * When using affine transforms (combinations of rotate, scale, and translate), it is faster to use
 * the affine functions and macros.
 *
 * @see dsMatrix33f dsMatrix33d
 */

/**
 * @brief Sets a matrix to be identity.
 * @param[out] result The matrix to hold the identity.
 */
#define dsMatrix33_identity(result) \
	do \
	{ \
		(result).values[0][0] = 1; \
		(result).values[0][1] = 0; \
		(result).values[0][2] = 0; \
		\
		(result).values[1][0] = 0; \
		(result).values[1][1] = 1; \
		(result).values[1][2] = 0; \
		\
		(result).values[2][0] = 0; \
		(result).values[2][1] = 0; \
		(result).values[2][2] = 1; \
	} while (0)

/**
 * @brief Multiplies two matrices.
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix33_mul(result, a, b) \
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
 * @brief Multiplies two affine matrices.
 *
 * This assumes that the last row of both matrices is [0, 0, 1].
 *
 * @param[out] result The result of a*b. This may NOT be the same as a or b.
 * @param a The first matrix.
 * @param b The second matrix.
 */
#define dsMatrix33_affineMul(result, a, b) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		DS_ASSERT(&(result) != (const void*)&(b)); \
		\
		(result).values[0][0] = (a).values[0][0]*(b).values[0][0] + \
								(a).values[1][0]*(b).values[0][1]; \
		(result).values[0][1] = (a).values[0][1]*(b).values[0][0] + \
								(a).values[1][1]*(b).values[0][1]; \
		(result).values[0][2] = 0; \
		\
		(result).values[1][0] = (a).values[0][0]*(b).values[1][0] + \
								(a).values[1][0]*(b).values[1][1]; \
		(result).values[1][1] = (a).values[0][1]*(b).values[1][0] + \
								(a).values[1][1]*(b).values[1][1]; \
		(result).values[1][2] = 0; \
		\
		(result).values[2][0] = (a).values[0][0]*(b).values[2][0] + \
								(a).values[1][0]*(b).values[2][1]; \
		(result).values[2][1] = (a).values[0][1]*(b).values[2][0] + \
								(a).values[1][1]*(b).values[2][1]; \
		(result).values[2][2] = 1; \
	} while (0)

/**
 * @brief Transforms a vector with a matrix.
 * @param[out] result The result of vec*mat. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix33_transform(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[1][0]*(vec).values[1] + \
							 (mat).values[2][0]*(vec).values[2]; \
		(result).values[1] = (mat).values[0][1]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1] + \
							 (mat).values[2][1]*(vec).values[2]; \
		(result).values[2] = (mat).values[0][2]*(vec).values[0] + \
							 (mat).values[1][2]*(vec).values[1] + \
							 (mat).values[2][2]*(vec).values[2]; \
	} while (0)

/**
 * @brief Transforms a vector with a transposed matrix.
 * @param[out] result The result of mat*vec. This may NOT be the same as vec.
 * @param mat The matrix to transform with.
 * @param vec The vector to transform.
 */
#define dsMatrix33_transformTransposed(result, mat, vec) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(vec)); \
		(result).values[0] = (mat).values[0][0]*(vec).values[0] + \
							 (mat).values[0][1]*(vec).values[1] + \
							 (mat).values[0][2]*(vec).values[2]; \
		(result).values[1] = (mat).values[1][0]*(vec).values[0] + \
							 (mat).values[1][1]*(vec).values[1] + \
							 (mat).values[1][2]*(vec).values[2]; \
		(result).values[2] = (mat).values[2][0]*(vec).values[0] + \
							 (mat).values[2][1]*(vec).values[1] + \
							 (mat).values[2][2]*(vec).values[2]; \
	} while (0)

/**
 * @brief Transposes a matrix.
 * @param[out] result The transposed matrix. This may NOT be the same as a.
 * @param a The matrix to transpose.
 */
#define dsMatrix33_transpose(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[1][0]; \
		(result).values[0][2] = (a).values[2][0]; \
		\
		(result).values[1][0] = (a).values[0][1]; \
		(result).values[1][1] = (a).values[1][1]; \
		(result).values[1][2] = (a).values[2][1]; \
		\
		(result).values[2][0] = (a).values[0][2]; \
		(result).values[2][1] = (a).values[1][2]; \
		(result).values[2][2] = (a).values[2][2]; \
	} while (0)

/**
 * @brief Gets the determinant of a matrix.
 * @param a The matrix to get the determinant of.
 * @return The determinant.
 */
#define dsMatrix33_determinant(a) dsMatrix33_determinantImpl(a, 0, 1, 2, 0, 1, 2)

/**
 * @brief Inverts an matrix that only contains a rotation and translation.
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
#define dsMatrix33_fastInvert(result, a) \
	do \
	{ \
		DS_ASSERT(&(result) != (const void*)&(a)); \
		\
		(result).values[0][0] = (a).values[0][0]; \
		(result).values[0][1] = (a).values[1][0]; \
		(result).values[0][2] = 0; \
		\
		(result).values[1][0] = (a).values[0][1]; \
		(result).values[1][1] = (a).values[1][1]; \
		(result).values[1][2] = 0; \
		\
		(result).values[2][0] = -(a).values[2][0]*(result).values[0][0] - \
			(a).values[2][1]*(result).values[1][0]; \
		(result).values[2][1] = -(a).values[2][0]*(result).values[0][1] - \
			(a).values[2][1]*(result).values[1][1]; \
		(result).values[2][2] = 1; \
	} \
	while (0)

/**
 * @brief Inverts an affine matrix.
 *
 * An affine matrix will be a 2D transformation matrix that preserves parallel lines.
 *
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix33f_affineInvert(dsMatrix33f* result, const dsMatrix33f* a);

/** @copydoc dsMatrix33d_invert() */
DS_MATH_EXPORT void dsMatrix33d_affineInvert(dsMatrix33d* result, const dsMatrix33d* a);

/**
 * @brief Inverts a matrix.
 * @param[out] result The inverted matrix. This may NOT be the same as a.
 * @param a The matrix to invert.
 */
DS_MATH_EXPORT void dsMatrix33f_invert(dsMatrix33f* result, const dsMatrix33f* a);

/** @copydoc dsMatrix33f_invert() */
DS_MATH_EXPORT void dsMatrix33d_invert(dsMatrix33d* result, const dsMatrix33d* a);

/**
 * @brief Calculates the inverse-transpose transformation matrix.
 * @param[out] result The inverse-transposed matrix. This may NOT be the same as a.
 * @param a The matrix to inverse-transpose.
 */
DS_MATH_EXPORT void dsMatrix33f_inverseTranspose(dsMatrix33f* result, const dsMatrix33f* a);

/** @copydoc dsMatrix44d_invert() */
DS_MATH_EXPORT void dsMatrix33d_inverseTranspose(dsMatrix33d* result, const dsMatrix33d* a);

/**
 * @brief Makes a 2D rotation matrix.
 * @param[out] result The matrix for the result.
 * @param angle The angle to rotate by in radians.
 */
DS_MATH_EXPORT void dsMatrix33f_makeRotate(dsMatrix33f* result, float angle);

/** @copydoc dsMatrix33f_makeRotate() */
DS_MATH_EXPORT void dsMatrix33d_makeRotate(dsMatrix33d* result, double angle);

/**
 * @brief Makes a 3D rotation matrix.
 * @param[out] result The matrix for the result.
 * @param x The angle around the x axis in radians.
 * @param y The angle around the y axis in radians.
 * @param z The angle around the z axis in radians.
 */
DS_MATH_EXPORT void dsMatrix33f_makeRotate3D(dsMatrix33f* result, float x, float y, float z);

/** @copydoc dsMatrix33f_makeRotate3D() */
DS_MATH_EXPORT void dsMatrix33d_makeRotate3D(dsMatrix33d* result, double x, double y, double z);

/**
 * @brief Makes a 3D rotation matrix.
 * @param[out] result The matrix for the result.
 * @param axis The axis to rotate around. This should be a unit vector.
 * @param angle The angle to rotate in radians.
 */
DS_MATH_EXPORT void dsMatrix33f_makeRotate3DAxisAngle(dsMatrix33f* result, const dsVector3f* axis,
	float angle);

/** @copydoc dsMatrix33f_makeRotate3DAxisAngle() */
DS_MATH_EXPORT void dsMatrix33d_makeRotate3DAxisAngle(dsMatrix33d* result, const dsVector3d* axis,
	double angle);

/**
 * @brief Makes a translation matrix.
 * @param[out] result The matrix for the result.
 * @param x The transition in the x axis.
 * @param y The transition in the y axis.
 */
DS_MATH_EXPORT void dsMatrix33f_makeTranslate(dsMatrix33f* result, float x, float y);

/** @copydoc dsMatrix33f_makeTranslate() */
DS_MATH_EXPORT void dsMatrix33d_makeTranslate(dsMatrix33d* result, double x, double y);

/**
 * @brief Makes a 2D scale matrix.
 * @param[out] result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 */
DS_MATH_EXPORT void dsMatrix33f_makeScale(dsMatrix33f* result, float x, float y);

/** @copydoc dsMatrix33f_makeScale() */
DS_MATH_EXPORT void dsMatrix33d_makeScale(dsMatrix33d* result, double x, double y);

/**
 * @brief Makes a 3D scale matrix.
 * @param[out] result The matrix for the result.
 * @param x The scale in the x axis.
 * @param y The scale in the y axis.
 * @param z The scale in the z axis.
 */
DS_MATH_EXPORT void dsMatrix33f_makeScale3D(dsMatrix33f* result, float x, float y, float z);

/** @copydoc dsMatrix33f_makeScale3D() */
DS_MATH_EXPORT void dsMatrix33d_makeScale3D(dsMatrix33d* result, double x, double y, double z);

/// \{
#define dsMatrix33_determinantImpl(a, i0, i1, i2, j0, j1, j2) \
	((a).values[i0][j0]*(a).values[i1][j1]*(a).values[i2][j2] + \
	 (a).values[i1][j0]*(a).values[i2][j1]*(a).values[i0][j2] + \
	 (a).values[i2][j0]*(a).values[i0][j1]*(a).values[i1][j2] - \
	 (a).values[i2][j0]*(a).values[i1][j1]*(a).values[i0][j2] - \
	 (a).values[i1][j0]*(a).values[i0][j1]*(a).values[i2][j2] - \
	 (a).values[i0][j0]*(a).values[i2][j1]*(a).values[i1][j2])
/// \}

/** @copydoc dsMatrix33_identity() */
inline void dsMatrix33f_identity(dsMatrix33f* result)
{
	DS_ASSERT(result);
	dsMatrix33_identity(*result);
}

/** @copydoc dsMatrix33_identity() */
inline void dsMatrix33d_identity(dsMatrix33d* result)
{
	DS_ASSERT(result);
	dsMatrix33_identity(*result);
}

/** @copydoc dsMatrix33_mul() */
inline void dsMatrix33f_mul(dsMatrix33f* result, const dsMatrix33f* a, const dsMatrix33f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix33_mul(*result, *a, *b);
}

/** @copydoc dsMatrix33_mul() */
inline void dsMatrix33d_mul(dsMatrix33d* result, const dsMatrix33d* a, const dsMatrix33d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix33_mul(*result, *a, *b);
}

/** @copydoc dsMatrix33_affineMul() */
inline void dsMatrix33f_affineMul(dsMatrix33f* result, const dsMatrix33f* a, const dsMatrix33f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix33_affineMul(*result, *a, *b);
}

/** @copydoc dsMatrix33_affineMul() */
inline void dsMatrix33d_affineMul(dsMatrix33d* result, const dsMatrix33d* a, const dsMatrix33d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsMatrix33_affineMul(*result, *a, *b);
}

/** @copydoc dsMatrix33_transform() */
inline void dsMatrix33f_transform(dsVector3f* result, const dsMatrix33f* mat, const dsVector3f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix33_transform(*result, *mat, *vec);
}

/** @copydoc dsMatrix33_transform() */
inline void dsMatrix33d_transform(dsVector3d* result, const dsMatrix33d* mat, const dsVector3d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix33_transform(*result, *mat, *vec);
}

/** @copydoc dsMatrix33_transformTransposed() */
inline void dsMatrix33f_transformTransposed(dsVector3f* result, const dsMatrix33f* mat,
	const dsVector3f* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix33_transformTransposed(*result, *mat, *vec);
}

/** @copydoc dsMatrix33_transformTransposed() */
inline void dsMatrix33d_transformTransposed(dsVector3d* result, const dsMatrix33d* mat,
	const dsVector3d* vec)
{
	DS_ASSERT(result);
	DS_ASSERT(mat);
	DS_ASSERT(vec);
	dsMatrix33_transformTransposed(*result, *mat, *vec);
}

/** @copydoc dsMatrix33_transpose() */
inline void dsMatrix33f_transpose(dsMatrix33f* result, const dsMatrix33f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix33_transpose(*result, *a);
}

/** @copydoc dsMatrix33_transpose() */
inline void dsMatrix33d_transpose(dsMatrix33d* result, const dsMatrix33d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix33_transpose(*result, *a);
}

/** @copydoc dsMatrix33_determinant() */
inline float dsMatrix33f_determinant(dsMatrix33f* a)
{
	DS_ASSERT(a);
	return dsMatrix33_determinant(*a);
}

/** @copydoc dsMatrix33_determinant() */
inline double dsMatrix33d_determinant(dsMatrix33d* a)
{
	DS_ASSERT(a);
	return dsMatrix33_determinant(*a);
}

/** @copydoc dsMatrix33_fastInvert() */
inline void dsMatrix33f_fastInvert(dsMatrix33f* result, const dsMatrix33f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix33_fastInvert(*result, *a);
}

/** @copydoc dsMatrix33_fastInvert() */
inline void dsMatrix33d_fastInvert(dsMatrix33d* result, const dsMatrix33d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsMatrix33_fastInvert(*result, *a);
}

#ifdef __cplusplus
}
#endif
