/*
 * Copyright 2019-2023 Aaron Barany
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
#define DS_NO_SCENE_NODE ((uint64_t)-1)

/**
 * @brief Constant for the maximum length of a scene name, including the null terminator.
 *
 * This is used for names stored in dsSceneResources and registered with dsSceneLoadContext
 */
#define DS_MAX_SCENE_NAME_LENGTH 104U

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
	dsSceneResourceType_SceneNode,               ///< dsSceneNode
	dsSceneResourceType_Custom                   ///< dsCustomSceneResource
} dsSceneResourceType;

/**
 * @brief Struct that describes a scene.
 * @see Scene.h
 */
typedef struct dsScene dsScene;

/// @cond
typedef struct dsView dsView;
/// @endcond

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
 * @brief Arbitrary type used to denote a custom resource type.
 *
 * Declare this as a static variable and take the address to denote the type. e.g.
 *
 * ```
 * static dsCustomResourceType myResourceType;
 * const dsSceneResourceType* dsMyResourceType_type(void)
 * {
 *     return &myResourceType;
 * }
 * ```
 */
typedef int dsCustomSceneResourceType;

/**
 * @brief Function to destroy a custom scene resource.
 * @param resource The resource to destroy.
 * @return False if the resource couldn't be destroyed.
 */
typedef bool (*dsDestroyCustomSceneResourceFunction)(void* resource);

/**
 * @brief Struct containing the information for a custom resource.
 * @see CustomSceneResource.h
 */
typedef struct dsCustomSceneResource
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the resource.
	 */
	const dsCustomSceneResourceType* type;

	/**
	 * @brief The pointer to the resource.
	 */
	void* resource;

	/**
	 * @brief The function to destroy the resource.
	 *
	 * This may be NULL if the resource will not be destroyed.
	 */
	dsDestroyCustomSceneResourceFunction destroyFunc;
} dsCustomSceneResource;

/**
 * @brief Function to destroy the user data stored within various scene objects.
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
 * @brief Function for visiting the item lists in a scene.
 * @param itemList The item list being visited.
 * @param userData User data passed through to the callback.
 * @return True to continue iteration, false to stop.
 */
typedef bool (*dsVisitSceneItemListsFunction)(dsSceneItemList* itemList, void* userData);

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
	 *
	 * If the surface is an offscreen, it will be bound as a global shader variable using the
	 * surface name.
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
	 * @brief True if the surface is used in the same framebuffer as the window surface.
	 *
	 * Set this to true to follow the rotation of the view and window surface.
	 */
	bool windowFramebuffer;

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
	 * The surface pointer must be the name of the surface from dsViewSurfaceInfo. surfaceType may
	 * be set to -1 to inherit the surface type from the found surface, otherwise it must match the
	 * found surface's type.
	 *
	 * The array and surface names will be copied.
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
	 * The x and y values will be treated as a fraction of the overall framebuffer dimensions in the
	 * the range [0, 1]. The viewport will automatically be adjusted based on the view rotation.
	 */
	dsAlignedBox3f viewport;
} dsViewFramebufferInfo;

/**
 * @brief Struct that describes a view to draw a scene with.
 * @remark Members should be modified outside of the implementation unless otherwise specified.
 * @see View.h
 */
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
	 * @brief The allocator for graphics resources in the view.
	 */
	dsAllocator* resourceAllocator;

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
	 * @brief The width of the view before applying rotation.
	 *
	 * This will be different from the width if rotation is 90 or 270 degrees. This is the dimension
	 * that should be used for any surfaces that are used in the same framebuffer as a window render
	 * surface.
	 */
	uint32_t preRotateWidth;

	/**
	 * @brief The height of the render surface before applying rotation.
	 *
	 * This will be different from the height if rotation is 90 or 270 degrees. This is the
	 * dimension that should be used for any surfaces that are used in the same framebuffer as a
	 * window render surface.
	 */
	uint32_t preRotateHeight;

	/**
	 * @brief The rotation of the window surface.
	 */
	dsRenderSurfaceRotation rotation;

	/**
	 * @brief Parameters for the projection matrix.
	 */
	dsProjectionParams projectionParams;

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
	 * @brief The view frustum in world space.
	 */
	dsFrustum3f viewFrustum;

	/**
	 * @brief The bias to apply when choosing which LOD to use.
	 *
	 * This will multiply the distance of the object when determining the distance to draw it at.
	 * A value of 1 is default, while a value < 1 will consider the objects to be closer (using
	 * higher LOD), while a value > 1 will consider the objects to be farther (using a lower LOD).
	 *
	 * @remark This member may be modified directly.
	 */
	float lodBias;

	/**
	 * @brief Global material values to do while drawing.
	 *
	 * The contents of this may be modified as needed within the sharedItems list of a scene before
	 * drawing occurs. To do so, call dsView_lockGlobalValues() to lock it for writing, followed by
	 * dsView_unlockGlobalValues() to unlock it.
	 */
	const dsSharedMaterialValues* globalValues;

	/**
	 * @brief Lock for modifying globalValues.
	 */
	dsSpinlock globalValuesLock;
};

