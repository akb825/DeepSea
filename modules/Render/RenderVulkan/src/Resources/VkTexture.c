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

#include "Resources/VkTexture.h"

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

inline static void adjustAlignment(size_t alignment, VkDeviceSize totalSize, VkDeviceSize* offset,
	VkDeviceSize* size, size_t* rem)
{
	*rem = (size_t)(*offset % alignment);
	*offset -= *rem;
	*size += *rem;

	VkDeviceSize count = (*size + alignment - 1)/alignment;
	*size = count*alignment;
	*size = dsMin(*size, totalSize - *offset);
}

static bool isCombinedDepthStencil(dsGfxFormat format)
{
	return format == dsGfxFormat_D16S8 || format == dsGfxFormat_D24S8 ||
		format == dsGfxFormat_D32S8_Float;
}

static bool createHostImageBuffer(dsVkDevice* device, dsVkTexture* texture, const void* data,
	size_t dataSize)
{
	dsVkInstance* instance = &device->instance;
	dsTexture* baseTexture = (dsTexture*)texture;

	VkBufferCreateInfo bufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		dataSize,
		baseTexture->offscreen ? VK_BUFFER_USAGE_TRANSFER_DST_BIT :
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL
	};

	VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
		instance->allocCallbacksPtr, &texture->hostBuffer);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create buffer"))
		return false;

	VkMemoryRequirements memoryRequirements;
	VkBuffer dedicatedBuffer;
	dsVkGetBufferMemoryRequirements(device, texture->hostBuffer, &memoryRequirements,
		&dedicatedBuffer);
	uint32_t memoryIndex = dsVkMemoryIndex(device, &memoryRequirements, 0);
	if (memoryIndex == DS_INVALID_HEAP)
		return false;

	texture->hostMemory = dsAllocateVkMemory(device, &memoryRequirements, memoryIndex, 0,
		dedicatedBuffer);
	if (!texture->hostMemory)
		return false;

	texture->hostMemorySize = dataSize;
	texture->hostMemoryCoherent = dsVkHeapIsCoherent(device, memoryIndex);

	result = DS_VK_CALL(device->vkBindBufferMemory)(device->device, texture->hostBuffer,
		texture->hostMemory, 0);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't bind buffer memory"))
		return false;

	// Populate the data.
	if (data)
	{
		void* hostData;
		VkResult result = DS_VK_CALL(device->vkMapMemory)(device->device, texture->hostMemory, 0,
			VK_WHOLE_SIZE, 0, &hostData);
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't map buffer memory"))
			return false;

		memcpy(hostData, data, dataSize);

		if (!texture->hostMemoryCoherent)
		{
			VkMappedMemoryRange range =
			{
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				NULL,
				texture->hostMemory,
				0,
				VK_WHOLE_SIZE
			};
			DS_VK_CALL(device->vkFlushMappedMemoryRanges)(device->device, 1, &range);
		}
		DS_VK_CALL(device->vkUnmapMemory)(device->device, texture->hostMemory);
	}

	return true;
}

static bool createSurfaceImage(dsVkDevice* device, const dsTextureInfo* info,
	const dsVkFormatInfo* formatInfo, VkImageAspectFlags aspectMask, dsVkTexture* texture)
{
	dsVkInstance* instance = &device->instance;
	VkImageUsageFlags usageFlags = 0;
	if (dsGfxFormat_isDepthStencil(info->format))
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	else
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (device->hasLazyAllocation && dsVkImageUsageSupportsTransient(usageFlags))
		usageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

	VkImageType imageType;
	VkImageViewType imageViewType;
	if (info->dimension == dsTextureDim_1D)
	{
		imageType = VK_IMAGE_TYPE_1D;
		imageViewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else
	{
		imageType = VK_IMAGE_TYPE_2D;
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	}

	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		imageType,
		formatInfo->vkFormat,
		{info->width, info->height, 1},
		1,
		1,
		dsVkSampleCount(info->samples),
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &texture->surfaceImage);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image"))
		return false;

	VkMemoryRequirements surfaceRequirements;
	VkImage dedicatedImage;
	dsVkGetImageMemoryRequirements(device, texture->surfaceImage, &surfaceRequirements,
		&dedicatedImage);

	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t surfaceMemoryIndex = dsVkMemoryIndexImpl(device, &surfaceRequirements, memoryFlags,
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	if (surfaceMemoryIndex == DS_INVALID_HEAP)
		return false;

	texture->surfaceMemory = dsAllocateVkMemory(device, &surfaceRequirements, surfaceMemoryIndex,
		dedicatedImage, 0);
	if (!texture->surfaceMemory)
		return false;

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, texture->surfaceImage,
		texture->surfaceMemory, 0);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't bind image memory"))
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
	return DS_HANDLE_VK_RESULT(result, "Couldn't create image view");
}

