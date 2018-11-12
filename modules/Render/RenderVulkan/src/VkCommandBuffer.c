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

#include "VkCommandBuffer.h"
#include "VkBarrierList.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool processOffscreenReadbacks(dsCommandBuffer* commandBuffer,
	VkCommandBuffer renderCommands)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	// Need image barriers for the offscreen textures to make sure all writes are finished.
	uint32_t imageBarrierCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->imageBarriers,
		imageBarrierCount, vkCommandBuffer->maxImageBarriers,
		vkCommandBuffer->readbackOffscreenCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsOffscreen* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
		VkImageMemoryBarrier* imageBarrier = vkCommandBuffer->imageBarriers + i;

		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		if (dsGfxFormat_isDepthStencil(offscreen->info.format))
			imageBarrier->srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		else
			imageBarrier->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = vkOffscreen->deviceImage;
		imageBarrier->subresourceRange.aspectMask = dsVkImageAspectFlags(info->format);
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	}

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL,
		imageBarrierCount, vkCommandBuffer->imageBarriers);

	// Copy offscreen texture data to host images that can be read back from.
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsTexture* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
		uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
		uint32_t layerCount;
		bool is3D = info->dimension == dsTextureDim_3D;
		if (is3D)
			layerCount = 1;
		else
			layerCount = dsMax(1U, info->depth)*faceCount;

		if (vkOffscreen->hostImage)
		{
			// Single image for all surfaces within each mip level.
			uint32_t imageCopiesCount = 0;
			if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->imageCopies,
				imageCopiesCount, vkCommandBuffer->maxImageCopies, info->mipLevels))
			{
				return false;
			}

			uint32_t copyIndex = imageCopiesCount;
			for (uint32_t j = 0; j < info->mipLevels; ++j)
			{
				uint32_t width = info->width >> j;
				uint32_t height = info->height >> j;
				uint32_t depth = is3D ? info->depth >> j : info->depth;
				width = dsMax(1U, width);
				height = dsMax(1U, height);
				depth = dsMax(1U, depth);

				VkImageCopy* imageCopy = vkCommandBuffer->imageCopies + copyIndex + j;
				imageCopy->srcSubresource.aspectMask = aspectMask;
				imageCopy->srcSubresource.mipLevel = j;
				imageCopy->srcSubresource.baseArrayLayer = 0;
				imageCopy->srcSubresource.layerCount = layerCount;
				imageCopy->srcOffset.x = 0;
				imageCopy->srcOffset.y = 0;
				imageCopy->srcOffset.z = 0;
				imageCopy->dstSubresource = imageCopy->srcSubresource;
				imageCopy->dstOffset = imageCopy->srcOffset;
				imageCopy->extent.width = width;
				imageCopy->extent.height = height;
				imageCopy->extent.depth = depth;
			}

			DS_VK_CALL(device->vkCmdCopyImage)(renderCommands, vkOffscreen->deviceImage,
				VK_IMAGE_LAYOUT_GENERAL, vkOffscreen->hostImage, VK_IMAGE_LAYOUT_GENERAL,
				imageCopiesCount, vkCommandBuffer->imageCopies);
		}
		else
		{
			// Separate copies for each image level.
			for (uint32_t j = 0, index = 0; j < vkOffscreen->hostImageCount; ++j)
			{
				uint32_t width = info->width >> j;
				uint32_t height = info->height >> j;
				uint32_t depth = is3D ? info->depth >> j : info->depth;
				width = dsMax(1U, width);
				height = dsMax(1U, height);
				depth = dsMax(1U, depth);

				for (uint32_t k = 0; k < depth; ++k)
				{
					for (uint32_t l = 0; l < faceCount; ++l, ++index)
					{
						DS_ASSERT(index < vkOffscreen->hostImageCount);
						const dsVkHostImage* hostImage = vkOffscreen->hostImages + index;

						VkImageCopy imageCopy =
						{
							{aspectMask, j, is3D ? 0 : k*faceCount + l, 1},
							{0, 0, is3D ? k : 0},
							{aspectMask, 0, 0, 1},
							{0, 0, 0},
							{width, height, 1}
						};

						DS_VK_CALL(device->vkCmdCopyImage)(renderCommands, vkOffscreen->deviceImage,
							VK_IMAGE_LAYOUT_GENERAL, hostImage->image, VK_IMAGE_LAYOUT_GENERAL,
							1, &imageCopy);
					}
				}
			}
		}
	}

	// NOTE: Don't need a separate barrier for the host images since a general memory barrier is
	// used for host readback.

	return true;
}

void dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;

	memset(commandBuffer, 0, sizeof(*commandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;
	dsVkBarrierList_initialize(&commandBuffer->barriers, allocator, &vkRenderer->device);
}

bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkCommandBuffer* vkSubmitBuffer = (dsVkCommandBuffer*)submitBuffer;

	// Copy over the used resources.
	uint32_t offset = vkCommandBuffer->usedResourceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->usedResources,
		vkCommandBuffer->usedResourceCount, vkCommandBuffer->maxUsedResources,
		vkSubmitBuffer->usedResourceCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < vkSubmitBuffer->usedResourceCount; ++i)
	{
		dsVkResource* resource = vkSubmitBuffer->usedResources[i];
		DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, 1);
		vkCommandBuffer->usedResources[offset + i] = resource;
	}

	// Copy over the readback offscreens
	offset = vkCommandBuffer->readbackOffscreenCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->readbackOffscreens,
		vkCommandBuffer->readbackOffscreenCount, vkCommandBuffer->maxReadbackOffscreens,
		vkSubmitBuffer->readbackOffscreenCount))
	{
		return false;
	}

	memcpy(vkCommandBuffer->readbackOffscreens + offset, vkSubmitBuffer->readbackOffscreens,
		sizeof(dsTexture*)*vkSubmitBuffer->readbackOffscreenCount);

	DS_VK_CALL(device->vkCmdExecuteCommands)(vkCommandBuffer->vkCommandBuffer, 1,
		&vkSubmitBuffer->vkCommandBuffer);

	// Reset immediately if not submitted multiple times. This frees any internal references to
	// resources.
	if (!(submitBuffer->usage &
		(dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
	{
		dsVkCommandBuffer_clearUsedResources(submitBuffer);
	}

	return true;
}

bool dsVkCommandBuffer_endSubmitCommands(dsCommandBuffer* commandBuffer,
	VkCommandBuffer renderCommands)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	// Copy the readback offscreens.
	if (!processOffscreenReadbacks(commandBuffer, renderCommands))
		return false;

	// Make sure any writes are visible for mapping buffers.
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
		VK_PIPELINE_STAGE_TRANSFER_BIT;
	if (renderer->hasTessellationShaders)
	{
		srcStage |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		srcStage |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_HOST_BIT;

	VkMemoryBarrier memoryBarrier =
	{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, srcStage, dstStage, 0, 1,
		&memoryBarrier, 0, NULL, 0, NULL);

	return true;
}

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->usedResourceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->usedResources,
		vkCommandBuffer->usedResourceCount, vkCommandBuffer->maxUsedResources, 1))
	{
		return false;
	}

	vkCommandBuffer->usedResources[index] = resource;
	DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, 1);
	return true;
}

bool dsVkCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsTexture* offscreen)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->readbackOffscreenCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->readbackOffscreens,
		vkCommandBuffer->readbackOffscreenCount, vkCommandBuffer->maxReadbackOffscreens, 1))
	{
		return false;
	}

	vkCommandBuffer->readbackOffscreens[index] = offscreen;
	return true;
}

void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
		DS_ATOMIC_FETCH_ADD32(&vkCommandBuffer->usedResources[i]->commandBufferCount, -1);

	vkCommandBuffer->usedResourceCount = 0;
	vkCommandBuffer->readbackOffscreenCount = 0;
}

void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
	{
		dsVkResource* resource = vkCommandBuffer->usedResources[i];
		DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
		resource->lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
	}

	vkCommandBuffer->usedResourceCount = 0;

	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsVkTexture* texture = (dsVkTexture*)vkCommandBuffer->readbackOffscreens[i];
		DS_VERIFY(dsSpinlock_lock(&texture->resource.lock));
		texture->lastDrawSubmit = submitCount;
		DS_VERIFY(dsSpinlock_lock(&texture->resource.lock));
	}
	vkCommandBuffer->readbackOffscreenCount = 0;
}

void dsVkCommandBuffer_shutdown(dsVkCommandBuffer* commandBuffer)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->usedResources));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->readbackOffscreens));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageCopies));
	dsVkBarrierList_shutdown(&commandBuffer->barriers);
}
