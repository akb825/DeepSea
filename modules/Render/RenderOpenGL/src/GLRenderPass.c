/*
 * Copyright 2017-2026 Aaron Barany
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
#include <DeepSea/Render/RenderPass.h>
#include <string.h>

dsRenderPass* dsGLRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);

	uint32_t finalDependencyCount = dependencyCount;
	if (dependencyCount == 0)
		finalDependencyCount = 0;
	else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
		finalDependencyCount = dsRenderPass_countDefaultDependencies(subpasses, subpassCount);

	size_t fullSize = sizeof(dsGLRenderPass);
	dsMemorySize sizes[] =
	{
		{sizeof(dsAttachmentInfo), attachmentCount},
		{sizeof(dsRenderSubpassInfo), subpassCount},
		{sizeof(dsSubpassDependency), finalDependencyCount},
		{sizeof(uint32_t), attachmentCount}
	};
	if (!dsAccumulateAlignedSizes(&fullSize, sizes, DS_ARRAY_SIZE(sizes), DS_ALLOC_ALIGNMENT))
		return NULL;

	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		const dsRenderSubpassInfo* subpass = subpasses + i;
		dsMemorySize subpassSizes[] =
		{
			{sizeof(uint32_t), subpass->inputAttachmentCount},
			{sizeof(dsAttachmentRef), subpass->colorAttachmentCount},
			{sizeof(char), strlen(subpass->name) + 1}
		};
		if (!dsAccumulateAlignedSizes(
				&fullSize, subpassSizes, DS_ARRAY_SIZE(subpassSizes), DS_ALLOC_ALIGNMENT))
		{
			return NULL;
		}
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGLRenderPass* renderPass = DS_ALLOCATE_OBJECT(&bufferAlloc, dsGLRenderPass);
	DS_ASSERT(renderPass);

	dsRenderPass* baseRenderPass = (dsRenderPass*)renderPass;
	baseRenderPass->renderer = renderer;
	baseRenderPass->allocator = dsAllocator_keepPointer(allocator);

	if (attachmentCount > 0)
	{
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, dsAttachmentInfo, attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments,
			sizeof(dsAttachmentInfo)*attachmentCount);

		// Find the first subpass that cleared attachments appear in.
		renderPass->clearSubpass = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, uint32_t, attachmentCount);
		memset(renderPass->clearSubpass, 0xFF, sizeof(uint32_t)*attachmentCount);
		for (uint32_t i = 0; i < subpassCount; ++i)
		{
			const dsRenderSubpassInfo* subpass = subpasses + i;
			for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
			{
				uint32_t attachmentIndex = subpass->colorAttachments[j].attachmentIndex;
				if (attachmentIndex == DS_NO_ATTACHMENT)
					continue;

				const dsAttachmentInfo* attachment = attachments + attachmentIndex;
				if (!(attachment->usage & dsAttachmentUsage_Clear))
					continue;

				if (renderPass->clearSubpass[attachmentIndex] == DS_NO_ATTACHMENT)
					renderPass->clearSubpass[attachmentIndex] = i;
			}

			uint32_t depthStencilAttachment = subpass->depthStencilAttachment.attachmentIndex;
			if (depthStencilAttachment != DS_NO_ATTACHMENT)
			{
				const dsAttachmentInfo* attachment = attachments + depthStencilAttachment;
				if ((attachment->usage & dsAttachmentUsage_Clear) &&
					renderPass->clearSubpass[depthStencilAttachment] == DS_NO_ATTACHMENT)
				{
					renderPass->clearSubpass[depthStencilAttachment] = i;
				}
			}
		}
	}
	else
	{
		baseRenderPass->attachments = NULL;
		renderPass->clearSubpass = NULL;
	}

	dsRenderSubpassInfo* subpassesCopy = DS_ALLOCATE_OBJECT_ARRAY(
		&bufferAlloc, dsRenderSubpassInfo, subpassCount);
	DS_ASSERT(subpassesCopy);
	memcpy(subpassesCopy, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);
	baseRenderPass->subpasses = subpassesCopy;
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		const dsRenderSubpassInfo* origSubpass = subpasses + i;
		dsRenderSubpassInfo* subpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;
		if (subpass->inputAttachmentCount > 0)
		{
			subpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY(
				&bufferAlloc, uint32_t, subpass->inputAttachmentCount);
			DS_ASSERT(subpass->inputAttachments);
			memcpy((void*)subpass->inputAttachments, origSubpass->inputAttachments,
				sizeof(uint32_t)*subpass->inputAttachmentCount);
		}

		if (subpass->colorAttachmentCount > 0)
		{
			subpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
				dsAttachmentRef, subpass->colorAttachmentCount);
			DS_ASSERT(subpass->colorAttachments);
			memcpy((void*)subpass->colorAttachments, origSubpass->colorAttachments,
				sizeof(dsAttachmentRef)*subpass->colorAttachmentCount);
		}

		size_t nameLen = strlen(origSubpass->name) + 1;
		subpass->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(subpass->name);
		memcpy((void*)subpass->name, origSubpass->name, nameLen);
	}

	if (dependencyCount > 0)
	{
		baseRenderPass->subpassDependencies = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, dsSubpassDependency, finalDependencyCount);
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
	}
	else
		baseRenderPass->subpassDependencies = NULL;

	baseRenderPass->attachmentCount = attachmentCount;
	baseRenderPass->subpassCount = subpassCount;
	baseRenderPass->subpassDependencyCount = finalDependencyCount;

	dsGLResource_initialize(&renderPass->resource);
	return baseRenderPass;
}

bool dsGLRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsAlignedBox2f* scissor,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount, bool secondary)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(framebuffer);
	DS_UNUSED(secondary);

	return dsGLCommandBuffer_beginRenderPass(commandBuffer, renderPass, framebuffer, viewport,
		scissor, clearValues, clearValueCount);
}

bool dsGLRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index, bool secondary)
{
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(renderPass);
	DS_UNUSED(secondary);

	return dsGLCommandBuffer_nextRenderSubpass(commandBuffer, renderPass, index);
}

bool dsGLRenderPass_end(
	dsRenderer* renderer, dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass)
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
