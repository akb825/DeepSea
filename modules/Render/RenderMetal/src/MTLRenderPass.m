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

#include "MTLRenderPass.h"

#include "Resources/MTLShader.h"
#include "MTLCommandBuffer.h"
#include "MTLShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/RenderPass.h>
#include <string.h>

#import <QuartzCore/CAMetalLayer.h>

static size_t fullAllocSize(uint32_t attachmentCount, const dsRenderSubpassInfo* subpasses,
	uint32_t subpassCount, uint32_t dependencyCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsMTLRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsAttachmentInfo)*attachmentCount) +
		DS_ALIGNED_SIZE(sizeof(dsRenderSubpassInfo)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(dsMTLSubpassInfo)*subpassCount) +
		DS_ALIGNED_SIZE(sizeof(dsSubpassDependency)*dependencyCount);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*subpasses[i].inputAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsAttachmentRef)*subpasses[i].colorAttachmentCount) +
			DS_ALIGNED_SIZE(sizeof(dsMTLAttachmentInfo)*subpasses[i].colorAttachmentCount) +
			DS_ALIGNED_SIZE(strlen(subpasses[i].name) + 1);
	}
	return fullSize;
}

static bool subpassHasAttachment(const dsRenderSubpassInfo* subpass, uint32_t attachment)
{
	if (subpass->depthStencilAttachment.attachmentIndex == attachment)
		return true;

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		if (subpass->colorAttachments[i].attachmentIndex == attachment)
			return true;
	}

	return false;
}

static bool hasUsageBefore(uint32_t attachment, uint32_t subpass,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount)
{
	DS_UNUSED(subpassCount);
	for (uint32_t i = 0; i < subpass; ++i)
	{
		if (subpassHasAttachment(subpasses + i, attachment))
			return true;
	}

	return false;
}

static bool hasUsageAfter(uint32_t attachment, uint32_t subpass,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount)
{
	for (uint32_t i = subpass + 1; i < subpassCount; ++i)
	{
		if (subpassHasAttachment(subpasses + i, attachment))
			return true;
	}

	return false;
}

static MTLLoadAction getLoadAction(uint32_t attachment, uint32_t subpass,
	const dsAttachmentInfo* attachments, const dsRenderSubpassInfo* subpasses,
	uint32_t subpassCount)
{
	if (attachment == DS_NO_ATTACHMENT)
		return MTLLoadActionDontCare;

	if (hasUsageBefore(attachment, subpass, subpasses, subpassCount))
		return MTLLoadActionLoad;

	dsAttachmentUsage usage = attachments[attachment].usage;
	if (usage & dsAttachmentUsage_Clear)
		return MTLLoadActionClear;
	else if (usage & dsAttachmentUsage_KeepBefore)
		return MTLLoadActionLoad;
	return MTLLoadActionDontCare;
}

static MTLStoreAction getStoreAction(uint32_t attachment, uint32_t subpass,
	const dsAttachmentInfo* attachments, const dsRenderSubpassInfo* subpasses,
	uint32_t subpassCount)
{
	if (attachment == DS_NO_ATTACHMENT)
		return MTLStoreActionDontCare;

	dsAttachmentUsage usage = attachments[attachment].usage;
	if (usage & (dsAttachmentUsage_KeepAfter | dsAttachmentUsage_UseLater) ||
		hasUsageAfter(attachment, subpass, subpasses, subpassCount))
	{
		return MTLStoreActionStore;
	}
	return MTLStoreActionDontCare;
}

