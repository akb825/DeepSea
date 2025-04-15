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

#include "Platform/VkPlatformX11.h"
#include "VkShared.h"
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_VK_HAS_X11

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

static VkInstance loadedInstance;
static PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

void dsVkPlatformX11_initialize(void)
{
}

VkSurfaceKHR dsVkPlatformX11_createSurface(dsVkInstance* instance, void* display, void* window)
{
	if (instance->instance != loadedInstance || !vkCreateXlibSurfaceKHR)
	{
		loadedInstance = instance->instance;
		vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)instance->vkGetInstanceProcAddr(
			instance->instance, "vkCreateXlibSurfaceKHR");
		if (!vkCreateXlibSurfaceKHR)
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load vkCreateXlibSurfaceKHR");
			errno = EPERM;
			return 0;
		}
	}

	VkXlibSurfaceCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		(Display*)display,
		(Window)window
	};

	VkSurfaceKHR surface;
	VkResult result = DS_VK_CALL(vkCreateXlibSurfaceKHR)(instance->instance, &createInfo,
		instance->allocCallbacksPtr, &surface);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create surface"))
		return 0;

	return surface;
}

#endif // DS_VK_HAS_X11
