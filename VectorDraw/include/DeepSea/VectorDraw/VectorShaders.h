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
 * @brief Instantiates the shaders required to draw vector graphics from a shader module.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the shaders from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param shaderModule The shader module to create the shaders with.
 * @param samples The number of anti-alias samples, or DS_DEFAULT_ANTIALIAS_SAMPLES for the default.
 * @return The vector shaders, or NULL if it couldn't be loaded.
 */
DS_VECTORDRAW_EXPORT dsVectorShaders* dsVectorShaders_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule, uint32_t samples);

/**
 * @brief Destroys the vector shaders.
 * @remark errno will be set on failure.
 * @param shaders The shaders to destroy.
 * @return False if the shaders couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorShaders_destroy(dsVectorShaders* shaders);

#ifdef __cplusplus
}
#endif
