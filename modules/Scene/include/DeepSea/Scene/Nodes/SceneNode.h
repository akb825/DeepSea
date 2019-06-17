/*
 * Copyright 2019 Aaron Barany
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
 * @brief Functions for manipulating scene nodes.
 * @see dsSceneNode
 */

/**
 * @brief Gets the allocated size for draw lists.
 *
 * This can be added to the other node memory to combine with a single allocation. This assumes one
 * allocation for the array, then a separate allocation for each string.
 *
 * @param drawLists The list of draw list names to use.
 * @param drawListCount The number of draw lists.
 */
DS_SCENE_EXPORT size_t dsSceneNode_drawListsAllocSize(const char** drawLists,
	uint32_t drawListCount);

/**
 * @brief Initializes a scene node.
 * @remark errno will be set on failure.
 * @remark The ref count of the node will begin at 1.
 * @param node The node to initialize.
 * @param allocator The allocator the node was created with. This must support freeing memory.
 * @param type The type node.
 * @param drawLists The list of draw list names to use. These should be allocated with the node by
 *     using a dsBufferAllocator so they may all be freed at once.
 * @param drawListCount The number of draw lists.
 * @param destroyFunc The function to destroy the node.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneNode_initialize(dsSceneNode* node, dsAllocator* allocator,
	dsSceneNodeType type, const char** drawLists, uint32_t drawListCount,
	dsDestroySceneNodeFunction destroyFunc);

/**
 * @brief Adds a child to a node.
 * @remark errno will be set on failure.
 * @remark Adding a circular reference can result in infinite loops.
 * @param node The node to add the child to.
 * @param child The node to add.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_addChild(dsSceneNode* node, dsSceneNode* child);

/**
 * @brief Removes a child from a node by index.
 * @remark errno will be set on failure.
 * @param node The node to remove the child from.
 * @param childIndex The index fo the child to remove.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_removeChildIndex(dsSceneNode* node, uint32_t childIndex);

/**
 * @brief Removes a child from a node by pointer.
 * @remark errno will be set on failure.
 * @param node The node to remove the child from.
 * @param child The child to remove. If the child node was inserted multiple times, all occurences
 *     will be removed.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_removeChildNode(dsSceneNode* node, dsSceneNode* child);

/**
 * @brief Adds the reference count to the node.
 * @remark This function is thread-safe.
 * @param node The node to add the reference to.
 * @return The node with a reference added.
 */
DS_SCENE_EXPORT dsSceneNode* dsSceneNode_addRef(dsSceneNode* node);

/**
 * @brief Subtracts the reference count to the node.
 *
 * Once the reference count reaches 0 the node will be destroyed.
 *
 * @remark This function is thread-safe.
 * @param node The node to add the reference to.
 */
DS_SCENE_EXPORT void dsSceneNode_freeRef(dsSceneNode* node);

#ifdef __cplusplus
}
#endif

