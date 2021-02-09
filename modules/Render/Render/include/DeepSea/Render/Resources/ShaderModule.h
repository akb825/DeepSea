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
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating shader modules.
 *
 * Shader modules contain a list of shaders, which can then be used for rendering objects on screen.
 *
 * @see dsShaderModule
 */

/**
 * @brief Loads a shader module from a file.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param filePath The file path for the shader module to load.
 * @param name The name of the module. The name will be copied.
 * @return The created shader module, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadFile(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* filePath, const char* name);

/**
 * @brief Loads a shader module from a resource file.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param type The resource type.
 * @param filePath The file path for the shader module to load.
 * @param name The name of the module. This will be used for error checking and caching of shader
 *     binaries and should be unique. The lifetime of name should exceed the lifetime of the shader
 *     module. (such as a string constant)
 * @return The created shader module, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadResource(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsFileResourceType type, const char* filePath, const char* name);

/**
 * @brief Loads a shader module from a stream.
 *
 * The shader module is expected to have been created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param stream The stream to load the shader module from. This stream will be read from the
 *     current position until the end, and must be seekable.
 * @param name The name of the module. This will be used for error checking and caching of shader
 *     binaries and should be unique. The lifetime of name should exceed the lifetime of the shader
 *     module. (such as a string constant)
 * @return The created shader module, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream, const char* name);

/**
 * @brief Loads a shader module from a data buffer.
 *
 * The shader module is expected to have been created with ModuleShaderLanguage.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param data The data for the shader module. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param name The name of the module. This will be used for error checking and caching of shader
 *     binaries and should be unique. The lifetime of name should exceed the lifetime of the shader
 *     module. (such as a string constant)
 * @return The created shader module, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size, const char* name);

/**
 * @brief Gets the number of shaders within a module.
 * @param shaderModule The shader module.
 * @return The number of shader pipelines.
 */
DS_RENDER_EXPORT uint32_t dsShaderModule_shaderCount(const dsShaderModule* shaderModule);

/**
 * @brief Gets the name of a shader within a module.
 * @param shaderModule The shader module.
 * @param shader The index of the shader.
 * @return The name of the shader, or NULL if the parameters are invalid.
 */
DS_RENDER_EXPORT const char* dsShaderModule_shaderName(const dsShaderModule* shaderModule,
	uint32_t shader);

/**
 * @brief Gets the index of a shader within a module.
 * @param shaderModule The shader module.
 * @param name The name of the shader.
 * @return The index of the shader, or DS_MATERIAL_UNKNOWN if not present.
 */
DS_RENDER_EXPORT uint32_t dsShaderModule_shaderIndex(const dsShaderModule* shaderModule,
	const char* name);

/**
 * @brief Checks whether or not a shader in the module has a stage.
 * @param shaderModule The shader module.
 * @param name The name of the shader.
 * @param stage The stage to check.
 * @return True if the shader has the stage.
 */
DS_RENDER_EXPORT bool dsShaderModule_shaderNameHasStage(const dsShaderModule* shaderModule,
	const char* name, dsShaderStage stage);

/**
 * @brief Checks whether or not a shader in the module has a stage.
 * @param shaderModule The shader module.
 * @param index The index of the shader.
 * @param stage The stage to check.
 * @return True if the shader has the stage.
 */
DS_RENDER_EXPORT bool dsShaderModule_shaderIndexHasStage(const dsShaderModule* shaderModule,
	uint32_t index, dsShaderStage stage);

/**
 * @brief Destroys a shader module.
 * @remark errno will be set on failure.
 * @param shaderModule The shader module to destroy.
 * @return False if the shader module couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderModule_destroy(dsShaderModule* shaderModule);

#ifdef __cplusplus
}
#endif
