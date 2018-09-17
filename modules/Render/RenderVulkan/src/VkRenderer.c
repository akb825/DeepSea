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

#include <DeepSea/RenderVulkan/VkRenderer.h>

#include "Types.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/DynamicLib.h>
#include <stdlib.h>
#include <string.h>

#if DS_WINDOWS
#define DS_VULKAN_LIBRARY DS_LIBRARY_NAME("vulkan-1")
#else
#define DS_VULKAN_LIBRARY DS_LIBRARY_NAME("vulkan")
#endif

#define DS_LOAD_VK_INSTANCE_FUNCTION(instance, function) \
	do \
	{ \
		instance->function = (PFN_ ## function)instance->vkGetInstanceProcAddr(instance->instance, \
			#function); \
		if (!instance->function) \
		{ \
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load " #function); \
			errno = EPERM; \
			return false; \
		} \
	} \
	while (false)

#define DS_LOAD_VK_DEVICE_FUNCTION(device, function) \
	do \
	{ \
		device->function = (PFN_ ## function)device->vkGetDeviceProcAddr(device->device, \
			#function); \
		if (!device->function) \
		{ \
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load " #function); \
			errno = EPERM; \
			return false; \
		} \
	} \
	while (false)

#define DS_MAX_ENABLED_EXTENSIONS 100

#define DS_ADD_EXTENSION(extensions, count, extension) \
	do \
	{ \
		DS_ASSERT(count < DS_MAX_ENABLED_EXTENSIONS); \
		extensions[count++] = extension; \
	} \
	while (false)

#define DS_MAX_DEVICES 16

DS_STATIC_ASSERT(DS_DEVICE_UUID_SIZE == VK_UUID_SIZE, unexpected_uuid_size);

typedef struct InstanceExtensions
{
	bool initialized;
	bool debug;
	bool deviceInfo;
} InstanceExtensions;

static const char* debugLayerName = "VK_LAYER_LUNARG_standard_validation";
static const char* debugExtensionName = "VK_EXT_debug_utils";
static const char* devicePropertiesExtensionName = "VK_KHR_get_physical_device_properties2";
static const char* memoryCapabilitiesExtensionName = "VK_KHX_external_memory_capabilities";

static InstanceExtensions instanceExtensions;
static uint32_t physicalDeviceCount;
static VkPhysicalDeviceProperties physicalDevices[DS_MAX_DEVICES];
static uint8_t physicalDevicesIDs[DS_MAX_DEVICES][DS_DEVICE_UUID_SIZE];

static size_t dsVkRenderer_fullAllocSize(const dsRendererOptions* options)
{
	size_t pathLen = options->shaderCacheDir ? strlen(options->shaderCacheDir) + 1 : 0;
	return DS_ALIGNED_SIZE(sizeof(dsVkRenderer)) + DS_ALIGNED_SIZE(pathLen);
}

static void* vkAllocFunc(void* pUserData, size_t size, size_t alignment,
	VkSystemAllocationScope allocationScope)
{
	DS_UNUSED(allocationScope);
	dsAllocator* allocator = (dsAllocator*)pUserData;
	return allocator->allocFunc(allocator, size, (unsigned int)alignment);
}

static void* vkReallocFunc(void* pUserData, void* pOriginal, size_t size, size_t alignment,
	VkSystemAllocationScope allocationScope)
{
	DS_UNUSED(allocationScope);
	dsAllocator* allocator = (dsAllocator*)pUserData;
	return allocator->reallocFunc(allocator, pOriginal, size, (unsigned int)alignment);
}

static void vkFreeFunc(void* pUserData, void* pMemory)
{
	dsAllocator* allocator = (dsAllocator*)pUserData;
	allocator->freeFunc(allocator, pMemory);
}

static bool queryInstanceExtensions(dsVkInstance* instance)
{
	if (instanceExtensions.initialized)
		return true;

	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumerateInstanceLayerProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumerateInstanceExtensionProperties);

	uint32_t layerCount = 0;
	instance->vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	bool hasDebugLayer = false;
	if (layerCount > 0)
	{
		VkLayerProperties* layers =
			(VkLayerProperties*)malloc(layerCount*sizeof(VkLayerProperties));
		if (!layers)
			return false;

		instance->vkEnumerateInstanceLayerProperties(&layerCount, layers);
		for (uint32_t i = 0; i < layerCount; ++i)
		{
			if (strcmp(layers[i].layerName, debugLayerName) == 0)
				hasDebugLayer = true;
		}
		free(layers);
	}

	uint32_t extensionCount = 0;
	instance->vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
	bool hasDeviceProperties = false;
	bool hasMemoryCapabilities = false;
	VkExtensionProperties* extensions =
		(VkExtensionProperties*)malloc(extensionCount*sizeof(VkExtensionProperties));
	if (!extensions)
		return false;

	instance->vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		if (strcmp(extensions[i].extensionName, devicePropertiesExtensionName) == 0)
			hasDeviceProperties = true;
		else if (strcmp(extensions[i].extensionName, memoryCapabilitiesExtensionName) == 0)
			hasMemoryCapabilities = true;
	}
	free(extensions);

	if (hasDeviceProperties && hasMemoryCapabilities)
		instanceExtensions.deviceInfo = true;

	if (hasDebugLayer)
	{
		instance->vkEnumerateInstanceExtensionProperties(debugLayerName, &extensionCount, NULL);
		extensions = (VkExtensionProperties*)malloc(extensionCount*sizeof(VkExtensionProperties));
		if (!extensions)
			return false;

		instance->vkEnumerateInstanceExtensionProperties(debugLayerName, &extensionCount,
			extensions);
		for (uint32_t i = 0; i < extensionCount; ++i)
		{
			if (strcmp(extensions[i].extensionName, debugExtensionName) == 0)
			{
				instanceExtensions.debug = true;
				return true;
			}
		}
		free(extensions);
	}

	instanceExtensions.initialized = true;
	return true;
}

static bool createInstance(dsVkInstance* instance, const dsRendererOptions* options,
	bool handleErrors)
{
	if (options && options->gfxAPIAllocator && (!options->gfxAPIAllocator->freeFunc ||
		!options->gfxAPIAllocator->reallocFunc))
	{
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
			"Graphics API allocator must support freeing and reallocation.");
		errno = EPERM;
		return false;
	}

	dsDynamicLib* library = &instance->library;
	if (!dsDynamicLib_open(library, DS_VULKAN_LIBRARY))
	{
		if (handleErrors)
		{
			DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't open vulkan library: %s",
				library->error);
			errno = EPERM;
		}
		return false;
	}

	instance->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dsDynamicLib_loadSymbol(library,
		"vkGetInstanceProcAddr");
	if (!instance->vkGetInstanceProcAddr)
	{
		DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load vkGetInstanceProcAddr: %s",
			library->error);
		errno = EPERM;
		return false;
	}

	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCreateInstance);
	queryInstanceExtensions(instance);

	const char* enabledLayers[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledLayerCount = 0;
	DS_ADD_EXTENSION(enabledLayers, enabledLayerCount, debugLayerName);

	const char* enabledExtensions[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledExtensionCount = 0;
	DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, "VK_KHR_surface");
#if DS_WINDOWS
	DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, "VK_KHR_win32_surface");
#elif DS_ANDROID
	DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, "VK_KHR_android_surface");
