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
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsFrustum3* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsFrustum3* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsFrustum3 types. These are used when
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsFrustum3f dsFrustum3d
 */

/**
 * @brief Makes a frustum from a projection matrix.
 *
 * The matrix may be a perspective or orthographic projection.
 *
 * @remark The planes aren't guaranteed to be normalized.
 * @param[out] result The resulting frustum.
 * @param matrix The projection matrix to extract the frustum planes from.
 * @param options The projection matrix options.
 */
#define dsFrustum3_fromMatrix(result, matrix, options) \
	do \
	{ \
		int8_t _yMult = (options) & dsProjectionMatrixOptions_InvertY ? -1 : 1; \
		(result).planes[dsFrustumPlanes_Left].n.x = (matrix).values[0][3] + (matrix).values[0][0]; \
		(result).planes[dsFrustumPlanes_Left].n.y = (matrix).values[1][3] + (matrix).values[1][0]; \
		(result).planes[dsFrustumPlanes_Left].n.z = (matrix).values[2][3] + (matrix).values[2][0]; \
		(result).planes[dsFrustumPlanes_Left].d = (matrix).values[3][3] + (matrix).values[3][0]; \
		\
		(result).planes[dsFrustumPlanes_Right].n.x = (matrix).values[0][3] - \
			(matrix).values[0][0]; \
		(result).planes[dsFrustumPlanes_Right].n.y = (matrix).values[1][3] - \
			(matrix).values[1][0]; \
		(result).planes[dsFrustumPlanes_Right].n.z = (matrix).values[2][3] - \
			(matrix).values[2][0]; \
		(result).planes[dsFrustumPlanes_Right].d = (matrix).values[3][3] - (matrix).values[3][0]; \
		\
		(result).planes[dsFrustumPlanes_Bottom].n.x = (matrix).values[0][3] + \
			(matrix).values[0][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Bottom].n.y = (matrix).values[1][3] + \
			(matrix).values[1][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Bottom].n.z = (matrix).values[2][3] + \
			(matrix).values[2][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Bottom].d = (matrix).values[3][3] + \
			(matrix).values[3][1]*_yMult; \
		\
		(result).planes[dsFrustumPlanes_Top].n.x = (matrix).values[0][3] - \
			(matrix).values[0][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Top].n.y = (matrix).values[1][3] - \
			(matrix).values[1][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Top].n.z = (matrix).values[2][3] - \
			(matrix).values[2][1]*_yMult; \
		(result).planes[dsFrustumPlanes_Top].d = (matrix).values[3][3] - \
			(matrix).values[3][1]*_yMult; \
		\
		if ((options) & dsProjectionMatrixOptions_InvertZ) \
		{ \
			if ((options) & dsProjectionMatrixOptions_HalfZRange) \
			{ \
				(result).planes[dsFrustumPlanes_Far].n.x = (matrix).values[0][2]; \
				(result).planes[dsFrustumPlanes_Far].n.y = (matrix).values[1][2]; \
				(result).planes[dsFrustumPlanes_Far].n.z = (matrix).values[2][2]; \
				(result).planes[dsFrustumPlanes_Far].d = (matrix).values[3][2]; \
			} \
			else \
			{ \
				(result).planes[dsFrustumPlanes_Far].n.x = (matrix).values[0][3] + \
					(matrix).values[0][2]; \
				(result).planes[dsFrustumPlanes_Far].n.y = (matrix).values[1][3] + \
					(matrix).values[1][2]; \
				(result).planes[dsFrustumPlanes_Far].n.z = (matrix).values[2][3] + \
					(matrix).values[2][2]; \
				(result).planes[dsFrustumPlanes_Far].d = (matrix).values[3][3] + \
					(matrix).values[3][2]; \
			} \
			\
			(result).planes[dsFrustumPlanes_Near].n.x = (matrix).values[0][3] - \
				(matrix).values[0][2]; \
			(result).planes[dsFrustumPlanes_Near].n.y = (matrix).values[1][3] - \
				(matrix).values[1][2]; \
			(result).planes[dsFrustumPlanes_Near].n.z = (matrix).values[2][3] - \
				(matrix).values[2][2]; \
			(result).planes[dsFrustumPlanes_Near].d = (matrix).values[3][3] - \
				(matrix).values[3][2]; \
		} \
		else \
		{ \
			if ((options) & dsProjectionMatrixOptions_HalfZRange) \
			{ \
				(result).planes[dsFrustumPlanes_Near].n.x = (matrix).values[0][2]; \
				(result).planes[dsFrustumPlanes_Near].n.y = (matrix).values[1][2]; \
				(result).planes[dsFrustumPlanes_Near].n.z = (matrix).values[2][2]; \
				(result).planes[dsFrustumPlanes_Near].d = (matrix).values[3][2]; \
			} \
			else \
			{ \
				(result).planes[dsFrustumPlanes_Near].n.x = (matrix).values[0][3] + \
					(matrix).values[0][2]; \
				(result).planes[dsFrustumPlanes_Near].n.y = (matrix).values[1][3] + \
					(matrix).values[1][2]; \
				(result).planes[dsFrustumPlanes_Near].n.z = (matrix).values[2][3] + \
					(matrix).values[2][2]; \
				(result).planes[dsFrustumPlanes_Near].d = (matrix).values[3][3] + \
					(matrix).values[3][2]; \
			} \
			\
			(result).planes[dsFrustumPlanes_Far].n.x = (matrix).values[0][3] - \
				(matrix).values[0][2]; \
			(result).planes[dsFrustumPlanes_Far].n.y = (matrix).values[1][3] - \
				(matrix).values[1][2]; \
			(result).planes[dsFrustumPlanes_Far].n.z = (matrix).values[2][3] - \
				(matrix).values[2][2]; \
			(result).planes[dsFrustumPlanes_Far].d = (matrix).values[3][3] - \
				(matrix).values[3][2]; \
		} \
	} while (0)

