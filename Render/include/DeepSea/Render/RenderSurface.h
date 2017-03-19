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
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and using render surfaces.
 *
 * Render surfaces are the final target for drawing contents to, such as a window. These functions
 * create and manage the renderable surface with the OS surface. It is dependent on the platform
 * whether a generic OS handle (such as a window handle) can be used or if setup is required
 * beforehand to make it compatible with drawing.
 *
 * @see dsRenderSurface
 */

/**
 * @brief Creates a render surface.
 * @remark errno will be set on failure.
 * @param renderer The renderer to use the render surface with.
 * @param allocator The allocator to create the render surface with. If NULL, it will use the same
 *     allocator as the renderer.
 * @param osHandle The handle to the OS surface, such as the window handle.
 * @param type The render surface type.
 * @return The created renderbuffer, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsRenderSurface* dsRenderSurface_create(dsRenderer* renderer,
	dsAllocator* allocator, void* osHandle, dsRenderSurfaceType type);

/**
 * @brief Updates a render surface.
 * @param renderSurface The render surface to update.
 * @return True if the render surface was resized. Any framebuffers that use the render surface
 *     should be re-created with the new parameters.
 */
DS_RENDER_EXPORT bool dsRenderSurface_update(dsRenderSurface* renderSurface);

/**
 * @brief Destroys a render surface.
 * @remark errno will be set on failure.
 * @param renderSurface The render surface to destroy.
 * @return False if the render surface couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsRenderSurface_destroy(dsRenderSurface* renderSurface);


#ifdef __cplusplus
}
#endif
