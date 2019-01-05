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

#include "VkRenderPass.h"

#include "Resources/VkFramebuffer.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "Resources/VkShader.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Color.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool needsResolve(const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsAttachmentInfo* attachments, uint32_t attachment)
{
	if (attachments[attachment].samples == 1)
		return false;

	if (attachments[attachment].usage & dsAttachmentUsage_Resolve)
		return true;

	// Check to see if this will be resolved.
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		const dsRenderSubpassInfo* subpass = subpasses + i;
		for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
		{
			if (subpass->colorAttachments[j].attachmentIndex == attachment &&
				subpass->colorAttachments[j].resolve)
			{
				return true;
			}
		}
	}
	return false;
}

static void addPreserveAttachment(uint32_t* outCount, uint32_t* outAttachments, uint32_t attachment,
	uint32_t attachmentCount, const dsRenderSubpassInfo* subpass)
{
	for (uint32_t i = 0; i < subpass->inputAttachmentCount; ++i)
	{
		if (subpass->inputAttachments[i] == attachment)
			return;
	}

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		if (subpass->colorAttachments[i].attachmentIndex == attachment)
			return;
	}

	if (subpass->depthStencilAttachment == attachment)
		return;

	DS_UNUSED(attachmentCount);
	for (uint32_t i = 0; i < *outCount; ++i)
	{
		if (outAttachments[i] == attachment)
			return;
	}

	DS_ASSERT(*outCount < attachmentCount);
	outAttachments[(*outCount)++] = attachment;
}

static void findPreserveAttachments(uint32_t* outCount, uint32_t* outAttachments,
	uint32_t attachmentCount, const dsRenderSubpassInfo* subpasses,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curSubpass,
	uint32_t curDependency)
{
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const dsSubpassDependency* dependency = dependencies + i;
		if (dependency->dstSubpass != curDependency ||
			dependency->srcSubpass == curSubpass)
			continue;

		const dsRenderSubpassInfo* depSubpass = subpasses + dependency->srcSubpass;
		for (uint32_t j = 0; j < depSubpass->colorAttachmentCount; ++j)
		{
			uint32_t curAttachment = depSubpass->colorAttachments[j].attachmentIndex;
			if (depSubpass->colorAttachments[j].attachmentIndex == DS_NO_ATTACHMENT)
				continue;

			addPreserveAttachment(outCount, outAttachments, curAttachment, attachmentCount,
				subpasses + curSubpass);
		}

		if (depSubpass->depthStencilAttachment == DS_NO_ATTACHMENT)
		{
			addPreserveAttachment(outCount, outAttachments, depSubpass->depthStencilAttachment,
				attachmentCount, subpasses + curSubpass);
		}

		findPreserveAttachments(outCount, outAttachments, attachmentCount, subpasses, dependencies,
			dependencyCount, curSubpass, dependency->srcSubpass);
	}
}

static VkPipelineStageFlags getPipelineStages(dsSubpassDependencyStage stage)
{
	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	if (stage == dsSubpassDependencyStage_Vertex)
	{
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	return flags;
}

static VkAccessFlags getSrcAccessFlags(dsSubpassDependencyStage stage)
{
	VkAccessFlags flags = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	if (stage == dsSubpassDependencyStage_Vertex)
	{
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
			VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}
	return flags;
}

static VkAccessFlags getDstAccessFlags(dsSubpassDependencyStage stage)
{
	DS_UNUSED(stage);
	return VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
}

static bool submitResourceBarriers(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
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
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, dstStages);
}

