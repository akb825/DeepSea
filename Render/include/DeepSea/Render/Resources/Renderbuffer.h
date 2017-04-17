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
 * @brief Functions for creating and using renderbuffers.
 *
 * Renderbuffers can be rendered to similar to render surfaces and offscreens. They can be shared
 * across different framebuffers and may be used when access isn't required from a shader.
 *
 * @see dsRenderbuffer
 */

/**
 * @brief Creates a renderbuffer.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the renderbuffer from. If NULL, it will use
 *     the same allocator as the resource manager.
 * @param allocator The allocator to create the renderbuffer with.
 * @param format The format of the renderbuffer. This format must be compatible with offscreens.
 * @param width The width of the renderbuffer.
 * @param height The height of the renderbuffer.
 * @param samples The number of samples to use for multisampling. This may be set to
 *     DS_DEFAULT_ANTIALIAS_SAMPLES to use the default set on the renderer. The renderbuffer will
 *     need to be re-created by the caller if the default changes.
 * @return The created renderbuffer, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsRenderbuffer* dsRenderbuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxFormat format, uint32_t width, uint32_t height, uint16_t samples);

/**
 * @brief Destroys a renderbuffer.
 * @remark errno will be set on failure.
 * @param renderbuffer The renderbuffer to destroy.
 * @return False if the renderbuffer couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsRenderbuffer_destroy(dsRenderbuffer* renderbuffer);


#ifdef __cplusplus
}
#endif
