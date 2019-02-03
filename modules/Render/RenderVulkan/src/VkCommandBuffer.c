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

#include "VkCommandBuffer.h"

#include "Resources/VkFramebuffer.h"
#include "Resources/VkTexture.h"
#include "VkBarrierList.h"
#include "VkCommandBufferData.h"
#include "VkRendererInternal.h"
#include "VkShared.h"
#include "VkVolatileDescriptorSets.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool addOffscreenHostBeginBarrier(dsCommandBuffer* commandBuffer, VkImage image,
	VkImageAspectFlags aspectMask)
{
	VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
	if (imageBarrier)
		return false;

	imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier->pNext = NULL;
	imageBarrier->srcAccessMask = VK_ACCESS_HOST_READ_BIT;
	imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageBarrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier->image = image;
	imageBarrier->subresourceRange.aspectMask = aspectMask;
	imageBarrier->subresourceRange.baseMipLevel = 0;
	imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier->subresourceRange.baseArrayLayer = 0;
	imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	return true;
}

static bool addOffscreenHostEndBarrier(dsCommandBuffer* commandBuffer, VkImage image,
	VkImageAspectFlags aspectMask)
{
	VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
	if (imageBarrier)
		return false;

	imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier->pNext = NULL;
	imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier->dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	imageBarrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier->image = image;
	imageBarrier->subresourceRange.aspectMask = aspectMask;
	imageBarrier->subresourceRange.baseMipLevel = 0;
	imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier->subresourceRange.baseArrayLayer = 0;
	imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	return true;
}

static bool processOffscreenReadbacks(dsCommandBuffer* commandBuffer,
	VkCommandBuffer renderCommands)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	dsVkCommandBuffer_resetCopyImageBarriers(commandBuffer);
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsOffscreen* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (imageBarrier)
			return false;

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
		bool isDepthStencil = dsGfxFormat_isDepthStencil(info->format);
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = dsVkReadImageAccessFlags(offscreen->usage) |
			dsVkWriteImageAccessFlags(offscreen->usage, true, isDepthStencil);
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->oldLayout = dsVkTexture_imageLayout(offscreen);
		imageBarrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = vkOffscreen->deviceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		if (vkOffscreen->hostImage)
		{
			if (!addOffscreenHostBeginBarrier(commandBuffer, vkOffscreen->hostImage, aspectMask))
				return false;
		}
		else
		{
			for (uint32_t i = 0; i < vkOffscreen->hostImageCount; ++i)
			{
				if (!addOffscreenHostBeginBarrier(commandBuffer, vkOffscreen->hostImages[i].image,
					aspectMask))
				{
					return false;
				}
			}
		}
	}

	VkPipelineStageFlags stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	if (renderer->hasTessellationShaders)
	{
		stages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		stages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, stages | VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL,
		vkCommandBuffer->copyImageBarrierCount, vkCommandBuffer->copyImageBarriers);
	vkCommandBuffer->copyImageBarrierCount = 0;

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
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkOffscreen->hostImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageCopiesCount,
				vkCommandBuffer->imageCopies);
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
							VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, hostImage->image,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
					}
				}
			}
		}
	}

	dsVkCommandBuffer_resetCopyImageBarriers(commandBuffer);
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsOffscreen* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (imageBarrier)
			return false;

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
		bool isDepthStencil = dsGfxFormat_isDepthStencil(info->format);
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->dstAccessMask = dsVkReadImageAccessFlags(offscreen->usage) |
			dsVkWriteImageAccessFlags(offscreen->usage, true, isDepthStencil);
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier->newLayout = dsVkTexture_imageLayout(offscreen);
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = vkOffscreen->deviceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		if (vkOffscreen->hostImage)
		{
			if (!addOffscreenHostEndBarrier(commandBuffer, vkOffscreen->hostImage, aspectMask))
				return false;
		}
		else
		{
			for (uint32_t i = 0; i < vkOffscreen->hostImageCount; ++i)
			{
				if (!addOffscreenHostEndBarrier(commandBuffer, vkOffscreen->hostImages[i].image,
					aspectMask))
				{
					return false;
				}
			}
		}
	}

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, stages,
		VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT, 0, 0, NULL, 0, NULL,
		vkCommandBuffer->copyImageBarrierCount, vkCommandBuffer->copyImageBarriers);
	vkCommandBuffer->copyImageBarrierCount = 0;

	return true;
}