static dsTexture* createTextureImpl(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size, bool offscreen, bool resolve)
{
	size_t textureSize = dsTexture_size(info);
	DS_ASSERT(size == 0 || size == textureSize);
	DS_UNUSED(size);

	dsVkRenderer* renderer = (dsVkRenderer*)resourceManager->renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, info->format);
	if (!formatInfo)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return NULL;
	}

	bool needsHostMemory = data || (offscreen && (info->samples == 1 || resolve) &&
		(usage & dsTextureUsage_CopyFrom) && (memoryHints & dsGfxMemory_Read));
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

	dsVkTexture* texture = DS_ALLOCATE_OBJECT(allocator, dsVkTexture);
	if (!texture)
		return NULL;

	memset(texture, 0, sizeof(*texture));
	dsVkResource_initialize(&texture->resource);

	dsTexture* baseTexture = (dsTexture*)texture;
	baseTexture->resourceManager = resourceManager;
	baseTexture->allocator = dsAllocator_keepPointer(allocator);
	baseTexture->usage = usage;
	baseTexture->memoryHints = memoryHints;
	baseTexture->info = *info;
	baseTexture->offscreen = offscreen;
	baseTexture->resolve = resolve;

	texture->lifetime = dsLifetime_create(allocator, baseTexture);
	if (!texture->lifetime)
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	// Base flags determined from the usage flags passed in.
	VkImageUsageFlags usageFlags = 0;
	if (usage & dsTextureUsage_Texture)
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usage & dsTextureUsage_Image)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if ((usage & dsTextureUsage_CopyFrom))
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if ((usage & dsTextureUsage_CopyTo) || data)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (usage & dsTextureUsage_SubpassInput)
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	if (offscreen)
	{
		if (dsGfxFormat_isDepthStencil(info->format))
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (device->hasLazyAllocation && dsVkImageUsageSupportsTransient(usageFlags))
		usageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);

	// Create device image for general usage.
	uint32_t depthCount = dsMax(1U, info->depth);
	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	VkImageCreateFlags flags = 0;
	if (info->dimension == dsTextureDim_Cube)
		flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	else if (offscreen && info->dimension == dsTextureDim_3D)
		flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		flags,
		imageType,
		formatInfo->vkFormat,
		{info->width, info->height, info->dimension == dsTextureDim_3D ? info->depth : 1},
		info->mipLevels,
		info->dimension == dsTextureDim_3D ? 1 : depthCount*faceCount,
		resolve ? VK_SAMPLE_COUNT_1_BIT : dsVkSampleCount(info->samples),
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device, &imageCreateInfo,
		instance->allocCallbacksPtr, &texture->deviceImage);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image"))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	VkMemoryRequirements deviceRequirements;
	VkImage dedicatedImage;
	dsVkGetImageMemoryRequirements(device, texture->deviceImage, &deviceRequirements,
		&dedicatedImage);

	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t deviceMemoryIndex = dsVkMemoryIndexImpl(device, &deviceRequirements, memoryFlags,
		VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	if (deviceMemoryIndex == DS_INVALID_HEAP)
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	texture->deviceMemory = dsAllocateVkMemory(device, &deviceRequirements, deviceMemoryIndex,
		dedicatedImage, 0);
	if (!texture->deviceMemory)
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	result = DS_VK_CALL(device->vkBindImageMemory)(device->device, texture->deviceImage,
		texture->deviceMemory, 0);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't bind image memory"))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	if (usage & (dsTextureUsage_Texture | dsTextureUsage_Image | dsTextureUsage_SubpassInput))
	{
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
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image view"))
		{
			dsVkTexture_destroyImpl(baseTexture);
			return NULL;
		}

		if ((usage & dsTextureUsage_Texture) && isCombinedDepthStencil(info->format))
		{
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			result = DS_VK_CALL(device->vkCreateImageView)(device->device, &imageViewCreateInfo,
				instance->allocCallbacksPtr, &texture->depthOnlyImageView);
			if (!DS_HANDLE_VK_RESULT(result, "Couldn't create image view"))
			{
				dsVkTexture_destroyImpl(baseTexture);
				return NULL;
			}
		}
	}

	if (needsHostMemory && !createHostImageBuffer(device, texture, data, textureSize))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	if (resolve && !createSurfaceImage(device, info, formatInfo, aspectMask, texture))
	{
		dsVkTexture_destroyImpl(baseTexture);
		return NULL;
	}

	texture->needsInitialCopy = true;
	texture->lastDrawSubmit = DS_NOT_SUBMITTED;
	texture->aspectMask = aspectMask;
	return baseTexture;
}

