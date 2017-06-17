/*
 * Copyright 2017 Aaron Barany
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
	DS_ASSERT(dependencies || dependencyCount == 0);

	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = subpassCount - 1;
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsAttachmentInfo)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(dsRenderSubpassInfo)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(dsSubpassDependency)*finalDependencyCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		totalSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].colorAttachmentCount);
	}
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, totalSize));
	dsRenderPass* renderPass = (dsRenderPass*)dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		sizeof(dsRenderPass));
	DS_ASSERT(renderPass);
	renderPass->renderer = renderer;
	renderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		renderPass->attachments = (dsAttachmentInfo*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsAttachmentInfo)*attachmentCount);
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

	renderPass->subpasses = (dsRenderSubpassInfo*)dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		sizeof(dsRenderSubpassInfo)*subpassCount);
	DS_ASSERT(renderPass->subpasses);
	memcpy((void*)renderPass->subpasses, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)renderPass->subpasses + i;
		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = (uint32_t*)dsAllocator_alloc
				((dsAllocator*)&bufferAllocator, sizeof(uint32_t)*curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = (uint32_t*)dsAllocator_alloc
				((dsAllocator*)&bufferAllocator, sizeof(uint32_t)*curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(uint32_t)*curSubpass->colorAttachmentCount);
		}
	}
	renderPass->subpassCount = subpassCount;

	if (dependencyCount > 0)
	{
		renderPass->subpassDependencies = (dsSubpassDependency*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsSubpassDependency)*dependencyCount);
		DS_ASSERT(renderPass->subpassDependencies);
		memcpy((void*)renderPass->subpassDependencies, dependencies,
			sizeof(dsSubpassDependency)*dependencyCount);
		renderPass->subpassDependencyCount = dependencyCount;
	}
	else if (subpassCount > 1)
	{
		renderPass->subpassDependencies = (dsSubpassDependency*)dsAllocator_alloc(
			(dsAllocator*)&bufferAllocator, sizeof(dsSubpassDependency)*finalDependencyCount);
		DS_ASSERT(renderPass->subpassDependencies);
		renderPass->subpassDependencyCount = finalDependencyCount;
		for (uint32_t i = 0; i < finalDependencyCount; ++i)
		{
			dsSubpassDependency* dependency =
				(dsSubpassDependency*)renderPass->subpassDependencies + i;
			dependency->srcStage = i;
			dependency->srcSubpass = dsSubpassDependencyStage_Fragment;
			dependency->dstStage = i + 1;
			dependency->dstSubpass = dsSubpassDependencyStage_Fragment;
			dependency->regionDependency = true;
		}
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
	uint32_t clearValueCount, bool indirectCommands)
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
	DS_UNUSED(indirectCommands);

	return true;
}

bool dsMockRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, bool indirectCommands)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(renderPass);
	DS_UNUSED(indirectCommands);

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