static bool beginSubpass(dsVkDevice* device, VkCommandBuffer commandBuffer,
	dsCommandBufferUsage usage, VkRenderPass renderPass, uint32_t subpass,
	VkFramebuffer framebuffer, const VkRect2D* viewBounds, const dsVector2f* depthRange)
{
	VkCommandBufferUsageFlags usageFlags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	if (!(usage & (dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
		usageFlags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (usage & dsCommandBufferUsage_MultiSubmit)
		usageFlags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkCommandBufferInheritanceInfo inheritanceInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		NULL,
		renderPass,
		subpass,
		framebuffer,
		true,
		VK_QUERY_CONTROL_PRECISE_BIT,
		0
	};

	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		usageFlags,
		&inheritanceInfo
	};

	VkResult result = DS_VK_CALL(device->vkBeginCommandBuffer)(commandBuffer, &beginInfo);
	if (!dsHandleVkResult(result))
		return false;

	VkViewport viewport =
	{
		(float)viewBounds->offset.x,
		(float)viewBounds->offset.y,
		(float)viewBounds->extent.width,
		(float)viewBounds->extent.height,
		depthRange->x,
		depthRange->y
	};
	DS_VK_CALL(device->vkCmdSetViewport)(commandBuffer, 0, 1, &viewport);
	DS_VK_CALL(device->vkCmdSetScissor)(commandBuffer, 0, 1, viewBounds);

	return true;
}

static void resetActiveRenderState(dsVkCommandBuffer* commandBuffer)
{
	commandBuffer->activePipeline = 0;
	commandBuffer->activeVertexGeometry = NULL;
	commandBuffer->activeIndexBuffer = NULL;
}

static void resetActiveRenderAndComputeState(dsVkCommandBuffer* commandBuffer)
{
	resetActiveRenderState(commandBuffer);
	commandBuffer->activeComputePipeline = 0;
}

bool dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;

	memset(commandBuffer, 0, sizeof(*commandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;

	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		0,
		device->queueFamilyIndex
	};

	VkResult result = DS_VK_CALL(device->vkCreateCommandPool)(device->device,
		&commandPoolCreateInfo, instance->allocCallbacksPtr, &commandBuffer->commandPool);
	if (!dsHandleVkResult(result))
		return false;

	dsVkCommandBufferData_initialize(&commandBuffer->commandBufferData, allocator, device,
		commandBuffer->commandPool, false);
	dsVkCommandBufferData_initialize(&commandBuffer->subpassBufferData, allocator, device,
		commandBuffer->commandPool, true);
	dsVkBarrierList_initialize(&commandBuffer->barriers, allocator, &vkRenderer->device);
	dsVkVolatileDescriptorSets_initialize(&commandBuffer->volatileDescriptorSets, allocator,
		&vkRenderer->device);

	return true;
}

dsCommandBuffer* dsVkCommandBuffer_get(dsCommandBuffer* commandBuffer)
{
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

	return commandBuffer;
}

bool dsVkCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	return true;
}

bool dsVkCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);
	DS_UNUSED(renderer);
	dsVkCommandBuffer_finishCommandBuffer(commandBuffer);
	return true;
}

bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(renderer);
	DS_ASSERT(submitBuffer != renderer->mainCommandBuffer);
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkCommandBuffer* vkSubmitBuffer = (dsVkCommandBuffer*)submitBuffer;

	if (vkSubmitBuffer->resource &&
		!dsVkCommandBuffer_addResource(commandBuffer, vkSubmitBuffer->resource))
	{
		return false;
	}

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

	// Copy over the readback offscreens.
	for (uint32_t i = 0; i < vkSubmitBuffer->readbackOffscreenCount; ++i)
	{
		if (!dsVkCommandBuffer_addReadbackOffscreen(commandBuffer,
			vkSubmitBuffer->readbackOffscreens[i]))
		{
			return false;
		}
	}

	// Copy over the render surfaces.
	for (uint32_t i = 0; i < vkSubmitBuffer->renderSurfaceCount; ++i)
	{
		if (!dsVkCommandBuffer_addRenderSurface(commandBuffer, vkSubmitBuffer->renderSurfaces[i]))
			return false;
	}

	// Append the list of submit buffers.
	if (vkSubmitBuffer->submitBufferCount > 0)
	{
		dsVkCommandBuffer_finishCommandBuffer(commandBuffer);

		offset = vkCommandBuffer->submitBufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->submitBuffers,
			vkCommandBuffer->submitBufferCount, vkCommandBuffer->maxSubmitBuffers,
			vkSubmitBuffer->submitBufferCount))
		{
			return false;
		}

		for (uint32_t i = 0; i < vkSubmitBuffer->submitBufferCount; ++i)
			vkCommandBuffer->submitBuffers[offset + i] = vkSubmitBuffer->submitBuffers[i];
	}

	// Reset immediately if not submitted multiple times. This frees any internal references to
	// resources.
	if (!(submitBuffer->usage &
		(dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
	{
		dsVkCommandBuffer_clearUsedResources(submitBuffer);
	}

	if (vkSubmitBuffer->fenceSet)
	{
		dsVkCommandBuffer_submitFence(commandBuffer, vkSubmitBuffer->fenceReadback);
		vkSubmitBuffer->fenceSet = false;
		vkSubmitBuffer->fenceReadback = false;
	}

	return true;
}

void dsVkCommandBuffer_prepare(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;

	DS_VK_CALL(device->vkResetCommandPool)(device->device, vkCommandBuffer->commandPool, 0);
	vkCommandBuffer->activeCommandBuffer = 0;
	vkCommandBuffer->activeSubpassBuffer = 0;
	vkCommandBuffer->submitBufferCount = 0;
	resetActiveRenderAndComputeState(vkCommandBuffer);
	dsVkCommandBufferData_reset(&vkCommandBuffer->commandBufferData);
	dsVkCommandBufferData_reset(&vkCommandBuffer->subpassBufferData);
	dsVkVolatileDescriptorSets_clear(&vkCommandBuffer->volatileDescriptorSets);
}

VkCommandBuffer dsVkCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	if (vkCommandBuffer->activeSubpassBuffer)
		return vkCommandBuffer->activeSubpassBuffer;

	if (vkCommandBuffer->activeCommandBuffer)
		return vkCommandBuffer->activeCommandBuffer;

	uint32_t index = vkCommandBuffer->submitBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->submitBuffers,
		vkCommandBuffer->submitBufferCount, vkCommandBuffer->maxSubmitBuffers, 1))
	{
		return 0;
	}

	VkCommandBuffer newBuffer = dsVkCommandBufferData_getCommandBuffer(
		&vkCommandBuffer->commandBufferData);
	if (!newBuffer)
	{
		--vkCommandBuffer->submitBufferCount;
		return 0;
	}

	vkCommandBuffer->submitBuffers[index] = newBuffer;

	dsCommandBufferUsage usage = commandBuffer->usage;
	VkCommandBufferUsageFlags usageFlags = 0;
	if (!(usage & (dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
		usageFlags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (usage & dsCommandBufferUsage_MultiSubmit)
		usageFlags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		usageFlags,
		NULL
	};

	DS_VK_CALL(device->vkBeginCommandBuffer)(newBuffer, &beginInfo);
	vkCommandBuffer->activeCommandBuffer = newBuffer;
	return newBuffer;
}

void dsVkCommandBuffer_forceNewCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	vkCommandBuffer->activeCommandBuffer = 0;
	resetActiveRenderAndComputeState(vkCommandBuffer);
}

void dsVkCommandBuffer_finishCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;

	if (vkCommandBuffer->activeCommandBuffer)
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeCommandBuffer);
	vkCommandBuffer->activeCommandBuffer = 0;
	resetActiveRenderAndComputeState(vkCommandBuffer);
}

