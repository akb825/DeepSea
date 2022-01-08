/*
 * Copyright 2016-2021 Aaron Barany
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
 * @brief Macros and functions for manipulating dsAlignedBox2* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsAlignedBox2* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsAlignedBox2 types. These are used when
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsAlignedBox2f dsAlignedBox2d dsAlignedBox2i
 */

/**
 * @brief Returns whether or not a box is valid.
 *
 * A box is invalid if a minimum value is larger than a maximum value.
 *
 * @param box The box to check.
 * @return True if the box is valid.
 */
#define dsAlignedBox2_isValid(box) \
	((box).min.x <= (box).max.x && (box).min.y <= (box).max.y)

/**
 * @brief Adds a point to the box.
 * @param[inout] box The box to add the point to.
 * @param point The point to add.
 */
#define dsAlignedBox2_addPoint(box, point) \
	do \
	{ \
		(box).min.x = dsMin((box).min.x, (point).x); \
		(box).min.y = dsMin((box).min.y, (point).y); \
		(box).max.x = dsMax((box).max.x, (point).x); \
		(box).max.y = dsMax((box).max.y, (point).y); \
	} while (0)

/**
 * @brief Adds one box to another.
 * @param[inout] box The box to add the other box to.
 * @param otherBox The box to add.
 */
#define dsAlignedBox2_addBox(box, otherBox) \
	do \
	{ \
		(box).min.x = dsMin((box).min.x, (otherBox).min.x); \
		(box).min.y = dsMin((box).min.y, (otherBox).min.y); \
		(box).max.x = dsMax((box).max.x, (otherBox).max.x); \
		(box).max.y = dsMax((box).max.y, (otherBox).max.y); \
	} while (0)

/**
 * @brief Checks if a box contains a point.
 * @param box The box to check.
 * @param point The point to check within the box.
 * @return True if the point is contained in the box.
 */
#define dsAlignedBox2_containsPoint(box, point) \
	((point).x >= (box).min.x && (point).y >= (box).min.y && \
	 (point).x <= (box).max.x && (point).y <= (box).max.y)

/**
 * @brief Checks if a box contains another box.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box contains otherBox.
 */
#define dsAlignedBox2_containsBox(box, otherBox) \
	((box).min.x <= (otherBox).min.x && (box).min.y <= (otherBox).min.y && \
	 (box).max.x >= (otherBox).max.x && (box).max.y >= (otherBox).max.y)

/**
 * @brief Checks if a box intersects with another box.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box intersects with otherBox.
 */
#define dsAlignedBox2_intersects(box, otherBox) \
	((box).min.x <= (otherBox).max.x && (box).min.y <= (otherBox).max.y && \
	 (box).max.x >= (otherBox).min.x && (box).max.y >= (otherBox).min.y)

/**
 * @brief Creates a box by intersecting two boxes.
 * @param[out] result The intersected box.
 * @param a The first box to intersect.
 * @param b The second box to intersect.
 */
#define dsAlignedBox2_intersect(result, a, b) \
	do \
	{ \
		(result).min.x = dsMax((a).min.x, (b).min.x); \
		(result).min.y = dsMax((a).min.y, (b).min.y); \
		(result).max.x = dsMin((a).max.x, (b).max.x); \
		(result).max.y = dsMin((a).max.y, (b).max.y); \
	} while (0)

/**
 * @brief Computes the center of the box.
 * @param[out] result The dsVector2* type for the center.
 * @param box The box to compute the center for.
 */
#define dsAlignedBox2_center(result, box) \
	do \
	{ \
		(result).x = ((box).min.x + (box).max.x)/2; \
		(result).y = ((box).min.y + (box).max.y)/2; \
	} while (0)

/**
 * @brief Computes the extents of the box.
 *
 * The extents are the distance between the min and max bounds along each axis.
 *
 * @param[out] result The dsVector2* type for the extents.
 * @param box The box to compute the extents for.
 */
#define dsAlignedBox2_extents(result, box) \
	do \
	{ \
		(result).x = (box).max.x - (box).min.x; \
		(result).y = (box).max.y - (box).min.y; \
	} while (0)

