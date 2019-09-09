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
#include "VkRenderPassCreation.h"
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

static bool beginFramebuffer(dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer)
{
	// Move framebuffer images into the "optimal" layouts.
	dsRenderer* renderer = commandBuffer->renderer;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		if (surface->surfaceType != dsGfxSurfaceType_Offscreen)
			continue;

		dsTexture* texture = (dsTexture*)surface->surface;
		DS_ASSERT(texture->offscreen);
		dsVkRenderer_processTexture(renderer, texture);
		if (dsVkTexture_canReadBack(texture) &&
			!dsVkCommandBuffer_addReadbackOffscreen(commandBuffer, texture))
		{
			return false;
		}

		if (dsVkTexture_onlySubpassInput(texture->usage))
			continue;

		// Don't layout transition for resolved depth/stencil images, since you can't resolve
		// in render subpasses.
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		if (vkTexture->surfaceImage && dsGfxFormat_isDepthStencil(texture->info.format))
			continue;

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
		imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

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
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (renderer->hasTessellationShaders)
	{
		srcStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		srcStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, srcStages);
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
		imageBarrier->srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageBarrier->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	else
	{
		imageBarrier->srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	imageBarrier->newLayout = layout;
	imageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageBarrier->image = image;
	imageBarrier->subresourceRange.aspectMask = aspectMask;
	imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
	imageBarrier->subresourceRange.levelCount = 1;
	imageBarrier->subresourceRange.baseArrayLayer = baseLayer;
	imageBarrier->subresourceRange.layerCount = framebuffer->layers;
}

static bool endFramebuffer(dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer)
{
	// Move framebuffer images into the expected layouts.
	dsRenderer* renderer = commandBuffer->renderer;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_Offscreen:
			{
				dsTexture* texture = (dsTexture*)surface->surface;
				DS_ASSERT(texture->offscreen);
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				if (!dsVkCommandBuffer_addResource(commandBuffer, &vkTexture->resource))
					return false;

				// Skip textures only used as subpass inputs since they stay in the optimal
				// attachment layout.
				if (dsVkTexture_onlySubpassInput(texture->usage))
					break;

				VkImageMemoryBarrier* imageBarrier =
					dsVkCommandBuffer_addImageBarrier(commandBuffer);
				if (!imageBarrier)
					return false;

				uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
				setEndImageBarrier(imageBarrier, framebuffer, surface, texture->info.format,
					vkTexture->deviceImage, dsVkTexture_imageLayout(texture),
					surface->layer*faceCount + surface->cubeFace);

				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsVkRenderbuffer* renderbuffer = (dsVkRenderbuffer*)surface->surface;
				if (!dsVkCommandBuffer_addResource(commandBuffer, &renderbuffer->resource))
					return false;
				break;
			}
			default:
				break;
		}
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (renderer->hasTessellationShaders)
	{
		srcStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		srcStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, srcStages);
}

