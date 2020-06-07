/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/VectorDrawScene/VectorSceneMaterialSet.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>

const char* const dsVectorSceneMaterialSet_typeName = "VectorMaterialSet";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsVectorSceneMaterialSet_type(void)
{
	return &resourceType;
}

dsCustomSceneResource* dsVectorSceneMaterialSet_create(dsAllocator* allocator,
	dsVectorMaterialSet* materialSet)
{
	if (!allocator || !materialSet)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = materialSet;
	customResource->destroyFunc =
		(dsDestroyCustomSceneResourceFunction)&dsVectorMaterialSet_destroy;
	return customResource;
}
