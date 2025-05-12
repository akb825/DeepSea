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
#include <DeepSea/Scene/Nodes/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating shift nodes.
 * @see dsSceneShiftNode
 */

/**
 * @brief The type name for a shift node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneShiftNode_typeName;

/**
 * @brief Gets the type of a shift node.
 * @return The type of a shift node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneShiftNode_type(void);

/**
 * @brief Creates a shift node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node. This must support freeing memory.
 * @param origin The origin of the node. NULL is treated the same as zero.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The shift node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneShiftNode* dsSceneShiftNode_create(dsAllocator* allocator,
	const dsVector3d* origin, const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Sets the origin of a shift node.
 *
 * This will shift the immediate children of the node to compensate for the change in origin.
 *
 * @remark errno will be set on failure.
 * @param node The shift node.
 * @param origin The new origin for the node.
 * @return False if the origin couldn't be set.
 */
DS_SCENE_EXPORT bool dsSceneShiftNode_setOrigin(dsSceneShiftNode* node, const dsVector3d* origin);

/**
 * @brief Gets the position of a child node including any parent shift node's origin.
 * @remark errno will be set on failure.
 * @param[out] outPosition The position of the node.
 * @param node The node to get the position of.
 * @return False if the position couldn't be queried.
 */
DS_SCENE_EXPORT bool dsSceneShiftNode_getChildPosition(
	dsVector3d* outPosition, dsSceneTreeNode* node);

/**
 * @brief Gets the transform of a child node including any parent shift node's origin.
 * @remark errno will be set on failure.
 * @param[out] outTransform The transform of the node.
 * @param node The node to get the position of.
 * @return False if the transform couldn't be queried.
 */
DS_SCENE_EXPORT bool dsSceneShiftNode_getChildTransform(
	dsMatrix44d* outTransform, dsSceneTreeNode* node);

#ifdef __cplusplus
}
#endif