static void setAttachmentSurface(MTLRenderPassAttachmentDescriptor* descriptor,
	const dsFramebufferSurface* surface, const dsMTLAttachmentInfo* info, bool resolve,
	bool stencil)
{
	if (info->storeAction == MTLStoreActionMultisampleResolve)
		resolve = true;

	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		{
			DS_ASSERT(!stencil);
			const dsMTLRenderSurface* renderSurface = (const dsMTLRenderSurface*)surface->surface;
			id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)renderSurface->drawable;
			if (renderSurface->resolveSurface)
			{
				descriptor.texture = (__bridge id<MTLTexture>)renderSurface->resolveSurface;
				if (resolve)
					descriptor.resolveTexture = drawable.texture;
			}
			else
				descriptor.texture = drawable.texture;
			break;
		}
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
		{
			const dsMTLRenderSurface* renderSurface = (const dsMTLRenderSurface*)surface->surface;
			if (stencil)
				descriptor.texture = (__bridge id<MTLTexture>)renderSurface->stencilSurface;
			else
				descriptor.texture = (__bridge id<MTLTexture>)renderSurface->depthSurface;
			break;
		}
		case dsGfxSurfaceType_Offscreen:
		{
			const dsOffscreen* offscreen = (const dsOffscreen*)surface->surface;
			const dsMTLTexture* mtlTexture = (const dsMTLTexture*)offscreen;
			if (mtlTexture->resolveTexture || mtlTexture->resolveStencilTexture)
			{
				if (stencil)
				{
					descriptor.texture = (__bridge id<MTLTexture>)mtlTexture->resolveStencilTexture;
					if (resolve)
					{
						descriptor.resolveTexture =
							(__bridge id<MTLTexture>)mtlTexture->stencilTexture;
					}
				}
				else
				{
					descriptor.texture = (__bridge id<MTLTexture>)mtlTexture->resolveTexture;
					if (resolve)
						descriptor.resolveTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
				}
			}
			else
			{
				uint32_t faceCount = offscreen->info.dimension == dsTextureDim_Cube ? 6 : 1;
				descriptor.level = surface->mipLevel;
				if (offscreen->info.dimension == dsTextureDim_3D)
					descriptor.depthPlane = surface->layer;
				else
					descriptor.slice = surface->layer*faceCount + surface->cubeFace;

				if (stencil)
					descriptor.texture = (__bridge id<MTLTexture>)mtlTexture->stencilTexture;
				else
					descriptor.texture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
			}
			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			const dsMTLRenderbuffer* renderbuffer = (const dsMTLRenderbuffer*)surface->surface;
			if (stencil)
				descriptor.texture = (__bridge id<MTLTexture>)renderbuffer->stencilSurface;
			else
				descriptor.texture = (__bridge id<MTLTexture>)renderbuffer->surface;
			break;
		}
		default:
			DS_ASSERT(false);
			break;
	}

	descriptor.loadAction = info->loadAction;
	if (descriptor.resolveTexture)
		descriptor.storeAction = MTLStoreActionMultisampleResolve;
	else
		descriptor.storeAction = info->storeAction;
}

