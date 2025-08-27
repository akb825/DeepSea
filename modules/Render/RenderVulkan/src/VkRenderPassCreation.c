/*
 * Copyright 2019-2025 Aaron Barany
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

#include "VkRenderPassCreation.h"

#include "Resources/VkResourceManager.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

typedef enum AttachmentUsage
{
	AttachmentUsage_WriteBefore = 0x1,
	AttachmentUsage_ReadAfter = 0x2,
	AttachmentUsage_Current = 0x4
} AttachmentUsage;

static bool mustKeepMultisampledAttachment(dsAttachmentUsage usage, uint32_t samples)
{
	return (usage & dsAttachmentUsage_KeepAfter) &&
		(samples == 1 || (usage & dsAttachmentUsage_UseLater));
}

static bool needsResolve(uint32_t samples, uint32_t surfaceSamples, uint32_t defaultSamples)
{
	return (samples == DS_SURFACE_ANTIALIAS_SAMPLES && surfaceSamples > 1) ||
		(samples == DS_DEFAULT_ANTIALIAS_SAMPLES && defaultSamples > 1) ||
		(samples != DS_SURFACE_ANTIALIAS_SAMPLES && samples != DS_DEFAULT_ANTIALIAS_SAMPLES &&
			samples > 1);
}

static void addLegacySubpassAttachmentUsageBits(AttachmentUsage* usages,
	const VkSubpassDescription* subpass, AttachmentUsage usage)
{
	// Don't add input attchments if only writing, since it's a read-only operation.
	if (usage & ~AttachmentUsage_WriteBefore)
	{
		for (uint32_t i = 0; i < subpass->inputAttachmentCount; ++i)
		{
			uint32_t attachment = subpass->pInputAttachments[i].attachment;
			if (attachment != VK_ATTACHMENT_UNUSED)
				usages[attachment] |= usage;
		}
	}

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpass->pColorAttachments[i].attachment;
		if (attachment != VK_ATTACHMENT_UNUSED)
			usages[attachment] |= usage;
	}

	// Don't add resolve attchments if only reading, since it's a write-only operation.
	if (subpass->pResolveAttachments && (usage & ~AttachmentUsage_ReadAfter))
	{
		for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
		{
			uint32_t attachment = subpass->pResolveAttachments[i].attachment;
			if (attachment != VK_ATTACHMENT_UNUSED)
				usages[attachment] |= usage;
		}
	}

	if (subpass->pDepthStencilAttachment &&
		subpass->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
	{
		usages[subpass->pDepthStencilAttachment->attachment] |= usage;
	}
}

static void findLegacyAttachmentsBeforeUses(AttachmentUsage* usages,
	const VkSubpassDescription* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curDepSubpass,
	uint32_t depth)
{
	if (depth >= subpassCount)
		return;

	// Find all dependencies that have a destination dependency for the current dependency to
	// determine which attachments are written to.
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const VkSubpassDependency* dependency = dependencies + i;
		if (dependency->dstSubpass != curDepSubpass ||
			dependency->srcSubpass == VK_SUBPASS_EXTERNAL)
		{
			continue;
		}

		const VkSubpassDescription* depSubpass = subpasses + dependency->srcSubpass;
		addLegacySubpassAttachmentUsageBits(usages, depSubpass, AttachmentUsage_WriteBefore);

		// Recurse for the source dependency.
		findLegacyAttachmentsBeforeUses(usages, subpasses, subpassCount, dependencies,
			dependencyCount, dependency->srcSubpass, depth + 1);
	}
}

static void findLegacyAttachmentsAfterUses(AttachmentUsage* usages,
	const VkSubpassDescription* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curDepSubpass,
	uint32_t depth)
{
	if (depth >= subpassCount)
		return;

	// Find all dependencies that have a source dependency for the current dependency to
	// determine which attachments are read from.
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const VkSubpassDependency* dependency = dependencies + i;
		if (dependency->srcSubpass != curDepSubpass ||
			dependency->dstSubpass == VK_SUBPASS_EXTERNAL)
		{
			continue;
		}

		const VkSubpassDescription* depSubpass = subpasses + dependency->dstSubpass;
		addLegacySubpassAttachmentUsageBits(usages, depSubpass, AttachmentUsage_ReadAfter);

		// Recurse for the destination dependency.
		findLegacyAttachmentsAfterUses(usages, subpasses, subpassCount, dependencies,
			dependencyCount, dependency->dstSubpass, depth + 1);
	}
}

static void findLegacyPreserveAttachments(uint32_t* outAttachments, uint32_t* outCount,
	const VkAttachmentDescription* attachments, uint32_t attachmentCount,
	const VkSubpassDescription* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curSubpass)
{
	AttachmentUsage* usages = DS_ALLOCATE_STACK_OBJECT_ARRAY(AttachmentUsage, attachmentCount);
	memset(usages, 0, sizeof(AttachmentUsage)*attachmentCount);

	// Find the usage flags for the current subpass, before the current subpass (by dependencies),
	// and after the current subpass (by dependencies).
	addLegacySubpassAttachmentUsageBits(usages, subpasses + curSubpass, AttachmentUsage_Current);
	findLegacyAttachmentsBeforeUses(
		usages, subpasses, subpassCount, dependencies, dependencyCount, curSubpass, 0);
	findLegacyAttachmentsAfterUses(
		usages, subpasses, subpassCount, dependencies, dependencyCount, curSubpass, 0);

	*outCount = 0;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		// Add implicit uses based on attachment operations.
		AttachmentUsage usage = usages[i];
		const VkAttachmentDescription* attachment = attachments + i;
		if (attachment->loadOp != VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			usage |= AttachmentUsage_WriteBefore;
		if (attachment->storeOp != VK_ATTACHMENT_STORE_OP_DONT_CARE)
			usage |= AttachmentUsage_ReadAfter;

		// Add attachments that are used before and after, but not during, the current subpass.
		if (usage == (AttachmentUsage_WriteBefore | AttachmentUsage_ReadAfter))
			outAttachments[(*outCount)++] = i;
	}
}

static bool createLegacyRenderPass(dsVkRenderPassData* renderPassData,
	uint32_t resolveAttachmentCount)
{
	DS_UNUSED(resolveAttachmentCount);
	const dsRenderPass* renderPass = renderPassData->renderPass;
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;
	const dsRenderer* renderer = renderPass->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	VkAttachmentDescription* vkAttachments = NULL;
	if (renderPassData->attachmentCount > 0)
	{
		vkAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkAttachmentDescription,
			renderPassData->fullAttachmentCount);

		uint32_t resolveIndex = 0;
		for (uint32_t i = 0; i < renderPassData->attachmentCount; ++i)
		{
			const dsAttachmentInfo* attachment = renderPass->attachments + i;
			VkAttachmentDescription* vkAttachment = vkAttachments + i;
			dsAttachmentUsage usage = attachment->usage;

			const dsVkFormatInfo* format = dsVkResourceManager_getFormat(renderer->resourceManager,
				attachment->format);
			if (!format)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
				return 0;
			}

			vkAttachment->flags = 0;
			vkAttachment->format = format->vkFormat;
			uint32_t samples = attachment->samples;
			if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
				samples = renderer->surfaceSamples;
			else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
				samples = renderer->defaultSamples;

			vkAttachment->samples = dsVkSampleCount(samples);

			if (usage & dsAttachmentUsage_Clear)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if (usage & dsAttachmentUsage_KeepBefore)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachment->stencilLoadOp = vkAttachment->loadOp;

			if (mustKeepMultisampledAttachment(usage, samples))
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

			if (dsVkAttachmentHasResolve(renderPass->subpasses, renderPass->subpassCount, i,
					attachment->samples, renderer->surfaceSamples, renderer->defaultSamples))
			{
				uint32_t resolveAttachmentIndex = renderPassData->attachmentCount + resolveIndex;
				VkAttachmentDescription* vkResolveAttachment = vkAttachments +
					resolveAttachmentIndex;
				*vkResolveAttachment = *vkAttachment;
				vkResolveAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
				vkResolveAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkResolveAttachment->stencilLoadOp = vkResolveAttachment->loadOp;
				if (usage & dsAttachmentUsage_KeepAfter)
					vkResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				else
					vkResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkResolveAttachment->stencilStoreOp = vkResolveAttachment->storeOp;
				++resolveIndex;
			}
		}

		DS_ASSERT(resolveIndex == resolveAttachmentCount);
	}

	VkSubpassDescription* vkSubpasses = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSubpassDescription,
		renderPass->subpassCount);
	for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
	{
		const dsRenderSubpassInfo* curSubpass = renderPass->subpasses + i;
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
			VkAttachmentReference* inputAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference, curSubpass->inputAttachmentCount);
			for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
			{
				uint32_t attachment = curSubpass->inputAttachments[j];
				VkAttachmentReference* inputAttachment = inputAttachments + j;
				if (attachment == DS_NO_ATTACHMENT)
					inputAttachment->attachment = VK_ATTACHMENT_UNUSED;
				else
				{
					// Use resolved result if available.
					uint32_t resolveAttachment = renderPassData->resolveIndices[attachment];
					if (resolveAttachment == DS_NO_ATTACHMENT)
						inputAttachment->attachment = attachment;
					else
						inputAttachment->attachment = resolveAttachment;
				}

				if (attachment == DS_NO_ATTACHMENT)
					inputAttachment->layout = VK_IMAGE_LAYOUT_GENERAL;
				else if (dsGfxFormat_isDepthStencil(renderPass->attachments[attachment].format))
					inputAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				else
					inputAttachment->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			vkSubpass->pInputAttachments = inputAttachments;
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			VkAttachmentReference* colorAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference, curSubpass->colorAttachmentCount);

			bool hasResolve = false;
			for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
			{
				const dsAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
				VkAttachmentReference* colorAttachment = colorAttachments + j;
				uint32_t attachmentIndex = curAttachment->attachmentIndex;
				colorAttachment->attachment = attachmentIndex;
				colorAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if (attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve &&
					needsResolve(renderPass->attachments[curAttachment->attachmentIndex].samples,
						renderer->surfaceSamples, renderer->defaultSamples))
				{
					hasResolve = true;
				}
			}

			vkSubpass->pColorAttachments = colorAttachments;
			if (hasResolve)
			{
				VkAttachmentReference* resolveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
					VkAttachmentReference, curSubpass->colorAttachmentCount);

				for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
				{
					const dsAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
					VkAttachmentReference* resolveAttachment = resolveAttachments + j;
					uint32_t attachmentIndex = curAttachment->attachmentIndex;
					if (attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve &&
						needsResolve(
							renderPass->attachments[curAttachment->attachmentIndex].samples,
							renderer->surfaceSamples, renderer->defaultSamples))
					{
						uint32_t resolveIndex = renderPassData->resolveIndices[attachmentIndex];
						DS_ASSERT(resolveIndex != DS_NO_ATTACHMENT);
						resolveAttachment->attachment = resolveIndex;
					}
					else
						resolveAttachment->attachment = VK_ATTACHMENT_UNUSED;
					resolveAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				vkSubpass->pResolveAttachments = resolveAttachments;
			}
		}

		const dsAttachmentRef* depthStencilAttachment = &curSubpass->depthStencilAttachment;
		if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT)
		{
			// Check if the depth is also used as an input, in which case this can be used for
			// read-only depth checks.
			bool isInput = false;
			for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
			{
				if (vkSubpass->pInputAttachments[j].attachment ==
					depthStencilAttachment->attachmentIndex)
				{
					isInput = true;
					break;
				}
			}

			VkAttachmentReference* depthSubpass = DS_ALLOCATE_STACK_OBJECT(VkAttachmentReference);
			depthSubpass->attachment = depthStencilAttachment->attachmentIndex;
			depthSubpass->layout = isInput ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vkSubpass->pDepthStencilAttachment = depthSubpass;
		}
	}

	// Set up dependencies after all of the subpasses are otherwise set up.
	for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
	{
		VkSubpassDescription* vkSubpass = vkSubpasses + i;
		uint32_t* preserveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t,
			renderPassData->fullAttachmentCount);
		DS_ASSERT(preserveAttachments);
		vkSubpass->pPreserveAttachments = preserveAttachments;
		findLegacyPreserveAttachments(preserveAttachments, &vkSubpass->preserveAttachmentCount,
			vkAttachments, renderPassData->fullAttachmentCount, vkSubpasses,
			renderPass->subpassCount, vkRenderPass->vkDependencies,
			renderPass->subpassDependencyCount, i);
	}

	VkRenderPassCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		renderPassData->fullAttachmentCount, vkAttachments,
		renderPass->subpassCount, vkSubpasses,
		renderPass->subpassDependencyCount, vkRenderPass->vkDependencies
	};

	VkResult result = DS_VK_CALL(device->vkCreateRenderPass)(device->device, &createInfo,
		instance->allocCallbacksPtr, &renderPassData->vkRenderPass);
	return DS_HANDLE_VK_RESULT(result, "Couldn't create render pass");
}

static void addSubpassAttachmentUsageBits(AttachmentUsage* usages,
	const VkSubpassDescription2KHR* subpass, AttachmentUsage usage)
{
	// Don't add input attchments if only writing, since it's a read-only operation.
	if (usage & ~AttachmentUsage_WriteBefore)
	{
		for (uint32_t i = 0; i < subpass->inputAttachmentCount; ++i)
		{
			uint32_t attachment = subpass->pInputAttachments[i].attachment;
			if (attachment != VK_ATTACHMENT_UNUSED)
				usages[attachment] |= usage;
		}
	}

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpass->pColorAttachments[i].attachment;
		if (attachment != VK_ATTACHMENT_UNUSED)
			usages[attachment] |= usage;
	}

	// Don't add resolve attchments if only reading, since it's a write-only operation.
	if (usage & ~AttachmentUsage_ReadAfter)
	{
		if (subpass->pResolveAttachments)
		{
			for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
			{
				uint32_t attachment = subpass->pResolveAttachments[i].attachment;
				if (attachment != VK_ATTACHMENT_UNUSED)
					usages[attachment] |= usage;
			}
		}

		const VkSubpassDescriptionDepthStencilResolveKHR* depthStencilResolve =
			(const VkSubpassDescriptionDepthStencilResolveKHR*)subpass->pNext;
		if (depthStencilResolve)
		{
			DS_ASSERT(depthStencilResolve->sType ==
				VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR);
			if (depthStencilResolve->pDepthStencilResolveAttachment &&
				depthStencilResolve->pDepthStencilResolveAttachment->attachment !=
					VK_ATTACHMENT_UNUSED)
			{
				usages[depthStencilResolve->pDepthStencilResolveAttachment->attachment] |= usage;
			}
		}
	}

	if (subpass->pDepthStencilAttachment &&
		subpass->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
	{
		usages[subpass->pDepthStencilAttachment->attachment] |= usage;
	}
}

static void findAttachmentsBeforeUses(AttachmentUsage* usages,
	const VkSubpassDescription2KHR* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curDepSubpass,
	uint32_t depth)
{
	if (depth >= subpassCount)
		return;

	// Find all dependencies that have a destination dependency for the current dependency to
	// determine which attachments are written to.
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const VkSubpassDependency* dependency = dependencies + i;
		if (dependency->dstSubpass != curDepSubpass ||
			dependency->srcSubpass == VK_SUBPASS_EXTERNAL)
		{
			continue;
		}

		const VkSubpassDescription2KHR* depSubpass = subpasses + dependency->srcSubpass;
		addSubpassAttachmentUsageBits(usages, depSubpass, AttachmentUsage_WriteBefore);

		// Recurse for the source dependency.
		findAttachmentsBeforeUses(usages, subpasses, subpassCount, dependencies, dependencyCount,
			dependency->srcSubpass, depth + 1);
	}
}

static void findAttachmentsAfterUses(AttachmentUsage* usages,
	const VkSubpassDescription2KHR* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curDepSubpass,
	uint32_t depth)
{
	if (depth >= subpassCount)
		return;

	// Find all dependencies that have a source dependency for the current dependency to
	// determine which attachments are read from.
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const VkSubpassDependency* dependency = dependencies + i;
		if (dependency->srcSubpass != curDepSubpass ||
			dependency->dstSubpass == VK_SUBPASS_EXTERNAL)
		{
			continue;
		}

		const VkSubpassDescription2KHR* depSubpass = subpasses + dependency->dstSubpass;
		addSubpassAttachmentUsageBits(usages, depSubpass, AttachmentUsage_ReadAfter);

		// Recurse for the destination dependency.
		findAttachmentsAfterUses(usages, subpasses, subpassCount, dependencies, dependencyCount,
			dependency->dstSubpass, depth + 1);
	}
}

static void findPreserveAttachments(uint32_t* outAttachments, uint32_t* outCount,
	const VkAttachmentDescription2KHR* attachments, uint32_t attachmentCount,
	const VkSubpassDescription2KHR* subpasses, uint32_t subpassCount,
	const VkSubpassDependency* dependencies, uint32_t dependencyCount, uint32_t curSubpass)
{
	AttachmentUsage* usages = DS_ALLOCATE_STACK_OBJECT_ARRAY(AttachmentUsage, attachmentCount);
	memset(usages, 0, sizeof(AttachmentUsage)*attachmentCount);

	// Find the usage flags for the current subpass, before the current subpass (by dependencies),
	// and after the current subpass (by dependencies).
	addSubpassAttachmentUsageBits(usages, subpasses + curSubpass, AttachmentUsage_Current);
	findAttachmentsBeforeUses(usages, subpasses, subpassCount, dependencies, dependencyCount,
		curSubpass, 0);
	findAttachmentsAfterUses(usages, subpasses, subpassCount, dependencies, dependencyCount,
		curSubpass, 0);

	*outCount = 0;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		// Add implicit uses based on attachment operations.
		const VkAttachmentDescription2KHR* attachment = attachments + i;
		AttachmentUsage usage = usages[i];
		if (attachment->loadOp != VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			usage |= AttachmentUsage_WriteBefore;
		if (attachment->storeOp != VK_ATTACHMENT_STORE_OP_DONT_CARE)
			usage |= AttachmentUsage_ReadAfter;

		// Add attachments that are used before and after, but not during, the current subpass.
		if (usage == (AttachmentUsage_WriteBefore | AttachmentUsage_ReadAfter))
			outAttachments[(*outCount)++] = i;
	}
}

static bool createRenderPass(dsVkRenderPassData* renderPassData, uint32_t resolveAttachmentCount)
{
	DS_UNUSED(resolveAttachmentCount);
	const dsRenderPass* renderPass = renderPassData->renderPass;
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;
	const dsRenderer* renderer = renderPass->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	VkAttachmentDescription2KHR* vkAttachments = NULL;
	if (renderPassData->attachmentCount > 0)
	{
		vkAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkAttachmentDescription2KHR,
			renderPassData->fullAttachmentCount);

		uint32_t resolveIndex = 0;
		for (uint32_t i = 0; i < renderPassData->attachmentCount; ++i)
		{
			const dsAttachmentInfo* attachment = renderPass->attachments + i;
			VkAttachmentDescription2KHR* vkAttachment = vkAttachments + i;
			dsAttachmentUsage usage = attachment->usage;

			const dsVkFormatInfo* format = dsVkResourceManager_getFormat(renderer->resourceManager,
				attachment->format);
			if (!format)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
				return 0;
			}

			vkAttachment->sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
			vkAttachment->pNext = NULL;
			vkAttachment->flags = 0;
			vkAttachment->format = format->vkFormat;
			uint32_t samples = attachment->samples;
			if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
				samples = renderer->surfaceSamples;
			else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
				samples = renderer->defaultSamples;

			vkAttachment->samples = dsVkSampleCount(samples);

			if (usage & dsAttachmentUsage_Clear)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else if (usage & dsAttachmentUsage_KeepBefore)
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			else
				vkAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachment->stencilLoadOp = vkAttachment->loadOp;

			if (mustKeepMultisampledAttachment(usage, samples))
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

			if (dsVkAttachmentHasResolve(renderPass->subpasses, renderPass->subpassCount, i,
					attachment->samples, renderer->surfaceSamples, renderer->defaultSamples))
			{
				uint32_t resolveAttachmentIndex = renderPassData->attachmentCount + resolveIndex;
				VkAttachmentDescription2KHR* vkResolveAttachment = vkAttachments +
					resolveAttachmentIndex;
				*vkResolveAttachment = *vkAttachment;
				vkResolveAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
				vkResolveAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkResolveAttachment->stencilLoadOp = vkResolveAttachment->loadOp;
				if (usage & dsAttachmentUsage_KeepAfter)
					vkResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				else
					vkResolveAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkResolveAttachment->stencilStoreOp = vkResolveAttachment->storeOp;
				++resolveIndex;
			}
		}

		DS_ASSERT(resolveIndex == resolveAttachmentCount);
	}

	VkSubpassDescription2KHR* vkSubpasses = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSubpassDescription2KHR,
		renderPass->subpassCount);
	for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
	{
		const dsRenderSubpassInfo* curSubpass = renderPass->subpasses + i;
		VkSubpassDescription2KHR* vkSubpass = vkSubpasses + i;

		vkSubpass->sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
		vkSubpass->pNext = NULL;
		vkSubpass->flags = 0;
		vkSubpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vkSubpass->inputAttachmentCount = curSubpass->inputAttachmentCount;
		vkSubpass->viewMask = 0;
		vkSubpass->pInputAttachments = NULL;
		vkSubpass->colorAttachmentCount = curSubpass->colorAttachmentCount;
		vkSubpass->pColorAttachments = NULL;
		vkSubpass->pResolveAttachments = NULL;
		vkSubpass->pDepthStencilAttachment = NULL;
		vkSubpass->preserveAttachmentCount = 0;
		vkSubpass->pPreserveAttachments = NULL;

		if (curSubpass->inputAttachmentCount > 0)
		{
			VkAttachmentReference2KHR* inputAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference2KHR, curSubpass->inputAttachmentCount);
			for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
			{
				VkAttachmentReference2KHR* inputAttachment = inputAttachments + j;
				uint32_t attachment = curSubpass->inputAttachments[j];
				inputAttachment->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
				inputAttachment->pNext = NULL;
				if (attachment == DS_NO_ATTACHMENT)
					inputAttachment->attachment = VK_ATTACHMENT_UNUSED;
				else
				{
					// Use resolved result if available.
					uint32_t resolveAttachment = renderPassData->resolveIndices[attachment];
					if (resolveAttachment == DS_NO_ATTACHMENT)
						inputAttachment->attachment = attachment;
					else
						inputAttachment->attachment = resolveAttachment;
				}

				dsGfxFormat format = renderPass->attachments[attachment].format;
				if (attachment == DS_NO_ATTACHMENT)
					inputAttachment->layout = VK_IMAGE_LAYOUT_GENERAL;
				else if (dsGfxFormat_isDepthStencil(format))
					inputAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				else
					inputAttachment->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				inputAttachment->aspectMask = dsVkImageAspectFlags(format);
			}
			vkSubpass->pInputAttachments = inputAttachments;
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			VkAttachmentReference2KHR* colorAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
				VkAttachmentReference2KHR, curSubpass->colorAttachmentCount);

			bool hasResolve = false;
			for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
			{
				VkAttachmentReference2KHR* colorAttachment = colorAttachments + j;
				const dsAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
				uint32_t attachmentIndex = curAttachment->attachmentIndex;
				colorAttachment->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
				colorAttachment->pNext = NULL;
				colorAttachment->attachment = attachmentIndex;
				colorAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachment->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

				if (attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve &&
					needsResolve(renderPass->attachments[curAttachment->attachmentIndex].samples,
						renderer->surfaceSamples, renderer->defaultSamples))
				{
					hasResolve = true;
				}
			}

			vkSubpass->pColorAttachments = colorAttachments;
			if (hasResolve)
			{
				VkAttachmentReference2KHR* resolveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(
					VkAttachmentReference2KHR, curSubpass->colorAttachmentCount);

				for (uint32_t j = 0; j < vkSubpass->colorAttachmentCount; ++j)
				{
					const dsAttachmentRef* curAttachment = curSubpass->colorAttachments + j;
					VkAttachmentReference2KHR* resolveAttachment = resolveAttachments + j;
					uint32_t attachmentIndex = curAttachment->attachmentIndex;
					resolveAttachment->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
					resolveAttachment->pNext = NULL;
					if (attachmentIndex != DS_NO_ATTACHMENT && curAttachment->resolve &&
						needsResolve(
							renderPass->attachments[curAttachment->attachmentIndex].samples,
							renderer->surfaceSamples, renderer->defaultSamples))
					{
						uint32_t resolveIndex = renderPassData->resolveIndices[attachmentIndex];
						DS_ASSERT(resolveIndex != DS_NO_ATTACHMENT);
						resolveAttachment->attachment = resolveIndex;
					}
					else
						resolveAttachment->attachment = VK_ATTACHMENT_UNUSED;
					resolveAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					resolveAttachment->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				vkSubpass->pResolveAttachments = resolveAttachments;
			}
		}

		const dsAttachmentRef* depthStencilAttachment = &curSubpass->depthStencilAttachment;
		if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT)
		{
			// Check if the depth is also used as an input, in which case this can be used for
			// read-only depth checks.
			bool resolve = depthStencilAttachment->resolve && needsResolve(
				renderPass->attachments[depthStencilAttachment->attachmentIndex].samples,
				renderer->surfaceSamples, renderer->defaultSamples);
			bool isInput = false;
			if (!resolve)
			{
				for (uint32_t j = 0; j < vkSubpass->inputAttachmentCount; ++j)
				{
					if (vkSubpass->pInputAttachments[j].attachment ==
						depthStencilAttachment->attachmentIndex)
					{
						isInput = true;
						break;
					}
				}
			}

			VkImageAspectFlags aspectMask = dsVkImageAspectFlags(
				renderPass->attachments[depthStencilAttachment->attachmentIndex].format);
			VkAttachmentReference2KHR* depthAttachment =
				DS_ALLOCATE_STACK_OBJECT(VkAttachmentReference2KHR);
			depthAttachment->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
			depthAttachment->pNext = NULL;
			depthAttachment->attachment = depthStencilAttachment->attachmentIndex;
			depthAttachment->layout = isInput ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment->aspectMask = aspectMask;
			vkSubpass->pDepthStencilAttachment = depthAttachment;

			if (resolve)
			{
				uint32_t resolveAttachment =
					renderPassData->resolveIndices[depthStencilAttachment->attachmentIndex];
				DS_ASSERT(resolveAttachment != DS_NO_ATTACHMENT);
				VkAttachmentReference2KHR* attachmentRef =
					DS_ALLOCATE_STACK_OBJECT(VkAttachmentReference2KHR);
				attachmentRef->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
				attachmentRef->pNext = NULL;
				attachmentRef->attachment = resolveAttachment;
				attachmentRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				attachmentRef->aspectMask = aspectMask;

				VkSubpassDescriptionDepthStencilResolveKHR* depthStencilResolve =
					DS_ALLOCATE_STACK_OBJECT(VkSubpassDescriptionDepthStencilResolveKHR);
				depthStencilResolve->sType =
					VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
				depthStencilResolve->pNext = NULL;
				if (attachmentRef->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
					depthStencilResolve->depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
				else
					depthStencilResolve->depthResolveMode = VK_RESOLVE_MODE_NONE_KHR;
				if (attachmentRef->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
					depthStencilResolve->stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
				else
					depthStencilResolve->stencilResolveMode = VK_RESOLVE_MODE_NONE_KHR;
				depthStencilResolve->pDepthStencilResolveAttachment = attachmentRef;

				vkSubpass->pNext = depthStencilResolve;
			}
		}
	}

	// Set up dependencies after all of the subpasses are otherwise set up.
	for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
	{
		VkSubpassDescription2KHR* vkSubpass = vkSubpasses + i;
		uint32_t* preserveAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t,
			renderPassData->fullAttachmentCount);
		DS_ASSERT(preserveAttachments);
		vkSubpass->pPreserveAttachments = preserveAttachments;
		findPreserveAttachments(preserveAttachments, &vkSubpass->preserveAttachmentCount,
			vkAttachments, renderPassData->fullAttachmentCount, vkSubpasses,
			renderPass->subpassCount, vkRenderPass->vkDependencies,
			renderPass->subpassDependencyCount, i);
	}

	VkSubpassDependency2KHR* dependencies = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSubpassDependency2KHR,
		renderPass->subpassDependencyCount);
	for (uint32_t i = 0; i < renderPass->subpassDependencyCount; ++i)
	{
		VkSubpassDependency2KHR* dependency = dependencies + i;
		VkSubpassDependency* baseDependency = vkRenderPass->vkDependencies + i;
		dependency->sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
		dependency->pNext = NULL;
		dependency->srcSubpass = baseDependency->srcSubpass;
		dependency->dstSubpass = baseDependency->dstSubpass;
		dependency->srcStageMask = baseDependency->srcStageMask;
		dependency->dstStageMask = baseDependency->dstStageMask;
		dependency->srcAccessMask = baseDependency->srcAccessMask;
		dependency->dstAccessMask = baseDependency->dstAccessMask;
		dependency->dependencyFlags = baseDependency->dependencyFlags;
		dependency->viewOffset = 0;
	}

	VkRenderPassCreateInfo2KHR createInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
		NULL,
		0,
		renderPassData->fullAttachmentCount, vkAttachments,
		renderPass->subpassCount, vkSubpasses,
		renderPass->subpassDependencyCount, dependencies,
		0, NULL
	};

	VkResult result = DS_VK_CALL(device->vkCreateRenderPass2)(device->device, &createInfo,
		instance->allocCallbacksPtr, &renderPassData->vkRenderPass);
	return DS_HANDLE_VK_RESULT(result, "Couldn't create render pass");
}

bool dsVkAttachmentHasResolve(const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	uint32_t attachment, uint32_t samples, uint32_t surfaceSamples, uint32_t defaultSamples)
{
	if (samples == 1 || (samples == DS_SURFACE_ANTIALIAS_SAMPLES && surfaceSamples == 1) ||
		(samples == DS_DEFAULT_ANTIALIAS_SAMPLES && defaultSamples == 1))
	{
		return false;
	}

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

		if (subpass->depthStencilAttachment.attachmentIndex == attachment &&
			subpass->depthStencilAttachment.resolve)
		{
			return true;
		}
	}
	return false;
}

bool dsCreateUnderlyingVkRenderPass(
	dsVkRenderPassData* renderPassData, uint32_t resolveAttachmentCount)
{
	if (renderPassData->device->vkCreateRenderPass2)
		return createRenderPass(renderPassData, resolveAttachmentCount);
	return createLegacyRenderPass(renderPassData, resolveAttachmentCount);
}
