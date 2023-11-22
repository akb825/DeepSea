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
	shape->refCount = 1;
	shape->destroyFunc = destroyFunc;
	return true;
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
	if (shape && DS_ATOMIC_FETCH_ADD32(&shape->refCount, -1) == 1)
		shape->destroyFunc(shape);
}
