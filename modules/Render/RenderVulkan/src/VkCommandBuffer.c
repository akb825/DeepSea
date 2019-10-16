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
#include "Resources/VkRealFramebuffer.h"
#include "Resources/VkTempBuffer.h"
#include "Resources/VkTexture.h"
#include "VkBarrierList.h"
#include "VkCommandBufferData.h"
#include "VkRendererInternal.h"
#include "VkRenderPass.h"
#include "VkShared.h"
#include "VkSubpassBuffers.h"
#include "VkSharedDescriptorSets.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

static VkCommandBuffer getMainCommandBuffer(dsCommandBuffer* commandBuffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
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

static bool processOffscreenReadbacks(dsCommandBuffer* commandBuffer,
	VkCommandBuffer renderCommands)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	DS_ASSERT(vkCommandBuffer->bufferBarrierCount == 0);
	DS_ASSERT(vkCommandBuffer->imageBarrierCount == 0);
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsOffscreen* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
		VkBufferMemoryBarrier* bufferBarrier =
			dsVkCommandBuffer_addBufferBarrier(commandBuffer);
		if (!bufferBarrier)
		{
			vkCommandBuffer->bufferBarrierCount = 0;
			vkCommandBuffer->imageBarrierCount = 0;
			return false;
		}

		bufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier->pNext = NULL;
		bufferBarrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
		bufferBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		bufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->buffer = vkOffscreen->hostBuffer;
		bufferBarrier->offset = 0;
		bufferBarrier->size = vkOffscreen->hostMemorySize;

		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!imageBarrier)
		{
			vkCommandBuffer->bufferBarrierCount = 0;
			vkCommandBuffer->imageBarrierCount = 0;
			return false;
		}

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
		VkImageLayout layout = dsVkTexture_imageLayout(offscreen);
		bool isDepthStencil = dsGfxFormat_isDepthStencil(info->format);
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = dsVkReadImageAccessFlags(offscreen->usage) |
			dsVkWriteImageAccessFlags(offscreen->usage, true, isDepthStencil);
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->oldLayout = layout;
		imageBarrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = vkOffscreen->deviceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	}

	VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	if (renderer->hasTessellationShaders)
	{
		stages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		stages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, stages | VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
		vkCommandBuffer->bufferBarrierCount, vkCommandBuffer->bufferBarriers,
		vkCommandBuffer->imageBarrierCount, vkCommandBuffer->imageBarriers);
	vkCommandBuffer->bufferBarrierCount = 0;
	vkCommandBuffer->imageBarrierCount = 0;

	// Copy offscreen texture data to host images that can be read back from.
	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsTexture* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;

		uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
		bool is3D = info->dimension == dsTextureDim_3D;

		uint32_t imageCopiesCount = 0;
		if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->imageCopies,
				imageCopiesCount, vkCommandBuffer->maxImageCopies, info->mipLevels))
		{
			return false;
		}

		size_t offset = 0;
		dsTextureInfo surfaceInfo = offscreen->info;
		surfaceInfo.mipLevels = 1;
		for (uint32_t j = 0; j < info->mipLevels; ++j)
		{
			uint32_t width = info->width >> j;
			uint32_t height = info->height >> j;
			uint32_t depth = is3D ? info->depth >> j : info->depth;
			width = dsMax(1U, width);
			height = dsMax(1U, height);
			depth = dsMax(1U, depth);

			uint32_t layerCount = faceCount*(is3D ? 1U : depth);
			VkBufferImageCopy* imageCopy = vkCommandBuffer->imageCopies + j;
			imageCopy->bufferOffset = offset;
			imageCopy->bufferRowLength = 0;
			imageCopy->bufferImageHeight = 0;
			imageCopy->imageSubresource.aspectMask = vkOffscreen->aspectMask;
			imageCopy->imageSubresource.mipLevel = j;
			imageCopy->imageSubresource.baseArrayLayer = 0;
			imageCopy->imageSubresource.layerCount = layerCount;
			imageCopy->imageOffset.x = 0;
			imageCopy->imageOffset.y = 0;
			imageCopy->imageOffset.z = 0;
			imageCopy->imageExtent.width = width;
			imageCopy->imageExtent.height = height;
			imageCopy->imageExtent.depth = is3D ? depth : 1U;

			surfaceInfo.width = width;
			surfaceInfo.height = height;
			if (is3D)
				surfaceInfo.depth = depth;
			offset += dsTexture_size(&surfaceInfo);
		}
		DS_ASSERT(offset <= vkOffscreen->hostMemorySize);

		DS_VK_CALL(device->vkCmdCopyImageToBuffer)(renderCommands, vkOffscreen->deviceImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkOffscreen->hostBuffer, imageCopiesCount,
			vkCommandBuffer->imageCopies);
	}

	for (uint32_t i = 0; i < vkCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsOffscreen* offscreen = vkCommandBuffer->readbackOffscreens[i];
		DS_ASSERT(offscreen->offscreen);
		const dsTextureInfo* info = &offscreen->info;
		dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
		VkBufferMemoryBarrier* bufferBarrier =
			dsVkCommandBuffer_addBufferBarrier(commandBuffer);
		DS_ASSERT(bufferBarrier);
		bufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier->pNext = NULL;
		bufferBarrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		bufferBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
		bufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->buffer = vkOffscreen->hostBuffer;
		bufferBarrier->offset = 0;
		bufferBarrier->size = vkOffscreen->hostMemorySize;

		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		DS_ASSERT(imageBarrier);
		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(info->format);
		VkImageLayout layout = dsVkTexture_imageLayout(offscreen);
		bool isDepthStencil = dsGfxFormat_isDepthStencil(info->format);
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->dstAccessMask = dsVkReadImageAccessFlags(offscreen->usage) |
			dsVkWriteImageAccessFlags(offscreen->usage, true, isDepthStencil);
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier->newLayout = layout;
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->image = vkOffscreen->deviceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = 0;
		imageBarrier->subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	}

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_HOST_BIT | stages, 0, 0, NULL,
		vkCommandBuffer->bufferBarrierCount, vkCommandBuffer->bufferBarriers,
		vkCommandBuffer->imageBarrierCount, vkCommandBuffer->imageBarriers);
	vkCommandBuffer->bufferBarrierCount = 0;
	vkCommandBuffer->imageBarrierCount = 0;

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

	VkQueryControlFlags queryControlFlags = 0;
	if (device->features.inheritedQueries && device->features.occlusionQueryPrecise)
		queryControlFlags = VK_QUERY_CONTROL_PRECISE_BIT;
	VkCommandBufferInheritanceInfo inheritanceInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		NULL,
		renderPass,
		subpass,
		framebuffer,
		device->features.inheritedQueries,
		queryControlFlags,
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
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't begin command buffer"))
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
	commandBuffer->activeShader = NULL;
	commandBuffer->activePipeline = 0;
	commandBuffer->activeVertexGeometry = NULL;
	commandBuffer->activeIndexBuffer = NULL;
	memset(commandBuffer->activeDescriptorSets[VK_PIPELINE_BIND_POINT_GRAPHICS], 0,
		sizeof(commandBuffer->activeDescriptorSets[VK_PIPELINE_BIND_POINT_GRAPHICS]));
}

