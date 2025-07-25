/*
 * Copyright 2019-2025 Aaron Barany
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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating cull lists.
 *
 * This will perform culling on nodes that subclass from dsSceneCullNode.
 *
 * The item data is treated as a bool value for whether or not the item is out of view. In other
 * words, check if the void* value is zero if it's in view or non-zero for out of view.
 */

/**
 * @brief The view cull list type name.
 */
DS_SCENE_EXPORT extern const char* const dsViewCullList_typeName;

/**
 * @brief Gets the type of a cull list.
 * @return The type of a cull list.
 */
DS_SCENE_EXPORT const dsSceneItemListType* dsViewCullList_type(void);

/**
 * @brief Creates a view cull list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the cull list. This will be copied.
 * @return The cull list or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsViewCullList_create(dsAllocator* allocator, const char* name);

#ifdef __cplusplus
}
#endif