/**
 * @brief Struct that manages resources used to draw across multiple threads.
 *
 * A thread manager may optionally be provided to dsView_draw() to perform draws across multiple
 * threads. The same thread manager may not itself be used concurrently across threads.
 *
 * @see SceneThreadManager.h
 */
typedef struct dsSceneThreadManager dsSceneThreadManager;

/**
 * @brief Struct for a context that contains information to aid in loading scenes from file.
 *
 * Custom node, item list, and global data types can be registered with the dsSceneLoadContext to
 * support loading them from scene files.
 *
 * The load context is not when loading scene files, so it may be re-used across threads.
 *
 * @see SceneLoadContext.h
 */
typedef struct dsSceneLoadContext dsSceneLoadContext;

/**
 * @brief Struct containing temporary data used during loading of a scene.
 *
 * This object should not be used across multiple threads. It may be used across multiple loads to
 * re-use the internal buffers and miniize re-allocations.
 *
 * @see SceneLoadScratchData.h
 */
typedef struct dsSceneLoadScratchData dsSceneLoadScratchData;

/**
 * @brief Function to load a scene node.
 * @remark errno should be set on failure.
 * @param loadContext The load context.
 * @param scratchData The scratch data.
 * @param allocator The allocator to create the node with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the node allocator.
 * @param userData User data registered with this function.
 * @param data The data for the node.
 * @param dataSize The size fo the data.
 * @return The node or NULL if it couldn't be loaded.
 */
typedef dsSceneNode* (*dsLoadSceneNodeFunction)(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize);

/**
 * @brief Function to load a scene item list.
 * @remark errno should be set on failure.
 * @param loadContext The load context.
 * @param scratchData The scratch data.
 * @param allocator The allocator to create the item list with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the item list allocator.
 * @param userData User data registered with this function.
 * @param name The name of the item list.
 * @param data The data for the item list.
 * @param dataSize The size fo the data.
 * @return The item list or NULL if it couldn't be loaded.
 */
typedef dsSceneItemList* (*dsLoadSceneItemListFunction)(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize);

/**
 * @brief Function to load a scene instance data.
 * @remark errno should be set on failure.
 * @param loadContext The load context.
 * @param scratchData The scratch data.
 * @param allocator The allocator to create the instance data with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the instance data allocator.
 * @param userData User data registered with this function.
 * @param data The data for the global data.
 * @param dataSize The size fo the data.
 * @return The instance data or NULL if it couldn't be loaded.
 */
typedef dsSceneInstanceData* (*dsLoadSceneInstanceDataFunction)(
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	dsAllocator* allocator, dsAllocator* resourceAllocator, void* userData, const uint8_t* data,
	size_t dataSize);

/**
 * @brief Function to load a custom scene resource.
 * @remark errno should be set on failure.
 * @param loadContext The load context.
 * @param scratchData The scratch data.
 * @param allocator The allocator to create the custom resource with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the custom resource allocator.
 * @param userData User data registered with this function.
 * @param data The data for the custom resource.
 * @param dataSize The size fo the data.
 * @return The custom resource or NULL if it couldn't be loaded.
 */
typedef void* (*dsLoadCustomSceneResourceFunction)(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize);

/**
 * @brief Function to load a scene resource action.
 * @remark errno should be set on failure.
 * @param loadContext The load context.
 * @param scratchData The scratch data.
 * @param allocator The allocator to create data with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the data allocator.
 * @param userData User data registered with this function.
 * @param data The data for the custom resource.
 * @param dataSize The size fo the data.
 * @return The custom resource or NULL if it couldn't be loaded.
 */
typedef bool (*dsLoadSceneResourceActionFunction)(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize);

#ifdef __cplusplus
}
#endif
