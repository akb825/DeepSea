/*
 * Copyright 2017 Aaron Barany
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
 * @param maxTextures The maximum textures to store.
 * @param maxFaceGroups The maximum face groups to store.
 * @param maxFonts The maximum fonts to store.
 * @return The full allocation size.
 */
DS_VECTORDRAW_EXPORT size_t dsVectorResources_fullAllocSize(uint32_t maxTextures,
	uint32_t maxFaceGroups, uint32_t maxFonts);

/**
 * @brief Creates a vector resources object.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources with.
 * @param maxTextures The maximum textures to store.
 * @param maxFaceGroups The maximum face groups to store.
 * @param maxFonts The maximum fonts to store.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorReosurces_create(dsAllocator* allocator,
	uint32_t maxTextures, uint32_t maxFaceGroups, uint32_t maxFonts);

/**
 * @brief Loads vector resources from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param filePath The file path for the vector resources to load.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadFile(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsResourceManager* resourceManager, const char* filePath);

/**
 * @brief Loads vector resources from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param stream The stream to load the vector resources from. This stream will be read from the
 *     current position until the end.
 * @param loadUserData The user data to provide for the texture and face group load functions.
 * @param loadTextureFunc Function to perform loading of textures.
 * @param loadFontFaceFunc Function to perform loading of font faces.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsResourceManager* resourceManager, dsStream* stream,
	void* loadUserData, dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc);

/**
 * @brief Loads vector resources from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector resources and sub-resources with.
 * @param scratchAllocator The allocator to use for scratch data. If NULL, it will use the same as
 *     allocator.
 * @param resourceManager The resource manager to create textures from.
 * @param data The data for the vector resources. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param loadUserData The user data to provide for the texture and face group load functions.
 * @param loadTextureFunc Function to perform loading of textures.
 * @param loadFontFaceFunc Function to perform loading of font faces.
 * @return The created vector resources, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorResources* dsVectorResources_loadData(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsResourceManager* resourceManager, const void* data,
	size_t size, void* loadUserData, dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc);

/**
 * @brief Gets the number of remaining textures that can be set.
 * @param resources The vector resources.
 * @return The number of remaining textures.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorResources_getRemainingTextures(
	const dsVectorResources* resources);

/**
 * @brief Adds a texture to the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the texture. The length, including the null terminator, must not exceed
 *     DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param texture The texture to add. It is assumed that any alpha in the texture is pre-multiplied.
 * @param own True to take ownership of the texture.
 * @return False if the texture couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_addTexture(dsVectorResources* resources,
	const char* name, dsTexture* texture, bool own);

/**
 * @brief Removes a texture from the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the texture to remove.
 * @param relinquish True to relinquish ownership of the texture. This will prevent this from
 *     freeing the texture if it is owned.
 * @return False if the texture couldn't be removed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResource_removeTexture(dsVectorResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a texture in the vector resources.
 * @param resources The vector resources.
 * @param name The name of the texture.
 * @return The texture or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT dsTexture* dsVectorResources_findTexture(const dsVectorResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining faceGroups that can be set.
 * @param resources The vector resources.
 * @return The number of remaining faceGroups.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorResources_getRemainingFaceGroups(
	const dsVectorResources* resources);

/**
 * @brief Adds a face group to the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the face group. The length, including the null terminator, must not
 *     exceed DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param faceGroup The faceGroup to add.
 * @param own True to take ownership of the face group.
 * @return False if the face group couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_addFaceGroup(dsVectorResources* resources,
	const char* name, dsFaceGroup* faceGroup, bool own);

/**
 * @brief Removes a face group from the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the face group to remove.
 * @param relinquish True to relinquish ownership of the face group. This will prevent this from
 *     freeing the face group if it is owned.
 * @return False if the face group couldn't be removed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResource_removeFaceGroup(dsVectorResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a face group in the vector resources.
 * @param resources The vector resources.
 * @param name The name of the face group.
 * @return The faceGroup or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT dsFaceGroup* dsVectorResources_findFaceGroup(
	const dsVectorResources* resources, const char* name);

/**
 * @brief Gets the number of remaining fonts that can be set.
 * @param resources The vector resources.
 * @return The number of remaining fonts.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorResources_getRemainingFonts(
	const dsVectorResources* resources);

/**
 * @brief Adds a font to the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the font. The length, including the null terminator, must not exceed
 *     DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param font The font to add.
 * @param own True to take ownership of the font.
 * @return False if the font couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_addFont(dsVectorResources* resources,
	const char* name, dsFont* font, bool own);

/**
 * @brief Removes a font from the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the font to remove.
 * @param relinquish True to relinquish ownership of the font. This will prevent this from
 *     freeing the font if it is owned.
 * @return False if the font couldn't be removed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResource_removeFont(dsVectorResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a font in the vector resources.
 * @param resources The vector resources.
 * @param name The name of the font.
 * @return The font or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT dsFont* dsVectorResources_findFont(const dsVectorResources* resources,
	const char* name);

/**
 * @brief Gets the number of remaining fonts that can be set.
 * @param resources The vector resources.
 * @return The number of remaining fonts.
 */
DS_VECTORDRAW_EXPORT uint32_t dsVectorResources_getRemainingFonts(
	const dsVectorResources* resources);

/**
 * @brief Adds a font to the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the font. The length, including the null terminator, must not exceed
 *     DS_MAX_VECTOR_RESOURCE_NAME_LENGTH.
 * @param font The font to add.
 * @param own True to take ownership of the font.
 * @return False if the font couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResources_addFont(dsVectorResources* resources,
	const char* name, dsFont* font, bool own);

/**
 * @brief Removes a font from the vector resources.
 * @remark errno will be set on failure.
 * @param resources The vector resources.
 * @param name The name of the font to remove.
 * @param relinquish True to relinquish ownership of the font. This will prevent this from
 *     freeing the font if it is owned.
 * @return False if the font couldn't be removed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorResource_removeFont(dsVectorResources* resources,
	const char* name, bool relinquish);

/**
 * @brief Finds a font in the vector resources.
 * @param resources The vector resources.
 * @param name The name of the font.
 * @return The font or NULL if it couldn't be found.
 */
DS_VECTORDRAW_EXPORT dsFont* dsVectorResources_findFont(const dsVectorResources* resources,
	const char* name);

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
