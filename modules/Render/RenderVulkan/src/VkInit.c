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

#define _GNU_SOURCE // Expose strcasestr for GNU libc.

#include "VkInit.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/DynamicLib.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <stdlib.h>
#include <string.h>

#if DS_WINDOWS
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")

#define strcasestr(a, b) StrStrIA(a, b)

#define DS_VULKAN_LIBRARY DS_LIBRARY_NAME("vulkan-1")
#else
#define DS_VULKAN_LIBRARY DS_LIBRARY_NAME("vulkan")
#if DS_LINUX
#define DS_ALTERNATE_VULKAN_LIBRARY DS_VERSIONED_LIBRARY_NAME("vulkan", "1")
#endif
#endif

#define DS_LOAD_VK_INSTANCE_FUNCTION(instance, function) \
	do \
	{ \
		(instance)->function = (PFN_ ## function)(instance)->vkGetInstanceProcAddr( \
			(instance)->instance, #function); \
		if (!(instance)->function) \
		{ \
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load " #function); \
			errno = EPERM; \
			return false; \
		} \
	} \
	while (false)

#define DS_LOAD_VK_INSTANCE_EXT_FUNCTION(instance, function, version, postfix) \
	do \
	{ \
		if ((instance)->instanceVersion < version) \
		{ \
			(instance)->function = (PFN_ ## function)(instance)->vkGetInstanceProcAddr( \
				(instance)->instance, #function postfix); \
		} \
		else \
		{ \
			(instance)->function = (PFN_ ## function)(instance)->vkGetInstanceProcAddr( \
				(instance)->instance, #function); \
		} \
		if (!(instance)->function) \
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
		(device)->function = (PFN_ ## function)(device)->instance.vkGetDeviceProcAddr( \
			(device)->device,  #function); \
		if (!(device)->function) \
		{ \
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load " #function); \
			errno = EPERM; \
			return false; \
		} \
	} \
	while (false)

#define DS_LOAD_VK_DEVICE_EXT_FUNCTION(device, function, version, postfix) \
	do \
	{ \
		if ((device)->properties.apiVersion < version) \
		{ \
			(device)->function = (PFN_ ## function)(device)->instance.vkGetDeviceProcAddr( \
				(device)->device,  #function postfix); \
		} \
		else \
		{ \
			(device)->function = (PFN_ ## function)(device)->instance.vkGetDeviceProcAddr( \
				(device)->device,  #function); \
		} \
		if (!(device)->function) \
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
		DS_ASSERT((count) < DS_MAX_ENABLED_EXTENSIONS); \
		(extensions)[(count)++] = extension; \
	} \
	while (false)

#define DS_MAX_DEVICES 16
#define DS_MAX_QUEUE_FAMILIES 8

#define DS_DEVICE_INFO_CORE_VERSION VK_API_VERSION_1_1
#define DS_MAINTENANCE1_CORE_VERSION VK_API_VERSION_1_1
#define DS_CREATE_RENDERPASS2_CORE_VERSION VK_API_VERSION_1_1
#define DS_DEPTH_STENCIL_RESOVE_CORE_VERSION VK_API_VERSION_1_2
#define DS_DEDICATED_ALLOCATION_CORE_VERSION VK_API_VERSION_1_1

_Static_assert(DS_DEVICE_UUID_SIZE == VK_UUID_SIZE, "Unexpected UUID size.");

typedef struct InstanceExtensions
{
	bool initialized;
	bool validation;
	bool standardValidation;
	bool oldValidation;
	bool debug;
	bool oldDebugReport;
	bool deviceInfo;
	bool xlib;
	bool wayland;
	bool win32;
	bool android;
} InstanceExtensions;

typedef struct DeviceExtensions
{
	bool maintenance1;
	bool oldDebugMarker;
	bool depthStencilResolve;
	bool pvrtc;
	bool dedicatedAllocation;
} DeviceExtensions;

typedef struct ExtraDeviceInfo
{
	uint8_t uuid[DS_DEVICE_UUID_SIZE];
	bool supportsGraphics;
} ExtraDeviceInfo;

// Layers.
static const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
static const char* standardValidationLayerName = "VK_LAYER_LUNARG_standard_validation";
static const char* threadingValLayerName = "VK_LAYER_GOOGLE_threading";
static const char* paramValLayerName = "VK_LAYER_LUNARG_parameter_validation";
static const char* objectValLayerName = "VK_LAYER_LUNARG_object_tracker";
static const char* coreValLayerName = "VK_LAYER_LUNARG_core_validation";
static const char* uniqueObjectValLayerName = "VK_LAYER_GOOGLE_unique_objects";

// Instance extensions.
static const char* swapChainExtensionName = "VK_KHR_swapchain";
static const char* surfaceExtensionName = "VK_KHR_surface";
static const char* xlibDisplayExtensionName = "VK_KHR_xlib_surface";
static const char* waylandDisplayExtensionName = "VK_KHR_wayland_surface";
static const char* win32DisplayExtensionName = "VK_KHR_win32_surface";
static const char* androidDisplayExtensionName = "VK_KHR_android_surface";
static const char* debugExtensionName = "VK_EXT_debug_utils";
static const char* oldDebugReportExtensionName = "VK_EXT_debug_report";
static const char* physicalDeviceProperties2ExtensionName =
	"VK_KHR_get_physical_device_properties2";
static const char* externalMemoryCapabilitiesExtensionName =
	"VK_KHR_external_memory_capabilities";

// Device extensions.
static const char* oldDebugMarkerExtensionName = "VK_EXT_debug_marker";
static const char* maintenance1ExtensionName = "VK_KHR_maintenance1";
static const char* maintenance2ExtensionName = "VK_KHR_maintenance2";
static const char* multiviewExtensionName = "VK_KHR_multiview";
static const char* createRenderPass2ExtensionName = "VK_KHR_create_renderpass2";
static const char* depthStencilResolveExtensionName = "VK_KHR_depth_stencil_resolve";
static const char* pvrtcExtensionName = "VK_IMG_format_pvrtc";
static const char* getMemoryRequirements2ExtensionName = "VK_KHR_get_memory_requirements2";
static const char* dedicatedAllocationExtensionName = "VK_KHR_dedicated_allocation";

static InstanceExtensions instanceExtensions;
static uint32_t physicalDeviceCount;
static uint32_t graphicsDeviceCount;
static uint32_t defaultPhysicalDevice;
static VkPhysicalDeviceProperties physicalDevices[DS_MAX_DEVICES];
static ExtraDeviceInfo extraDeviceInfo[DS_MAX_DEVICES];

static const char* ignoredMessages[] =
{
	"UNASSIGNED-CoreValidation-DevLimit-MissingQueryCount",
	"UNASSIGNED-CoreValidation-DevLimitCountMismatch",
	"UNASSIGNED-ObjectTracker-Info",
	"CREATE",
	"OBJ_STAT Destroy"
};

static void* VKAPI_PTR vkAllocFunc(void* pUserData, size_t size, size_t alignment,
	VkSystemAllocationScope allocationScope)
{
	DS_UNUSED(allocationScope);
	dsAllocator* allocator = (dsAllocator*)pUserData;
	return allocator->allocFunc(allocator, size, (unsigned int)alignment);
}

static void* VKAPI_PTR vkReallocFunc(void* pUserData, void* pOriginal, size_t size,
	size_t alignment, VkSystemAllocationScope allocationScope)
{
	DS_UNUSED(allocationScope);
	dsAllocator* allocator = (dsAllocator*)pUserData;
	return allocator->reallocFunc(allocator, pOriginal, size, (unsigned int)alignment);
}

static void VKAPI_PTR vkFreeFunc(void* pUserData, void* pMemory)
{
	dsAllocator* allocator = (dsAllocator*)pUserData;
	allocator->freeFunc(allocator, pMemory);
}

static VkBool32 VKAPI_PTR debugFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	DS_UNUSED(messageType);
	DS_UNUSED(pUserData);

	uint32_t ignoredCount = DS_ARRAY_SIZE(ignoredMessages);
	for (uint32_t i = 0; i < ignoredCount; ++i)
	{
		if (strcmp(pCallbackData->pMessageIdName, ignoredMessages[i]) == 0)
			return false;
	}

	dsLogLevel logLevel = dsLogLevel_Info;
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		logLevel = dsLogLevel_Error;
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		logLevel = dsLogLevel_Warning;
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		logLevel = dsLogLevel_Info;
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		logLevel = dsLogLevel_Debug;

	const char* file = NULL;
	const char* function = NULL;
	unsigned int line = 0;
	dsGetLastVkCallsite(&file, &function, &line);
	if (!file)
	{
		file = "<unknown>";
		function = "<unknown>";
		line = 0;
	}
	dsLog_messagef(logLevel, DS_RENDER_VULKAN_LOG_TAG, file, line, function, "%s: %s",
		pCallbackData->pMessageIdName, pCallbackData->pMessage);

	// Continue executing the function.
	return false;
}

static VkBool32 VKAPI_PTR oldDebugFunc(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode,
	const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	DS_UNUSED(objectType);
	DS_UNUSED(object);
	DS_UNUSED(location);
	DS_UNUSED(messageCode);
	DS_UNUSED(pUserData);

	uint32_t ignoredCount = DS_ARRAY_SIZE(ignoredMessages);
	for (uint32_t i = 0; i < ignoredCount; ++i)
	{
		if (strstr(pMessage, ignoredMessages[i]))
			return false;
	}

	dsLogLevel logLevel = dsLogLevel_Info;
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		logLevel = dsLogLevel_Error;
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT ||
		flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		logLevel = dsLogLevel_Warning;
	}
	else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		logLevel = dsLogLevel_Info;
	else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		logLevel = dsLogLevel_Debug;

	const char* file = NULL;
	const char* function = NULL;
	unsigned int line = 0;
	dsGetLastVkCallsite(&file, &function, &line);
	if (!file)
	{
		file = "<unknown>";
		function = "<unknown>";
		line = 0;
	}
	dsLog_messagef(logLevel, DS_RENDER_VULKAN_LOG_TAG, file, line, function, "%s: %s", pLayerPrefix,
		pMessage);

	// Continue executing the function.
	return false;
}

static bool enableValidation(const dsRendererOptions* options)
{
	if (!options)
		return false;

	const char* env = getenv("DS_DISABLE_VULKAN_VALIDATIONS");
	if (env && strcmp(env, "0") != 0)
		return false;

	// NOTE: RenderDoc will crash when adding the debug message function.
	if (getenv("RENDERDOC_CAPFILE"))
		return false;

	return options->debug;
}

static bool enableMarkers(bool useValidation)
{
	return useValidation || DS_PROFILING_ENABLED;
}

static dsRenderDeviceType convertDeviceType(VkPhysicalDeviceType type)
{
	switch (type)
	{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return dsRenderDeviceType_Integrated;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return dsRenderDeviceType_Discrete;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return dsRenderDeviceType_Virtual;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return dsRenderDeviceType_CPU;
		default:
			return dsRenderDeviceType_Unknown;
	}
}

static bool queryInstanceExtensions(dsVkInstance* instance)
{
	if (instanceExtensions.initialized)
		return true;

	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumerateInstanceLayerProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumerateInstanceExtensionProperties);

	uint32_t layerCount = 0;
	instance->vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	if (layerCount > 0)
	{
		VkLayerProperties* layers =
			(VkLayerProperties*)malloc(layerCount*sizeof(VkLayerProperties));
		if (!layers)
			return false;

		instance->vkEnumerateInstanceLayerProperties(&layerCount, layers);
		for (uint32_t i = 0; i < layerCount; ++i)
		{
			if (strcmp(layers[i].layerName, validationLayerName) == 0)
				instanceExtensions.validation = true;
			else if (strcmp(layers[i].layerName, standardValidationLayerName) == 0)
				instanceExtensions.standardValidation = true;
			else if (strcmp(layers[i].layerName, coreValLayerName) == 0)
				instanceExtensions.oldValidation = true;
		}
		free(layers);
	}

	uint32_t extensionCount = 0;
	instance->vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
	bool hasSurface = false;
	VkExtensionProperties* extensions =
		(VkExtensionProperties*)malloc(extensionCount*sizeof(VkExtensionProperties));
	if (!extensions)
		return false;

	bool hasPhysicalDeviceProperties2ExtensionName = false;
	bool hasExternalMemoryCapabilitiesExtensionName = false;
	instance->vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		if (strcmp(extensions[i].extensionName, surfaceExtensionName) == 0)
			hasSurface = true;
		else if (strcmp(extensions[i].extensionName, debugExtensionName) == 0)
			instanceExtensions.debug = true;
		else if (strcmp(extensions[i].extensionName, oldDebugReportExtensionName) == 0)
			instanceExtensions.oldDebugReport = true;
		else if (strcmp(extensions[i].extensionName, physicalDeviceProperties2ExtensionName) == 0)
			hasPhysicalDeviceProperties2ExtensionName = true;
		else if (strcmp(extensions[i].extensionName, externalMemoryCapabilitiesExtensionName) == 0)
			hasExternalMemoryCapabilitiesExtensionName = true;
		else if (strcmp(extensions[i].extensionName, xlibDisplayExtensionName) == 0)
			instanceExtensions.xlib = true;
		else if (strcmp(extensions[i].extensionName, waylandDisplayExtensionName) == 0)
			instanceExtensions.wayland = true;
		else if (strcmp(extensions[i].extensionName, win32DisplayExtensionName) == 0)
			instanceExtensions.win32 = true;
		else if (strcmp(extensions[i].extensionName, androidDisplayExtensionName) == 0)
			instanceExtensions.android = true;
	}
	free(extensions);

	if (!hasSurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Vulkan requires surface support.");
		return false;
	}

	if (hasPhysicalDeviceProperties2ExtensionName && hasExternalMemoryCapabilitiesExtensionName)
		instanceExtensions.deviceInfo = true;

	instanceExtensions.initialized = true;
	return true;
}

static void addLayers(const char** layerNames, uint32_t* layerCount, bool useValidation)
{
	if (!useValidation)
		return;

	if (instanceExtensions.validation)
		DS_ADD_EXTENSION(layerNames, *layerCount, validationLayerName);
	else if (instanceExtensions.standardValidation)
		DS_ADD_EXTENSION(layerNames, *layerCount, standardValidationLayerName);
	else if (instanceExtensions.oldValidation)
	{
		// Need to add each validation layer individually for older systems. (e.g. Android)
		DS_ADD_EXTENSION(layerNames, *layerCount, threadingValLayerName);
		DS_ADD_EXTENSION(layerNames, *layerCount, paramValLayerName);
		DS_ADD_EXTENSION(layerNames, *layerCount, objectValLayerName);
		DS_ADD_EXTENSION(layerNames, *layerCount, coreValLayerName);
		DS_ADD_EXTENSION(layerNames, *layerCount, uniqueObjectValLayerName);
	}
}

static void addInstanceExtensions(const char** extensionNames, uint32_t* extensionCount,
	const dsRendererOptions* options, uint32_t instanceVersion)
{
	DS_ADD_EXTENSION(extensionNames, *extensionCount, surfaceExtensionName);
	if (instanceExtensions.xlib)
		DS_ADD_EXTENSION(extensionNames, *extensionCount, xlibDisplayExtensionName);
	if (instanceExtensions.wayland)
		DS_ADD_EXTENSION(extensionNames, *extensionCount, waylandDisplayExtensionName);
	if (instanceExtensions.win32)
		DS_ADD_EXTENSION(extensionNames, *extensionCount, win32DisplayExtensionName);
	if (instanceExtensions.android)
		DS_ADD_EXTENSION(extensionNames, *extensionCount, androidDisplayExtensionName);
	if (instanceExtensions.deviceInfo && instanceVersion < DS_DEVICE_INFO_CORE_VERSION)
	{
		DS_ADD_EXTENSION(extensionNames, *extensionCount, physicalDeviceProperties2ExtensionName);
		DS_ADD_EXTENSION(extensionNames, *extensionCount, externalMemoryCapabilitiesExtensionName);
	}

	// NOTE: Push groups use the debug utils extension, so use it if profiling is enabled.
	if (enableMarkers(enableValidation(options)))
	{
		if (instanceExtensions.debug)
			DS_ADD_EXTENSION(extensionNames, *extensionCount, debugExtensionName);
		else if (instanceExtensions.oldDebugReport)
			DS_ADD_EXTENSION(extensionNames, *extensionCount, oldDebugReportExtensionName);
	}
}

static void findDeviceExtensions(DeviceExtensions* outExtensions, dsVkDevice* device,
	dsAllocator* allocator)
{
	dsVkInstance* instance = &device->instance;
	device->hasMaintenance1 = false;
	device->hasDepthStencilResolve = false;
	device->hasPVRTC = false;
	device->hasLazyAllocation = false;

	uint32_t extensionCount = 0;
	DS_VK_CALL(instance->vkEnumerateDeviceExtensionProperties)(device->physicalDevice, NULL,
		&extensionCount, NULL);
	if (extensionCount == 0)
		return;

	VkExtensionProperties* extensions = DS_ALLOCATE_OBJECT_ARRAY(allocator, VkExtensionProperties,
		extensionCount);
	if (!extensions)
		return;

	instance->vkEnumerateDeviceExtensionProperties(device->physicalDevice, NULL, &extensionCount,
		extensions);
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		if (strcmp(extensions[i].extensionName, maintenance1ExtensionName) == 0)
			outExtensions->maintenance1 = true;
		else if (strcmp(extensions[i].extensionName, oldDebugMarkerExtensionName) == 0)
			outExtensions->oldDebugMarker = true;
		else if (strcmp(extensions[i].extensionName, depthStencilResolveExtensionName) == 0)
			outExtensions->depthStencilResolve = true;
		else if (strcmp(extensions[i].extensionName, pvrtcExtensionName) == 0)
			outExtensions->pvrtc = true;
		else if (strcmp(extensions[i].extensionName, dedicatedAllocationExtensionName) == 0)
			outExtensions->dedicatedAllocation = true;
	}

	dsAllocator_free(allocator, extensions);
}

static void addDeviceExtensions(dsVkDevice* device,
	dsAllocator* allocator, DeviceExtensions* extensions, const char** extensionNames,
	uint32_t* extensionCount, bool useMarkers)
{
	uint32_t apiVersion = device->properties.apiVersion;
	findDeviceExtensions(extensions, device, allocator);
	DS_ADD_EXTENSION(extensionNames, *extensionCount, swapChainExtensionName);
	if (useMarkers && extensions->maintenance1)
	{
		device->hasMaintenance1 = true;
		if (apiVersion < DS_MAINTENANCE1_CORE_VERSION)
			DS_ADD_EXTENSION(extensionNames, *extensionCount, maintenance1ExtensionName);
	}
	if (useMarkers && !instanceExtensions.debug && extensions->oldDebugMarker)
		DS_ADD_EXTENSION(extensionNames, *extensionCount, oldDebugMarkerExtensionName);
	if (extensions->depthStencilResolve)
	{
		device->hasDepthStencilResolve = true;
		if (apiVersion < DS_CREATE_RENDERPASS2_CORE_VERSION)
		{
			DS_ADD_EXTENSION(extensionNames, *extensionCount, maintenance2ExtensionName);
			DS_ADD_EXTENSION(extensionNames, *extensionCount, multiviewExtensionName);
			DS_ADD_EXTENSION(extensionNames, *extensionCount, createRenderPass2ExtensionName);
		}
		if (apiVersion < DS_DEPTH_STENCIL_RESOVE_CORE_VERSION)
			DS_ADD_EXTENSION(extensionNames, *extensionCount, depthStencilResolveExtensionName);
	}
	if (extensions->pvrtc)
	{
		device->hasPVRTC = true;
		DS_ADD_EXTENSION(extensionNames, *extensionCount, pvrtcExtensionName);
	}
	if (extensions->dedicatedAllocation)
	{
		device->hasDedicatedAllocation = true;
		if (apiVersion < DS_DEDICATED_ALLOCATION_CORE_VERSION)
		{
			DS_ADD_EXTENSION(extensionNames, *extensionCount, getMemoryRequirements2ExtensionName);
			DS_ADD_EXTENSION(extensionNames, *extensionCount, dedicatedAllocationExtensionName);
		}
	}
}

static VkPhysicalDevice findPhysicalDevice(dsVkInstance* instance,
	const dsRendererOptions* options)
{
	if (!dsGatherVkPhysicalDevices(instance))
		return NULL;

	dsRenderDeviceType defaultDeviceType = dsRenderDeviceType_Unknown;
	VkPhysicalDevice fallbackDefaultDevice = NULL;
	VkPhysicalDevice defaultDevice = NULL;
	VkPhysicalDevice explicitDevice = NULL;
	VkPhysicalDevice namedDevice = NULL;

	// Find the explicit device.
	uint32_t deviceCount = 0;
	DS_VK_CALL(instance->vkEnumeratePhysicalDevices)(instance->instance, &deviceCount, NULL);
	if (deviceCount > DS_MAX_DEVICES)
	{
		DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "An unusually high number of devices (%u) are "
			"present. Only the first %u devices will be considered.", deviceCount, DS_MAX_DEVICES);
		deviceCount = DS_MAX_DEVICES;
	}

	VkPhysicalDevice devices[DS_MAX_DEVICES];
	DS_VK_CALL(instance->vkEnumeratePhysicalDevices)(instance->instance, &deviceCount, devices);
	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		// Make sure this device supports graphics.
		uint32_t queueFamilyCount = 0;
		DS_VK_CALL(instance->vkGetPhysicalDeviceQueueFamilyProperties)(devices[i],
			&queueFamilyCount, NULL);
		if (queueFamilyCount > DS_MAX_QUEUE_FAMILIES)
		{
			DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "An unusually high number of queue families "
				"(%u) are present. Only the first %u queue families will be considered.",
				queueFamilyCount, DS_MAX_QUEUE_FAMILIES);
			queueFamilyCount = DS_MAX_QUEUE_FAMILIES;
		}

		VkQueueFamilyProperties queueFamilies[DS_MAX_QUEUE_FAMILIES];
		DS_VK_CALL(instance->vkGetPhysicalDeviceQueueFamilyProperties)(devices[i],
			&queueFamilyCount, queueFamilies);
		bool supportsGraphics = false;
		for (uint32_t j = 0; j < queueFamilyCount; ++j)
		{
			if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				supportsGraphics = true;
				break;
			}
		}

		if (!supportsGraphics)
			continue;

		if (instanceExtensions.deviceInfo)
		{
			// Try to find the explicit device as well as the previously found default. (to ensure
			// consistency given no guarantee that the device list remains the same)
			VkPhysicalDeviceIDPropertiesKHR deviceID;
			deviceID.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
			deviceID.pNext = NULL;

			VkPhysicalDeviceProperties2KHR properties2;
			properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
			properties2.pNext = &deviceID;
			DS_VK_CALL(instance->vkGetPhysicalDeviceProperties2)(devices[i], &properties2);

			if (memcmp(deviceID.deviceUUID, extraDeviceInfo[defaultPhysicalDevice].uuid,
					DS_DEVICE_UUID_SIZE) == 0)
			{
				defaultDevice = devices[i];
			}

			if (memcmp(deviceID.deviceUUID, options->deviceUUID, DS_DEVICE_UUID_SIZE) == 0)
				explicitDevice = devices[i];

			if (options->deviceName && !namedDevice &&
					strcasestr(properties2.properties.deviceName, options->deviceName))
			{
				namedDevice = devices[i];
			}
		}

		// Fallback default, used in case the devices have changed for some reason or if device info
		// isn't supported.
		VkPhysicalDeviceProperties properties;
		DS_VK_CALL(instance->vkGetPhysicalDeviceProperties)(devices[i], &properties);

		dsRenderDeviceType deviceType = convertDeviceType(properties.deviceType);
		if (deviceType < defaultDeviceType)
		{
			fallbackDefaultDevice = devices[i];
			defaultDeviceType = deviceType;
		}
	}

	if (explicitDevice)
		return explicitDevice;
	if (namedDevice)
		return namedDevice;
	if (defaultDevice)
		return defaultDevice;
	return fallbackDefaultDevice;
}

static uint32_t findQueueFamily(dsVkInstance* instance, VkPhysicalDevice physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	DS_VK_CALL(instance->vkGetPhysicalDeviceQueueFamilyProperties)(physicalDevice,
		&queueFamilyCount, NULL);
	if (queueFamilyCount > DS_MAX_QUEUE_FAMILIES)
	{
		DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "An unusually high number of queue families "
			"(%u) are present. Only the first %u queue families will be considered.",
			queueFamilyCount, DS_MAX_QUEUE_FAMILIES);
		queueFamilyCount = DS_MAX_QUEUE_FAMILIES;
	}

	VkQueueFamilyProperties queueFamilies[DS_MAX_QUEUE_FAMILIES];
	DS_VK_CALL(instance->vkGetPhysicalDeviceQueueFamilyProperties)(physicalDevice,
		&queueFamilyCount, queueFamilies);
	uint32_t queueFamily = 0;
	VkQueueFlags queueFlags = 0;
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		// Find the graphics queue with the most functionality.
		// TODO: might want to seach for individual bits for a more accurate result, though in
		// practice I would expect there to be only a single graphics queue family anyway.
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
			queueFamilies[i].queueFlags > queueFlags)
		{
			queueFamily = i;
			queueFlags = queueFamilies[i].queueFlags;
		}
	}

	return queueFamily;
}

