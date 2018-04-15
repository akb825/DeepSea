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
 * @brief Functions for creating vector shaders.
 * @see dsVectorShaders
 */

/**
 * @brief Creates the vector shader module from a file.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param filePath The file path for the shader module to load.
 * @param customElements Custom elements to add to the material description when using a custom
 *     shader module. This may be NULL for no custom elements.
 * @param customElementCount The number of custom elements to add.
 * @return The vector shaders, or NULL if it couldn't be loaded.
 */
DS_VECTORDRAW_EXPORT dsVectorShaderModule* dsVectorShaderModule_loadFile(
	dsResourceManager* resourceManager, dsAllocator* allocator, const char* filePath,
	dsMaterialElement* customElements, uint32_t customElementCount);

/**
 * @brief Creates the vector shader module from a stream.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param stream The stream to load the shader module from. This stream will be read from the
 *     current position until the end, and must be seekable.
 * @param customElements Custom elements to add to the material description when using a custom
 *     shader module. This may be NULL for no custom elements.
 * @param customElementCount The number of custom elements to add.
 * @return The vector shaders, or NULL if it couldn't be loaded.
 */
DS_VECTORDRAW_EXPORT dsVectorShaderModule* dsVectorShaderModule_loadStream(
	dsResourceManager* resourceManager, dsAllocator* allocator, dsStream* stream,
	dsMaterialElement* customElements, uint32_t customElementCount);

/**
 * @brief Creates the vector shader module from a data buffer.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param data The data for the shader module. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param customElements Custom elements to add to the material description when using a custom
 *     shader module. This may be NULL for no custom elements.
 * @param customElementCount The number of custom elements to add.
 * @return The vector shaders, or NULL if it couldn't be loaded.
 */
DS_VECTORDRAW_EXPORT dsVectorShaderModule* dsVectorShaderModule_loadData(
	dsResourceManager* resourceManager, dsAllocator* allocator, const void* data, size_t size,
	dsMaterialElement* customElements, uint32_t customElementCount);

/**
 * @brief Creates a material to be used when drawing a vector image with a shader module.
 * @remark errno will be set on failure.
 * @param shaderModule The shader module.
 * @param allocator The allocator to create the material with. If NULL, it will use the same
 *     allocator as the shader module.
 * @return The material, or NULL if it couldn't be created.
 */
DS_VECTORDRAW_EXPORT dsMaterial* dsVectorShaderModule_createMaterial(
	dsVectorShaderModule* shaderModule, dsAllocator* allocator);

/**
 * @brief Destroys the vector shaders.
 * @remark errno will be set on failure.
 * @param shaderModule The shader module to destroy.
 * @return False if the shaders couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorShaderModule_destroy(dsVectorShaderModule* shaderModule);

#ifdef __cplusplus
}
#endif