static void resetActiveRenderAndComputeState(dsVkCommandBuffer* commandBuffer)
{
	resetActiveRenderState(commandBuffer);
	commandBuffer->activeComputeShader = NULL;
	commandBuffer->activeComputePipeline = 0;
	memset(commandBuffer->activeDescriptorSets[VK_PIPELINE_BIND_POINT_COMPUTE], 0,
		sizeof(commandBuffer->activeDescriptorSets[VK_PIPELINE_BIND_POINT_COMPUTE]));
}

bool dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;

	DS_ASSERT(allocator->freeFunc);
	memset(commandBuffer, 0, sizeof(*commandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;

	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		usage & dsCommandBufferUsage_MultiFrame  ? 0 : VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		device->queueFamilyIndex
	};

	VkResult result = DS_VK_CALL(device->vkCreateCommandPool)(device->device,
		&commandPoolCreateInfo, instance->allocCallbacksPtr, &commandBuffer->commandPool);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create command pool"))
		return false;

	if (!(usage & dsCommandBufferUsage_Secondary))
	{
		dsVkCommandBufferData_initialize(&commandBuffer->commandBufferData, allocator, device,
			commandBuffer->commandPool, false);
	}
	dsVkCommandBufferData_initialize(&commandBuffer->subpassBufferData, allocator, device,
		commandBuffer->commandPool, true);
	dsVkBarrierList_initialize(&commandBuffer->barriers, allocator, &vkRenderer->device);
	dsVkSharedDescriptorSets_initialize(&commandBuffer->globalDescriptorSets, renderer, allocator,
		dsMaterialBinding_Global);
	dsVkSharedDescriptorSets_initialize(&commandBuffer->instanceDescriptorSets, renderer,
		allocator, dsMaterialBinding_Instance);
	dsVkSubpassBuffers_initialize(&commandBuffer->subpassBuffers, allocator);

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
	dsVkCommandBuffer_prepare(commandBuffer);
	dsVkCommandBuffer_clearUsedResources(commandBuffer);
	return true;
}

