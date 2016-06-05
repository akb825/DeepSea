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
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Geometry library.
 */

/**
 * @brief The number of corners for a 2D box.
 */
#define DS_BOX2_CORNER_COUNT 4

/**
 * @brief The number of corners for a 3D box.
 */
#define DS_BOX3_CORNER_COUNT 8

/**
 * @brief Enum for the result of an intersection.
 */
typedef enum dsIntersectResult
{
	dsIntersectResult_Inside,    ///< Lies fully inside the shape boundary.
	dsIntersectResult_Outside,   ///< Lies fully outside the shape boundary.
	dsIntersectResult_Intersects ///< Intersects the shape boundary.
} dsIntersectResult;

/**
 * @brief Structure for 2D axis-aligned bounding box using floats.
 */
typedef struct dsAlignedBox2f
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector2f min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector2f max;
} dsAlignedBox2f;

/**
 * @brief Structure for 2D axis-aligned bounding box using doubles.
 */
typedef struct dsAlignedBox2d
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector2d min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector2d max;
} dsAlignedBox2d;

/**
 * @brief Structure for 2D axis-aligned bounding box using ints.
 */
typedef struct dsAlignedBox2i
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector2i min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector2i max;
} dsAlignedBox2i;

/**
 * @brief Structure for 3D axis-aligned bounding box using floats.
 */
typedef struct dsAlignedBox3f
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector3f min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector3f max;
} dsAlignedBox3f;

/**
 * @brief Structure for 3D axis-aligned bounding box using doubles.
 */
typedef struct dsAlignedBox3d
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector3d min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector3d max;
} dsAlignedBox3d;

/**
 * @brief Structure for 3D axis-aligned bounding box using ints.
 */
typedef struct dsAlignedBox3i
{
	/**
	 * @brief Minimum values for the box.
	 */
	dsVector3i min;

	/**
	 * @brief Maximum values for the box.
	 */
	dsVector3i max;
} dsAlignedBox3i;

/**
 * @brief Structure for a 2D oriented bounding box using floats.
 */
typedef struct dsOrientedBox2f
{
	/**
	 * @brief The orientation of the box.
	 *
	 * This must contain only a rotation. This transforms from local aligned box space to world
	 * space.
	 */
	dsMatrix22f orientation;

	/**
	 * @brief The center of the box.
	 */
	dsVector2f center;

	/**
	 * @brief The half width and height of the box.
	 */
	dsVector2f halfExtents;
} dsOrientedBox2f;

/**
 * @brief Structure for a 2D oriented bounding box using doubles.
 */
typedef struct dsOrientedBox2d
{
	/**
	 * @brief The orientation of the box.
	 *
	 * This must contain only a rotation. This transforms from local aligned box space to world
	 * space.
	 */
	dsMatrix22d orientation;

	/**
	 * @brief The center of the box.
	 */
	dsVector2d center;

	/**
	 * @brief The half width and height of the box.
	 */
	dsVector2d halfExtents;
} dsOrientedBox2d;

/**
 * @brief Structure for a 3D oriented bounding box using floats.
 */
typedef struct dsOrientedBox3f
{
	/**
	 * @brief The orientation of the box.
	 *
	 * This must contain only a rotation. This transforms from local aligned box space to world
	 * space.
	 */
	dsMatrix33f orientation;

	/**
	 * @brief The center of the box.
	 */
	dsVector3f center;

	/**
	 * @brief The half width, height, and depth of the box.
	 */
	dsVector3f halfExtents;
} dsOrientedBox3f;

/**
 * @brief Structure for a 3D oriented bounding box using doubles.
 */
typedef struct dsOrientedBox3d
{
	/**
	 * @brief The orientation of the box.
	 *
	 * This must contain only a rotation. This transforms from local aligned box space to world
	 * space.
	 */
	dsMatrix33d orientation;

	/**
	 * @brief The center of the box.
	 */
	dsVector3d center;

	/**
	 * @brief The half width, height, and depth of the box.
	 */
	dsVector3d halfExtents;
} dsOrientedBox3d;

/**
 * @brief Structure for a plane using floats.
 */
typedef struct dsPlane3f
{
	/**
	 * @brief The normal of the plane.
	 */
	dsVector3f n;

	/**
	 * @brief The distane from the origin along the normal to the plane.
	 */
	float d;
} dsPlane3f;

/**
 * @brief Structure for a plane using doubles.
 */
typedef struct dsPlane3d
{
	/**
	 * @brief The normal of the plane.
	 */
	dsVector3d n;

	/**
	 * @brief The distane from the origin along the normal to the plane.
	 */
	double d;
} dsPlane3d;

/**
 * @brief Enum for the planes within a frustum.
 */
typedef enum dsFrustumPlanes
{
	dsFrustumPlanes_Left,   ///< The left plane.
	dsFrustumPlanes_Right,  ///< The right plane.
	dsFrustumPlanes_Bottom, ///< The bottom plane.
	dsFrustumPlanes_Top,    ///< The top plane.
	dsFrustumPlanes_Near,   ///< The near plane.
	dsFrustumPlanes_Far,    ///< The far plane.
	dsFrustumPlanes_Count   ///< The number of planes.
} dsFrustumPlanes;

/**
 * @brief Structure for a frustum using floats.
 */
typedef struct dsFrustum3f
{
	/**
	 * @brief The planes for the frustum.
	 */
	dsPlane3f planes[dsFrustumPlanes_Count];
} dsFrustum3f;

/**
 * @brief Structure for a frustum using doubles.
 */
typedef struct dsFrustum3d
{
	/**
	 * @brief The planes for the frustum.
	 */
	dsPlane3d planes[dsFrustumPlanes_Count];
} dsFrustum3d;

#ifdef __cplusplus
}
#endif
