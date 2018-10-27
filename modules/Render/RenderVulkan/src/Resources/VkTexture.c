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

#include "Resources/VkTexture.h"

#include "Resources/VkResourceManager.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

static size_t fullAllocSize(const dsTextureInfo* info, bool needsHost)
{
	size_t size = DS_ALIGNED_SIZE(sizeof(dsVkTexture));
	if (needsHost)
		size += DS_ALIGNED_SIZE(dsTexture_surfaceCount(info)*sizeof(dsVkHostImage));
	return size;
}

static bool supportsHostImage(dsVkDevice* device, const dsVkFormatInfo* formatInfo,
	VkImageType imageType, const dsTextureInfo* info)
{
	VkImageCreateFlags createFlags =
		info->dimension == dsTextureDim_Cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkImageFormatProperties properties;
	VkResult result = DS_VK_CALL(device->instance.vkGetPhysicalDeviceImageFormatProperties)(
		device->physicalDevice, formatInfo->vkFormat, imageType, VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, createFlags, &properties);
	if (result != VK_SUCCESS)
		return false;

	if (info->dimension == dsTextureDim_3D)
	{
		return info->depth <= properties.maxExtent.depth &&
			info->mipLevels <= properties.maxMipLevels;
	}
	return info->depth <= properties.maxArrayLayers && info->mipLevels <= properties.maxMipLevels;
}

