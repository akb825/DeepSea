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

#include "Resources/MTLGfxBuffer.h"

#include "Resources/MTLGfxBufferData.h"
#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

#import <Metal/MTLBlitCommandEncoder.h>

dsGfxBuffer* dsMTLGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
{
	@autoreleasepool
	{
		DS_ASSERT(resourceManager);
		DS_ASSERT(allocator);

		dsMTLGfxBuffer* buffer = DS_ALLOCATE_OBJECT(allocator, dsMTLGfxBuffer);
		if (!buffer)
			return NULL;

		dsGfxBuffer* baseBuffer = (dsGfxBuffer*)buffer;
		baseBuffer->resourceManager = resourceManager;
		baseBuffer->allocator = dsAllocator_keepPointer(allocator);
		baseBuffer->usage = usage;
		baseBuffer->memoryHints = memoryHints;
		baseBuffer->size = size;

		buffer->bufferData = dsMTLGfxBufferData_create(resourceManager, allocator,
			resourceManager->allocator, usage, memoryHints, data, size);
		if (!buffer->bufferData)
		{
			if (baseBuffer->allocator)
				dsAllocator_free(baseBuffer->allocator, buffer);
			return NULL;
		}

		DS_VERIFY(dsSpinlock_initialize(&buffer->lock));
		return baseBuffer;
	}
}

void* dsMTLGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size)
{
	@autoreleasepool
	{
		dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;
		dsRenderer* renderer = resourceManager->renderer;

		DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));

		dsMTLGfxBufferData* bufferData = mtlBuffer->bufferData;

		if (bufferData->mappedSize > 0)
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Buffer is already mapped.");
			return NULL;
		}

		// Orphan the data if requested and not previously used.
		if ((flags & dsGfxBufferMap_Orphan) && bufferData->used)
		{
			dsMTLGfxBufferData* newBufferData = dsMTLGfxBufferData_create(resourceManager,
					buffer->allocator, resourceManager->allocator, buffer->usage,
					buffer->memoryHints, NULL, buffer->size);
			if (!newBufferData)
			{
				DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
				return NULL;
			}

			// Delete the previous buffer data and replace with the new one.
			mtlBuffer->bufferData = newBufferData;
			dsMTLGfxBufferData_destroy(bufferData);
			bufferData = newBufferData;
		}

		bufferData->mappedStart = offset;
		bufferData->mappedSize = size;
		bufferData->mappedWrite = (flags & dsGfxBufferMap_Write) &&
			!(flags & dsGfxBufferMap_Persistent);
		uint64_t lastUsedSubmit;
		DS_ATOMIC_LOAD64(&bufferData->lastUsedSubmit, &lastUsedSubmit);

		// Wait for the submitted command to be finished when synchronized.
		if ((buffer->memoryHints & dsGfxMemory_Synchronize) && lastUsedSubmit != DS_NOT_SUBMITTED)
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
			dsGfxFenceResult fenceResult = dsMTLRenderer_waitForSubmit(renderer, lastUsedSubmit,
				DS_DEFAULT_WAIT_TIMEOUT);

			DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));
			if (fenceResult == dsGfxFenceResult_WaitingToQueue)
			{
				bufferData->mappedStart = 0;
				bufferData->mappedSize = 0;
				bufferData->mappedWrite = false;
				DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));

				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Buffer still queued to be rendered.");
				return NULL;
			}

			if (bufferData != mtlBuffer->bufferData || bufferData->mappedSize == 0)
			{
				DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Buffer was unlocked while waiting.");
				return NULL;
			}
		}

		id<MTLBuffer> realMTLBuffer = (__bridge id<MTLBuffer>)(bufferData->mtlBuffer);
		uint8_t* memory = (uint8_t*)realMTLBuffer.contents;
		if (!memory)
		{
			bufferData->mappedStart = 0;
			bufferData->mappedSize = 0;
			bufferData->mappedWrite = false;
			DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
			errno = EPERM;
			return NULL;
		}

		DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
		return memory + offset;
	}
}

