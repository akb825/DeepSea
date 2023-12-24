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

#include <DeepSea/Physics/Shapes/PhysicsConvexHull.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Physics/Types.h>

static dsPhysicsShapeType convexHullType = {.staticBodiesOnly = false, .uniformScaleOnly = false};
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

	if (!shape->engine->debug)
	{
		errno = EPERM;
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

	if (!shape->engine->debug)
	{
		errno = EPERM;
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

bool dsPhysicsConvexHull_destroy(dsPhysicsConvexHull* convexHull)
{
	if (!convexHull)
		return true;

	dsPhysicsEngine* engine = ((dsPhysicsShape*)convexHull)->engine;
	if (!engine || !engine->destroyConvexHullFunc)
	{
		errno = EINVAL;
		return false;
	}

	return engine->destroyConvexHullFunc(engine, convexHull);
}