static bool createHostImages(dsVkDevice* device, dsAllocator* allocator, const dsTextureInfo* info,
	const dsVkFormatInfo* formatInfo, VkImageAspectFlags aspectMask,
	VkImageCreateInfo* baseCreateInfo, dsVkTexture* texture, const void* data)
{
	dsVkInstance* instance = &device->instance;
	dsTexture* baseTexture = (dsTexture*)texture;
	VkMemoryRequirements memoryRequirements = {0, 0, 0};

	texture->hostImageCount = dsTexture_surfaceCount(info);
	texture->hostImages = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsVkHostImage,
		texture->hostImageCount);
	DS_ASSERT(texture->hostImages);
	memset(texture->hostImages, 0, texture->hostImageCount*sizeof(dsVkHostImage));

	VkImageLayout initialLayout;
	VkImageUsageFlags hostUsageFlags;
	if (baseTexture->offscreen)
	{
		initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		hostUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else
	{
		initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		hostUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		texture->needsInitialCopy = true;
	}

	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = info->dimension == dsTextureDim_3D;
	if (baseCreateInfo)
	{
		// Single image for all surfaces.
		VkImageCreateInfo imageCreateInfo = *baseCreateInfo;
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage = hostUsageFlags;
		imageCreateInfo.initialLayout = initialLayout;
		VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
			instance->allocCallbacksPtr, &texture->hostImage);
		if (!dsHandleVkResult(result))
			return false;

		DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device,
			texture->hostImage, &memoryRequirements);

		for (uint32_t i = 0, index = 0; i < info->mipLevels; ++i)
		{
			VkSubresourceLayout baseLayout;
			VkImageSubresource subresource = {aspectMask, 0, i};
			DS_VK_CALL(device->vkGetImageSubresourceLayout)(device->device, texture->hostImage,
				&subresource, &baseLayout);

			uint32_t depth = is3D ? info->depth >> i : info->depth;
			for (uint32_t j = 0; j < depth; ++j)
			{
				for (uint32_t k = 0; k < faceCount; ++k, ++index)
				{
					VkSubresourceLayout* imageLayout = &texture->hostImages[index].layout;
					*imageLayout = baseLayout;
					if (is3D)
						imageLayout->offset += j*baseLayout.depthPitch;
					else
						imageLayout->offset += (j*faceCount + k)*baseLayout.arrayPitch;
				}
			}
		}

	}
	else
	{
		// Fall back to a separate image for each surface.
		for (uint32_t i = 0, index = 0; i < info->mipLevels; ++i)
		{
			uint32_t width = info->width >> i;
			uint32_t height = info->height >> i;
			uint32_t depth = is3D ? info->depth >> i : info->depth;
			width = dsMax(1U, width);
			height = dsMax(1U, height);
			depth = dsMax(1U, depth);
			for (uint32_t j = 0; j < depth; ++j)
			{
				for (uint32_t k = 0; k < faceCount; ++k, ++index)
				{
					DS_ASSERT(index < texture->hostImageCount);
					dsVkHostImage* hostImage = texture->hostImages + index;
					VkImageCreateInfo imageCreateInfo =
					{
						VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
						NULL,
						0,
						VK_IMAGE_TYPE_2D,
						formatInfo->vkFormat,
						{width, height, 1},
						info->mipLevels,
						1,
						VK_SAMPLE_COUNT_1_BIT,
						VK_IMAGE_TILING_LINEAR,
						hostUsageFlags,
						VK_SHARING_MODE_EXCLUSIVE,
						1, &device->queueFamilyIndex,
						initialLayout
					};
					VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device,
						&imageCreateInfo, instance->allocCallbacksPtr, &hostImage->image);
					if (!dsHandleVkResult(result))
						return false;

					VkMemoryRequirements imageRequirements;
					DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device,
						hostImage->image, &imageRequirements);

					VkDeviceSize alignment = imageRequirements.alignment;
					memoryRequirements.size =
						((memoryRequirements.size + (alignment - 1))/alignment)*alignment;

					hostImage->offset = (size_t)memoryRequirements.size;
					VkImageSubresource subresource = {aspectMask, 0, 0};
					DS_VK_CALL(device->vkGetImageSubresourceLayout)(device->device,
						hostImage->image, &subresource, &hostImage->layout);

					memoryRequirements.alignment = dsMax(alignment, memoryRequirements.alignment);
					memoryRequirements.size += imageRequirements.size;
					memoryRequirements.memoryTypeBits |= imageRequirements.memoryTypeBits;
				}
			}
		}
	}

	uint32_t memoryIndex = dsVkMemoryIndex(device, &memoryRequirements, 0);
	if (memoryIndex == DS_INVALID_HEAP)
		return false;

	texture->hostMemory = dsAllocateVkMemory(device, &memoryRequirements, memoryIndex);
	if (!texture->hostMemory)
		return false;

	// Share the same block of memory for all host images.
	for (uint32_t i = 0; i < texture->hostImageCount; ++i)
	{
		dsVkHostImage* hostImage = texture->hostImages + i;
		VkResult result = DS_VK_CALL(device->vkBindImageMemory)(device->device, hostImage->image,
			texture->hostMemory, hostImage->offset);
		if (!dsHandleVkResult(result))
			return false;
	}

	// Populate the data.
	if (data)
	{
		const uint8_t* dataBytes = (uint8_t*)data;
		void* hostData;
		VkResult result = DS_VK_CALL(device->vkMapMemory)(device->device, texture->hostMemory, 0,
			VK_WHOLE_SIZE, 0, &hostData);
		if (!dsHandleVkResult(result))
			return false;
		uint8_t* hostBytes = (uint8_t*)hostData;

		unsigned int blockX, blockY;
		if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
			return 0;
		unsigned int formatSize = dsGfxFormat_size(info->format);

		for (uint32_t i = 0, index = 0; i < info->mipLevels; ++i)
		{
			uint32_t width = info->width >> i;
			uint32_t height = info->height >> i;
			uint32_t depth = info->dimension == dsTextureDim_3D ? info->depth >> i : info->depth;
			width = dsMax(1U, width);
			height = dsMax(1U, height);
			depth = dsMax(1U, depth);

			uint32_t xBlocks = (width + blockX - 1)/blockX;
			uint32_t yBlocks = (height + blockY - 1)/blockY;
			uint32_t pitch = xBlocks*formatSize;
			for (uint32_t j = 0; j < depth; ++j)
			{
				for (uint32_t k = 0; k < faceCount; ++k, ++index)
				{
					dsVkHostImage* hostImage = texture->hostImages + index;
					uint8_t* surfaceData = hostBytes + hostImage->offset +
						(size_t)hostImage->layout.offset;
					size_t hostPitch = (size_t)hostImage->layout.rowPitch;
					for (uint32_t y = 0; y < yBlocks; ++y, dataBytes += pitch,
						surfaceData += hostPitch)
					{
						memcpy(surfaceData, dataBytes, pitch);
					}
				}
			}
		}

		DS_VK_CALL(device->vkUnmapMemory)(device->device, texture->hostMemory);
	}

	return true;
}

