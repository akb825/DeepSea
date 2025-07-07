/*
 * Copyright 2020-2025 Aaron Barany
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
#include <DeepSea/SceneVectorDraw/Export.h>
#include <DeepSea/SceneVectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene vector item lists.
 * @see dsSceneVectorItemList
 */

/**
 * @brief The scene vector item list type name.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsSceneVectorItemList_typeName;

/**
 * @brief Gets the type of a scene vector item list.
 * @return The type of a scene vector item list.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneItemListType dsSceneVectorItemList_type(void);

/**
 * @brief Creates a scene vector item list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the vector item list. This will be copied.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param instanceData The list of instance datas to use. The array will be copied, and this will
 *     take ownership of each instance data. The instances will be destroyed if an error occurrs.
 * @param instanceDataCount The number of instance datas.
 * @param renderStates The render states to use, or NULL if no special render states are needed.
 * @return The vector item list or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneVectorItemList* dsSceneVectorItemList_create(
	dsAllocator* allocator, const char* name, dsResourceManager* resourceManager,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Gets the render states for a vector item list.
 * @param vectorList The vector item list.
 * @return The render states or NULL if no special render states are used.
 */
DS_SCENEVECTORDRAW_EXPORT const dsDynamicRenderStates* dsSceneVectorItemList_getRenderStates(
	const dsSceneVectorItemList* vectorList);

/**
 * @brief Sets the render states for a vector item list.
 * @param vectorList The vector item list.
 * @param renderStates The render states or NULL if no special render states are needed.
 */
DS_SCENEVECTORDRAW_EXPORT void dsdsSceneVectorItemList_setRenderStates(
	dsSceneVectorItemList* vectorList, const dsDynamicRenderStates* renderStates);

#ifdef __cplusplus
}
#endif
