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
#include <DeepSea/Geometry/Types.h>
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
#define DS_NO_SCENE_NODE (uint64_t)-1

/**
 * @brief Constant for the maximum length of a scene resource name, including the null terminator.
 */
#define DS_MAX_SCENE_RESOURCE_NAME_LENGTH 100U

/**
 * @brief Enum for the type of a resource stored in dsSceneResources.
 */
typedef enum dsSceneResourceType
{
	dsSceneResourceType_Buffer,
	dsSceneResourceType_Texture,
	dsSceneResourceType_ShaderVariableGroupDesc,
	dsSceneResourceType_ShaderVariableGroup,
	dsSceneResourceType_MaterialDesc,
	dsSceneResourceType_Material,
	dsSceneResourceType_ShaderModule,
	dsSceneResourceType_Shader,
	dsSceneResourceType_DrawGeometry
} dsSceneResourceType;

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
 * @remark Members should be modified outside of the implementation unless otherwise specified.
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
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneNode.h
 */
typedef struct dsSceneNode dsSceneNode;

/**
 * @brief Struct for a node in the scene tree, which reflects the scene graph.
 */
typedef struct dsSceneTreeNode dsSceneTreeNode;

/**
 * @brief Struct for holding a collection of resources used in a scene.
 *
 * The resources held in the collection may be referenced by name, and allow a way to easily access
 * them within nodes in a scene graph. The struct is reference counted, ensuring that the resources
 * remain valid as long as they're in use.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneResources.h
 */
typedef struct dsSceneResources dsSceneResources;

/**
 * @brief Info for a single instance inside a scene that will be drawn.
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneInstanceData.h
 */
typedef struct dsSceneInstanceInfo
{
	/**
	 * @brief The original node for the data.
	 */
	const dsSceneNode* node;

	/**
	 * @brief The transform for the instance.
	 */
	dsMatrix44f transform;
} dsSceneInstanceInfo;

/**
 * @brief Function for populating the underlying instance data.
 *
 * The data is stored with the same packing rules as uniform blocks. (or std140)
 *
 * @param userData The user data for managing the instance data.
 * @param view The view being drawn.
 * @param instances The instances to populate the data for.
 * @param instanceCount The number of instances.
 * @param data The data to populate.
 * @param stride The stride between each instance in the data.
 *
 * @see SceneInstanceData.h
 */
typedef void (*dsPopulateSceneInstanceDataFunction)(void* userData, const dsView* view,
	const dsSceneInstanceInfo* instances, uint32_t instanceCount, uint8_t* data, uint32_t stride);

/**
 * @brief Function to destroy the user data associated with dsSceneInstanceData.
 * @param userData The user data to destroy.
 *
 * @see SceneInstanceData.h
 */
typedef void (*dsDestroySceneInstanceUserDataFunction)(void* userData);

/**
 * @brief Struct for controlling data that's set for each instance being drawn.
 *
 * This fulfills a role similar to dsShaderVariableGroup, excpet it can be used efficiently for
 * constantly changing values.
 *
 * @see SceneInstanceData.h
 */
typedef struct dsSceneInstanceData dsSceneInstanceData;

/**
 * @brief Function for adding a node to the item list.
 * @param itemList The item list.
 * @param node The node to add.
 * @param transform The transform for the node. The contents of the pointer will change as the node
 *     is updated.
 * @return The ID of the node within the item list, or DS_NO_SCENE_NODE if not added.
 */
typedef uint64_t (*dsAddSceneItemListNodeFunction)(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param nodeID The ID of the node to update.
 */
typedef void (*dsUpdateSceneItemListNodeFunction)(dsSceneItemList* itemList, uint64_t nodeID);

/**
 * @brief Function for updating a node in an item list.
 * @param itemList The item list.
 * @param nodeID The ID of the node to update.
 */
typedef void (*dsRemoveSceneItemListNodeFunction)(dsSceneItemList* itemList, uint64_t nodeID);

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
 * @brief Function to destroy scene node user data.
 * @param node The node the user data was set on.
 * @param data The user data for the node.
 */
typedef void (*dsDestroySceneNodeUserDataFunction)(dsSceneNode* node, void* data);

/**
 * @brief Function for destroying a scene ndoe.
 * @param node The node to destroy.
 */
typedef void (*dsDestroySceneNodeFunction)(dsSceneNode* node);

/** @copydoc dsSceneItemList */
struct dsSceneItemList
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The name of the scene item list.
	 */
	const char* name;

	/**
	 * @brief The name ID for the item list.
	 */
	uint32_t nameID;

	/**
	 * @brief Function for adding a node to the item list.
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
 * @brief Scene item list implementation for drawing models.
 *
 * This will hold information from dsSceneModelNode node types.
 *
 * @see SceneModelList.h
 */
typedef struct dsSceneModelList dsSceneModelList;

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
	 * @brief Custom user data to store with the node.
	 */
	void* userData;

	/**
	 * @brief Function called on destruction to destroy the user data.
	 */
	dsDestroySceneNodeUserDataFunction destroyUserDataFunc;

	/**
	 * @brief Destroy function.
	 */
	dsDestroySceneNodeFunction destroyFunc;
};