dsVkRenderPassData* dsVkRenderPassData_create(dsAllocator* allocator, dsVkDevice* device,
	const dsRenderPass* renderPass)
{
	const dsRenderer* renderer = renderPass->renderer;
	uint32_t attachmentCount = renderPass->attachmentCount;
	uint32_t fullAttachmentCount = attachmentCount;
	uint32_t resolveAttachmentCount = 0;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		// Don't resolve default samples since we need space for the attachment when multisampling
		// is disabled in case it's enabled later.
		if (dsVkAttachmentHasResolve(renderPass->subpasses, renderPass->subpassCount, i,
				renderPass->attachments[i].samples, renderer->surfaceSamples))
		{
			++fullAttachmentCount;
			++resolveAttachmentCount;
		}
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkRenderPassData)) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*attachmentCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVkRenderPassData* renderPassData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkRenderPassData);
	DS_ASSERT(renderPassData);

	memset(renderPassData, 0, sizeof(*renderPassData));
	DS_ASSERT(allocator->freeFunc);
	renderPassData->allocator = allocator;
	dsVkResource_initialize(&renderPassData->resource);
	renderPassData->device = device;
	renderPassData->renderPass = renderPass;
	DS_VERIFY(dsSpinlock_initialize(&renderPassData->shaderLock));
	DS_VERIFY(dsSpinlock_initialize(&renderPassData->framebufferLock));

	if (attachmentCount > 0)
	{
		renderPassData->resolveIndices = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t,
			attachmentCount);
		DS_ASSERT(renderPassData->resolveIndices);

		uint32_t resolveIndex = 0;
		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			const dsAttachmentInfo* attachment = renderPass->attachments + i;
			const dsVkFormatInfo* format = dsVkResourceManager_getFormat(renderer->resourceManager,
				attachment->format);
			if (!format)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
				dsVkRenderPassData_destroy(renderPassData);
				return NULL;
			}

			uint32_t samples = attachment->samples;
			if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
				samples = renderer->surfaceSamples;
			if (dsVkAttachmentHasResolve(renderPass->subpasses, renderPass->subpassCount, i,
					attachment->samples, renderer->surfaceSamples))
			{
				uint32_t resolveAttachmentIndex = attachmentCount + resolveIndex;
				renderPassData->resolveIndices[i] = resolveAttachmentIndex;
				++resolveIndex;
			}
			else
				renderPassData->resolveIndices[i] = DS_NO_ATTACHMENT;
		}

		DS_ASSERT(resolveIndex == resolveAttachmentCount);
	}
	else
		renderPassData->resolveIndices = NULL;
	renderPassData->attachmentCount = attachmentCount;
	renderPassData->fullAttachmentCount = fullAttachmentCount;
	renderPassData->lifetime = dsLifetime_create(allocator, renderPassData);
	if (!renderPassData->lifetime)
	{
		dsVkRenderPassData_destroy(renderPassData);
		return NULL;
	}

	if (!dsCreateUnderlyingVkRenderPass(renderPassData, resolveAttachmentCount))
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
		(dsFramebuffer*)framebuffer, commandBuffer, renderPass);
	if (!realFramebuffer)
		return false;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkRect2D renderArea;
	dsVector2f depthRange;
	if (viewport)
	{
		renderArea.offset.x = (int32_t)floorf(viewport->min.x);
		renderArea.offset.y = (int32_t)floorf(viewport->min.y);
		renderArea.extent.width = (uint32_t)ceilf(viewport->max.x - viewport->min.x);
		renderArea.extent.height = (uint32_t)ceilf(viewport->max.y - viewport->min.y);
		depthRange.x = viewport->min.z;
		depthRange.y = viewport->max.z;
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
	DS_UNUSED(renderPass);
	DS_ASSERT(commandBuffer->boundFramebuffer);
	const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
	DS_ASSERT(framebuffer);

	if (!beginFramebuffer(commandBuffer, framebuffer))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}

	dsVkCommandBuffer_endRenderPass(commandBuffer);
	if (!endFramebuffer(commandBuffer, commandBuffer->boundFramebuffer))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
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
		if (renderPass->usedShaders[i] == vkShader->lifetime)
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
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&renderPass->shaderLock));
	for (uint32_t i = 0; i < renderPass->usedShaderCount; ++i)
	{
		dsLifetime* shaderLifetime = renderPass->usedShaders[i];
		if (shaderLifetime == vkShader->lifetime)
		{
			renderPass->usedShaders[i] = renderPass->usedShaders[renderPass->usedShaderCount - 1];
			--renderPass->usedShaderCount;
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
		if (renderPass->usedFramebuffers[i] == vkFramebuffer->lifetime)
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
	dsVkFramebuffer* vkFramebuffer = (dsVkFramebuffer*)framebuffer;
	DS_VERIFY(dsSpinlock_lock(&renderPass->framebufferLock));
	for (uint32_t i = 0; i < renderPass->usedFramebufferCount; ++i)
	{
		dsLifetime* framebufferLifetime = renderPass->usedFramebuffers[i];
		if (framebufferLifetime == vkFramebuffer->lifetime)
		{
			renderPass->usedFramebuffers[i] =
				renderPass->usedFramebuffers[renderPass->usedFramebufferCount - 1];
			--renderPass->usedFramebufferCount;
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