static bool beginFramebuffer(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer)
{
	bool hasImages = false;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		if (surface->surfaceType != dsGfxSurfaceType_Texture)
			continue;

		dsTexture* texture = (dsTexture*)surface->surface;
		DS_ASSERT(texture->offscreen);
		dsVkRenderer_processTexture(renderer, texture);

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
			VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
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

static void setEndImageBarrier(dsTexture* texture, VkImageMemoryBarrier* imageBarrier,
	const dsFramebuffer* framebuffer, const dsFramebufferSurface* surface, VkImage image,
	VkImageLayout layout)
{
	VkImageAspectFlags aspectMask = dsVkImageAspectFlags(texture->info.format);
	bool isDepthStencil = dsGfxFormat_isDepthStencil(texture->info.format);
	imageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier->pNext = NULL;
	imageBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
		VK_ACCESS_TRANSFER_WRITE_BIT;
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

	uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	imageBarrier->image = image;
	imageBarrier->subresourceRange.aspectMask = aspectMask;
	imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
	imageBarrier->subresourceRange.levelCount = 1;
	imageBarrier->subresourceRange.baseArrayLayer = surface->layer*faceCount +
		surface->cubeFace;
	imageBarrier->subresourceRange.layerCount = framebuffer->layers;
}

static bool endFramebuffer(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsAttachmentInfo* attachments)
{
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces + i;
		if (surface->surfaceType != dsGfxSurfaceType_Texture)
			continue;

		dsTexture* texture = (dsTexture*)surface->surface;
		DS_ASSERT(texture->offscreen);
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		// Multisampled depth/stencil images weren't transitioned.
		if (!vkTexture->surfaceImage || !dsGfxFormat_isDepthStencil(texture->info.format))
		{
			VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
			if (!imageBarrier)
				return false;

			setEndImageBarrier(texture, imageBarrier, framebuffer, surface, vkTexture->deviceImage,
				VK_IMAGE_LAYOUT_GENERAL);
		}

		// Manually resolved images.
		if (vkTexture->surfaceImage && (attachments[i].usage & dsAttachmentUsage_Resolve))
		{
			VkImageMemoryBarrier* imageBarrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
			if (!imageBarrier)
				return false;

			setEndImageBarrier(texture, imageBarrier, framebuffer, surface,
				vkTexture->surfaceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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
		if (surface->surfaceType != dsGfxSurfaceType_Texture &&
			!(attachments[i].usage & dsAttachmentUsage_Resolve))
		{
			continue;
		}

		dsTexture* texture = (dsTexture*)surface->surface;
		DS_ASSERT(texture->offscreen);
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		if (!vkTexture->surfaceImage)
			continue;

		uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(texture->info.format);
		VkImageResolve imageResolve =
		{
			{aspectMask, 0, 0, 1},
			{0, 0, 0},
			{aspectMask, surface->mipLevel, surface->layer*faceCount + surface->cubeFace, 1},
			{0, 0, 0},
			{framebuffer->width, framebuffer->height, 1}
		};
		DS_VK_CALL(device->vkCmdResolveImage)(vkCommandBuffer, vkTexture->surfaceImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkTexture->deviceImage, VK_IMAGE_LAYOUT_GENERAL,
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

		imageBarrier->image = vkTexture->surfaceImage;
		imageBarrier->subresourceRange.aspectMask = aspectMask;
		imageBarrier->subresourceRange.baseMipLevel = surface->mipLevel;
		imageBarrier->subresourceRange.levelCount = 1;
		imageBarrier->subresourceRange.baseArrayLayer = surface->layer*faceCount +
			surface->cubeFace;
		imageBarrier->subresourceRange.layerCount = framebuffer->layers;
	}

	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		dstStages);
}

dsRenderPass* dsVkRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	uint32_t fullAttachmentCount = attachmentCount;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		if (needsResolve(subpasses, subpassCount, attachments, i))
			++fullAttachmentCount;
	}

	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = 0;
	else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		finalDependencyCount = subpassCount;
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsAttachmentInfo)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(dsRenderSubpassInfo)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(dsSubpassDependency)*finalDependencyCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		totalSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsColorAttachmentRef)*subpasses[i].colorAttachmentCount);
	}
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, totalSize));
	dsVkRenderPass* renderPass = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator, dsVkRenderPass);
	DS_ASSERT(renderPass);

	renderPass->lifetime = dsLifetime_create(allocator, renderPass);
	if (!renderPass->lifetime)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, renderPass));
		return NULL;
	}

	renderPass->scratchAllocator = renderer->allocator;
	dsVkResource_initialize(&renderPass->resource);
	renderPass->fullAttachmentCount = fullAttachmentCount;
	renderPass->vkRenderPass = 0;

	renderPass->usedShaders = NULL;
	renderPass->usedShaderCount = 0;
	renderPass->maxUsedShaders = 0;
	DS_VERIFY(dsSpinlock_initialize(&renderPass->shaderLock));

	dsRenderPass* baseRenderPass = (dsRenderPass*)renderPass;
	baseRenderPass->renderer = renderer;
	baseRenderPass->allocator = dsAllocator_keepPointer(allocator);

	VkAttachmentDescription* vkAttachments = NULL;
	uint32_t* resolveIndices = NULL;
	if (attachmentCount > 0)
	{
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsAttachmentInfo, attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments,
			sizeof(dsAttachmentInfo)*attachmentCount);

		uint32_t resolveIndex = attachmentCount;
		vkAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkAttachmentDescription,
			fullAttachmentCount);
		resolveIndices = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, fullAttachmentCount);
		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			const dsAttachmentInfo* attachment = attachments + i;
			VkAttachmentDescription* vkAttachment = vkAttachments + i;
			dsAttachmentUsage usage = attachment->usage;

			bool resolve = needsResolve(subpasses, subpassCount, attachments, i);
			if (resolve)
			{
				DS_ASSERT(resolveIndex < fullAttachmentCount);
				resolveIndices[i] = resolveIndex;
			}
			else
				resolveIndices[i] = DS_NO_ATTACHMENT;

			const dsVkFormatInfo* format = dsVkResourceManager_getFormat(renderer->resourceManager,
				attachment->format);
			if (!format)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
				dsVkRenderPass_destroy(renderer, baseRenderPass);
				return NULL;
			}

			vkAttachment->flags = 0;
			vkAttachment->format = format->vkFormat;
			vkAttachment->samples = dsVkSampleCount(attachment->format);

			if (usage & dsAttachmentUsage_Clear)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if (usage & dsAttachmentUsage_KeepBefore)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachment->stencilLoadOp = vkAttachment->loadOp;

			if ((usage & dsAttachmentUsage_KeepAfter) && !resolve)
				vkAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			else
				vkAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			vkAttachment->stencilStoreOp = vkAttachment->storeOp;

			VkImageLayout layout;
			if (dsGfxFormat_isDepthStencil(attachment->format))
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else
				layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			vkAttachment->initialLayout = layout;
			vkAttachment->finalLayout = layout;

			if (resolve)
			{
				VkAttachmentDescription* resolveAttachment = vkAttachments + resolveIndex;
				++resolveIndex;

				*resolveAttachment = *vkAttachment;
				resolveAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
				resolveAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				if (usage & dsAttachmentUsage_KeepAfter)
					resolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			}
		}

		DS_ASSERT(resolveIndex == fullAttachmentCount);
	}
	else
		baseRenderPass->attachments = NULL;
	baseRenderPass->attachmentCount = attachmentCount;

	VkSubpassDependency* vkDependencies = NULL;
	if (finalDependencyCount > 0)
	{
		baseRenderPass->subpassDependencies = DS_ALLOCATE_OBJECT_ARRAY(
			(dsAllocator*)&bufferAllocator, dsSubpassDependency, finalDependencyCount);
		if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		{
			for (uint32_t i = 0; i < subpassCount; ++i)
			{
				dsSubpassDependency* dependency =
					(dsSubpassDependency*)(baseRenderPass->subpassDependencies + i);
				dependency->srcSubpass = i == 0 ? DS_EXTERNAL_SUBPASS : i - 1;
				dependency->srcStage = dsSubpassDependencyStage_Fragment;
				dependency->dstSubpass = i;
				dependency->dstStage = dsSubpassDependencyStage_Fragment;
				dependency->regionDependency = i > 0;
			}
		}
		else
		{
			DS_ASSERT(baseRenderPass->subpassDependencies);
			memcpy((void*)baseRenderPass->subpassDependencies, dependencies,
				sizeof(dsSubpassDependency)*dependencyCount);
		}

		vkDependencies = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSubpassDependency, finalDependencyCount);
		for (uint32_t i = 0; i < finalDependencyCount; ++i)
		{
			const dsSubpassDependency* curDependency = baseRenderPass->subpassDependencies + i;
			VkSubpassDependency* vkDependency = vkDependencies + i;
			vkDependency->srcSubpass = curDependency->srcSubpass;
			vkDependency->dstSubpass = curDependency->dstSubpass;
			vkDependency->srcStageMask = getPipelineStages(curDependency->srcStage);
			vkDependency->dstStageMask = getPipelineStages(curDependency->dstStage);
			vkDependency->srcAccessMask = getSrcAccessFlags(curDependency->srcStage);
			vkDependency->dstAccessMask = getDstAccessFlags(curDependency->srcStage);
			if (curDependency->regionDependency)
				vkDependency->dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			else
				vkDependency->dependencyFlags = 0;
		}

	}
	else
		baseRenderPass->subpassDependencies = NULL;
	baseRenderPass->subpassDependencyCount = finalDependencyCount;

	baseRenderPass->subpasses = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
		dsRenderSubpassInfo, subpassCount);
	DS_ASSERT(baseRenderPass->subpasses);
	memcpy((void*)baseRenderPass->subpasses, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);
	VkSubpassDescription* vkSubpasses = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSubpassDescription,
		subpassCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;
		VkSubpassDescription* vkSubpass = vkSubpasses + i;

		vkSubpass->flags = 0;
		vkSubpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vkSubpass->inputAttachmentCount = curSubpass->inputAttachmentCount;
		vkSubpass->pInputAttachments = NULL;
		vkSubpass->colorAttachmentCount = curSubpass->colorAttachmentCount;
		vkSubpass->pColorAttachments = NULL;
		vkSubpass->pResolveAttachments = NULL;
		vkSubpass->pDepthStencilAttachment = NULL;
		vkSubpass->preserveAttachmentCount = 0;
		vkSubpass->pPreserveAttachments = NULL;

		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
				uint32_t, curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);

			VkAttachmentReference* inputAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference, curSubpass->inputAttachmentCount);
			for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
			{
				uint32_t attachment = curSubpass->inputAttachments[j];
				inputAttachments[j].attachment = attachment;
				if (attachment == DS_NO_ATTACHMENT)
					inputAttachments[j].layout = VK_IMAGE_LAYOUT_GENERAL;
				else if (dsGfxFormat_isDepthStencil(attachments[attachment].format))
					inputAttachments[j].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				else
					inputAttachments[j].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			vkSubpass->pInputAttachments = inputAttachments;
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
				dsColorAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);

			uint32_t resolveAttachmentCount = 0;
			for (uint32_t i = 0; i < curSubpass->colorAttachmentCount; ++i)
			{
				const dsColorAttachmentRef* curAttachment = curSubpass->colorAttachments + i;
				uint32_t attachmentIndex = curAttachment->attachmentIndex;
				if (attachmentIndex == DS_NO_ATTACHMENT || !curAttachment->resolve)
					continue;

				if (resolveIndices[attachmentIndex] != DS_NO_ATTACHMENT)
					++resolveAttachmentCount;
			}

			VkAttachmentReference* colorAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference, curSubpass->colorAttachmentCount);
			VkAttachmentReference* resolveAttachments = NULL;
			if (resolveAttachmentCount > 0)
			{
				resolveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkAttachmentReference,
					curSubpass->colorAttachmentCount);
			}

			for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
			{
				const dsColorAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
				colorAttachments[j].attachment = curAttachment->attachmentIndex;
				colorAttachments[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if (!resolveAttachments)
					continue;

				uint32_t attachmentIndex = curAttachment->attachmentIndex;
				if (attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve)
					resolveAttachments[j].attachment = resolveIndices[attachmentIndex];
				else
					resolveAttachments[j].attachment = VK_ATTACHMENT_UNUSED;
				resolveAttachments[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			vkSubpass->pColorAttachments = colorAttachments;
			vkSubpass->pResolveAttachments = resolveAttachments;
		}

		if (curSubpass->depthStencilAttachment != DS_NO_ATTACHMENT)
		{
			VkAttachmentReference* depthSubpass = DS_ALLOCATE_STACK_OBJECT(VkAttachmentReference);
			depthSubpass->attachment = curSubpass->depthStencilAttachment;
			depthSubpass->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vkSubpass->pDepthStencilAttachment = depthSubpass;
		}

		uint32_t* preserveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, attachmentCount);
		findPreserveAttachments(&vkSubpass->preserveAttachmentCount, preserveAttachments,
			attachmentCount, subpasses, baseRenderPass->subpassDependencies,
			baseRenderPass->subpassDependencyCount, i, i);
	}
	baseRenderPass->subpassCount = subpassCount;

	VkRenderPassCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		attachmentCount, vkAttachments,
		subpassCount, vkSubpasses,
		finalDependencyCount, vkDependencies
	};

	VkResult result = DS_VK_CALL(device->vkCreateRenderPass)(device->device, &createInfo,
		instance->allocCallbacksPtr, &renderPass->vkRenderPass);
	if (!dsHandleVkResult(result))
	{
		dsVkRenderPass_destroy(renderer, baseRenderPass);
		return NULL;
	}

	return baseRenderPass;
}

