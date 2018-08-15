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
 * @brief Functions for creating and using framebuffers.
 *
 * Framebuffers contain a list of surfaces (render surfaces, offscreens, and renderbuffers) that
 * would be rendered to within a render pass. A render pass may contain multiple subpasses, so not
 * all render targets are necessarily drawn to at once.
 *
 * @see dsFramebuffer
 */

/**
 * @brief Creates a framebuffer.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the framebuffer from.
 * @param allocator The allocator to create the framebuffer with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param name The name of the framebuffer, used for profiling info. This should remain allocated
 *     for the duration of the application, such as a string constant.
 * @param surfaces The surfaces that make up the framebuffer. The surfaces must match the dimensions
 *     of the framebuffer.
 * @param surfaceCount The number of surfaces.
 * @param width The width of the framebuffer.
 * @param height The height of the framebuffer.
 * @param layers The number of image layers in the framebuffer. This can be array layers, cube map
 *     images, or a combination of both. This must either be 1 or cover all of the layers within the
 *     surfaces.
 * @return The created framebuffer, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsFramebuffer* dsFramebuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* name, const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount, uint32_t width, uint32_t height, uint32_t layers);

/**
 * @brief Destroys a framebuffer.
 * @remark errno will be set on failure.
 * @param framebuffer The framebuffer to destroy.
 * @return False if the framebuffer couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsFramebuffer_destroy(dsFramebuffer* framebuffer);

#ifdef __cplusplus
}
#endif
