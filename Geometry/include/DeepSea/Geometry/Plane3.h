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
 * the implementation cannot be practically done within a macro.
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
 * plane.
 */
#define dsPlane3_distanceToPoint(plane, point) \
	(dsVector3_dot((plane).n, point) - (plane).d)

/**
 * @brief Intersects a plane with an aligned box.
 * @param plane The plane to intersect with the box.
 * @param box The box to intersect with the plane.
 * @return The side of the plane that the box lies.
 */
DS_GEOMETRY_EXPORT dsPlaneSide dsPlane3f_intersectAlignedBox(const dsPlane3f* plane,
	const dsAlignedBox3f* box);

/** @copydoc dsPlane3f_intersectAlignedBox() */
DS_GEOMETRY_EXPORT dsPlaneSide dsPlane3d_intersectAlignedBox(const dsPlane3d* plane,
	const dsAlignedBox3d* box);

/**
 * @brief Intersects a plane with an oriented box.
 * @param plane The plane to intersect with the box.
 * @param box The box to intersect with the plane.
 * @return The side of the plane that the box lies.
 */
DS_GEOMETRY_EXPORT dsPlaneSide dsPlane3f_intersectOrientedBox(const dsPlane3f* plane,
	const dsOrientedBox3f* box);

/** @copydoc dsPlane3f_intersectOrientedBox() */
DS_GEOMETRY_EXPORT dsPlaneSide dsPlane3d_intersectOrientedBox(const dsPlane3d* plane,
	const dsOrientedBox3d* box);

#ifdef __cplusplus
}
#endif
