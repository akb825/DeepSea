/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include "SceneTypes.h"
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

extern "C"
dsSceneItemList* dsSceneItemList_load(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const char* type,
	const char* name, const void* data, size_t size)
{
	auto foundType = reinterpret_cast<dsLoadSceneItemListItem*>(
		dsHashTable_find(&loadContext->itemListTypeTable.hashTable, type));
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown scene item list type '%s'.", type);
		return NULL;
	}

	dsSceneItemList* itemList = foundType->loadFunc(loadContext, scratchData, allocator,
		resourceAllocator, foundType->userData, name, reinterpret_cast<const uint8_t*>(data), size);
	if (!itemList)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load scene item list '%s': %s.", name,
			dsErrorString(errno));
	}
	return itemList;
}