bool dsVkRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	DS_UNUSED(renderer);
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;

	if (!submitResourceBarriers(renderer, commandBuffer) ||
		!beginFramebuffer(renderer, commandBuffer, framebuffer))
	{
		return false;
	}

	dsVkRealFramebuffer* realFramebuffer = dsVkFramebuffer_getRealFramebuffer(
		(dsFramebuffer*)framebuffer, vkRenderPass->vkRenderPass, true);
	if (!realFramebuffer)
		return false;

	if (vkRenderPass->fullAttachmentCount != realFramebuffer->imageCount)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
			"Resolved surfaces doesn't match between render pass and framebuffer.");
		return false;
	}

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkRect2D renderArea;
	if (viewport)
	{
		renderArea.offset.x = (int32_t)floorf((float)framebuffer->width*viewport->min.x);
		renderArea.offset.y = (int32_t)floorf((float)framebuffer->height*viewport->min.y);
		renderArea.extent.width = (uint32_t)ceilf((float)framebuffer->width*
			(viewport->max.x - viewport->min.x));
		renderArea.extent.height = (uint32_t)ceilf((float)framebuffer->height*
			(viewport->max.y - viewport->min.y));
	}
	else
	{
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		renderArea.extent.width = framebuffer->width;
		renderArea.extent.height = framebuffer->height;
	}

	// Same memory layout for dsSurfaceClearValue and VkClearValue
	return dsVkCommandBuffer_beginRenderPass(commandBuffer, vkRenderPass->vkRenderPass,
		realFramebuffer->framebuffer, &renderArea, (const VkClearValue*)clearValues,
		clearValueCount);
}

