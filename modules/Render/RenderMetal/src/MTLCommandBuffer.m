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

#include "MTLCommandBuffer.h"

#include "MTLRendererInternal.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

#import <Metal/MTLCommandQueue.h>

void dsMTLCommandBuffer_initialize(dsMTLCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	DS_ASSERT(allocator->freeFunc);

	memset(commandBuffer, 0, sizeof(dsMTLCommandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;
}

id<MTLCommandBuffer> dsMTLCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
		return (__bridge id<MTLCommandBuffer>)(mtlCommandBuffer);

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)commandBuffer->renderer;
	id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)mtlRenderer->commandQueue;
	id<MTLCommandBuffer> newCommandBuffer = [commandQueue commandBuffer];
	if (!newCommandBuffer)
	{
		errno = ENOMEM;
		return nil;
	}

	uint32_t index = mtlCommandBuffer->submitBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
			mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers, 1))
	{
		return nil;
	}

	mtlCommandBuffer->mtlCommandBuffer = CFBridgingRetain(newCommandBuffer);
	mtlCommandBuffer->submitBuffers[index] = CFBridgingRetain(newCommandBuffer);
	return newCommandBuffer;
}

id<MTLBlitCommandEncoder> dsMTLCommandBuffer_getBlitCommandEncoder(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->blitCommandEncoder)
		return (__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = dsMTLCommandBuffer_getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLCommandBuffer_endEncoding(commandBuffer);

	id<MTLBlitCommandEncoder> encoder = [realCommandBuffer blitCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->blitCommandEncoder = CFBridgingRetain(encoder);
	return encoder;
}

id<MTLComputeCommandEncoder> dsMTLCommandBuffer_getComputeCommandEncoder(
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->computeCommandEncoder)
		return (__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = dsMTLCommandBuffer_getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLCommandBuffer_endEncoding(commandBuffer);

	id<MTLComputeCommandEncoder> encoder = [realCommandBuffer computeCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->computeCommandEncoder = CFBridgingRetain(encoder);
	return encoder;
}

void dsMTLCommandBuffer_endEncoding(dsCommandBuffer* commandBuffer)
{
	// Render encoder is fully managed by render passes.
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	DS_ASSERT(!mtlCommandBuffer->renderCommandEncoder);

	if (mtlCommandBuffer->blitCommandEncoder)
	{
		id<MTLBlitCommandEncoder> encoder =
			(__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->blitCommandEncoder);
		mtlCommandBuffer->blitCommandEncoder = NULL;
	}

	if (mtlCommandBuffer->computeCommandEncoder)
	{
		id<MTLComputeCommandEncoder> encoder =
			(__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->computeCommandEncoder);
		mtlCommandBuffer->computeCommandEncoder = NULL;
	}
}

bool dsMTLCommandBuffer_addGfxBuffer(dsCommandBuffer* commandBuffer, dsMTLGfxBufferData* buffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, mtlCommandBuffer->gfxBufferCount);
	for (uint32_t i = mtlCommandBuffer->gfxBufferCount - checkCount;
		i < mtlCommandBuffer->gfxBufferCount; ++i)
	{
		if (mtlCommandBuffer->gfxBuffers[i] == buffer->lifetime)
			return true;
	}

	uint32_t index = mtlCommandBuffer->gfxBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->gfxBuffers,
			mtlCommandBuffer->gfxBufferCount, mtlCommandBuffer->maxGfxBuffers, 1))
	{
		return false;
	}

	mtlCommandBuffer->gfxBuffers[index] = dsLifetime_addRef(buffer->lifetime);
	return true;
}

bool dsMTLCommandBuffer_addFence(dsCommandBuffer* commandBuffer, dsGfxFence* fence)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	dsMTLGfxFence* mtlFence = (dsMTLGfxFence*)fence;

	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, mtlCommandBuffer->fenceCount);
	for (uint32_t i = mtlCommandBuffer->fenceCount - checkCount;
		i < mtlCommandBuffer->fenceCount; ++i)
	{
		if (mtlCommandBuffer->fences[i] == mtlFence->lifetime)
			return true;
	}

	uint32_t index = mtlCommandBuffer->fenceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->fences,
			mtlCommandBuffer->fenceCount, mtlCommandBuffer->maxFences, 1))
	{
		return false;
	}

	mtlCommandBuffer->fences[index] = dsLifetime_addRef(mtlFence->lifetime);
	return true;
}

void dsMTLCommandBuffer_submitFence(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;

	// Process immediately for the main command buffer if not in a render pass.
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer &&
		!commandBuffer->boundRenderPass)
	{
		dsMTLRenderer_flush(commandBuffer->renderer);
		mtlCommandBuffer->fenceSet = false;
		return;
	}

	mtlCommandBuffer->fenceSet = true;
}

void dsMTLCommandBuffer_clear(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlCommandBuffer->submitBuffers[i])
			CFRelease(mtlCommandBuffer->submitBuffers[i]);
	}
	mtlCommandBuffer->submitBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->gfxBuffers[i]);
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->fences[i]);
	mtlCommandBuffer->fenceCount = 0;
}

void dsMTLCommandBuffer_submitted(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlCommandBuffer->submitBuffers[i])
			CFRelease(mtlCommandBuffer->submitBuffers[i]);
	}
	mtlCommandBuffer->submitBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->gfxBuffers[i];
		dsMTLGfxBufferData* bufferData = (dsMTLGfxBufferData*)dsLifetime_acquire(lifetime);
		if (bufferData)
		{
			DS_ATOMIC_STORE64(&bufferData->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->fences[i];
		dsMTLGfxFence* fence = (dsMTLGfxFence*)dsLifetime_acquire(lifetime);
		if (fence)
		{
			DS_ATOMIC_STORE64(&fence->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->fenceCount = 0;
}

void dsMTLCommandBuffer_shutdown(dsMTLCommandBuffer* commandBuffer)
{
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	// Not initialized yet.
	if (!allocator)
		return;

	if (commandBuffer->mtlCommandBuffer)
		CFRelease(commandBuffer->mtlCommandBuffer);
	if (commandBuffer->renderCommandEncoder)
		CFRelease(commandBuffer->renderCommandEncoder);
	if (commandBuffer->blitCommandEncoder)
		CFRelease(commandBuffer->blitCommandEncoder);
	if (commandBuffer->computeCommandEncoder)
		CFRelease(commandBuffer->computeCommandEncoder);

	for (uint32_t i = 0; i < commandBuffer->submitBufferCount; ++i)
	{
		if (commandBuffer->submitBuffers[i])
			CFRelease(commandBuffer->submitBuffers[i]);
	}
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->submitBuffers));

	for (uint32_t i = 0; i < commandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(commandBuffer->gfxBuffers[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->gfxBuffers));

	for (uint32_t i = 0; i < commandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(commandBuffer->fences[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fences));

	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->secondaryCommands));
}
