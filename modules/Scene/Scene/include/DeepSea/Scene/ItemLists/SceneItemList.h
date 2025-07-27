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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating scene item lists.
 * @see dsSceneItemLIst
 */

/**
 * @brief Loads a scene item list from a flatbuffer data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the item list.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the item list allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The type name of the item list to load.
 * @param name The name of the item list.
 * @param data The data for the item list. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The item list hierarchy, or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsSceneItemList_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const char* name, const void* data,
	size_t size);

/**
 * @brief Gets the hash for a scene item list.
 * @param itemList The list to get the hash for.
 * @return The hash for the list.
 */
DS_SCENE_EXPORT uint32_t dsSceneItemList_hash(const dsSceneItemList* itemList);

/**
 * @brief Checks whether two item scene lists are equal.
 * @param left The left hand side.
 * @param right The right hand side.
 * @return Whether left and right are equal.
 */
DS_SCENE_EXPORT bool dsSceneItemList_equal(
	const dsSceneItemList* left, const dsSceneItemList* right);

/**
 * @brief Destroys a scene item list.
 * @param list The list to destroy.
 */
DS_SCENE_EXPORT void dsSceneItemList_destroy(dsSceneItemList* list);

#ifdef __cplusplus
}
#endif