void dsVkCommandBuffer_submitFence(dsCommandBuffer* commandBuffer, bool readback)
{
	// Process immediately for the main command buffer if not in a render pass.
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer &&
		!commandBuffer->boundRenderPass)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)wrapper->realCommandBuffer;
		dsVkRenderer_flushImpl(commandBuffer->renderer, readback || vkCommandBuffer->fenceReadback,
			false);
		vkCommandBuffer->fenceSet = false;
		vkCommandBuffer->fenceReadback = false;
		return;
	}

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	vkCommandBuffer->fenceSet = true;
	if (readback)
		vkCommandBuffer->fenceReadback = true;
}

bool dsVkCommandBuffer_endSubmitCommands(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	VkCommandBuffer renderCommands = vkCommandBuffer->activeCommandBuffer;
	if (!renderCommands)
		return true;

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

bool dsVkCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer, VkRenderPass renderPass,
	VkFramebuffer framebuffer, const VkRect2D* renderArea, const dsVector2f* depthRange,
	const VkClearValue* clearValues, uint32_t clearValueCount)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	DS_ASSERT(!vkCommandBuffer->activeSubpassBuffer);

	VkCommandBuffer subpassBuffer = dsVkCommandBufferData_getCommandBuffer(
		&vkCommandBuffer->subpassBufferData);
	if (!subpassBuffer)
		return false;

	if (!beginSubpass(device, subpassBuffer, commandBuffer->usage, renderPass, 0, framebuffer,
			renderArea, depthRange))
	{
		return false;
	}

	vkCommandBuffer->clearValueCount = 0;
	if (clearValueCount > 0)
	{
		if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->clearValues,
				vkCommandBuffer->clearValueCount, vkCommandBuffer->maxClearValues, clearValueCount))
		{
			return false;
		}
		memcpy(vkCommandBuffer->clearValues, clearValues, sizeof(VkClearValue)*clearValueCount);
	}

	vkCommandBuffer->subpassBufferCount = 0;
	uint32_t index = vkCommandBuffer->subpassBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->subpassBuffers,
			vkCommandBuffer->subpassBufferCount, vkCommandBuffer->maxSubpassBuffers, 1))
	{
		return false;
	}

	vkCommandBuffer->subpassBuffers[index] = subpassBuffer;
	vkCommandBuffer->activeSubpassBuffer = subpassBuffer;
	vkCommandBuffer->activeRenderPass = renderPass;
	vkCommandBuffer->activeFramebuffer = framebuffer;
	vkCommandBuffer->renderArea = *renderArea;
	vkCommandBuffer->depthRange = *depthRange;
	return true;
}

bool dsVkCommandBuffer_nextSubpass(dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	VkCommandBuffer subpassBuffer = dsVkCommandBufferData_getCommandBuffer(
		&vkCommandBuffer->subpassBufferData);
	if (!subpassBuffer)
		return false;

	if (!beginSubpass(device, subpassBuffer, commandBuffer->usage,
			vkCommandBuffer->activeRenderPass, vkCommandBuffer->subpassBufferCount,
			vkCommandBuffer->activeFramebuffer, &vkCommandBuffer->renderArea,
			&vkCommandBuffer->depthRange))
	{
		return false;
	}

	uint32_t index = vkCommandBuffer->subpassBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->subpassBuffers,
			vkCommandBuffer->subpassBufferCount, vkCommandBuffer->maxSubpassBuffers, 1))
	{
		return false;
	}

	DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeSubpassBuffer);
	vkCommandBuffer->subpassBuffers[index] = subpassBuffer;
	vkCommandBuffer->activeSubpassBuffer = subpassBuffer;
	resetActiveRenderState(vkCommandBuffer);
	return true;
}

