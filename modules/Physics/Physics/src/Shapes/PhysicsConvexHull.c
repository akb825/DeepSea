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

#include <DeepSea/Physics/Shapes/PhysicsConvexHull.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox3.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Physics/Types.h>
#include <DeepSea/Physics/PhysicsMassProperties.h>

#define MAX_STACK_INDICES 2048

static bool dsPhysicsConvexHull_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	DS_ASSERT(outMassProperties);
	DS_ASSERT(shape);
	DS_ASSERT(shape->type == dsPhysicsConvexHull_type());
	DS_ASSERT(density > 0);

	const dsPhysicsConvexHull* convexHull = (const dsPhysicsConvexHull*)shape;
	if (convexHull->baseMassProperties.mass == 0.0f)
	{
		// Convex hull degenerated into a plane.
		errno = EPERM;
		return false;
	}

	*outMassProperties = convexHull->baseMassProperties;
	return dsPhysicsMassProperties_setMass(outMassProperties, outMassProperties->mass*density);
}

static dsPhysicsShapeType convexHullType = {.staticBodiesOnly = false, .uniformScaleOnly = false,
	.getMassPropertiesFunc = &dsPhysicsConvexHull_getMassProperties, .getMaterialFunc = NULL};
const dsPhysicsShapeType* dsPhysicsConvexHull_type(void)
{
	return &convexHullType;
}

dsPhysicsConvexHull* dsPhysicsConvexHull_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const void* vertices, uint32_t vertexCount, size_t vertexStride, float convexRadius,
	const char* cacheName)
{
	DS_PROFILE_FUNC_START();

	if (!engine || !engine->createConvexHullFunc || !engine->destroyConvexHullFunc || !vertices ||
		vertexCount < 3)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (vertexCount > engine->maxConvexHullVertices)
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Convex hull may not have more than %u vertices.",
			engine->maxConvexHullVertices);
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (vertexStride < sizeof(dsVector3f))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Convex hull vertex stride must be at least 3 floats.");
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (convexRadius < 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Convex hull convex radius must be >= 0.");
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = engine->allocator;

	dsPhysicsConvexHull* convexHull = engine->createConvexHullFunc(engine, allocator, vertices,
		vertexCount, vertexStride, convexRadius, cacheName);
	DS_PROFILE_FUNC_RETURN(convexHull);
}

