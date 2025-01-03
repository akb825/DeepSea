/*
 * Copyright 2019-2024 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>

#include <DeepSea/Core/UniqueNameID.h>

void* dsSceneNodeItemData_findName(const dsSceneNodeItemData* itemData, const char* name)
{
	return dsSceneNodeItemData_findID(itemData, dsUniqueNameID_get(name));
}

void* dsSceneNodeItemData_findID(const dsSceneNodeItemData* itemData, uint32_t nameID)
{
	if (!itemData)
		return NULL;

	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		if (itemData->itemData[i].nameID == nameID)
			return itemData->itemData[i].data;
	}

	return NULL;
}