bool dsVkCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);

	dsVkCommandBuffer_prepare(commandBuffer);
	dsVkCommandBuffer_clearUsedResources(commandBuffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	const dsVkRenderPassData* renderPassData = dsVkRenderPass_getData(renderPass);
	if (!renderPassData)
		return false;

	VkFramebuffer vkFramebuffer = 0;
	// Avoid using the framebuffer if can submit across frames.
	if (framebuffer && !(commandBuffer->usage & dsCommandBufferUsage_MultiFrame))
	{
		dsVkRealFramebuffer* realFramebuffer = dsVkFramebuffer_getRealFramebuffer(
			(dsFramebuffer*)framebuffer, commandBuffer, renderPassData);
		if (!realFramebuffer)
			return false;

		vkFramebuffer = dsVkRealFramebuffer_getFramebuffer(realFramebuffer);
		DS_ASSERT(vkFramebuffer);
	}

	VkCommandBuffer subpassBuffer = dsVkCommandBufferData_getCommandBuffer(
		&vkCommandBuffer->subpassBufferData);
	if (!subpassBuffer)
		return false;

	VkRect2D renderArea;
	dsVector2f depthRange;
	if (viewport)
	{
		renderArea.offset.x = (int32_t)floorf((float)framebuffer->width*viewport->min.x);
		renderArea.offset.y = (int32_t)floorf((float)framebuffer->height*viewport->min.y);
		renderArea.extent.width = (uint32_t)ceilf((float)(viewport->max.x - viewport->min.x));
		renderArea.extent.height = (uint32_t)ceilf((float)(viewport->max.y - viewport->min.y));
		depthRange.x = viewport->min.z;
		depthRange.y = viewport->max.z;
	}
	else
	{
		DS_ASSERT(framebuffer);
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		renderArea.extent.width = framebuffer->width;
		renderArea.extent.height = framebuffer->height;
		depthRange.x = 0.0f;
		depthRange.y = 1.0f;
	}

	if (!beginSubpass(&vkRenderer->device, subpassBuffer, commandBuffer->usage,
			renderPassData->vkRenderPass, subpass, vkFramebuffer, &renderArea, &depthRange))
	{
		return false;
	}

	vkCommandBuffer->activeSubpassBuffer = subpassBuffer;
	vkCommandBuffer->activeRenderPass = renderPassData->vkRenderPass;
	vkCommandBuffer->activeFramebuffer = vkFramebuffer;
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
	bool isSecondary = (submitBuffer->usage & dsCommandBufferUsage_Secondary) != 0;
	if (isSecondary && vkSubmitBuffer->activeSubpassBuffer)
	{
		DS_ASSERT(vkCommandBuffer->activeRenderPass == vkSubmitBuffer->activeRenderPass);
		DS_ASSERT(!vkSubmitBuffer->activeFramebuffer ||
			vkCommandBuffer->activeFramebuffer == vkSubmitBuffer->activeFramebuffer);
		if (!dsVkSubpassBuffers_addCommandBuffer(&vkCommandBuffer->subpassBuffers,
				vkSubmitBuffer->activeSubpassBuffer))
		{
			return false;
		}
		vkCommandBuffer->activeSubpassBuffer = 0;
	}
	else if (!isSecondary && vkSubmitBuffer->submitBufferCount > 0)
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
	DS_PROFILE_FUNC_START();
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
	DS_PROFILE_FUNC_RETURN_VOID();
}

