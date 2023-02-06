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

#include <DeepSea/SceneAnimation/SceneAnimation.h>

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>

static bool dsSceneAnimation_destroy(void* animation)
{
	dsAnimation_destroy((dsAnimation*)animation);
	return true;
}

const char* const dsSceneAnimation_typeName = "Animation";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneAnimation_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsSceneAnimation_create(dsAllocator* allocator,
	dsAnimation* animation)
{
	if (!allocator || !animation)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = animation;
	customResource->destroyFunc = &dsSceneAnimation_destroy;
	return customResource;
}
