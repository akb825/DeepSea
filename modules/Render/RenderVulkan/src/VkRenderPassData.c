/*
 * Copyright 2019 Aaron Barany
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

#include "VkRenderPassData.h"

#include "Resources/VkFramebuffer.h"
#include "Resources/VkRealFramebuffer.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "Resources/VkShader.h"
#include "Resources/VkTexture.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool submitResourceBarriers(dsCommandBuffer* commandBuffer)
{
	dsRenderer* renderer = commandBuffer->renderer;
	VkPipelineStageFlagBits dstStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if (renderer->hasTessellationShaders)
	{
		dstStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		dstStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

	VkPipelineStageFlags srcStages = dstStages | VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT;
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, dstStages);
}

static bool beginFramebuffer(dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer)
{
	dsRenderer* renderer = commandBuffer->renderer;
	bool hasImages = false;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		if (surface->surfaceType != dsGfxSurfaceType_Texture)
			continue;

		dsTexture* texture = (dsTexture*)surface->surface;
		DS_ASSERT(texture->offscreen);
		dsVkRenderer_processTexture(renderer, texture);
		if (dsVkTexture_canReadBack(texture) &&
			!dsVkCommandBuffer_addReadbackOffscreen(commandBuffer, texture))
		{
			return false;
		}

		// Don't layout transition for resolved depth/stencil images, since you can't resolve
		// in render subpasses.
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		if (vkTexture->surfaceImage && dsGfxFormat_isDepthStencil(texture->info.format))
			continue;

		if (texture->usage & dsTextureUsage_Image)
			hasImages = true;

		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!imageBarrier)
			return false;

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(texture->info.format);
		bool isDepthStencil = dsGfxFormat_isDepthStencil(texture->info.format);
		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
			VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT |
			VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier->oldLayout = dsVkTexture_imageLayout(texture);
		if (isDepthStencil)
		{
			imageBarrier->srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			imageBarrier->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			imageBarrier->newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			imageBarrier->srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageBarrier->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageBarrier->newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
		imageBarrier->image = vkTexture->deviceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
		imageBarrier->subresourceRange.levelCount = 1;
		imageBarrier->subresourceRange.baseArrayLayer = surface->layer*faceCount +
			surface->cubeFace;
		imageBarrier->subresourceRange.layerCount = framebuffer->layers;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkPipelineStageFlags dstStages = srcStages;
	if (hasImages)
	{
		srcStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (renderer->hasTessellationShaders)
		{
			srcStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
				VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
		if (renderer->hasGeometryShaders)
			srcStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, dstStages);
}

static void setEndImageBarrier(VkImageMemoryBarrier* imageBarrier, const dsFramebuffer* framebuffer,
	const dsFramebufferSurface* surface, dsGfxFormat format, VkImage image, VkImageLayout layout,
	uint32_t baseLayer)
{
	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(format);
	bool isDepthStencil = dsGfxFormat_isDepthStencil(format);
	imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier->pNext = NULL;
	imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
		VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	if (isDepthStencil)
	{
		imageBarrier->srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageBarrier->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	else
	{
		imageBarrier->srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	imageBarrier->newLayout = layout;

	imageBarrier->image = image;
	imageBarrier->subresourceRange.aspectMask = aspectMask;
	imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
	imageBarrier->subresourceRange.levelCount = 1;
	imageBarrier->subresourceRange.baseArrayLayer = baseLayer;
	imageBarrier->subresourceRange.layerCount = framebuffer->layers;
}

static bool endFramebuffer(dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer,
	const bool* resolveAttachment)
{
	dsRenderer* renderer = commandBuffer->renderer;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			{
				dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface->surface;
				dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
				if (!surfaceData->resolveImage || !resolveAttachment[i])
					continue;

				// Need to have copy format to resolve.
				VkImageMemoryBarrier* imageBarrier =
					dsVkCommandBuffer_addImageBarrier(commandBuffer);
				if (!imageBarrier)
					return false;

				uint32_t layer = surface->surfaceType == dsGfxSurfaceType_ColorRenderSurfaceRight;
				setEndImageBarrier(imageBarrier, framebuffer, surface, renderer->surfaceColorFormat,
					surfaceData->images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer);
				setEndImageBarrier(imageBarrier, framebuffer, surface, renderer->surfaceColorFormat,
					surfaceData->resolveImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0);
				break;
			}
			case dsGfxSurfaceType_Texture:
			{
				dsTexture* texture = (dsTexture*)surface->surface;
				DS_ASSERT(texture->offscreen);
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				// Multisampled depth/stencil images weren't transitioned.
				if (!vkTexture->surfaceImage || !dsGfxFormat_isDepthStencil(texture->info.format))
				{
					VkImageMemoryBarrier* imageBarrier =
						dsVkCommandBuffer_addImageBarrier(commandBuffer);
					if (!imageBarrier)
						return false;

					uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
					setEndImageBarrier(imageBarrier, framebuffer, surface, texture->info.format,
						vkTexture->deviceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						surface->layer*faceCount + surface->cubeFace);
				}

				// Manually resolved images.
				if (vkTexture->surfaceImage && resolveAttachment[i])
				{
					VkImageMemoryBarrier* imageBarrier =
						dsVkCommandBuffer_addImageBarrier(commandBuffer);
					if (!imageBarrier)
						return false;

					setEndImageBarrier(imageBarrier, framebuffer, surface, texture->info.format,
						vkTexture->surfaceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0);
				}
				break;
			}
			default:
				break;
		}
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkPipelineStageFlags dstStages = srcStages | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (renderer->hasTessellationShaders)
	{
		dstStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		dstStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	if (!dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, dstStages))
		return false;

	// Resolved images.
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		if (!resolveAttachment[i])
			continue;

		dsTextureUsage usage = dsTextureUsage_CopyTo;
		dsGfxFormat format;
		uint32_t firstLayer;
		VkImage multisampleImage, finalImage;
		VkImageLayout finalLayout;
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			{
				dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface->surface;
				dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
				if (!surfaceData->resolveImage)
					continue;

				format = renderer->surfaceColorFormat;
				firstLayer = surface->surfaceType == dsGfxSurfaceType_ColorRenderSurfaceRight;
				multisampleImage = surfaceData->resolveImage;
				finalImage = surfaceData->images[surfaceData->imageIndex];
				finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			case dsGfxSurfaceType_Texture:
			{
				dsTexture* texture = (dsTexture*)surface->surface;
				DS_ASSERT(texture->offscreen);
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				if (!vkTexture->surfaceImage)
					continue;

				usage |= texture->usage | dsTextureUsage_CopyFrom;
				format = texture->info.format;
				uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
				firstLayer = surface->layer*faceCount + surface->cubeFace;
				multisampleImage = vkTexture->surfaceImage;
				finalImage = vkTexture->deviceImage;
				finalLayout = dsVkTexture_imageLayout(texture);
				break;
			}
			default:
				continue;
		}

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(format);
		VkImageResolve imageResolve =
		{
			{aspectMask, 0, 0, 1},
			{0, 0, 0},
			{aspectMask, surface->mipLevel, firstLayer, 1},
			{0, 0, 0},
			{framebuffer->width, framebuffer->height, 1}
		};
		DS_VK_CALL(device->vkCmdResolveImage)(vkCommandBuffer, multisampleImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, finalImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &imageResolve);

		bool isDepthStencil = dsGfxFormat_isDepthStencil(aspectMask);
		VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!imageBarrier)
			return false;

		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		if (isDepthStencil)
		{
			imageBarrier->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			imageBarrier->newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			imageBarrier->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageBarrier->newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		imageBarrier->image = multisampleImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
		imageBarrier->subresourceRange.levelCount = 1;
		imageBarrier->subresourceRange.baseArrayLayer = 0;
		imageBarrier->subresourceRange.layerCount = framebuffer->layers;

		imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!imageBarrier)
			return false;

		imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier->pNext = NULL;
		imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier->dstAccessMask = dsVkReadImageStageFlags(usage, isDepthStencil),
			dsVkWriteImageStageFlags(usage, true, isDepthStencil);
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier->newLayout = finalLayout;
		imageBarrier->image = finalImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
		imageBarrier->subresourceRange.levelCount = 1;
		imageBarrier->subresourceRange.baseArrayLayer = firstLayer;
		imageBarrier->subresourceRange.layerCount = framebuffer->layers;
	}

	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		dstStages);
}

dsVkRenderPassData* dsVkRenderPassData_create(dsAllocator* allocator, dsVkDevice* device,
	const dsRenderPass* renderPass)
{
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;
	dsVkInstance* instance = &device->instance;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkRenderPassData)) +
		DS_ALIGNED_SIZE(sizeof(bool)*renderPass->attachmentCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVkRenderPassData* renderPassData = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkRenderPassData);
	DS_ASSERT(renderPassData);

	memset(renderPassData, 0, sizeof(*renderPassData));
	DS_ASSERT(allocator->freeFunc);
	renderPassData->allocator = allocator;
	dsVkResource_initialize(&renderPassData->resource);
	renderPassData->device = device;
	renderPassData->renderPass = renderPass;
	DS_VERIFY(dsSpinlock_initialize(&renderPassData->shaderLock));
	DS_VERIFY(dsSpinlock_initialize(&renderPassData->framebufferLock));

	if (renderPass->attachmentCount > 0)
	{
		renderPassData->resolveAttachment = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, bool,
			renderPass->attachmentCount);
		DS_ASSERT(renderPassData->resolveAttachment);
		renderPassData->resolveAttachmentCount = renderPass->attachmentCount;

		for (uint32_t i = 0; i < renderPass->attachmentCount; ++i)
		{
			renderPassData->resolveAttachment[i] =
				(renderPass->attachments[i].usage & dsAttachmentUsage_Resolve) != 0;
		}
	}

	renderPassData->lifetime = dsLifetime_create(allocator, renderPassData);
	if (!renderPassData->lifetime)
	{
		dsVkRenderPassData_destroy(renderPassData);
		return NULL;
	}

	VkRenderPassCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		vkRenderPass->fullAttachmentCount, vkRenderPass->vkAttachments,
		renderPass->subpassCount, vkRenderPass->vkSubpasses,
		renderPass->subpassDependencyCount, vkRenderPass->vkDependencies
	};

	VkResult result = DS_VK_CALL(device->vkCreateRenderPass)(device->device, &createInfo,
		instance->allocCallbacksPtr, &renderPassData->vkRenderPass);
	if (!dsHandleVkResult(result))
	{
		dsVkRenderPassData_destroy(renderPassData);
		return NULL;
	}

	return renderPassData;
}

bool dsVkRenderPassData_begin(const dsVkRenderPassData* renderPass,
	dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	dsVkRealFramebuffer* realFramebuffer = dsVkFramebuffer_getRealFramebuffer(
		(dsFramebuffer*)framebuffer, renderPass);
	if (!realFramebuffer)
		return false;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkRect2D renderArea;
	dsVector2f depthRange;
	if (viewport)
	{
		renderArea.offset.x = (int32_t)floorf((float)framebuffer->width*viewport->min.x);
		renderArea.offset.y = (int32_t)floorf((float)framebuffer->height*viewport->min.y);
		renderArea.extent.width = (uint32_t)ceilf((float)framebuffer->width*
			(viewport->max.x - viewport->min.x));
		renderArea.extent.height = (uint32_t)ceilf((float)framebuffer->height*
			(viewport->max.y - viewport->min.y));
		depthRange.x = viewport->min.x;
		depthRange.y = viewport->max.x;
	}
	else
	{
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		renderArea.extent.width = framebuffer->width;
		renderArea.extent.height = framebuffer->height;
		depthRange.x = 0.0f;
		depthRange.y = 1.0f;
	}

	// Same memory layout for dsSurfaceClearValue and VkClearValue
	return dsVkCommandBuffer_beginRenderPass(commandBuffer, renderPass->vkRenderPass,
		dsVkRealFramebuffer_getFramebuffer(realFramebuffer), &renderArea, &depthRange,
		(const VkClearValue*)clearValues, clearValueCount);
}

bool dsVkRenderPassData_nextSubpass(const dsVkRenderPassData* renderPass,
	dsCommandBuffer* commandBuffer, uint32_t index)
{
	DS_UNUSED(renderPass);
	DS_UNUSED(index);
	return dsVkCommandBuffer_nextSubpass(commandBuffer);
}

bool dsVkRenderPassData_end(const dsVkRenderPassData* renderPass, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer->boundFramebuffer);
	const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
	DS_ASSERT(framebuffer);

	// Submit resource barriers first so they get cleared before the framebuffer barriers are
	// processed.
	if (!submitResourceBarriers(commandBuffer) ||
		!beginFramebuffer(commandBuffer, framebuffer))
	{
		return false;
	}

	dsVkCommandBuffer_endRenderPass(commandBuffer);
	if (!endFramebuffer(commandBuffer, commandBuffer->boundFramebuffer,
		renderPass->resolveAttachment))
	{
		return false;
	}

	// Handle if a fence was set during the render pass.
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)dsVkCommandBuffer_get(commandBuffer);
	if (vkCommandBuffer->fenceSet)
		dsVkCommandBuffer_submitFence(commandBuffer, false);
	return true;
}

bool dsVkRenderPassData_addShader(dsVkRenderPassData* renderPass, dsShader* shader)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&renderPass->shaderLock));

	for (uint32_t i = 0; i < renderPass->usedShaderCount; ++i)
	{
		void* usedShader = dsLifetime_getObject(renderPass->usedShaders[i]);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(dsSpinlock_unlock(&renderPass->shaderLock));
			return true;
		}
	}

	uint32_t index = renderPass->usedShaderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderPass->allocator, renderPass->usedShaders,
		renderPass->usedShaderCount, renderPass->maxUsedShaders, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&renderPass->shaderLock));
		return false;
	}

	renderPass->usedShaders[index] = dsLifetime_addRef(vkShader->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&renderPass->shaderLock));
	return true;
}

void dsVkRenderPassData_removeShader(dsVkRenderPassData* renderPass, dsShader* shader)
{
	DS_VERIFY(dsSpinlock_lock(&renderPass->shaderLock));
	for (uint32_t i = 0; i < renderPass->usedShaderCount; ++i)
	{
		dsLifetime* shaderLifetime = renderPass->usedShaders[i];
		void* usedShader = dsLifetime_getObject(shaderLifetime);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(renderPass->usedShaders,
				renderPass->usedShaderCount, i, 1));
			dsLifetime_freeRef(shaderLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&renderPass->shaderLock));
}

bool dsVkRenderPassData_addFramebuffer(dsVkRenderPassData* renderPass, dsFramebuffer* framebuffer)
{
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	DS_VERIFY(dsSpinlock_lock(&renderPass->framebufferLock));

	for (uint32_t i = 0; i < renderPass->usedFramebufferCount; ++i)
	{
		void* usedFramebuffer = dsLifetime_getObject(renderPass->usedFramebuffers[i]);
		DS_ASSERT(usedFramebuffer);
		if (usedFramebuffer == framebuffer)
		{
			DS_VERIFY(dsSpinlock_unlock(&renderPass->framebufferLock));
			return true;
		}
	}

	uint32_t index = renderPass->usedFramebufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderPass->allocator, renderPass->usedFramebuffers,
		renderPass->usedFramebufferCount, renderPass->maxUsedFramebuffers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&renderPass->framebufferLock));
		return false;
	}

	renderPass->usedFramebuffers[index] = dsLifetime_addRef(vkFramebuffer->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&renderPass->framebufferLock));
	return true;
}

void dsVkRenderPassData_removeFramebuffer(dsVkRenderPassData* renderPass,
	dsFramebuffer* framebuffer)
{
	DS_VERIFY(dsSpinlock_lock(&renderPass->framebufferLock));
	for (uint32_t i = 0; i < renderPass->usedFramebufferCount; ++i)
	{
		dsLifetime* framebufferLifetime = renderPass->usedFramebuffers[i];
		void* usedFramebuffer = dsLifetime_getObject(framebufferLifetime);
		DS_ASSERT(usedFramebuffer);
		if (usedFramebuffer == framebuffer)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(renderPass->usedFramebuffers,
				renderPass->usedFramebufferCount, i, 1));
			dsLifetime_freeRef(framebufferLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&renderPass->framebufferLock));
}

void dsVkRenderPassData_destroy(dsVkRenderPassData* renderPass)
{
	if (!renderPass)
		return;

	dsVkDevice* device = renderPass->device;
	dsVkInstance* instance = &device->instance;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&renderPass->shaderLock));
	dsLifetime** usedShaders = renderPass->usedShaders;
	uint32_t usedShaderCount = renderPass->usedShaderCount;
	renderPass->usedShaders = NULL;
	renderPass->usedShaderCount = 0;
	renderPass->maxUsedShaders = 0;
	DS_VERIFY(dsSpinlock_unlock(&renderPass->shaderLock));

	DS_VERIFY(dsSpinlock_lock(&renderPass->framebufferLock));
	dsLifetime** usedFramebuffers = renderPass->usedFramebuffers;
	uint32_t usedFramebufferCount = renderPass->usedFramebufferCount;
	renderPass->usedFramebuffers = NULL;
	renderPass->usedFramebufferCount = 0;
	renderPass->maxUsedFramebuffers = 0;
	DS_VERIFY(dsSpinlock_unlock(&renderPass->framebufferLock));

	for (uint32_t i = 0; i < usedShaderCount; ++i)
	{
		dsShader* shader = (dsShader*)dsLifetime_acquire(usedShaders[i]);
		if (shader)
		{
			dsVkShader_removeRenderPass(shader, renderPass);
			dsLifetime_release(usedShaders[i]);
		}
		dsLifetime_freeRef(usedShaders[i]);
	}
	DS_VERIFY(dsAllocator_free(renderPass->allocator, usedShaders));
	DS_ASSERT(!renderPass->usedShaders);

	for (uint32_t i = 0; i < usedFramebufferCount; ++i)
	{
		dsFramebuffer* framebuffer = (dsFramebuffer*)dsLifetime_acquire(usedFramebuffers[i]);
		if (framebuffer)
		{
			dsVkFramebuffer_removeRenderPass(framebuffer, renderPass);
			dsLifetime_release(usedFramebuffers[i]);
		}
		dsLifetime_freeRef(usedFramebuffers[i]);
	}
	DS_VERIFY(dsAllocator_free(renderPass->allocator, usedFramebuffers));
	DS_ASSERT(!renderPass->usedFramebuffers);

	dsLifetime_destroy(renderPass->lifetime);

	if (renderPass->vkRenderPass)
	{
		DS_VK_CALL(device->vkDestroyRenderPass)(device->device, renderPass->vkRenderPass,
			instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&renderPass->resource);
	dsSpinlock_shutdown(&renderPass->shaderLock);
	dsSpinlock_shutdown(&renderPass->framebufferLock);
	DS_VERIFY(dsAllocator_free(renderPass->allocator, renderPass));
}
