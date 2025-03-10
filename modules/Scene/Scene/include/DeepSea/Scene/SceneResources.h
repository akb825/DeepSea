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

#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene resource collections.
 * @see dsSceneResources
 */

/**
 * @brief Gets the size of dsSceneResources.
 * @return sizeof(dsSceneResources)
 */
DS_SCENE_EXPORT size_t dsSceneResources_sizeof(void);

/**
 * @brief Gets the full allocated size of dsSceneResources.
 * @param maxResources The maximum number of resources that can be held.
 * @return The full allocated size of the dsSceneResources instance.
 */
DS_SCENE_EXPORT size_t dsSceneResources_fullAllocSize(uint32_t maxResources);

/**
 * @brief Creates a scene resources object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources with.
 * @param maxResources The maximum number of resources that can be held.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_create(dsAllocator* allocator,
	uint32_t maxResources);

/**
 * @brief Loads scene resources from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene resources allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param filePath The file path for the scene resources to load.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_loadFile(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* filePath);

/**
 * @brief Loads scene resources from a resource file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene resources allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The resource type.
 * @param filePath The file path for the scene resources to load.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_loadResource(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsFileResourceType type, const char* filePath);

/**
 * @brief Loads scene resources from a file within an archive.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene resources allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param archive The archive to load the scene resources from.
 * @param filePath The file path for the scene resources to load.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_loadArchive(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const dsFileArchive* archive, const char* filePath);

/**
 * @brief Loads scene resources from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene resources allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param stream The stream for the scene resources to load.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_loadStream(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsStream* stream, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc);

/**
 * @brief Loads scene resources from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene resources.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the scene resources allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param data The data for the scene resources. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @return The scene resources or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_loadData(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const void* data, size_t size, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc);

/**
 * @brief Gets the number of remaining resources that can be set.
 * @param resources The scene resources.
 * @return The number of remaining resources.
 */
DS_SCENE_EXPORT uint32_t dsSceneResources_getRemainingResources(const dsSceneResources* resources);

/**
 * @brief Adds a resource to the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the resource. The length, including the null terminator, must not exceed
 *     DS_MAX_SCENE_NAME_LENGTH.
 * @param type The type of the resource to add.
 * @param resource The resource to add.
 * @param own True to take ownership of the resource when the addition succeeds. If the resource
 *     type is dsSceneResourceType_SceneNode, the value of @c own will be ignored and the node will
 *     always have its reference count incremented.
 * @return False if the resource couldn't be added.
 */
DS_SCENE_EXPORT bool dsSceneResources_addResource(dsSceneResources* resources,
	const char* name, dsSceneResourceType type, void* resource, bool own);

/**
 * @brief Removes a resource from the scene resources.
 * @remark errno will be set on failure.
 * @param resources The scene resources.
 * @param name The name of the resource to remove.
 * @param relinquish True to relinquish ownership of the resource. This will prevent this from
 *     freeing the resource if it is owned.
 * @return False if the resource couldn't be removed.
 */
DS_SCENE_EXPORT bool dsSceneResource_removeResource(dsSceneResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a scene resource.
 * @param[out] outType The type of the resource.
 * @param[out] outResource The pointer of the resource.
 * @param resources The scene resources.
 * @param name The name of the resource.
 * @return True if the resource was found.
 */
DS_SCENE_EXPORT bool dsSceneResources_findResource(dsSceneResourceType* outType, void** outResource,
	const dsSceneResources* resources, const char* name);

/**
 * @brief Adds the reference count for the scene resources.
 * @remark This function is thread-safe.
 * @param resources The resources to add a reference count to.
 * @return The resources with an incremented reference count.
 */
DS_SCENE_EXPORT dsSceneResources* dsSceneResources_addRef(dsSceneResources* resources);

/**
 * @brief Subtracts the reference count to the node.
 *
 * Once the reference count reaches 0 the resources will be destroyed.
 *
 * @remark This function is thread-safe, though should only be called from threads with an active
 *     resource context if there are owned resources.
 * @param resources The resources to subtract the reference from.
 */
DS_SCENE_EXPORT void dsSceneResources_freeRef(dsSceneResources* resources);

#ifdef __cplusplus
}
#endif
