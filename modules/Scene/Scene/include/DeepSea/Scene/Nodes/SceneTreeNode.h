/*
 * Copyright 2022 Aaron Barany
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
 * @brief Functions for querying and traversing dsSceneTreeNode instances.
 * @see dsSceneTreeNode
 */

/**
 * @brief Gets the scene node associated with a tree node.
 * @remark errno will be set on failure.
 * @param node The scene tree node.
 * @return The scene node associated with the tree node or NULL if node is NULL.
 */
DS_SCENE_EXPORT const dsSceneNode* dsSceneTreeNode_getNode(const dsSceneTreeNode* node);

/**
 * @brief Gets the parent to a tree node.
 * @param node The scene tree node.
 * @return The parent tree node or NULL if node is NULL or is at the root of the scene.
 */
DS_SCENE_EXPORT const dsSceneTreeNode* dsSceneTreeNode_getParent(const dsSceneTreeNode* node);

/**
 * @brief Gets the number of children of a tree node.
 * @param node The scene tree node.
 * @return The number of child nodes.
 */
DS_SCENE_EXPORT uint32_t dsSceneTreeNode_getChildCount(const dsSceneTreeNode* node);

/**
 * @brief Gets a child node of a tree node.
 * @remark errno will be set on failure.
 * @param node The scene tree node.
 * @param index The index of the child.
 * @return The child node or NULL if node is NULL or index is out of range.
 */
DS_SCENE_EXPORT const dsSceneTreeNode* dsSceneTreeNode_getChild(const dsSceneTreeNode* node,
	uint32_t index);

/**
 * @brief Gets the number item lists of a tree node.
 *
 * This is the same as the number of item lists of the scene node.
 *
 * @param node The scene tree node.
 * @return The number of item lists.
 */
DS_SCENE_EXPORT uint32_t dsSceneTreeNode_getItemListCount(const dsSceneTreeNode* node);

/**
 * @brief Gets the item list for a tree node.
 * @param node The scene tree node.
 * @param index The index of the item list.
 * @return The item list or NULL if node is null, index is out of range, or the item list set on the
 *     scene node was invalid for the node.
 */
DS_SCENE_EXPORT const dsSceneItemList* dsSceneTreeNode_getItemList(const dsSceneTreeNode* node,
	uint32_t index);

/**
 * @brief Gets the transform for a tree node.
 * @remark errno will be set on failure.
 * @param node The scene tree node.
 * @return The transform or NULL if node is NULL.
 */
DS_SCENE_EXPORT const dsMatrix44fSIMD* dsSceneTreeNode_getTransform(const dsSceneTreeNode* node);

/**
 * @brief Gets the item data associated with a tree node.
 * @remark errno will be set on failure.
 * @param node The scene tree node.
 * @return The item data or NULL if node is NULL.
 */
DS_SCENE_EXPORT const dsSceneNodeItemData* dsSceneTreeNode_getItemData(const dsSceneTreeNode* node);

#ifdef __cplusplus
}
#endif
