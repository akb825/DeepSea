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
 * @brief Functions for creating and simplifying complex polygons.
 *
 * A complex polygon is one or more closed loops of connected points. These loops may
 * self-intersect, intersect with each-other, contained completely, or separate. Simplification may
 * follow either the even-odd rule or non-zero filling rule to deal with intersecting and
 * overlapping polygons.
 *
 * Once simplified, a series of polygons with holes will be produced. Each polygon and its
 * corresponding holes may be triangulated with dsSimpleHoledPolygon.
 *
 * dsComplexPolygon will keep memory allocated between triangulations, re-using existing allocations
 * when possible. This means that if you use the same instance to triangulate multiple polygons, it
 * will only allocate memory if more was required than any previous triangulation.
 *
 * @remark Polygon triangulation is a 2D operation. Triangulation in 3D space can be done by
 * resolving the 3D positions to 2D positions, either be dropping a coordinate (e.g. dropping Z) or
 * performing a transform. (e.g. projecting to a plane or other surface)
 *
 * @see dsComplexPolygon
 */

/**
 * @brief Creates a complex polygon.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the polygon with. This must support freeing memory.
 * @param element The type for each vector element.
 * @param userData User data associated with the polygon.
 * @param epsilon The epsilon to use for removing duplicate points.
 * @return The created polygon, or NULL if it couldn't be created.
 */
DS_GEOMETRY_EXPORT dsComplexPolygon* dsComplexPolygon_create(dsAllocator* allocator,
	dsGeometryElement element, void* userData, double epsilon);

/**
 * @brief Gets the type for the vector elements within a complex polygon.
 * @param polygon The complex polygon.
 * @return The type of each bounds element.
 */
DS_GEOMETRY_EXPORT dsGeometryElement dsComplexPolygon_getElement(const dsComplexPolygon* polygon);

/**
 * @brief Gets the user data for the complex polygon.
 * @param polygon The complex polygon.
 * @return The user data.
 */
DS_GEOMETRY_EXPORT void* dsComplexPolygon_getUserData(const dsComplexPolygon* polygon);

/**
 * @brief Sets the user data for the simple polygon.
 * @param polygon The complex polygon.
 * @param userData The user data.
 */
DS_GEOMETRY_EXPORT void dsComplexPolygon_setUserData(dsComplexPolygon* polygon, void* userData);

/**
 * @brief Gets the epsilon used when testing equality between points.
 * @param polygon The complex polygon.
 * @return The equality test epsilon.
 */
DS_GEOMETRY_EXPORT double dsComplexPolygon_getEpsilon(const dsComplexPolygon* polygon);

/**
 * @brief Sets the epsilon used when testing equality between points.
 * @param polygon The complex polygon.
 * @param epsilon The epsilon.
 */
DS_GEOMETRY_EXPORT void dsComplexPolygon_setEpsilon(dsComplexPolygon* polygon, double epsilon);

/**
 * @brief Simplifies a complex polygon into simple polygons.
 *
 * After simplification, you can query the results with dsComplexPolygon_getLoopCount() and
 * dsComplexPolygon_getLoop().
 *
 * @remark The output polygon loops may be very different from the input loops. Additionally, the
 * points that do match may be off by a small epsilon. (for tests with doubles in the range
 * [-10, 10], points were within 1e-14)
 *
 * @remark The simplification is performed by an external library, which will allocate memory
 * outside of the provided allocator. The memory is temporary and will be freed before the function
 * exits.
 *
 * @remark errno will be set on failure.
 * @param polygon The polygon to process the simplification.
 * @param loops The list of loops to simplify.
 * @param loopCount The number of loops present.
 * @param pointFunc The function to get the position for each point. This may be used if a custom
 *     type is used for the points set on each loop. If NULL, it is assumed the points will be the
 *     appropriate dsVector2* type.
 * @param fillRule The fill rule for self-intersections and overlaps.
 * @return False if an error occurred.
 */
DS_GEOMETRY_EXPORT bool dsComplexPolygon_simplify(dsComplexPolygon* polygon,
	const dsComplexPolygonLoop* loops, uint32_t loopCount, dsComplexPolygonPointFunction pointFunc,
	dsPolygonFillRule fillRule);

/**
 * @brief Gets the number of holed polygons from the simplified complex polygon.
 * @param polygon The complex polygon that's been simplified.
 * @return The number of holed polygons.
 */
DS_GEOMETRY_EXPORT uint32_t dsComplexPolygon_getHoledPolygonCount(const dsComplexPolygon* polygon);

/**
 * @brief Gets the number of loops for a holed polygon.
 * @param polygon The complex polygon that's been simplified.
 * @param index The index of the holed polygon.
 * @return The number of loops. The first loop will always be the outer loop, while the following
 *     loops are holes.
 */
DS_GEOMETRY_EXPORT uint32_t dsComplexPolygon_getHoledPolygonLoopCount(
	const dsComplexPolygon* polygon, uint32_t index);

/**
 * @brief Gets the loops for a holed polygon.
 * @remark errno will be set on failure.
 * @param polygon The complex polygon that's been simplified.
 * @param index The index of the holed polygon.
 * @return The polygon loops, or NULL if the parameters were invalid. The first loop will always be
 *     the outer loop, while the following loops are holes.
 */
DS_GEOMETRY_EXPORT const dsSimplePolygonLoop* dsComplexPolygon_getHoledPolygonLoops(
	const dsComplexPolygon* polygon, uint32_t index);

/**
 * @brief Gets the number of points total for a holed polygon.
 * @param polygon The complex polygon that's been simplified.
 * @param index The index of the holed polygon.
 * @return The number of points in the holed polygon.
 */
DS_GEOMETRY_EXPORT uint32_t dsComplexPolygon_getHoledPolygonPointCount(
	const dsComplexPolygon* polygon, uint32_t index);

/**
 * @brief Gets the points for a holed polygon.
 * @remark errno will be set on failure.
 * @param polygon The complex polygon that's been simplified.
 * @param index The index of the holed polygon.
 * @return The points in the holed polygon. This will be an array of dsVector2f, dsVector2d, or
 *     dsVector2i depending on the element type, or NULL if the parameters were invalid.
 */
DS_GEOMETRY_EXPORT const void* dsComplexPolygon_getHoledPolygonPoints(
	const dsComplexPolygon* polygon, uint32_t index);

/**
 * @brief Destroyes a simple polygon.
 * @param polygon The polygon to destroy.
 */
DS_GEOMETRY_EXPORT void dsComplexPolygon_destroy(dsComplexPolygon* polygon);

#ifdef __cplusplus
}
#endif
