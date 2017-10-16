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

#include "GLRenderPass.h"

#include "Resources/GLResource.h"
#include "GLCommandBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsRenderPass* dsGLRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	size_t attachmentArraySize = sizeof(dsAttachmentInfo)*attachmentCount;
	size_t subpassArraySize = sizeof(dsRenderSubpassInfo)*subpassCount;
	size_t dependencyArraySize = sizeof(dsSubpassDependency)*dependencyCount;
	size_t clearSubpassArraySize = sizeof(uint32_t)*attachmentCount;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLRenderPass)) +
		DS_ALIGNED_SIZE(attachmentArraySize) + DS_ALIGNED_SIZE(subpassArraySize) +
		DS_ALIGNED_SIZE(dependencyArraySize) + DS_ALIGNED_SIZE(clearSubpassArraySize);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsColorAttachmentRef)*subpasses[i].colorAttachmentCount);
	}
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGLRenderPass* renderPass = (dsGLRenderPass*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsGLRenderPass));
	DS_ASSERT(renderPass);

	dsRenderPass* baseRenderPass = (dsRenderPass*)renderPass;
	baseRenderPass->renderer = renderer;
	baseRenderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		baseRenderPass->attachments = (dsAttachmentInfo*)dsAllocator_alloc(
			(dsAllocator*)&bufferAlloc, attachmentArraySize);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments, attachmentArraySize);

		// Find the first subpass that cleared attachments appear in.
		renderPass->clearSubpass = (uint32_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			clearSubpassArraySize);
		memset(renderPass->clearSubpass, 0xFF, clearSubpassArraySize);
		for (uint32_t i = 0; i < subpassCount; ++i)
		{
			for (uint32_t j = 0; j < subpasses[i].colorAttachmentCount; ++j)
			{
				uint32_t attachmentIndex = subpasses[i].colorAttachments[j].attachmentIndex;
				if (attachmentIndex == DS_NO_ATTACHMENT)
					continue;
				const dsAttachmentInfo* attachment = attachments + attachmentIndex;
				if (!(attachment->usage & dsAttachmentUsage_Clear))
					continue;

				if (renderPass->clearSubpass[attachmentIndex] == DS_NO_ATTACHMENT)
					renderPass->clearSubpass[attachmentIndex] = i;
			}

			if (subpasses[i].depthStencilAttachment != DS_NO_ATTACHMENT)
			{
				uint32_t attachmentIndex = subpasses[i].depthStencilAttachment;
				const dsAttachmentInfo* attachment = attachments + attachmentIndex;
				if ((attachment->usage & dsAttachmentUsage_Clear) &&
					renderPass->clearSubpass[attachmentIndex] == DS_NO_ATTACHMENT)
				{
					renderPass->clearSubpass[attachmentIndex] = i;
				}
			}
		}
	}
	else
	{
		baseRenderPass->attachments = NULL;
		renderPass->clearSubpass = NULL;
	}

	dsRenderSubpassInfo* subpassesCopy = (dsRenderSubpassInfo*)dsAllocator_alloc(
		(dsAllocator*)&bufferAlloc, subpassArraySize);
	DS_ASSERT(subpassesCopy);
	memcpy(subpassesCopy, subpasses, subpassArraySize);
	baseRenderPass->subpasses = subpassesCopy;
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;
		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = (uint32_t*)dsAllocator_alloc
				((dsAllocator*)&bufferAlloc, sizeof(uint32_t)*curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = (dsColorAttachmentRef*)dsAllocator_alloc
				((dsAllocator*)&bufferAlloc,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);
		}
	}

	if (dependencyCount > 0)
	{
		baseRenderPass->subpassDependencies = (dsSubpassDependency*)dsAllocator_alloc(
			(dsAllocator*)&bufferAlloc, dependencyArraySize);
		DS_ASSERT(baseRenderPass->subpassDependencies);
		memcpy((void*)baseRenderPass->subpassDependencies, dependencies, dependencyArraySize);
	}
	else
		baseRenderPass->subpassDependencies = NULL;

	baseRenderPass->attachmentCount = attachmentCount;
	baseRenderPass->subpassCount = subpassCount;
	baseRenderPass->subpassDependencyCount = dependencyCount;

	dsGLResource_initialize(&renderPass->resource);
	return baseRenderPass;
}

bool dsGLRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool indirectCommands)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(framebuffer);
	DS_UNUSED(indirectCommands);

	return dsGLCommandBuffer_beginRenderPass(commandBuffer, renderPass, framebuffer, viewport,
		clearValues, clearValueCount);
}

bool dsGLRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, bool indirectCommands)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(indirectCommands);

	return dsGLCommandBuffer_nextRenderSubpass(commandBuffer, renderPass);
}

bool dsGLRenderPass_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(renderPass);

	return dsGLCommandBuffer_endRenderPass(commandBuffer, renderPass);
}

static bool destroyImpl(dsRenderPass* renderPass)
{
	if (renderPass->allocator)
		return dsAllocator_free(renderPass->allocator, renderPass);

	return true;
}

bool dsGLRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	DS_UNUSED(renderer);
	DS_ASSERT(renderPass);

	dsGLRenderPass* glRenderPass = (dsGLRenderPass*)renderPass;
	if (dsGLResource_destroy(&glRenderPass->resource))
		return destroyImpl(renderPass);

	return true;
}

void dsGLRenderPass_addInternalRef(dsRenderPass* renderPass)
{
	DS_ASSERT(renderPass);
	dsGLRenderPass* glRenderPass = (dsGLRenderPass*)renderPass;
	dsGLResource_addRef(&glRenderPass->resource);
}

void dsGLRenderPass_freeInternalRef(dsRenderPass* renderPass)
{
	DS_ASSERT(renderPass);
	dsGLRenderPass* glRenderPass = (dsGLRenderPass*)renderPass;
	if (dsGLResource_freeRef(&glRenderPass->resource))
		destroyImpl(renderPass);
}
