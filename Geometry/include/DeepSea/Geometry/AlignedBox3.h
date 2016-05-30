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
#include <DeepSea/Math/Core.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsAlignedBox3* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsAlignedBox3* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsAlignedBox3 types. These are used when
 * the implementation cannot be practically done within a macro.
 */

/**
 * @brief Returns whether or not a box is valid.
 *
 * A box is invalid if a minimum value is larger than a maximum value.
 *
 * @param box The box to check.
 * @return True if the box is valid.
 */
#define dsAlignedBox3_isValid(box) \
	((box).min.x <= (box).max.x && (box).min.y <= (box).max.y && (box).min.z <= (box).max.z)

/**
 * @brief Adds a point to the box.
 * @param[inout] box The box to add the point to.
 * @param point The point to add.
 */
#define dsAlignedBox3_addPoint(box, point) \
	do \
	{ \
		(box).min.x = dsMin((box).min.x, (point).x); \
		(box).min.y = dsMin((box).min.y, (point).y); \
		(box).min.z = dsMin((box).min.z, (point).z); \
		(box).max.x = dsMax((box).max.x, (point).x); \
		(box).max.y = dsMax((box).max.y, (point).y); \
		(box).max.z = dsMax((box).max.z, (point).z); \
	} while (0)

/**
 * @brief Adds one box to another.
 * @param[inout] box The box to add the other box to.
 * @param otherBox The box to add.
 */
#define dsAlignedBox3_addBox(box, otherBox) \
	do \
	{ \
		(box).min.x = dsMin((box).min.x, (otherBox).min.x); \
		(box).min.y = dsMin((box).min.y, (otherBox).min.y); \
		(box).min.z = dsMin((box).min.z, (otherBox).min.z); \
		(box).max.x = dsMax((box).max.x, (otherBox).max.x); \
		(box).max.y = dsMax((box).max.y, (otherBox).max.y); \
		(box).max.z = dsMax((box).max.z, (otherBox).max.z); \
	} while (0)

/**
 * @brief Checks if a box contains a point.
 * @param box The box to check.
 * @param point The point to check within the box.
 * @return True if the point is contained in the box.
 */
#define dsAlignedBox3_containsPoint(box, point) \
	((point).x >= (box).min.x && (point).y >= (box).min.y && (point).z >= (box).min.z && \
	 (point).x <= (box).max.x && (point).y <= (box).max.y && (point).z <= (box).max.z)

/**
 * @brief Checks if a box contains another box.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box contains otherBox.
 */
#define dsAlignedBox3_containsBox(box, otherBox) \
	((box).min.x <= (otherBox).min.x && (box).min.y <= (otherBox).min.y && \
		(box).min.z <= (otherBox).min.z && \
	 (box).max.x >= (otherBox).max.x && (box).max.y >= (otherBox).max.y && \
		(box).max.z >= (otherBox).max.z)

/**
 * @brief Checks if a box intersects with another box.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box intersects with otherBox.
 */
#define dsAlignedBox3_intersects(box, otherBox) \
	((box).min.x <= (otherBox).max.x && (box).min.y <= (otherBox).max.y && \
		(box).min.z <= (otherBox).max.z && \
	 (box).max.x >= (otherBox).min.x && (box).max.y >= (otherBox).min.y && \
		(box).max.z >= (otherBox).min.z)

/**
 * @brief Creates a box by intersecting two boxes.
 * @param[out] result The intersected box.
 * @param a The first box to intersect.
 * @param b The second box to intersect.
 */
#define dsAlignedBox3_intersect(result, a, b) \
	do \
	{ \
		(result).min.x = dsMax((a).min.x, (b).min.x); \
		(result).min.y = dsMax((a).min.y, (b).min.y); \
		(result).min.z = dsMax((a).min.z, (b).min.z); \
		(result).max.x = dsMin((a).max.x, (b).max.x); \
		(result).max.y = dsMin((a).max.y, (b).max.y); \
		(result).max.z = dsMin((a).max.z, (b).max.z); \
	} while (0)

/**
 * @brief Computes the center of the box.
 * @param[out] result The dsVector3* type for the center.
 * @param box The box to compute the center for.
 */
#define dsAlignedBox3_center(result, box) \
	do \
	{ \
		(result).x = ((box).min.x + (box).max.x)/2; \
		(result).y = ((box).min.y + (box).max.y)/2; \
		(result).z = ((box).min.z + (box).max.z)/2; \
	} while (0)

/**
 * @brief Computes the extents of the box.
 *
 * The extents are the distance between the min and max bounds along each axis.
 *
 * @param[out] result The dsVector3* type for the extents.
 * @param box The box to compute the extents for.
 */
