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

#include <DeepSea/ScenePhysics/ScenePhysicsShape.h>

#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>

static bool destroyResource(void* shape)
{
	dsPhysicsShape_freeRef((dsPhysicsShape*)shape);
	return true;
}

const char* const dsScenePhysicsShape_typeName = "PhysicsShape";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsScenePhysicsShape_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsScenePhysicsShape_create(dsAllocator* allocator, dsPhysicsShape* shape)
{
	if (!allocator || !shape)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = dsPhysicsShape_addRef(shape);
	customResource->destroyFunc = &destroyResource;
	return customResource;
}
