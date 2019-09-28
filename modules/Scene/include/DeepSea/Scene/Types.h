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
#define DS_MAX_SCENE_RESOURCE_NAME_LENGTH 104U

/**
 * @brief Enum for the type of a resource stored in dsSceneResources.
 */
typedef enum dsSceneResourceType
{
	dsSceneResourceType_Buffer,                  ///< dsGfxBuffer
	dsSceneResourceType_Texture,                 ///< dsTexture
	dsSceneResourceType_ShaderVariableGroupDesc, ///< dsShadrVariableGroupDesc
	dsSceneResourceType_ShaderVariableGroup,     ///< dsShaderVariableGroup
	dsSceneResourceType_MaterialDesc,            ///< dsMaterialDesc
	dsSceneResourceType_Material,                ///< dsMaterial
	dsSceneResourceType_ShaderModule,            ///< dsShaderModule
	dsSceneResourceType_Shader,                  ///< dsShader
	dsSceneResourceType_DrawGeometry,            ///< dsDrawGeometry
	dsSceneResourceType_SceneNode                ///< dsSceneNode
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
 * @brief Struct that holds a list of dsSceneItemList instances.
 */
typedef struct dsSceneItemLists
{
	/**
	 * @brief The scene item draw.
	 */
	dsSceneItemList** itemLists;

	/**
	 * @brief The number of scene item lists.
	 */
	uint32_t count;
} dsSceneItemLists;

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
	 * @brief The name of the framebuffer.
	 *
	 * This will be copied when creating the scene.
	 */
	const char* framebuffer;

	/**
	 * @brief The clear values for the render pass.
	 *
	 * This may be NULL if no surfaces are cleared, otherwise it must have an element for each
	 * attachment in the render pass. This will be copied when creating the scene.
	 */
	const dsSurfaceClearValue* clearValues;

	/**
	 * @brief The scene item lists for each subpass.
	 */
	dsSceneItemLists* drawLists;
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
 * @param commandBuffer The command buffer.
 * @return False if an error occurred.
 */
typedef bool (*dsPopulateSceneGlobalDataFunction)(dsSceneGlobalData* globalData,
	const dsView* view, dsCommandBuffer* commandBuffer);

/**
 * @brief Function for finishing the current set of global data.
 * @param globalData The global data.
 */
typedef void (*dsFinishSceneGlobalDataFunction)(dsSceneGlobalData* globalData);

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
 * @brief Info for a surface used within a view.
 */
typedef struct dsViewSurfaceInfo
{
	/**
	 * @brief The name of the surface.
	 *
	 * This will be copied.
	 */
	const char* name;

	/**
	 * @brief The type of the surface.
	 */
	dsGfxSurfaceType surfaceType;

	/**
	 * @brief Info to be used to create the texture or renderbuffer when no surface is provided.
	 */
	dsTextureInfo createInfo;

	/**
	 * @brief When the createInfo's width is set to 0, the width will be set to the view's width
	 * times widthRatio.
	 *
	 * The result will be rounded. This will be ignored if an existing surface was provided.
	 */
	float widthRatio;

	/**
	 * @brief When the createInfo's height is set to 0, the height will be set to the view's height
	 * times heightRatio.
	 *
	 * The result will be rounded. This will be ignored if an existing surface was provided.
	 */
	float heightRatio;

	/**
	 * @brief The usage of the surface.
	 *
	 * This should be a combination of dsTextureUsage flags if surfaceType is
	 * dsGfxSurfaceType_Offscreen or dsRenderbufferUsage flags if surfaceType is
	 * dsGfxSurfaceType_Renderbuffer.
	 *
	 * This will be ignored if an existing surface was provided.
	 */
	uint32_t usage;

	/**
	 * @brief The memory hints for the surface.
	 *
	 * This will be ignored if an existing surface was provided.
	 */
	dsGfxMemory memoryHints;

	/**
	 * @brief True to resolve a created offscreen.
	 *
	 * This is ignored when not creating a surface or if the surface type isn't an offscreen.
	 */
	bool resolve;

	/**
	 * @brief The existing surface.
	 *
	 * When NULL, a surface will be created based on createInfo. surfaceType must be
	 * dsGfxSurfaceTye_Offscreen or dsGfxSurfaceType_Renderbuffer if NULL.
	 */
	void* surface;
} dsViewSurfaceInfo;

/**
 * @brief Info for a framebuffer used within the view.
 */
typedef struct dsViewFramebufferInfo
{
	/**
	 * @brief The name fo the framebuffer.
	 *
	 * This will be copied.
	 */
	const char* name;

	/**
	 * @brief The list of surfaces.
	 *
	 * The surface pointer must be the name of the surface from dsViewSurfaceInfo. The array and
	 * surface names will be copied.
	 */
	const dsFramebufferSurface* surfaces;

	/**
	 * @brief The number of surfaces.
	 */
	uint32_t surfaceCount;

	/**
	 * @brief The width of the framebuffer.
	 *
	 * When > 0, this is used as-is for the width. When < 0, it's treated as a ratio to multiply the
	 * view's width. For example, a value of -1.0 is the view's width, while -0.5 is 1/2 the view's
	 * width. The result will be rounded.
	 */
	float width;


	/**
	 * @brief The height of the framebuffer.
	 *
	 * When > 0, this is used as-is for the width. When < 0, it's treated as a ratio to multiply the
	 * view's height. For example, a value of -1.0 is the view's height, while -0.5 is 1/2 the
	 * view's height. The result will be rounded.
	 */
	float height;

	/**
	 * @brief The number of layers for the framebuffer.
	 */
	uint32_t layers;

	/**
	 * The viewport to draw to.
	 *
	 * The x and y values will be treated as a fraction of the overall framebuffer dimensions.
	 */
	dsAlignedBox3f viewport;
} dsViewFramebufferInfo;

/** @copydoc dsView */
struct dsView
{
	/**
	 * @brief The scene to draw with the view.
	 */
	const dsScene* scene;

	/**
	 * @brief The allocator for the view.
	 */
	dsAllocator* allocator;

	/**
	 * @brief User data for the view.
	 */
	void* userData;

	/**
	 * @brief Function to destroy user data.
	 */
	dsDestroySceneUserDataFunction destroyUserDataFunc;

	/**
	 * @brief The width of the view.
	 */
	uint32_t width;

	/**
	 * @brief The height of the view.
	 */
	uint32_t height;

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
	 * @brief Global material values to do while drawing.
	 *
	 * The contents of this may be modified as needed.
	 */
	dsSharedMaterialValues* globalValues;
};

/**
 * @brief Struct that manages threads used to draw across multiple threads.
 *
 * A thread manager may optionally be provided to dsView_draw() to perform draws across multiple
 * threads. The same thread manager may not itself be used concurrently across threads.
 *
 * @see SceneThreadManager.h
 */
typedef struct dsSceneThreadManager dsSceneThreadManager;

#ifdef __cplusplus
}
#endif
