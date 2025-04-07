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
 * @brief Functions for creating and manipulating scene handoff nodes.
 * @see dsSceneHandoffNode
 */

/**
 * @brief The type name for a scene handoff node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneHandoffNode_typeName;

/**
 * @brief Gets the type of a scene handoff node.
 * @return The type of a scene handoff node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneHandoffNode_type(void);

/**
 * @brief Creates a scene handoff node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param transitionTime The time in seconds to interpolate from the original to latest the
 *     transform.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The scene handoff node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneHandoffNode* dsSceneHandoffNode_create(dsAllocator* allocator,
	float transitionTime, const char* const* itemLists, uint32_t itemListCount);

#ifdef __cplusplus
}
#endif