static bool createSurfaceImage(dsVkDevice* device, const dsTextureInfo* info,
	const dsVkFormatInfo* formatInfo, VkImageAspectFlags aspectMask, dsOffscreenResolve resolve,
	VkImageType imageType, VkImageViewType imageViewType, dsVkTexture* texture)
{
	dsVkInstance* instance = &device->instance;
	uint32_t mipLevels = resolve == dsOffscreenResolve_ResolveSingle ? 1 : info->mipLevels;
	uint32_t depthCount = dsMax(1U, info->depth);
	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (dsGfxFormat_isDepthStencil(info->format))
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	else
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		info->dimension == dsTextureDim_Cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
		imageType,
		formatInfo->vkFormat,
		{info->width, info->height, info->dimension == dsTextureDim_3D ? info->depth : 1},
		mipLevels,
		info->dimension == dsTextureDim_3D ? 1 : depthCount*faceCount,
		resolve ? VK_SAMPLE_COUNT_1_BIT : dsVkSampleCount(info->samples),
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		1, &device->queueFamilyIndex,
		VK_IMAGE_LAYOUT_GENERAL
	};
	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &texture->surfaceImage);
	if (!dsHandleVkResult(result))
		return false;

	VkMemoryRequirements surfaceRequirements;
	DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device, texture->surfaceImage,
		&surfaceRequirements);
	uint32_t surfaceMemoryIndex = dsVkMemoryIndex(device, &surfaceRequirements, dsGfxMemory_GPUOnly);
	if (surfaceMemoryIndex == DS_INVALID_HEAP)
		return false;

	texture->deviceMemory = dsAllocateVkMemory(device, &surfaceRequirements, surfaceMemoryIndex);
	if (!texture->deviceMemory)
		return false;

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, texture->surfaceImage,
		texture->surfaceMemory, 0);
	if (!dsHandleVkResult(result))
		return false;

	VkImageViewCreateInfo imageViewCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		texture->surfaceImage,
		imageViewType,
		formatInfo->vkFormat,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};
	result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
		instance->allocCallbacksPtr, &texture->surfaceImageView);
	return dsHandleVkResult(result);
}

static dsTexture* createTextureImpl(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size, bool offscreen, dsOffscreenResolve resolve)
{
	DS_ASSERT(size == 0 || size == dsTexture_size(info));
	DS_UNUSED(size);

	dsVkRenderer* renderer = (dsVkRenderer*)resourceManager->renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, info->format);
	if (!formatInfo)
	{
		errno = EINVAL;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return NULL;
	}

	bool needsHostMemory = data || (offscreen && (info->samples == 1 || resolve) &&
		(memoryHints & dsGfxMemory_Read));
	if (needsHostMemory && dsGfxFormat_isDepthStencil(info->format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
			"Cannot acces depth/stencil format texture data from the host.");
		return NULL;
	}

	VkImageType imageType;
	VkImageViewType imageViewType;
	switch (info->dimension)
	{
		case dsTextureDim_1D:
			imageType = VK_IMAGE_TYPE_1D;
			if (info->depth > 0)
				imageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			else
				imageViewType = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case dsTextureDim_2D:
			imageType = VK_IMAGE_TYPE_2D;
			if (info->depth > 0)
				imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			else
				imageViewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case dsTextureDim_3D:
			imageType = VK_IMAGE_TYPE_3D;
			imageViewType = VK_IMAGE_VIEW_TYPE_3D;
			break;
		case dsTextureDim_Cube:
			imageType = VK_IMAGE_TYPE_2D;
			if (info->depth > 0)
				imageViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			else
				imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		default:
			DS_ASSERT(false);
			return NULL;
	}

	bool singleHostImage = true;
	if (needsHostMemory)
		singleHostImage = supportsHostImage(device, formatInfo, imageType, info);

	size_t bufferSize = fullAllocSize(info, needsHostMemory);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkTexture* texture = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkTexture);
	DS_ASSERT(texture);

	memset(texture, 0, sizeof(*texture));

	dsTexture* baseTexture = (dsTexture*)texture;
	baseTexture->resourceManager = resourceManager;
	baseTexture->allocator = dsAllocator_keepPointer(allocator);
	baseTexture->usage = usage;
	baseTexture->memoryHints = memoryHints;
	baseTexture->info = *info;
	baseTexture->offscreen = offscreen;
	baseTexture->resolve = resolve;

	DS_VERIFY(dsSpinlock_initialize(&texture->lock));

	// Base flags determined from the usage flags passed in.
	VkImageUsageFlags usageFlags = 0;
	if (usage & dsTextureUsage_Texture)
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usage & dsTextureUsage_Image)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (usage & dsTextureUsage_CopyFrom)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (usage & dsTextureUsage_CopyTo || data)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (offscreen)
	{
		if (dsGfxFormat_isDepthStencil(info->format))
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	VkImageAspectFlags aspectMask;
	switch (info->format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
		case dsGfxFormat_D32_Float:
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		case dsGfxFormat_S8:
			aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		default:
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
	}

	// Create device image for general usage.
	if (needsHostMemory || resolve)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	uint32_t depthCount = dsMax(1U, info->depth);
	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		info->dimension == dsTextureDim_Cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
		imageType,
		formatInfo->vkFormat,
		{info->width, info->height, info->dimension == dsTextureDim_3D ? info->depth : 1},
		info->mipLevels,
		info->dimension == dsTextureDim_3D ? 1 : depthCount*faceCount,
		resolve ? VK_SAMPLE_COUNT_1_BIT : dsVkSampleCount(info->samples),
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		1, &device->queueFamilyIndex,
		VK_IMAGE_LAYOUT_GENERAL
	};
	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &texture->deviceImage);
	if (!dsHandleVkResult(result))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	VkMemoryRequirements deviceRequirements;
	DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device, texture->deviceImage,
		&deviceRequirements);
	uint32_t deviceMemoryIndex = dsVkMemoryIndex(device, &deviceRequirements, dsGfxMemory_GPUOnly);
	if (deviceMemoryIndex == DS_INVALID_HEAP)
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	texture->deviceMemory = dsAllocateVkMemory(device, &deviceRequirements, deviceMemoryIndex);
	if (!texture->deviceMemory)
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, texture->deviceImage,
		texture->deviceMemory, 0);
	if (!dsHandleVkResult(result))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	VkImageViewCreateInfo imageViewCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		texture->deviceImage,
		imageViewType,
		formatInfo->vkFormat,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};
	result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
		instance->allocCallbacksPtr, &texture->deviceImageView);

	if (needsHostMemory && !createHostImages(device, (dsAllocator*)&bufferAlloc, info, formatInfo,
		aspectMask, singleHostImage ? &imageCreateInfo : NULL, texture, data))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	if (resolve != dsOffscreenResolve_NoResolve && !createSurfaceImage(device, info, formatInfo,
		aspectMask, resolve, imageType, imageViewType, texture))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	return baseTexture;
}

