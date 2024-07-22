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

#include <DeepSea/ScenePhysics/SceneRigidBodyTemplate.h>

#include <DeepSea/Physics/RigidBodyTemplate.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>

const char* const dsSceneRigidBodyTemplate_typeName = "RigidBodyTemplate";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneRigidBodyTemplate_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsSceneRigidBodyTemplate_create(dsAllocator* allocator, dsRigidBodyTemplate* rigidBodyTemplate)
{
	if (!allocator || !rigidBodyTemplate)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = rigidBodyTemplate;
	customResource->destroyFunc = (dsDestroyCustomSceneResourceFunction)&dsRigidBodyTemplate_destroy;
	return customResource;
}
