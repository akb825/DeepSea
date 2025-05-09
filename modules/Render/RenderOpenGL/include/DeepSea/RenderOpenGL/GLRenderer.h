/*
 * Copyright 2017-2025 Aaron Barany
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
#include <DeepSea/RenderOpenGL/Export.h>
#include <DeepSea/RenderOpenGL/RendererIDs.h>
#include <DeepSea/Render/Renderer.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating an OpenGL renderer.
 */

/**
 * @brief Whether OpenGL ES will be used.
 */
DS_RENDEROPENGL_EXPORT extern const bool dsGLRenderer_isGLES;

/**
 * @brief Gets whether or not the OpenGL renderer is supported.
 * @return True if supported.
 */
DS_RENDEROPENGL_EXPORT bool dsGLRenderer_isSupported(void);

/**
 * @brief Queries the devices available for use.
 *
 * This will always return an empty set of devices.
 *
 * @remark errno will be set on failure.
 * @param[out] outDevices Output pointer for the devices.
 * @param[inout] outDeviceCount The number of devices. This will be set to 0.
 * @return False if the parameters are invalid.
 */
DS_RENDEROPENGL_EXPORT bool dsGLRenderer_queryDevices(dsRenderDeviceInfo* outDevices,
	uint32_t* outDeviceCount);

/**
 * @brief Creates an OpenGL renderer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use.
 * @param options The options to initialize OpenGL with.
 * @return The created renderer, or NULL if the renderer coulnd't be created.
 */
DS_RENDEROPENGL_EXPORT dsRenderer* dsGLRenderer_create(dsAllocator* allocator,
	const dsRendererOptions* options);

#ifdef __cplusplus
}
#endif
