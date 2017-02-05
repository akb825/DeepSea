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
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with. If NULL, it will use the same allocator
 *     as the resource manager.
 * @param shaderModule The shader module to create the shader from. It must remain alive for at
 *     least as long as the shader.
 * @param name The name of the shader within the shader module.
 * @param materialDesc The description of the material that will be used with the shader. It must
 *     remain alive for at least as long as the shader.
 * @return The created shader , or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsShader* dsShader_createName(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, const char* name,
	const dsMaterialDesc* materialDesc);

/**
 * @brief Creates a shader by index.
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with. If NULL, it will use the same allocator
 *     as the resource manager.
 * @param shaderModule The shader module to create the shader from. It must remain alive for at
 *     least as long as the shader.
 * @param index The index of the shader within the shader module.
 * @param materialDesc The description of the material that will be used with the shader. It must
 *     remain alive for at least as long as the shader.
 * @return The created shader , or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsShader* dsShader_createIndex(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* shaderModule, uint32_t index,
	const dsMaterialDesc* materialDesc);

/**
 * @brief Destroys a shader.
 * @param shader The shader to destroy.
 * @return False if the shader couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShader_destroy(dsShader* shader);

#ifdef __cplusplus
}
#endif