#else
	DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, "VK_KHR_xlib_surface");
#endif

	if (instanceExtensions.deviceInfo)
	{
		DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, devicePropertiesExtensionName);
		DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, memoryCapabilitiesExtensionName);
	}

	if (options && options->debug && instanceExtensions.debug)
		DS_ADD_EXTENSION(enabledExtensions, enabledExtensionCount, debugExtensionName);

	VkApplicationInfo applicationInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		options ? options->applicationName : NULL,
		options ? options->applicationVersion : 0,
		"DeepSea",
		DS_VERSION,
		VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		NULL,
		0,
		&applicationInfo,
		enabledLayerCount,
		enabledLayers,
		enabledExtensionCount,
		enabledExtensions
	};

	if (options && options->gfxAPIAllocator)
	{
		instance->allocCallbacks.pUserData = options->gfxAPIAllocator;
		instance->allocCallbacks.pfnAllocation = &vkAllocFunc;
		instance->allocCallbacks.pfnReallocation = &vkReallocFunc;
		instance->allocCallbacks.pfnFree = &vkFreeFunc;
		instance->allocCallbacks.pfnInternalAllocation = NULL;
		instance->allocCallbacks.pfnInternalFree = NULL;
		instance->allocCallbacksPtr = &instance->allocCallbacks;
	}
	else
		instance->allocCallbacksPtr = NULL;

	VkResult result = instance->vkCreateInstance(&createInfo, instance->allocCallbacksPtr,
		&instance->instance);
	if (handleErrors)
	{
		if (!dsHandleVkResult(result))
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't create Vulkan instance.");
			return false;
		}
	}
	else if (result != VK_SUCCESS)
		return false;

	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDestroyInstance);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumeratePhysicalDevices);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceProperties);
	if (instanceExtensions.deviceInfo)
		DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceProperties2KHR);

	return true;
}

