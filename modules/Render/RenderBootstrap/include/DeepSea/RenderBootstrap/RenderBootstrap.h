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
#include <DeepSea/RenderBootstrap/Export.h>
#include <DeepSea/RenderBootstrap/RendererIDs.h>
#include <DeepSea/RenderBootstrap/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for bootstrapping a renderer.
 */

/**
 * @brief Gets the name for a renderer.
 * @param type The type of the renderer.
 * @return The renderer name, or NULL if an invalid enum or dsRendererType_Default.
 */
DS_RENDERBOOTSTRAP_EXPORT const char* dsRenderBootstrap_rendererName(dsRendererType type);

/**
 * @brief Gets the type of a renderer by name.
 * @param name The name of the renderer.
 * @return The renderer type, or dsRendererType_Default if no such name was found.
 */
DS_RENDERBOOTSTRAP_EXPORT dsRendererType dsRenderBootstrap_rendererTypeFromName(const char* name);

/**
 * @brief Gets the default renderer.
 * @return The default renderer, or dsRenderType_Default if no renderer is supported.
 */
DS_RENDERBOOTSTRAP_EXPORT dsRendererType dsRenderBootstrap_defaultRenderer(void);

/**
 * @brief Gets whether or not a renderer is supported.
 * @param type The renderer type to check.
 * @return True if the renderer is supported.
 */
DS_RENDERBOOTSTRAP_EXPORT bool dsRenderBootstrap_isSupported(dsRendererType type);

/**
 * @brief Queries the devices available for use.
 *
 * This may return an empty list of devices if the renderer doesn't expose a list of devices.
 *
 * @remark errno will be set on failure.
 * @param[out] outDevices Output pointer for the devices. This may be NULL to query the total number
 *     of devices.
 * @param[out] outDeviceCount The number of devices that were set. If outDevices isn't NULL, the
 *     initial value is the capacity of outDevices.
 * @param type The type of renderer to get the devices for.
 * @return False if an error occurred.
 */
DS_RENDERBOOTSTRAP_EXPORT bool dsRenderBootstrap_queryDevices(dsRenderDeviceInfo* outDevices,
	uint32_t* outDeviceCount, dsRendererType type);

/**
 * @brief Creates a renderer.
 * @remark errno will be set on failure.
 * @param type The type of renderer to create.
 * @param allocator The allocator to create the renderer with.
 * @param options The options for creating the renderer.
 * @return The
 */
DS_RENDERBOOTSTRAP_EXPORT dsRenderer* dsRenderBootstrap_createRenderer(dsRendererType type,
	dsAllocator* allocator, const dsRendererOptions* options);

#ifdef __cplusplus
}
#endif