bool dsVkCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	VkCommandBuffer activeSubpassBuffer = vkCommandBuffer->activeSubpassBuffer;
	vkCommandBuffer->activeSubpassBuffer = 0;
	VkCommandBuffer activeCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!activeCommandBuffer)
	{
		vkCommandBuffer->activeSubpassBuffer = activeSubpassBuffer;
		return false;
	}

	DS_ASSERT(activeSubpassBuffer);
	DS_VK_CALL(device->vkEndCommandBuffer)(activeSubpassBuffer);

	VkRenderPassBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		vkCommandBuffer->activeRenderPass,
		vkCommandBuffer->activeFramebuffer,
		vkCommandBuffer->renderArea,
		vkCommandBuffer->clearValueCount, vkCommandBuffer->clearValues
	};

	DS_VK_CALL(device->vkCmdBeginRenderPass)(activeCommandBuffer, &beginInfo,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	for (uint32_t i = 0; i < vkCommandBuffer->subpassBufferCount; ++i)
	{
		DS_VK_CALL(device->vkCmdExecuteCommands)(activeCommandBuffer, 1,
			vkCommandBuffer->subpassBuffers + i);
		if (i != vkCommandBuffer->subpassBufferCount - 1)
		{
			DS_VK_CALL(device->vkCmdNextSubpass)(activeCommandBuffer,
				VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
	}

	DS_VK_CALL(device->vkCmdEndRenderPass)(activeCommandBuffer);

	vkCommandBuffer->subpassBufferCount = 0;
	resetActiveRenderState(vkCommandBuffer);
	return true;
}

void dsVkCommandBuffer_bindPipeline(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	VkPipeline pipeline)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	if (vkCommandBuffer->activePipeline == pipeline)
		return;

	DS_VK_CALL(device->vkCmdBindPipeline)(submitBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCommandBuffer->activePipeline = pipeline;
}

void dsVkCommandBuffer_bindComputePipeline(dsCommandBuffer* commandBuffer,
	VkCommandBuffer submitBuffer, VkPipeline pipeline)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	if (vkCommandBuffer->activeComputePipeline == pipeline)
		return;

	DS_VK_CALL(device->vkCmdBindPipeline)(submitBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCommandBuffer->activeComputePipeline = pipeline;
}

bool dsVkCommandBuffer_recentlyAddedImageBarrier(dsCommandBuffer* commandBuffer,
	const VkImageMemoryBarrier* barrier)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	// Check recently added barriers for duplicates. Don't check all so it's not O(n^2).
	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, vkCommandBuffer->imageBarrierCount);
	for (uint32_t i = vkCommandBuffer->imageBarrierCount - checkCount;
		i < vkCommandBuffer->imageBarrierCount; ++i)
	{
		const VkImageMemoryBarrier* curBarrier = vkCommandBuffer->imageBarriers + i;
		if (curBarrier->srcAccessMask == barrier->srcAccessMask &&
			curBarrier->dstAccessMask == barrier->dstAccessMask &&
			curBarrier->oldLayout == barrier->oldLayout &&
			curBarrier->newLayout == barrier->newLayout &&  curBarrier->image == barrier->image &&
			memcmp(&curBarrier->subresourceRange, &barrier->subresourceRange,
				sizeof(VkImageSubresourceRange)) == 0)
		{
			return true;
		}
	}

	return false;
}

VkImageMemoryBarrier* dsVkCommandBuffer_addImageBarrier(dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->imageBarrierCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->imageBarriers,
		vkCommandBuffer->imageBarrierCount, vkCommandBuffer->maxImageBarriers, 1))
	{
		return NULL;
	}

	return vkCommandBuffer->imageBarriers + index;
}

bool dsVkCommandBuffer_recentlyAddedBufferBarrier(dsCommandBuffer* commandBuffer,
	const VkBufferMemoryBarrier* barrier)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	// Check recently added barriers for duplicates. Don't check all so it's not O(n^2).
	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, vkCommandBuffer->bufferBarrierCount);
	for (uint32_t i = vkCommandBuffer->bufferBarrierCount - checkCount;
		i < vkCommandBuffer->bufferBarrierCount; ++i)
	{
		const VkBufferMemoryBarrier* curBarrier = vkCommandBuffer->bufferBarriers + i;
		if (curBarrier->srcAccessMask == barrier->srcAccessMask &&
			curBarrier->dstAccessMask == barrier->dstAccessMask &&
			curBarrier->buffer == barrier->buffer && curBarrier->offset == barrier->offset &&
			curBarrier->size == barrier->size)
		{
			return true;
		}
	}

	return false;
}

VkBufferMemoryBarrier* dsVkCommandBuffer_addBufferBarrier(dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->bufferBarrierCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->bufferBarriers,
		vkCommandBuffer->bufferBarrierCount, vkCommandBuffer->maxBufferBarriers, 1))
	{
		return NULL;
	}

	return vkCommandBuffer->bufferBarriers + index;
}

bool dsVkCommandBuffer_submitMemoryBarriers(dsCommandBuffer* commandBuffer,
	VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	if (vkCommandBuffer->imageBarrierCount == 0)
		return true;

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, srcStages, dstStages, 0, 0, NULL,
		vkCommandBuffer->bufferBarrierCount, vkCommandBuffer->bufferBarriers,
		vkCommandBuffer->imageBarrierCount, vkCommandBuffer->imageBarriers);
	vkCommandBuffer->imageBarrierCount = 0;
	return true;
}

VkImageMemoryBarrier* dsVkCommandBuffer_addCopyImageBarrier(dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	uint32_t index = vkCommandBuffer->copyImageBarrierCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->copyImageBarriers,
		vkCommandBuffer->copyImageBarrierCount, vkCommandBuffer->maxCopyImageBarriers, 1))
	{
		return NULL;
	}

	return vkCommandBuffer->copyImageBarriers + index;
}

bool dsVkCommandBuffer_submitCopyImageBarriers(dsCommandBuffer* commandBuffer,
	VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	if (vkCommandBuffer->copyImageBarrierCount == 0)
		return true;

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
	{
		vkCommandBuffer->copyImageBarrierCount = 0;
		return false;
	}

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, srcStages, dstStages, 0, 0, NULL,
		0, NULL, vkCommandBuffer->copyImageBarrierCount, vkCommandBuffer->copyImageBarriers);
	vkCommandBuffer->copyImageBarrierCount = 0;
	return true;
}

void dsVkCommandBuffer_resetCopyImageBarriers(dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	vkCommandBuffer->copyImageBarrierCount = 0;
}

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	// Check recently added resources for duplicates. Don't check all so it's not O(n^2).
	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, vkCommandBuffer->usedResourceCount);
	for (uint32_t i = vkCommandBuffer->usedResourceCount - checkCount;
		i < vkCommandBuffer->usedResourceCount; ++i)
	{
		if (vkCommandBuffer->usedResources[i] == resource)
			return true;
	}

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

bool dsVkCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsOffscreen* offscreen)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		if (vkCommandBuffer->readbackOffscreens[i] == offscreen)
			return true;
	}

	uint32_t index = vkCommandBuffer->readbackOffscreenCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->readbackOffscreens,
		vkCommandBuffer->readbackOffscreenCount, vkCommandBuffer->maxReadbackOffscreens, 1))
	{
		return false;
	}

	dsVkTexture* vkTexture = (dsVkTexture*)offscreen;
	DS_ATOMIC_FETCH_ADD32(&vkTexture->resource.commandBufferCount, 1);
	vkCommandBuffer->readbackOffscreens[index] = offscreen;
	return true;
}

bool dsVkCommandBuffer_addRenderSurface(dsCommandBuffer* commandBuffer,
	dsVkRenderSurfaceData* surface)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < vkCommandBuffer->renderSurfaceCount; ++i)
	{
		if (vkCommandBuffer->renderSurfaces[i] == surface)
			return true;
	}

	uint32_t index = vkCommandBuffer->renderSurfaceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->renderSurfaces,
		vkCommandBuffer->renderSurfaceCount, vkCommandBuffer->maxRenderSurfaces, 1))
	{
		return false;
	}

	DS_ATOMIC_FETCH_ADD32(&surface->resource.commandBufferCount, 1);
	vkCommandBuffer->renderSurfaces[index] = surface;
	return true;
}

