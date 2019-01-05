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

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkResource.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

dsGfxBuffer* dsVkGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsVkGfxBuffer* buffer = DS_ALLOCATE_OBJECT(allocator, dsVkGfxBuffer);
	if (!buffer)
		return NULL;

	dsGfxBuffer* baseBuffer = (dsGfxBuffer*)buffer;
	baseBuffer->resourceManager = resourceManager;
	baseBuffer->allocator = dsAllocator_keepPointer(allocator);
	baseBuffer->usage = usage;
	baseBuffer->memoryHints = memoryHints;
	baseBuffer->size = size;

	buffer->bufferData = dsVkGfxBufferData_create(resourceManager, allocator,
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

void* dsVkGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;

	DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));
	if (bufferData->mappedSize > 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer is already mapped.");
		return NULL;
	}

	if (!bufferData->keepHost)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be mapped.");
		return NULL;
	}

	// Orphan the data if invalidated.
	if ((flags & dsGfxBufferMap_Invalidate) && bufferData->used)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		dsVkGfxBufferData* newBufferData = dsVkGfxBufferData_create(resourceManager,
			buffer->allocator, resourceManager->allocator, buffer->usage, buffer->memoryHints, NULL,
			buffer->size);
		if (!newBufferData)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
			return NULL;
		}

		// Delete the previous buffer data and replace with the new one.
		vkBuffer->bufferData = bufferData = newBufferData;
		dsVkRenderer_deleteGfxBuffer(renderer, bufferData);
		bufferData = newBufferData;
		DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));
		DS_ASSERT(bufferData->keepHost);
		DS_ASSERT(bufferData->hostMemory);
	}

	bufferData->mappedStart = offset;
	bufferData->mappedSize = size;
	bufferData->mappedWrite = (flags & dsGfxBufferMap_Write) != 0;
	uint64_t lastUsedSubmit = bufferData->resource.lastUsedSubmit;

	// Wait for the submitted command to be finished when reading.
	if ((flags & dsGfxBufferMap_Read) && (buffer->memoryHints & dsGfxMemory_Synchronize) &&
		lastUsedSubmit != DS_NOT_SUBMITTED)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));

		dsGfxFenceResult fenceResult = dsVkRenderer_waitForSubmit(renderer, lastUsedSubmit,
			DS_DEFAULT_WAIT_TIMEOUT);

		DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));

		if (fenceResult == dsGfxFenceResult_WaitingToQueue)
		{
			bufferData->mappedStart = 0;
			bufferData->mappedSize = 0;
			bufferData->mappedWrite = false;
			DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
			DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer still queued to be rendered.");
			return NULL;
		}

		if (bufferData->mappedSize == 0)
		{
			DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
			DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer was unlocked while waiting.");
			return NULL;
		}
	}

	DS_ASSERT(bufferData->hostMemory);
	void* memory = NULL;
	VkResult result = DS_VK_CALL(device->vkMapMemory)(device->device, bufferData->hostMemory,
		offset, size, 0, &memory);
	if (!dsHandleVkResult(result))
	{
		bufferData->mappedStart = 0;
		bufferData->mappedSize = 0;
		bufferData->mappedWrite = false;
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		return NULL;
	}

	DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
	return memory;
}

bool dsVkGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;

	DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));
	if (bufferData->mappedSize == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer isn't mapped.");
		return false;
	}

	// Need to mark the range as dirty to copy to the GPU when next used.
	if (bufferData->mappedWrite && bufferData->deviceMemory && !bufferData->needsInitialCopy)
	{
		uint32_t rangeIndex = bufferData->dirtyRangeCount;
		if (DS_RESIZEABLE_ARRAY_ADD(bufferData->scratchAllocator, bufferData->dirtyRanges,
			bufferData->dirtyRangeCount, bufferData->maxDirtyRanges, 1))
		{
			bufferData->dirtyRanges[rangeIndex].start = bufferData->mappedStart;
			bufferData->dirtyRanges[rangeIndex].size = bufferData->mappedSize;
		}
	}

	DS_VK_CALL(device->vkUnmapMemory)(device->device, bufferData->hostMemory);

	bufferData->mappedStart = 0;
	bufferData->mappedSize = 0;
	bufferData->mappedWrite = false;
	DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

	return true;
}