static MTLRenderPassDescriptor* createRenderPassDescriptor(const dsRenderPass* renderPass,
	uint32_t subpassIndex, const dsFramebuffer* framebuffer, const dsCommandBuffer* commandBuffer)
{
	const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;
	const dsMTLCommandBuffer* mtlCommandBuffer = (const dsMTLCommandBuffer*)commandBuffer;
	MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor new];
	if (!descriptor)
		return NULL;

	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	const dsMTLSubpassInfo* subpassInfo = mtlRenderPass->subpassInfos + subpassIndex;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		const dsAttachmentRef* colorAttachment = subpass->colorAttachments + i;
		if (colorAttachment->attachmentIndex == DS_NO_ATTACHMENT)
			continue;

		MTLRenderPassColorAttachmentDescriptor* colorDescriptor = descriptor.colorAttachments[i];
		if (!colorDescriptor)
			return NULL;

		const dsFramebufferSurface* surface =
			framebuffer->surfaces + colorAttachment->attachmentIndex;
		const dsMTLAttachmentInfo* attachmentInfo = subpassInfo->colorAttachments + i;
		setAttachmentSurface(colorDescriptor, surface, attachmentInfo, colorAttachment->resolve,
			false);
		if (attachmentInfo->loadAction == MTLLoadActionClear)
		{
			dsGfxFormat format = dsFramebuffer_getSurfaceFormat(commandBuffer->renderer, surface);
			DS_ASSERT(colorAttachment->attachmentIndex < mtlCommandBuffer->clearValueCount);
			colorDescriptor.clearColor = dsGetMTLClearColor(format,
				&mtlCommandBuffer->clearValues[colorAttachment->attachmentIndex].colorValue);
		}
	}

	const dsAttachmentRef* depthStencilAttachment = &subpass->depthStencilAttachment;
	if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT)
	{
		const dsFramebufferSurface* surface = framebuffer->surfaces +
			depthStencilAttachment->attachmentIndex;
		dsGfxFormat format = dsFramebuffer_getSurfaceFormat(commandBuffer->renderer, surface);
		if (format == dsGfxFormat_D16 || format == dsGfxFormat_X8D24 ||
			format == dsGfxFormat_D16S8 || format == dsGfxFormat_D24S8 ||
			format == dsGfxFormat_D32S8_Float)
		{
			MTLRenderPassDepthAttachmentDescriptor* depthDescriptor = descriptor.depthAttachment;
			if (!depthDescriptor)
				return NULL;

			setAttachmentSurface(depthDescriptor, surface, &subpassInfo->depthStencilAttachment,
				depthStencilAttachment->resolve, false);
			if (subpassInfo->depthStencilAttachment.loadAction == MTLLoadActionClear)
			{
				DS_ASSERT(depthStencilAttachment->attachmentIndex <
					mtlCommandBuffer->clearValueCount);
				const dsSurfaceClearValue* clearValue =
					mtlCommandBuffer->clearValues + depthStencilAttachment->attachmentIndex;
				depthDescriptor.clearDepth = clearValue->depthStencil.depth;
			}
		}

		if (format == dsGfxFormat_S8 || format == dsGfxFormat_D16S8 ||
			format == dsGfxFormat_D24S8 || format == dsGfxFormat_D32S8_Float)
		{
			MTLRenderPassStencilAttachmentDescriptor* stencilDescriptor =
				descriptor.stencilAttachment;
			if (!stencilDescriptor)
				return NULL;

			setAttachmentSurface(stencilDescriptor, surface, &subpassInfo->depthStencilAttachment,
				false, true);
			if (subpassInfo->depthStencilAttachment.loadAction == MTLLoadActionClear)
			{
				DS_ASSERT(depthStencilAttachment->attachmentIndex <
					mtlCommandBuffer->clearValueCount);
				const dsSurfaceClearValue* clearValue =
					mtlCommandBuffer->clearValues + depthStencilAttachment->attachmentIndex;
				stencilDescriptor.clearStencil = clearValue->depthStencil.stencil;
			}
		}
	}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
	if (framebuffer->layers > 1)
		descriptor.renderTargetArrayLength = framebuffer->layers;
#endif

	return descriptor;
}

static void addReadbackOffscreens(const dsRenderPass* renderPass, uint32_t subpass,
	const dsFramebuffer* framebuffer, dsCommandBuffer* commandBuffer)
{
	const dsRenderSubpassInfo* subpassInfo = renderPass->subpasses + subpass;
	for (uint32_t i = 0; i < subpassInfo->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpassInfo->colorAttachments[i].attachmentIndex;
		if (attachment == DS_NO_ATTACHMENT)
			continue;

		const dsFramebufferSurface* surface = framebuffer->surfaces + attachment;
		if (surface->surfaceType != dsGfxSurfaceType_Offscreen)
			continue;

		dsOffscreen* offscreen = (dsOffscreen*)surface->surface;
		if (!(offscreen->memoryHints & dsGfxMemory_Read))
			continue;

		dsMTLCommandBuffer_addReadbackOffscreen(commandBuffer, offscreen);
	}
}

