/*
 * Copyright 2023 Aaron Barany
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
#include <DeepSea/Physics/Shapes/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating physics convex hulls.
 * @see dsPhysicsConvexHull
 */

/**
 * @brief Gets the type of a physics convex hull.
 * @return The type of a convex hull.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsConvexHull_type(void);

/**
 * @brief Creates a physics convex hull.
 * @param engine The physics engine to create the convex hull with.
 * @param allocator The allocator to create the convex hull with.
 * @param vertices Pointer to the first vertex. Each vertex is defined as 3 floats.
 * @param vertexCount The number of vertices. At least 3 vertices must be provided.
 * @param vertexStride The stride in bytes between each vertex. This must be at least
 *     3*sizeof(float).
 * @param convexRadius The convex radius used for collision checks. Larger values will improve
 *     performance at the expense of precision by rounding the corners of the shape. This must be
 *     >= 0. Set to DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS for typical shapes in meter space.
 * @param cacheName Unique name of the convex hull. If not NULL, and a cache directory is set on
 *     engine, the pre-computed convex hull will be loaded from the cache if present or saved to the
 *     cache if not.
 * @return The conex hull or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsConvexHull* dsPhysicsConvexHull_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const void* vertices, uint32_t vertexCount, size_t vertexStride,
	float convexRadius, const char* cacheName);

/**
 * @brief Gets a vertex in a convex hull.
 * @remark errno will be set on failure.
 * @param[out] outVertex The storage to hold the vertex.
 * @param convexHull The convex hull to get the vertex from.
 * @param vertexIndex The index of the vertex.
 * @return False if the vertex couldn't be accessed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConvexHull_getVertex(dsVector3f* outVertex,
	const dsPhysicsConvexHull* convexHull, uint32_t vertexIndex);

/**
 * @brief Gets the number of vertices in a face for a convex hull.
 * @remark errno will be set on failure.
 * @param convexHull The convex hull to get the face vertex count from.
 * @param faceIndex The index of the face.
 * @return The number of vertices in the face or 0 if the face couldn't be queried.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsConvexHull_getFaceVertexCount(
	const dsPhysicsConvexHull* convexHull, uint32_t faceIndex);

/**
 * @brief Gets the information for a face of a convex hull.
 * @remark errno will be set on failure.
 * @param[out] outIndices The storage to hold the indives for the face.
 * @param outIndexCapacity The number of indies outIndices has space for. This must be at least as
 *     large as what is returned from dsPhysicsConvexHull_getFaceVertexCount().
 * @param[out] outNormal The normal of the face. This may be NULL if the normal isn't needed.
 * @param convexHull The convex hull to get the face from.
 * @param faceIndex The index of the face.
 * @return The number of vertices in the face or 0 if the face couldn't be queried.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsConvexHull_getFace(uint32_t* outIndices,
	uint32_t outIndexCapacity, dsVector3f* outNormal, const dsPhysicsConvexHull* convexHull,
	uint32_t faceIndex);

/**
 * @brief Destroys a physics convex hull.
 * @remark errno will be set on failure.
 * @param convexHull The convex hull to destroy.
 * @return False if the convex hull couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConvexHull_destroy(dsPhysicsConvexHull* convexHull);

#ifdef __cplusplus
}
#endif
