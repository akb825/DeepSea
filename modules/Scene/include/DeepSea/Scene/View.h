/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating views.
 * @see dsView
 */

/**
 * @brief Registers a cull ID with the cull manager owned by the view.
 *
 * This must be done as part of the synchronous item list in the scene. While this modifies the
 * state of view despite being const, this is temporary data that is allowed to be modified.
 *
 * @remark Other interactions with the cull manager should be done directly from the cullManager
 * member of dsView. This is provided to make it explicit that it's safe to perform this non-const
 * operation on a const dsView instance.
 *
 * @remark errno will be set on failure.
 * @param view The view to register the cull ID with.
 * @param cullID The cull ID to register.
 * @return The cull instance or DS_SCENE_NO_CULL if it cannot be registered.
 */
DS_SCENE_EXPORT uint32_t dsView_registerCullID(const dsView* view, dsSceneCullID cullID);

/**
 * @brief Creates a view.
 * @remark errno will be set on failure.
 * @param scene The scene the view will display.
 * @param allocator The allocator for the view. If NULL, the scene's allocator will be used.
 * @param surfaces The surfaces to use with the view.
 * @param surfaceCount THe number of surfaces.
 * @param framebuffers The framebuffers to use with the view.
 * @param framebufferCount The number of framebuffers.
 * @param width The width of the view.
 * @param height The height of the view.
 * @param userData User data to hold with the view.
 * @param destroyUserDataFunc Function to destroy the user data for the view.
 * @return The view or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsView* dsView_create(const dsScene* scene, dsAllocator* allocator,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount,
	const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount, uint32_t width,
	uint32_t height, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the dimensions of the view.
 *
 * This will be applied the next time dsView_update() is called.
 *
 * @remark errno will be set on failure.
 * @param view The view.
 * @param width The width of the view.
 * @param height The height of the view.
 * @return False if parameters are invalid.
 */
DS_SCENE_EXPORT bool dsView_setDimensions(dsView* view, uint32_t width, uint32_t height);

/**
 * @brief Gets a surface used within the view.
 * @param[out] outType The surface type.
 * @param view The view to get the surface from.
 * @param name The naem of the surface.
 * @return The surface or NULL if the surface wasn't found. The surface may also be NULL if
 *     dsView_update() was never called.
 */
DS_SCENE_EXPORT void* dsView_getSurface(dsGfxSurfaceType* outType, const dsView* view,
	const char* name);

/**
 * @brief Sets a surface used within the view.
 *
 * This may only be used for surfaces that originally had an external surface provided. This
 * function can be used if the surface needs to be updated. (e.g. the render surface for the window
 * gets invalidated)
 *
 * @remark errno will be set on failure.
 * @param view The view to set the surface on.
 * @param name The name of the surface.
 * @param surface The surface to set.
 * @param surfaceType The type of the surface. This must be the same as the original surface that
 *     was set.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsView_setSurface(dsView* view, const char* name, void* surface,
	dsGfxSurfaceType surfaceType);

/**
 * @brief Updates the view.
 *
 * This will re-create any surfaces and framebuffers it needs to based on the size or anti-alias
 * samples changing. This must be called before drawing the view, even if the view was just created.
 *
 * @param view The view to update.
 * @return False if the view couldn't be updated.
 */
DS_SCENE_EXPORT bool dsView_update(dsView* view);

/**
 * @brief Destroys a view.
 * @remark errno will be set on failure.
 * @param view The view to destroy.
 * @return False if the view couldn't be destroyed.
 */
DS_SCENE_EXPORT bool dsView_destroy(dsView* view);

#ifdef __cplusplus
}
#endif