static bool addCopyImageBarriers(dsCommandBuffer* commandBuffer, const dsTextureCopyRegion* regions,
	uint32_t regionCount, dsTexture* srcTexture, dsTexture* dstTexture, bool reverse)
{
	dsVkTexture* srcVkTexture = (dsVkTexture*)srcTexture;
	dsVkTexture* dstVkTexture = (dsVkTexture*)dstTexture;

	VkImageAspectFlags srcAspectMask = dsVkImageAspectFlags(srcTexture->info.format);
	uint32_t srcFaceCount = srcTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;
	bool srcIsDepthStencil = dsGfxFormat_isDepthStencil(srcTexture->info.format);
	VkAccessFlags srcAccessFlags = dsVkReadImageAccessFlags(srcTexture->usage) |
		dsVkWriteImageAccessFlags(srcTexture->usage, srcTexture->offscreen, srcIsDepthStencil);

	VkImageAspectFlags dstAspectMask = dsVkImageAspectFlags(dstTexture->info.format);
	uint32_t dstFaceCount = dstTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;
	bool dstIsDepthStencil = dsGfxFormat_isDepthStencil(dstTexture->info.format);
	VkAccessFlags dstAccessFlags = dsVkReadImageAccessFlags(dstTexture->usage) |
		dsVkWriteImageAccessFlags(dstTexture->usage, dstTexture->offscreen, dstIsDepthStencil);

	VkImageLayout srcMainLayout = dsVkTexture_imageLayout(srcTexture);
	VkImageLayout srcLayout = srcTexture == dstTexture ? VK_IMAGE_LAYOUT_GENERAL :
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkImageLayout dstMainLayout = dsVkTexture_imageLayout(dstTexture);
	VkImageLayout dstLayout = srcTexture == dstTexture ? VK_IMAGE_LAYOUT_GENERAL :
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsTextureCopyRegion* region = regions + i;

		const dsTexturePosition* srcPosition = &region->srcPosition;
		uint32_t srcLayers, srcBaseLayer;
		if (srcIs3D)
		{
			srcLayers = 1;
			srcBaseLayer = 0;
		}
		else
		{
			srcLayers = region->layers;
			srcBaseLayer = srcPosition->depth*srcFaceCount + srcPosition->face;
		}

		VkImageMemoryBarrier* barrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!barrier)
			return false;

		barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier->pNext = NULL;
		if (reverse)
		{
			barrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier->dstAccessMask = srcAccessFlags;
			barrier->oldLayout = srcLayout;
			barrier->newLayout = srcMainLayout;
		}
		else
		{
			barrier->srcAccessMask = srcAccessFlags;
			barrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier->oldLayout = srcMainLayout;
			barrier->newLayout = srcLayout;
		}
		barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->image = srcVkTexture->deviceImage;
		barrier->subresourceRange.aspectMask = srcAspectMask;
		barrier->subresourceRange.baseMipLevel = srcPosition->mipLevel;
		barrier->subresourceRange.levelCount = 1;
		barrier->subresourceRange.baseArrayLayer = srcBaseLayer;
		barrier->subresourceRange.layerCount = srcLayers;

		const dsTexturePosition* dstPosition = &region->dstPosition;
		uint32_t dstLayers, dstBaseLayer;
		if (dstIs3D)
		{
			dstLayers = 1;
			dstBaseLayer = 0;
		}
		else
		{
			dstLayers = region->layers;
			dstBaseLayer = dstPosition->depth*dstFaceCount + dstPosition->face;
		}

		barrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!barrier)
			return false;

		barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier->pNext = NULL;
		if (reverse)
		{
			barrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->dstAccessMask = dstAccessFlags;
			barrier->oldLayout = dstLayout;
			barrier->newLayout = dstMainLayout;
		}
		else
		{
			barrier->srcAccessMask = dstAccessFlags;
			barrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->oldLayout = dstMainLayout;
			barrier->newLayout = dstLayout;
		}
		barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->image = dstVkTexture->deviceImage;
		barrier->subresourceRange.aspectMask = dstAspectMask;
		barrier->subresourceRange.baseMipLevel = dstPosition->mipLevel;
		barrier->subresourceRange.levelCount = 1;
		barrier->subresourceRange.baseArrayLayer = dstBaseLayer;
		barrier->subresourceRange.layerCount = dstLayers;
	}

	return true;
}

