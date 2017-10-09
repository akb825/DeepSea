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
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Types used for the public interface to the RenderOpenGL library.
 */

/**
 * @brief Log tag for the RenderOpenGL library.
 */
#define DS_RENDER_OPENGL_LOG_TAG "opengl"

/**
 * @brief Constant for the renderer type ID of the OpenGL renderer using desktop OpenGL.
 */
#define DS_GL_RENDERER_TYPE DS_FOURCC('G', 'L', 0, 0)

/**
 * @brief Constant for the renderer type ID of the OpenGL renderer using OpenGL ES.
 */
#define DS_GLES_RENDERER_TYPE DS_FOURCC('G', 'L', 'E', 'S')

/**
 * @brief Struct containing the otpions for initializing OpenGL.
 */
typedef struct dsOpenGLOptions
{
	/**
	 * @brief The platform display.
	 */
	void* display;

	/**
	 * @brief The number of bits for the red channel.
	 */
	uint8_t redBits;

	/**
	 * @brief The number of bits for the green channel.
	 */
	uint8_t greenBits;

	/**
	 * @brief The number of bits for the blue channel.
	 */
	uint8_t blueBits;

	/**
	 * @brief The number of bits for the alpha channel.
	 */
	uint8_t alphaBits;

	/**
	 * @brief The number of bits for the depth buffer.
	 */
	uint8_t depthBits;

	/**
	 * @brief The number of bits for the stencil buffer.
	 */
	uint8_t stencilBits;

	/**
	 * @brief The default number of anti-alias samples.
	 *
	 * This may be changed later, but all surfaces must be re-created. It will be clamped to the
	 * maximum number of supported samples.
	 */
	uint8_t samples;

	/**
	 * @brief True to double-buffer rendering, false to single-buffer.
	 */
	bool doubleBuffer;

	/**
	 * @brief True to use sRGB, false to use linear color space.
	 */
	bool srgb;

	/**
	 * @brief True to use stereoscopic rendering, false to use a single surface.
	 */
	bool stereoscopic;

	/**
	 * @brief Whether or not to use an accelerated target.
	 *
	 * Set to true to force accelerated, false to force software, or -1 to not care.
	 */
	int8_t accelerated;

	/**
	 * @brief True to enable debugging.
	 */
	bool debug;

	/**
	 * @brief The maximum number of resource threads.
	 */
	uint8_t maxResourceThreads;

	/**
	 * @brief Directory to cache shader binaries.
	 *
	 * This will be copied when the renderer is created so it need not be permanently allocated.
	 */
	const char* shaderCacheDir;
} dsOpenGLOptions;

#ifdef __cplusplus
}
#endif
