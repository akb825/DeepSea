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

#include <DeepSea/Scene/SceneGlobalData.h>

#include "SceneTypes.h"
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

extern "C"
dsSceneGlobalData* dsSceneGlobalData_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size)
{
	auto foundType = reinterpret_cast<dsLoadSceneGlobalDataItem*>(
		dsHashTable_find(&loadContext->nodeTypeTable.hashTable, type));
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown scene global data type '%s'.", type);
		return NULL;
	}

	dsSceneGlobalData* globalData = foundType->loadFunc(loadContext, scratchData, allocator,
		resourceAllocator, foundType->userData, reinterpret_cast<const uint8_t*>(data), size);
	if (!globalData)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load scene global data '%s': %s.", type,
			dsErrorString(errno));
	}
	return globalData;
}