static bool addCopyToBufferBarriers(dsCommandBuffer* commandBuffer,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount, dsTexture* srcTexture,
	dsVkGfxBufferData* dstBufferData, bool dstCanMap, bool reverse)
{
	dsVkTexture* srcVkTexture = (dsVkTexture*)srcTexture;

	VkImageAspectFlags srcAspectMask = dsVkImageAspectFlags(srcTexture->info.format);
	uint32_t srcFaceCount = srcTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;
	bool srcIsDepthStencil = dsGfxFormat_isDepthStencil(srcTexture->info.format);
	VkAccessFlags srcAccessFlags = dsVkReadImageAccessFlags(srcTexture->usage) |
		dsVkWriteImageAccessFlags(srcTexture->usage, srcTexture->offscreen, srcIsDepthStencil);

	VkImageLayout srcMainLayout = dsVkTexture_imageLayout(srcTexture);

	VkAccessFlags dstAccessFlags = dsVkWriteBufferAccessFlags(dstBufferData->usage, dstCanMap) |
		dsVkReadBufferAccessFlags(dstBufferData->usage);
	VkBuffer dstVkBuffer = dsVkGfxBufferData_getBuffer(dstBufferData);

	unsigned int formatSize = dsGfxFormat_size(srcTexture->info.format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, srcTexture->info.format));

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;

		const dsTexturePosition* srcPosition = &region->texturePosition;
		uint32_t srcLayers, srcBaseLayer;
		if (srcIs3D)
		{
			srcLayers = 1;
			srcBaseLayer = 0;
		}
		else
		{
			srcLayers = region->layers;
			srcBaseLayer = srcPosition->depth*srcFaceCount + srcPosition->face;
		}

		VkImageMemoryBarrier* barrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!barrier)
			return false;

		barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier->pNext = NULL;
		if (reverse)
		{
			barrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier->dstAccessMask = srcAccessFlags;
			barrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier->newLayout = srcMainLayout;
		}
		else
		{
			barrier->srcAccessMask = srcAccessFlags;
			barrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier->oldLayout = srcMainLayout;
			barrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->image = srcVkTexture->deviceImage;
		barrier->subresourceRange.aspectMask = srcAspectMask;
		barrier->subresourceRange.baseMipLevel = srcPosition->mipLevel;
		barrier->subresourceRange.levelCount = 1;
		barrier->subresourceRange.baseArrayLayer = srcBaseLayer;
		barrier->subresourceRange.layerCount = srcLayers;

		if (!reverse)
		{
			VkBufferMemoryBarrier* bufferBarrier =
				dsVkCommandBuffer_addBufferBarrier(commandBuffer);
			if (!bufferBarrier)
				return false;

			bufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier->pNext = NULL;
			if (reverse)
			{
				bufferBarrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				bufferBarrier->dstAccessMask = dstAccessFlags;
			}
			else
			{
				bufferBarrier->srcAccessMask = dstAccessFlags;
				bufferBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			bufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier->buffer = dstVkBuffer;
			bufferBarrier->offset = region->bufferOffset;

			uint32_t bufferWidth = region->bufferWidth;
			if (bufferWidth == 0)
				bufferWidth = region->textureWidth;
			uint32_t bufferHeight = region->bufferHeight;
			if (bufferHeight == 0)
				bufferHeight = region->textureHeight;
			size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
			size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
			size_t textureXBlocks = (region->textureWidth + blockX - 1)/blockY;
			size_t remainderBlocks = bufferXBlocks - textureXBlocks;
			bufferBarrier->size =
				((bufferXBlocks*bufferYBlocks*region->layers) - remainderBlocks)*formatSize;
		}
	}

	return true;
}

dsTexture* dsVkTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size)
{
	return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, data, size,
		false, false);
}

dsOffscreen* dsVkTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, bool resolve)
{
	return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, NULL, 0, true,
		resolve);
}

bool dsVkTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkTexture* vkTexture = (dsVkTexture*)texture;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	const dsTextureInfo* texInfo = &texture->info;
	unsigned int formatSize = dsGfxFormat_size(texInfo->format);
	unsigned int blocksX, blocksY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, texInfo->format));

	uint32_t z, depth;
	uint32_t baseLayer, copyLayerCount;
	if (texInfo->dimension == dsTextureDim_3D)
	{
		z = position->depth;
		depth = layers;
		baseLayer = 0;
		copyLayerCount = 1;
	}
	else
	{
		z = 0;
		depth = 1;
		uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
		baseLayer = position->depth*faceCount + position->face;
		copyLayerCount = layers;
	}

	size_t offset = 0;
	VkBuffer tempBuffer = 0;
	void* tempData =
		dsVkCommandBuffer_getTempData(&offset, &tempBuffer, commandBuffer, size, formatSize);
	if (!tempData)
		return false;

	memcpy(tempData, data, size);

	dsVkRenderer_processTexture(renderer, texture);

	VkBufferMemoryBarrier bufferBarrier =
	{
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_HOST_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		tempBuffer,
		offset,
		size
	};

	bool isDepthStencil = dsGfxFormat_isDepthStencil(texture->info.format);
	VkImageLayout layout = dsVkTexture_imageLayout(texture);
	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(texInfo->format);
	VkImageMemoryBarrier imageBarrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		dsVkReadImageAccessFlags(texture->usage) | dsVkWriteImageAccessFlags(texture->usage,
			texture->offscreen, isDepthStencil),
		VK_ACCESS_TRANSFER_WRITE_BIT,
		layout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		vkTexture->deviceImage,
		{aspectMask, position->mipLevel, 1, baseLayer, copyLayerCount}
	};

	if (texture->info.dimension != dsTextureDim_3D)
	{
		uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
		imageBarrier.subresourceRange.baseArrayLayer = position->depth*faceCount + position->face;
		imageBarrier.subresourceRange.layerCount = layers;
	}

	VkPipelineStageFlags pipelineStages = dsVkReadImageStageFlags(renderer, texture->usage,
			texture->offscreen && isDepthStencil && !texture->resolve) |
		dsVkWriteImageStageFlags(renderer, texture->usage, texture->offscreen, isDepthStencil);
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer,
		pipelineStages | VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1,
		&bufferBarrier, 1, &imageBarrier);

	VkBufferImageCopy copyInfo =
	{
		offset,
		0,
		0,
		{aspectMask, position->mipLevel, baseLayer, copyLayerCount},
		{position->x, position->y, z},
		{width, height, depth}
	};

	DS_VK_CALL(device->vkCmdCopyBufferToImage)(vkCommandBuffer, tempBuffer, vkTexture->deviceImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

	imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.dstAccessMask = imageBarrier.srcAccessMask;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.newLayout = layout;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		pipelineStages, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
	return true;
}

