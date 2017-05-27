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
#include <DeepSea/RenderOpenGL/Export.h>
#include <DeepSea/RenderOpenGL/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and destroying an OpenGL renderer.
 */

/**
 * @brief Initializes the dsOpenGLOptions structure with the default options.
 * @param options The options to initialize.
 */
DS_RENDEROPENGL_EXPORT void dsGLRenderer_defaultOptions(dsOpenGLOptions* options);

/**
 * @brief Creates an OpenGL renderer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use.
 * @param options The options to initialize OpenGL with.
 * @return The created renderer, or NULL if the renderer coulnd't be created.
 */
DS_RENDEROPENGL_EXPORT dsRenderer* dsGLRenderer_create(dsAllocator* allocator,
	const dsOpenGLOptions* options);

/**
 * @brief Sets whether or not to enable error checking.
 *
 * When enabled, errors will be aggressively checked at the cost of performance. This won't have any
 * effect in release builds.
 *
 * @remark This shouldn't be called when other threads might be processing graphics resources.
 * @param renderer The renderer.
 * @param enabled True to enable error checking, false to disable it.
 */
DS_RENDEROPENGL_EXPORT void dsGLRenderer_setEnableErrorChecking(dsRenderer* renderer,
	bool enabled);

/**
 * @brief Destroys an OpenGL renderer.
 * @param renderer The renderer to destroy.
 */
DS_RENDEROPENGL_EXPORT void dsGLRenderer_destroy(dsRenderer* renderer);

#ifdef __cplusplus
}
#endif
