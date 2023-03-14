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
 * @brief Functions for creating and manipulating scene animation tree nodes.
 * @see dsSceneAnimationTreeNode
 */

/**
 * @brief The type name for a scene animation node.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationTreeNode_typeName;

/**
 * @brief Gets the type of a transform node.
 * @return The type of a transform node.
 */
DS_SCENEANIMATION_EXPORT const dsSceneNodeType* dsSceneAnimationTreeNode_type(void);

/**
 * @brief Creates a scene animation tree node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node. This must support freeing memory.
 * @param animationTree The scene animation tree that will be animated.
 * @param nodeMapCache The cache to maintain animation node maps.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The scene animation node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationTreeNode* dsSceneAnimationTreeNode_create(
	dsAllocator* allocator, dsAnimationTree* animationTree, dsAnimationNodeMapCache* nodeMapCache,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Gets the animation tree for a scene tree node.
 *
 * This will check starting with the scene tree node passed in, then go up for each successive
 * parent until a dsSceneAnimationTreeNode is found. This assumes that the animation tree was
 * created from a dsSceneAnimationList.
 *
 * @param treeNode The scene tree node to get the animation for.
 * @return The animation tree or NULL if there isn't one present.
 */
DS_SCENEANIMATION_EXPORT dsAnimationTree* dsSceneAnimationTreeNode_getAnimationTreeForInstance(
	const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif
