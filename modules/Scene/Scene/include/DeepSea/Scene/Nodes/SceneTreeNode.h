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
 * @brief Functions for manipulating scene tree nodes.
 * @see dsSceneTreeNode
 */

/**
 * @brief Marks a scene tree node as dirty.
 *
 * This should be called anytime the contents of treeNode->baseTransform changes. This is intended
 * only be called in very specialized scenarios when the transform is manually manipulated. Due to
 * this, no error checking is done apart from asserts for performance.
 *
 * @param node The node to mark as dirty.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_markDirty(dsSceneTreeNode* node);

/**
 * @brief Gets the current transform for a scene tree node.
 *
 * This may be used in situations where the transform may not be fully updated, for example inside
 * of the preTransformUpdateFunc of a dsSceneItemList. This is intended to only be used in very
 * specialized circumstances, and as such no error checking is done apart from asserts for
 * performance.
 *
 * @param[out] outTransform The current transform value.
 * @param node The scene tree node to get the transform for.
 */
DS_SCENE_EXPORT void dsSceneTreeNode_getCurrentTransform(dsMatrix44f* outTransform,
	dsSceneTreeNode* node);

/**
 * @brief Gets the node ID for a node within an item list.
 * @param node The node to geth the ID for.
 * @param itemList The item list that tracks the node.
 * @return The node ID within the item list or DS_NO_SCENE_NODE if not tracked.
 */
DS_SCENE_EXPORT uint64_t dsSceneTreeNode_getNodeID(const dsSceneTreeNode* node,
	const dsSceneItemList* itemList);

#ifdef __cplusplus
}
#endif
