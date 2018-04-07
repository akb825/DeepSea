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
 * @brief Functions for creating and manipulating vector images.
 * @see dsVectorImage
 */

/**
 * @brief Creates a vector image.
 * @remark errno will be set on failure.
 * @remark At least one of sharedMaterials and localMaterials must be set. If both are set,
 *     sharedMaterials is searched first.
 * @param allocator The allocator to create the image with.
 * @param scratchData The scratch data to use when building up the image. This may be re-used across
 *     multiple images, so long as it isn't used concurrently across multiple threads.
 * @param resourceManager The resource manaer to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the vector image allocator.
 * @param commands The commands that comprise the image.
 * @param commandCount The number of commands.
 * @param sharedMaterials The material set that is shared with other materials.
 * @param localMaterials The material set that is local to this image only. This will take ownership
 *     of the material set.
 * @param shaderModule The shader module for the vector shaders.
 * @param size The size of the vector image.
 * @param pixelSize The size of a pixel, determining tessellation quality.
 * @return The created image, or NULL if the image couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorImage_create(dsAllocator* allocator,
	dsVectorScratchData* scratchData, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const dsVectorCommand* commands, uint32_t commandCount,
	const dsVectorMaterialSet* sharedMaterials, dsVectorMaterialSet* localMaterials,
	dsVectorShaderModule* shaderModule, const dsVector2f* size, float pixelSize);

/**
 * @brief Loads a vector image from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector image.
 * @param scratchData The scratch data to use when building up the image. This may be re-used across
 *     multiple images, so long as it isn't used concurrently across multiple threads.
 * @param resourceManager The resource manaer to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the vector image allocator.
 * @param filePath The file path for the vector image to load.
 * @param sharedMaterials The material set that is shared with other materials.
 * @param shaderModule The shader module for the vector shaders.
 * @param resources The vector resources.
 * @param resourceCount The number of vector resources.
 * @param pixelSize The size of a pixel, determining tessellation quality.
 * @param targetSize The target size of the image. If not NULL, it will be used in place of the real
 *     image size for calculating the tessellation quality.
 * @param srgb True if the materials loaded with the vector image should be interpreted as sRGB.
 * @return The created vector image, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorImage_loadFile(dsAllocator* allocator,
	dsVectorScratchData* scratchData, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const char* filePath,
	const dsVectorMaterialSet* sharedMaterials, dsVectorShaderModule* shaderModule,
	const dsVectorResources** resources, uint32_t resourceCount, float pixelSize,
	const dsVector2f* targetSize, bool srgb);

/**
 * @brief Loads a vector image from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector image.
 * @param scratchData The scratch data to use when building up the image. This may be re-used across
 *     multiple images, so long as it isn't used concurrently across multiple threads.
 * @param resourceManager The resource manaer to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the vector image allocator.
 * @param stream The stream to load the vector image from. This stream will be read from the
 *     current position until the end.
 * @param sharedMaterials The material set that is shared with other materials.
 * @param shaderModule The shader module for the vector shaders.
 * @param resources The vector resources.
 * @param resourceCount The number of vector resources.
 * @param pixelSize The size of a pixel, determining tessellation quality.
 * @param targetSize The target size of the image. If not NULL, it will be used in place of the real
 *     image size for calculating the tessellation quality.
 * @param srgb True if the materials loaded with the vector image should be interpreted as sRGB.
 * @return The created vector image, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorImage_loadStream(dsAllocator* allocator,
	dsVectorScratchData* scratchData, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, dsStream* stream, const dsVectorMaterialSet* sharedMaterials,
	dsVectorShaderModule* shaderModule, const dsVectorResources** resources,
	uint32_t resourceCount, float pixelSize, const dsVector2f* targetSize, bool srgb);

/**
 * @brief Loads a vector image from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector image.
 * @param scratchData The scratch data to use when building up the image. This may be re-used across
 *     multiple images, so long as it isn't used concurrently across multiple threads.
 * @param resourceManager The resource manaer to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the vector image allocator.
 * @param data The data for the vector resources. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param sharedMaterials The material set that is shared with other materials.
 * @param shaderModule The shader module for the vector shaders.
 * @param resources The vector resources.
 * @param resourceCount The number of vector resources.
 * @param pixelSize The size of a pixel, determining tessellation quality.
 * @param targetSize The target size of the image. If not NULL, it will be used in place of the real
 *     image size for calculating the tessellation quality.
 * @param srgb True if the materials loaded with the vector image should be interpreted as sRGB.
 * @return The created vector image, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorImage_loadData(dsAllocator* allocator,
	dsVectorScratchData* scratchData, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const void* data, size_t size,
	const dsVectorMaterialSet* sharedMaterials, dsVectorShaderModule* shaderModule,
	const dsVectorResources** resources, uint32_t resourceCount, float pixelSize,
	const dsVector2f* targetSize, bool srgb);

/**
 * @brief Draws a vector image.
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param vectorImage The vector image to draw.
 * @param commandBuffer The command buffer to put the draw commands on.
 * @param shaders The shaders to draw with.
 * @param material The material created from the shader module.
 * @param modelViewProjection The model/view/projection materis for this image.
 * @param volatileValues The volatile material values. This is only required if custom shaders are
 *     used that require them.
 * @param renderStates The dynamic render states. This is only required if custom shaders are used
 *     that require them.
 * @return False if an error occurred.
 */
DS_VECTORDRAW_EXPORT bool dsVectorImage_draw(const dsVectorImage* vectorImage,
	dsCommandBuffer* commandBuffer, const dsVectorShaders* shaders, dsMaterial* material,
	const dsMatrix44f* modelViewProjection, const dsVolatileMaterialValues* volatileValues,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Gets the shared materials used by the vector image.
 * @param vectorImage The vector image.
 * @return The shared materials.
 */
DS_VECTORDRAW_EXPORT const dsVectorMaterialSet* dsVectorImage_getSharedMaterials(
	const dsVectorImage* vectorImage);

/**
 * @brief Gets the local materials used by the vector image.
 * @param vectorImage The vector image.
 * @return The local materials.
 */
DS_VECTORDRAW_EXPORT dsVectorMaterialSet* dsVectorImage_getLocalMaterials(
	dsVectorImage* vectorImage);

/**
 * @brief Destroys a vector image.
 * @remark errno will be set on failure.
 * @param vectorImage The vector image.
 * @return False if the resources couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorImage_destroy(dsVectorImage* vectorImage);

#ifdef __cplusplus
}
#endif