static bool initializeDeviceList(void)
{
	if (physicalDeviceCount > 0)
		return true;

	dsVkInstance instance;
	memset(&instance, 0, sizeof(dsVkInstance));
	if (!dsCreateVkInstance(&instance, NULL, true))
	{
		dsDestroyVkInstance(&instance);
		return false;
	}

	dsGatherVkPhysicalDevices(&instance);
	dsDestroyVkInstance(&instance);
	return true;
}

bool dsCreateVkInstance(dsVkInstance* instance, const dsRendererOptions* options,
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
#ifdef DS_ALTERNATE_VULKAN_LIBRARY
		if (!dsDynamicLib_open(library, DS_ALTERNATE_VULKAN_LIBRARY))
#endif
		{
			if (handleErrors)
			{
				DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't open vulkan library: %s",
					library->error);
				errno = EPERM;
			}
			return false;
		}
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
	if (!queryInstanceExtensions(instance))
		return false;

	bool wantValidation = enableValidation(options);
	const char* enabledLayers[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledLayerCount = 0;
	addLayers(enabledLayers, &enabledLayerCount, wantValidation);

	const char* enabledExtensions[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledExtensionCount = 0;

	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion =
		(PFN_vkEnumerateInstanceVersion)instance->vkGetInstanceProcAddr(
			NULL, "vkEnumerateInstanceVersion");
	uint32_t instanceVersion = VK_API_VERSION_1_0;
	bool force10 = true;
	if (vkEnumerateInstanceVersion)
	{
		if (vkEnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS)
		{
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't get Vulkan instance version.");
			errno = EPERM;
			return false;
		}

		force10 = false;
	}

	addInstanceExtensions(enabledExtensions, &enabledExtensionCount, options, instanceVersion);

	// Request the highest version we know of.
	uint32_t version = force10 ? VK_API_VERSION_1_0 : DS_HIGHEST_KNOWN_VULKAN_VERSOIN;

	VkApplicationInfo applicationInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		options ? options->applicationName : NULL,
		options ? options->applicationVersion : 0,
		"DeepSea",
		DS_VERSION,
		version
	};

	VkInstanceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		NULL,
		0,
		&applicationInfo,
		enabledLayerCount, enabledLayers,
		enabledExtensionCount, enabledExtensions
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
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't create Vulkan instance"))
			return false;
	}
	else if (result != VK_SUCCESS)
		return false;

	instance->instanceVersion = instanceVersion;
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDestroyInstance);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceQueueFamilyProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumeratePhysicalDevices);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkEnumerateDeviceExtensionProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceQueueFamilyProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceProperties);
	if (instanceExtensions.deviceInfo)
	{
		DS_LOAD_VK_INSTANCE_EXT_FUNCTION(instance, vkGetPhysicalDeviceProperties2,
			DS_DEVICE_INFO_CORE_VERSION, "KHR");
	}
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceFeatures);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceFormatProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCreateDevice);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetDeviceProcAddr);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceMemoryProperties);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceImageFormatProperties);

	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDestroySurfaceKHR);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
	DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);

	instance->debugCallback = 0;
	instance->oldDebugCallback = 0;
	bool wantDebug = wantValidation || DS_PROFILING_ENABLED;
	if (wantDebug && instanceExtensions.debug)
	{
		if (wantValidation)
		{
			DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCreateDebugUtilsMessengerEXT);
			DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDestroyDebugUtilsMessengerEXT);

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
			{
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				NULL,
				0,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				&debugFunc, NULL
			};
			instance->vkCreateDebugUtilsMessengerEXT(instance->instance, &debugCreateInfo,
				instance->allocCallbacksPtr, &instance->debugCallback);
		}

		DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCmdBeginDebugUtilsLabelEXT);
		DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCmdEndDebugUtilsLabelEXT);
	}
	else
	{
		if (wantValidation && instanceExtensions.oldDebugReport)
		{
			DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkCreateDebugReportCallbackEXT);
			DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDestroyDebugReportCallbackEXT);
			DS_LOAD_VK_INSTANCE_FUNCTION(instance, vkDebugReportMessageEXT);

			VkDebugReportCallbackCreateInfoEXT debugCreateInfo =
			{
				VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
				NULL,
				VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
					VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
					VK_DEBUG_REPORT_DEBUG_BIT_EXT,
				&oldDebugFunc, NULL
			};
			instance->vkCreateDebugReportCallbackEXT(instance->instance, &debugCreateInfo,
				instance->allocCallbacksPtr, &instance->oldDebugCallback);
		}
	}

	return true;
}

