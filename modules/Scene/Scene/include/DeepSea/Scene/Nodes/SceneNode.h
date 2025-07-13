/*
 * Copyright 2019-2025 Aaron Barany
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
 * @brief The type name for a reference node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneNodeRef_typeName;

/**
 * @brief Gets the allocated size for item lists.
 *
 * This can be added to the other node memory to combine with a single allocation. This assumes one
 * allocation for the array, then a separate allocation for each string.
 *
 * @param itemLists The list of item list names to use.
 * @param itemListCount The number of item lists.
 * @return The size of the memory for the item lists or 0 if the inputs are invalid.
 */
DS_SCENE_EXPORT size_t dsSceneNode_itemListsAllocSize(const char* const* itemLists,
	uint32_t itemListCount);

/**
 * @brief Copies a list of item list names.
 * @param allocator The allocator to create the copy with.
 * @param itemLists The list of item list names to copy.
 * @param itemListCount The number of item lsits.
 * @return The copied item list names or NULL if itemLists is empty or an error occurred.
 */
DS_SCENE_EXPORT const char* const* dsSceneNode_copyItemLists(dsAllocator* allocator,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Sets up the parent type for a node.
 *
 * This should be called for dsSceneNode implementations that have a base type. (apart from
 * dsSceneNode itself)
 *
 * In order to support further subclassing, each node should have a
 * xxx_setupParentType(dsSceneNodeType* type) function like so:
 *
 * ```c
 * static dsSceneNodeType mySceneNodeType;
 * const dsSceneNodeType* dsMySceneNode_setupParentType(dsSceneNodeType* type)
 * {
 *     // First guarantee that the type for dsMySceneNode is fully set up.
 *     // Use this line if dsBaseSceneNode itself has a parent type.
 *     dsBaseSceneNode_setupParentType(&mySceneNodeType);
 *     // Use this line instead of the parent line if dsBaseSceneNode has no parent type.
 *     dsSceneNode_setupParentType(&mySceneNodeType, dsBaseSceneNode_type());
 *
 *     // Now set up type passed in.
 *     return dsSceneNode_setupParentType(type, &mySceneNodeType);
 * }
 * ```
 *
 * The type should be set up in the create function for your node. This guarantees that the parent
 * type for the full hierarchy is initialized before it's ever needed.
 *
 * ```c
 * dsMySceneNode* dsMySceneNode_create(...)
 * {
 *     ...
 *     // Pass in NULL to not set up a derived type of dsMySceneNode, but ensure the full hierarchy
 *     // is initialized.
 *     node->type = dsMySceneNode_setupParentType(NULL);
 *     ...
 * }
 * ```
 *
 * @param type The type to initialize.
 * @param parentType The parent type to set.
 * @return type, or parentType if type is NULL.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneNode_setupParentType(dsSceneNodeType* type,
	const dsSceneNodeType* parentType);

/**
 * @brief Loads a scene node hierarchy from a flatbuffer data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene nodes.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene node allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The type name of the node to load.
 * @param data The data for the scene node. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @return The scene node hierarchy or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneNode* dsSceneNode_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size,
	void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc);

/**
 * @brief Initializes a scene node.
 * @remark errno will be set on failure.
 * @remark The ref count of the node will begin at 1.
 * @param node The node to initialize.
 * @param allocator The allocator the node was created with. This must support freeing memory.
 * @param type The type node.
 * @param itemLists The list of item list names to use. These should be allocated with the node by
 *     using a dsBufferAllocator and dsSceneNode_copyItemLists() so they may all be freed at once.
 * @param itemListCount The number of item lists.
 * @param destroyFunc The function to destroy the node.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneNode_initialize(dsSceneNode* node, dsAllocator* allocator,
	const dsSceneNodeType* type, const char* const* itemLists, uint32_t itemListCount,
	dsDestroySceneNodeFunction destroyFunc);

/**
 * @brief Returns whether or nto a scene node is of a specific type.
 * @param node The node to check the type of.
 * @param type The type to check against node.
 * @return True if the type of the node, or a parent type, is type.
 */
DS_SCENE_EXPORT bool dsSceneNode_isOfType(const dsSceneNode* node, const dsSceneNodeType* type);

/**
 * @brief Adds a child to a node.
 * @remark errno will be set on failure.
 * @remark Adding a circular reference can result in infinite loops.
 * @param node The node to add the child to. It may not already be a child of this node.
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
 * @param child The child to remove.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_removeChildNode(dsSceneNode* node, dsSceneNode* child);

/**
 * @brief Moves a child node from this to another parent.
 *
 * This will preserve the underlying tree structure. This can be important when item lists store
 * state for the node and you wish to change what transform it's associated with.
 *
 * @remark errno will be set on failure.
 * @param node The node to remove the child from.
 * @param childIndex The index fo the child to reparent.
 * @param newParent The node to move the child to. This must have the same number of internal tree
 *     nodes (i.e. node->treeNodeCount == newParent->treeNodeCount) to move the child.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_reparentChildIndex(dsSceneNode* node, uint32_t childIndex,
	dsSceneNode* newParent);

/**
 * @brief Moves a child node from this to another parent.
 *
 * This will preserve the underlying tree structure. This can be important when item lists store
 * state for the node and you wish to change what transform it's associated with.
 *
 * @remark errno will be set on failure.
 * @param node The node to remove the child from.
 * @param child The node to reparent.
 * @param newParent The node to move the child to. This must have the same number of internal tree
 *     nodes (i.e. node->treeNodeCount == newParent->treeNodeCount) to move the child.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneNode_reparentChildNode(dsSceneNode* node, dsSceneNode* child,
	dsSceneNode* newParent);

/**
 * @brief Finds a unique tree node based on a scene node hierarchy.
 *
 * This can be useful for having a base node representing a unique instance and subnodes, which may
 * shared across multiple unique base node instances, that store scene data inside the respective
 * tree nodes. For example, a set of chairs that share the same model, but each have a base
 * transform node that uniquely represents each individual chair.
 *
 * @remark errno will be set on failure.
 * @param baseNode The base node. This must only be instantiated once (i.e. have a single
 *     dsSceneTreeNode) to find any unique instances. This may be NULL if descendent node is itself
 *     unique.
 * @param descendentNode The descendent node to find the unique tree node for.
 * @return The unique tree node for descendentNode or NULL if not found.
 */
DS_SCENE_EXPORT dsSceneTreeNode* dsSceneNode_findUniqueTreeNode(const dsSceneNode* baseNode,
	const dsSceneNode* descendentNode);

/**
 * @brief Clears all chidlren from a scene node.
 * @param node The node to clear.
 */
DS_SCENE_EXPORT void dsSceneNode_clear(dsSceneNode* node);

/**
 * @brief Increments the reference count to the node.
 * @remark This function is thread-safe.
 * @param node The node to increment the reference count.
 * @return The node with a reference incremented.
 */
DS_SCENE_EXPORT dsSceneNode* dsSceneNode_addRef(dsSceneNode* node);

/**
 * @brief Decrements the reference count to the node.
 *
 * Once the reference count reaches 0 the node will be destroyed.
 *
 * @remark This function is thread-safe.
 * @param node The node to decrement the reference count from.
 */
DS_SCENE_EXPORT void dsSceneNode_freeRef(dsSceneNode* node);

#ifdef __cplusplus
}
#endif