bool dsVkTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkTexture* srcVkTexture = (dsVkTexture*)srcTexture;
	dsVkTexture* dstVkTexture = (dsVkTexture*)dstTexture;
	if (!dsVkCommandBuffer_addResource(commandBuffer, &srcVkTexture->resource) ||
		!dsVkCommandBuffer_addResource(commandBuffer, &dstVkTexture->resource))
	{
		return false;
	}

	dsVkRenderer_processTexture(renderer, srcTexture);
	dsVkRenderer_processTexture(renderer, dstTexture);

	VkImageAspectFlags srcAspectMask = dsVkImageAspectFlags(srcTexture->info.format);
	uint32_t srcFaceCount = srcTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;
	bool srcIsDepthStencil = dsGfxFormat_isDepthStencil(srcTexture->info.format);

	VkImageAspectFlags dstAspectMask = dsVkImageAspectFlags(dstTexture->info.format);
	uint32_t dstFaceCount = dstTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;
	bool dstIsDepthStencil = dsGfxFormat_isDepthStencil(dstTexture->info.format);

	VkImageLayout srcLayout = srcTexture == dstTexture ? VK_IMAGE_LAYOUT_GENERAL :
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkImageLayout dstLayout = srcTexture == dstTexture ? VK_IMAGE_LAYOUT_GENERAL :
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	if (!addCopyImageBarriers(commandBuffer, regions, regionCount, srcTexture, dstTexture, false))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}

	// 1024 regions is ~72 KB of stack space. After that use heap space.
	bool heapRegions = regionCount > 1024;
	dsAllocator* scratchAllocator = resourceManager->allocator;
	VkImageCopy* imageCopies;
	if (heapRegions)
	{
		imageCopies = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, VkImageCopy, regionCount);
		if (!imageCopies)
			return false;
	}
	else
		imageCopies = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkImageCopy, regionCount);

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsTextureCopyRegion* region = regions + i;
		uint32_t srcLayer, srcDepth;
		if (srcIs3D)
		{
			srcLayer = 0;
			srcDepth = region->srcPosition.depth;
		}
		else
		{
			srcLayer = region->srcPosition.depth*srcFaceCount + region->srcPosition.face;
			srcDepth = 0;
		}

		uint32_t dstLayer, dstDepth;
		if (dstIs3D)
		{
			dstLayer = 0;
			dstDepth = region->dstPosition.depth;
		}
		else
		{
			dstLayer = region->dstPosition.depth*dstFaceCount + region->dstPosition.face;
			dstDepth = 0;
		}

		uint32_t layerCount, depthCount;
		if (srcIs3D != dstIs3D && region->layers != 1)
		{
			if (heapRegions)
				DS_VERIFY(dsAllocator_free(scratchAllocator, imageCopies));
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
				"Cannot copy between a 3D texture and non-3D texture with multiple layers.");
			return false;
		}

		if (srcIs3D)
		{
			layerCount = 1;
			depthCount = region->layers;
		}
		else
		{
			layerCount = region->layers;
			depthCount = 1;
		}

		VkImageCopy* imageCopy = imageCopies + i;
		imageCopy->srcSubresource.aspectMask = srcAspectMask;
		imageCopy->srcSubresource.mipLevel = region->srcPosition.mipLevel;
		imageCopy->srcSubresource.baseArrayLayer = srcLayer;
		imageCopy->srcSubresource.layerCount = layerCount;
		imageCopy->srcOffset.x = region->srcPosition.x;
		imageCopy->srcOffset.y = region->srcPosition.y;
		imageCopy->srcOffset.z = srcDepth;
		imageCopy->dstSubresource.aspectMask = dstAspectMask;
		imageCopy->dstSubresource.mipLevel = region->dstPosition.mipLevel;
		imageCopy->dstSubresource.baseArrayLayer = dstLayer;
		imageCopy->dstSubresource.layerCount = layerCount;
		imageCopy->dstOffset.x = region->dstPosition.x;
		imageCopy->dstOffset.y = region->dstPosition.y;
		imageCopy->dstOffset.z = dstDepth;
		imageCopy->extent.width = region->width;
		imageCopy->extent.height = region->height;
		imageCopy->extent.depth = depthCount;
	}

	VkPipelineStageFlags srcStageFlags = dsVkReadImageStageFlags(renderer, srcTexture->usage,
			srcTexture->offscreen && srcIsDepthStencil && !srcTexture->resolve) |
		dsVkWriteImageStageFlags(renderer, srcTexture->usage, srcTexture->offscreen,
			srcIsDepthStencil);
	VkPipelineStageFlags dstStageFlags = dsVkReadImageStageFlags(renderer, dstTexture->usage,
			dstTexture->offscreen && dstIsDepthStencil && !dstTexture->resolve) |
		dsVkWriteImageStageFlags(renderer, dstTexture->usage, dstTexture->offscreen,
			dstIsDepthStencil);
	VkPipelineStageFlags stageFlags = srcStageFlags | dstStageFlags;
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, stageFlags,
		VK_PIPELINE_STAGE_TRANSFER_BIT);
	DS_VK_CALL(device->vkCmdCopyImage)(vkCommandBuffer, srcVkTexture->deviceImage,
		srcLayout, dstVkTexture->deviceImage, dstLayout, regionCount, imageCopies);

	if (heapRegions)
		DS_VERIFY(dsAllocator_free(scratchAllocator, imageCopies));

	if (!addCopyImageBarriers(commandBuffer, regions, regionCount, srcTexture, dstTexture, true))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, stageFlags);

	return true;
}

