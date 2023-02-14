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
 * @param animationTree The animation tree that will be animated. A pointer will be kept with the
 *     node and it will be cloned for each instance.
 * @return The scene animation node or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	const dsAnimationTree* animationTree);

/**
 * @brief Gets the base animation tree associated with a scene animation node.
 * @remark errno will be set on failure.
 * @param node The scene animation node.
 * @return The animation tree or NULL if node is NULL.
 */
DS_SCENEANIMATION_EXPORT const dsAnimationTree* dsSceneAnimationNode_getAnimationTree(
	const dsSceneAnimationNode* node);

/**
 * @brief Adds a keyframe animation node map.
 *
 * This will re-use an existing keyframe animation node map if already present.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param node The scene animation node.
 * @param animation The animation to add the node map for.
 * @return The node map or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT const dsKeyframeAnimationNodeMap*
	dsSceneAnimationNode_addKeyframeAnimationNodeMap(dsSceneAnimationNode* node,
	const dsKeyframeAnimation* animation);

/**
 * @brief Removes a keyframe animation node map.
 *
 * This will fully remove the animation node map once all calls to
 * dsSceneAnimationNode_addKeyframeAnimationNodeMap() have been matched with a call to
 * dsSceneAnimationNode_removeKeyframeAnimationNodeMap(), reducing an internal ref count to 0.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param node The scene animation node.
 * @param animation The animation to remove the node map for.
 * @return False if the node map couldn't be removed.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationNode_removeKeyframeAnimationNodeMap(
	dsSceneAnimationNode* node, const dsKeyframeAnimation* animation);

/**
 * @brief Adds a direct animation node map.
 *
 * This will re-use an existing direct animation node map if already present.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param node The scene animation node.
 * @param animation The animation to add the node map for.
 * @return The node map or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT const dsDirectAnimationNodeMap*
	dsSceneAnimationNode_addDirectAnimationNodeMap(dsSceneAnimationNode* node,
	const dsDirectAnimation* animation);

/**
 * @brief Removes a direct animation node map.
 *
 * This will fully remove the animation node map once all calls to
 * dsSceneAnimationNode_addDirectAnimationNodeMap() have been matched with a call to
 * dsSceneAnimationNode_removeDirectAnimationNodeMap(), reducing an internal ref count to 0.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param node The scene animation node.
 * @param animation The animation to remove the node map for.
 * @return False if the node map couldn't be removed.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationNode_removeDirectAnimationNodeMap(
	dsSceneAnimationNode* node, const dsDirectAnimation* animation);

#ifdef __cplusplus
}
#endif
