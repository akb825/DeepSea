/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/RenderMock/Export.h>
#include <DeepSea/RenderMock/RendererIDs.h>
#include <DeepSea/Render/Renderer.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating a mock renderer.
 *
 * The mock renderer cannot be used to do actual rendering. Its intent is to be used for unit tests.
 */

/**
 * @brief Gets whether or not the mock renderer is supported.
 * @return Always returns true.
 */
DS_RENDERMOCK_EXPORT bool dsMockRenderer_isSupported(void);

/**
 * @brief Queries the devices available for use.
 *
 * This will always return an empty set of devices.
 *
 * @remark errno will be set on failure.
 * @param[out] outDevices Output pointer for the devices.
 * @param[out] outDeviceCount The number of devices. This will be set to 0.
 * @return False if the parameters are invalid.
 */
DS_RENDERMOCK_EXPORT bool dsMockRenderer_queryDevices(dsRenderDeviceInfo* outDevices,
	uint32_t* outDeviceCount);

/**
 * @brief Creates a mock renderer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use.
 * @return The renderer.
 */
DS_RENDERMOCK_EXPORT dsRenderer* dsMockRenderer_create(dsAllocator* allocator);

#ifdef __cplusplus
}
#endif