void dsDestroyVkInstance(dsVkInstance* instance)
{
	// NOTE: errno might get changed by these, overwriting the desired value for initialization.
	int prevErrno = errno;
	if (instance->instance && instance->vkDestroyInstance)
	{
		if (instance->debugCallback)
		{
			instance->vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debugCallback,
				instance->allocCallbacksPtr);
		}
		if (instance->oldDebugCallback)
		{
			instance->vkDestroyDebugReportCallbackEXT(instance->instance,
				instance->oldDebugCallback, instance->allocCallbacksPtr);
		}
		instance->vkDestroyInstance(instance->instance, instance->allocCallbacksPtr);
		instance->instance = NULL;
	}
	dsDynamicLib_close(&instance->library);
	errno = prevErrno;
}

bool dsGatherVkPhysicalDevices(dsVkInstance* instance)
{
	if (physicalDeviceCount > 0)
		return graphicsDeviceCount > 0;

	DS_ASSERT(graphicsDeviceCount == 0);
	dsRenderDeviceType defaultDeviceType = dsRenderDeviceType_Unknown;
	VkPhysicalDevice devices[DS_MAX_DEVICES];
	physicalDeviceCount = DS_MAX_DEVICES;
	DS_VK_CALL(instance->vkEnumeratePhysicalDevices)(instance->instance, &physicalDeviceCount,
		devices);
	for (uint32_t i = 0; i < physicalDeviceCount;)
	{
		DS_VK_CALL(instance->vkGetPhysicalDeviceProperties)(devices[i], physicalDevices + i);

		// Make sure this device supports graphics.
		uint32_t queueFamilyCount = DS_MAX_QUEUE_FAMILIES;
		VkQueueFamilyProperties queueFamilies[DS_MAX_QUEUE_FAMILIES];
		DS_VK_CALL(instance->vkGetPhysicalDeviceQueueFamilyProperties)(devices[i],
			&queueFamilyCount, queueFamilies);
		extraDeviceInfo[i].supportsGraphics = false;
		for (uint32_t j = 0; j < queueFamilyCount; ++j)
		{
			if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				extraDeviceInfo[i].supportsGraphics = true;
				break;
			}
		}

		if (!extraDeviceInfo[i].supportsGraphics)
		{
			++i;
			continue;
		}

		++graphicsDeviceCount;
		dsRenderDeviceType deviceType = convertDeviceType(physicalDevices[i].deviceType);
		if (deviceType < defaultDeviceType)
		{
			defaultPhysicalDevice = i;
			defaultDeviceType = deviceType;
		}

		if (instanceExtensions.deviceInfo)
		{
			VkPhysicalDeviceIDPropertiesKHR deviceID;
			deviceID.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
			deviceID.pNext = NULL;

			VkPhysicalDeviceProperties2KHR properties2;
			properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
			properties2.pNext = &deviceID;
			DS_VK_CALL(instance->vkGetPhysicalDeviceProperties2)(devices[i], &properties2);
			memcpy(extraDeviceInfo[i].uuid, deviceID.deviceUUID, DS_DEVICE_UUID_SIZE);
		}

		++i;
	}

	return graphicsDeviceCount > 0;
}

