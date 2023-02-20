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
 * @brief Functions for creating and manipulating scene animation nodes.
 * @see dsSceneAnimationNode
 */

/**
 * @brief The type name for a scene animation node.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationNode_typeName;

/**
 * @brief Gets the type of a transform node.
 * @return The type of a transform node.
 */
DS_SCENEANIMATION_EXPORT const dsSceneNodeType* dsSceneAnimationNode_type(void);

/**
 * @brief Creates a scene animation node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node. This must support freeing memory.
 * @param animationTree The scene animation tree that will be animated.
 * @return The scene animation node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	dsSceneAnimationTree* animationTree);

/**
 * @brief Gets the animation for a tree node.
 * @remark This assumes that the animation was created from a dsSceneAnimationList.
 * @param treeNode The tree node to get the animation for.
 * @return The animation or NULL if there isn't one present.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimation* dsSceneAnimationNode_getAnimationForInstance(
	const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif
