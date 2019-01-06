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

#include "Resources/VkResourceManager.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkRenderPassData.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool needsResolve(uint32_t samples, uint32_t defaultSamples)
{
	return (samples == DS_DEFAULT_ANTIALIAS_SAMPLES && defaultSamples > 1) ||
		(samples != DS_DEFAULT_ANTIALIAS_SAMPLES && samples > 1);
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

dsRenderPass* dsVkRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	uint32_t fullAttachmentCount = attachmentCount*2;
	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = 0;
	else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		finalDependencyCount = subpassCount;
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsAttachmentInfo)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(dsRenderSubpassInfo)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(dsSubpassDependency)*finalDependencyCount) +
		DS_ALIGNED_SIZE(sizeof(VkAttachmentDescription)*fullAttachmentCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(VkSubpassDescription)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(VkSubpassDependency)*finalDependencyCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		totalSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsColorAttachmentRef)*subpasses[i].colorAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(VkAttachmentReference)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(VkAttachmentReference)*subpasses[i].colorAttachmentCount*2) +
			DS_ALIGNED_SIZE(sizeof(uint32_t)*fullAttachmentCount);
		if (subpasses[i].depthStencilAttachment != DS_NO_ATTACHMENT)
			totalSize += DS_ALIGNED_SIZE(sizeof(VkAttachmentReference));
	}
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, totalSize));
	dsVkRenderPass* renderPass = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkRenderPass);
	DS_ASSERT(renderPass);

	renderPass->lastCheckedFrame = renderer->frameNumber;
	renderPass->scratchAllocator = renderer->allocator;
	renderPass->defaultSamples = renderer->surfaceSamples;
	renderPass->usesDefaultSamples = false;
	renderPass->renderPassData = NULL;
	DS_VERIFY(dsSpinlock_initialize(&renderPass->lock));

	dsRenderPass* baseRenderPass = (dsRenderPass*)renderPass;
	baseRenderPass->renderer = renderer;
	baseRenderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsAttachmentInfo, attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments,
			sizeof(dsAttachmentInfo)*attachmentCount);

		uint32_t resolveIndex = attachmentCount;
		renderPass->vkAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkAttachmentDescription, fullAttachmentCount);
		DS_ASSERT(renderPass->vkAttachments);
		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			const dsAttachmentInfo* attachment = attachments + i;
			VkAttachmentDescription* vkAttachment = renderPass->vkAttachments + i*2;
			VkAttachmentDescription* vkResolveAttachment = renderPass->vkAttachments + i*2 + 1;
			dsAttachmentUsage usage = attachment->usage;

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
			uint32_t samples = attachment->samples;
			if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			{
				samples = renderer->surfaceSamples;
				renderPass->usesDefaultSamples = true;
			}
			vkAttachment->samples = dsVkSampleCount(samples);

			if (usage & dsAttachmentUsage_Clear)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if (usage & dsAttachmentUsage_KeepBefore)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachment->stencilLoadOp = vkAttachment->loadOp;

			if (usage & dsAttachmentUsage_KeepAfter)
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

			*vkResolveAttachment = *vkAttachment;
			vkResolveAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
			vkResolveAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			if (usage & dsAttachmentUsage_KeepAfter)
				vkResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}

		DS_ASSERT(resolveIndex == fullAttachmentCount);
	}
	else
	{
		baseRenderPass->attachments = NULL;
		renderPass->vkAttachments = NULL;
	}
	baseRenderPass->attachmentCount = attachmentCount;

	if (finalDependencyCount > 0)
	{
		baseRenderPass->subpassDependencies = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsSubpassDependency, finalDependencyCount);
		DS_ASSERT(baseRenderPass->subpassDependencies);
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

		renderPass->vkDependencies = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkSubpassDependency, finalDependencyCount);
		DS_ASSERT(renderPass->vkDependencies);
		for (uint32_t i = 0; i < finalDependencyCount; ++i)
		{
			const dsSubpassDependency* curDependency = baseRenderPass->subpassDependencies + i;
			VkSubpassDependency* vkDependency = renderPass->vkDependencies + i;
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
	{
		baseRenderPass->subpassDependencies = NULL;
		renderPass->vkDependencies = NULL;
	}
	baseRenderPass->subpassDependencyCount = finalDependencyCount;

	baseRenderPass->subpasses = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		dsRenderSubpassInfo, subpassCount);
	DS_ASSERT(baseRenderPass->subpasses);
	memcpy((void*)baseRenderPass->subpasses, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);

	renderPass->vkSubpasses = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		VkSubpassDescription, subpassCount);
	DS_ASSERT(renderPass->vkSubpasses);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;
		VkSubpassDescription* vkSubpass = renderPass->vkSubpasses + i;

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
			curSubpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				uint32_t, curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);

			VkAttachmentReference* inputAttachments = DS_ALLOCATE_OBJECT_ARRAY(
				(dsAllocator*)&bufferAlloc, VkAttachmentReference,
				curSubpass->inputAttachmentCount);
			DS_ASSERT(inputAttachments);
			for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
			{
				uint32_t attachment = curSubpass->inputAttachments[j];
				// Use resolved result. If unresolved surface, the pointer will be duplicated.
				if (attachment == DS_NO_ATTACHMENT)
					inputAttachments[j].attachment = VK_ATTACHMENT_UNUSED;
				else
					inputAttachments[j].attachment = attachment*2 + 1;

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
			curSubpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				dsColorAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);

			VkAttachmentReference* colorAttachments = DS_ALLOCATE_OBJECT_ARRAY(
				(dsAllocator*)&bufferAlloc, VkAttachmentReference,
				curSubpass->colorAttachmentCount*2);
			DS_ASSERT(colorAttachments);
			VkAttachmentReference* resolveAttachments = colorAttachments +
				curSubpass->colorAttachmentCount;
			for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
			{
				const dsColorAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
				if (curAttachment->attachmentIndex == DS_NO_ATTACHMENT)
					colorAttachments[j].attachment = VK_ATTACHMENT_UNUSED;
				else
					colorAttachments[j].attachment = curAttachment->attachmentIndex*2;
				colorAttachments[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if (curAttachment->attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve &&
					needsResolve(attachments[curAttachment->attachmentIndex].samples,
						renderer->surfaceSamples))
				{
					resolveAttachments[j].attachment = curAttachment->attachmentIndex*2 + 1;
				}
				else
					resolveAttachments[j].attachment = VK_ATTACHMENT_UNUSED;
				resolveAttachments[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			vkSubpass->pColorAttachments = colorAttachments;
			vkSubpass->pResolveAttachments = resolveAttachments;
		}

		if (curSubpass->depthStencilAttachment != DS_NO_ATTACHMENT)
		{
			VkAttachmentReference* depthSubpass = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
				VkAttachmentReference);
			depthSubpass->attachment = curSubpass->depthStencilAttachment;
			depthSubpass->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vkSubpass->pDepthStencilAttachment = depthSubpass;
		}

		uint32_t* preserveAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			uint32_t, attachmentCount);
		findPreserveAttachments(&vkSubpass->preserveAttachmentCount, preserveAttachments,
			attachmentCount, subpasses, baseRenderPass->subpassDependencies,
			baseRenderPass->subpassDependencyCount, i, i);
	}
	baseRenderPass->subpassCount = subpassCount;

	renderPass->renderPassData = dsVkRenderPassData_create(renderPass->scratchAllocator,
		device, attachments, attachmentCount, renderPass->vkAttachments, fullAttachmentCount,
		renderPass->vkSubpasses, subpassCount, renderPass->vkDependencies, finalDependencyCount);
	if (!renderPass->renderPassData)
	{
		dsVkRenderPass_destroy(renderer, baseRenderPass);
		return false;
	}

	return baseRenderPass;
}

