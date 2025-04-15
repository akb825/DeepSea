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

#include "Platform/VkPlatformWin32.h"
#include "VkShared.h"
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#if DS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#if WINVER >= 0x0810
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#endif

static VkInstance loadedInstance;
static PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

void dsVkPlatformWin32_initialize(void)
{
#if WINVER >= 0x0810
	// Prevent Windows from scaling the windows.
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
}

VkSurfaceKHR dsVkPlatformWin32_createSurface(dsVkInstance* instance, void* display, void* window)
{
	if (instance->instance != loadedInstance || !vkCreateWin32SurfaceKHR)
	{
		loadedInstance = instance->instance;
		vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)instance->vkGetInstanceProcAddr(
			instance->instance, "vkCreateWin32SurfaceKHR");
		if (!vkCreateWin32SurfaceKHR)
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load vkCreateWin32SurfaceKHR");
			errno = EPERM;
			return 0;
		}
	}

	VkWin32SurfaceCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		GetModuleHandle(NULL),
		(HWND)window
	};

	VkSurfaceKHR surface;
	VkResult result = DS_VK_CALL(vkCreateWin32SurfaceKHR)(instance->instance, &createInfo,
		instance->allocCallbacksPtr, &surface);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create surface"))
		return 0;

	return surface;
}

#endif // DS_WINDOWS
