/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/SceneVectorDraw/Export.h>
#include <DeepSea/SceneVectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate discard bounds nodes.
 * @see dsSceneDiscardBoundsNode
 */

/**
 * @brief The type name for a discard bounds node.
 */
DS_SCENEVECTORDRAW_EXPORT extern const char* const dsSceneDiscardBoundsNode_typeName;

/**
 * @brief Gets the type of a discard bounds node.
 * @return The type of a discard bounds node.
 */
DS_SCENEVECTORDRAW_EXPORT const dsSceneNodeType* dsSceneDiscardBoundsNode_type(void);

/**
 * @brief Creates a discard bounds node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the discard bounds node with.
 * @param bounds The bounds outside of which to discard shader fragments. This may be NULL for an
 *     initial invalid bounds, which will perform no discarding.
 * @return The discard bounds node or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneDiscardBoundsNode* dsSceneDiscardBoundsNode_create(
	dsAllocator* allocator, const dsAlignedBox2f* bounds);

/**
 * @brief Gets the discard bounds for a tree node.
 * @param[outTransform] The transform for the discard bounds.
 * @param treeNode The tree node to get the discard bounds for.
 * @return The discard bounds or NULL if there is no valid bounds.
 */
DS_SCENEVECTORDRAW_EXPORT const dsAlignedBox2f*
dsSceneDiscardBoundsNode_getDiscardBoundsForInstance(
	dsMatrix44f* outTransform, const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif
