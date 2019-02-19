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

#include "Resources/VkCopyImage.h"

#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "Resources/VkTexture.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

static size_t fullAllocSize(uint32_t imageCount, uint32_t layers)
{
	return DS_ALIGNED_SIZE(sizeof(dsVkCopyImage)) + DS_ALIGNED_SIZE(sizeof(VkImage)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(VkImageMemoryBarrier)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(VkImageCopy)*layers);
}

dsVkCopyImage* dsVkCopyImage_create(dsAllocator* allocator, dsVkDevice* device, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	dsVkInstance* instance = &device->instance;
	const dsTextureInfo* info = &texture->info;
	bool is3D = info->dimension == dsTextureDim_3D;
	uint32_t vkLayers = is3D ? 1U : layers;

	dsTextureDim dimension;
	VkImageType imageType;
	if (info->dimension == dsTextureDim_3D && layers > 1)
	{
		dimension = dsTextureDim_3D;
		imageType = VK_IMAGE_TYPE_3D;
	}
	else
	{
		dimension = dsTextureDim_2D;
		imageType = VK_IMAGE_TYPE_2D;
	}
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(texture->resourceManager,
		info->format);
	uint32_t imageCount;
	// NOTE: Intel seems to break on this, allocating incorrect sizes. NVidia only supports a single
	// image as well, so just disable for now. Perhaps this can be used sometime in the future.
	/*dsTextureInfo tempInfo = {info->format, dimension, width, height, layers, 1, 1};
	if (dsVkTexture_supportsHostImage(device, formatInfo, imageType, &tempInfo))
		imageCount = 1;
	else*/
	{
		imageCount = layers;
		imageType = VK_IMAGE_TYPE_2D;
	}

	size_t fullSize = fullAllocSize(imageCount, vkLayers);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return false;

	// Create the main object.
	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkCopyImage* copyImage = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkCopyImage);
	DS_ASSERT(copyImage);
	dsVkResource_initialize(&copyImage->resource);
	copyImage->allocator = dsAllocator_keepPointer(allocator);
	copyImage->device = device;
	copyImage->memory = 0;
	copyImage->images = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkImage, imageCount);
	DS_ASSERT(copyImage->images);
	memset(copyImage->images, 0, sizeof(VkImage)*imageCount);
	copyImage->imageBarriers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		VkImageMemoryBarrier, imageCount);
	copyImage->imageCount = imageCount;
	copyImage->imageCopies = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkImageCopy,
		vkLayers);
	DS_ASSERT(copyImage->imageCopies);
	copyImage->imageCopyCount = vkLayers;
	copyImage->memory = 0;

	// Create the Vulkan images.
	VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		imageType,
		formatInfo->vkFormat,
		{width, height, 1},
		1,
		imageCount == vkLayers ? 1 : vkLayers,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		1, &device->queueFamilyIndex,
		VK_IMAGE_LAYOUT_PREINITIALIZED
	};

	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
	VkMemoryRequirements memoryRequirements = {0, 0, 0};
	size_t imageSize = 0;
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImage* image = copyImage->images + i;
		VkResult result = DS_VK_CALL(device->vkCreateImage)(device->device,
			&imageCreateInfo, instance->allocCallbacksPtr, image);
		if (!dsHandleVkResult(result))
			return false;

		VkImageMemoryBarrier* imageBarrier = copyImage->imageBarriers + i;
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageBarrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = *image;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		if (i != 0)
			continue;

		VkMemoryRequirements imageRequirements;
		DS_VK_CALL(device->vkGetImageMemoryRequirements)(device->device,
			*image, &imageRequirements);
		VkDeviceSize alignment = imageRequirements.alignment;
		memoryRequirements.size =
			((imageRequirements.size + (alignment - 1))/alignment)*alignment;
		imageSize = (size_t)memoryRequirements.size;

		memoryRequirements.alignment = alignment;
		memoryRequirements.size = imageSize*imageCount;
		memoryRequirements.memoryTypeBits = imageRequirements.memoryTypeBits;
	}

	// Allocate the memory.
	uint32_t memoryIndex = dsVkMemoryIndex(device, &memoryRequirements, 0);
	if (memoryIndex == DS_INVALID_HEAP)
	{
		dsVkCopyImage_destroy(copyImage);
		return NULL;
	}

	copyImage->memory = dsAllocateVkMemory(device, &memoryRequirements, memoryIndex);
	if (!copyImage->memory)
	{
		dsVkCopyImage_destroy(copyImage);
		return NULL;
	}

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImage image = copyImage->images[i];
		VkResult result = DS_VK_CALL(device->vkBindImageMemory)(device->device, image,
			copyImage->memory, imageSize*i);
		if (!dsHandleVkResult(result))
		{
			dsVkCopyImage_destroy(copyImage);
			return NULL;
		}
	}

	// Populate the data.
	VkImageSubresource subresource = {aspectMask, 0, 0};
	VkSubresourceLayout baseLayout;
	DS_VK_CALL(device->vkGetImageSubresourceLayout)(device->device, copyImage->images[0],
		&subresource, &baseLayout);

	dsTextureInfo layerInfo = {info->format, dimension, width, height, 1, 1, 1};
	size_t layerSize = dsTexture_size(&layerInfo);
	DS_ASSERT(layerSize*layers == size);
	DS_UNUSED(size);

	size_t imageLayerSize;
	if (imageCount > 1)
		imageLayerSize = imageSize;
	else if (is3D)
		imageLayerSize = (size_t)baseLayout.depthPitch;
	else
		imageLayerSize = (size_t)baseLayout.arrayPitch;

	unsigned int blockX, blockY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
	{
		dsVkCopyImage_destroy(copyImage);
		return NULL;
	}
	unsigned int formatSize = dsGfxFormat_size(info->format);

	uint32_t xBlocks = (width + blockX - 1)/blockX;
	uint32_t yBlocks = (height + blockY - 1)/blockY;
	uint32_t pitch = xBlocks*formatSize;

	void* imageData;
	VkResult result = DS_VK_CALL(device->vkMapMemory)(device->device, copyImage->memory, 0,
		VK_WHOLE_SIZE, 0, &imageData);
	if (!dsHandleVkResult(result))
	{
		dsVkCopyImage_destroy(copyImage);
		return NULL;
	}

	for (uint32_t i = 0; i < layers; ++i)
	{
		const uint8_t* srcBytes = (const uint8_t*)data + i*layerSize;
		uint8_t* dstBytes = (uint8_t*)imageData + i*imageLayerSize;
		for (uint32_t y = 0; y < yBlocks; ++y, srcBytes += pitch, dstBytes += baseLayout.rowPitch)
			memcpy(dstBytes, srcBytes, pitch);
	}

	DS_VK_CALL(device->vkUnmapMemory)(device->device, copyImage->memory);

	// Create the copy regions.
	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	uint32_t copyLayerCount = vkLayers == 1 ? layers : 1;
	for (uint32_t i = 0; i < vkLayers; ++i)
	{
		VkImageCopy* copy = copyImage->imageCopies + i;
		copy->srcSubresource.aspectMask = aspectMask;
		copy->srcSubresource.mipLevel = 0;
		copy->srcSubresource.baseArrayLayer = 0;
		copy->srcSubresource.layerCount = copyLayerCount;
		copy->srcOffset.x = 0;
		copy->srcOffset.y = 0;
		copy->srcOffset.z = 0;
		copy->dstSubresource.aspectMask = aspectMask;
		copy->dstSubresource.mipLevel = position->mipLevel;
		copy->dstSubresource.baseArrayLayer = is3D ? 0 :
			faceCount*position->depth + position->face + i;
		copy->dstSubresource.layerCount = copyLayerCount;
		copy->dstOffset.x = position->x;
		copy->dstOffset.y = position->y;
		copy->dstOffset.z = is3D ? position->depth + i: 0;
		copy->extent.width = width;
		copy->extent.height = height;
		copy->extent.depth = 1;
	}

	return copyImage;
}

void dsVkCopyImage_destroy(dsVkCopyImage* copyImage)
{
	dsVkDevice* device = copyImage->device;
	dsVkInstance* instance = &device->instance;
	for (uint32_t i = 0; i < copyImage->imageCount; ++i)
	{
		if (copyImage->images[i])
		{
			DS_VK_CALL(device->vkDestroyImage)(device->device, copyImage->images[i],
				instance->allocCallbacksPtr);
		}
	}

	if (copyImage->memory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, copyImage->memory,
			instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&copyImage->resource);
	if (copyImage->allocator)
		DS_VERIFY(dsAllocator_free(copyImage->allocator, copyImage));
}