VkCommandBuffer dsVkCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	// NOTE: If need to get a different underlying command buffer, this value needs to come from
	// the original command buffer.
	uint32_t activeRenderSubpass = commandBuffer->activeRenderSubpass;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	if (vkCommandBuffer->activeRenderPass)
	{
		if (vkCommandBuffer->activeSubpassBuffer)
			return vkCommandBuffer->activeSubpassBuffer;

		dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
		VkCommandBuffer subpassBuffer = dsVkCommandBufferData_getCommandBuffer(
			&vkCommandBuffer->subpassBufferData);
		if (!subpassBuffer)
			return 0;

		if (!beginSubpass(device, subpassBuffer, commandBuffer->usage,
				vkCommandBuffer->activeRenderPass, activeRenderSubpass,
				vkCommandBuffer->activeFramebuffer, &vkCommandBuffer->renderArea,
				&vkCommandBuffer->depthRange))
		{
			return 0;
		}

		if (!dsVkSubpassBuffers_addCommandBuffer(&vkCommandBuffer->subpassBuffers, subpassBuffer))
			return false;
		vkCommandBuffer->activeSubpassBuffer = subpassBuffer;
		return subpassBuffer;
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG,
			"Invalid location to request Vulkan command buffer from a secondary command buffer.");
		return 0;
	}
	return getMainCommandBuffer(commandBuffer);
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
	DS_PROFILE_FUNC_START();
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;

	if (vkCommandBuffer->activeCommandBuffer)
	{
		DS_ASSERT(!vkCommandBuffer->activeSubpassBuffer);
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeCommandBuffer);
		vkCommandBuffer->activeCommandBuffer = 0;
	}
	else if ((commandBuffer->usage & dsCommandBufferUsage_Secondary) &&
		vkCommandBuffer->activeSubpassBuffer)
	{
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeSubpassBuffer);
	}

	resetActiveRenderAndComputeState(vkCommandBuffer);
	DS_PROFILE_FUNC_RETURN_VOID();
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

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)dsVkCommandBuffer_get(commandBuffer);
	vkCommandBuffer->fenceSet = true;
	if (readback)
		vkCommandBuffer->fenceReadback = true;
}

bool dsVkCommandBuffer_endSubmitCommands(dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	// First submit buffer is always for resource processing.
	if (vkCommandBuffer->submitBufferCount <= 1)
		DS_PROFILE_FUNC_RETURN(true);

	VkCommandBuffer renderCommands = getMainCommandBuffer(commandBuffer);
	if (!renderCommands)
		DS_PROFILE_FUNC_RETURN(false);

	// Copy the readback offscreens.
	if (!processOffscreenReadbacks(commandBuffer, renderCommands))
		DS_PROFILE_FUNC_RETURN(false);

	// Make sure any writes are visible for mapping buffers.
	VkMemoryBarrier memoryBarrier =
	{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
			VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
			VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(renderCommands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	DS_PROFILE_FUNC_RETURN(true);
}

bool dsVkCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer, VkRenderPass renderPass,
	VkFramebuffer framebuffer, const VkRect2D* renderArea, const dsVector2f* depthRange,
	const VkClearValue* clearValues, uint32_t clearValueCount)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	DS_ASSERT(!vkCommandBuffer->activeSubpassBuffer);

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

	dsVkSubpassBuffers_reset(&vkCommandBuffer->subpassBuffers);
	if (!dsVkSubpassBuffers_addSubpass(&vkCommandBuffer->subpassBuffers))
		return false;

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

	if (!dsVkSubpassBuffers_addSubpass(&vkCommandBuffer->subpassBuffers))
		return false;

	if (vkCommandBuffer->activeSubpassBuffer)
	{
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeSubpassBuffer);
		vkCommandBuffer->activeSubpassBuffer = 0;
	}
	resetActiveRenderState(vkCommandBuffer);
	return true;
}

