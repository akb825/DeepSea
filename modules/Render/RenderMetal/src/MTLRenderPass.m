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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

dsRenderPass* dsMTLRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	DS_UNUSED(dependencies);
	DS_UNUSED(dependencyCount);
	size_t attachmentArraySize = sizeof(dsAttachmentInfo)*attachmentCount;
	size_t subpassArraySize = sizeof(dsRenderSubpassInfo)*subpassCount;
	size_t clearSubpassArraySize = sizeof(uint32_t)*attachmentCount;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsMTLRenderPass)) +
		DS_ALIGNED_SIZE(attachmentArraySize) + DS_ALIGNED_SIZE(subpassArraySize) +
		DS_ALIGNED_SIZE(clearSubpassArraySize);
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
	dsMTLRenderPass* renderPass = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsMTLRenderPass);
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
		baseRenderPass->attachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsAttachmentInfo, attachmentCount);
		DS_ASSERT(baseRenderPass->attachments);
		memcpy((void*)baseRenderPass->attachments, attachments, attachmentArraySize);
	}
	else
		baseRenderPass->attachments = NULL;

	dsRenderSubpassInfo* subpassesCopy = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		dsRenderSubpassInfo, subpassCount);
	DS_ASSERT(subpassesCopy);
	memcpy(subpassesCopy, subpasses, subpassArraySize);
	baseRenderPass->subpasses = subpassesCopy;
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		dsRenderSubpassInfo* curSubpass = (dsRenderSubpassInfo*)baseRenderPass->subpasses + i;
		if (curSubpass->inputAttachmentCount > 0)
		{
			curSubpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				uint32_t, curSubpass->inputAttachmentCount);
			DS_ASSERT(curSubpass->inputAttachments);
			memcpy((void*)curSubpass->inputAttachments, subpasses[i].inputAttachments,
				sizeof(uint32_t)*curSubpass->inputAttachmentCount);
		}

		if (curSubpass->colorAttachmentCount > 0)
		{
			curSubpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				dsColorAttachmentRef, curSubpass->colorAttachmentCount);
			DS_ASSERT(curSubpass->colorAttachments);
			memcpy((void*)curSubpass->colorAttachments, subpasses[i].colorAttachments,
				sizeof(dsColorAttachmentRef)*curSubpass->colorAttachmentCount);
		}
	}

	baseRenderPass->subpassDependencies = NULL;

	baseRenderPass->attachmentCount = attachmentCount;
	baseRenderPass->subpassCount = subpassCount;
	baseRenderPass->subpassDependencyCount = 0;

	renderPass->scratchAllocator = renderer->allocator;
	renderPass->usedShaders = NULL;
	renderPass->usedShaderCount = 0;
	renderPass->maxUsedShaders = 0;

	DS_VERIFY(dsSpinlock_initialize(&renderPass->shaderLock));

	return baseRenderPass;
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
	DS_VERIFY(dsAllocator_free(renderPass->allocator, usedShaders));
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
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlRenderPass->usedShaders,
				mtlRenderPass->usedShaderCount, i, 1));
			dsLifetime_freeRef(shaderLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderPass->shaderLock));
}
