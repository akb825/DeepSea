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

#include "Platform/VkPlatformAndroid.h"
#include "VkShared.h"
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_ANDROID

#include <android/native_window.h>
#include <vulkan/vulkan_android.h>

static VkInstance loadedInstance;
static PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;

void* dsVkPlatformAndroid_getDisplay(void)
{
	return NULL;
}

void dsVkPlatformAndroid_releaseDisplay(void* display)
{
	DS_UNUSED(display);
}

VkSurfaceKHR dsVkPlatformAndroid_createSurface(dsVkInstance* instance, void* display, void* window)
{
	if (instance->instance != loadedInstance || !vkCreateAndroidSurfaceKHR)
	{
		loadedInstance = instance->instance;
		vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)instance->vkGetInstanceProcAddr(
			instance->instance, "vkCreateAndroidSurfaceKHR");
		if (!vkCreateAndroidSurfaceKHR)
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load vkCreateAndroidSurfaceKHR");
			errno = EPERM;
			return 0;
		}
	}

	VkAndroidSurfaceCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		(struct ANativeWindow*)window
	};

	VkSurfaceKHR surface;
	VkResult result = DS_VK_CALL(vkCreateAndroidSurfaceKHR)(instance->instance, &createInfo,
		instance->allocCallbacksPtr, &surface);
	if (!dsHandleVkResult(result))
		return 0;

	return surface;
}

#endif // DS_ANDROID