bool dsQueryVkDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	if (!initializeDeviceList())
		return false;

	if (!outDevices)
	{
		*outDeviceCount = graphicsDeviceCount;
		return true;
	}

	if (*outDeviceCount > graphicsDeviceCount)
		*outDeviceCount = graphicsDeviceCount;

	for (uint32_t i = 0, idx = 0; i < physicalDeviceCount && idx < *outDeviceCount; ++i)
	{
		if (!extraDeviceInfo[i].supportsGraphics)
			continue;

		outDevices[idx].name = physicalDevices[i].deviceName;
		outDevices[idx].vendorID = physicalDevices[i].vendorID;
		outDevices[idx].deviceID = physicalDevices[i].deviceID;
		outDevices[idx].deviceType = convertDeviceType(physicalDevices[i].deviceType);
		outDevices[idx].isDefault = i == defaultPhysicalDevice;
		memcpy(outDevices[idx].deviceUUID, extraDeviceInfo[i].uuid, DS_DEVICE_UUID_SIZE);
		++idx;
	}

	return true;
}

bool dsGetDefaultVkDevice(dsRenderDeviceInfo* outDevice)
{
	if (!outDevice)
	{
		errno = EINVAL;
		return false;
	}

	if (!initializeDeviceList())
		return false;

	const VkPhysicalDeviceProperties* physicalDevice = physicalDevices + defaultPhysicalDevice;
	outDevice->name = physicalDevice->deviceName;
	outDevice->vendorID = physicalDevice->vendorID;
	outDevice->deviceID = physicalDevice->deviceID;
	outDevice->deviceType = convertDeviceType(physicalDevice->deviceType);
	outDevice->isDefault = true;
	memcpy(outDevice->deviceUUID, extraDeviceInfo->uuid, DS_DEVICE_UUID_SIZE);
	return true;
}

