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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief File containing the renderer IDs for OpenGL.
 */

/**
 * @brief Log tag for the RenderOpenGL library.
 */
#define DS_RENDER_OPENGL_LOG_TAG "opengl"

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

#ifdef __cplusplus
}
#endif