static void destroyInstance(dsVkInstance* instance)
{
	if (instance->instance && instance->vkDestroyInstance)
		instance->vkDestroyInstance(instance->instance, instance->allocCallbacksPtr);
	dsDynamicLib_close(&instance->library);
}

static bool createDevice(dsVkDevice* device, const dsRendererOptions* options)
{
	if (!createInstance(&device->instance, options, true))
		return false;

	return true;
}

static void destroyDevice(dsVkDevice* device)
{
	destroyInstance(&device->instance);
}

bool dsVkRenderer_destroy(dsRenderer* renderer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	destroyDevice(device);
	DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));
	return true;
}

bool dsVkRenderer_isSupported(void)
{
	static int supported = -1;
	if (supported >= 0)
		return supported;

	dsVkInstance instance = {};
	supported = createInstance(&instance, NULL, false);
	destroyInstance(&instance);
	return supported;
}

bool dsVkRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	if (physicalDeviceCount == 0)
	{
		dsVkInstance instance = {};
		if (!createInstance(&instance, NULL, true))
		{
			destroyInstance(&instance);
			return false;
		}

		VkPhysicalDevice devices[DS_MAX_DEVICES];
		physicalDeviceCount = DS_MAX_DEVICES;
		instance.vkEnumeratePhysicalDevices(instance.instance, &physicalDeviceCount, devices);
		for (uint32_t i = 0; i < physicalDeviceCount; ++i)
		{
			instance.vkGetPhysicalDeviceProperties(devices[i], physicalDevices + i);
			if (instanceExtensions.deviceInfo)
			{
				VkPhysicalDeviceIDPropertiesKHR deviceID;
				deviceID.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
				deviceID.pNext = NULL;

				VkPhysicalDeviceProperties2KHR properties2;
				properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
				properties2.pNext = &deviceID;
				instance.vkGetPhysicalDeviceProperties2KHR(devices[i], &properties2);
				memcpy(physicalDevicesIDs + i, deviceID.deviceUUID, DS_DEVICE_UUID_SIZE);
			}
		}

		destroyInstance(&instance);
	}

	if (!outDevices)
	{
		*outDeviceCount = physicalDeviceCount;
		return true;
	}

	if (*outDeviceCount > physicalDeviceCount)
		*outDeviceCount = physicalDeviceCount;

	for (uint32_t i = 0; i < *outDeviceCount; ++i)
	{
		outDevices[i].name = physicalDevices[i].deviceName;
		outDevices[i].vendorID = physicalDevices[i].vendorID;
		outDevices[i].deviceID = physicalDevices[i].deviceID;
		switch (physicalDevices[i].deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				outDevices[i].deviceType = dsRenderDeviceType_Integrated;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				outDevices[i].deviceType = dsRenderDeviceType_Discrete;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				outDevices[i].deviceType = dsRenderDeviceType_Virtual;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				outDevices[i].deviceType = dsRenderDeviceType_CPU;
				break;
			default:
				outDevices[i].deviceType = dsRenderDeviceType_Unknown;
				break;
		}

		memcpy(outDevices[i].deviceUUID, physicalDevicesIDs[i], DS_DEVICE_UUID_SIZE);
	}

	return true;
}

dsRenderer* dsVkRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Renderer allocator must support freeing memory.");
		return NULL;
	}

	size_t bufferSize = dsVkRenderer_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkRenderer);
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	baseRenderer->allocator = allocator;

	if (!createDevice(&renderer->device, options))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	return baseRenderer;
}