#define dsAlignedBox3_extents(result, box) \
	do \
	{ \
		(result).x = (box).max.x - (box).min.x; \
		(result).y = (box).max.y - (box).min.y; \
		(result).z = (box).max.z - (box).min.z; \
	} while (0)

/**
 * @brief Extracts the corners from an aligned box.
 * @param[out] corners The corners for the box.
 * @param box The box to extract the corners from.
 * @return False if box is empty.
 */
#define dsAlignedBox3_corners(corners, box) \
	do \
	{ \
		(corners)[0].x = (box).min.x; \
		(corners)[0].y = (box).min.y; \
		(corners)[0].z = (box).min.z; \
		\
		(corners)[1].x = (box).min.x; \
		(corners)[1].y = (box).min.y; \
		(corners)[1].z = (box).max.z; \
		\
		(corners)[2].x = (box).min.x; \
		(corners)[2].y = (box).max.y; \
		(corners)[2].z = (box).min.z; \
		\
		(corners)[3].x = (box).min.x; \
		(corners)[3].y = (box).max.y; \
		(corners)[3].z = (box).max.z; \
		\
		(corners)[4].x = (box).max.x; \
		(corners)[4].y = (box).min.y; \
		(corners)[4].z = (box).min.z; \
		\
		(corners)[5].x = (box).max.x; \
		(corners)[5].y = (box).min.y; \
		(corners)[5].z = (box).max.z; \
		\
		(corners)[6].x = (box).max.x; \
		(corners)[6].y = (box).max.y; \
		(corners)[6].z = (box).min.z; \
		\
		(corners)[7].x = (box).max.x; \
		(corners)[7].y = (box).max.y; \
		(corners)[7].z = (box).max.z; \
	} while (0)

/**
 * @brief Computes the closest point to the box.
 * @param[out] result The closest point.
 * @param box The box to compute the closest point on.
 * @param point The point to check.
 */
#define dsAlignedBox3_closestPoint(result, box, point) \
	do \
	{ \
		if (dsAlignedBox3_isValid(box)) \
		{ \
			(result).x = dsMax((box).min.x, (point).x); \
			(result).x = dsMin((box).max.x, (result).x); \
			\
			(result).y = dsMax((box).min.y, (point).y); \
			(result).y = dsMin((box).max.y, (result).y); \
			\
			(result).z = dsMax((box).min.z, (point).z); \
			(result).z = dsMin((box).max.z, (result).z); \
		} \
		else \
			(result) = (point); \
	} while (0)

/**
 * @brief Makes an invalid box.
 *
 * An invalid box has the minimum values larger than the maximum values. These are set to the
 * largest positive/negative values to ensure that intersections will always result in an invalid
 * box.
 *
 * @param result The box to make invalid.
 */
DS_GEOMETRY_EXPORT void dsAlignedBox3f_makeInvalid(dsAlignedBox3f* result);

/** @copydoc dsAlignedBox3f_makeInvalid() */
DS_GEOMETRY_EXPORT void dsAlignedBox3d_makeInvalid(dsAlignedBox3d* result);

/** @copydoc dsAlignedBox3f_makeInvalid() */
DS_GEOMETRY_EXPORT void dsAlignedBox3i_makeInvalid(dsAlignedBox3i* result);

/**
 * @brief Computes the squared distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The squared distance from box to point. If the point is within the box, 0 will be
 * returned. If the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsAlignedBox3f_dist2(const dsAlignedBox3f* box, const dsVector3f* point);

/** @copydoc dsAlignedBox3f_dist2() */
DS_GEOMETRY_EXPORT double dsAlignedBox3d_dist2(const dsAlignedBox3d* box, const dsVector3d* point);

/** @copydoc dsAlignedBox3f_dist2() */
DS_GEOMETRY_EXPORT int dsAlignedBox3i_dist2(const dsAlignedBox3i* box, const dsVector3i* point);

/**
 * @brief Computes the distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The distance from box to point. If the point is within the box, 0 will be returned. If
 * the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsAlignedBox3f_dist(const dsAlignedBox3f* box, const dsVector3f* point);

/** @copydoc dsAlignedBox3f_dist() */
DS_GEOMETRY_EXPORT double dsAlignedBox3d_dist(const dsAlignedBox3d* box, const dsVector3d* point);

/** @copydoc dsAlignedBox3f_dist() */
DS_GEOMETRY_EXPORT double dsAlignedBox3i_dist(const dsAlignedBox3i* box, const dsVector3i* point);

#ifdef __cplusplus
}
#endif
