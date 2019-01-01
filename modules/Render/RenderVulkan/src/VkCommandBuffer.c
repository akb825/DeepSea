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

#include "Resources/VkFramebuffer.h"
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
		commandBuffer->commandPool, false);
	dsVkBarrierList_initialize(&commandBuffer->barriers, allocator, &vkRenderer->device);
	dsVkVolatileDescriptorSets_initialize(&commandBuffer->volatileDescriptorSets, allocator,
		&vkRenderer->device);

	return true;
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
	if (commandBuffer == renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

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
	dsVkCommandBufferData_reset(&vkCommandBuffer->commandBufferData);
	dsVkCommandBufferData_reset(&vkCommandBuffer->subpassBufferData);
	dsVkVolatileDescriptorSets_clear(&vkCommandBuffer->volatileDescriptorSets);
}

VkCommandBuffer dsVkCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)wrapper;
		commandBuffer = wrapper->realCommandBuffer;
	}

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	if (commandBuffer->boundRenderPass)
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
	return newBuffer;
}

void dsVkCommandBuffer_forceNewCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	vkCommandBuffer->activeCommandBuffer = 0;
}

void dsVkCommandBuffer_finishCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != commandBuffer->renderer->mainCommandBuffer);
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;

	if (vkCommandBuffer->activeCommandBuffer)
		DS_VK_CALL(device->vkEndCommandBuffer)(vkCommandBuffer->activeCommandBuffer);
	vkCommandBuffer->activeCommandBuffer = 0;
}

void dsVkCommandBuffer_submitFence(dsCommandBuffer* commandBuffer, bool readback)
{
	// Process immediately for the main command buffer if not in a render pass.
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer &&
		!commandBuffer->boundRenderPass)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)wrapper->realCommandBuffer;
		dsVkRenderer_flushImpl(commandBuffer->renderer, readback || vkCommandBuffer->fenceReadback);
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

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource)
{
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

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

bool dsVkCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsOffscreen* offscreen)
{
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

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
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

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
		DS_VERIFY(dsSpinlock_lock(&resource->lock));
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
		DS_VERIFY(dsSpinlock_lock(&texture->resource.lock));
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
		DS_VERIFY(dsSpinlock_lock(&surface->resource.lock));
	}
	vkCommandBuffer->renderSurfaceCount = 0;
}

dsVkVolatileDescriptorSets* dsVkCommandBuffer_getVolatileDescriptorSets(
	dsCommandBuffer* commandBuffer)
{
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)commandBuffer;
	return &vkCommandBuffer->volatileDescriptorSets;
}

uint8_t* dsVkCommandBuffer_allocatePushConstantData(dsCommandBuffer* commandBuffer, uint32_t size)
{
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer)
	{
		dsVkCommandBufferWrapper* wrapper = (dsVkCommandBufferWrapper*)commandBuffer;
		commandBuffer = wrapper->realCommandBuffer;
	}

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
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->submitBuffers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->usedResources));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->readbackOffscreens));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->renderSurfaces));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageBarriers));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->imageCopies));
	DS_VERIFY(dsAllocator_free(baseCommandBuffer->allocator, commandBuffer->pushConstantBytes));
	dsVkVolatileDescriptorSets_shutdown(&commandBuffer->volatileDescriptorSets);
}
