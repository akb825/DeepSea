/*
 * Copyright 2017-2019 Aaron Barany
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

#include "MockRenderPass.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsRenderPass* dsMockRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	DS_ASSERT(attachments || attachmentCount == 0);
	DS_ASSERT(subpasses && subpassCount > 0);
	DS_ASSERT(dependencies || dependencyCount == 0 ||
		dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES);

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
	dsRenderPass* renderPass = DS_ALLOCATE_OBJECT(&bufferAllocator, dsRenderPass);
	DS_ASSERT(renderPass);
	renderPass->renderer = renderer;
	renderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		renderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAllocator, dsAttachmentInfo,
			attachmentCount);
		DS_ASSERT(renderPass->attachments);
		memcpy((void*)renderPass->attachments, attachments,
			sizeof(dsAttachmentInfo)*attachmentCount);
		renderPass->attachmentCount = attachmentCount;
	}
	else
	{
		renderPass->attachments = NULL;
		renderPass->attachmentCount = 0;
	}

	renderPass->subpasses = DS_ALLOCATE_OBJECT_ARRAY(&bufferAllocator, dsRenderSubpassInfo,
		subpassCount);
	DS_ASSERT(renderPass->subpasses);
	memcpy((void*)renderPass->subpasses, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)renderPass->subpasses + i;
		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAllocator, uint32_t,
				curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAllocator,
				dsColorAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);
		}
	}
	renderPass->subpassCount = subpassCount;

	if (finalDependencyCount > 0)
	{
		renderPass->subpassDependencies = DS_ALLOCATE_OBJECT_ARRAY(&bufferAllocator,
			dsSubpassDependency, finalDependencyCount);
		if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		{
			for (uint32_t i = 0; i < subpassCount; ++i)
			{
				dsSubpassDependency* dependency =
					(dsSubpassDependency*)(renderPass->subpassDependencies + i);
				dependency->srcSubpass = i == 0 ? DS_EXTERNAL_SUBPASS : i - 1;
				dependency->srcStage = dsSubpassDependencyStage_Fragment;
				dependency->dstSubpass = i;
				dependency->dstStage = dsSubpassDependencyStage_Fragment;
				dependency->regionDependency = i > 0;
			}
		}
		else
		{
			DS_ASSERT(renderPass->subpassDependencies);
			memcpy((void*)renderPass->subpassDependencies, dependencies,
				sizeof(dsSubpassDependency)*dependencyCount);
		}
		renderPass->subpassDependencyCount = finalDependencyCount;
	}
	else
	{
		renderPass->subpassDependencies = NULL;
		renderPass->subpassDependencyCount = 0;
	}

	return renderPass;
}

bool dsMockRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(renderPass);
	DS_ASSERT(framebuffer);
	DS_UNUSED(framebuffer);
	DS_UNUSED(viewport);
	DS_UNUSED(clearValues);
	DS_UNUSED(clearValueCount);

	return true;
}

bool dsMockRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(renderPass);
	DS_ASSERT(index < renderPass->subpassCount);
	DS_UNUSED(index);

	return true;
}

bool dsMockRenderPass_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(renderPass);

	return true;
}

bool dsMockRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(renderPass);

	if (renderPass->allocator)
		return dsAllocator_free(renderPass->allocator, renderPass);
	return true;
}
