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
#include <DeepSea/Scene/Nodes/Types.h>

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
 * @brief Struct for managing data that's each instance being drawn.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsSceneItemList and the true internal type.
 *
 * @see SceneInstanceData.h
 */
typedef struct dsSceneInstanceData dsSceneInstanceData;

/**
 * @brief Function to populate scene instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @param view The view being drawn.
 * @param instances The list of instances.
 * @param instanceCount The number of instances.
 * @return False if an error occurred.
 */
typedef bool (*dsPopulateSceneInstanceDataFunction)(dsSceneInstanceData* instanceData,
	const dsView* view, const dsSceneInstanceInfo* instances, uint32_t instanceCount);

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
 * @brief Function for destroying scene instance data.
 * @remark errno should be set on failure.
 * @param instanceData The instance data.
 * @return False if an error occurred.
 */
typedef bool (*dsFinishSceneInstanceDataFunction)(dsSceneInstanceData* instanceData);

/** @copydoc dsSceneInstanceData */
struct dsSceneInstanceData
{
	/**
	 * @brief The allocator the instance data was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of values that will be stored on dsSharedMaterialValues.
	 */
	uint32_t valueCount;

	/**
	 * @brief Data populate function.
	 */
	dsPopulateSceneInstanceDataFunction populateDataFunc;

	/**
	 * @brief Bind instance function.
	 */
	dsBindSceneInstanceDataFunction bindInstanceFunc;

	/**
	 * @brief Finish function.
	 */
	dsFinishSceneInstanceDataFunction finishFunc;

	/**
	 * @brief Destroy function.
	 */
	dsFinishSceneInstanceDataFunction destroyFunc;
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
 * @param data The data to populate.
 * @param stride The stride between each instance in the data.
 *
 * @see SceneInstanceVariables.h
 */
typedef void (*dsPopulateSceneInstanceVariablesFunction)(void* userData, const dsView* view,
	const dsSceneInstanceInfo* instances, uint32_t instanceCount, uint8_t* data, uint32_t stride);

/**
 * @brief Function to destroy the user data associated with dsSceneInstanceVariables.
 * @param userData The user data to destroy.
 * @see SceneInstanceVariables.h
 */
typedef void (*dsDestroySceneInstanceVariablesUserDataFunction)(void* userData);

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