bool dsVkTexture_copyToBuffer(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsGfxBuffer* dstBuffer, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkTexture* srcVkTexture = (dsVkTexture*)srcTexture;
	dsVkGfxBufferData* dstBufferData = dsVkGfxBuffer_getData(dstBuffer, commandBuffer);
	if (!dstBufferData || !dsVkCommandBuffer_addResource(commandBuffer, &srcVkTexture->resource))
		return false;

	dsVkRenderer_processTexture(renderer, srcTexture);
	dsVkRenderer_processGfxBuffer(renderer, dstBufferData);

	VkImageAspectFlags srcAspectMask = dsVkImageAspectFlags(srcTexture->info.format);
	uint32_t srcFaceCount = srcTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;
	bool srcIsDepthStencil = dsGfxFormat_isDepthStencil(srcTexture->info.format);

	bool dstCanMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(dstBufferData);

	if (!addCopyToBufferBarriers(commandBuffer, regions, regionCount, srcTexture, dstBufferData,
			dstCanMapMainBuffer, false))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}

	// 1024 regions is ~56 KB of stack space. After that use heap space.
	bool heapRegions = regionCount > 1024;
	dsAllocator* scratchAllocator = resourceManager->allocator;
	VkBufferImageCopy* imageCopies;
	if (heapRegions)
	{
		imageCopies = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, VkBufferImageCopy, regionCount);
		if (!imageCopies)
			return false;
	}
	else
		imageCopies = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkBufferImageCopy, regionCount);

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;
		uint32_t srcLayer, srcDepth, layerCount, depthCount;
		if (srcIs3D)
		{
			srcLayer = 0;
			srcDepth = region->texturePosition.depth;
			layerCount = 1;
			depthCount = region->layers;
		}
		else
		{
			srcLayer = region->texturePosition.depth*srcFaceCount + region->texturePosition.face;
			srcDepth = 0;
			layerCount = region->layers;
			depthCount = 1;
		}

		VkBufferImageCopy* imageCopy = imageCopies + i;
		imageCopy->bufferOffset = region->bufferOffset;
		imageCopy->bufferRowLength = region->bufferWidth;
		imageCopy->bufferImageHeight = region->bufferHeight;
		imageCopy->imageSubresource.aspectMask = srcAspectMask;
		imageCopy->imageSubresource.mipLevel = region->texturePosition.mipLevel;
		imageCopy->imageSubresource.baseArrayLayer = srcLayer;
		imageCopy->imageSubresource.layerCount = layerCount;
		imageCopy->imageOffset.x = region->texturePosition.x;
		imageCopy->imageOffset.y = region->texturePosition.y;
		imageCopy->imageOffset.z = srcDepth;
		imageCopy->imageExtent.width = region->textureWidth;
		imageCopy->imageExtent.height = region->textureHeight;
		imageCopy->imageExtent.depth = depthCount;
	}

	VkPipelineStageFlags srcStageFlags = dsVkReadImageStageFlags(renderer, srcTexture->usage,
			srcTexture->offscreen && srcIsDepthStencil && !srcTexture->resolve) |
		dsVkWriteImageStageFlags(renderer, srcTexture->usage, srcTexture->offscreen,
			srcIsDepthStencil);
	VkPipelineStageFlags dstStageFlags = dsVkReadBufferStageFlags(renderer, dstBuffer->usage) |
		dsVkWriteBufferStageFlags(renderer, dstBuffer->usage, dstCanMapMainBuffer);
	VkPipelineStageFlags stageFlags = srcStageFlags | dstStageFlags;
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, stageFlags,
		VK_PIPELINE_STAGE_TRANSFER_BIT);
	DS_VK_CALL(device->vkCmdCopyImageToBuffer)(vkCommandBuffer, srcVkTexture->deviceImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dsVkGfxBufferData_getBuffer(dstBufferData),
		regionCount, imageCopies);

	if (heapRegions)
		DS_VERIFY(dsAllocator_free(scratchAllocator, imageCopies));

	if (!addCopyToBufferBarriers(commandBuffer, regions, regionCount, srcTexture, dstBufferData,
			dstCanMapMainBuffer, true))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stageFlags);

	return true;
}

bool dsVkTexture_generateMipmaps(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	const dsTextureInfo* info = &texture->info;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkTexture->resource))
		return false;

	dsVkRenderer_processTexture(renderer, texture);

	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = info->dimension == dsTextureDim_3D;
	uint32_t totalLayers = is3D ? 1 : info->depth*faceCount;
	totalLayers = dsMax(totalLayers, 1U);

	bool isDepthStencil = dsGfxFormat_isDepthStencil(info->format);
	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
	VkAccessFlags accessFlags = dsVkReadImageAccessFlags(texture->usage) |
		dsVkWriteImageAccessFlags(texture->usage, texture->offscreen, isDepthStencil);
	VkPipelineStageFlags stages = dsVkReadImageStageFlags(renderer, texture->usage,
			texture->offscreen && isDepthStencil) |
		dsVkWriteImageStageFlags(renderer, texture->usage, texture->offscreen, isDepthStencil);
	VkImageLayout layout = dsVkTexture_imageLayout(texture);

	uint32_t width = info->width;
	uint32_t height = info->height;
	uint32_t depth = is3D ? info->depth : 1;
	for (uint32_t i = 0; i < texture->info.mipLevels - 1; ++i)
	{
		VkImageMemoryBarrier barriers[2] =
		{
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				i == 0 ? accessFlags : VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,
				i == 0 ? layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				vkTexture->deviceImage,
				{aspectMask, i, 1, 0, totalLayers}
			},
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				accessFlags,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				vkTexture->deviceImage,
				{aspectMask, i + 1, 1, 0, totalLayers}
			}
		};

		DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, stages,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2, barriers);

		uint32_t dstWidth = width/2;
		uint32_t dstHeight = height/2;
		uint32_t dstDepth = depth/2;
		dstWidth = dsMax(dstWidth, 1U);
		dstHeight = dsMax(dstHeight, 1U);
		dstDepth = dsMax(dstDepth, 1U);

		VkImageBlit blit =
		{
			{aspectMask, i, 0, totalLayers},
			{{0, 0, 0}, {width, height, depth}},
			{aspectMask, i + 1, 0, totalLayers},
			{{0, 0, 0}, {dstWidth, dstHeight, dstDepth}}
		};
		DS_VK_CALL(device->vkCmdBlitImage)(vkCommandBuffer, vkTexture->deviceImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkTexture->deviceImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		width = dstWidth;
		height = dstHeight;
		depth = dstDepth;
	}

	VkImageMemoryBarrier finishBarriers[2] =
	{
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			VK_ACCESS_TRANSFER_READ_BIT,
			accessFlags,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			layout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			vkTexture->deviceImage,
			{aspectMask, 0, texture->info.mipLevels - 1, 0, totalLayers}
		},
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			accessFlags,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			layout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			vkTexture->deviceImage,
			{aspectMask, texture->info.mipLevels - 1, 1, 0, totalLayers}
		}
	};
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stages, 0, 0, NULL, 0, NULL, 2, finishBarriers);

	return true;
}

