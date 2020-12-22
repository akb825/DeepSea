/*
 * Copyright 2017-2021 Aaron Barany
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
 * @return The created shader, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShader* dsShader_createName(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, const char* name,
	const dsMaterialDesc* materialDesc);

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
 * @return The created shader, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShader* dsShader_createIndex(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, uint32_t index,
	const dsMaterialDesc* materialDesc);

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
 * @param globalValues The global values to apply to the shader. This may be NULL if the
 *     material description doesn't use global values.
 * @param renderStates The dynamic render states to apply. This may be NULL to use the default
 *     values.
 * @return False if the values couldn't be bound.
 */
DS_RENDER_EXPORT bool dsShader_bind(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Updates the instance material values for the shader.
 *
 * This will try to only update the values that have changed.
 *
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param instanceValues The instance values to updte.
 * @return False if the values couldn't be updated.
 */
DS_RENDER_EXPORT bool dsShader_updateInstanceValues(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* instanceValues);

/**
 * @brief Updates the dynamic render states.
 * @remark This must be called inside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the render states for.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param renderStates The dynamic render states to apply.
 * @return False if the render states couldn't be set.
 */
DS_RENDER_EXPORT bool dsShader_updateDynamicRenderStates(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsDynamicRenderStates* renderStates);

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
 * @param globalValues The global values to apply to the shader. This may be NULL if the
 *     material description doesn't use global values.
 * @return False if the values couldn't be bound.
 */
DS_RENDER_EXPORT bool dsShader_bindCompute(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues);

/**
 * @brief Updates the instance material values for the compute shader.
 *
 * This will try to only update the values that have changed.
 *
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param shader The shader to update the values on.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param instanceValues The instance values to updte.
 * @return False if the values couldn't be updated.
 */
DS_RENDER_EXPORT bool dsShader_updateComputeInstanceValues(const dsShader* shader,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* instanceValues);

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
 * @brief Destroys a shader.
 * @remark errno will be set on failure.
 * @param shader The shader to destroy.
 * @return False if the shader couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShader_destroy(dsShader* shader);

/**
 * @brief Prepares the shader cache directory.
 *
 * This will print an error message on failure. It will avoid re-printing error messages until a
 * different cache directory is passed in.
 *
 * @remark errno will be set on failure.
 * @remark This is primarily used for the different renderer implementations.
 * @param cacheDir The shader cache directory.
 * @return True if the cache directory can be used.
 */
DS_RENDER_EXPORT bool dsShader_prepareCacheDirectory(const char* cacheDir);

/**
 * @brief Gets the file name for a cached shader.
 * @remark errno will be set on failure.
 * @remark This is primarily used for the different renderer implementations.
 * @param[out] result The output file name.
 * @param resultSize The size of the buffer for the result.
 * @param shader The shader to get the file name for.
 * @param cacheDir The shader cache directory.
 * @param extension The file extension, including the '.'
 * @return False if an error occurred.
 */
DS_RENDER_EXPORT bool dsShader_cacheFileName(char* result, size_t resultSize,
	const dsShader* shader, const char* cacheDir, const char* extension);

#ifdef __cplusplus
}
#endif
