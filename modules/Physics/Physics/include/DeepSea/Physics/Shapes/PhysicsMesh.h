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
 * @brief Functions for creating and manipulating physics meshes.
 * @see dsPhysicsMesh
 */

/**
 * @brief Gets the type of a physics mesh.
 * @return The type of a mesh.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsMesh_type(void);

/**
 * @brief Creates a physics mesh.
 * @param engine The physics engine to create the mesh with.
 * @param allocator The allocator to create the mesh with.
 * @param vertices Pointer to the first vertex. Each vertex is defined as 3 floats.
 * @param vertexCount The number of vertices. At least 3 vertices must be provided.
 * @param vertexStride The stride in bytes between each vertex. This must be at least
 *     3*sizeof(float).
 * @param indices The pointer to the first index. Three indices are expected for each triangle.
 * @param triangleCount The number of triangles in the mesh.
 * @param indexSize The size of each index. Must be either sizeof(uint16_t) or sizeof(uint32_t).
 * @param triangleMaterialIndices Material indices for each triangle, which index into the
 *     triangleMaterials array. May be NULL if per-triangle materials aren't used.
 * @param triangleMaterialIndexSize The size of each triangle material index. Must be either
 *     sizeof(uint16_t) or sizeof(uint32_t). If triangleMaterialIndices is unused, the value will
 *     be ignored.
 * @param triangleMaterials The per-triangle materials, or NULL if per-triangle materials aren't
 *     used.
 * @param triangleMaterialCount The number of per-triangle materials.
 * @param cacheName Unique name of the mesh. If not NULL, and a cache directory is set on
 *     engine, the pre-computed mesh will be loaded from the cache if present or saved to the cache
 *     if not.
 * @return The mesh or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsMesh* dsPhysicsMesh_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const void* vertices, uint32_t vertexCount, size_t vertexStride,
	const void* indices, uint32_t triangleCount, size_t indexSize,
	const void* triangleMaterialIndices, size_t triangleMaterialIndexSize,
	const dsPhysicsShapePartMaterial* triangleMaterials, uint32_t triangleMaterialCount,
	const char* cacheName);

/**
 * @brief Destroys a physics mesh.
 * @remark errno will be set on failure.
 * @param mesh The mesh to destroy.
 * @return False if the mesh couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMesh_destroy(dsPhysicsMesh* mesh);

#ifdef __cplusplus
}
#endif
