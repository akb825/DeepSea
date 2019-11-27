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

#include "Resources/VkRenderbuffer.h"

#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

dsRenderbuffer* dsVkRenderbuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsRenderbufferUsage usage, dsGfxFormat format, uint32_t width, uint32_t height,
	uint32_t samples)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return NULL;
	}

	dsVkRenderbuffer* renderbuffer = DS_ALLOCATE_OBJECT(allocator, dsVkRenderbuffer);
	if (!renderbuffer)
		return NULL;

	dsRenderbuffer* baseRenderbuffer = (dsRenderbuffer*)renderbuffer;
	baseRenderbuffer->resourceManager = resourceManager;
	baseRenderbuffer->allocator = dsAllocator_keepPointer(allocator);
	baseRenderbuffer->usage = usage;
	baseRenderbuffer->format = format;
	baseRenderbuffer->width = width;
	baseRenderbuffer->height = height;
	baseRenderbuffer->samples = samples;

	dsVkResource_initialize(&renderbuffer->resource);
	renderbuffer->memory = 0;
	renderbuffer->image = 0;
	renderbuffer->imageView = 0;

	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	bool isDepthStencil = dsGfxFormat_isDepthStencil(format);
	VkImageUsageFlags usageFlags = 0;
	if (usage & dsRenderbufferUsage_BlitFrom)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (usage & dsRenderbufferUsage_BlitTo)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (isDepthStencil)
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	else
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (device->hasLazyAllocation && dsVkImageUsageSupportsTransient(usageFlags))
		usageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		formatInfo->vkFormat,
		{width, height, 1},
		1,
		1,
		dsVkSampleCount(samples),
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &renderbuffer->image);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image"))
	{
		dsVkRenderbuffer_destroyImpl(baseRenderbuffer);
		return NULL;
	}

	VkMemoryRequirements surfaceRequirements;
	DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device, renderbuffer->image,
		&surfaceRequirements);

	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t surfaceMemoryIndex = dsVkMemoryIndexImpl(device, &surfaceRequirements, memoryFlags,
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	if (surfaceMemoryIndex == DS_INVALID_HEAP)
	{
		dsVkRenderbuffer_destroyImpl(baseRenderbuffer);
		return NULL;
	}

	renderbuffer->memory = dsAllocateVkMemory(device, &surfaceRequirements, surfaceMemoryIndex);
	if (!renderbuffer->memory)
	{
		dsVkRenderbuffer_destroyImpl(baseRenderbuffer);
		return NULL;
	}

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, renderbuffer->image,
		renderbuffer->memory, 0);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't bind image memory"))
	{
		dsVkRenderbuffer_destroyImpl(baseRenderbuffer);
		return NULL;
	}

	VkImageViewCreateInfo imageViewCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		renderbuffer->image,
		VK_IMAGE_VIEW_TYPE_2D,
		formatInfo->vkFormat,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{dsVkImageAspectFlags(format), 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};
	result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
		instance->allocCallbacksPtr, &renderbuffer->imageView);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image view"))
	{
		dsVkRenderbuffer_destroyImpl(baseRenderbuffer);
		return NULL;
	}

	// Queue processing immediately.
	dsVkRenderer_processRenderbuffer(resourceManager->renderer, baseRenderbuffer);

	return baseRenderbuffer;
}

bool dsVkRenderbuffer_destroy(dsResourceManager* resourceManager, dsRenderbuffer* renderbuffer)
{
	dsVkRenderer_deleteRenderbuffer(resourceManager->renderer, renderbuffer);
	return true;
}

void dsVkRenderbuffer_destroyImpl(dsRenderbuffer* renderbuffer)
{
	dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;
	dsVkDevice* device = &((dsVkRenderer*)renderbuffer->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	if (vkRenderbuffer->imageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, vkRenderbuffer->imageView,
			instance->allocCallbacksPtr);
	}
	if (vkRenderbuffer->image)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, vkRenderbuffer->image,
			instance->allocCallbacksPtr);
	}
	if (vkRenderbuffer->memory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, vkRenderbuffer->memory,
			instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&vkRenderbuffer->resource);
	if (renderbuffer->allocator)
		dsAllocator_free(renderbuffer->allocator, renderbuffer);
}
