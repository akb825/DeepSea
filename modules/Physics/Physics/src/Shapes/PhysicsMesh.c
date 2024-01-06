/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Physics/Shapes/PhysicsMesh.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox3.h>

#include <DeepSea/Physics/Types.h>

static dsPhysicsShapeType meshType = {.staticBodiesOnly = true, .uniformScaleOnly = false,
	.getMassPropertiesFunc = NULL};
const dsPhysicsShapeType* dsPhysicsMesh_type(void)
{
	return &meshType;
}

dsPhysicsMesh* dsPhysicsMesh_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, const void* indices,
	uint32_t triangleCount, size_t indexSize, const void* triangleMaterialIndices,
	size_t triangleMaterialIndexSize, const dsPhysicsShapePartMaterial* triangleMaterials,
	uint32_t triangleMaterialCount, const char* cacheName)
{
	DS_PROFILE_FUNC_START();

	if (!engine || !engine->createMeshFunc || !engine->destroyMeshFunc || !vertices ||
		vertexCount < 3 || !indices || triangleCount == 0 ||
		(!triangleMaterials && triangleMaterialCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (vertexStride < sizeof(dsVector3f))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Mesh vertex stride must be at least 3 floats.");
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (indexSize != sizeof(uint16_t) && indexSize != sizeof(uint32_t))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Mesh index size must be sizeof(uint16_t) or sizeof(uint32_t).");
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (triangleMaterialIndices || triangleMaterials)
	{
		if (!triangleMaterialIndices || !triangleMaterials || triangleMaterialCount == 0)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Mesh must have either both triangle material indices "
				"and triangle materials or neither.");
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (triangleMaterialIndexSize != sizeof(uint16_t) &&
			triangleMaterialIndexSize != sizeof(uint32_t))
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Mesh triangle material index size must be sizeof(uint16_t) or sizeof(uint32_t).");
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	dsPhysicsMesh* mesh = engine->createMeshFunc(engine, allocator, vertices, vertexCount,
		vertexStride, indices, triangleCount, indexSize, triangleMaterialIndices,
		triangleMaterialIndexSize, triangleMaterials, triangleMaterialCount, cacheName);
	if (!mesh)
		DS_PROFILE_FUNC_RETURN(NULL);

	const uint8_t* vertexBytes = (const uint8_t*)vertices;
	dsPhysicsShape* shape = (dsPhysicsShape*)mesh;
	dsAlignedBox3f_makeInvalid(&shape->bounds);
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const dsVector3f* point = (const dsVector3f*)(vertexBytes + i*vertexStride);
		dsAlignedBox3_addPoint(shape->bounds, *point);
	}
	DS_PROFILE_FUNC_RETURN(mesh);
}

bool dsPhysicsMesh_destroy(dsPhysicsMesh* mesh)
{
	if (!mesh)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)mesh)->engine;
	if (!engine || !engine->destroyMeshFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyMeshFunc(engine, mesh);
}
