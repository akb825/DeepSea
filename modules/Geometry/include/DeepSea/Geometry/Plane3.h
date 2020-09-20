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
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Vector3.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsPlane3* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsPlane3* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsPlane3 types. These are used when
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsPlane3f dsPlane3d
 */

/**
 * @brief Creates a plane from a normal and point.
 * @param[out] result The plane to create.
 * @param normal The plane normal.
 * @param point A point on the plane.
 */
#define dsPlane3_fromNormalPoint(result, normal, point) \
	do \
	{ \
		(result).n = normal; \
		(result).d = dsVector3_dot(normal, point); \
	} while (0)

/**
 * @brief Calculates the distance from a plane to a point.
 * @param plane The plane to calculate the distance from.
 * @param point The point to calculate the distance to.
 * @return The distance from the plane to the point. This will be negative if it is behind the
 * plane. The distance will be scaled based on the length of the normal, so it will only give an
 * accurate result for normalized planes.
 */
#define dsPlane3_distanceToPoint(plane, point) \
	(dsVector3_dot((plane).n, point) - (plane).d)

/**
 * @brief Normalizes a plane.
 * @param[out] result The normalized plane.
 * @param plane The plane to normalize.
 */
DS_GEOMETRY_EXPORT void dsPlane3f_normalize(dsPlane3f* result, const dsPlane3f* plane);

/** @copydoc dsPlane3f_normalize() */
DS_GEOMETRY_EXPORT void dsPlane3d_normalize(dsPlane3d* result, const dsPlane3d* plane);

/**
 * @brief Transforms a plane by a matrix.
 *
 * In order to do the transformation, this will need to take the inverse-transpose of the matrix.
 * If the inverse-transpose is already calculated, call dsPlane3_transformInverseTranspose()
 * instead.
 *
 * @param[out] result The transformed plane.
 * @param transform The transformation matrix.
 * @param plane The plane to transform.
 */
DS_GEOMETRY_EXPORT void dsPlane3f_transform(dsPlane3f* result, const dsMatrix44f* transform,
	const dsPlane3f* plane);

/** @copydoc dsPlane3f_transform() */
DS_GEOMETRY_EXPORT void dsPlane3d_transform(dsPlane3d* result, const dsMatrix44d* transform,
	const dsPlane3d* plane);

/**
 * @brief Transforms a plane by a matrix when the inverse-transpose is already calculated.
 * @param[out] result The transformed plane.
 * @param transform The inverse-transpose transformation matrix.
 * @param plane The plane to transform.
 */
DS_GEOMETRY_EXPORT void dsPlane3f_transformInverseTranspose(dsPlane3f* result,
	const dsMatrix44f* transform, const dsPlane3f* plane);

/** @copydoc dsPlane3f_transformInverseTranspose() */
DS_GEOMETRY_EXPORT void dsPlane3d_transformInverseTranspose(dsPlane3d* result,
	const dsMatrix44d* transform, const dsPlane3d* plane);

/**
 * @brief Intersects a plane with an aligned box.
 * @param plane The plane to intersect with the box.
 * @param box The box to intersect with the plane.
 * @return The side of the plane that the box lies.
 */
DS_GEOMETRY_EXPORT dsIntersectResult dsPlane3f_intersectAlignedBox(const dsPlane3f* plane,
	const dsAlignedBox3f* box);

/** @copydoc dsPlane3f_intersectAlignedBox() */
DS_GEOMETRY_EXPORT dsIntersectResult dsPlane3d_intersectAlignedBox(const dsPlane3d* plane,
	const dsAlignedBox3d* box);

/**
 * @brief Intersects a plane with an oriented box.
 * @param plane The plane to intersect with the box.
 * @param box The box to intersect with the plane.
 * @return The side of the plane that the box lies.
 */
DS_GEOMETRY_EXPORT dsIntersectResult dsPlane3f_intersectOrientedBox(const dsPlane3f* plane,
	const dsOrientedBox3f* box);

/** @copydoc dsPlane3f_intersectOrientedBox() */
DS_GEOMETRY_EXPORT dsIntersectResult dsPlane3d_intersectOrientedBox(const dsPlane3d* plane,
	const dsOrientedBox3d* box);

/** @copydoc dsPlane3_fromNormalPoint() */
DS_GEOMETRY_EXPORT inline void dsPlane3f_fromNormalPoint(dsPlane3f* result,
	const dsVector3f* normal, const dsVector3f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(normal);
	DS_ASSERT(point);
	dsPlane3_fromNormalPoint(*result, *normal, *point);
}

/** @copydoc dsPlane3_fromNormalPoint() */
DS_GEOMETRY_EXPORT inline void dsPlane3d_fromNormalPoint(dsPlane3d* result,
	const dsVector3d* normal, const dsVector3d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(normal);
	DS_ASSERT(point);
	dsPlane3_fromNormalPoint(*result, *normal, *point);
}

/** @copydoc dsPlane3_distanceToPoint() */
DS_GEOMETRY_EXPORT inline float dsPlane3f_distanceToPoint(const dsPlane3f* plane,
	const dsVector3f* point)
{
	DS_ASSERT(plane);
	DS_ASSERT(point);
	return dsPlane3_distanceToPoint(*plane, *point);
}

/** @copydoc dsPlane3_distanceToPoint() */
DS_GEOMETRY_EXPORT inline double dsPlane3d_distanceToPoint(const dsPlane3d* plane,
	const dsVector3d* point)
{
	DS_ASSERT(plane);
	DS_ASSERT(point);
	return dsPlane3_distanceToPoint(*plane, *point);
}

#ifdef __cplusplus
}
#endif
