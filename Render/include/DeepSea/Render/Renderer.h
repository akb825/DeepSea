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
#include <DeepSea/Render/Export.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with a renderer.
 *
 * Unlike most types, this doesn't contain any functions for creating or destroying a renderer.
 * This is handled by the renderer implementation libraries.
 *
 * @see dsRenderer
 */

/**
 * @brief Function for queueing a memcpy on the command buffer.
 *
 * The main purpose of this is to handle updates of shader variables on the command queue. If shader
 * uniform blocks are supported, this may not be implemented.
 *
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to queue the memcpy on.
 * @param dest The buffer to copy the data to.
 * @param src The buffer to copy the data from. If not executed immediately, this should be copied
 *     to the command buffer.
 * @param size The size to copy.
 * @return False on failure.
 */
DS_RENDER_EXPORT bool dsRenderer_queueMemcpy(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	void* dest, const void* src, size_t size);

#ifdef __cplusplus
}
#endif