bool dsMTLGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;

		DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));

		dsMTLGfxBufferData* bufferData = mtlBuffer->bufferData;

		if (bufferData->mappedSize == 0)
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Buffer isn't mapped.");
			return false;
		}

		// Need to mark the range as dirty to copy to the GPU when next used.
#if DS_MAC
		if (bufferData->mappedWrite && bufferData->managed)
		{
			id<MTLBuffer> realMTLBuffer = (__bridge id<MTLBuffer>)(bufferData->mtlBuffer);
			size_t mappedSize = dsMin(bufferData->mappedSize, buffer->size -
				bufferData->mappedStart);
			NSRange range = {bufferData->mappedStart, mappedSize};
			[realMTLBuffer didModifyRange: range];
		}
#endif

		bufferData->mappedStart = 0;
		bufferData->mappedSize = 0;
		bufferData->mappedWrite = false;
		DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));

		return true;
	}
}

bool dsMTLGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;

		DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));
		dsMTLGfxBufferData* bufferData = mtlBuffer->bufferData;

		id<MTLBuffer> realMTLBuffer = (__bridge id<MTLBuffer>)(bufferData->mtlBuffer);
		NSRange range = {offset, size};
		[realMTLBuffer didModifyRange: range];

		DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
		return true;
	}
}

bool dsMTLGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);
	return true;
}

bool dsMTLGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		id<MTLBuffer> realBuffer = dsMTLGfxBuffer_getBuffer(buffer, commandBuffer);
		if (!realBuffer)
			return false;

		return dsMTLCommandBuffer_copyBufferData(commandBuffer, realBuffer, offset, data, size);
	}
}

bool dsMTLGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		id<MTLBuffer> realSrcBuffer = dsMTLGfxBuffer_getBuffer(srcBuffer, commandBuffer);
		if (!realSrcBuffer)
			return false;

		id<MTLBuffer> realDstBuffer = dsMTLGfxBuffer_getBuffer(dstBuffer, commandBuffer);
		if (!realDstBuffer)
			return false;

		return dsMTLCommandBuffer_copyBuffer(commandBuffer, realSrcBuffer, srcOffset, realDstBuffer,
			dstOffset, size);
	}
}

void dsMTLGfxBuffer_process(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;
	DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));
	dsMTLGfxBufferData_process(mtlBuffer->bufferData, resourceManager->renderer);
	DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));
}

bool dsMTLGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;
	dsMTLGfxBufferData_destroy(mtlBuffer->bufferData);
	dsSpinlock_shutdown(&mtlBuffer->lock);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
	return true;
}

dsMTLGfxBufferData* dsMTLGfxBuffer_getData(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer)
{
	dsMTLGfxBuffer* mtlBuffer = (dsMTLGfxBuffer*)buffer;

	DS_VERIFY(dsSpinlock_lock(&mtlBuffer->lock));

	dsMTLGfxBufferData* bufferData = mtlBuffer->bufferData;
	DS_VERIFY(dsLifetime_acquire(bufferData->lifetime) == bufferData);
	if (buffer->memoryHints & dsGfxMemory_Synchronize)
		dsMTLCommandBuffer_addGfxBuffer(commandBuffer, bufferData);
	dsMTLGfxBufferData_process(bufferData, commandBuffer->renderer);

	DS_VERIFY(dsSpinlock_unlock(&mtlBuffer->lock));

	return bufferData;
}

id<MTLBuffer> dsMTLGfxBuffer_getBuffer(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer)
{
	dsMTLGfxBufferData* bufferData = dsMTLGfxBuffer_getData(buffer, commandBuffer);
	if (!bufferData)
		return nil;

	dsMTLGfxBufferData_markAsUsed(bufferData);
	id<MTLBuffer> realBuffer = (__bridge id<MTLBuffer>)bufferData->mtlBuffer;
	dsLifetime_release(bufferData->lifetime);
	return realBuffer;
}