/**
 * @brief Scene node implementation that contains a transform for any subnodes.
 * @remark None of the members should be modified outside of the implementation.
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
	 *
	 * This is the local transform for this node relative to any parent nodes.
	 *
	 * This should not be assigned directly since it won't udpate the transforms for any children.
	 * Instead, dsSceneTransformNode_setTransform() should be called. The children will then have
	 * their transforms updated in the call to dsScene_update().
	 */
	dsMatrix44f transform;
} dsSceneTransformNode;

/**
 * @brief Info for what to draw inside a model node when initializing.
 * @see SceneDrawNode.h
 */
typedef struct dsSceneModelInitInfo
{
	/**
	 * @brief The shader to draw the model with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to draw the model with.
	 */
	dsMaterial* material;

	/**
	 * @brief Geometry instance to draw.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief The distance range to draw the model.
	 *
	 * Lower range is inclusive, upperrange is exclusive. If the x value is larger than the y value,
	 * then the model will always be drawn.
	 */
	dsVector2f distanceRange;

	union
	{
		/**
		 * @brief The draw range.
		 *
		 * This will be used if geometry doesn't have an index buffer.
		 */
		dsDrawRange drawRange;

		/**
		 * @brief The indexed draw range.
		 *
		 * This will be used if geometry has an index buffer.
		 */
		dsDrawIndexedRange drawIndexedRange;
	};

	/**
	 * @brief The primitive type for the draw.
	 */
	dsPrimitiveType primitiveType;

	/**
	 * @brief The name for the list to use the model with.
	 */
	const char* listName;
} dsSceneModelInitInfo;

/**
 * @brief Info for what to draw inside a model node.
 * @see SceneDrawNode.h
 */
typedef struct dsSceneModelInfo
{
	/**
	 * @brief The shader to draw the model with.
	 */
	dsShader* shader;

	/**
	 * @brief The material to draw the model with.
	 */
	dsMaterial* material;

	/**
	 * @brief Geometry instance to draw.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief The distance range to draw the model.
	 *
	 * Lower range is inclusive, upperrange is exclusive. If the x value is larger than the y value,
	 * then the model will always be drawn.
	 */
	dsVector2f distanceRange;

	union
	{
		/**
		 * @brief The draw range.
		 *
		 * This will be used if geometry doesn't have an index buffer.
		 */
		dsDrawRange drawRange;

		/**
		 * @brief The indexed draw range.
		 *
		 * This will be used if geometry has an index buffer.
		 */
		dsDrawIndexedRange drawIndexedRange;
	};

	/**
	 * @brief The primitive type for the draw.
	 */
	dsPrimitiveType primitiveType;

	/**
	 * @brief The name ID for the list to use the model with.
	 */
	uint32_t listNameID;
} dsSceneModelInfo;

/**
 * @brief Scene node implementation that contains model geometry to draw.
 * @remark None of the members should be modified outside of the implementation.
 * @see SceneModelNode.h
 */
typedef struct dsSceneModelNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The models that will be drawn within the ndoe.
	 */
	dsSceneModelInfo* models;

	/**
	 * @brief The resources to keep a reference to.
	 *
	 * This will ensure that any resources used within models are kept alive.
	 */
	dsSceneResources** resources;

	/**
	 * @brief The number of models.
	 */
	uint32_t modelCount;

	/**
	 * @brief The number of resources.
	 */
	uint32_t resourceCount;

	/**
	 * @brief The bounding box for the model.
	 */
	dsOrientedBox3f bounds;
} dsSceneModelNode;

/** @copydoc dsView */
struct dsView
{
	/**
	 * @brief The allocator for the view.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The scene to draw with the view.
	 */
	dsScene* scene;

	/**
	 * @brief The camera matrix, transforming from camera to world.
	 */
	dsMatrix44f cameraMatrix;

	/**
	 * @brief The view matrix, transforming from world to camera.
	 *
	 * This is the inverse of the camera matrix.
	 */
	dsMatrix44f viewMatrix;

	/**
	 * @brief The projection matrix.
	 */
	dsMatrix44f projectionMatrix;

	/**
	 * @brief The pre-multiplied view projection matrix.
	 */
	dsMatrix44f viewProjectionMatrix;

	/**
	 * @brief The view frustum.
	 */
	dsFrustum3f viewFrustum;

	/**
	 * @brief The viewport to draw to.
	 *
	 * This may be modified directly, though not in the middle of drawing a view.
	 */
	dsAlignedBox3f viewport;

	/**
	 * @brief Global material values to do while drawing.
	 *
	 * The contents of this may be modified as needed.
	 */
	dsSharedMaterialValues* globalValues;
};

#ifdef __cplusplus
}
#endif
