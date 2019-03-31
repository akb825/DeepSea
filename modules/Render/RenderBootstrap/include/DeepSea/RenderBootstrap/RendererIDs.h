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

#if DS_HAS_RENDER_OPENGL
#include <DeepSea/RenderOpenGL/RendererIDs.h>
#endif

#if DS_HAS_RENDER_VULKAN
#include <DeepSea/RenderVulkan/RendererIDs.h>
#endif

#if DS_HAS_RENDER_METAL
#include <DeepSea/RenderMetal/RendererIDs.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief File containing the renderer IDs for all standard renderers.
 *
 * The most common usage for these IDs is for determining what shader versiont to use with
 * dsRenderer_chooseShaderVersion(). They may also be used for renderer-specific code.
 */

// Define the IDs here for libraries that aren't compiled in.
#if !DS_HAS_RENDER_OPENGL
/**
 * @brief Constant for the renderer ID of the OpenGL renderer using desktop OpenGL.
 */
#define DS_GL_RENDERER_ID DS_FOURCC('G', 'L', 0, 0)

/**
 * @brief Constant for the renderer ID of the OpenGL renderer using OpenGL ES.
 */
#define DS_GLES_RENDERER_ID DS_FOURCC('G', 'L', 'E', 'S')

/**
 * @brief Constant for the renderer platform ID of EGL.
 */
#define DS_EGL_RENDERER_PLATFORM_ID DS_FOURCC('E', 'G', 'L', 0)

/**
 * @brief Constant for the renderer platform ID of GLX.
 */
#define DS_GLX_RENDERER_PLATFORM_ID DS_FOURCC('G', 'L', 'X', 0)

/**
 * @brief Constant for the renderer platform ID of WGL.
 */
#define DS_WGL_RENDERER_PLATFORM_ID DS_FOURCC('W', 'G', 'L', 0)
#endif

#if !DS_HAS_RENDER_VULKAN
/**
 * @brief Log tag for the RenderVulkan library.
 */
#define DS_RENDER_VULKAN_LOG_TAG "vulkan"

/**
 * @brief Constant for the renderer type ID of the Vulkan renderer.
 */
#define DS_VK_RENDERER_ID DS_FOURCC('V', 'K', 0, 0)

/**
 * @brief Constant for the renderer platform type ID of xlib.
 */
#define DS_VK_XLIB_RENDERER_PLATFORM_ID DS_FOURCC('X', 'L', 'I', 'B')

/**
 * @brief Constant for the renderer platform type ID of Win32.
 */
#define DS_VK_WIN32_RENDERER_PLATFORM_ID DS_FOURCC('W', 'I', 'N', 0)
#endif

#if !DS_HAS_RENDER_METAL
/**
 * @brief Log tag for the RenderMetal library.
 */
#define DS_RENDER_METAL_LOG_TAG "metal"

/**
 * @brief Constant for the renderer type ID of the Metal renderer.
 */
#define DS_MTL_RENDERER_ID DS_FOURCC('M', 'T', 'L', 0)
#endif

#ifdef __cplusplus
}
#endif
