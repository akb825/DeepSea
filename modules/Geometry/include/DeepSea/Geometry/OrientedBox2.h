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
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Matrix22.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsOrientedBox2* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsOrientedBox2* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsOrientedBox2 types. These are used when
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsOrientedBox2f dsOrientedBox2d
 */

/**
 * @brief Returns whether or not a box is valid.
 *
 * A box is invalid if any of the extents are negative.
 *
 * @param box The box to check.
 * @return True if the box is valid.
 */
#define dsOrientedBox2_isValid(box) \
	((box).halfExtents.x >= 0 && (box).halfExtents.y >= 0)

/**
 * @brief Makes an oriented box from an aligned box.
 * @param[out] result The oriented box.
 * @param alignedBox The aligned box to make the oriented box from.
 */
#define dsOrientedBox2_fromAlignedBox(result, alignedBox) \
	do \
	{ \
		dsMatrix22_identity((result).orientation); \
		dsAlignedBox2_center((result).center, alignedBox); \
		dsAlignedBox2_extents((result).halfExtents, alignedBox); \
		(result).halfExtents.x /= 2; \
		(result).halfExtents.y /= 2; \
	} while (0)

/**
 * @brief Makes an invalid box.
 *
 * This will set the extents to -1.
 *
 * @param[out] result The box to make invalid.
 */
#define dsOrientedBox2_makeInvalid(result) \
	do \
	{ \
		(result).halfExtents.x = -1; \
		(result).halfExtents.y = -1; \
	} while (0)

/**
 * @brief Transforms an oriented box.
 * @param box The box to transform.
 * @param transform The transformation matrix. This may contain a scale.
 * @return False if the box is invalid.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox2f_transform(dsOrientedBox2f* box,
	const dsMatrix33f* transform);

/** @copydoc dsOrientedBox2f_transform() */
DS_GEOMETRY_EXPORT bool dsOrientedBox2d_transform(dsOrientedBox2d* box,
	const dsMatrix33d* transform);

/**
 * @brief Adds a point to an oriented box.
 * @remark The box may be invalid, but the orientation must be set.
 * @param box The box to add the point to.
 * @param point The point to add to the box.
 */
DS_GEOMETRY_EXPORT void dsOrientedBox2f_addPoint(dsOrientedBox2f* box, const dsVector2f* point);

/** @copydoc dsOrientedBox2f_addPoint() */
DS_GEOMETRY_EXPORT void dsOrientedBox2d_addPoint(dsOrientedBox2d* box, const dsVector2d* point);

/**
 * @brief Adds a box to another oriented box.
 * @param box The box to add the other box to. If invalid, it will be set to otherBox
 * @param otherBox The other box to add.
 * @return False if otherBox is empty.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox2f_addBox(dsOrientedBox2f* box,
	const dsOrientedBox2f* otherBox);

/** @copydoc dsOrientedBox2f_addBox() */
DS_GEOMETRY_EXPORT bool dsOrientedBox2d_addBox(dsOrientedBox2d* box,
	const dsOrientedBox2d* otherBox);

/**
 * @brief Extracts the corners from an oriented box.
 * @param[out] corners The corners for the box.
 * @param box The box to extract the corners from.
 * @return False if box is empty.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox2f_corners(dsVector2f corners[DS_BOX2_CORNER_COUNT],
	const dsOrientedBox2f* box);

/** @copydoc dsOrientedBox2f_corners() */
DS_GEOMETRY_EXPORT bool dsOrientedBox2d_corners(dsVector2d corners[DS_BOX2_CORNER_COUNT],
	const dsOrientedBox2d* box);

/**
 * @brief Tests if one box intersects with another.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box intersects with otherBox.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox2f_intersects(const dsOrientedBox2f* box,
	const dsOrientedBox2f* otherBox);

/** @copydoc dsOrientedBox2f_intersects() */
DS_GEOMETRY_EXPORT bool dsOrientedBox2d_intersects(const dsOrientedBox2d* box,
	const dsOrientedBox2d* otherBox);

/**
 * @brief Computes the closest point to the box.
 * @param[out] result The closest point.
 * @param box The box to compute the closest point on.
 * @param point The point to check.
 * @return False if the box is invalid.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox2f_closestPoint(dsVector2f* result, const dsOrientedBox2f* box,
	const dsVector2f* point);

/** @copydoc dsOrientedBox2f_closestPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox2d_closestPoint(dsVector2d* result, const dsOrientedBox2d* box,
	const dsVector2d* point);

/**
 * @brief Computes the squared distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The squared distance from box to point. If the point is within the box, 0 will be
 * returned. If the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsOrientedBox2f_dist2(const dsOrientedBox2f* box,
	const dsVector2f* point);

/** @copydoc dsOrientedBox2f_dist2() */
DS_GEOMETRY_EXPORT double dsOrientedBox2d_dist2(const dsOrientedBox2d* box,
	const dsVector2d* point);

/**
 * @brief Computes the distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The distance from box to point. If the point is within the box, 0 will be returned. If
 * the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsOrientedBox2f_dist(const dsOrientedBox2f* box,
	const dsVector2f* point);

/** @copydoc dsOrientedBox2f_dist() */
DS_GEOMETRY_EXPORT double dsOrientedBox2d_dist(const dsOrientedBox2d* box,
	const dsVector2d* point);

/** @copydoc dsOrientedBox2_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox2f_isValid(const dsOrientedBox2f* box)
{
	DS_ASSERT(box);
	return dsOrientedBox2_isValid(*box);
}

/** @copydoc dsOrientedBox2_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox2d_isValid(const dsOrientedBox2d* box)
{
	DS_ASSERT(box);
	return dsOrientedBox2_isValid(*box);
}

/** @copydoc dsOrientedBox2_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox2f_fromAlignedBox(dsOrientedBox2f* result,
	const dsAlignedBox2f* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);
	dsOrientedBox2_fromAlignedBox(*result, *alignedBox);
}

/** @copydoc dsOrientedBox2_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox2d_fromAlignedBox(dsOrientedBox2d* result,
	const dsAlignedBox2d* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);
	dsOrientedBox2_fromAlignedBox(*result, *alignedBox);
}

/** @copydoc dsOrientedBox2_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox2f_makeInvalid(dsOrientedBox2f* result)
{
	DS_ASSERT(result);
	dsOrientedBox2_makeInvalid(*result);
}

/** @copydoc dsOrientedBox2_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox2d_makeInvalid(dsOrientedBox2d* result)
{
	DS_ASSERT(result);
	dsOrientedBox2_makeInvalid(*result);
}

#ifdef __cplusplus
}
#endif
