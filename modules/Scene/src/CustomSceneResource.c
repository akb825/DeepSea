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

#include <DeepSea/Scene/CustomSceneResource.h>

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

dsCustomSceneResource* dsCustomSceneResource_create(dsAllocator* allocator,
	const dsCustomSceneResourceType* type, void* resource,
	dsDestroyCustomSceneResourceFunction destroyFunc)
{
	if (!allocator || !type || !resource)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = type;
	customResource->resource = resource;
	customResource->destroyFunc = destroyFunc;
	return customResource;
}

dsCustomSceneResource* dsCustomSceneResource_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size)
{
	if (!allocator || !loadContext || !scratchData || !type || (!data && size > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	dsLoadCustomSceneResourceItem* foundType = (dsLoadCustomSceneResourceItem*)dsHashTable_find(
		&loadContext->customResourceTypeTable.hashTable, type);
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown custom scene resource type '%s'.", type);
		return NULL;
	}

	void* resource = foundType->loadFunc(loadContext, scratchData, allocator, resourceAllocator,
		foundType->userData, (const uint8_t*)data, size);
	if (!resource)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load custom scene resource '%s': %s.", type,
			dsErrorString(errno));
		return NULL;
	}

	dsCustomSceneResource* customResource = dsCustomSceneResource_create(allocator, foundType->type,
		resource, foundType->destroyResourceFunc);
	if (!customResource)
	{
		if (foundType->destroyResourceFunc)
			DS_VERIFY(foundType->destroyResourceFunc(resource));
		return NULL;
	}

	return customResource;
}

bool dsCustomSceneResource_destroy(dsCustomSceneResource* resource)
{
	if (!resource)
		return true;

	if (resource->destroyFunc)
	{
		if (!resource->destroyFunc(resource->resource))
			return false;
	}

	if (resource->allocator)
		DS_VERIFY(dsAllocator_free(resource->allocator, resource));
	return true;
}