void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
		DS_ATOMIC_FETCH_ADD32(&vkCommandBuffer->usedResources[i]->commandBufferCount, -1);

	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsVkTexture* vkTexture = (dsVkTexture*)vkCommandBuffer->readbackOffscreens[i];
		DS_ATOMIC_FETCH_ADD32(&vkTexture->resource.commandBufferCount, -1);
	}

	for (uint32_t i = 0; i < vkCommandBuffer->renderSurfaceCount; ++i)
		DS_ATOMIC_FETCH_ADD32(&vkCommandBuffer->renderSurfaces[i]->resource.commandBufferCount, -1);

	vkCommandBuffer->usedResourceCount = 0;
	vkCommandBuffer->readbackOffscreenCount = 0;
	vkCommandBuffer->renderSurfaceCount = 0;
}

void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->usedResourceCount; ++i)
	{
		dsVkResource* resource = vkCommandBuffer->usedResources[i];
		DS_ATOMIC_FETCH_ADD32(&resource->commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
		resource->lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_unlock(&resource->lock));
	}

	vkCommandBuffer->usedResourceCount = 0;
}

void dsVkCommandBuffer_submittedReadbackOffscreens(dsCommandBuffer* commandBuffer,
	uint64_t submitCount)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsVkTexture* texture = (dsVkTexture*)vkCommandBuffer->readbackOffscreens[i];
		DS_ATOMIC_FETCH_ADD32(&texture->resource.commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&texture->resource.lock));
		texture->resource.lastUsedSubmit = submitCount;
		texture->lastDrawSubmit = submitCount;
		DS_VERIFY(dsSpinlock_unlock(&texture->resource.lock));
	}
	vkCommandBuffer->readbackOffscreenCount = 0;
}

void dsVkCommandBuffer_submittedRenderSurfaces(dsCommandBuffer* commandBuffer,
	uint64_t submitCount)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	for (uint32_t i = 0; i < vkCommandBuffer->renderSurfaceCount; ++i)
	{
		dsVkRenderSurfaceData* surface = vkCommandBuffer->renderSurfaces[i];
		DS_ATOMIC_FETCH_ADD32(&surface->resource.commandBufferCount, -1);
		DS_VERIFY(dsSpinlock_lock(&surface->resource.lock));
		surface->resource.lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_unlock(&surface->resource.lock));
	}
	vkCommandBuffer->renderSurfaceCount = 0;
}

dsVkVolatileDescriptorSets* dsVkCommandBuffer_getVolatileDescriptorSets(
	dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	return &vkCommandBuffer->volatileDescriptorSets;
}

uint8_t* dsVkCommandBuffer_allocatePushConstantData(dsCommandBuffer* commandBuffer, uint32_t size)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	uint32_t count = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->pushConstantBytes,
		count, vkCommandBuffer->maxPushConstantBytes, size))
	{
		return NULL;
	}

	return vkCommandBuffer->pushConstantBytes;
}

void dsVkCommandBuffer_shutdown(dsVkCommandBuffer* commandBuffer)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	 // Not initialized.
	if (!baseCommandBuffer->renderer)
		return;

	dsVkDevice* device = &((dsVkRenderer*)baseCommandBuffer->renderer)->device;
	dsVkInstance* instance = &device->instance;

	if (commandBuffer->commandPool)
	{
		DS_VK_CALL(device->vkDestroyCommandPool)(device->device, commandBuffer->commandPool,
			instance->allocCallbacksPtr);
	}

	dsVkCommandBufferData_shutdown(&commandBuffer->commandBufferData);
	dsVkCommandBufferData_shutdown(&commandBuffer->subpassBufferData);
	dsVkBarrierList_shutdown(&commandBuffer->barriers);
	dsVkCommandBuffer_clearUsedResources(baseCommandBuffer);
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->clearValues));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->submitBuffers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->usedResources));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->readbackOffscreens));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->renderSurfaces));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->bufferBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->copyImageBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->subpassBuffers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageCopies));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->pushConstantBytes));
	dsVkVolatileDescriptorSets_shutdown(&commandBuffer->volatileDescriptorSets);
}
