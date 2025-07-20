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
#include <DeepSea/Scene/Nodes/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the basic item list and supporting types in the DeeSea/Scene library.
 */

/**
 * @brief Enum for how to sort models.
 */
typedef enum dsModelSortType
{
	dsModelSortType_None,        ///< Don't sort the models.
	dsModelSortType_Material,    ///< Sort by material to reduce state changes.
	dsModelSortType_BackToFront, ///< Sort back to front, typically for drawing transparent objects.
	dsModelSortType_FrontToBack  ///< Sort front to back, typically for reducing pixel fill.
} dsModelSortType;

/// @cond
typedef struct dsScene dsScene;
typedef struct dsView dsView;
typedef struct dsSceneItemList dsSceneItemList;
typedef struct dsSceneInstanceData dsSceneInstanceData;
/// @endcond

/**
 * @brief Function to populate scene instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @param view The view being drawn.
 * @param commandBuffer The command buffer to process GPU operations. This may be NULL if
 *     needsCommandBuffer is false.
 * @param instances The list of instances.
 * @param instanceCount The number of instances.
 * @return False if an error occurred.
 */
typedef bool (*dsPopulateSceneInstanceDataFunction)(dsSceneInstanceData* instanceData,
	const dsView* view, dsCommandBuffer* commandBuffer, const dsSceneTreeNode* const* instances,
	uint32_t instanceCount);

/**
 * @brief Function for binding scene instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @param index The index of the instance to set.
 * @param values The material values to bind to.
 * @return False if an error occurred.
 */
typedef bool (*dsBindSceneInstanceDataFunction)(dsSceneInstanceData* instanceData, uint32_t index,
	dsSharedMaterialValues* values);

/**
 * @brief Function for finishing the current set of instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @return False if an error occurred.
 */
typedef bool (*dsFinishSceneInstanceDataFunction)(dsSceneInstanceData* instanceData);

/**
 * @brief Function for destroying scene instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @return False if an error occurred.
 */
typedef bool (*dsDestroySceneInstanceDataFunction)(dsSceneInstanceData* instanceData);

/**
 * @brief Struct defining a type for dsSceneInstanceData and functions for the implementation.
 */
typedef struct dsSceneInstanceDataType
{
	/**
	 * @brief Function to populate instance data..
	 */
	dsPopulateSceneInstanceDataFunction populateDataFunc;

	/**
	 * @brief Function to bind the instances for usage.
	 */
	dsBindSceneInstanceDataFunction bindInstanceFunc;

	/**
	 * @brief Function to finish using the current set of instance data.
	 */
	dsFinishSceneInstanceDataFunction finishFunc;

	/**
	 * @brief Function to destroy the instance data.
	 */
	dsDestroySceneInstanceDataFunction destroyFunc;
} dsSceneInstanceDataType;

/**
 * @brief Struct for managing data that's each instance being drawn.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 *
 * @see SceneInstanceData.h
 */
struct dsSceneInstanceData
{
	/**
	 * @brief The allocator the instance data was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the instance data.
	 */
	const dsSceneInstanceDataType* type;

	/**
	 * @brief The number of values that will be stored on dsSharedMaterialValues.
	 */
	uint32_t valueCount;

	/**
	 * @brief Whether a command buffer is needed in populateDataFunc.
	 *
	 * If this is true, populateDataFunc will also be called before the render pass that uses the
	 * instance data starts.
	 */
	bool needsCommandBuffer;
};

/**
 * @brief Function for populating the underlying instance data.
 *
 * The data is stored with the same packing rules as uniform blocks. (or std140)
 *
 * @param userData The user data for managing the instance data.
 * @param view The view being drawn.
 * @param instances The instances to populate the data for.
 * @param instanceCount The number of instances.
 * @param dataDesc The shader variable group description for the data.
 * @param data The data to populate.
 * @param stride The stride between each instance in the data.
 *
 * @see SceneInstanceVariables.h
 */
typedef void (*dsPopulateSceneInstanceVariablesFunction)(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride);

/**
 * @brief Function for adding a node to the item list.
 * @param itemList The item list.
 * @param node The node to add.
 * @param treeNode The node in the scene tree. This is primarily used to get the transform, though
 *     more detailed information such as parent nodes can be queried as well.
 * @param itemData The item data associated with the node.
 * @param thisItemData The pointer to the item data for this specific item. This pointer is
 *     guaranteed to remain the same. Some implementations may choose to store the necessary data in
 *     the pointer value rather than pointing to a separate structure.
 * @return The ID of the node within the item list, or DS_NO_SCENE_NODE if not added.
 */
typedef uint64_t (*dsAddSceneItemListNodeFunction)(dsSceneItemList* itemList,
	dsSceneNode* node, dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData,
	void** thisItemData);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param treeNode The node in the scene tree being updated.
 * @param nodeID The ID of the node to update.
 */
typedef void (*dsUpdateSceneItemListNodeFunction)(dsSceneItemList* itemList,
	dsSceneTreeNode* treeNode, uint64_t nodeID);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param treeNode The node in the scene tree to remove.
 * @param nodeID The ID of the node to remove.
 */
typedef void (*dsRemoveSceneItemListNodeFunction)(dsSceneItemList* itemList,
	dsSceneTreeNode* treeNode, uint64_t nodeID);

/**
 * @brief Function for reparenting a node in an item list.
 * @param itemList The item list.
 * @param nodeID The ID of the node to update.
 * @param prevAncestor The previous shared ancestor for the node in the reparented subtree.
 * @param newAncestor The new shared ancestor for the node in the reparented subtree.
 */
