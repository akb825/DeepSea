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

#include <DeepSea/Physics/Shapes/PhysicsShape.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

bool dsPhysicsShape_initialize(dsPhysicsShape* shape, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsShapeType* type, void* impl,
	dsDestroyPhysicsShapeFunction destroyFunc)
{
	if (!shape || !engine || !allocator || !type || !impl || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	shape->engine = engine;
	shape->allocator = dsAllocator_keepPointer(allocator);
	shape->type = type;
	shape->impl = impl;
	shape->debugData = NULL;
	shape->destroyDebugDataFunc = NULL;
	shape->refCount = 1;
	shape->destroyFunc = destroyFunc;
	return true;
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
	shape->destroyFunc(shape);
}
