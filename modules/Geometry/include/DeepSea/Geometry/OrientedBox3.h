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
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Matrix33.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsOrientedBox3* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsOrientedBox3* type. All parameters
 * are by value.
 *
 * The functions have different versions for the supported dsOrientedBox3 types. These are used when
 * the implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsOrientedBox3f dsOrientedBox3d
 */

/**
 * @brief Returns whether or not a box is valid.
 *
 * A box is invalid if any of the extents are negative.
 *
 * @param box The box to check.
 * @return True if the box is valid.
 */
#define dsOrientedBox3_isValid(box) \
	((box).halfExtents.x >= 0 && (box).halfExtents.y >= 0 && (box).halfExtents.z >= 0)

/**
 * @brief Makes an oriented box from an aligned box.
 * @param[out] result The oriented box.
 * @param alignedBox The aligned box to make the oriented box from.
 */
#define dsOrientedBox3_fromAlignedBox(result, alignedBox) \
	do \
	{ \
		dsMatrix33_identity((result).orientation); \
		dsAlignedBox3_center((result).center, alignedBox); \
		dsAlignedBox3_extents((result).halfExtents, alignedBox); \
		(result).halfExtents.x /= 2; \
		(result).halfExtents.y /= 2; \
		(result).halfExtents.z /= 2; \
	} while (0)

/**
 * @brief Converts the oriented box to a matrix representation.
 *
 * The matrix will convert (-1, -1, -1) to the min point and (1, 1, 1) to the max point.
 *
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
#define dsOrientedBox3_toMatrix(result, box) \
	do \
	{ \
		(result).values[0][0] = (box).orientation.values[0][0]*(box).halfExtents.x; \
		(result).values[0][1] = (box).orientation.values[0][1]*(box).halfExtents.x; \
		(result).values[0][2] = (box).orientation.values[0][2]*(box).halfExtents.x; \
		(result).values[0][3] = 0; \
		(result).values[1][0] = (box).orientation.values[1][0]*(box).halfExtents.y; \
		(result).values[1][1] = (box).orientation.values[1][1]*(box).halfExtents.y; \
		(result).values[1][2] = (box).orientation.values[1][2]*(box).halfExtents.y; \
		(result).values[1][3] = 0; \
		(result).values[2][0] = (box).orientation.values[2][0]*(box).halfExtents.z; \
		(result).values[2][1] = (box).orientation.values[2][1]*(box).halfExtents.z; \
		(result).values[2][2] = (box).orientation.values[2][2]*(box).halfExtents.z; \
		(result).values[2][3] = 0; \
		(result).values[3][0] = (box).center.x; \
		(result).values[3][1] = (box).center.y; \
		(result).values[3][2] = (box).center.z; \
		(result).values[3][3] = 1; \
	} while (0)

/**
 * @brief Converts the oriented box to a transposed matrix representation.
 *
 * This is useful to avoid the transpose in specific situations where the transpose is used.
 *
 * @param[out] result The matrix.
 * @param box The box to convert.
 */
#define dsOrientedBox3_toMatrixTranspose(result, box) \
	do \
	{ \
		(result).values[0][0] = (box).orientation.values[0][0]*(box).halfExtents.x; \
		(result).values[1][0] = (box).orientation.values[0][1]*(box).halfExtents.x; \
		(result).values[2][0] = (box).orientation.values[0][2]*(box).halfExtents.x; \
		(result).values[3][0] = 0; \
		(result).values[0][1] = (box).orientation.values[1][0]*(box).halfExtents.y; \
		(result).values[1][1] = (box).orientation.values[1][1]*(box).halfExtents.y; \
		(result).values[2][1] = (box).orientation.values[1][2]*(box).halfExtents.y; \
		(result).values[3][1] = 0; \
		(result).values[0][2] = (box).orientation.values[2][0]*(box).halfExtents.z; \
		(result).values[1][2] = (box).orientation.values[2][1]*(box).halfExtents.z; \
		(result).values[2][2] = (box).orientation.values[2][2]*(box).halfExtents.z; \
		(result).values[3][2] = 0; \
		(result).values[0][3] = (box).center.x; \
		(result).values[1][3] = (box).center.y; \
		(result).values[2][3] = (box).center.z; \
		(result).values[3][3] = 1; \
	} while (0)