bool dsPhysicsConvexHull_getVertex(dsVector3f* outVertex, const dsPhysicsConvexHull* convexHull,
	uint32_t vertexIndex)
{
	const dsPhysicsShape* shape = (const dsPhysicsShape*)convexHull;
	if (!outVertex || !convexHull || !shape->engine || !shape->engine->getConvexHullVertexFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (vertexIndex >= convexHull->vertexCount)
	{
		errno = EINDEX;
		return false;
	}

	shape->engine->getConvexHullVertexFunc(outVertex, shape->engine, convexHull, vertexIndex);
	return true;
}

uint32_t dsPhysicsConvexHull_getFaceVertexCount(const dsPhysicsConvexHull* convexHull,
	uint32_t faceIndex)
{
	const dsPhysicsShape* shape = (const dsPhysicsShape*)convexHull;
	if (!convexHull || !shape->engine || !shape->engine->getConvexHullFaceVertexCountFunc)
	{
		errno = EINVAL;
		return 0;
	}

	if (faceIndex >= convexHull->faceCount)
	{
		errno = EINDEX;
		return 0;
	}

	return shape->engine->getConvexHullFaceVertexCountFunc(shape->engine, convexHull, faceIndex);
}

uint32_t dsPhysicsConvexHull_getFace(uint32_t* outIndices, uint32_t outIndexCapacity,
	dsVector3f* outNormal, const dsPhysicsConvexHull* convexHull, uint32_t faceIndex)
{
	const dsPhysicsShape* shape = (const dsPhysicsShape*)convexHull;
	if (!outIndices || !convexHull || !shape->engine || !shape->engine->getConvexHullFaceFunc)
	{
		errno = EINVAL;
		return 0;
	}

	if (faceIndex >= convexHull->faceCount)
	{
		errno = EINDEX;
		return 0;
	}

	return shape->engine->getConvexHullFaceFunc(outIndices, outIndexCapacity, outNormal,
		shape->engine, convexHull, faceIndex);
}

bool dsPhysicsConvexHull_initialize(dsPhysicsConvexHull* convexHull, dsPhysicsEngine* engine,
	dsAllocator* allocator, void* impl, const void* vertices, uint32_t vertexCount,
	size_t vertexStride, uint32_t faceCount, float convexRadius)
{
	DS_ASSERT(convexHull);
	DS_ASSERT(engine);
	DS_ASSERT(vertices);
	DS_ASSERT(vertexCount > 0);
	DS_ASSERT(vertexStride >= sizeof(dsVector3f));
	DS_ASSERT(convexRadius >= 0);

	dsPhysicsShape* shape = (dsPhysicsShape*)convexHull;
	shape->engine = engine;
	shape->allocator = dsAllocator_keepPointer(allocator);
	shape->type = dsPhysicsConvexHull_type();
	shape->impl = impl;
	shape->debugData = NULL;
	shape->destroyDebugDataFunc = NULL;
	shape->refCount = 1;
	shape->destroyFunc = (dsDestroyPhysicsShapeFunction)engine->destroyConvexHullFunc;

	convexHull->vertexCount = vertexCount;
	convexHull->faceCount = faceCount;

	// Bounding box for the shape.
	const uint8_t* vertexBytes = (const uint8_t*)vertices;
	dsAlignedBox3f_makeInvalid(&shape->bounds);
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const dsVector3f* point = (const dsVector3f*)(vertexBytes + i*vertexStride);
		dsAlignedBox3_addPoint(shape->bounds, *point);
	}

	// Get the indices for the shape to compute the base mass properties.
	uint32_t indexCount = 0, maxFaceVertices = 0;
	for (uint32_t i = 0; i < convexHull->faceCount; ++i)
	{
		uint32_t faceVertexCount = engine->getConvexHullFaceVertexCountFunc(engine, convexHull, i);
		DS_ASSERT(faceVertexCount >= 3);
		maxFaceVertices = dsMax(maxFaceVertices, faceVertexCount);
		indexCount += (faceVertexCount - 2)*3;
	}

	uint32_t* indices;
	bool heapIndices = indexCount > MAX_STACK_INDICES;
	if (heapIndices)
	{
		indices = DS_ALLOCATE_OBJECT_ARRAY(engine->allocator, uint32_t, indexCount);
		if (!indices)
			return false;
	}
	else
		indices = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, indexCount);
	uint32_t* faceVertices = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, maxFaceVertices);

	for (uint32_t i = 0, index = 0; i < convexHull->faceCount; ++i)
	{
		uint32_t faceVertexCount = engine->getConvexHullFaceFunc(faceVertices, maxFaceVertices, NULL,
			engine, convexHull, i);
		// Triangulate as a vertex fan.
		DS_ASSERT(faceVertexCount >= 3);
		for (uint32_t j = 2; j < faceVertexCount; ++j)
		{
			indices[index++] = faceVertices[0];
			indices[index++] = faceVertices[j - 1];
			indices[index++] = faceVertices[j];
		}
	}

	// Get the mass properties with a density of one to re-use when getting mass properties later.
	// If it fails, the convex hull degenerates into a plane, clear out the mass properties to throw
	// an error if requested later. (e.g. a static body won't need mass properties)
	if (!dsPhysicsMassProperties_initializeMesh(&convexHull->baseMassProperties, vertices,
			vertexCount, vertexStride, indices, indexCount, sizeof(uint32_t), 1.0f))
	{
		DS_VERIFY(dsPhysicsMassProperties_initializeEmpty(&convexHull->baseMassProperties));
	}

	if (heapIndices)
		DS_VERIFY(dsAllocator_free(engine->allocator, indices));

	return true;
}