typedef void (*dsReparentSceneItemListNodeFunction)(dsSceneItemList* itemList, uint64_t nodeID,
	 dsSceneTreeNode* prevAncestor, dsSceneTreeNode* newAncestor);

/**
 * @brief Function for updating a scene item list.
 * @param itemList The scene item list to update.
 * @param scene The scene the item list belongs to.
 * @param time The time since the last update.
 */
typedef void (*dsUpdateSceneItemListFunction)(dsSceneItemList* itemList, const dsScene* scene,
	float time);

/**
 * @brief Function for execution operations for a scene item list before a render pass.
 * @param itemList The scene item list to prepare.
 * @param view The view used with the scene item list.
 * @param commandBuffer The command buffer to execute with.
 */
typedef void (*dsPreRenderPassSceneItemListFunction)(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Function for executing a scene item list.
 * @param itemList The scene item list to execute.
 * @param view The view used with the scene item list.
 * @param commandBuffer The command buffer to execute with.
 */
typedef void (*dsCommitSceneItemListFunction)(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Function for destroying a scene item list.
 * @param itemList The scene item list to destroy.
 */
typedef void (*dsDestroySceneItemListFunction)(dsSceneItemList* itemList);

/**
 * @brief Value that denotes the type of a scene item list.
 *
 * To define a type, create a static variable and return the address. This contains the function
 * pointers as well to reduce the size of each item list.
 */
typedef struct dsSceneItemListType
{
	/**
	 * @brief Function for adding a node to the item list.
	 *
	 * This may be NULL if the scene list doesn't interact with nodes.
	 */
	dsAddSceneItemListNodeFunction addNodeFunc;

	/**
	 * @brief Function for updating a node in the item list.
	 *
	 * This may be NULL if nodes don't need to be updated.
	 */
	dsUpdateSceneItemListNodeFunction updateNodeFunc;

	/**
	 * @brief Function for updating a node in the item list.
	 *
	 * This may be NULL if the scene list doesn't interact with nodes.
	 */
	dsRemoveSceneItemListNodeFunction removeNodeFunc;

	/**
	 * @brief Function for notifying that a node's ancestor changed in the item list.
	 *
	 * This may be NULL if the item list doesn't track ancestor nodes.
	 */
	dsReparentSceneItemListNodeFunction reparentNodeFunc;

	/**
	 * @brief Function for updating the scene item list before transforms are processed.
	 *
	 * This may be used to adjust the transforms of nodes. This may be NULL if the item list doesn't
	 * need to be udpated before transforms.
	 */
	dsUpdateSceneItemListFunction preTransformUpdateFunc;

	/**
	 * @brief Function for updating the scene item list.
	 *
	 * This may be NULL if the item list doesn't need to be udpated.
	 */
	dsUpdateSceneItemListFunction updateFunc;

	/**
	 * @brief Function for executing operations for the scene item list before the render pass.
	 *
	 * This is generally used for operations that need to be performed outside of a render pass,
	 * such as GPU copies. This may be NULL if nothing needs to happen before the render pass. This
	 * must not be provided if not a part of a render pass in the scene.
	 *
	 * @remark This will always be called on the same thread as commitFunc. In other words, it's
	 * safe to store state between preRenderPassFunc and commitFunc.
	 */
	dsPreRenderPassSceneItemListFunction preRenderPassFunc;

	/**
	 * @brief Function for committing the scene item list.
	 *
	 * This may be NULL if the item list only needs itself or nodes to be updated.
	 */
	dsCommitSceneItemListFunction commitFunc;

	/**
	 * @brief Function for destroying the scene item list.
	 */
	dsDestroySceneItemListFunction destroyFunc;
} dsSceneItemListType;

/**
 * @brief Struct for processing items within a scene.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 *
 * @see SceneItemList.h
 */
struct dsSceneItemList
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the item list.
	 */
	const dsSceneItemListType* type;

	/**
	 * @brief The name of the scene item list.
	 */
	const char* name;

	/**
	 * @brief The name ID for the item list.
	 */
	uint32_t nameID;

	/**
	 * @brief The number of global values that will be stored on dsSharedMaterialValues.
	 *
	 * A dsSceneItemList instance that has a globalValueCount > 1 must:
	 * 1. Be in the sharedItems list for a dsScene.
	 * 2. Be the only itemList in the dsSceneItemLists entry. (i.e. count == 1)
	 *
	 * This avoids unsafe concurrent access to the dsSharedMaterialValues for global data.
	 */
	uint32_t globalValueCount;

	/**
	 * @brief Whether or not the command buffer is required for the commit function.
	 */
	bool needsCommandBuffer;

	/**
	 * @brief Whether to skip preRenderPassFunc even if it is set in the type.
	 *
	 * This is useful for situations where the type provides preRenderPassFunc, but it is not always
	 * needed.
	 */
	bool skipPreRenderPass;
};

/**
 * @brief Scene item list implementation for drawing models.
 *
 * This will hold information from dsSceneModelNode node types.
 *
 * @see SceneModelList.h
 */
typedef struct dsSceneModelList dsSceneModelList;

/**
 * @brief Full screen resolve within a scene.
 *
 * Full screen resolve draws a full screen quad with a shader and material. This is an item list
 * type to fit into the scene layout, though it doesn't draw any items from the scene.
 *
 * The geometry drawn will be vec2 values in the range [-1, 1], with (-1, -1) being the lower-left
 * corner and (1, 1) being the upper-right corner.
 */
typedef struct dsSceneFullScreenResolve dsSceneFullScreenResolve;

#ifdef __cplusplus
}
#endif
