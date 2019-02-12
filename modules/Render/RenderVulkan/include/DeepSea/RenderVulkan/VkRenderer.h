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
#include <DeepSea/RenderVulkan/Export.h>
#include <DeepSea/RenderVulkan/RendererIDs.h>
#include <DeepSea/Render/Renderer.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating an Vulkan renderer.
 */

/**
 * @brief Gets whether or not the Vulkan renderer is supported.
 * @return True if supported.
 */
DS_RENDERVULKAN_EXPORT bool dsVkRenderer_isSupported(void);

/**
 * @brief Queries the devices available for use with Vulkan.
 * @remark errno will be set on failure.
 * @param[out] outDevices Output pointer for the devices. This may be NULL to query the total number
 *     of devices.
 * @param[out] outDeviceCount The number of devices that were set. If outDevices isn't NULL, the
 *     initial value is the capacity of outDevices.
 * @return False if the parameters are invalid.
 */
DS_RENDERVULKAN_EXPORT bool dsVkRenderer_queryDevices(dsRenderDeviceInfo* outDevices,
	uint32_t* outDeviceCount);

/**
 * @brief Gets the default device that will be used with Vulkan.
 * @remark errno will be set on failure.
 * @param[out] outDevice Output pointer for the device.
 * @return False if no device is available.
 */
DS_RENDERVULKAN_EXPORT bool dsVkRenderer_getDefaultDevice(dsRenderDeviceInfo* outDevice);

/**
 * @brief Creates a Vulkan renderer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use.
 * @param options The options to initialize Vulkan with.
 * @return The created renderer, or NULL if the renderer coulnd't be created.
 */
DS_RENDERVULKAN_EXPORT dsRenderer* dsVkRenderer_create(dsAllocator* allocator,
	const dsRendererOptions* options);

#ifdef __cplusplus
}
#endif
