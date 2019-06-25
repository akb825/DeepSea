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
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Scene library.
 */

/**
 * @brief Log tag used by the scene library.
 */
#define DS_SCENE_LOG_TAG "scene"

/**
 * @brief Constant for no scene node.
 */
#define DS_NO_SCENE_NODE (uint32_t)-1

/**
 * @brief Struct for processing items within a scene.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 */
typedef struct dsSceneItemList dsSceneItemList;

/**
 * @brief Struct that describes a scene.
 * @see Scene.h
 */
typedef struct dsScene dsScene;

/**
 * @brief Struct that describes a view to draw a scene with.
 * @see View.h
 */
typedef struct dsView dsView;

/**
 * @brief ID for a type of a scene node.
 *
 * Specific node implementations should declare an arbitrary static int variable, the address of
 * which will be the ID of that node type. This guarantees a unique identifier that can be ued
 * program-wide.
 */
typedef const int* dsSceneNodeType;

/**
 * @brief Struct for a node within a scene graph.
 *
 * Scene nodes are reference counted. They may be referenced multiple times, or even within
 * different scenes. The reference count starts at 1 on creation and once the last reference has
 * been freed the node will be deleted.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 *
 * @remark A node may not be a sibbling with itself, sharing the same direct parent. If you want to
 * have the same node appear multiple times, there must be a separate parent between them. For
 * example, the following is not allowed:
 * ```
 *     A
 *    / \
 *   B   B
 * ```
 * However, the following is allowed:
 * ```
 *     A
 *    / \
 *   C   D
 *   |   |
 *   B   B
 * ```
 *
 * @see SceneNode.h
 */
typedef struct dsSceneNode dsSceneNode;

/**
 * @brief Struct for a node in the scene tree, which reflects the scene graph.
 */
typedef struct dsSceneTreeNode dsSceneTreeNode;

/**
 * @brief Function for adding a node to the item list.
 * @param itemList The item list.
 * @param node The node to add.
 * @param transform The transform for the node. The contents of the pointer will change as the node
 *     is updated.
 * @return The ID of the node within the item list, or DS_NO_SCENE_NODE if not added.
 */
typedef uint32_t (*dsAddSceneItemListNodeFunction)(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param nodeID The ID of the node to update.
 */
typedef void (*dsUpdateSceneItemListNodeFunction)(dsSceneItemList* itemList, uint32_t nodeID);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param nodeID The ID of the node to update.
 */
typedef void (*dsRemoveSceneItemListNodeFunction)(dsSceneItemList* itemList, uint32_t nodeID);

/**
 * @brief Function for drawing a scene item list.
 * @param itemList The scene item list to draw.
 * @param view The view to draw to.
 * @param commandBuffer The command buffer to draw to.
 */
typedef void (*dsCommitSceneItemListFunction)(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Function for destroying a scene item list.
 * @param itemList The scene item list to destroy.
 */
typedef void (*dsDestroySceneItemListFunction)(dsSceneItemList* itemList);

/**
 * @brief Function for destroying a scene ndoe.
 * @param node The node to destroy.
 */
typedef void (*dsDestroySceneNodeFunction)(dsSceneNode* node);

/** @copydoc dsSceneItemList */
struct dsSceneItemList
{
	/**
	 * @brief The scene the scene item list is used with.
	 */
	dsScene* scene;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The name of the scene item list.
	 */
	const char* name;

	/**
	 * @brief Function for adding a node to the item list.
	 */
	dsAddSceneItemListNodeFunction addNodeFunc;

	/**
	 * @brief Function for updating a node in the item list.
	 */
	dsUpdateSceneItemListNodeFunction updateNodeFunc;

	/**
	 * @brief Function for updating a node in the item list.
	 */
	dsRemoveSceneItemListNodeFunction removeNodeFunc;

	/**
	 * @brief Function for committing the scene item list.
	 */
	dsCommitSceneItemListFunction commitFunc;

	/**
	 * @brief Function for destroying the scene item list.
	 */
	dsDestroySceneItemListFunction destroyFunc;
};

/**
 * @brief Struct that holds a list of dsSceneItemList instances used for a render subpass.
 *
 * The dsScenItemLists must draw the items as part of the render pass. (as opposed to processing
 * compute shader items)
 */
typedef struct dsSubpassDrawLists
{
	/**
	 * @brief The scene item lists to draw.
	 */
	dsSceneItemList** drawLists;

	/**
	 * @brief The number of scene item lists.
	 */
	uint32_t count;
} dsSubpassDrawLists;

/**
 * @brief Struct describing a render pass within a scene.
 *
 * This extends dsRenderPass in the renderer library by containing one or more dsSceneItemList
 * instances for each subpass.
 *
 * @see SceneRenderPass.h
 */
typedef struct dsSceneRenderPass
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The base render pass this extends.
	 */
	dsRenderPass* renderPass;

	/**
	 * @brief The scene item lists for each subpass.
	 */
	dsSubpassDrawLists* drawLists;
} dsSceneRenderPass;

/**
 * @brief Struct containing an item within the rendering pipeline for a scene.
 */
typedef struct dsScenePipelineItem
{
	/**
	 * @brief The render pass.
	 * @remark If this is set, computeItems must be NULL.
	 */
	dsSceneRenderPass* renderPass;

	/**
	 * @brief The compute items to process.
	 * @remark If this is set, renderPass must be NULL.
	 */
	dsSceneItemList* computeItems;
} dsScenePipelineItem;

/** @copydoc dsSceneNode */
struct dsSceneNode
{
	/**
	 * @brief The allocator for the node.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the node.
	 */
	dsSceneNodeType type;

	/**
	 * @brief The children of the node.
	 */
	dsSceneNode** children;

	/**
	 * @brief The draw lists that will use the node.
	 */
	const char** drawLists;

	/**
	 * @brief The tree nodes that correspond to this node in various scenes.
	 *
	 * This is for internal management of the scene graph.
	 */
	dsSceneTreeNode** treeNodes;

	/**
	 * @brief The number of children.
	 */
	uint32_t childCount;

	/**
	 * @brief The maximum number of children.
	 */
	uint32_t maxChildren;

	/**
	 * @brief The number of draw lists.
	 */
	uint32_t drawListCount;

	/**
	 * @brief The number of tree nodes.
	 */
	uint32_t treeNodeCount;

	/**
	 * @brief The maximum number of tree nodes.
	 */
	uint32_t maxTreeNodes;

	/**
	 * @brief The reference count for the node.
	 *
	 * This will start at 1 on creation.
	 */
	uint32_t refCount;

	/**
	 * @brief Destroy function.
	 */
	dsDestroySceneNodeFunction destroyFunc;
};

/**
 * @brief Scene node implementation that contains a transform for any subnodes.
 * @see SceneTransformNode
 */
typedef struct dsSceneTransformNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The transform for the node.
	 */
	dsMatrix44f transform;
} dsSceneTransformNode;

#ifdef __cplusplus
}
#endif
