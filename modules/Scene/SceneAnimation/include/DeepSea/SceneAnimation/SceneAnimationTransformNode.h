/*
 * Copyright 2023 Aaron Barany
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
#include <DeepSea/SceneAnimation/Export.h>
#include <DeepSea/SceneAnimation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene animation transform nodes.
 * @see dsSceneAnimationTransformNode
 */

/**
 * @brief The type name for an animation transform node.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationTransformNode_typeName;

/**
 * @brief Gets the type of an animation transform node.
 * @return The type of an animation transform node.
 */
DS_SCENEANIMATION_EXPORT const dsSceneNodeType* dsSceneAnimationTransformNode_type(void);

/**
 * @brief Creates an animation transform node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param animationNodeName The name of the animation node to get the transform from. This will be
 *     copied.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The animation transform node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationTransformNode* dsSceneAnimationTransformNode_create(
	dsAllocator* allocator, const char* animationNodeName, const char* const* itemLists,
	uint32_t itemListCount);

#ifdef __cplusplus
}
#endif
