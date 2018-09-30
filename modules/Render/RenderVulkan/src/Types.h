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

#pragma once

#include <DeepSea/Core/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderVulkan/RendererIDs.h>
#include <vulkan/vulkan_core.h>

#define DS_NOT_SUBMITTED (uint64_t)-1

typedef struct dsVkInstance
{
	dsDynamicLib library;

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

	VkDebugReportCallbackEXT debugCallback;

	VkAllocationCallbacks allocCallbacks;
	const VkAllocationCallbacks* allocCallbacksPtr;
	VkInstance instance;
} dsVkInstance;

typedef struct dsVkDevice
{
	dsVkInstance instance;

	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
	PFN_vkUnmapMemory vkUnmapMemory;
	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkCreateBufferView vkCreateBufferView;
	PFN_vkDestroyBufferView vkDestroyBufferView;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t queueFamilyIndex;

	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	bool hasPVRTC;

	VkPhysicalDeviceMemoryProperties memoryProperties;
} dsVkDevice;

typedef struct dsVkFormatInfo
{
	VkFormat vkFormat;
	VkFormatProperties properties;
} dsVkFormatInfo;

typedef struct dsVkGfxBufferData
{
	dsAllocator* allocator;
	VkDeviceMemory deviceMemory;
	VkBuffer deviceBuffer;
	uint64_t lastUsedSubmit;

	VkDeviceMemory hostMemory;
	VkBuffer hostBuffer;
	uint64_t uploadedSubmit;
	bool needsUpload;
	bool keepHost;
} dsVkGfxBufferData;

typedef struct dsVkGfxBuffer
{
	dsGfxBuffer buffer;
	dsSpinlock lock;
	dsVkGfxBufferData* bufferData;
} dsVkGfxBuffer;

typedef struct dsVkRenderer
{
	dsRenderer renderer;
	dsVkDevice device;
} dsVkRenderer;

typedef struct dsVkResourceManager
{
	dsResourceManager resourceManager;
	dsVkDevice* device;

	dsVkFormatInfo standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	dsVkFormatInfo specialFormats[dsGfxFormat_SpecialCount];
	dsVkFormatInfo compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];
} dsVkResourceManager;