bool dsVkGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;

	if (!bufferData->keepHost)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be flushed.");
		return false;
	}

	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

	VkMappedMemoryRange range =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		NULL,
		bufferData->hostMemory,
		offset,
		size
	};
	VkResult result = DS_VK_CALL(device->vkFlushMappedMemoryRanges)(device->device, 1, &range);
	return dsHandleVkResult(result);
}

bool dsVkGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;

	if (!bufferData->keepHost)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be flushed.");
		return false;
	}

	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

	VkMappedMemoryRange range =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		NULL,
		bufferData->hostMemory,
		offset,
		size
	};
	VkResult result = DS_VK_CALL(device->vkInvalidateMappedMemoryRanges)(device->device, 1, &range);
	return dsHandleVkResult(result);
}

bool dsVkGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
	if (!bufferData)
		return false;

	dsVkRenderer_processGfxBuffer(renderer, bufferData);
	VkBuffer dstBuffer = dsVkGfxBufferData_getBuffer(bufferData);

	const size_t maxSize = 65536;
	for (size_t block = 0; block < size; block += maxSize)
	{
		size_t copySize = dsMin(maxSize, size - block);
		DS_VK_CALL(device->vkCmdUpdateBuffer)(vkCommandBuffer, dstBuffer, offset + block, copySize,
			(const uint8_t*)data + block);
	}

	return true;
}

void dsVkGfxBuffer_process(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;
	// Make sure it's not destroyed before we can process it.
	dsLifetime* lifetime = bufferData->lifetime;
	DS_VERIFY(dsLifetime_acquire(lifetime));

	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

	dsVkRenderer_processGfxBuffer(resourceManager->renderer, bufferData);
	dsLifetime_release(lifetime);
}

bool dsVkGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkGfxBufferData* srcBufferData = dsVkGfxBuffer_getData(srcBuffer, commandBuffer);
	if (!srcBufferData)
		return false;

	dsVkGfxBufferData* dstBufferData = dsVkGfxBuffer_getData(dstBuffer, commandBuffer);
	if (!dstBufferData)
		return false;

	VkBuffer srcCopyBuffer = dsVkGfxBufferData_getBuffer(srcBufferData);
	VkBuffer dstCopyBuffer = dsVkGfxBufferData_getBuffer(dstBufferData);

	if (!dsVkGfxBufferData_isStatic(srcBufferData))
	{
		bool canMap = dsVkGfxBufferData_canMap(srcBufferData);
		VkBufferMemoryBarrier barrier =
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			dsVkSrcBufferAccessFlags(srcBufferData->usage, canMap),
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			srcCopyBuffer,
			srcOffset,
			size
		};
		DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer,
			dsVkSrcBufferStageFlags(srcBufferData->usage, canMap), VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, NULL, 1, &barrier, 0, NULL);
	}

	VkBufferCopy bufferCopy = {srcOffset, dstOffset, size};
	DS_VK_CALL(device->vkCmdCopyBuffer)(vkCommandBuffer, srcCopyBuffer, dstCopyBuffer, 1,
		&bufferCopy);
	return true;
}

bool dsVkGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsVkRenderer_deleteGfxBuffer(resourceManager->renderer, vkBuffer->bufferData);
	dsSpinlock_shutdown(&vkBuffer->lock);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
	return true;
}

dsVkGfxBufferData* dsVkGfxBuffer_getData(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;

	DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));

	dsVkGfxBufferData* bufferData = vkBuffer->bufferData;
	if (!dsVkCommandBuffer_addResource(commandBuffer, &bufferData->resource))
		bufferData = NULL;

	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

	return bufferData;
}
