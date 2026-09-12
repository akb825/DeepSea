/*
 * Copyright 2018-2026 Aaron Barany
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
#include <DeepSea/Render/RenderPass.h>
#include <string.h>

static size_t fullAllocSize(uint32_t attachmentCount, const dsRenderSubpassInfo* subpasses,
	uint32_t subpassCount, uint32_t dependencyCount)
{
	size_t fullSize = sizeof(dsVkRenderPass);
	dsMemorySize sizes[] =
	{
		{sizeof(dsRenderPassAttachmentInfo), attachmentCount},
		{sizeof(dsSubpassDependency), dependencyCount},
		{sizeof(VkSubpassDependency), dependencyCount},
		{sizeof(dsRenderSubpassInfo), subpassCount}
	};
	if (!dsAccumulateAlignedSizes(&fullSize, sizes, DS_ARRAY_SIZE(sizes), DS_ALLOC_ALIGNMENT))
		return 0;

	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		const dsRenderSubpassInfo* subpass = subpasses + i;
		dsMemorySize subpassSizes[] =
		{
			{sizeof(uint32_t), subpass->inputAttachmentCount},
			{sizeof(dsRenderPassAttachmentRef), subpass->colorAttachmentCount},
			{sizeof(char), strlen(subpass->name) + 1}
		};
		if (!dsAccumulateAlignedSizes(
				&fullSize, subpassSizes, DS_ARRAY_SIZE(subpassSizes), DS_ALLOC_ALIGNMENT))
		{
			return 0;
		}
	}
	return fullSize;
}

dsRenderPass* dsVkRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsRenderPassAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = 0;
	else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		finalDependencyCount = dsRenderPass_countDefaultDependencies(subpasses, subpassCount);

	size_t fullSize = fullAllocSize(
		attachmentCount, subpasses, subpassCount, finalDependencyCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkRenderPass* renderPass = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkRenderPass);
	DS_ASSERT(renderPass);

	renderPass->lastCheckedFrame = renderer->frameNumber;
	renderPass->scratchAllocator = renderer->allocator;
	renderPass->surfaceSamples = renderer->surfaceSamples;
	renderPass->defaultSamples = renderer->defaultSamples;
	renderPass->surfaceColorFormat = renderer->surfaceColorFormat;
	renderPass->surfaceDepthStencilFormat = renderer->surfaceDepthStencilFormat;
	renderPass->usesDefaultSamples = false;
	renderPass->usesSurfaceColorFormat = false;
	renderPass->usesSurfaceDepthStencilFormat = false;
	renderPass->renderPassData = NULL;
	DS_VERIFY(dsSpinlock_initialize(&renderPass->lock));

	dsRenderPass* baseRenderPass = (dsRenderPass*)renderPass;
	baseRenderPass->renderer = renderer;
	baseRenderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, dsRenderPassAttachmentInfo, attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments,
			sizeof(dsRenderPassAttachmentInfo)*attachmentCount);

		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			const dsRenderPassAttachmentInfo* attachment = attachments + i;
			if (attachment->samples == DS_SURFACE_ANTIALIAS_SAMPLES ||
				attachment->samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			{
				renderPass->usesDefaultSamples = true;
			}

			if (attachment->format == dsGfxFormat_SurfaceColor)
				renderPass->usesSurfaceColorFormat = true;

			if (attachment->format == dsGfxFormat_SurfaceDepthStencil)
				renderPass->usesSurfaceDepthStencilFormat = true;
		}
	}
	else
	{
		baseRenderPass->attachments = NULL;
	}
	baseRenderPass->attachmentCount = attachmentCount;

	baseRenderPass->subpasses = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsRenderSubpassInfo,
		subpassCount);
	DS_ASSERT(baseRenderPass->subpasses);
	memcpy((void*)baseRenderPass->subpasses, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);

	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;

		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t,
				curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
				dsRenderPassAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsRenderPassAttachmentRef)*curSubpass->colorAttachmentCount);
		}

		size_t nameLen = strlen(subpasses[i].name) + 1;
		curSubpass->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(curSubpass->name);
		memcpy((void*)curSubpass->name, subpasses[i].name, nameLen);
	}
	baseRenderPass->subpassCount = subpassCount;

	if (finalDependencyCount > 0)
	{
		baseRenderPass->subpassDependencies = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
			dsSubpassDependency, finalDependencyCount);
		DS_ASSERT(baseRenderPass->subpassDependencies);
		if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		{
			DS_VERIFY(dsRenderPass_setDefaultDependencies(
				(dsSubpassDependency*)baseRenderPass->subpassDependencies, finalDependencyCount,
				subpasses, subpassCount));
		}
		else
		{
			memcpy((void*)baseRenderPass->subpassDependencies, dependencies,
				sizeof(dsSubpassDependency)*dependencyCount);
		}

		renderPass->vkDependencies = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkSubpassDependency,
			finalDependencyCount);
		DS_ASSERT(renderPass->vkDependencies);
		for (uint32_t i = 0; i < finalDependencyCount; ++i)
		{
			const dsSubpassDependency* curDependency = baseRenderPass->subpassDependencies + i;
			VkSubpassDependency* vkDependency = renderPass->vkDependencies + i;
			vkDependency->srcSubpass = curDependency->srcSubpass;
			vkDependency->dstSubpass = curDependency->dstSubpass;
			vkDependency->srcStageMask =
				dsVkPipelineStageFlags(renderer, curDependency->srcStages, true);
			vkDependency->dstStageMask =
				dsVkPipelineStageFlags(renderer, curDependency->dstStages, false);
			vkDependency->srcAccessMask = dsVkAccessFlags(curDependency->srcAccess);
			vkDependency->dstAccessMask = dsVkAccessFlags(curDependency->dstAccess);

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

	baseRenderPass->subpassCount = subpassCount;

	renderPass->renderPassData = dsVkRenderPassData_create(
		renderPass->scratchAllocator, device, baseRenderPass);
	if (!renderPass->renderPassData)
	{
		dsVkRenderPass_destroy(renderer, baseRenderPass);
		return false;
	}

	return baseRenderPass;
}

bool dsVkRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsAlignedBox2f* scissor,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount, bool secondary)
{
	DS_UNUSED(renderer);
	dsVkRenderPassData* renderPassData = dsVkRenderPass_getData(renderPass);
	return dsVkRenderPassData_begin(renderPassData, commandBuffer, framebuffer, viewport, scissor,
		clearValues, clearValueCount, secondary);
}

bool dsVkRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index, bool secondary)
{
	DS_UNUSED(renderer);
	// Guaranteed that dsVkRenderPass_getData() was called earlier, and will return the same value.
	const dsVkRenderPassData* renderPassData = ((const dsVkRenderPass*)renderPass)->renderPassData;
	return dsVkRenderPassData_nextSubpass(renderPassData, commandBuffer, index, secondary);
}

bool dsVkRenderPass_end(
	dsRenderer* renderer, dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass)
{
	DS_UNUSED(renderer);
	// Guaranteed that dsVkRenderPass_getData() was called earlier, and will return the same value.
	const dsVkRenderPassData* renderPassData = ((const dsVkRenderPass*)renderPass)->renderPassData;
	return dsVkRenderPassData_end(renderPassData, commandBuffer);
}

bool dsVkRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	dsVkRenderPass* vkRenderPass = (dsVkRenderPass*)renderPass;
	dsVkRenderer_deleteRenderPass(renderer, vkRenderPass->renderPassData, false);
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
	uint32_t surfaceSamples = renderer->surfaceSamples;
	uint32_t defaultSamples = renderer->defaultSamples;
	dsGfxFormat surfaceColorFormat = renderer->surfaceColorFormat;
	dsGfxFormat surfaceDepthFormat = renderer->surfaceDepthStencilFormat;

	DS_VERIFY(dsSpinlock_lock(&vkRenderPass->lock));
	if (vkRenderPass->lastCheckedFrame == frame)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->lock));
		return vkRenderPass->renderPassData;
	}

	if ((vkRenderPass->usesDefaultSamples && (surfaceSamples != vkRenderPass->surfaceSamples ||
			defaultSamples != vkRenderPass->defaultSamples)) ||
		(vkRenderPass->usesSurfaceColorFormat &&
			surfaceColorFormat != vkRenderPass->surfaceColorFormat) ||
		(vkRenderPass->usesSurfaceDepthStencilFormat &&
			surfaceDepthFormat != vkRenderPass->surfaceDepthStencilFormat))
	{
		dsVkRenderPassData* renderPassData = dsVkRenderPassData_create(
			vkRenderPass->scratchAllocator, device, renderPass);
		if (renderPassData)
		{
			dsVkRenderer_deleteRenderPass(renderer, vkRenderPass->renderPassData, false);
			vkRenderPass->renderPassData = renderPassData;
		}

		vkRenderPass->surfaceSamples = surfaceSamples;
		vkRenderPass->defaultSamples = defaultSamples;
		vkRenderPass->surfaceColorFormat = surfaceColorFormat;
		vkRenderPass->surfaceDepthStencilFormat = surfaceDepthFormat;
	}

	vkRenderPass->lastCheckedFrame = frame;
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->lock));
	return vkRenderPass->renderPassData;
}
