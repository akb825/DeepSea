/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/Application/Export.h>
#include <DeepSea/Application/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manageing a Wayland color manager.
 *
 * The color manager is intended to be used for HDR support with known reference luminance values.
 * Otherwise, the reference luminance may be implementation-defined, making it potentially
 * compositor and/or driver specific what values correspond to SDR content.
 *
 * This runs undeer the assumption that the outputs and surfaces will be externally managed. In
 * order to track the association between outputs and surfaces, the callers must do the following:
 * 1. Register and unregister outputs as they get added and removed.
 * 2. Register and unregister surfaces as they get created and destroyed.
 * 3. Notify when a surface is associated with a different output.
 */

#if DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER || defined(DOXYGEN)

#ifdef DOXYGEN
/**
 * @brief Define for whether the Wayland color manager is available.
 *
 * Having this set to 1 ensures that the compile-time requirements are met, which generally means a
 * playform that *can* support Wayland. However, creation of the color manager may still fail if the
 * runtime requirements aren't met.
 */
#define DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER 1
#endif

/// @cond
struct wl_display;
struct wl_output;
struct wl_surface;
/// @endcond

/**
 * @brief Color manager for Wayland surfaces.
 */
typedef struct dsWaylandColorManager dsWaylandColorManager;

/**
 * @brief Surface used within a color manager.
 */
typedef struct dsWaylandColorManagerSurface dsWaylandColorManagerSurface;

/**
 * @brief Creates a Wayland color manager.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the color manager with.
 * @param display The Wayland display.
 * @return The color manager or NULL if an error occurred. An error likely indicates that the
 *     current target doesn't support the necessary protocols.
 */
DS_APPLICATION_EXPORT dsWaylandColorManager* dsWaylandColorManager_create(
	dsAllocator* allocator, struct wl_display* display);

/**
 * @brief Adds an output to the color manager.
 * @remark errno will be set on failure.
 * @param colorManager The color manager.
 * @param output The output to add.
 * @return False if the output couldn't be added.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManager_addOutput(
	dsWaylandColorManager* colorManager, struct wl_output* output);

/**
 * @brief Removes an output from the color manager.
 * @remark errno will be set on failure.
 * @param colorManager The color manager.
 * @param output The output to remove.
 * @return False if the output couldn't be removed.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManager_removeOutput(
	dsWaylandColorManager* colorManager, struct wl_output* output);

/**
 * @brief Gets the luminance values for an output.
 * @remark errno will be set on failure.
 * @param[out] outReferenceNits The refernce luminance value in nits.
 * @param[out] outMaxNits The maximum luminance value in nits.
 * @param colorManager The color manager.
 * @param output The output to get the lumance values for.
 * @return Flase if the luminance values couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManager_getOutputLuminances(float* outReferenceNits,
	float* outMaxNits, const dsWaylandColorManager* colorManager, struct wl_output* output);

/**
 * @brief Adds a surface to the color manager.
 * @remark errno will be set on failure.
 * @param colorManager The color manager.
 * @param surface The surface to add.
 * @param output The primary output the surface is displayed on. This may be NULL if the surface is
 *     not yet added to the display.
 * @param colorSpace The color space for the surface.
 * @return The color manager surface or NULL if the surface couldn't be added.
 */
DS_APPLICATION_EXPORT dsWaylandColorManagerSurface* dsWaylandColorManager_addSurface(
	dsWaylandColorManager* colorManager, struct wl_surface* surface, struct wl_output* output,
	dsRenderColorSpace colorSpace);

/**
 * @brief Updates the state of the color manager.
 *
 * This will get the latest state for the luminance values from the registered outputs.
 *
 * @param colorManager The color manager to update.
 * @return Whether any surfaces were updated.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManager_update(dsWaylandColorManager* colorManager);

/**
 * @brief Destroys a Wayland color manager.
 *
 * Any color manager surfaces from surfaces that were added will also be destroyed.
 *
 * @param colorManager The color manager to destroy.
 */
DS_APPLICATION_EXPORT void dsWaylandColorManager_destroy(dsWaylandColorManager* colorManager);

/**
 * @brief Sets the output for a color manager surface.
 * @remark errno will be set on failure.
 * @param surface The surface to set the output on.
 * @param output The output for the surface. This may be NULL if the surface is no longer associated
 *     with an output.
 * @return False if the output couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManagerSurface_setOutput(
	dsWaylandColorManagerSurface* surface, struct wl_output* output);

/**
 * @brief Sets the color space for a color manager surface.
 * @remark errno will be set on failure.
 * @param surface The surface to set the color space on.
 * @param colorSpace The color space for the surface.
 * @return False if the color space couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManagerSurface_setColorSpace(
	dsWaylandColorManagerSurface* surface, dsRenderColorSpace colorSpace);

/**
 * @brief Gets the luminance values for a color manager surface.
 * @remark errno will be set on failure.
 * @param[out] outReferenceNits The refernce luminance value in nits.
 * @param[out] outMaxNits The maximum luminance value in nits.
 * @param surface The surface to get the luminance values for.
 * @return Flase if the luminance values couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsWaylandColorManagerSurface_getLumiances(
	float* outReferenceNits, float* outMaxNits, const dsWaylandColorManagerSurface* surface);

/**
 * @brief Destroys a color manager surface, removing it from the color manager.
 * @remark errno will be set on failure.
 * @param surface The surface to destroy.
 */
DS_APPLICATION_EXPORT void dsWaylandColorManagerSurface_destroy(
	dsWaylandColorManagerSurface* surface);

#endif //DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER || defined(DOXYGEN)

#ifdef __cplusplus
}
#endif
