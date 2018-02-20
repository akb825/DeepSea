/*
 * Copyright 2018 Aaron Barany
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
 * @brief Functions for creating vector draw contexts.
 *
 * In most cases, the draw context merely needs to be created and passed to dsVectorImage_draw().
 * When multiple threads are used, each thread should contain its own draw context. When custom
 * material elements are used, the values should be set on the internal material or volatile
 * material values before calling dsVectorImage_draw().
 *
 * @see dsVectorDrawContext
 */

/**
 * @brief Creates a vector draw context.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the material elements from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param shaderModule The shader module to create the context information for.
 * @return The vector draw context, or NULL an error occurred.
 */
DS_VECTORDRAW_EXPORT dsVectorDrawContext* dsVectorDrawContext_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule);

/**
 * @brief Destroys a vector draw context.
 * @remark errno will be set on failure.
 * @param context The vector draw context to destroy.
 * @return False if the draw context couldn't be destroyed.
 */
DS_VECTORDRAW_EXPORT bool dsVectorDrawContext_destroy(dsVectorDrawContext* context);

#ifdef __cplusplus
}
#endif