bool dsVkRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index)
{
	DS_UNUSED(renderer);
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;

	const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
	DS_ASSERT(framebuffer);

	dsVkRealFramebuffer* realFramebuffer = dsVkFramebuffer_getRealFramebuffer(
		(dsFramebuffer*)framebuffer, vkRenderPass->vkRenderPass, false);
	if (!realFramebuffer)
		return false;

	return dsVkCommandBuffer_nextSubpass(commandBuffer, vkRenderPass->vkRenderPass,
		index, realFramebuffer->framebuffer);
}

bool dsVkRenderPass_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	DS_UNUSED(renderPass);
	DS_ASSERT(commandBuffer->boundFramebuffer);

	dsVkCommandBuffer_endRenderPass(commandBuffer);
	return endFramebuffer(renderer, commandBuffer, commandBuffer->boundFramebuffer,
		renderPass->attachments);
}

bool dsVkRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->shaderLock));
	dsLifetime** usedShaders = vkRenderPass->usedShaders;
	uint32_t usedShaderCount = vkRenderPass->usedShaderCount;
	vkRenderPass->usedShaders = NULL;
	vkRenderPass->usedShaderCount = 0;
	vkRenderPass->maxUsedShaders = 0;
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));

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
	DS_VERIFY(dsAllocator_free(vkRenderPass->scratchAllocator, usedShaders));
	DS_ASSERT(!vkRenderPass->usedShaders);

	dsVkResource_shutdown(&vkRenderPass->resource);
	dsSpinlock_shutdown(&vkRenderPass->shaderLock);

	if (vkRenderPass->vkRenderPass)
	{
		DS_VK_CALL(device->vkDestroyRenderPass)(device->device, vkRenderPass->vkRenderPass,
			instance->allocCallbacksPtr);
	}

	if (renderPass->allocator)
		DS_VERIFY(dsAllocator_free(renderPass->allocator, renderPass));
	return true;
}

bool dsVkRenderPass_addShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->shaderLock));

	for (uint32_t i = 0; i < vkRenderPass->usedShaderCount; ++i)
	{
		void* usedShader = dsLifetime_getObject(vkRenderPass->usedShaders[i]);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
			return true;
		}
	}

	uint32_t index = vkRenderPass->usedShaderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(vkRenderPass->scratchAllocator, vkRenderPass->usedShaders,
		vkRenderPass->usedShaderCount, vkRenderPass->maxUsedShaders, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
		return false;
	}

	vkRenderPass->usedShaders[index] = dsLifetime_addRef(vkShader->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
	return true;
}

void dsVkRenderPass_removeShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->shaderLock));
	for (uint32_t i = 0; i < vkRenderPass->usedShaderCount; ++i)
	{
		void* usedShader = dsLifetime_getObject(vkRenderPass->usedShaders[i]);
		DS_ASSERT(usedShader);
		if (usedShader == shader)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(vkRenderPass->usedShaders,
				vkRenderPass->usedShaderCount, i, 1));
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->shaderLock));
}
