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

#include "Platform/VkPlatformWayland.h"
#include "VkShared.h"
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_VK_HAS_WAYLAND

#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>

static VkInstance loadedInstance;
static PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;

void* dsVkPlatformWayland_getDisplay(void)
{
	return wl_display_connect(NULL);
}

void dsVkPlatformWayland_releaseDisplay(void* display)
{
	wl_display_disconnect((struct wl_display*)display);
}

VkSurfaceKHR dsVkPlatformWayland_createSurface(dsVkInstance* instance, void* display, void* window)
{
	if (instance->instance != loadedInstance || !vkCreateWaylandSurfaceKHR)
	{
		loadedInstance = instance->instance;
		vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)instance->vkGetInstanceProcAddr(
			instance->instance, "vkCreateWaylandSurfaceKHR");
		if (!vkCreateWaylandSurfaceKHR)
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load vkCreateWaylandSurfaceKHR");
			errno = EPERM;
			return 0;
		}
	}

	VkWaylandSurfaceCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		(struct wl_display*)display,
		(struct wl_surface*)window
	};

	VkSurfaceKHR surface;
	VkResult result = DS_VK_CALL(vkCreateWaylandSurfaceKHR)(instance->instance, &createInfo,
		instance->allocCallbacksPtr, &surface);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create surface"))
		return 0;

	return surface;
}

#endif // DS_VK_HAS_WAYLAND
