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
#include <DeepSea/SceneAnimation/Export.h>
#include <DeepSea/SceneAnimation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene animation ragdoll nodes.
 * @see dsSceneAnimationRagdollNode
 */

/**
 * @brief The type name for an animation ragdoll node.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationRagdollNode_typeName;

/**
 * @brief Gets the type of an animation ragdoll node.
 * @return The type of an animation ragdoll node.
 */
DS_SCENEANIMATION_EXPORT const dsSceneNodeType* dsSceneAnimationRagdollNode_type(void);

/**
 * @brief Creates an animation ragdoll node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param ragdollType The type of ragdoll this will be a part of.
 * @param animationComponents Bitmask for the components to use, taken as 1 << dsAnimationComponent.
 * @param relativeAncestor The number of nodes to go up for the relative transform.
 * @param animationNodeName The name of the animation node to get the ragdoll from. This will be
 *     copied.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The animation ragdoll node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationRagdollNode* dsSceneAnimationRagdollNode_create(
	dsAllocator* allocator, dsSceneAnimationRagdollType ragdollType, uint32_t animationComponents,
	unsigned int relativeAncestor, const char* animationNodeName, const char* const* itemLists,
	uint32_t itemListCount);

#ifdef __cplusplus
}
#endif
