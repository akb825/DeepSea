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
#include <DeepSea/Scene/ItemLists/Types.h>
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
 * @brief Function to destroy the user data stored within various scene objects..
 * @param userData The user data to destroy.
 */
typedef void (*dsDestroySceneUserDataFunction)(void* userData);

/**
 * @brief Struct that holds a list of dsSceneItemList instances used for a render subpass.
 *
 * The dsScenItemLists must draw the items as part of the render pass. (as opposed to processing
 * compute shader items)
 *
 * @see SceneRenderPass.h
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
 * @see Scene.h
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

/**
 * @brief Struct containing global data used within a scene.
 * @see SceneGlobalData.h
 */
typedef struct dsSceneGlobalData dsSceneGlobalData;

/**
 * @brief Function to populate scene global data.
 * @remark errno should be set on failure.
 * @param globalData The instance data.
 * @param view The view being drawn. Material values should be set on view->globalValues.
 * @return False if an error occurred.
 */
typedef bool (*dsPopulateSceneGlobalDataFunction)(dsSceneGlobalData* globalData,
	const dsView* view);

/**
 * @brief Function for finishing the current set of global data.
 * @remark errno should be set on failure.
 * @param globalData The global data.
 * @return False if an error occurred.
 */
typedef bool (*dsFinishSceneGlobalDataFunction)(dsSceneGlobalData* globalData);

/**
 * @brief Function for destroying scene global data.
 * @remark errno should be set on failure.
 * @param globalData The global data.
 * @return False if an error occurred.
 */
typedef bool (*dsDestroySceneGlobalDataFunction)(dsSceneGlobalData* globalData);

/** @copydoc dsSceneGlobalData */
struct dsSceneGlobalData
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of values that will be stored on dsSharedMaterialValues.
	 */
	uint32_t valueCount;

	/**
	 * @brief Data populate function.
	 */
	dsPopulateSceneGlobalDataFunction populateDataFunc;

	/**
	 * @brief Finish function.
	 */
	dsFinishSceneGlobalDataFunction finishFunc;

	/**
	 * @brief Destroy function.
	 */
	dsDestroySceneGlobalDataFunction destroyFunc;
};

/**
 * @brief Type for an ID for a unique cull type.
 *
 * Implementations should declare an int as a static variable and return its pointer to get the ID.
 *
 * @see SceneCullManager.h
 */
typedef int* dsSceneCullID;

/**
 * @brief Struct to manage multiple types of culls.
 *
 * Ultimately this will convert a cull ID into a bit to use within a bitmask of cull results.
 *
 * @see SceneCullManager.h
 */
typedef struct dsSceneCullManager
{
	/**
	 * @brief The registered cull IDs.
	 */
	dsSceneCullID cullIDs[32];

	/**
	 * @brief The number of registered ID.
	 */
	uint32_t registeredIDCount;
} dsSceneCullManager;

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
	const dsScene* scene;

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
	 * @brief Cull manager used when drawing the scene.
	 */
	dsSceneCullManager cullManager;

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