bool dsVkTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	const dsTextureInfo* info = &texture->info;

	if (vkTexture->lastDrawSubmit == DS_NOT_SUBMITTED)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
			"Trying to read to an offscreen that hasn't had a draw flushed yet.");
		return false;
	}

	dsVkResource_waitUntilNotInUse(&vkTexture->resource, resourceManager->renderer);

	dsTextureInfo surfaceInfo;
	surfaceInfo.format = info->format;
	surfaceInfo.dimension = info->dimension;
	surfaceInfo.width = dsMax(info->width >> position->mipLevel, 1U);
	surfaceInfo.height = dsMax(info->height >> position->mipLevel, 1U);
	surfaceInfo.depth = 1;
	surfaceInfo.mipLevels = 1;
	surfaceInfo.samples = 1;

	void* imageMemory;
	VkDeviceSize offset = dsTexture_surfaceOffset(info, position->face, position->depth,
		position->mipLevel);
	VkDeviceSize mapSize = dsTexture_size(&surfaceInfo);
	size_t rem = 0;
	adjustAlignment(resourceManager->minNonCoherentMappingAlignment, vkTexture->hostMemorySize,
		&offset, &mapSize, &rem);
	if (offset + mapSize >= vkTexture->hostMemorySize)
		mapSize = VK_WHOLE_SIZE;
	VkResult vkResult = DS_VK_CALL(device->vkMapMemory)(device->device, vkTexture->hostMemory,
		offset, mapSize, 0, &imageMemory);
	if (!DS_HANDLE_VK_RESULT(vkResult, "Couldn't map image memory"))
		return false;

	imageMemory = (uint8_t*)imageMemory + rem;

	if (!vkTexture->hostMemoryCoherent)
	{
		VkMappedMemoryRange range =
		{
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			NULL,
			vkTexture->hostMemory,
			offset,
			mapSize
		};
		vkResult = DS_VK_CALL(device->vkInvalidateMappedMemoryRanges)(device->device, 1, &range);
		if (!DS_HANDLE_VK_RESULT(vkResult, "Couldn't invalidate image memory"))
			return false;
	}

	unsigned int blockX, blockY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
		return false;
	unsigned int formatSize = dsGfxFormat_size(info->format);

	uint32_t xBlocks = (width + blockX - 1)/blockX;
	uint32_t yBlocks = (height + blockY - 1)/blockY;
	uint32_t pitch = xBlocks*formatSize;
	DS_ASSERT(size == pitch*yBlocks);
	DS_UNUSED(size);

	uint32_t imagePitch = surfaceInfo.width/blockX*formatSize;

	uint32_t startXBlock = position->x/blockX;
	uint32_t startYBlock = position->y/blockY;

	uint8_t* resultBytes = (uint8_t*)result;
	const uint8_t* imageBytes = (const uint8_t*)imageMemory + startYBlock*imagePitch +
		startXBlock*formatSize;
	for (uint32_t y = 0; y < yBlocks; ++y, resultBytes += pitch, imageBytes += imagePitch)
		memcpy(resultBytes, imageBytes, pitch);

	DS_VK_CALL(device->vkUnmapMemory)(device->device, vkTexture->hostMemory);
	return true;
}

void dsVkTexture_process(dsResourceManager* resourceManager, dsTexture* texture)
{
	dsVkRenderer_processTexture(resourceManager->renderer, texture);
}

bool dsVkTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture)
{
	dsVkRenderer_deleteTexture(resourceManager->renderer, texture, false);
	return true;
}

bool dsVkTexture_supportsHostImage(dsVkDevice* device, const dsVkFormatInfo* formatInfo,
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

bool dsVkTexture_isStatic(const dsTexture* texture)
{
	return (texture->usage & (dsTextureUsage_CopyTo | dsTextureUsage_Image)) == 0 &&
		!texture->offscreen;
}

bool dsVkTexture_onlySubpassInput(dsTextureUsage usage)
{
	return (usage & dsTextureUsage_SubpassInput) &&
		!(usage & (dsTextureUsage_Texture | dsTextureUsage_Image));
}

VkImageLayout dsVkTexture_imageLayout(const dsTexture* texture)
{
	if (texture->usage & dsTextureUsage_Image)
		return VK_IMAGE_LAYOUT_GENERAL;

	if (dsVkTexture_onlySubpassInput(texture->usage))
	{
		if (dsGfxFormat_isDepthStencil(texture->info.format))
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (texture->usage & dsTextureUsage_Texture)
	{
		if (dsGfxFormat_isDepthStencil(texture->info.format))
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if (texture->usage == dsTextureUsage_CopyFrom)
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	else if (texture->usage == dsTextureUsage_CopyTo)
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	return VK_IMAGE_LAYOUT_GENERAL;
}

VkImageLayout dsVkTexture_bindImageLayout(const dsTexture* texture)
{
	if (texture->usage & dsTextureUsage_Image)
		return VK_IMAGE_LAYOUT_GENERAL;

	if (dsGfxFormat_isDepthStencil(texture->info.format))
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	else
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

bool dsVkTexture_canReadBack(const dsTexture* texture)
{
	return texture->offscreen && (texture->usage & dsTextureUsage_CopyFrom) &&
		(texture->memoryHints & dsGfxMemory_Read);
}

bool dsVkTexture_processAndAddResource(dsTexture* texture, dsCommandBuffer* commandBuffer)
{
	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	dsVkRenderer_processTexture(commandBuffer->renderer, texture);
	return dsVkCommandBuffer_addResource(commandBuffer, &vkTexture->resource);
}

void dsVkTexture_destroyImpl(dsTexture* texture)
{
	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	dsVkDevice* device = &((dsVkRenderer*)texture->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	dsLifetime_destroy(vkTexture->lifetime);

	if (vkTexture->deviceImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, vkTexture->deviceImageView,
			instance->allocCallbacksPtr);
	}
	if (vkTexture->depthOnlyImageView)
	{
		DS_VK_CALL(device->vkDestroyImageView)(device->device, vkTexture->depthOnlyImageView,
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

	if (vkTexture->hostBuffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, vkTexture->hostBuffer,
			instance->allocCallbacksPtr);
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

	dsVkResource_shutdown(&vkTexture->resource);
	if (texture->allocator)
		dsAllocator_free(texture->allocator, texture);
}
