/*
 * Copyright 2017-2026 Aaron Barany
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
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating shared vector resources.
 *
 * This can be used to hold resources that are shared across multiple vector draw objects. This may
 * optionally take ownership of the resources to delete them when removed.
 *
 * @see dsVectorResources
 */

/**
 * @brief Gets the full allocation size of vector resources.
 * @param maxResources The maximum resources to store.
 * @return The full allocation size.
 */
DS_VECTORDRAW_EXPORT size_t dsVectorResources_fullAllocSize(uint32_t maxResources);

/**
 * @brief Creates a vector resources object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources with.
 * @param maxResources The maximum resources to store.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorReosurces_create(
	dsAllocator* allocator, uint32_t maxResources);

/**
 * @brief Loads vector resources from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the same as allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param filePath The file path for the vector resources to load.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param initResources The initialization resources for any vector images. This may be NULL if
 *     there are no vector images saved with the resources.
 * @param pixelSize The size of a pixel, determining tessellation quality for any vector images.
 * @param vectorIconShaders The shaders to use for vector text icons. This may be NULL if there are
 *     no vector text icons saved with the resources.
 * @param textureIconShader The shader to use for texture text icons. This may be NULL if there are
 *     no texture text icons saved within the resources.
 * @param textureIconMaterial The material to use for texture text icons. This may be NULL if there
 *     are no texture text icons saved within the resources or an empty material is sufficient.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadFile(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, const char* filePath, const dsTextQuality* qualityRemap,
	const dsVectorImageInitResources* initResources, float pixelSize,
	const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial);

/**
 * @brief Loads vector resources from a resource file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the same as allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param type The resource type.
 * @param filePath The file path for the vector resources to load.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param initResources The initialization resources for any vector images. This may be NULL if
 *     there are no vector images saved with the resources.
 * @param pixelSize The size of a pixel, determining tessellation quality for any vector images.
 * @param vectorIconShaders The shaders to use for vector text icons. This may be NULL if there are
 *     no vector text icons saved with the resources.
 * @param textureIconShader The shader to use for texture text icons. This may be NULL if there are
 *     no texture text icons saved within the resources.
 * @param textureIconMaterial The material to use for texture text icons. This may be NULL if there
 *     are no texture text icons saved within the resources or an empty material is sufficient.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, dsFileResourceType type, const char* filePath,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial);

/**
 * @brief Loads vector resources from a file within an archive.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the same as allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param archive The archive to load the vector resources from.
 * @param filePath The file path for the vector resources to load.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param initResources The initialization resources for any vector images. This may be NULL if
 *     there are no vector images saved with the resources.
 * @param pixelSize The size of a pixel, determining tessellation quality for any vector images.
 * @param vectorIconShaders The shaders to use for vector text icons. This may be NULL if there are
 *     no vector text icons saved with the resources.
 * @param textureIconShader The shader to use for texture text icons. This may be NULL if there are
 *     no texture text icons saved within the resources.
 * @param textureIconMaterial The material to use for texture text icons. This may be NULL if there
 *     are no texture text icons saved within the resources or an empty material is sufficient.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadArchive(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, const dsFileArchive* archive, const char* filePath,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial);

/**
 * @brief Loads vector resources from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the same as allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param stream The stream to load the vector resources from. This stream will be read from the
 *     current position until the end.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param initResources The initialization resources for any vector images. This may be NULL if
 *     there are no vector images saved with the resources.
 * @param pixelSize The size of a pixel, determining tessellation quality for any vector images.
 * @param vectorIconShaders The shaders to use for vector text icons. This may be NULL if there are
 *     no vector text icons saved with the resources.
 * @param textureIconShader The shader to use for texture text icons. This may be NULL if there are
 *     no texture text icons saved within the resources.
 * @param textureIconMaterial The material to use for texture text icons. This may be NULL if there
 *     are no texture text icons saved within the resources or an empty material is sufficient.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, dsStream* stream, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap,
	const dsVectorImageInitResources* initResources, float pixelSize,
	const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial);

/**
 * @brief Loads vector resources from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the same as allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param data The data for the vector resources. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @param qualityRemap Array to remap text qualities, or NULL to use values as-is. If not NULL,
 *     it must be of size DS_TEXT_QUALITY_REMAP_SIZE.
 * @param initResources The initialization resources for any vector images. This may be NULL if
 *     there are no vector images saved with the resources.
 * @param pixelSize The size of a pixel, determining tessellation quality for any vector images.
 * @param vectorIconShaders The shaders to use for vector text icons. This may be NULL if there are
 *     no vector text icons saved with the resources.
 * @param textureIconShader The shader to use for texture text icons. This may be NULL if there are
 *     no texture text icons saved within the resources.
 * @param textureIconMaterial The material to use for texture text icons. This may be NULL if there
 *     are no texture text icons saved within the resources or an empty material is sufficient.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadData(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, const void* data, size_t size, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial);

/**
 * @brief Gets the number of remaining resources that can be added.
 * @param resources The vector resources.
 * @return The number of remaining resources.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorResources_getRemainingResources(
	const dsVectorResources* resources);

/**
 * @brief Adds a resource to the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the texture. The length, including the null terminator, must not exceed
 *     DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param type The type of the resource.
 * @param resource The resource to add.
 * @param own True to take ownership of the resource. If ownership is taken, the resource will be
 *     destroyed if addition fails.
 * @return False if the resource couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_addResource(dsVectorResources* resources,
	const char* name, dsVectorResourceType type, void* resource, bool own);

/**
 * @brief Removes a resource from the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the resource to remove.
 * @param relinquish True to relinquish ownership of the resource. This will prevent this from
 *     freeing the resource if it is owned.
 * @return False if the resource couldn't be removed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_removeResource(
	dsVectorResources* resources, const char* name, bool relinquish);

/**
 * @brief Finds a resource in the vector resources.
 * @param[out] outType The type of the resource.
 * @param[out] outResource The pointer of the resource.
 * @param resources The vector resources.
 * @param name The name of the texture.
 * @return The texture or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_findResource(dsVectorResourceType* outType,
	void** outResource, const dsVectorResources* resources, const char* name);

/**
 * @brief Preloads glyphs for ASCII characters for all fonts inside of the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @return False if an error occurred.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_preloadASCII(
	dsVectorResources* resources, dsCommandBuffer* commandBuffer);

/**
 * @brief Destroys a vector resources object.
 *
 * Any owned resources will also be destroyed.
 *
 * @remark errno will be set on failure.
 * @param resources The resources to destroy.
 * @return False if the resources couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_destroy(dsVectorResources* resources);

#ifdef __cplusplus
}
#endif