/**
 * @brief Makes an invalid box.
 *
 * This will set the extents to -1.
 *
 * @param[out] result The box to make invalid.
 */
#define dsOrientedBox3_makeInvalid(result) \
	do \
	{ \
		(result).halfExtents.x = -1; \
		(result).halfExtents.y = -1; \
		(result).halfExtents.z = -1; \
	} while (0)

/**
 * @brief Creates an oriented box from a matrix representation.
 * @param[out] result The oriented box.
 * @param matrix The matrix representation.
 */
DS_GEOMETRY_EXPORT void dsOrientedBox3f_fromMatrix(dsOrientedBox3f* result,
	const dsMatrix44f* matrix);

/** @copydoc dsOrientedBox3f_fromMatrix() */
DS_GEOMETRY_EXPORT void dsOrientedBox3d_fromMatrix(dsOrientedBox3d* result,
	const dsMatrix44d* matrix);

/**
 * @brief Transforms an oriented box.
 * @param box The box to transform.
 * @param transform The transformation matrix. This may contain a scale.
 * @return False if the box is invalid.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_transform(dsOrientedBox3f* box,
	const dsMatrix44f* transform);

/** @copydoc dsOrientedBox3f_transform() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_transform(dsOrientedBox3d* box,
	const dsMatrix44d* transform);

/**
 * @brief Adds a point to an oriented box.
 * @remark The box may be invalid, but the orientation must be set.
 * @param box The box to add the point to.
 * @param point The point to add to the box.
 */
DS_GEOMETRY_EXPORT void dsOrientedBox3f_addPoint(dsOrientedBox3f* box, const dsVector3f* point);

/** @copydoc dsOrientedBox3f_addPoint() */
DS_GEOMETRY_EXPORT void dsOrientedBox3d_addPoint(dsOrientedBox3d* box, const dsVector3d* point);

/**
 * @brief Adds a box to another oriented box.
 * @param box The box to add the other box to. If invalid, it will be set to otherBox
 * @param otherBox The other box to add.
 * @return False if otherBox is empty.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_addBox(dsOrientedBox3f* box,
	const dsOrientedBox3f* otherBox);

/** @copydoc dsOrientedBox3f_addBox() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_addBox(dsOrientedBox3d* box,
	const dsOrientedBox3d* otherBox);

/**
 * @brief Extracts the corners from an oriented box.
 * @param[out] corners The corners for the box.
 * @param box The box to extract the corners from.
 * @return False if box is empty.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_corners(dsVector3f corners[DS_BOX3_CORNER_COUNT],
	const dsOrientedBox3f* box);

/** @copydoc dsOrientedBox3f_corners() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_corners(dsVector3d corners[DS_BOX3_CORNER_COUNT],
	const dsOrientedBox3d* box);

/**
 * @brief Tests if one box intersects with another.
 * @param box The box to check.
 * @param otherBox The other box to check.
 * @return True if box intersects with otherBox.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_intersects(const dsOrientedBox3f* box,
	const dsOrientedBox3f* otherBox);

/** @copydoc dsOrientedBox3f_intersects() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_intersects(const dsOrientedBox3d* box,
	const dsOrientedBox3d* otherBox);

/**
 * @brief Checks whether or not a point is contained in an oriented box.
 * @param box The box to check.
 * @param point The point to check for containment.
 * @return True if the point is within the box.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_containsPoint(const dsOrientedBox3f* box,
	const dsVector3f* point);

/** @copydoc dsOrientedBox3f_containsPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_containsPoint(const dsOrientedBox3d* box,
	const dsVector3d* point);

/**
 * @brief Computes the closest point to the box.
 * @param[out] result The closest point.
 * @param box The box to compute the closest point on.
 * @param point The point to check.
 * @return False if the box is invalid.
 */
