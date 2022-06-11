/*
 * Copyright 2019-2022 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Scene/Nodes/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for requesting item data associated with a specific node.
 * @see dsSceneNodeItemData
 */

/**
 * @brief Finds item data based on the name.
 * @param itemData The item data to search in.
 * @param name The name of the item list to get the instance data for.
 * @return The data or NULL if not found. Note that for some items, the data may be stored directly
 *     in the pointer itself.
 */
DS_SCENE_EXPORT void* dsSceneNodeItemData_findName(const dsSceneNodeItemData* itemData,
	const char* name);

/**
 * @brief Finds item data based on the name ID.
 * @param itemData The item data to search in.
 * @param nameID The ID for the name of the item list to get the instance data for.
 * @return The data or NULL if not found. Note that for some items, the data may be stored directly
 *     in the pointer itself.
 */
DS_SCENE_EXPORT void* dsSceneNodeItemData_findID(const dsSceneNodeItemData* itemData,
	uint32_t nameID);

#ifdef __cplusplus
}
#endif
