/*
 * Copyright 2016-2018 Aaron Barany
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
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Geometry library.
 */

/**
 * @brief Log tag used by the geometry library.
 */
#define DS_GEOMETRY_LOG_TAG "geometry"

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
 * @brief Enum for the element type of a geometry structure.
 */
typedef enum dsGeometryElement
{
	dsGeometryElement_Float,  ///< float (e.g. dsVector2f)
	dsGeometryElement_Double, ///< double (e.g. dsVector2d)
	dsGeometryElement_Int     ///< int (e.g. dsVector2i)
} dsGeometryElement;

/**
 * @brief Structure for 2D axis-aligned bounding box using floats.
 * @see AlignedBox2.h
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
 * @see AlignedBox2.h
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
 * @see AlignedBox2.h
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
 * @see AlignedBox3.h
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
 * @see AlignedBox3.h
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
 * @see AlignedBox3.h
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
 * @see OrientedBox2.h
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
 * @see OrientedBox2.h
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
 * @see OrientedBox3.h
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
 * @see OrientedBox3.h
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
 * @see Plane3.h
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
 * @see Plane3.h
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
 * @see Frustum3.h
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
 * @see Frustum3.h
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
 * @see Frustum3.h
 */
typedef struct dsFrustum3d
{
	/**
	 * @brief The planes for the frustum.
	 */
	dsPlane3d planes[dsFrustumPlanes_Count];
} dsFrustum3d;

/**
 * @brief Structure for a bounding volume hierarchy spacial data structure.
 * @see BVH.h
 */
typedef struct dsBVH dsBVH;

/**
 * @brief Function for getting the bounds for an object.
 * @remark errno should be set on failure.
 * @param[out] outBounds The memory to place the bounding box into. This should be cast to the
 *    appropriate dsAlignedBounds* type based on axisCount and element from the BVH.
 * @param bvh The BVH the bounds will be queried for.
 * @param object The object to get the bounds from.
 * @return True if outBounds was successfully assigned.
 * @see BVH.h
 */
typedef bool (*dsBVHObjectBoundsFunction)(void* outBounds, const dsBVH* bvh, const void* object);

/**
 * @brief Function called when visiting BVH nodes that intersect.
 * @param userData User data forwarded for the function.
 * @param bvh The BVH that the intersection was performed with.
 * @param object The object that was visited.
 * @param bounds The bounds being checked. This should be cast to the appropriate dsAlignedBounds*
 *     type based on the axis count and precision queried from bvh.
 * @return True to continue traversal, false to stop traversal.
 * @see BVH.h
 */
typedef bool (*dsBVHVisitFunction)(void* userData, const dsBVH* bvh, const void* object,
	const void* bounds);

/**
 * @brief Enum for the winding order when triangulating geometry.
 */
typedef enum dsTriangulateWinding
{
	dsTriangulateWinding_CW, ///< Clockwise winding order
	dsTriangulateWinding_CCW ///< Counter-clockwise winding order
} dsTriangulateWinding;

/**
 * @brief Structure to define a simple polygon for triangulation.
 * @see SimplePolygon.h
 */
typedef struct dsSimplePolygon dsSimplePolygon;

/**
 * @brief Function for getting the position of a polygon point for a simple polygon.
 * @remark errno should be set on failure.
 * @param[out] outPosition The memory to place the position into.
 * @param polygon The polygon to get the point for.
 * @param points The point data to index into.
 * @param index The index of the point.
 * @return True if outPosition was successfully assigned.
 * @see SimplePolygon.h
 */
typedef bool (*dsPolygonPositionFunction)(dsVector2d* outPosition, const dsSimplePolygon* polygon,
	const void* points, uint32_t index);

/**
 * @brief Enum for the fill rule when triangulating a complex polygon.
 *
 * When overlaps occur, this is calculated by adding counter-clockwise sections and subtracing
 * clockwise sections.
 */
typedef enum dsPolygonFillRule
{
	dsPolygonFillRule_EvenOdd, ///< Fill when the sum of winding orders is odd, hole when even.
	dsPolygonFillRule_NonZero  ///< Fill when the sum of winding orders isn't equal to 0.
} dsPolygonFillRule;

/**
 * @brief Struct for a polygon loop used as part of a complex polygon.
 * @see ComplexPolygon.h
 */
typedef struct dsPolygonLoop
{
	/**
	 * @brief The list of points.
	 *
	 * This must be an array of dsVector2f, dsVector2d, or dsVector2i based on the element set on
	 * the dsComplexPolygon it's used with. The last point will automatically be connected to the
	 * first point to close the loop.
	 */
	const void* points;

	/**
	 * @brief The number of points in the loop.
	 */
	uint32_t pointCount;
} dsPolygonLoop;

/**
 * @brief Structure to define a complex polygon for simplification.
 * @see ComplexPolygon.h
 */
typedef struct dsComplexPolygon dsComplexPolygon;

/**
 * @brief Function for getting the position for a point.
 * @remark errno should be set on failure.
 * @param[out[ outPosition The position for the point. This should be cast to the appropriate
 *     dsVector2* type based on the element from the polygon.
 * @param polygon The complex polygon requesting the points.
 * @param points The points for the current loop.
 * @param index The index of the point.
 * @see ComplexPolygon.h
 */
typedef bool (*dsComplexPolygonPointFunction)(void* outPosition, const dsComplexPolygon* polygon,
	const void* points, uint32_t index);

#ifdef __cplusplus
}
#endif
