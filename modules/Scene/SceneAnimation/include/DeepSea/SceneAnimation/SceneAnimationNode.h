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
 * @param nodeMapCache The cache to maintain animation node maps.
 * @param itemLists The list of item list names that will be used to draw and process the node.
 *     These will be copied.
 * @param itemListCount The number of item lists.
 * @return The scene animation node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	dsAnimationNodeMapCache* nodeMapCache, const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Gets the animation for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneAnimationNode is found. This assumes that the animation was created from a
 * dsSceneAnimationList.
 *
 * @param treeNode The tree node to get the animation for.
 * @return The animation or NULL if there isn't one present.
 */
DS_SCENEANIMATION_EXPORT dsAnimation* dsSceneAnimationNode_getAnimationForInstance(
	const dsSceneTreeNode* treeNode);

/**
 * @brief Gets the weight for the skeleton ragdoll of an animation.
 * @param treeNode The tree ndoe the animation is associated with.
 * @return The weight of the skeleton ragdoll.
 */
DS_SCENEANIMATION_EXPORT float dsSceneAnimationNode_getSkeletonRagdollWeight(
	const dsSceneTreeNode* treeNode);

/**
 * @brief Sets the weight for the skeleton ragdoll of an animation.
 * @param treeNode The tree ndoe the animation is associated with.
 * @param weight The new weight for the skeleton ragdoll.
 * @return Whether the weight could be set.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationNode_setSkeletonRagdollWeight(
	const dsSceneTreeNode* treeNode, float weight);

/**
 * @brief Gets the weight for the addition ragdoll of an animation.
 * @param treeNode The tree ndoe the animation is associated with.
 * @return The weight of the addition ragdoll.
 */
DS_SCENEANIMATION_EXPORT float dsSceneAnimationNode_getAdditionRagdollWeight(
	const dsSceneTreeNode* treeNode);

/**
 * @brief Sets the weight for the addition ragdoll of an animation.
 * @param treeNode The tree ndoe the animation is associated with.
 * @param weight The new weight for the addition ragdoll.
 * @return Whether the weight could be set.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationNode_setAdditionRagdollWeight(
	const dsSceneTreeNode* treeNode, float weight);

#ifdef __cplusplus
}
#endif