bool dsVkCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;

	VkCommandBuffer activeCommandBuffer = getMainCommandBuffer(commandBuffer);
	if (!activeCommandBuffer)
		return false;

	if (vkCommandBuffer->activeSubpassBuffer)
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeSubpassBuffer);

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

	for (uint32_t i = 0; i < vkCommandBuffer->subpassBuffers.subpassCount; ++i)
	{
		const dsVkSubpassBufferRange* subpass = vkCommandBuffer->subpassBuffers.subpasses + i;
		if (subpass->count > 0)
		{
			DS_VK_CALL(device->vkCmdExecuteCommands)(activeCommandBuffer,
				subpass->count, vkCommandBuffer->subpassBuffers.commandBuffers + subpass->start);
		}

		if (i != vkCommandBuffer->subpassBuffers.subpassCount - 1)
		{
			DS_VK_CALL(device->vkCmdNextSubpass)(activeCommandBuffer,
				VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
	}

	DS_VK_CALL(device->vkCmdEndRenderPass)(activeCommandBuffer);

	dsVkSubpassBuffers_reset(&vkCommandBuffer->subpassBuffers);
	vkCommandBuffer->activeSubpassBuffer = 0;
	vkCommandBuffer->activeRenderPass = 0;
	vkCommandBuffer->activeFramebuffer = 0;
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

void dsVkCommandBuffer_bindDescriptorSet(dsCommandBuffer* commandBuffer,
	VkCommandBuffer submitBuffer, VkPipelineBindPoint bindPoint, uint32_t setIndex,
	VkPipelineLayout layout, VkDescriptorSet descriptorSet, const uint32_t* offsets,
	uint32_t offsetCount)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	if (!offsets && vkCommandBuffer->activeDescriptorSets[bindPoint][setIndex] == descriptorSet)
		return;

	DS_VK_CALL(device->vkCmdBindDescriptorSets)(submitBuffer, bindPoint, layout, setIndex, 1,
		&descriptorSet, offsetCount, offsets);
	vkCommandBuffer->activeDescriptorSets[bindPoint][setIndex] = descriptorSet;
}

void* dsVkCommandBuffer_getTempData(size_t* outOffset, VkBuffer* outBuffer,
	dsCommandBuffer* commandBuffer, size_t size, uint32_t alignment)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	// Too large for the temp buffer pools, create a temp buffer and destroy it once finished.
	if (size > DS_MAX_TEMP_BUFFER_ALLOC)
	{
		dsVkTempBuffer* buffer = dsVkTempBuffer_create(commandBuffer->allocator, device, size);
		if (!buffer)
			return NULL;

		if (!dsVkCommandBuffer_addResource(commandBuffer, &buffer->resource))
		{
			dsVkTempBuffer_destroy(buffer);
			return NULL;
		}
		dsVkRenderer_deleteTempBuffer(renderer, buffer);

		*outBuffer = buffer->buffer;
		return dsVkTempBuffer_allocate(outOffset, buffer, size, alignment);
	}

	if (vkCommandBuffer->curTempBuffer)
	{
		*outBuffer = vkCommandBuffer->curTempBuffer->buffer;
		void* data =
			dsVkTempBuffer_allocate(outOffset, vkCommandBuffer->curTempBuffer, size, alignment);
		if (data)
			return data;
	}

	uint64_t finishedSubmitCount = dsVkRenderer_getFinishedSubmitCount(commandBuffer->renderer);
	for (uint32_t i = 0; i < vkCommandBuffer->tempBufferCount; ++i)
	{
		dsVkTempBuffer* buffer = vkCommandBuffer->tempBuffers[i];
		if (!dsVkTempBuffer_reset(buffer, finishedSubmitCount))
			continue;

		vkCommandBuffer->curTempBuffer = buffer;
		dsVkCommandBuffer_addResource(commandBuffer, &buffer->resource);
		*outBuffer = buffer->buffer;
		return dsVkTempBuffer_allocate(outOffset, buffer, size, alignment);
	}

	dsVkTempBuffer* buffer = dsVkTempBuffer_create(commandBuffer->allocator, device,
		DS_TEMP_BUFFER_CAPACITY);
	if (!buffer)
		return NULL;

	uint32_t index = vkCommandBuffer->tempBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, vkCommandBuffer->tempBuffers,
			vkCommandBuffer->tempBufferCount, vkCommandBuffer->maxTempBuffers, 1))
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	if (!dsVkCommandBuffer_addResource(commandBuffer, &buffer->resource))
	{
		dsVkTempBuffer_destroy(buffer);
		--vkCommandBuffer->tempBufferCount;
		return NULL;
	}

	vkCommandBuffer->tempBuffers[index] = buffer;
	vkCommandBuffer->curTempBuffer = buffer;
	*outBuffer = buffer->buffer;
	return dsVkTempBuffer_allocate(outOffset, buffer, size, alignment);
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
	if (vkCommandBuffer->imageBarrierCount == 0 && vkCommandBuffer->bufferBarrierCount == 0)
		return true;

	VkCommandBuffer submitBuffer = getMainCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, srcStages, dstStages, 0, 0, NULL,
		vkCommandBuffer->bufferBarrierCount, vkCommandBuffer->bufferBarriers,
		vkCommandBuffer->imageBarrierCount, vkCommandBuffer->imageBarriers);
	vkCommandBuffer->imageBarrierCount = 0;
	vkCommandBuffer->bufferBarrierCount = 0;
	return true;
}