/**
 * @brief Normalizes the planes within a frustum.
 * @param[inout] frustum The frustum to normalize.
 */
DS_GEOMETRY_EXPORT void dsFrustum3f_normalize(dsFrustum3f* frustum);

/** @copydoc dsFrustum3f_normalize() */
DS_GEOMETRY_EXPORT void dsFrustum3d_normalize(dsFrustum3d* frustum);

/**
 * @brief Transforms a frustum by a matrix.
 *
 * In order to do the transformation, this will need to take the inverse-transpose of the matrix.
 * If the inverse-transpose is already calculated, call dsFrustum3_transformInverseTranspose()
 * instead.
 *
 * @param[inout] frustum The frustum to transform.
 * @param transform The transformation matrix.
 */
DS_GEOMETRY_EXPORT void dsFrustum3f_transform(dsFrustum3f* frustum, const dsMatrix44f* transform);

/** @copydoc dsFrustum3f_transform() */
DS_GEOMETRY_EXPORT void dsFrustum3d_transform(dsFrustum3d* frustum, const dsMatrix44d* transform);

/**
 * @brief Transforms a frustum by a matrix when the inverse-transpose is already calculated.
 * @param[inout] frustum The frustum to transform.
 * @param transform The inverse-transpose transformation matrix.
 */
DS_GEOMETRY_EXPORT void dsFrustum3f_transformInverseTranspose(dsFrustum3f* frustum,
	const dsMatrix44f* transform);

/** @copydoc dsFrustum3f_transformInverseTranspose() */
DS_GEOMETRY_EXPORT void dsFrustum3d_transformInverseTranspose(dsFrustum3d* frustum,
	const dsMatrix44d* transform);

/**
 * @brief Checks whether or not the frustum has an infinite far plane.
 * @param frustum The frustum to check.
 * @return Whether or not the frustum has an infinite far plane.
 */
DS_GEOMETRY_EXPORT bool dsFrustum3f_isInfinite(const dsFrustum3f* frustum);

/** @copydoc dsFrustum3f_isInfinite() */
DS_GEOMETRY_EXPORT bool dsFrustum3d_isInfinite(const dsFrustum3d* frustum);

/**
 * @brief Intersects an aligned box with a frustum.
 * @param frustum The frustum to intersect.
 * @param box The aligned box to intersect with.
 * @return The intersection result. Inside and outside is with respect to the frustum. If the box
 * fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3f_intersectAlignedBox(const dsFrustum3f* frustum,
	const dsAlignedBox3f* box);

/** @copydoc dsFrustum3f_intersectAlignedBox() */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3d_intersectAlignedBox(const dsFrustum3d* frustum,
	const dsAlignedBox3d* box);

/**
 * @brief Intersects an oriented box with a frustum.
 * @param frustum The frustum to intersect.
 * @param box The oriented box to intersect with.
 * @return The intersection result. Inside and outside is with respect to the frustum. If the box
 * fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3f_intersectOrientedBox(const dsFrustum3f* frustum,
	const dsOrientedBox3f* box);

/** @copydoc dsFrustum3f_intersectOrientedBox() */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3d_intersectOrientedBox(const dsFrustum3d* frustum,
	const dsOrientedBox3d* box);

/**
 * @brief Intersects a sphere with a frustum.
 * @remark This will only give accurate results for normalized frustums.
 * @param frustum The frustum to intersect.
 * @param center The center of the sphere.
 * @param radius The radius of the sphere.
 * @return The intersection result. Inside and outside is with respect to the frustum. If the box
 * fully contains the frustum, dsIntersectResult_Intersects will be returned.
 */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3f_intersectSphere(const dsFrustum3f* frustum,
	const dsVector3f* center, float radius);

/** @copydoc dsFrustum3f_intersectSphere() */
DS_GEOMETRY_EXPORT dsIntersectResult dsFrustum3d_intersectSphere(const dsFrustum3d* frustum,
	const dsVector3d* center, double radius);

/** @copydoc dsFrustum3_fromMatrix() */
DS_GEOMETRY_EXPORT inline void dsFrustum3f_fromMatrix(dsFrustum3f* result,
	const dsMatrix44f* matrix, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);
	dsFrustum3_fromMatrix(*result, *matrix, options);
}

/** @copydoc dsFrustum3_fromMatrix() */
DS_GEOMETRY_EXPORT inline void dsFrustum3d_fromMatrix(dsFrustum3d* result,
	const dsMatrix44d* matrix, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);
	dsFrustum3_fromMatrix(*result, *matrix, options);
}

#ifdef __cplusplus
}
#endif
