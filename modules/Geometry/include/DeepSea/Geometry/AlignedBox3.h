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
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsAlignedBox3f dsAlignedBox3d dsAlignedBox3i
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
 * @brief Converts the oriented box to a matrix representation.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
#define dsAlignedBox3_toMatrix(result, box) \
	do \
	{ \
		(result).values[0][0] = ((box).max.x - (box).min.x)/2; \
		(result).values[0][1] = 0; \
		(result).values[0][2] = 0; \
		(result).values[0][3] = 0; \
		(result).values[1][0] = 0; \
		(result).values[1][1] = ((box).max.y - (box).min.y)/2; \
		(result).values[1][2] = 0; \
		(result).values[1][3] = 0; \
		(result).values[2][0] = 0; \
		(result).values[2][1] = 0; \
		(result).values[2][2] = ((box).max.z - (box).min.z)/2; \
		(result).values[2][3] = 0; \
		(result).values[3][0] = ((box).min.x + (box).max.x)/2; \
		(result).values[3][1] = ((box).min.y + (box).max.y)/2; \
		(result).values[3][2] = ((box).min.z + (box).max.z)/2; \
		(result).values[3][3] = 1; \
	} while (0)

/**
 * @brief Extracts the corners from an aligned box.
 * @param[out] corners The corners for the box. This must contain DS_BOX3_CORNER_COUNT elements.
 * @param box The box to extract the corners from.
 */
#define dsAlignedBox3_corners(corners, box) \
	do \
	{ \
		(corners)[dsBox3Corner_xyz].x = (box).min.x; \
		(corners)[dsBox3Corner_xyz].y = (box).min.y; \
		(corners)[dsBox3Corner_xyz].z = (box).min.z; \
		\
		(corners)[dsBox3Corner_xyZ].x = (box).min.x; \
		(corners)[dsBox3Corner_xyZ].y = (box).min.y; \
		(corners)[dsBox3Corner_xyZ].z = (box).max.z; \
		\
		(corners)[dsBox3Corner_xYz].x = (box).min.x; \
		(corners)[dsBox3Corner_xYz].y = (box).max.y; \
		(corners)[dsBox3Corner_xYz].z = (box).min.z; \
		\
		(corners)[dsBox3Corner_xYZ].x = (box).min.x; \
		(corners)[dsBox3Corner_xYZ].y = (box).max.y; \
		(corners)[dsBox3Corner_xYZ].z = (box).max.z; \
		\
		(corners)[dsBox3Corner_Xyz].x = (box).max.x; \
		(corners)[dsBox3Corner_Xyz].y = (box).min.y; \
		(corners)[dsBox3Corner_Xyz].z = (box).min.z; \
		\
		(corners)[dsBox3Corner_XyZ].x = (box).max.x; \
		(corners)[dsBox3Corner_XyZ].y = (box).min.y; \
		(corners)[dsBox3Corner_XyZ].z = (box).max.z; \
		\
		(corners)[dsBox3Corner_XYz].x = (box).max.x; \
		(corners)[dsBox3Corner_XYz].y = (box).max.y; \
		(corners)[dsBox3Corner_XYz].z = (box).min.z; \
		\
		(corners)[dsBox3Corner_XYZ].x = (box).max.x; \
		(corners)[dsBox3Corner_XYZ].y = (box).max.y; \
		(corners)[dsBox3Corner_XYZ].z = (box).max.z; \
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

/** @copydoc dsAlignedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3f_isValid(const dsAlignedBox3f* box)
{
	DS_ASSERT(box);
	return dsAlignedBox3_isValid(*box);
}

/** @copydoc dsAlignedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3d_isValid(const dsAlignedBox3d* box)
{
	DS_ASSERT(box);
	return dsAlignedBox3_isValid(*box);
}

/** @copydoc dsAlignedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3i_isValid(const dsAlignedBox3i* box)
{
	DS_ASSERT(box);
	return dsAlignedBox3_isValid(*box);
}

/** @copydoc dsAlignedBox3_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_addPoint(dsAlignedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_addPoint(dsAlignedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_addPoint(dsAlignedBox3i* box, const dsVector3i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_addBox(dsAlignedBox3f* box,
	const dsAlignedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox3_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_addBox(dsAlignedBox3d* box,
	const dsAlignedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox3_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_addBox(dsAlignedBox3i* box,
	const dsAlignedBox3i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox3_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3f_containsPoint(const dsAlignedBox3f* box,
	const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox3_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3d_containsPoint(const dsAlignedBox3d* box,
	const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox3_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3i_containsPoint(const dsAlignedBox3i* box,
	const dsVector3i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox3_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox3_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3f_containsBox(const dsAlignedBox3f* box,
	const dsAlignedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3d_containsBox(const dsAlignedBox3d* box,
	const dsAlignedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3i_containsBox(const dsAlignedBox3i* box,
	const dsAlignedBox3i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3f_intersects(const dsAlignedBox3f* box,
	const dsAlignedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3d_intersects(const dsAlignedBox3d* box,
	const dsAlignedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox3i_intersects(const dsAlignedBox3i* box,
	const dsAlignedBox3i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox3_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox3_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_intersect(dsAlignedBox3f* result,
	const dsAlignedBox3f* a, const dsAlignedBox3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox3_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox3_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_intersect(dsAlignedBox3d* result,
	const dsAlignedBox3d* a, const dsAlignedBox3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox3_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox3_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_intersect(dsAlignedBox3i* result,
	const dsAlignedBox3i* a, const dsAlignedBox3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox3_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox3_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_center(dsVector3f* result, const dsAlignedBox3f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_center(*result, *box);
}

/** @copydoc dsAlignedBox3_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_center(dsVector3d* result, const dsAlignedBox3d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_center(*result, *box);
}

/** @copydoc dsAlignedBox3_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_center(dsVector3i* result, const dsAlignedBox3i* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_center(*result, *box);
}

/** @copydoc dsAlignedBox3_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_extents(dsVector3f* result, const dsAlignedBox3f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_extents(*result, *box);
}

/** @copydoc dsAlignedBox3_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_extents(dsVector3d* result, const dsAlignedBox3d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_extents(*result, *box);
}

/** @copydoc dsAlignedBox3_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_extents(dsVector3i* result, const dsAlignedBox3i* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_extents(*result, *box);
}

/** @copydoc dsAlignedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_toMatrix(dsMatrix44f* result, dsAlignedBox3f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_toMatrix(*result, *box);
}

/** @copydoc dsAlignedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_toMatrix(dsMatrix44d* result, dsAlignedBox3d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox3_toMatrix(*result, *box);
}

/** @copydoc dsAlignedBox3_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_corners(dsVector3f corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox3_corners(corners, *box);
}

/** @copydoc dsAlignedBox3_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_corners(dsVector3d corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox3_corners(corners, *box);
}

/** @copydoc dsAlignedBox3_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_corners(dsVector3i corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3i* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox3_corners(corners, *box);
}

/** @copydoc dsAlignedBox3_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3f_closestPoint(dsVector3f* result,
	const dsAlignedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_closestPoint(*result, *box, *point);
}

/** @copydoc dsAlignedBox3_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3d_closestPoint(dsVector3d* result,
	const dsAlignedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_closestPoint(*result, *box, *point);
}

/** @copydoc dsAlignedBox3_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox3i_closestPoint(dsVector3i* result,
	const dsAlignedBox3i* box, const dsVector3i* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox3_closestPoint(*result, *box, *point);
}

#ifdef __cplusplus
}
#endif