bool dsCreateVkDevice(dsVkDevice* device, dsAllocator* allocator, const dsRendererOptions* options)
{
	DS_ASSERT(allocator->freeFunc);
	dsVkInstance* instance = &device->instance;
	device->physicalDevice = findPhysicalDevice(instance, options);
	if (!device->physicalDevice)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Couldn't find a suitable physical device.");
		return false;
	}

	DS_VK_CALL(instance->vkGetPhysicalDeviceFeatures)(device->physicalDevice, &device->features);
	DS_VK_CALL(instance->vkGetPhysicalDeviceProperties)(device->physicalDevice,
		&device->properties);

	// We don't need these features.
	device->features.robustBufferAccess = false;
	device->features.largePoints = false;
	device->features.pipelineStatisticsQuery = false;
	device->features.shaderUniformBufferArrayDynamicIndexing = false;
	device->features.shaderSampledImageArrayDynamicIndexing = false;
	device->features.shaderStorageBufferArrayDynamicIndexing = false;
	device->features.shaderStorageImageArrayDynamicIndexing = false;
	device->features.shaderResourceResidency = false;
	device->features.sparseBinding = false;
	device->features.sparseResidencyBuffer = false;
	device->features.sparseResidencyImage2D = false;
	device->features.sparseResidencyImage3D = false;
	device->features.sparseResidency2Samples = false;
	device->features.sparseResidency4Samples = false;
	device->features.sparseResidency8Samples = false;
	device->features.sparseResidency16Samples = false;
	device->features.sparseResidencyAliased = false;
	device->features.variableMultisampleRate = false;

	float queuePriority = 1.0f;
	device->queueFamilyIndex = findQueueFamily(instance, device->physicalDevice);
	VkDeviceQueueCreateInfo queueCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		NULL,
		0,
		device->queueFamilyIndex,
		1,
		&queuePriority
	};

	bool useValidation = enableValidation(options);
	bool useMarkers = enableMarkers(useValidation);

	const char* enabledLayers[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledLayerCount = 0;
	addLayers(enabledLayers, &enabledLayerCount, useValidation);

	DeviceExtensions extensions;
	memset(&extensions, 0, sizeof(DeviceExtensions));
	const char* enabledExtensions[DS_MAX_ENABLED_EXTENSIONS];
	uint32_t enabledExtensionCount = 0;
	addDeviceExtensions(device, allocator, &extensions, enabledExtensions, &enabledExtensionCount,
		useMarkers);

	VkDeviceCreateInfo deviceCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		NULL,
		0,
		1, &queueCreateInfo,
		enabledLayerCount, enabledLayers,
		enabledExtensionCount, enabledExtensions,
		&device->features
	};
	VkResult result = DS_VK_CALL(instance->vkCreateDevice)(device->physicalDevice,
		&deviceCreateInfo, instance->allocCallbacksPtr, &device->device);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create Vulkan device"))
		return false;

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyDevice);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetDeviceQueue);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateSwapchainKHR);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroySwapchainKHR);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetSwapchainImagesKHR);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkAcquireNextImageKHR);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkQueuePresentKHR);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateCommandPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkResetCommandPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyCommandPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkAllocateCommandBuffers);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkResetCommandBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkFreeCommandBuffers);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkBeginCommandBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkEndCommandBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdExecuteCommands);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdPipelineBarrier);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkQueueSubmit);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkQueueWaitIdle);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateFence);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyFence);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkResetFences);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkWaitForFences);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateSemaphore);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroySemaphore);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkAllocateMemory);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkFreeMemory);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkMapMemory);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkFlushMappedMemoryRanges);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkInvalidateMappedMemoryRanges);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkUnmapMemory);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetBufferMemoryRequirements);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkBindBufferMemory);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdCopyBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdCopyBufferToImage);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdUpdateBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBindVertexBuffers);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBindIndexBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateBufferView);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyBufferView);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateImage);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetImageSubresourceLayout);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyImage);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetImageMemoryRequirements);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkBindImageMemory);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdCopyImage);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdCopyImageToBuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBlitImage);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateImageView);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyImageView);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateFramebuffer);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyFramebuffer);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateRenderPass);
	if (device->hasDepthStencilResolve)
	{
		DS_LOAD_VK_DEVICE_EXT_FUNCTION(device, vkCreateRenderPass2,
			DS_CREATE_RENDERPASS2_CORE_VERSION, "KHR");
	}
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyRenderPass);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBeginRenderPass);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdNextSubpass);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdEndRenderPass);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateQueryPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyQueryPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdResetQueryPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBeginQuery);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdEndQuery);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetQueryPoolResults);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdCopyQueryPoolResults);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdWriteTimestamp);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateShaderModule);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyShaderModule);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateDescriptorSetLayout);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyDescriptorSetLayout);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateDescriptorPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyDescriptorPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkResetDescriptorPool);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkAllocateDescriptorSets);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkFreeDescriptorSets);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkUpdateDescriptorSets);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBindDescriptorSets);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateSampler);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroySampler);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreatePipelineCache);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyPipelineCache);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkGetPipelineCacheData);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreatePipelineLayout);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyPipelineLayout);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateComputePipelines);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCreateGraphicsPipelines);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdBindPipeline);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkDestroyPipeline);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdPushConstants);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetViewport);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetScissor);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetLineWidth);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetBlendConstants);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetDepthBias);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetDepthBounds);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetStencilCompareMask);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetStencilWriteMask);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdSetStencilReference);

	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdClearAttachments);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDraw);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDrawIndexed);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDrawIndirect);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDrawIndexedIndirect);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDispatch);
	DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDispatchIndirect);

	if (useMarkers && !instanceExtensions.debug && extensions.oldDebugMarker)
	{
		DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDebugMarkerBeginEXT);
		DS_LOAD_VK_DEVICE_FUNCTION(device, vkCmdDebugMarkerEndEXT);
	}

	if (extensions.dedicatedAllocation)
	{
		DS_LOAD_VK_DEVICE_EXT_FUNCTION(device, vkGetBufferMemoryRequirements2,
			DS_DEDICATED_ALLOCATION_CORE_VERSION, "KHR");
		DS_LOAD_VK_DEVICE_EXT_FUNCTION(device, vkGetImageMemoryRequirements2,
			DS_DEDICATED_ALLOCATION_CORE_VERSION, "KHR");
	}

	DS_VK_CALL(device->vkGetDeviceQueue)(device->device, device->queueFamilyIndex, 0,
		&device->queue);
	DS_VK_CALL(instance->vkGetPhysicalDeviceMemoryProperties)(device->physicalDevice,
		&device->memoryProperties);

	device->hasLazyAllocation = false;
	for (uint32_t i = 0; i < device->memoryProperties.memoryTypeCount; ++i)
	{
		if (device->memoryProperties.memoryTypes[i].propertyFlags &
			VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
		{
			device->hasLazyAllocation = true;
			break;
		}
	}

	device->buggyDebugLabels = false;
#if !DS_WINDOWS
	// Intel driver on Mesa has buggy debug labels.
	// See https://gitlab.freedesktop.org/mesa/mesa/-/issues/6843
	if (device->properties.vendorID == DS_VENDOR_ID_INTEL)
		device->buggyDebugLabels = true;
#endif

	return true;
}

void dsDestroyVkDevice(dsVkDevice* device)
{
	if (!device->device || !device->vkDestroyDevice)
		return;

	DS_VK_CALL(device->vkDestroyDevice)(device->device, device->instance.allocCallbacksPtr);
	device = NULL;
}