dsTexture* dsVkTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size)
{
	return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, data, size,
		false, dsOffscreenResolve_NoResolve);
}

dsOffscreen* dsVkTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info,
	dsOffscreenResolve resolve)
{
	return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, NULL, 0, true,
		resolve);
}

void dsVkTexture_destroyImpl(dsTexture* texture)
{
	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	dsVkDevice* device = &((dsVkRenderer*)texture->resourceManager->renderer)->device;

	dsVkInstance* instance = &device->instance;
	if (vkTexture->deviceImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, vkTexture->deviceImageView,
			instance->allocCallbacksPtr);
	}
	if (vkTexture->deviceImage)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, vkTexture->deviceImage,
			instance->allocCallbacksPtr);
	}
	if (vkTexture->deviceMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, vkTexture->deviceMemory,
			instance->allocCallbacksPtr);
	}

	if (vkTexture->hostImage)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, vkTexture->hostImage,
			instance->allocCallbacksPtr);
	}
	for (uint32_t i = 0; i < vkTexture->hostImageCount; ++i)
	{
		dsVkHostImage* hostImage = vkTexture->hostImages + i;
		if (hostImage->image)
		{
			DS_VK_CALL(device->vkDestroyImage)(device->device, vkTexture->deviceImage,
				instance->allocCallbacksPtr);
		}
	}
	if (vkTexture->hostMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, vkTexture->hostMemory,
			instance->allocCallbacksPtr);
	}

	if (vkTexture->surfaceImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, vkTexture->surfaceImageView,
			instance->allocCallbacksPtr);
	}
	if (vkTexture->surfaceImage)
	{
		DS_VK_CALL(device->vkDestroyImage)(device->device, vkTexture->surfaceImage,
			instance->allocCallbacksPtr);
	}
	if (vkTexture->surfaceMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, vkTexture->surfaceMemory,
			instance->allocCallbacksPtr);
	}

	dsSpinlock_shutdown(&vkTexture->lock);
	if (texture->allocator)
		dsAllocator_free(texture->allocator, texture);
}
