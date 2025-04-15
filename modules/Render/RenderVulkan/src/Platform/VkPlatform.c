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

#include "Platform/VkPlatform.h"
#include "Platform/VkPlatformAndroid.h"
#include "Platform/VkPlatformWayland.h"
#include "Platform/VkPlatformWin32.h"
#include "Platform/VkPlatformX11.h"
#include "VkShared.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

bool dsVkPlatform_initialize(dsVkPlatform* platform, dsVkDevice* device,
	dsGfxPlatform gfxPlatform, void* display)
{
	platform->device = device;
	platform->initializeFunc = NULL;
	platform->createSurfaceFunc = NULL;
	switch (gfxPlatform)
	{
		case dsGfxPlatform_X11:
#if DS_VK_HAS_X11
			platform->initializeFunc = &dsVkPlatformX11_initialize;
			platform->createSurfaceFunc = &dsVkPlatformX11_createSurface;
			break;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "X11 platform not supported.");
			return false;
#endif
		case dsGfxPlatform_Wayland:
#if DS_VK_HAS_WAYLAND
			platform->initializeFunc = &dsVkPlatformWayland_initialize;
			platform->createSurfaceFunc = &dsVkPlatformWayland_createSurface;
			break;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Wayland platform not supported.");
			return false;
#endif
		default:
#if DS_ANDROID
			platform->initializeFunc = &dsVkPlatformAndroid_initialize;
			platform->createSurfaceFunc = &dsVkPlatformAndroid_createSurface;
#elif DS_WINDOWS
			platform->initializeFunc = &dsVkPlatformWin32_initialize;
			platform->createSurfaceFunc = &dsVkPlatformWin32_createSurface;
#elif DS_VK_HAS_X11
			platform->initializeFunc = &dsVkPlatformX11_initialize;
			platform->createSurfaceFunc = &dsVkPlatformX11_createSurface;
#elif DS_VK_HAS_WAYLAND
			platform->initializeFunc = &dsVkPlatformWayland_initialize;
			platform->createSurfaceFunc = &dsVkPlatformWayland_createSurface;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "No platform available.");
			return false;
#endif
	}

	platform->initializeFunc();
	return true;
}

VkSurfaceKHR dsVkPlatform_createSurface(dsVkPlatform* platform, void* display, void* window)
{
	return platform->createSurfaceFunc(&platform->device->instance, display, window);
}

void dsVkPlatform_destroySurface(dsVkPlatform* platform, VkSurfaceKHR surface)
{
	dsVkInstance* instance = &platform->device->instance;
	DS_VK_CALL(instance->vkDestroySurfaceKHR)(instance->instance, surface,
		instance->allocCallbacksPtr);
}
