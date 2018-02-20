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
 * @param allocator The allocator to create the image with.
 * @param scratchData The scratch data to use when building up the image. This may be re-used across
 *     multiple images, so long as it isn't used concurrently across multiple threads.
 * @param resourceManager The resource manaer to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the vector image allocator.
 * @param commands The commands that comprise the image.
 * @param commandCount The number of commands.
 * @param materials The material set to create the vector image with.
 * @param ownMaterials True to take ownership of the material set.
 * @param shaderModule The shader module for the vector shaders.
 * @param size The size of the vector image.
 * @param pixelSize The size of a pixel, determining tessellation quality.
 * @return The created image, or NULL if the image couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorImage_create(dsAllocator* allocator,
	dsVectorScratchData* scratchData, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const dsVectorCommand* commands, uint32_t commandCount,
	dsVectorMaterialSet* materials, bool ownMaterials, dsVectorShaderModule* shaderModule,
	const dsVector2f* size, float pixelSize);

/**
 * @brief Draws a vector image.
 * @remark errno will be set on failure.
 * @param vectorImage The vector image to draw.
 * @param commandBuffer The command buffer to put the draw commands on.
 * @param shaders The shaders to draw with.
 * @param drawContext The draw context for the current session.
 * @param modelViewProjection The model/view/projection materis for this image.
 * @param volatileValues The volatile material values. This is only required if custom shaders are
 *     used that require them.
 * @param renderStates The dynamic render states. This is only required if custom shaders are used
 *     that require them.
 * @return False if an error occurred.
 */
DS_VECTORDRAW_EXPORT bool dsVectorImage_draw(const dsVectorImage* vectorImage,
	dsCommandBuffer* commandBuffer, const dsVectorShaders* shaders,
	dsVectorDrawContext* drawContext, const dsMatrix44f* modelViewProjection,
	const dsVolatileMaterialValues* volatileValues, const dsDynamicRenderStates* renderStates);

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
