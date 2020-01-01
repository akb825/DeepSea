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

#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scenes.
 * @see dsScene
 */

/**
 * @brief Creates a scene.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene with. This must support freeing memory.
 * @param renderer The renderer the scene will be drawn with.
 * @param sharedItems The item lists to run before the rest of the scene. (e.g. cull item lists)
 *     This will copy the array itself and take ownership of the objects. If creation fails, this
 *     means it will immediately destroy all objects in this list. When multithreaded rendering is
 *     used, the lists within each sharedItems instance may be processed in parallel, but will be
 *     synchronized between each index of the top-level sharedItems.
 * @param sharedItemCount The number of shared items.
 * @param pipeline The pipeline to perform when rendering the scene. This will copy the array itself
 *     and take ownership of the objects, i.e. render passes and draw item lists. If creation fails,
 *     this means it will immediately destroy all objects in this list.
 * @param pipelineCount The number of pipeline items.
 * @param globalData The list of global data isntances used within the scene. This will copy the
 *     array itself and take ownership of the objects. If creation fails, this means it will
 *     immediately destroy all objects in this list.
 * @param globalDataCount The number of global data instances.
 * @param userData User data to hold with the scene.
 * @param destroyUserDataFunc Function to destroy the user data for the scene.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_create(dsAllocator* allocator, dsRenderer* renderer,
	const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Loads a scene from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param filePath The file path for the scene to load.
 * @param userData User data to hold with the scene.
 * @param destroyUserDataFunc Function to destroy the user data for the scene.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	const char* filePath, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Loads a scene from a resource file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The resource type.
 * @param filePath The file path for the scene to load.
 * @param userData User data to hold with the scene.
 * @param destroyUserDataFunc Function to destroy the user data for the scene.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_loadResource(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsFileResourceType type, const char* filePath,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Loads scene from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param stream The stream for the scene to load.
 * @param userData User data to hold with the scene.
 * @param destroyUserDataFunc Function to destroy the user data for the scene.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_loadStream(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, dsStream* stream,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Loads scene from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param data The data for the scene. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param userData User data to hold with the scene.
 * @param destroyUserDataFunc Function to destroy the user data for the scene.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const void* data,
	size_t size, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Gets the allocator used for a scene.
 * @param scene The scene to get the allocator for.
 * @return The allocator.
 */
DS_SCENE_EXPORT dsAllocator* dsScene_getAllocator(const dsScene* scene);

/**
 * @brief Gets the renderer used for a scene.
 * @param scene The scene to get the renderer for.
 * @return The renderer.
 */
DS_SCENE_EXPORT dsRenderer* dsScene_getRenderer(const dsScene* scene);

/**
 * @brief Gets the user data used for a scene.
 * @param scene The scene to get the user data for.
 * @return The user data.
 */
DS_SCENE_EXPORT void* dsScene_getUserData(const dsScene* scene);

/**
 * @brief Gets the number of nodes in the scene.
 * @param scene The scene to get the node count for.
 * @return The node count.
 */
DS_SCENE_EXPORT uint32_t dsScene_getNodeCount(const dsScene* scene);

/**
 * @brief Gets a node from the scene by index.
 * @remark errno will be set on failure.
 * @param scene The scene to get the node from.
 * @param index The index of the node.
 * @return The node, or NULL if the the parameters are invalid.
 */
DS_SCENE_EXPORT dsSceneNode* dsScene_getNode(const dsScene* scene, uint32_t index);

/**
 * @brief Adds a node to a scene.
 * @remark errno will be set on failure.
 * @remark Adding a circular reference can result in infinite loops.
 * @param scene The node to add the node to. It may not already be at the root of the scene.
 * @param node The node to add.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsScene_addNode(dsScene* scene, dsSceneNode* node);

/**
 * @brief Removes a node from the scene by index.
 * @remark errno will be set on failure.
 * @param scene The scene to remove the node from.
 * @param nodeIndex The index fo the node to remove.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsScene_removeNodeIndex(dsScene* scene, uint32_t nodeIndex);

/**
 * @brief Removes a node from the scene by pointer.
 * @remark errno will be set on failure.
 * @param scene The node to remove the node from.
 * @param node The node to remove.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsScene_removeNode(dsScene* scene, dsSceneNode* node);

/**
 * @brief Clears all nodes from a scene.
 * @param scene The scene to clear.
 */
DS_SCENE_EXPORT void dsScene_clearNodes(dsScene* scene);

/**
 * @brief Updates dirty nodes within the scene.
 * @remark errno will be set on failure.
 * @param scene The scene to update.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsScene_update(dsScene* scene);

/**
 * @brief Destroys a scene.
 * @param scene The scene to destroy.
 */
DS_SCENE_EXPORT void dsScene_destroy(dsScene* scene);

#ifdef __cplusplus
}
#endif