void dsVkCommandBuffer_resetMemoryBarriers(dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	vkCommandBuffer->bufferBarrierCount = 0;
	vkCommandBuffer->imageBarrierCount = 0;
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
	vkCommandBuffer->curTempBuffer = NULL;

	dsVkSharedDescriptorSets_clearLastSet(&vkCommandBuffer->globalDescriptorSets);
	dsVkSharedDescriptorSets_clearLastSet(&vkCommandBuffer->instanceDescriptorSets);
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
	vkCommandBuffer->curTempBuffer = NULL;

	dsVkSharedDescriptorSets_clearLastSet(&vkCommandBuffer->globalDescriptorSets);
	dsVkSharedDescriptorSets_clearLastSet(&vkCommandBuffer->instanceDescriptorSets);
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
		surface->imageData[surface->imageIndex].lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_unlock(&surface->resource.lock));
	}
	vkCommandBuffer->renderSurfaceCount = 0;
}

dsVkSharedDescriptorSets* dsVkCommandBuffer_getGlobalDescriptorSets(
	dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	return &vkCommandBuffer->globalDescriptorSets;
}

dsVkSharedDescriptorSets* dsVkCommandBuffer_getInstanceDescriptorSets(
	dsCommandBuffer* commandBuffer)
{
	commandBuffer = dsVkCommandBuffer_get(commandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	return &vkCommandBuffer->instanceDescriptorSets;
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

	dsRenderer* renderer = baseCommandBuffer->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
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
	for (uint32_t i = 0; i < commandBuffer->tempBufferCount; ++i)
		dsVkRenderer_deleteTempBuffer(renderer, commandBuffer->tempBuffers[i]);
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->tempBuffers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->readbackOffscreens));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->renderSurfaces));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->bufferBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageBarriers));
	dsVkSubpassBuffers_shutdown(&commandBuffer->subpassBuffers);
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageCopies));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->pushConstantBytes));
	dsVkSharedDescriptorSets_shutdown(&commandBuffer->globalDescriptorSets);
	dsVkSharedDescriptorSets_shutdown(&commandBuffer->instanceDescriptorSets);
}
