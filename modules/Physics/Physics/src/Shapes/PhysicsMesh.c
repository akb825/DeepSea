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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox3.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/Types.h>

static bool dsPhysicsMesh_getMaterial(dsPhysicsShapePartMaterial* outMaterial,
	const dsPhysicsShape* shape, uint32_t faceIndex)
{
	DS_ASSERT(outMaterial);
	DS_ASSERT(shape);

	const dsPhysicsMesh* mesh = (const dsPhysicsMesh*)shape;
	if (mesh->materialCount == 0)
	{
		errno = EPERM;
		return false;
	}

	if (faceIndex >= mesh->triangleCount)
	{
		errno = EINDEX;
		return false;
	}

	uint32_t materialIndex;
	switch (mesh->materialIndexSize)
	{
		case sizeof(uint16_t):
			materialIndex = ((const uint16_t*)mesh->materialIndices)[faceIndex];
			break;
		case sizeof(uint32_t):
			materialIndex = ((const uint32_t*)mesh->materialIndices)[faceIndex];
			break;
		default:
			errno = EINVAL;
			return false;
	}

	DS_ASSERT(materialIndex < mesh->materialCount);
	*outMaterial = mesh->materials[materialIndex];
	return true;
}

static dsPhysicsShapeType meshType = {.staticBodiesOnly = true, .uniformScaleOnly = false,
	.getMassPropertiesFunc = NULL, .getMaterialFunc = &dsPhysicsMesh_getMaterial};
const dsPhysicsShapeType* dsPhysicsMesh_type(void)
{
	return &meshType;
}

dsPhysicsMesh* dsPhysicsMesh_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, const void* indices,
	uint32_t triangleCount, size_t indexSize, const dsPhysicsShapePartMaterial* triangleMaterials,
	uint32_t triangleMaterialCount, const void* triangleMaterialIndices,
	size_t triangleMaterialIndexSize, const char* cacheName)
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
		vertexStride, indices, triangleCount, indexSize, triangleMaterials, triangleMaterialCount,
		triangleMaterialIndices, triangleMaterialIndexSize, cacheName);
	DS_PROFILE_FUNC_RETURN(mesh);
}

void dsPhysicsMesh_initialize(dsPhysicsMesh* mesh, dsPhysicsEngine* engine, dsAllocator* allocator,
	void* impl, const void* vertices, uint32_t vertexCount, size_t vertexStride,
	uint32_t triangleCount, const void* triangleMaterialIndices,
	size_t triangleMaterialIndexSize, const dsPhysicsShapePartMaterial* triangleMaterials,
	uint32_t triangleMaterialCount)
{
	DS_ASSERT(mesh);
	DS_ASSERT(engine);
	DS_ASSERT(vertices);
	DS_ASSERT(vertexCount > 0);
	DS_ASSERT(vertexStride >= sizeof(dsVector3f));
	DS_ASSERT(triangleCount > 0);
	DS_ASSERT(triangleMaterialCount == 0 || (triangleMaterialIndices && triangleMaterials));

	const uint8_t* vertexBytes = (const uint8_t*)vertices;
	dsAlignedBox3f bounds;
	dsAlignedBox3f_makeInvalid(&bounds);
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const dsVector3f* point = (const dsVector3f*)(vertexBytes + i*vertexStride);
		dsAlignedBox3_addPoint(bounds, *point);
	}

	DS_VERIFY(dsPhysicsShape_initialize((dsPhysicsShape*)mesh, engine, allocator,
		dsPhysicsMesh_type(), &bounds, impl,
		(dsDestroyPhysicsShapeFunction)engine->destroyMeshFunc));

	mesh->triangleCount = triangleCount;
	mesh->materialCount = triangleMaterialCount;
	mesh->materialIndexSize = (uint32_t)triangleMaterialIndexSize;
	mesh->materialIndices = triangleMaterialIndices;
	mesh->materials = triangleMaterials;
}
