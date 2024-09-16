/*
 * Copyright 2024 Aaron Barany
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
 * @brief Functions for creating and manipulating scene user data nodes.
 * @see dsSceneUserDataNode
 */

/**
 * @brief The type name for a user data node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneUserDataNode_typeName;

/**
 * @brief Gets the type of a user data node.
 * @return The type of a user data node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneUserDataNode_type(void);

/**
 * @brief Creates a scene user data node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param userData User data to associate with the node.
 * @param destroyUserDataFunc Function to destroy the node user data.
 * @param createInstanceDataFunc Function to create data for a sub-tree. This must be provided.
 * @param destroyInstanceDataFunc Function to destroy data for a sub-tree.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 */
DS_SCENE_EXPORT dsSceneUserDataNode* dsSceneUserDataNode_create(dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCreateSceneInstanceUserDataFunction createInstanceDataFunc,
	dsDestroyUserDataFunction destroyInstanceDataFunc, const char* const* itemLists,
	uint32_t itemListCount);

/**
 * @brief Gets the instance data for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneUserDataNode is found. This assumes that the instance data was created from a
 * dsSceneUserDataList.
 *
 * @param treeNode The tree node to get the instance data for.
 * @return The instance data or NULL if there isn't one present.
 */
DS_SCENE_EXPORT void* dsSceneUserDataNode_getInstanceData(const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif
