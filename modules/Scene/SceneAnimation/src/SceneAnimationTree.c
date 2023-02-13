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

#include <DeepSea/SceneAnimation/SceneAnimationTree.h>

#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>

const char* const dsSceneAnimationTree_typeName = "AnimationTree";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneAnimationTree_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsSceneAnimationTree_create(dsAllocator* allocator, dsAnimationTree* tree)
{
	if (!allocator || !tree)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = tree;
	customResource->destroyFunc = &dsSceneAnimationTree_destroy;
	return customResource;
}

bool dsSceneAnimationTree_destroy(void* tree)
{
	dsAnimationTree_destroy((dsAnimationTree*)tree);
	return true;
}
