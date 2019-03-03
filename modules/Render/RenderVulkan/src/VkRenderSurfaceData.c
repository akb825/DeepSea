/*
 * Copyright 2018-2019 Aaron Barany
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

#include "VkRenderSurfaceData.h"

#include "Platform/VkPlatform.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>

#include <string.h>

typedef struct dsVkFormatMap
{
	dsGfxFormat format;
	VkFormat vkFormat;
	VkFormat vkReverseFormat;
} dsVkFormatMap;

static bool hasFormat(const VkSurfaceFormatKHR* surfaceFormats, uint32_t formatCount,
	VkFormat format, VkColorSpaceKHR colorSpace)
{
	for (uint32_t i = 0; i < formatCount; ++i)
	{
		if (surfaceFormats[i].colorSpace == colorSpace && (surfaceFormats[i].format == format ||
			surfaceFormats[i].format == VK_FORMAT_UNDEFINED))
		{
			return true;
		}
	}

	return false;
}

static VkFormat supportsFormat(dsVkDevice* device, VkSurfaceKHR surface, VkFormat format,
	VkColorSpaceKHR colorSpace)
{
	dsVkInstance* instance = &device->instance;

	uint32_t formatCount = 0;
	VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceFormatsKHR)(
		device->physicalDevice, surface, &formatCount, 0);
	if (!dsHandleVkResult(result))
		return 0;

	VkSurfaceFormatKHR* surfaceFormats = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSurfaceFormatKHR,
		formatCount);
	result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceFormatsKHR)(
		device->physicalDevice, surface, &formatCount, surfaceFormats);
	return hasFormat(surfaceFormats, formatCount, format, colorSpace);

	return 0;
}

static bool hasPresentMode(const VkPresentModeKHR* presentModes, uint32_t presentModeCount,
	VkPresentModeKHR mode)
{
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (presentModes[i] == mode)
			return true;
	}

	return false;
}

static VkPresentModeKHR getPresentMode(dsVkDevice* device, VkSurfaceKHR surface, bool vsync)
{
	dsVkInstance* instance = &device->instance;
	uint32_t modeCount = 0;

	DS_VK_CALL(instance->vkGetPhysicalDeviceSurfacePresentModesKHR)(device->physicalDevice, surface,
		&modeCount, NULL);
	if (modeCount == 0)
		return VK_PRESENT_MODE_FIFO_KHR;

	VkPresentModeKHR* presentModes = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkPresentModeKHR, modeCount);
	VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfacePresentModesKHR)(
		device->physicalDevice, surface, &modeCount, presentModes);
	if (result != VK_SUCCESS)
		return VK_PRESENT_MODE_FIFO_KHR;

	if (!vsync)
	{
		if (hasPresentMode(presentModes, modeCount, VK_PRESENT_MODE_MAILBOX_KHR))
			return VK_PRESENT_MODE_MAILBOX_KHR;
		if (hasPresentMode(presentModes, modeCount, VK_PRESENT_MODE_IMMEDIATE_KHR))
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static bool createResolveImage(dsVkRenderSurfaceData* surfaceData, VkFormat format,
	uint32_t width, uint32_t height)
{
	dsRenderer* renderer = surfaceData->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	if (renderer->surfaceSamples <= 1)
		return true;

	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		format,
		{width, height, 1},
		1,
		1,
		dsVkSampleCount(renderer->surfaceSamples),
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &surfaceData->resolveImage);
	if (!dsHandleVkResult(result))
		return false;

	VkMemoryRequirements requirements;
	DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device, surfaceData->resolveImage,
		&requirements);

	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t memoryIndex = dsVkMemoryIndexImpl(device, &requirements, memoryFlags,
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	if (memoryIndex == DS_INVALID_HEAP)
		return false;

	surfaceData->resolveMemory = dsAllocateVkMemory(device, &requirements, memoryIndex);
	if (!surfaceData->resolveMemory)
		return false;

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, surfaceData->resolveImage,
		surfaceData->resolveMemory, 0);
	if (!dsHandleVkResult(result))
		return false;

	VkImageViewCreateInfo imageViewCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		surfaceData->resolveImage,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};

	result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
		instance->allocCallbacksPtr, &surfaceData->resolveImageView);
	return dsHandleVkResult(result);
}

static bool createDepthImage(dsVkRenderSurfaceData* surfaceData, uint32_t width, uint32_t height)
{
	dsRenderer* renderer = surfaceData->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	if (renderer->surfaceDepthStencilFormat == dsGfxFormat_Unknown)
		return true;

	const dsVkFormatInfo* depthFormat = dsVkResourceManager_getFormat(renderer->resourceManager,
		renderer->surfaceDepthStencilFormat);
	if (!depthFormat)
	{
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return false;
	}

	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		depthFormat->vkFormat,
		{width, height, 1},
		1,
		1,
		dsVkSampleCount(renderer->surfaceSamples),
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &surfaceData->depthImage);
	if (!dsHandleVkResult(result))
		return false;

	VkMemoryRequirements requirements;
	DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device, surfaceData->depthImage,
		&requirements);

	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t memoryIndex = dsVkMemoryIndexImpl(device, &requirements, memoryFlags,
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	if (memoryIndex == DS_INVALID_HEAP)
		return false;

	surfaceData->depthMemory = dsAllocateVkMemory(device, &requirements, memoryIndex);
	if (!surfaceData->depthMemory)
		return false;

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, surfaceData->depthImage,
		surfaceData->depthMemory, 0);
	if (!dsHandleVkResult(result))
		return false;

	VkImageViewCreateInfo imageViewCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		surfaceData->depthImage,
		VK_IMAGE_VIEW_TYPE_2D,
		depthFormat->vkFormat,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{dsVkImageAspectFlags(renderer->surfaceDepthStencilFormat), 0, VK_REMAINING_MIP_LEVELS, 0,
			VK_REMAINING_ARRAY_LAYERS}
	};

	result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
		instance->allocCallbacksPtr, &surfaceData->depthImageView);
	return dsHandleVkResult(result);
}

dsVkRenderSurfaceData* dsVkRenderSurfaceData_create(dsAllocator* allocator, dsRenderer* renderer,
	VkSurfaceKHR surface, bool vsync, VkSwapchainKHR prevSwapchain)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;

	const dsVkFormatInfo* colorFormat = dsVkResourceManager_getFormat(renderer->resourceManager,
		renderer->surfaceColorFormat);
	VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	if (!colorFormat)
	{
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return NULL;
	}

	VkBool32 supported = false;
	VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceSupportKHR)(
		device->physicalDevice, device->queueFamilyIndex, surface, &supported);
	if (!dsHandleVkResult(result))
		return NULL;
	if (!supported)
	{
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Window surface can't be rendered to.");
		return NULL;
	}

	VkSurfaceCapabilitiesKHR surfaceInfo;
	result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
		device->physicalDevice, surface, &surfaceInfo);
	if (!dsHandleVkResult(result))
		return NULL;

	if (renderer->stereoscopic && surfaceInfo.maxImageArrayLayers < 2)
	{
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG,
			"Window surface doesn't support stereoscopic rendering.");
		return NULL;
	}

	if (!supportsFormat(device, surface, colorFormat->vkFormat, colorSpace))
	{
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG,
			"Renderer color format not supported by window surface.");
		return NULL;
	}

	VkCompositeAlphaFlagBitsKHR alphaFlags = 0;
	if (vkRenderer->colorSurfaceAlpha &&
		(surfaceInfo.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR))
	{
		alphaFlags = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}
	else if (surfaceInfo.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		alphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	else
	{
		for (uint32_t i = 0; i < 32; ++i)
		{
			if (surfaceInfo.supportedCompositeAlpha & (1 << i))
			{
				alphaFlags = (VkCompositeAlphaFlagBitsKHR)(1 << i);
				break;
			}
		}
	}

	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	uint32_t imageCount = 2;
	// From Adreno programming guide for Vulkan:
	// We recommend using 3 images in the swapchain. Using 2 will reduce memory use, but has the
	// potential for some jittering.
	// Jittering has been confirmed in usage.
	if (renderer->vendorID == DS_VENDOR_ID_QUALCOMM)
		imageCount = 3;
	uint32_t maxImageCount = surfaceInfo.maxImageCount ? surfaceInfo.maxImageCount : UINT_MAX;
	imageCount = dsClamp(imageCount, surfaceInfo.minImageCount, maxImageCount);

	VkSwapchainCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		NULL,
		0,
		surface,
		imageCount,
		colorFormat->vkFormat,
		colorSpace,
		surfaceInfo.currentExtent,
		renderer->stereoscopic ? 2 : 1,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		alphaFlags,
		getPresentMode(device, surface, vsync),
		true,
		prevSwapchain
	};

	VkSwapchainKHR swapchain;
	result = DS_VK_CALL(device->vkCreateSwapchainKHR)(device->device, &createInfo,
		instance->allocCallbacksPtr, &swapchain);
	if (!dsHandleVkResult(result))
		return NULL;

	result = DS_VK_CALL(device->vkGetSwapchainImagesKHR)(device->device, swapchain, &imageCount,
		NULL);
	if (!dsHandleVkResult(result))
	{
		DS_VK_CALL(device->vkDestroySwapchainKHR)(device->device, swapchain,
			instance->allocCallbacksPtr);
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkRenderSurfaceData)) +
		DS_ALIGNED_SIZE(sizeof(VkImage)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(VkImageView)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(dsVkSurfaceImageData)*imageCount);
	if (renderer->stereoscopic)
		fullSize += DS_ALIGNED_SIZE(sizeof(VkImageView)*imageCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		DS_VK_CALL(device->vkDestroySwapchainKHR)(device->device, swapchain,
			instance->allocCallbacksPtr);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVkRenderSurfaceData* surfaceData = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkRenderSurfaceData);
	DS_ASSERT(surfaceData);

	memset(surfaceData, 0, sizeof(*surfaceData));

	surfaceData->allocator = dsAllocator_keepPointer(allocator);
	surfaceData->renderer = renderer;
	dsVkResource_initialize(&surfaceData->resource);

	surfaceData->swapchain = swapchain;
	surfaceData->images = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkImage, imageCount);
	DS_ASSERT(surfaceData->images);
	memset(surfaceData->images, 0, sizeof(VkImage)*imageCount);

	surfaceData->leftImageViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		VkImageView, imageCount);
	DS_ASSERT(surfaceData->leftImageViews);
	memset(surfaceData->leftImageViews, 0, sizeof(VkImageView)*imageCount);

	if (renderer->stereoscopic)
	{
		surfaceData->rightImageViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkImageView, imageCount);
		DS_ASSERT(surfaceData->rightImageViews);
		memset(surfaceData->rightImageViews, 0, sizeof(VkImageView)*imageCount);
	}

	surfaceData->imageData = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		dsVkSurfaceImageData, imageCount);
	DS_ASSERT(surfaceData->imageData);
	memset(surfaceData->imageData, 0, sizeof(dsVkSurfaceImageData)*imageCount);

	surfaceData->imageCount = imageCount;

	result = DS_VK_CALL(device->vkGetSwapchainImagesKHR)(device->device, swapchain, &imageCount,
		surfaceData->images);
	if (!dsHandleVkResult(result))
	{
		dsVkRenderSurfaceData_destroy(surfaceData);
		return NULL;
	}

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		dsVkSurfaceImageData* imageData = surfaceData->imageData + i;

		VkImageViewCreateInfo imageViewCreateInfo =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			NULL,
			0,
			surfaceData->images[i],
			renderer->stereoscopic ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
			colorFormat->vkFormat,
			{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
			{VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1}
		};

		result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
			instance->allocCallbacksPtr, surfaceData->leftImageViews + i);
		if (!dsHandleVkResult(result))
		{
			dsVkRenderSurfaceData_destroy(surfaceData);
			return NULL;
		}

		if (renderer->stereoscopic)
		{
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 1;
			result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
				instance->allocCallbacksPtr, surfaceData->rightImageViews + i);
			if (!dsHandleVkResult(result))
			{
				dsVkRenderSurfaceData_destroy(surfaceData);
				return NULL;
			}
		}

		VkSemaphoreCreateInfo semaphoreCreateInfo =
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			NULL,
			0
		};

		result = DS_VK_CALL(device->vkCreateSemaphore)(device->device, &semaphoreCreateInfo,
			instance->allocCallbacksPtr, &imageData->semaphore);
		if (!dsHandleVkResult(result))
		{
			dsVkRenderSurfaceData_destroy(surfaceData);
			return NULL;
		}

		imageData->lastUsedSubmit = DS_NOT_SUBMITTED;
	}

	surfaceData->vsync = vsync;

	uint32_t width = surfaceInfo.currentExtent.width;
	uint32_t height = surfaceInfo.currentExtent.height;
	if (!createResolveImage(surfaceData, colorFormat->vkFormat, width, height) ||
		!createDepthImage(surfaceData, width, height))
	{
		dsVkRenderSurfaceData_destroy(surfaceData);
		return NULL;
	}

	surfaceData->width = width;
	surfaceData->height = height;

	// Queue processing immediately.
	dsVkRenderer_processRenderSurface(renderer, surfaceData);

	return surfaceData;
}

dsVkSurfaceResult dsVkRenderSurfaceData_acquireImage(dsVkRenderSurfaceData* surfaceData)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = surfaceData->renderer;

	surfaceData->imageDataIndex = (surfaceData->imageDataIndex + 1) % surfaceData->imageCount;
	dsVkSurfaceImageData* imageData = surfaceData->imageData + surfaceData->imageDataIndex;
	if (imageData->lastUsedSubmit != DS_NOT_SUBMITTED)
	{
		dsGfxFenceResult fenceResult = dsVkRenderer_waitForSubmit(renderer,
			imageData->lastUsedSubmit, DS_DEFAULT_WAIT_TIMEOUT);
		if (fenceResult == dsGfxFenceResult_Error)
			DS_PROFILE_FUNC_RETURN(dsVkSurfaceResult_Error);
	}

	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	// NOTE: Would use default timeout, but warns each frame on Android.
	VkResult result = DS_VK_CALL(device->vkAcquireNextImageKHR)(device->device,
		surfaceData->swapchain, UINT64_MAX, imageData->semaphore, 0, &surfaceData->imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		DS_PROFILE_FUNC_RETURN(dsVkSurfaceResult_OutOfDate);
	else if (dsHandleVkResult(result))
		DS_PROFILE_FUNC_RETURN(dsVkSurfaceResult_Success);
	else
		DS_PROFILE_FUNC_RETURN(dsVkSurfaceResult_Error);
}

bool dsVkRenderSurfaceData_clearColor(dsVkRenderSurfaceData* renderSurface, bool rightSurface,
	dsCommandBuffer* commandBuffer, const dsSurfaceColorValue* colorValue)
{
	dsVkDevice* device = &((dsVkRenderer*)renderSurface->renderer)->device;
	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkImageMemoryBarrier barriers[2];
	uint32_t barrierCount = 1;

	VkAccessFlags accessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barriers[0].pNext = NULL;
	barriers[0].srcAccessMask = accessMask;
	barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].image = renderSurface->images[renderSurface->imageIndex];
	barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barriers[0].subresourceRange.baseMipLevel = 0;
	barriers[0].subresourceRange.baseMipLevel = VK_REMAINING_MIP_LEVELS;
	barriers[0].subresourceRange.baseArrayLayer = rightSurface;
	barriers[0].subresourceRange.layerCount = 1;

	if (renderSurface->resolveImage)
	{
		++barrierCount;
		barriers[1] = barriers[0];
		barriers[1].image = renderSurface->resolveImage;
		barriers[1].subresourceRange.baseArrayLayer = 0;
	}

	VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, pipelineStages,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, barrierCount, barriers);

	for (uint32_t i = 0; i < barrierCount; ++i)
	{
		DS_VK_CALL(device->vkCmdClearColorImage)(vkCommandBuffer, barriers[i].image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (const VkClearColorValue*)colorValue, 1,
			&barriers[i].subresourceRange);

		barriers[i].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[i].dstAccessMask = accessMask;
		barriers[i].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barriers[i].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		pipelineStages, 0, 0, NULL, 0, NULL, barrierCount, barriers);

	return true;
}

bool dsVkRenderSurfaceData_clearDepthStencil(dsVkRenderSurfaceData* renderSurface,
	dsCommandBuffer* commandBuffer, dsClearDepthStencil surfaceParts,
	const dsDepthStencilValue* depthStencilValue)
{
	if (!renderSurface->depthImage)
		return true;

	dsRenderer* renderer = renderSurface->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkAccessFlags accessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkImageAspectFlags aspectFlags = dsVkClearDepthStencilImageAspectFlags(
		renderer->surfaceDepthStencilFormat, surfaceParts);

	VkImageMemoryBarrier barrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		accessMask,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		renderSurface->depthImage,
		{aspectFlags, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};

	VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, pipelineStages,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	DS_VK_CALL(device->vkCmdClearDepthStencilImage)(vkCommandBuffer, barrier.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (const VkClearDepthStencilValue*)depthStencilValue, 1,
		&barrier.subresourceRange);

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = accessMask;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	pipelineStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		pipelineStages, 0, 0, NULL, 0, NULL, 1, &barrier);

	return true;
}

void dsVkRenderSurfaceData_destroy(dsVkRenderSurfaceData* surfaceData)
{
	if (!surfaceData)
		return;

	dsVkDevice* device = &((dsVkRenderer*)surfaceData->renderer)->device;
	dsVkInstance* instance = &device->instance;

	if (surfaceData->depthImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, surfaceData->depthImageView,
			instance->allocCallbacksPtr);
	}
	if (surfaceData->depthImage)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, surfaceData->depthImage,
			instance->allocCallbacksPtr);
	}
	if (surfaceData->depthMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, surfaceData->depthMemory,
			instance->allocCallbacksPtr);
	}

	if (surfaceData->resolveImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, surfaceData->resolveImageView,
			instance->allocCallbacksPtr);
	}
	if (surfaceData->resolveImage)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, surfaceData->resolveImage,
			instance->allocCallbacksPtr);
	}
	if (surfaceData->resolveMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, surfaceData->resolveMemory,
			instance->allocCallbacksPtr);
	}

	for (uint32_t i = 0; i < surfaceData->imageCount; ++i)
	{
		if (surfaceData->leftImageViews[i])
		{
			DS_VK_CALL(device->vkDestroyImageView)(device->device, surfaceData->leftImageViews[i],
				instance->allocCallbacksPtr);
		}

		if (surfaceData->rightImageViews && surfaceData->rightImageViews[i])
		{
			DS_VK_CALL(device->vkDestroyImageView)(device->device, surfaceData->rightImageViews[i],
				instance->allocCallbacksPtr);
		}

		const dsVkSurfaceImageData* imageData = surfaceData->imageData + i;
		if (imageData->semaphore)
		{
			DS_VK_CALL(device->vkDestroySemaphore)(device->device, imageData->semaphore,
				instance->allocCallbacksPtr);
		}
	}

	if (surfaceData->swapchain)
	{
		DS_VK_CALL(device->vkDestroySwapchainKHR)(device->device, surfaceData->swapchain,
			instance->allocCallbacksPtr);
	}

	if (surfaceData->allocator)
		DS_VERIFY(dsAllocator_free(surfaceData->allocator, surfaceData));
}