/**
 * @brief Extracts the corners from an aligned box.
 * @param[out] corners The corners for the box. This must contain DS_BOX2_CORNER_COUNT elements.
 * @param box The box to extract the corners from.
 */
#define dsAlignedBox2_corners(corners, box) \
	do \
	{ \
		(corners)[dsBox2Corner_xy].x = (box).min.x; \
		(corners)[dsBox2Corner_xy].y = (box).min.y; \
		\
		(corners)[dsBox2Corner_xY].x = (box).min.x; \
		(corners)[dsBox2Corner_xY].y = (box).max.y; \
		\
		(corners)[dsBox2Corner_Xy].x = (box).max.x; \
		(corners)[dsBox2Corner_Xy].y = (box).min.y; \
		\
		(corners)[dsBox2Corner_XY].x = (box).max.x; \
		(corners)[dsBox2Corner_XY].y = (box).max.y; \
	} while (0)

/**
 * @brief Computes the closest point to the box.
 * @param[out] result The closest point.
 * @param box The box to compute the closest point on.
 * @param point The point to check.
 */
#define dsAlignedBox2_closestPoint(result, box, point) \
	do \
	{ \
		if (dsAlignedBox2_isValid(box)) \
		{ \
			(result).x = dsMax((box).min.x, (point).x); \
			(result).x = dsMin((box).max.x, (result).x); \
			\
			(result).y = dsMax((box).min.y, (point).y); \
			(result).y = dsMin((box).max.y, (result).y); \
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
DS_GEOMETRY_EXPORT void dsAlignedBox2f_makeInvalid(dsAlignedBox2f* result);

/** @copydoc dsAlignedBox2f_makeInvalid() */
DS_GEOMETRY_EXPORT void dsAlignedBox2d_makeInvalid(dsAlignedBox2d* result);

/** @copydoc dsAlignedBox2f_makeInvalid() */
DS_GEOMETRY_EXPORT void dsAlignedBox2i_makeInvalid(dsAlignedBox2i* result);

/**
 * @brief Computes the squared distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The squared distance from box to point. If the point is within the box, 0 will be
 * returned. If the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsAlignedBox2f_dist2(const dsAlignedBox2f* box,
	const dsVector2f* point);

/** @copydoc dsAlignedBox2f_dist2() */
DS_GEOMETRY_EXPORT double dsAlignedBox2d_dist2(const dsAlignedBox2d* box,
	const dsVector2d* point);

/** @copydoc dsAlignedBox2f_dist2() */
DS_GEOMETRY_EXPORT int dsAlignedBox2i_dist2(const dsAlignedBox2i* box,
	const dsVector2i* point);

/**
 * @brief Computes the distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The distance from box to point. If the point is within the box, 0 will be returned. If
 * the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsAlignedBox2f_dist(const dsAlignedBox2f* box,
	const dsVector2f* point);

/** @copydoc dsAlignedBox2f_dist() */
DS_GEOMETRY_EXPORT double dsAlignedBox2d_dist(const dsAlignedBox2d* box,
	const dsVector2d* point);

/** @copydoc dsAlignedBox2f_dist() */
DS_GEOMETRY_EXPORT double dsAlignedBox2i_dist(const dsAlignedBox2i* box,
	const dsVector2i* point);

/** @copydoc dsAlignedBox2_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2f_isValid(const dsAlignedBox2f* box)
{
	DS_ASSERT(box);
	return dsAlignedBox2_isValid(*box);
}

/** @copydoc dsAlignedBox2_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2d_isValid(const dsAlignedBox2d* box)
{
	DS_ASSERT(box);
	return dsAlignedBox2_isValid(*box);
}

/** @copydoc dsAlignedBox2_isValid() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2i_isValid(const dsAlignedBox2i* box)
{
	DS_ASSERT(box);
	return dsAlignedBox2_isValid(*box);
}

/** @copydoc dsAlignedBox2_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_addPoint(dsAlignedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_addPoint(dsAlignedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_addPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_addPoint(dsAlignedBox2i* box, const dsVector2i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_addPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_addBox(dsAlignedBox2f* box,
	const dsAlignedBox2f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox2_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_addBox(dsAlignedBox2d* box,
	const dsAlignedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox2_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_addBox() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_addBox(dsAlignedBox2i* box,
	const dsAlignedBox2i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	dsAlignedBox2_addBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2f_containsPoint(const dsAlignedBox2f* box,
	const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox2_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2d_containsPoint(const dsAlignedBox2d* box,
	const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox2_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_containsPoint() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2i_containsPoint(const dsAlignedBox2i* box,
	const dsVector2i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);
	return dsAlignedBox2_containsPoint(*box, *point);
}

/** @copydoc dsAlignedBox2_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2f_containsBox(const dsAlignedBox2f* box,
	const dsAlignedBox2f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2d_containsBox(const dsAlignedBox2d* box,
	const dsAlignedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_containsBox() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2i_containsBox(const dsAlignedBox2i* box,
	const dsAlignedBox2i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_containsBox(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2f_intersects(const dsAlignedBox2f* box,
	const dsAlignedBox2f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2d_intersects(const dsAlignedBox2d* box,
	const dsAlignedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_intersects() */
DS_GEOMETRY_EXPORT inline bool dsAlignedBox2i_intersects(const dsAlignedBox2i* box,
	const dsAlignedBox2i* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);
	return dsAlignedBox2_intersects(*box, *otherBox);
}

/** @copydoc dsAlignedBox2_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_intersect(dsAlignedBox2f* result,
	const dsAlignedBox2f* a, const dsAlignedBox2f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox2_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox2_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_intersect(dsAlignedBox2d* result,
	const dsAlignedBox2d* a, const dsAlignedBox2d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox2_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox2_intersect() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_intersect(dsAlignedBox2i* result,
	const dsAlignedBox2i* a, const dsAlignedBox2i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsAlignedBox2_intersect(*result, *a, *b);
}

/** @copydoc dsAlignedBox2_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_center(dsVector2f* result, const dsAlignedBox2f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_center(*result, *box);
}

/** @copydoc dsAlignedBox2_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_center(dsVector2d* result, const dsAlignedBox2d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_center(*result, *box);
}

/** @copydoc dsAlignedBox2_center() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_center(dsVector2i* result, const dsAlignedBox2i* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_center(*result, *box);
}

/** @copydoc dsAlignedBox2_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_extents(dsVector2f* result, const dsAlignedBox2f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_extents(*result, *box);
}

/** @copydoc dsAlignedBox2_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_extents(dsVector2d* result, const dsAlignedBox2d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_extents(*result, *box);
}

/** @copydoc dsAlignedBox2_extents() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_extents(dsVector2i* result, const dsAlignedBox2i* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsAlignedBox2_extents(*result, *box);
}

/** @copydoc dsAlignedBox2_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_corners(dsVector2f corners[DS_BOX2_CORNER_COUNT],
	const dsAlignedBox2f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox2_corners(corners, *box);
}

/** @copydoc dsAlignedBox2_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_corners(dsVector2d corners[DS_BOX2_CORNER_COUNT],
	const dsAlignedBox2d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox2_corners(corners, *box);
}

/** @copydoc dsAlignedBox2_corners() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_corners(dsVector2i corners[DS_BOX2_CORNER_COUNT],
	const dsAlignedBox2i* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);
	dsAlignedBox2_corners(corners, *box);
}

/** @copydoc dsAlignedBox2_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2f_closestPoint(dsVector2f* result,
	const dsAlignedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_closestPoint(*result, *box, *point);
}

/** @copydoc dsAlignedBox2_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2d_closestPoint(dsVector2d* result,
	const dsAlignedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_closestPoint(*result, *box, *point);
}

/** @copydoc dsAlignedBox2_closestPoint() */
DS_GEOMETRY_EXPORT inline void dsAlignedBox2i_closestPoint(dsVector2i* result,
	const dsAlignedBox2i* box, const dsVector2i* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);
	dsAlignedBox2_closestPoint(*result, *box, *point);
}

#ifdef __cplusplus
}
#endif