DS_GEOMETRY_EXPORT bool dsOrientedBox3f_closestPoint(dsVector3f* result, const dsOrientedBox3f* box,
	const dsVector3f* point);

/** @copydoc dsOrientedBox2f_closestPoint() */
DS_GEOMETRY_EXPORT bool dsOrientedBox3d_closestPoint(dsVector3d* result, const dsOrientedBox3d* box,
	const dsVector3d* point);

/**
 * @brief Computes the squared distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The squared distance from box to point. If the point is within the box, 0 will be
 * returned. If the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsOrientedBox3f_dist2(const dsOrientedBox3f* box,
	const dsVector3f* point);

/** @copydoc dsOrientedBox3f_dist2() */
DS_GEOMETRY_EXPORT double dsOrientedBox3d_dist2(const dsOrientedBox3d* box,
	const dsVector3d* point);

/**
 * @brief Computes the distance from a box to a point.
 * @param box The box to compute the point from.
 * @param point The point to compute the distance to.
 * @return The distance from box to point. If the point is within the box, 0 will be returned. If
 * the box is invalid, -1 will be returned.
 */
DS_GEOMETRY_EXPORT float dsOrientedBox3f_dist(const dsOrientedBox3f* box,
	const dsVector3f* point);

/** @copydoc dsOrientedBox3f_dist() */
DS_GEOMETRY_EXPORT double dsOrientedBox3d_dist(const dsOrientedBox3d* box,
	const dsVector3d* point);

/** @copydoc dsOrientedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox3f_isValid(const dsOrientedBox3f* box)
{
	DS_ASSERT(box);
	return dsOrientedBox3_isValid(*box);
}

/** @copydoc dsOrientedBox3_isValid() */
DS_GEOMETRY_EXPORT inline bool dsOrientedBox3d_isValid(const dsOrientedBox3d* box)
{
	DS_ASSERT(box);
	return dsOrientedBox3_isValid(*box);
}

/** @copydoc dsOrientedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3f_toMatrix(dsMatrix44f* result,
	const dsOrientedBox3f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsOrientedBox3_toMatrix(*result, *box);
}

/** @copydoc dsOrientedBox3_toMatrix() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3d_toMatrix(dsMatrix44d* result,
	const dsOrientedBox3d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsOrientedBox3_toMatrix(*result, *box);
}

/** @copydoc dsOrientedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3f_toMatrixTranspose(dsMatrix44f* result,
	const dsOrientedBox3f* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsOrientedBox3_toMatrixTranspose(*result, *box);
}

/** @copydoc dsOrientedBox3_toMatrixTranspose() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3d_toMatrixTranspose(dsMatrix44d* result,
	const dsOrientedBox3d* box)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	dsOrientedBox3_toMatrixTranspose(*result, *box);
}

/** @copydoc dsOrientedBox3_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3f_fromAlignedBox(dsOrientedBox3f* result,
	const dsAlignedBox3f* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);
	dsOrientedBox3_fromAlignedBox(*result, *alignedBox);
}

/** @copydoc dsOrientedBox3_fromAlignedBox() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3d_fromAlignedBox(dsOrientedBox3d* result,
	const dsAlignedBox3d* alignedBox)
{
	DS_ASSERT(result);
	DS_ASSERT(alignedBox);
	dsOrientedBox3_fromAlignedBox(*result, *alignedBox);
}

/** @copydoc dsOrientedBox3_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3f_makeInvalid(dsOrientedBox3f* result)
{
	DS_ASSERT(result);
	dsOrientedBox3_makeInvalid(*result);
}

/** @copydoc dsOrientedBox3_makeInvalid() */
DS_GEOMETRY_EXPORT inline void dsOrientedBox3d_makeInvalid(dsOrientedBox3d* result)
{
	DS_ASSERT(result);
	dsOrientedBox3_makeInvalid(*result);
}

#ifdef __cplusplus
}
#endif
