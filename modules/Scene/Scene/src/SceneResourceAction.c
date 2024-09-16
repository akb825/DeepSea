/*
 * Copyright 2022-2024 Aaron Barany
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

#include <DeepSea/Scene/SceneResourceAction.h>

#include "SceneLoadContextInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

bool dsSceneResourceAction_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size)
{
	if (!allocator || !loadContext || !scratchData || !type || (!data && size > 0))
	{
		errno = EINVAL;
		return false;
	}

	dsLoadSceneResourceActionItem* foundType = (dsLoadSceneResourceActionItem*)dsHashTable_find(
		&loadContext->resourceActionTypeTable.hashTable, type);
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown scene resource action type '%s'.", type);
		return false;
	}

	if (!foundType->loadFunc(loadContext, scratchData, allocator, resourceAllocator,
			foundType->userData, (const uint8_t*)data, size))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load scene resource action '%s': %s.", type,
			dsErrorString(errno));
		return false;
	}

	return true;
}