dsRenderPass* dsMTLRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	@autoreleasepool
	{
		uint32_t finalDependencyCount = dependencyCount;
		if (dependencyCount == 0)
			finalDependencyCount = 0;
		else if (dependencyCount == DS_DEFAULT_SUBPASS_DEPENDENCIES)
			finalDependencyCount = dsRenderPass_countDefaultDependencies(subpasses, subpassCount);

		size_t fullSize = fullAllocSize(attachmentCount, subpasses, subpassCount,
			finalDependencyCount);
		void* buffer = dsAllocator_alloc(allocator, fullSize);
		if (!buffer)
			return NULL;

		dsBufferAllocator bufferAlloc;
		DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
		dsMTLRenderPass* renderPass = DS_ALLOCATE_OBJECT(&bufferAlloc, dsMTLRenderPass);
		DS_ASSERT(renderPass);

		renderPass->lifetime = dsLifetime_create(allocator, renderPass);
		if (!renderPass->lifetime)
		{
			if (allocator->freeFunc)
				DS_VERIFY(dsAllocator_free(allocator, renderPass));
			return NULL;
		}

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
		}
		else
			baseRenderPass->attachments = NULL;

		dsRenderSubpassInfo* subpassesCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
			dsRenderSubpassInfo, subpassCount);
		DS_ASSERT(subpassesCopy);
		memcpy(subpassesCopy, subpasses, sizeof(dsRenderSubpassInfo)*subpassCount);
		baseRenderPass->subpasses = subpassesCopy;
		renderPass->subpassInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMTLSubpassInfo,
			subpassCount);
		DS_ASSERT(renderPass->subpassInfos);
		for (uint32_t i = 0; i < subpassCount; ++i)
		{
			dsRenderSubpassInfo* curSubpass = subpassesCopy + i;
			dsMTLSubpassInfo* curSubpassInfo = renderPass->subpassInfos + i;
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
					dsAttachmentRef, curSubpass->colorAttachmentCount);
				DS_ASSERT(curSubpass->colorAttachments);
				memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
					sizeof(dsAttachmentRef)*curSubpass->colorAttachmentCount);

				curSubpassInfo->colorAttachments =
					DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMTLAttachmentInfo,
						curSubpass->colorAttachmentCount);
				DS_ASSERT(curSubpassInfo->colorAttachments);
				for (uint32_t j = 0; j < curSubpass->colorAttachmentCount; ++j)
				{
					dsMTLAttachmentInfo* attachmentInfo = curSubpassInfo->colorAttachments + j;
					const dsAttachmentRef* colorAttachment = curSubpass->colorAttachments + j;
					attachmentInfo->loadAction = getLoadAction(colorAttachment->attachmentIndex, i,
						attachments, subpasses, subpassCount);
					attachmentInfo->storeAction = getStoreAction(colorAttachment->attachmentIndex,
						i, attachments, subpasses, subpassCount);
				}
			}
			else
				curSubpassInfo->colorAttachments = NULL;

			const dsAttachmentRef* depthStencilAttachment = &curSubpass->depthStencilAttachment;
			curSubpassInfo->depthStencilAttachment.loadAction = getLoadAction(
				depthStencilAttachment->attachmentIndex, i, attachments, subpasses, subpassCount);
			curSubpassInfo->depthStencilAttachment.storeAction = getStoreAction(
				depthStencilAttachment->attachmentIndex, i, attachments, subpasses, subpassCount);

			size_t nameLen = strlen(subpasses[i].name) + 1;
			curSubpass->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			DS_ASSERT(curSubpass->name);
			memcpy((void*)curSubpass->name, subpasses[i].name, nameLen);
		}

		if (dependencyCount > 0)
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
		}
		else
			baseRenderPass->subpassDependencies = NULL;

		baseRenderPass->attachmentCount = attachmentCount;
		baseRenderPass->subpassCount = subpassCount;
		baseRenderPass->subpassDependencyCount = finalDependencyCount;

		renderPass->scratchAllocator = renderer->allocator;
		renderPass->usedShaders = NULL;
		renderPass->usedShaderCount = 0;
		renderPass->maxUsedShaders = 0;

		DS_VERIFY(dsSpinlock_initialize(&renderPass->shaderLock));

		return baseRenderPass;
	}
}

