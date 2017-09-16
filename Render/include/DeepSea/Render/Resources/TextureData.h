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
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with texture data.
 *
 * Texture data may be created directly or loaded from file.
 *
 * @see dsTextureData
 */

/**
 * @brief Creates texture data.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the texture data with.
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTextureData* dsTextureData_create(dsAllocator* allocator, dsGfxFormat format,
	dsTextureDim dimension, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels);

/**
 * @brief Creates a texture from texture data.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param allocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param textureData The texture data to create the texture from.
 * @param flags Flags for creating the texture from the data. This should be a combination of
 *     dsTextureDataFlags flags.
 * @param options Options for converting the texture data to a texture. If NULL, the texture will be
 *     created from the data without any modifications.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @return The created texture, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTextureData_createTexture(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsTextureData* textureData, const dsTextureDataOptions* options,
	int usage, int memoryHints);

/**
 * @brief Loads a DDS texture file to a new texture data instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the texture data with.
 * @param filePath The file to load.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTextureData* dsTextureData_loadDdsFile(dsAllocator* allocator,
	const char* filePath);

/**
 * @brief Loads a DDS texture file from a stream to a new texture data instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the texture data with.
 * @param stream The file to load the texture from.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTextureData* dsTextureData_loadDdsStream(dsAllocator* allocator,
	dsStream* stream);

/**
 * @brief Loads a DDS texture file to a new texture instance.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param textureAllocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param tempAllocator The allocator to use for temporary memory.  If NULL, it will use the same
 *     allocator as the texture.
 * @param filePath The file to load.
 * @param options Options for converting the texture data to a texture. If NULL, the texture will be
 *     created from the data without any modifications.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTextureData_loadDdsFileToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, const char* filePath,
	const dsTextureDataOptions* options, int usage, int memoryHints);

/**
 * @brief Loads a DDS texture file from a stream to a new texture instance.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param textureAllocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param tempAllocator The allocator to use for temporary memory.  If NULL, it will use the same
 *     allocator as the texture.
 * @param stream The file to load the texture from.
 * @param options Options for converting the texture data to a texture. If NULL, the texture will be
 *     created from the data without any modifications.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTextureData_loadDdsStreamToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsStream* stream,
	const dsTextureDataOptions* options, int usage, int memoryHints);

/**
 * @brief Loads a PVR texture file to a new texture data instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the texture data with.
 * @param filePath The file to load.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTextureData* dsTextureData_loadPvrFile(dsAllocator* allocator,
	const char* filePath);

/**
 * @brief Loads a PVR texture file from a stream to a new texture data instance.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the texture data with.
 * @param stream The file to load the texture from.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTextureData* dsTextureData_loadPvrStream(dsAllocator* allocator,
	dsStream* stream);

/**
 * @brief Loads a PVR texture file to a new texture instance.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param textureAllocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param tempAllocator The allocator to use for temporary memory.  If NULL, it will use the same
 *     allocator as the texture.
 * @param filePath The file to load.
 * @param options Options for converting the texture data to a texture. If NULL, the texture will be
 *     created from the data without any modifications.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTextureData_loadPvrFileToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, const char* filePath,
	const dsTextureDataOptions* options, int usage, int memoryHints);

/**
 * @brief Loads a PVR texture file from a stream to a new texture instance.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param textureAllocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param tempAllocator The allocator to use for temporary memory.  If NULL, it will use the same
 *     allocator as the texture.
 * @param stream The file to load the texture from.
 * @param options Options for converting the texture data to a texture. If NULL, the texture will be
 *     created from the data without any modifications.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @return The created texture data, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTextureData_loadPvrStreamToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsStream* stream,
	const dsTextureDataOptions* options, int usage, int memoryHints);

/**
 * @brief Destroys a texture data.
 * @param textureData The texture data to destroy.
 */
DS_RENDER_EXPORT void dsTextureData_destroy(dsTextureData* textureData);

#ifdef __cplusplus
}
#endif
