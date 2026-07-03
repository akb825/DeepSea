/*
 * Copyright 2025-2026 Aaron Barany
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
 * @brief Functions for manipulating scene tree nodes.
 * @see dsSceneTreeNode
 */

/**
 * @brief Marks a scene tree node as dirty.
 *
 * This should be called anytime the contents of treeNode->baseStepTransform or
 * treeNode->baseFrameTransform changes. This is intended only be called in very specialized
 * scenarios when the transform is manually manipulated. Due to this, no error checking is done
 * apart from asserts for performance.
 *
 * @param node The node to mark as dirty.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_markDirty(dsSceneTreeNode* node);

/**
 * @brief Gets the current step transform for a scene tree node.
 *
 * This gets the rigid transform for the current step in world space. This is intended to only be
 * used in very specialized circumstances, and as such no error checking is done apart from asserts
 * for performance.
 *
 * @param[out] outTransform The current step transform.
 * @param node The scene tree node to get the transform for.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_getCurrentStepTransform(
	dsRigidTransform3f* outTransform, const dsSceneTreeNode* node);

/**
 * @brief Gets the current step transform for a scene tree node relative to a parent node.
 *
 * This is intended to only be used in very specialized circumstances, and as such no error checking
 * is done apart from asserts for performance.
 *
 * @param[out] outTransform The current step transform.
 * @param node The scene tree node to get the transform for.
 * @param ancestorNode The ancestor node to get the transform relative to.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_getCurrentStepRelativeTransform(
	dsRigidTransform3f* outTransform, const dsSceneTreeNode* node,
	const dsSceneTreeNode* ancestorNode);

/**
 * @brief Gets the previous and current step transform for a scene tree node.
 *
 * This gets the rigid transform for the previous and current step in world space. This is intended
 * to only be used in very specialized circumstances, and as such no error checking is done apart
 * from asserts for performance.
 *
 * @param[out] outPrevTransform The previous step transform.
 * @param[out] outCurTransform The current step transform.
 * @param node The scene tree node to get the transform for.
 * @param stepNumber The current step number, which is used to find the proper previous transform.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_getStepTransforms(dsRigidTransform3f* outPrevTransform,
	dsRigidTransform3f* outCurTransform, const dsSceneTreeNode* node, uint64_t stepNumber);

/**
 * @brief Gets the previous current step transform for a scene tree node relative to a parent node.
 *
 * This is intended to only be used in very specialized circumstances, and as such no error checking
 * is done apart from asserts for performance.
 *
 * @param[out] outPrevTransform The previous step transform.
 * @param[out] outCurTransform The current step transform.
 * @param node The scene tree node to get the transform for.
 * @param ancestorNode The ancestor node to get the transform relative to.
 * @param stepNumber The current step number, which is used to find the proper previous transform.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_getStepRelativeTransforms(dsRigidTransform3f* outPrevTransform,
	dsRigidTransform3f* outCurTransform, const dsSceneTreeNode* node,
	const dsSceneTreeNode* ancestorNode, uint64_t stepNumber);

/**
 * @brief Gets the node ID for a node within an item list.
 * @param node The node to geth the ID for.
 * @param itemList The item list that tracks the node.
 * @return The node ID within the item list or DS_NO_SCENE_NODE if not tracked.
 */
DS_SCENE_EXPORT uint64_t dsSceneTreeNode_getNodeID(
	const dsSceneTreeNode* node, const dsSceneItemList* itemList);

#ifdef __cplusplus
}
#endif
