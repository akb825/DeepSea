/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and triangulating simple polygons with holes.
 *
 * This is very simple to dsSimplePolygon, except it additionally allows simple polygons to be used
 * unternally as holes. During triangulation, triangles will only occupy space within the simple
 * polygon that isn't taken by a hole. Holes may not intersect, though vertices are allowed to
 * touch, and each hole must itself be a simple polygon.
 *
 * dsSimpleHoledPolygon will keep memory allocated between triangulations, re-using existing
 * allocations when possible. This means that if you use the same instance to triangulate multiple
 * polygons, it will only allocate memory if more was required than any previous triangulation.
 *
 * @remark Polygon triangulation is a 2D operation. Triangulation in 3D space can be done by
 * resolving the 3D positions to 2D positions, either be dropping a coordinate (e.g. dropping Z) or
 * performing a transform. (e.g. projecting to a plane or other surface)
 *
 * @see dsSimpleHoledPolygon
 * @see SimplePolygon.h
 */

/**
 * @brief Creates a simple holed polygon.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the polygon with. This must support freeing memory.
 * @param userData User data associated with the polygon.
 * @return The created polygon, or NULL if it couldn't be created.
 */
DS_GEOMETRY_EXPORT dsSimpleHoledPolygon* dsSimpleHoledPolygon_create(dsAllocator* allocator,
	void* userData);

/**
 * @brief Gets the user data for the simple holed polygon.
 * @param polygon The simple holed polygon.
 * @return The user data.
 */
DS_GEOMETRY_EXPORT void* dsSimpleHoledPolygon_getUserData(const dsSimpleHoledPolygon* polygon);

/**
 * @brief Sets the user data for the simple holed polygon.
 * @param polygon The simple holed polygon.
 * @param userData The user data.
 */
DS_GEOMETRY_EXPORT void dsSimpleHoledPolygon_setUserData(dsSimpleHoledPolygon* polygon,
	void* userData);

/**
 * @brief Triangulates a simple polygon with holes.
 * @remark errno will be set on failure.
 * @param[out] outIndexCount The number of indices that were produced.
 * @param polygon The polygon to process the triangulation.
 * @param points The points for the loops. These will be indexed by the list of loops that follow.
 * @param pointCount The number of points.
 * @param loops The polygon loops to triangulate. The first loop is the outer polygon, while the
 *     following loops are holes. The last point in each loop will automatically be connected to the
 *     first point to close the loop. Duplicate points in a series are not allowed. Each point may
 *     only be used by a dsSimplePolygonLoop loop.
 * @param loopCount The number of loops.
 * @param pointPositionFunc Function to get the position of each point. This may be NULL if points
 *     is an array of dsVector2d, in which case dsSimplePolygon_getPointVector2d will be used.
 * @param winding The winding order to triangulate with.
 * @return An array of indices produced, the size of which is populated in outIndexCount, or NULL
 *     if the polygon couldn't be triangulated. This will be valid until the polygon is
 *     re-triangulated or destroyed.
 */
DS_GEOMETRY_EXPORT const uint32_t* dsSimpleHoledPolygon_triangulate(uint32_t* outIndexCount,
	dsSimpleHoledPolygon* polygon, const void* points, uint32_t pointCount,
	const dsSimplePolygonLoop* loops, uint32_t loopCount,
	dsPolygonPositionFunction pointPositionFunc, dsTriangulateWinding winding);

/**
 * @brief Destroyes a simple polygon.
 * @param polygon The polygon to destroy.
 */
DS_GEOMETRY_EXPORT void dsSimpleHoledPolygon_destroy(dsSimpleHoledPolygon* polygon);

#ifdef __cplusplus
}
#endif
