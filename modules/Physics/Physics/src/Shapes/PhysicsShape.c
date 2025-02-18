/*
 * Copyright 2023-2025 Aaron Barany
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

#include <DeepSea/Physics/Shapes/PhysicsShape.h>

#include "Shapes/PhysicsShapeLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

bool dsPhysicsShape_initialize(dsPhysicsShape* shape, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsShapeType* type, const dsAlignedBox3f* bounds,
	void* impl, dsDestroyPhysicsShapeFunction destroyFunc)
{
	if (!shape || !engine || !allocator || !type || !bounds || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	shape->engine = engine;
	shape->allocator = dsAllocator_keepPointer(allocator);
	shape->type = type;
	shape->bounds = *bounds;
	shape->impl = impl;
	shape->debugData = NULL;
	shape->destroyDebugDataFunc = NULL;
	shape->refCount = 1;
	shape->destroyFunc = destroyFunc;
	return true;
}

dsPhysicsShape* dsPhysicsShape_loadFile(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const char* filePath)
{
	if (!engine || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics shape file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsFileStream_close(&stream);
	if (!buffer)
		return NULL;

	dsPhysicsShape* shape = dsPhysicsShape_loadImpl(engine, allocator, findShapeFunc,
		findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return shape;
}

dsPhysicsShape* dsPhysicsShape_loadResource(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, dsFileResourceType type,
	const char* filePath)
{
	if (!engine || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics shape file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsResourceStream_close(&stream);
	if (!buffer)
		return NULL;

	dsPhysicsShape* shape = dsPhysicsShape_loadImpl(engine, allocator, findShapeFunc,
		findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return shape;
}

dsPhysicsShape* dsPhysicsShape_loadArchive(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const dsFileArchive* archive,
	const char* filePath)
{
	if (!engine || !archive || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics shape file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	dsStream_close(stream);
	if (!buffer)
		return NULL;

	dsPhysicsShape* shape = dsPhysicsShape_loadImpl(engine, allocator, findShapeFunc,
		findShapeUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return shape;
}

dsPhysicsShape* dsPhysicsShape_loadStream(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, dsStream* stream)
{
	if (!engine || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	if (!buffer)
		return NULL;

	dsPhysicsShape* shape = dsPhysicsShape_loadImpl(engine, allocator, findShapeFunc,
		findShapeUserData, buffer, size, NULL);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return shape;
}

dsPhysicsShape* dsPhysicsShape_loadData(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size)
{
	if (!engine || !data || size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsPhysicsShape_loadImpl(
		engine, allocator, findShapeFunc, findShapeUserData, data, size, NULL);
}

bool dsPhysicsShape_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density)
{
	if (!outMassProperties || !shape || !shape->type || density <= 0)
	{
		errno = EINVAL;
		return false;
	}

	if (!shape->type->getMassPropertiesFunc)
	{
		// Mass properties not allowed for this shape.
		errno = EPERM;
		return false;
	}

	return shape->type->getMassPropertiesFunc(outMassProperties, shape, density);
}

bool dsPhysicsShape_getMaterial(dsPhysicsShapePartMaterial* outMaterial,
	const dsPhysicsShape* shape, uint32_t faceIndex)
{
	if (!outMaterial || !shape || !shape->type)
	{
		errno = EINVAL;
		return false;
	}

	if (!shape->type->getMassPropertiesFunc)
	{
		// No material for this shape.
		errno = EPERM;
		return false;
	}

	return shape->type->getMaterialFunc(outMaterial, shape, faceIndex);
}

dsPhysicsShape* dsPhysicsShape_addRef(dsPhysicsShape* shape)
{
	if (!shape)
		return NULL;

	DS_ATOMIC_FETCH_ADD32(&shape->refCount, 1);
	return shape;
}

void dsPhysicsShape_freeRef(dsPhysicsShape* shape)
{
	if (!shape || DS_ATOMIC_FETCH_ADD32(&shape->refCount, -1) != 1)
		return;

	if (shape->destroyDebugDataFunc)
		shape->destroyDebugDataFunc(shape->debugData);
	shape->destroyFunc(shape->engine, shape);
}
