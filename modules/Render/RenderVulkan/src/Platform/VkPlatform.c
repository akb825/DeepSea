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
	platform->getDisplayFunc = NULL;
	platform->releaseDisplayFunc = NULL;
	platform->createSurfaceFunc = NULL;
	switch (gfxPlatform)
	{
		case dsGfxPlatform_X11:
#if DS_VK_HAS_X11
			platform->getDisplayFunc = &dsVkPlatformX11_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformX11_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformX11_createSurface;
			break;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "X11 platform not supported.");
			return false;
#endif
		case dsGfxPlatform_Wayland:
#if DS_VK_HAS_WAYLAND
			platform->getDisplayFunc = &dsVkPlatformWayland_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformWayland_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformWayland_createSurface;
			break;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Wayland platform not supported.");
			return false;
#endif
		default:
#if DS_ANDROID
			platform->getDisplayFunc = &dsVkPlatformAndroid_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformAndroid_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformAndroid_createSurface;
#elif DS_WINDOWS
			platform->getDisplayFunc = &dsVkPlatformWin32_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformWin32_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformWin32_createSurface;
#elif DS_VK_HAS_X11
			platform->getDisplayFunc = &dsVkPlatformX11_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformX11_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformX11_createSurface;
#elif DS_VK_HAS_WAYLAND
			platform->getDisplayFunc = &dsVkPlatformWayland_getDisplay;
			platform->releaseDisplayFunc = &dsVkPlatformWayland_releaseDisplay;
			platform->createSurfaceFunc = &dsVkPlatformWayland_createSurface;
#else
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "No platform available.");
			return false;
#endif
	}

	if (display)
	{
		platform->display = display;
		platform->createdDisplay = false;
	}
	else
	{
		platform->display = platform->getDisplayFunc();
		platform->createdDisplay = true;
	}

	return true;
}

VkSurfaceKHR dsVkPlatform_createSurface(dsVkPlatform* platform, void* window)
{
	return platform->createSurfaceFunc(&platform->device->instance, platform->display, window);
}

void dsVkPlatform_destroySurface(dsVkPlatform* platform, VkSurfaceKHR surface)
{
	dsVkInstance* instance = &platform->device->instance;
	DS_VK_CALL(instance->vkDestroySurfaceKHR)(instance->instance, surface,
		instance->allocCallbacksPtr);
}

void dsVkPlatform_shutdown(dsVkPlatform* platform)
{
	if (platform->releaseDisplayFunc && platform->createdDisplay)
		platform->releaseDisplayFunc(platform->display);
}
