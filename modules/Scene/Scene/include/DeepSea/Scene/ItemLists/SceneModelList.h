/*
 * Copyright 2019-2021 Aaron Barany
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
 * @brief Functions for creating and manipulating model lists.
 * @see dsSceneModelList
 */

/**
 * @brief The scene model list type name.
 */
DS_SCENE_EXPORT extern const char* const dsSceneModelList_typeName;

/**
 * @brief Creates a scene model list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the model list. This will be copied.
 * @param instanceData The list of instance datas to use. The array will be copied, and this will
 *     take ownership of each instance data. The instances will be destroyed if an error occurrs.
 * @param instanceDataCount The number of instance datas.
 * @param sortType How to sort the geometry.
 * @param renderStates The render states to use, or NULL if no special render states are needed.
 * @param cullList The name of the cull item list to determine what models are in view, or NULL if
 *     no culling is used. It's assumed that the void* for the item data directly relates to a zero
 *     if in view or non-zero if out of view.
 * @return The model list or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelList* dsSceneModelList_create(dsAllocator* allocator, const char* name,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount, dsModelSortType sortType,
	const dsDynamicRenderStates* renderStates, const char* cullList);

/**
 * @brief Gets the sort type for a model list.
 * @param modelList The model list.
 * @return The sort type.
 */
DS_SCENE_EXPORT dsModelSortType dsSceneModelList_getSortType(const dsSceneModelList* modelList);

/**
 * @brief Sets the sort type for a model list.
 * @param modelList The model list.
 * @param sortType The sort type.
 */
DS_SCENE_EXPORT void dsSceneModelList_setSortType(dsSceneModelList* modelList,
	dsModelSortType sortType);

/**
 * @brief Gets the render states for a model list.
 * @param modelList The model list.
 * @return The render states or NULL if no special render states are used.
 */
DS_SCENE_EXPORT const dsDynamicRenderStates* dsSceneModelList_getRenderStates(
	const dsSceneModelList* modelList);

/**
 * @brief Sets the render states for a model list.
 * @param modelList The model list.
 * @param renderStates The render states or NULL if no special render states are needed.
 */
DS_SCENE_EXPORT void dsSceneModelList_setRenderStates(dsSceneModelList* modelList,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Destroys a model list.
 * @param modelList The model list.
 */
DS_SCENE_EXPORT void dsSceneModelList_destroy(dsSceneModelList* modelList);

#ifdef __cplusplus
}
#endif