bool dsVkRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	DS_UNUSED(renderer);
	dsVkRenderPassData* renderPassData = dsVkRenderPass_getData(renderPass);
	return dsVkRenderPassData_begin(renderPassData, commandBuffer, framebuffer, viewport,
		clearValues, clearValueCount);
}

bool dsVkRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index)
{
	DS_UNUSED(renderer);
	// Guaranteed that dsVkRenderPass_getData() was called earlier, and will return the same value.
	const dsVkRenderPassData* renderPassData = ((const dsVkRenderPass*)renderPass)->renderPassData;
	return dsVkRenderPassData_nextSubpass(renderPassData, commandBuffer, index);;
}

bool dsVkRenderPass_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	DS_UNUSED(renderer);
	// Guaranteed that dsVkRenderPass_getData() was called earlier, and will return the same value.
	const dsVkRenderPassData* renderPassData = ((const dsVkRenderPass*)renderPass)->renderPassData;
	return dsVkRenderPassData_end(renderPassData, commandBuffer);
}

bool dsVkRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	dsVkRenderer_deleteRenderPass(renderer, vkRenderPass->renderPassData);
	dsSpinlock_shutdown(&vkRenderPass->lock);
	if (renderPass->allocator)
		DS_VERIFY(dsAllocator_free(renderPass->allocator, renderPass));
	return true;
}