bool dsMTLRenderPass_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool secondary)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		DS_UNUSED(secondary);
		if (!dsMTLCommandBuffer_copyClearValues(commandBuffer, clearValues, clearValueCount))
			return false;

		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		if (viewport)
			mtlCommandBuffer->viewport = *viewport;
		else
		{
			mtlCommandBuffer->viewport.min.x = 0.0f;
			mtlCommandBuffer->viewport.min.y = 0.0f;
			mtlCommandBuffer->viewport.min.z = 0.0f;
			mtlCommandBuffer->viewport.max.x = (float)framebuffer->width;
			mtlCommandBuffer->viewport.max.y = (float)framebuffer->height;
			mtlCommandBuffer->viewport.max.z = 1.0f;
		}

		MTLRenderPassDescriptor* descriptor = createRenderPassDescriptor(renderPass, 0, framebuffer,
			commandBuffer);
		if (!descriptor)
			return false;

		addReadbackOffscreens(renderPass, 0, framebuffer, commandBuffer);
		return dsMTLCommandBuffer_beginRenderPass(commandBuffer, descriptor,
			&mtlCommandBuffer->viewport);
	}
}

bool dsMTLRenderPass_nextSubpass(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index, bool secondary)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		DS_UNUSED(secondary);
		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
		DS_ASSERT(framebuffer);

		MTLRenderPassDescriptor* descriptor = createRenderPassDescriptor(renderPass, index,
			framebuffer, commandBuffer);
		if (!descriptor)
			return false;

		if (!dsMTLCommandBuffer_endRenderPass(commandBuffer))
			return false;

		addReadbackOffscreens(renderPass, index, framebuffer, commandBuffer);
		return dsMTLCommandBuffer_beginRenderPass(commandBuffer, descriptor,
			&mtlCommandBuffer->viewport);
	}
}

bool dsMTLRenderPass_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		DS_UNUSED(renderPass);
		return dsMTLCommandBuffer_endRenderPass(commandBuffer);
	}
}

bool dsMTLRenderPass_destroy(dsRenderer* renderer, dsRenderPass* renderPass)
{
	DS_UNUSED(renderer);
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&mtlRenderPass->shaderLock));
	dsLifetime** usedShaders = mtlRenderPass->usedShaders;
	uint32_t usedShaderCount = mtlRenderPass->usedShaderCount;
	mtlRenderPass->usedShaders = NULL;
	mtlRenderPass->usedShaderCount = 0;
	mtlRenderPass->maxUsedShaders = 0;
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));

	for (uint32_t i = 0; i < usedShaderCount; ++i)
	{
		dsShader* shader = (dsShader*)dsLifetime_acquire(usedShaders[i]);
		if (shader)
		{
			dsMTLShader_removeRenderPass(shader, renderPass);
			dsLifetime_release(usedShaders[i]);
		}
		dsLifetime_freeRef(usedShaders[i]);
	}
	DS_VERIFY(dsAllocator_free(mtlRenderPass->scratchAllocator, usedShaders));
	DS_ASSERT(!mtlRenderPass->usedShaders);

	dsLifetime_destroy(mtlRenderPass->lifetime);

	dsSpinlock_shutdown(&mtlRenderPass->shaderLock);
	if (renderPass->allocator)
		DS_VERIFY(dsAllocator_free(renderPass->allocator, renderPass));
	return true;
}

bool dsMTLRenderPass_addShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderPass->shaderLock));

	for (uint32_t i = 0; i < mtlRenderPass->usedShaderCount; ++i)
	{
		if (mtlRenderPass->usedShaders[i] == mtlShader->lifetime)
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
			return true;
		}
	}

	uint32_t index = mtlRenderPass->usedShaderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(mtlRenderPass->scratchAllocator, mtlRenderPass->usedShaders,
		mtlRenderPass->usedShaderCount, mtlRenderPass->maxUsedShaders, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
		return false;
	}

	mtlRenderPass->usedShaders[index] = dsLifetime_addRef(mtlShader->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
	return true;
}

void dsMTLRenderPass_removeShader(dsRenderPass* renderPass, dsShader* shader)
{
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderPass->shaderLock));
	for (uint32_t i = 0; i < mtlRenderPass->usedShaderCount; ++i)
	{
		dsLifetime* shaderLifetime = mtlRenderPass->usedShaders[i];
		if (shaderLifetime == mtlShader->lifetime)
		{
			mtlRenderPass->usedShaders[i] =
				mtlRenderPass->usedShaders[mtlRenderPass->usedShaderCount - 1];
			--mtlRenderPass->usedShaderCount;
			dsLifetime_freeRef(shaderLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
}
