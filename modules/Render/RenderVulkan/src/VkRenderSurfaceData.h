/*
 * Copyright 2018-2025 Aaron Barany
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
#include "VkTypes.h"

typedef enum dsVkSurfaceResult
{
	dsVkSurfaceResult_Success,
	dsVkSurfaceResult_Error,
	dsVkSurfaceResult_OutOfDate,
} dsVkSurfaceResult;


dsRenderSurfaceRotation dsVkRenderSurfaceData_getRotation(VkSurfaceTransformFlagBitsKHR rotation);
dsVkRenderSurfaceData* dsVkRenderSurfaceData_create(dsAllocator* allocator, dsRenderer* renderer,
	VkSurfaceKHR surface, dsVSync vsync, VkSwapchainKHR prevSwapchain, dsRenderSurfaceUsage usage,
	const VkSurfaceCapabilitiesKHR* surfaceInfo);
dsVkSurfaceResult dsVkRenderSurfaceData_acquireImage(dsVkRenderSurfaceData* surfaceData);

void dsVkRenderSurfaceData_destroy(dsVkRenderSurfaceData* surfaceData);