dsVkRenderPassData* dsVkRenderPass_getData(const dsRenderPass* renderPass)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	dsRenderer* renderer = renderPass->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	uint64_t frame = renderer->frameNumber;
	uint32_t samples = renderer->surfaceSamples;

	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->lock));
	if (vkRenderPass->lastCheckedFrame == frame)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->lock));
		return vkRenderPass->renderPassData;
	}

	if (vkRenderPass->usesDefaultSamples && samples != vkRenderPass->defaultSamples)
	{
		// Adjust the sample counts.
		VkSampleCountFlags vkSamples = dsVkSampleCount(samples);
		uint32_t fullAttachmentCount = renderPass->attachmentCount*2;
		for (uint32_t i = 0; i < renderPass->attachmentCount; ++i)
		{
			const dsAttachmentInfo* curAttachment = renderPass->attachments + i;
			if (curAttachment->samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
				vkRenderPass->vkAttachments[i*2].samples = vkSamples;
		}

		// May need to change the resolve attachment to enable/disable resolving.
		for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
		{
			const dsRenderSubpassInfo* curSubpass = renderPass->subpasses + i;
			VkSubpassDescription* vkSubpass = vkRenderPass->vkSubpasses + i;
			for (uint32_t j = 0; j < curSubpass->colorAttachmentCount; ++j)
			{
				const dsColorAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
				if (curAttachment->attachmentIndex == DS_NO_ATTACHMENT || !curAttachment->resolve)
					continue;

				VkAttachmentReference* resolveAttachment =
					(VkAttachmentReference*)(vkSubpass->pResolveAttachments + j);
				if (needsResolve(renderPass->attachments[curAttachment->attachmentIndex].samples,
						samples))
				{
					resolveAttachment->attachment = curAttachment->attachmentIndex*2 + 1;
				}
				else
					resolveAttachment->attachment = VK_ATTACHMENT_UNUSED;
			}
		}

		dsVkRenderPassData* renderPassData = dsVkRenderPassData_create(
			vkRenderPass->scratchAllocator, device, renderPass->attachments,
			renderPass->attachmentCount, vkRenderPass->vkAttachments, fullAttachmentCount,
			vkRenderPass->vkSubpasses, renderPass->subpassCount, vkRenderPass->vkDependencies,
			renderPass->subpassDependencyCount);
		if (renderPassData)
		{
			dsVkRenderer_deleteRenderPass(renderer, vkRenderPass->renderPassData);
			vkRenderPass->renderPassData = renderPassData;
		}
	}

	vkRenderPass->lastCheckedFrame = frame;
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->lock));
	return vkRenderPass->renderPassData;
}
