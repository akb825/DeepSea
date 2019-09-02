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

static size_t fullAllocSize(uint32_t attachmentCount, const dsRenderSubpassInfo* subpasses,
	uint32_t subpassCount, uint32_t dependencyCount)
{
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsVkRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsAttachmentInfo)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(dsSubpassDependency)*dependencyCount) +
		DS_ALIGNED_SIZE(sizeof(VkSubpassDependency)*dependencyCount) +
		DS_ALIGNED_SIZE(sizeof(dsRenderSubpassInfo)*subpassCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		totalSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsColorAttachmentRef)*subpasses[i].colorAttachmentCount) +
			DS_ALIGNED_SIZE(strlen(subpasses[i].name) + 1);
	}
	return totalSize;
}

static VkPipelineStageFlags getPipelineStages(const dsRenderer* renderer,
	dsSubpassDependencyFlags stages)
{
	VkPipelineStageFlags flags = 0;
	if (stages & dsSubpassDependencyFlags_DrawIndirect)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	if (stages & (dsSubpassDependencyFlags_VertexAttribute | dsSubpassDependencyFlags_Index))
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	if (stages & (dsSubpassDependencyFlags_VertexShaderRead |
			dsSubpassDependencyFlags_VertexShaderWrite))
	{
		flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	if (renderer->hasTessellationShaders)
	{
		if ((stages & (dsSubpassDependencyFlags_TessControlShaderRead |
				dsSubpassDependencyFlags_TessControlShaderWrite)))
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
		}
		if (stages & (dsSubpassDependencyFlags_TessEvalShaderRead |
				dsSubpassDependencyFlags_TessEvalShaderWrite))
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
	}
	if ((stages & (dsSubpassDependencyFlags_GeometryShaderRead |
			dsSubpassDependencyFlags_GeometryShaderWrite)) && renderer->hasGeometryShaders)
	{
		flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (stages & (dsSubpassDependencyFlags_FragmentShaderRead |
			dsSubpassDependencyFlags_FragmentShaderWrite))
	{
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (stages & (dsSubpassDependencyFlags_FragmentPreShadingTests |
			dsSubpassDependencyFlags_DepthStencilAttachmentRead))
	{
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	if (stages & (dsSubpassDependencyFlags_FragmentColorOutput |
			dsSubpassDependencyFlags_ColorAttachmentRead))
	{
		flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (stages & (dsSubpassDependencyFlags_FragmentPostShadingTests |
			dsSubpassDependencyFlags_DepthStencilAttachmentRead))
	{
		flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	return flags;
}

static VkAccessFlags getAccessFlags(dsSubpassDependencyFlags stages)
{
	VkAccessFlags flags = 0;
	if (stages & dsSubpassDependencyFlags_DrawIndirect)
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (stages & dsSubpassDependencyFlags_VertexAttribute)
		flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (stages & dsSubpassDependencyFlags_Index)
		flags |= VK_ACCESS_INDEX_READ_BIT;
	if (stages & (dsSubpassDependencyFlags_VertexShaderRead |
			dsSubpassDependencyFlags_TessControlShaderRead |
			dsSubpassDependencyFlags_TessEvalShaderRead |
			dsSubpassDependencyFlags_GeometryShaderRead |
			dsSubpassDependencyFlags_FragmentShaderRead))
	{
		flags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (stages & (dsSubpassDependencyFlags_VertexShaderWrite |
			dsSubpassDependencyFlags_TessControlShaderWrite |
			dsSubpassDependencyFlags_TessEvalShaderWrite |
			dsSubpassDependencyFlags_GeometryShaderWrite |
			dsSubpassDependencyFlags_FragmentShaderWrite))
	{
		flags |= VK_ACCESS_SHADER_WRITE_BIT;
	}
	if (stages & dsSubpassDependencyFlags_FragmentShaderRead)
		flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	if (flags & (dsSubpassDependencyFlags_FragmentPreShadingTests |
			dsSubpassDependencyFlags_DepthStencilAttachmentRead))
	{
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	if (flags & dsSubpassDependencyFlags_FragmentColorOutput)
		flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (flags & dsSubpassDependencyFlags_FragmentPostShadingTests)
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (flags & dsSubpassDependencyFlags_ColorAttachmentRead)
		flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	return flags;
}

dsRenderPass* dsVkRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = 0;
	else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		finalDependencyCount = subpassCount + 1;

	size_t totalSize = fullAllocSize(attachmentCount, subpasses, subpassCount,
		finalDependencyCount);
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, totalSize));
	dsVkRenderPass* renderPass = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkRenderPass);
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
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsAttachmentInfo,
			attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments,
			sizeof(dsAttachmentInfo)*attachmentCount);

		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			if (attachments[i].samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			{
				renderPass->usesDefaultSamples = true;
				break;
			}
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
				dsColorAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);
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
			for (uint32_t i = 0; i < subpassCount; ++i)
			{
				dsSubpassDependency* dependency =
					(dsSubpassDependency*)(baseRenderPass->subpassDependencies + i);
				dependency->srcSubpass = i == 0 ? DS_EXTERNAL_SUBPASS : i - 1;
				dependency->srcStages = dsSubpassDependencyFlags_FragmentColorOutput |
					dsSubpassDependencyFlags_FragmentPostShadingTests |
					dsSubpassDependencyFlags_RenderPipeline;
				dependency->dstSubpass = i;
				dependency->dstStages = dsSubpassDependencyFlags_FragmentShaderRead;
				if (i == 0)
				{
					dependency->dstStages |= dsSubpassDependencyFlags_DepthStencilAttachmentRead |
						dsSubpassDependencyFlags_ColorAttachmentRead |
						dsSubpassDependencyFlags_RenderPipeline;
				}
				dependency->regionDependency = i > 0;
			}

			dsSubpassDependency* lastDependency =
				(dsSubpassDependency*)(baseRenderPass->subpassDependencies + subpassCount);
			lastDependency->srcSubpass = subpassCount - 1;
			lastDependency->srcStages = dsSubpassDependencyFlags_RenderPipeline;
			lastDependency->dstSubpass = DS_EXTERNAL_SUBPASS;
			lastDependency->dstStages = dsSubpassDependencyFlags_RenderPipeline;
			lastDependency->regionDependency = false;
		}
		else
		{
			DS_ASSERT(baseRenderPass->subpassDependencies);
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
			vkDependency->srcStageMask = getPipelineStages(renderer, curDependency->srcStages);
			vkDependency->dstStageMask = getPipelineStages(renderer, curDependency->dstStages);
			vkDependency->srcAccessMask = getAccessFlags(curDependency->srcStages);
			vkDependency->dstAccessMask = getAccessFlags(curDependency->dstStages);

			// Default implicit dependencies for external subpasses.
			if (curDependency->srcSubpass == DS_EXTERNAL_SUBPASS &&
				((curDependency->srcStages & dsSubpassDependencyFlags_RenderPipeline) ||
					(curDependency->dstStages & dsSubpassDependencyFlags_RenderPipeline)))
			{
				vkDependency->srcStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				vkDependency->dstStageMask |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				vkDependency->dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			if (curDependency->dstSubpass == DS_EXTERNAL_SUBPASS &&
				((curDependency->srcStages & dsSubpassDependencyFlags_RenderPipeline) ||
					(curDependency->dstStages & dsSubpassDependencyFlags_RenderPipeline)))
			{
				vkDependency->srcStageMask |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				vkDependency->dstStageMask |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				vkDependency->srcAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

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

	renderPass->renderPassData = dsVkRenderPassData_create(renderPass->scratchAllocator,
		device, baseRenderPass);
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
		dsVkRenderPassData* renderPassData = dsVkRenderPassData_create(
			vkRenderPass->scratchAllocator, device, renderPass);
		if (renderPassData)
		{
			dsVkRenderer_deleteRenderPass(renderer, vkRenderPass->renderPassData);
			vkRenderPass->renderPassData = renderPassData;
		}

		vkRenderPass->defaultSamples = samples;
	}

	vkRenderPass->lastCheckedFrame = frame;
	DS_VERIFY(dsSpinlock_unlock(&vkRenderPass->lock));
	return vkRenderPass->renderPassData;
}
