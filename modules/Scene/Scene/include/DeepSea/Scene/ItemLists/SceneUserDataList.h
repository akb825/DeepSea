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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Scene/ItemLists/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene user data lists.
 */

/**
 * @brief The scene user data list type name.
 */
DS_SCENE_EXPORT extern const char* const dsSceneUserDataList_typeName;

/**
 * @brief Gets the type of a scene user data list.
 * @return The type of a scene user data list.
 */
DS_SCENE_EXPORT dsSceneItemListType dsSceneUserDataList_type(void);

/**
 * @brief Creates a scene user data list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the scene user data list. This will be copied.
 * @return The scene user data list or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsSceneUserDataList_create(dsAllocator* allocator,
	const char* name);

#ifdef __cplusplus
}
#endif
