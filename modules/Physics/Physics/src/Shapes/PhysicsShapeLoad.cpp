/*
 * Copyright 2024 Aaron Barany
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

#include "Shapes/PhysicsShapeLoad.h"

#include "Flatbuffers/PhysicsFlatbufferHelpers.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Shapes/PhysicsBox.h>
#include <DeepSea/Physics/Shapes/PhysicsCapsule.h>
#include <DeepSea/Physics/Shapes/PhysicsCone.h>
#include <DeepSea/Physics/Shapes/PhysicsConvexHull.h>
#include <DeepSea/Physics/Shapes/PhysicsCylinder.h>
#include <DeepSea/Physics/Shapes/PhysicsMesh.h>
#include <DeepSea/Physics/Shapes/PhysicsSphere.h>
#include <DeepSea/Physics/Types.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/PhysicsBox_generated.h"
#include "Flatbuffers/PhysicsCapsule_generated.h"
#include "Flatbuffers/PhysicsCone_generated.h"
#include "Flatbuffers/PhysicsConvexHull_generated.h"
#include "Flatbuffers/PhysicsCylinder_generated.h"
#include "Flatbuffers/PhysicsMesh_generated.h"
#include "Flatbuffers/PhysicsShape_generated.h"
#include "Flatbuffers/PhysicsSphere_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

namespace
{

// 64 KB
constexpr unsigned int maxStackMaterials = 2730;

dsPhysicsShape* loadBox(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Box& fbBox)
{
	float convexRadius = fbBox.convexRadius();
	if (convexRadius < 0)
		convexRadius = DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS;
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsBox_create(engine, allocator,
		&DeepSeaPhysics::convert(*fbBox.halfExtents()), convexRadius));
}

dsPhysicsShape* loadCapsule(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Capsule& fbCapsule)
{
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsCapsule_create(engine, allocator,
		fbCapsule.halfHeight(), fbCapsule.radius(), DeepSeaPhysics::convert(fbCapsule.axis())));
}

dsPhysicsShape* loadCone(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Cone& fbCone)
{
	float convexRadius = fbCone.convexRadius();
	if (convexRadius < 0)
		convexRadius = DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS;
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsCone_create(engine, allocator,
		fbCone.height(), fbCone.radius(), DeepSeaPhysics::convert(fbCone.axis()), convexRadius));
}

dsPhysicsShape* loadConvexHull(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::ConvexHull& fbConvexHull, const char* name)
{
	auto fbVertices = fbConvexHull.vertices();
	if (fbVertices->size() % 3 != 0)
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid convex hull shape vertices for '%s'.",
				name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid convex hull shape vertices.");
		return nullptr;
	}

	float convexRadius = fbConvexHull.convexRadius();
	if (convexRadius < 0)
		convexRadius = DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS;
	auto fbCacheName = fbConvexHull.cacheName();
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsConvexHull_create(engine, allocator,
		fbVertices->data(), fbVertices->size()/3, sizeof(dsVector3f), convexRadius,
		fbCacheName ? fbCacheName->c_str() : nullptr));
}

dsPhysicsShape* loadCylinder(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Cylinder& fbCylinder)
{
	float convexRadius = fbCylinder.convexRadius();
	if (convexRadius < 0)
		convexRadius = DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS;
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsCylinder_create(engine, allocator,
		fbCylinder.halfHeight(), fbCylinder.radius(), DeepSeaPhysics::convert(fbCylinder.axis()),
		convexRadius));
}

dsPhysicsShape* loadMesh(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Mesh& fbMesh, const char* name)
{
	auto fbVertices = fbMesh.vertices();
	if (fbVertices->size() % 3 != 0)
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid mesh shape vertices for '%s'.", name);
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid mesh shape vertices.");
		return nullptr;
	}

	const void* indices;
	uint32_t indexCount;
	size_t indexSize;
	if (auto fbIndices16 = fbMesh.indices16())
	{
		indices = fbIndices16->data();
		indexCount = fbIndices16->size();
		indexSize = sizeof(uint16_t);
	}
	else if (auto fbIndices32 = fbMesh.indices32())
	{
		indices = fbIndices32->data();
		indexCount = fbIndices32->size();
		indexSize = sizeof(uint32_t);
	}
	else
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Mesh shape has no indices for '%s'.", name);
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Mesh shape has no indices.");
		return nullptr;
	}

	if (indexCount % 3 != 0)
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid mesh shape indices for '%s'.", name);
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid mesh shape indices.");
		return nullptr;
	}

	const void* materialIndices = nullptr;
	uint32_t materialIndexCount = 0;
	size_t materialIndexSize = 0;
	if (auto fbMaterialIndices16 = fbMesh.materialIndices16())
	{
		materialIndices = fbMaterialIndices16->data();
		materialIndexCount = fbMaterialIndices16->size();
		materialIndexSize = sizeof(uint16_t);
	}
	else if (auto fbMaterialIndices32 = fbMesh.materialIndices32())
	{
		materialIndices = fbMaterialIndices32->data();
		materialIndexCount = fbMaterialIndices32->size();
		materialIndexSize = sizeof(uint32_t);
	}

	auto fbMaterials = fbMesh.triangleMaterials();
	if ((fbMaterials == nullptr) != (materialIndices == nullptr))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
				"Mesh shape triangle materials and indices mismatch for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Mesh shape triangle materials and indices mismatch.");
		return nullptr;
	}

	dsPhysicsShapePartMaterial* materials = nullptr;
	uint32_t materialCount = 0;
	bool heapMaterials = false;
	if (fbMaterials)
	{
		if (materialIndexCount != indexCount)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Mesh shape triangle material index mismatch for '%s'.", name);
			}
			else
				DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Mesh shape triangle material index mismatch.");
			return nullptr;
		}

		materialCount = fbMaterials->size();
		if (materialCount > maxStackMaterials)
		{
			materials = DS_ALLOCATE_OBJECT_ARRAY(
				engine->allocator, dsPhysicsShapePartMaterial, materialCount);
			if (!materials)
				return nullptr;
			heapMaterials = true;
		}
		else
			materials = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsPhysicsShapePartMaterial, materialCount);

		for (uint32_t i = 0; i < materialCount; ++i)
		{
			if (!(*fbMaterials)[i])
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid mesh shape material for '%s'.",
						name);
				}
				else
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid mesh shape material.");

				if (heapMaterials)
					dsAllocator_free(engine->allocator, materials);
				return nullptr;
			}

			materials[i] = DeepSeaPhysics::convert(*(*fbMaterials)[i]);
		}
	}

	auto fbCacheName = fbMesh.cacheName();
	dsPhysicsMesh* mesh = dsPhysicsMesh_create(engine, allocator, fbVertices->data(),
		fbVertices->size()/3, sizeof(dsVector3f), indices, indexCount/3, indexSize, materials,
		materialCount, materialIndices, materialIndexSize,
		fbCacheName ? fbCacheName->c_str() : nullptr);

	if (heapMaterials)
		dsAllocator_free(engine->allocator, materials);
	return reinterpret_cast<dsPhysicsShape*>(mesh);
}

dsPhysicsShape* loadSphere(dsPhysicsEngine* engine, dsAllocator* allocator,
	const DeepSeaPhysics::Sphere& fbSphere)
{
	return reinterpret_cast<dsPhysicsShape*>(dsPhysicsSphere_create(engine, allocator,
		fbSphere.radius()));
}

} // namespace

dsPhysicsShape* dsPhysicsShape_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaPhysics::VerifyShapeBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid shape flatbuffer format for '%s'.", name);
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid shape flatbuffer format.");
		return nullptr;
	}

	auto fbShape = DeepSeaPhysics::GetShape(data);
	switch (fbShape->shape_type())
	{
		case DeepSeaPhysics::ShapeUnion::Box:
			return loadBox(engine, allocator, *fbShape->shape_as_Box());
		case DeepSeaPhysics::ShapeUnion::Capsule:
			return loadCapsule(engine, allocator, *fbShape->shape_as_Capsule());
		case DeepSeaPhysics::ShapeUnion::Cone:
			return loadCone(engine, allocator, *fbShape->shape_as_Cone());
		case DeepSeaPhysics::ShapeUnion::ConvexHull:
			return loadConvexHull(engine, allocator, *fbShape->shape_as_ConvexHull(), name);
		case DeepSeaPhysics::ShapeUnion::Cylinder:
			return loadCylinder(engine, allocator, *fbShape->shape_as_Cylinder());
		case DeepSeaPhysics::ShapeUnion::Mesh:
			return loadMesh(engine, allocator, *fbShape->shape_as_Mesh(), name);
		case DeepSeaPhysics::ShapeUnion::Sphere:
			return loadSphere(engine, allocator, *fbShape->shape_as_Sphere());
		case DeepSeaPhysics::ShapeUnion::ShapeRef:
		{
			const char* shapeName = fbShape->shape_as_ShapeRef()->name()->c_str();
			dsPhysicsShape* shape =
				findShapeFunc ? findShapeFunc(engine, findShapeUserData, name) : nullptr;
			if (!shape)
			{
				errno = ENOTFOUND;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Shape '%s' not found for '%s'.", shapeName,
						name);
				}
				else
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Shape '%s' not found.", shapeName);
			}
			return shape;
		}
		default:
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid shape flatbuffer format for '%s'.",
					name);
			}
			else
				DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid shape flatbuffer format.");
			return nullptr;
	}
}
