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

#include <DeepSea/Animation/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneAnimation/Types.h>
#include <DeepSea/SceneAnimation/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for creating and manipulating scene animation trees.
 */

/**
 * @brief The type name for a scene animation tree.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationTree_typeName;

/**
 * @brief Gets the type for the dsAnimationTree custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEANIMATION_EXPORT const dsCustomSceneResourceType* dsSceneAnimationTree_type(void);

/**
 * @brief Creates a scene animation tree.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene animation tree. This must support freeing
 *     memory.
 * @param animationTree The animation tree. This takes ownership of the animation tree and will
 *     destroy it if creation fails.
 * @return The scene animation tree or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationTree* dsSceneAnimationTree_create(dsAllocator* allocator,
	dsAnimationTree* animationTree);

/**
 * @brief Creates a custom resource to wrap a dsSceneAnimationTree.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param tree The animation tree to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsCustomSceneResource* dsSceneAnimationTree_createResource(
	dsAllocator* allocator, dsSceneAnimationTree* tree);

/**
 * @brief Gets the base animation tree associated with a scene animation tree.
 * @remark errno will be set on failure.
 * @param tree The scene animation tree.
 * @return The animation tree or NULL if tree is NULL.
 */
DS_SCENEANIMATION_EXPORT const dsAnimationTree* dsSceneAnimationTree_getAnimationTree(
	const dsSceneAnimationTree* tree);

/**
 * @brief Adds a keyframe animation node map.
 *
 * This will re-use an existing keyframe animation node map if already present.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param tree The scene animation tree.
 * @param animation The animation to add the node map for.
 * @return The node map or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT const dsKeyframeAnimationNodeMap*
	dsSceneAnimationTree_addKeyframeAnimationNodeMap(dsSceneAnimationTree* tree,
	const dsKeyframeAnimation* animation);

/**
 * @brief Removes a keyframe animation node map.
 *
 * This will fully remove the animation node map once all calls to
 * dsSceneAnimationTree_addKeyframeAnimationNodeMap() have been matched with a call to
 * dsSceneAnimationTree_removeKeyframeAnimationNodeMap(), reducing an internal ref count to 0.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param tree The scene animation tree.
 * @param animation The animation to remove the node map for.
 * @return False if the node map couldn't be removed.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationTree_removeKeyframeAnimationNodeMap(
	dsSceneAnimationTree* tree, const dsKeyframeAnimation* animation);

/**
 * @brief Adds a direct animation node map.
 *
 * This will re-use an existing direct animation node map if already present.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param tree The scene animation tree.
 * @param animation The animation to add the node map for.
 * @return The node map or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT const dsDirectAnimationNodeMap*
	dsSceneAnimationTree_addDirectAnimationNodeMap(dsSceneAnimationTree* tree,
	const dsDirectAnimation* animation);

/**
 * @brief Removes a direct animation node map.
 *
 * This will fully remove the animation node map once all calls to
 * dsSceneAnimationTree_addDirectAnimationNodeMap() have been matched with a call to
 * dsSceneAnimationTree_removeDirectAnimationNodeMap(), reducing an internal ref count to 0.
 *
 * @remark This function is thread-safe.
 * @remark errno will be set on failure.
 * @param tree The scene animation tree.
 * @param animation The animation to remove the node map for.
 * @return False if the node map couldn't be removed.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationTree_removeDirectAnimationNodeMap(
	dsSceneAnimationTree* tree, const dsDirectAnimation* animation);

/**
 * @brief Destroys a scene animation tree.
 * @param tree The scene animation tree to destroy.
 */
DS_SCENEANIMATION_EXPORT void dsSceneAnimationTree_destroy(dsSceneAnimationTree* tree);

#ifdef __cplusplus
}
#endif
