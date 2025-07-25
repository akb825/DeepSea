/*
 * Copyright 2025 Aaron Barany
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
 * @brief Functions for creating and manipulating scene handoff lists.
 *
 * This will manage changes in transforms with a dsSceneHandoffNode when re-parenting from one
 * sub-graph to another within the scene.
 */

/**
 * @brief The scene handoff list type name.
 */
DS_SCENE_EXPORT extern const char* const dsSceneHandoffList_typeName;

/**
 * @brief Gets the type of a scene handoff list.
 * @return The type of a scene handoff list.
 */
DS_SCENE_EXPORT const dsSceneItemListType* dsSceneHandoffList_type(void);

/**
 * @brief Creates a scene handoff cull list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the handoff list. This will be copied.
 * @return The cull list or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsSceneHandoffList_create(
	dsAllocator* allocator, const char* name);

#ifdef __cplusplus
}
#endif
