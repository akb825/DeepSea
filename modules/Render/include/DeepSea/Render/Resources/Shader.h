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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating shaders.
 *
 * Shaders are created from a shader modules after they have been loaded.
 *
 * @see dsShader
 */

/**
 * @brief Creates a shader by name.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with. If NULL, it will use the same allocator
 *     as the resource manager.
 * @param shaderModule The shader module to create the shader from. It must remain alive for at
 *     least as long as the shader.
 * @param name The name of the shader within the shader module.
 * @param materialDesc The description of the material that will be used with the shader. It must
 *     remain alive for at least as long as the shader.
 * @param primitiveType The type of primitive the shader will be drawn with.
 * @param samples The number of samples to use for multisampling. This may be set to
 *     DS_DEFAULT_ANTIALIAS_SAMPLES to use the default set on the renderer. Unlike offscreens and
 *     renderbuffers, the shader need not be re-created when the renderer's anti-alias samples field
 *     is changed.
 * @return The created shader, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShader* dsShader_createName(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, const char* name,
	const dsMaterialDesc* materialDesc, dsPrimitiveType primitiveType, uint32_t samples);

/**
 * @brief Creates a shader by index.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with. If NULL, it will use the same allocator
 *     as the resource manager.
 * @param shaderModule The shader module to create the shader from. It must remain alive for at
 *     least as long as the shader.
 * @param index The index of the shader within the shader module.
 * @param materialDesc The description of the material that will be used with the shader. It must
 *     remain alive for at least as long as the shader.
 * @param primitiveType The type of primitive the shader will be drawn with.
 * @param samples The number of samples to use for multisampling. This may be set to
 *     DS_DEFAULT_ANTIALIAS_SAMPLES to use the default set on the renderer. Unlike offscreens and
 *     renderbuffers, the shader need not be re-created when the renderer samplers are changed.
 * @return The created shader, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShader* dsShader_createIndex(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, uint32_t index,
	const dsMaterialDesc* materialDesc, dsPrimitiveType primitiveType, uint32_t samples);

/**
 * @brief Checks whether or not a shader has a specific pipeline stage.
 * @param shader The shader.
 * @param stage The stage to check.
 * @return True if shader contains the stage.
 */
DS_RENDER_EXPORT bool dsShader_hasStage(const dsShader* shader, dsShaderStage stage);

/**
 * @brief Binds a shader for drawing.
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to draw with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param material The material values to apply to the shader.
 * @param volatileValues The volatile values to apply to the shader. This may be NULL if the
 *     material description doesn't use volatile values.
 * @param renderStates The dynamic render states to apply. This may be NULL to use the default
 *     values.
 * @return False if the values couldn't be bound.
 */
DS_RENDER_EXPORT bool dsShader_bind(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsVolatileMaterialValues* volatileValues,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Updates the volatile material values for the shader.
 *
 * This will try to only update the values that have changed.
 *
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param volatileValues The volatile values to updte.
 * @return False if the values couldn't be updated.
 */
DS_RENDER_EXPORT bool dsShader_updateVolatileValues(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsVolatileMaterialValues* volatileValues);

/**
 * @brief Un-binds a shader that was previously bound.
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if the values couldn't be unbound.
 */
DS_RENDER_EXPORT bool dsShader_unbind(const dsShader* shader, dsCommandBuffer* commandBuffer);

/**
 * @brief Binds a shader for compute.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to draw with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param material The material values to apply to the shader.
 * @param volatileValues The volatile values to apply to the shader. This may be NULL if the
 *     material description doesn't use volatile values.
 * @return False if the values couldn't be bound.
 */
DS_RENDER_EXPORT bool dsShader_bindCompute(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsVolatileMaterialValues* volatileValues);

/**
 * @brief Updates the volatile material values for the compute shader.
 *
 * This will try to only update the values that have changed.
 *
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param volatileValues The volatile values to updte.
 * @return False if the values couldn't be updated.
 */
DS_RENDER_EXPORT bool dsShader_updateComputeVolatileValues(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsVolatileMaterialValues* volatileValues);

/**
 * @brief Un-binds a compute shader that was previously bound.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if the values couldn't be unbound.
 */
DS_RENDER_EXPORT bool dsShader_unbindCompute(const dsShader* shader,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Updates the volatile material values for the shader.
 *
 * This will try to only update the values that have changed. This will work both with either
 *
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param volatileValues The volatile values to updte.
 * @return False if the values couldn't be updated.
 */
DS_RENDER_EXPORT bool dsShader_updateVolatileValues(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsVolatileMaterialValues* volatileValues);

/**
 * @brief Destroys a shader.
 * @remark errno will be set on failure.
 * @param shader The shader to destroy.
 * @return False if the shader couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShader_destroy(dsShader* shader);

#ifdef __cplusplus
}
#endif
