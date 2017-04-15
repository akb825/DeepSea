/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/RenderMock/MockRenderer.h>

#include "Resources/MockResourceManager.h"
#include "MockCommandBuffer.h"
#include "MockCommandBufferPool.h"
#include "MockRenderPass.h"
#include "MockRenderSurface.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Renderer.h>

dsRenderer* dsMockRenderer_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsRenderer)) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer));
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, totalSize));
	dsRenderer* renderer = (dsRenderer*)dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		sizeof(dsRenderer));
	DS_ASSERT(renderer);

	if (!dsRenderer_initialize(renderer))
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	dsResourceManager* resourceManager = dsMockResourceManager_create(renderer, allocator);
	if (!resourceManager)
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	renderer->allocator = dsAllocator_keepPointer(allocator);
	renderer->resourceManager = resourceManager;

	renderer->mainCommandBuffer = (dsCommandBuffer*)dsAllocator_alloc(
		(dsAllocator*)&bufferAllocator, sizeof(dsCommandBuffer));
	DS_ASSERT(renderer->mainCommandBuffer);
	renderer->mainCommandBuffer->renderer = renderer;

	renderer->maxColorAttachments = 4;
	renderer->maxAnisotropy = 16;

	renderer->surfaceColorFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
	renderer->surfaceDepthStencilFormat = dsGfxFormat_D24S8;
	renderer->surfaceSamples = 4;
	renderer->doubleBuffer = true;

	renderer->createRenderSurfaceFunc = &dsMockRenderSurface_create;
	renderer->destroyRenderSurfaceFunc = &dsMockRenderSurface_destroy;
	renderer->updateRenderSurfaceFunc = &dsMockRenderSurface_update;
	renderer->beginRenderSurfaceFunc = &dsMockRenderSurface_beginDraw;
	renderer->endRenderSurfaceFunc = &dsMockRenderSurface_endDraw;
	renderer->swapRenderSurfaceBuffersFunc = &dsMockRenderSurface_swapBuffers;

	renderer->createCommandBufferPoolFunc = &dsMockCommandBufferPool_create;
	renderer->resetCommandBufferPoolFunc = &dsMockCommandBufferPool_reset;
	renderer->destroyCommandBufferPoolFunc = &dsMockCommandBufferPool_destroy;

	renderer->beginCommandBufferFunc = &dsMockCommandBuffer_begin;
	renderer->endCommandBufferFunc = &dsMockCommandBuffer_end;
	renderer->submitCommandBufferFunc = &dsMockCommandBuffer_submit;

	renderer->createRenderPassFunc = &dsMockRenderPass_create;
	renderer->destroyRenderPassFunc = &dsMockRenderPass_destroy;
	renderer->beginRenderPassFunc = &dsMockRenderPass_begin;
	renderer->nextRenderSubpassFunc = &dsMockRenderPass_nextSubpass;
	renderer->endRenderPassFunc = &dsMockRenderPass_end;

	return renderer;
}

void dsMockRenderer_destroy(dsRenderer* renderer)
{
	if (!renderer)
		return;

	dsMockResourceManager_destroy(renderer->resourceManager);
	dsRenderer_shutdown(renderer);
	if (renderer->allocator)
		dsAllocator_free(renderer->allocator, renderer);
}
