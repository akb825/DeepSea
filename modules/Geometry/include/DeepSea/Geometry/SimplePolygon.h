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
 * @brief Functions for creating and triangulating simple polygons.
 *
 * A simple polygon is a closed loop of connected points. Only a single loop is allowed, and it may
 * not intersect with itself. Simple polygons are allowed to be concave.
 *
 * @remark Polygon triangulation is a 2D operation. Triangulation in 3D space can be done by
 * resolving the 3D positions to 2D positions, either be dropping a coordinate (e.g. dropping Z) or
 * performing a transform. (e.g. projecting to a plane or other surface)
 *
 * @see SimplePolygon
 */

/**
 * @brief Creates a simple polygon.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the polygon with. This must support freeing memory.
 * @param userData User data associated with the polygon.
 * @return The created polygon, or NULL if it couldn't be created.
 */
DS_GEOMETRY_EXPORT dsSimplePolygon* dsSimplePolygon_create(dsAllocator* allocator, void* userData);

/**
 * @brief Gets the user data for the simple polygon.
 * @param polygon The simple polygon.
 * @return The user data.
 */
DS_GEOMETRY_EXPORT void* dsSimplePolygon_getUserData(const dsSimplePolygon* polygon);

/**
 * @brief Sets the user data for the simple polygon.
 * @param polygon The simple polygon.
 * @param userData The user data.
 */
DS_GEOMETRY_EXPORT void dsSimplePolygon_setUserData(dsSimplePolygon* polygon, void* userData);

/**
 * @brief Triangulates a simple polygon.
 * @remark errno will be set on failure.
 * @param[out] outIndexCount The number of indices that were produced.
 * @param polygon The polygon to process the triangulation.
 * @param points The points to triangulate. The last point will automatically be connected to the
 *     first point to close the loop. Duplicate points in a series should be avoided for input.
 * @param pointCount The number of points.
 * @param pointPositionFunc Function to get the position of each point. This may be NULL if points
 *     is an array of dsVector2d.
 * @param winding The winding order to triangulate with.
 * @return An array of indices produced, the size of which is populated in outIndexCount, or NULL
 *     if the polygon couldn't be triangulated. This will be valid until the polygon is
 *     re-triangulated or destroyed.
 */
DS_GEOMETRY_EXPORT const uint32_t* dsSimplePolygon_triangulate(uint32_t* outIndexCount,
	dsSimplePolygon* polygon, const void* points, uint32_t pointCount,
	dsPolygonPositionFunction pointPositionFunc, dsTriangulateWinding winding);

/**
 * @brief Destroyes a simple polygon.
 * @param polygon The polygon to destroy.
 */
DS_GEOMETRY_EXPORT void dsSimplePolygon_destroy(dsSimplePolygon* polygon);

#ifdef __cplusplus
}
#endif
