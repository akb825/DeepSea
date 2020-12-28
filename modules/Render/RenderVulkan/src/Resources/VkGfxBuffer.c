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
#include "Resources/VkTexture.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool copyDataCommandBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = commandBuffer->renderer;
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

	bool canMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(bufferData);
	VkBufferMemoryBarrier barrier =
	{
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		dsVkReadBufferAccessFlags(buffer->usage) |
			dsVkWriteBufferAccessFlags(bufferData->usage, canMapMainBuffer),
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dstBuffer,
		offset,
		size
	};
	VkPipelineStageFlags stages = dsVkReadBufferStageFlags(renderer, buffer->usage) |
		dsVkWriteBufferStageFlags(renderer, bufferData->usage, canMapMainBuffer);
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, stages,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1, &barrier, 0, NULL);

	DS_VK_CALL(device->vkCmdUpdateBuffer)(vkCommandBuffer, dstBuffer, offset, size, data);

	barrier.dstAccessMask = barrier.srcAccessMask;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stages, 0, 0, NULL, 1, &barrier, 0, NULL);
	return true;
}

static bool copyDataTempBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
	if (!bufferData)
		return false;

	// You would expect vkCmdUpdateBuffer() to be the proper function to call. However, some drivers
	// (*cough* Qualcomm *cough*) take an obscenely large amount of time to copy even a trivial
	// amount of data. (WHY does it take 1/3 ms to copy 200 bytes?) So do the copy ourselves.
	size_t tempOffset = 0;
	VkBuffer tempBuffer = 0;
	void* tempData =
		dsVkCommandBuffer_getTempData(&tempOffset, &tempBuffer, commandBuffer, size, 4);
	if (!tempData)
		return false;

	memcpy(tempData, data, size);

	dsVkRenderer_processGfxBuffer(renderer, bufferData);
	VkBuffer dstBuffer = dsVkGfxBufferData_getBuffer(bufferData);

	bool canMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(bufferData);
	VkBufferMemoryBarrier barriers[] =
	{
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			dsVkReadBufferAccessFlags(buffer->usage) |
				dsVkWriteBufferAccessFlags(bufferData->usage, canMapMainBuffer),
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			dstBuffer,
			offset,
			size
		},
	{
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_HOST_WRITE_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		tempBuffer,
		tempOffset,
		size
	}
	};
	VkPipelineStageFlags stages = dsVkReadBufferStageFlags(renderer, buffer->usage) |
		dsVkWriteBufferStageFlags(renderer, bufferData->usage, canMapMainBuffer) |
		VK_PIPELINE_STAGE_HOST_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, stages,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 2, barriers, 0, NULL);

	VkBufferCopy copy =
	{
		tempOffset,
		offset,
		size
	};
	DS_VK_CALL(device->vkCmdCopyBuffer)(vkCommandBuffer, tempBuffer, dstBuffer, 1, &copy);

	barriers[0].dstAccessMask = barriers[0].srcAccessMask;
	barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stages, 0, 0, NULL, 1, barriers, 0, NULL);
	return true;
}

static bool addCopyToImageBarriers(dsCommandBuffer* commandBuffer,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount,
	dsVkGfxBufferData* srcBufferData, bool srcCanMap, dsTexture* dstTexture,
	bool reverse)
{
	VkAccessFlags srcAccessFlags = dsVkWriteBufferAccessFlags(srcBufferData->usage, srcCanMap) |
		dsVkReadBufferAccessFlags(srcBufferData->usage);
	VkBuffer srcVkBuffer = dsVkGfxBufferData_getBuffer(srcBufferData);

	dsVkTexture* dstVkTexture = (dsVkTexture*)dstTexture;

	VkImageAspectFlags dstAspectMask = dsVkImageAspectFlags(dstTexture->info.format);
	uint32_t dstFaceCount = dstTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;
	bool dstIsDepthStencil = dsGfxFormat_isDepthStencil(dstTexture->info.format);
	VkAccessFlags dstAccessFlags = dsVkReadImageAccessFlags(dstTexture->usage) |
		dsVkWriteImageAccessFlags(dstTexture->usage, dstTexture->offscreen, dstIsDepthStencil);

	VkImageLayout dstMainLayout = dsVkTexture_imageLayout(dstTexture);

	unsigned int formatSize = dsGfxFormat_size(dstTexture->info.format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, dstTexture->info.format));

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;

		// Buffer barrier.
		VkBufferMemoryBarrier* bufferBarrier =
			dsVkCommandBuffer_addBufferBarrier(commandBuffer);
		if (!bufferBarrier)
			return false;

		bufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier->pNext = NULL;
		if (reverse)
		{
			bufferBarrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			bufferBarrier->dstAccessMask = srcAccessFlags;
		}
		else
		{
			bufferBarrier->srcAccessMask = srcAccessFlags;
			bufferBarrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		bufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier->buffer = srcVkBuffer;
		bufferBarrier->offset = region->bufferOffset;

		uint32_t bufferWidth = region->bufferWidth;
		if (bufferWidth == 0)
			bufferWidth = region->textureWidth;
		uint32_t bufferHeight = region->bufferHeight;
		if (bufferHeight == 0)
			bufferHeight = region->textureHeight;
		size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
		size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
		size_t textureXBlocks = (region->textureWidth + blockX - 1)/blockY;
		size_t remainderBlocks = bufferXBlocks - textureXBlocks;
		bufferBarrier->size =
			((bufferXBlocks*bufferYBlocks*region->layers) - remainderBlocks)*formatSize;

		// Image barrier.
		const dsTexturePosition* dstPosition = &region->texturePosition;
		uint32_t dstLayers, dstBaseLayer;
		if (dstIs3D)
		{
			dstLayers = 1;
			dstBaseLayer = 0;
		}
		else
		{
			dstLayers = region->layers;
			dstBaseLayer = dstPosition->depth*dstFaceCount + dstPosition->face;
		}

		VkImageMemoryBarrier* barrier = dsVkCommandBuffer_addImageBarrier(commandBuffer);
		if (!barrier)
			return false;

		barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier->pNext = NULL;
		if (reverse)
		{
			barrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->dstAccessMask = dstAccessFlags;
			barrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier->newLayout = dstMainLayout;
		}
		else
		{
			barrier->srcAccessMask = dstAccessFlags;
			barrier->dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->oldLayout = dstMainLayout;
			barrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}
		barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier->image = dstVkTexture->deviceImage;
		barrier->subresourceRange.aspectMask = dstAspectMask;
		barrier->subresourceRange.baseMipLevel = dstPosition->mipLevel;
		barrier->subresourceRange.levelCount = 1;
		barrier->subresourceRange.baseArrayLayer = dstBaseLayer;
		barrier->subresourceRange.layerCount = dstLayers;
	}

	return true;
}

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

	// Orphan the data if requested and not previously used.
	if ((flags & dsGfxBufferMap_Orphan) && bufferData->used)
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
		vkBuffer->bufferData = newBufferData;
		dsVkRenderer_deleteGfxBuffer(renderer, bufferData);
		bufferData = newBufferData;
		DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));
		DS_ASSERT(bufferData->keepHost);
		DS_ASSERT(bufferData->hostMemory);
	}

	bufferData->mappedStart = offset;
	bufferData->mappedSize = size;
	bufferData->mappedWrite = (flags & dsGfxBufferMap_Write) &&
		!(flags & dsGfxBufferMap_Persistent);
	uint64_t lastUsedSubmit = bufferData->resource.lastUsedSubmit;

	// Wait for the submitted command to be finished when synchronized.
	if ((buffer->memoryHints & dsGfxMemory_Synchronize) && lastUsedSubmit != DS_NOT_SUBMITTED)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

		dsGfxFenceResult fenceResult = dsVkRenderer_waitForSubmit(renderer, lastUsedSubmit,
			DS_DEFAULT_WAIT_TIMEOUT);

		DS_VERIFY(dsSpinlock_lock(&vkBuffer->lock));
		DS_VERIFY(dsSpinlock_lock(&bufferData->resource.lock));

		if (fenceResult != dsGfxFenceResult_Success)
		{
			bufferData->mappedStart = 0;
			bufferData->mappedSize = 0;
			bufferData->mappedWrite = false;
			DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
			DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));

			if (fenceResult == dsGfxFenceResult_Timeout)
			{
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG,
					"Timed out waiting for synchronized buffer.");
				errno = ETIMEDOUT;
			}
			else
			{
				if (fenceResult == dsGfxFenceResult_WaitingToQueue)
					DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer still queued to be rendered.");
				errno = EPERM;
			}

			return NULL;
		}

		if (bufferData != vkBuffer->bufferData || bufferData->mappedSize == 0)
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
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't map buffer memory"))
	{
		bufferData->mappedStart = 0;
		bufferData->mappedSize = 0;
		bufferData->mappedWrite = false;
		DS_VERIFY(dsSpinlock_unlock(&bufferData->resource.lock));
		DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
		return NULL;
	}

	// Invalidate range if the GPU can write to the buffer and not coherent or persistently mapped.
	bool gpuCanWrite = (bufferData->usage & (dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_CopyTo)) != 0;
	if (!bufferData->hostMemoryCoherent && gpuCanWrite && !(flags & dsGfxBufferMap_Persistent) &&
		lastUsedSubmit != DS_NOT_SUBMITTED)
	{
		VkMappedMemoryRange range =
		{
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			NULL,
			bufferData->hostMemory,
			offset,
			offset + size == buffer->size ? VK_WHOLE_SIZE : size
		};
		DS_VK_CALL(device->vkInvalidateMappedMemoryRanges)(device->device, 1, &range);
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

	if (bufferData->mappedWrite)
	{
		// Need to mark the range as dirty to copy to the GPU when next used.
		if (bufferData->deviceMemory && !bufferData->needsInitialCopy)
		{
			uint32_t rangeIndex = bufferData->dirtyRangeCount;
			if (DS_RESIZEABLE_ARRAY_ADD(bufferData->scratchAllocator, bufferData->dirtyRanges,
				bufferData->dirtyRangeCount, bufferData->maxDirtyRanges, 1))
			{
				bufferData->dirtyRanges[rangeIndex].start = bufferData->mappedStart;
				bufferData->dirtyRanges[rangeIndex].size = bufferData->mappedSize;
			}
		}

		if (!bufferData->hostMemoryCoherent)
		{
			VkMappedMemoryRange range =
			{
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				NULL,
				bufferData->hostMemory,
				bufferData->mappedStart,
				bufferData->mappedStart + bufferData->mappedSize == buffer->size ? VK_WHOLE_SIZE :
					bufferData->mappedSize
			};
			DS_VK_CALL(device->vkFlushMappedMemoryRanges)(device->device, 1, &range);
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

	VkMappedMemoryRange range =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		NULL,
		bufferData->hostMemory,
		offset,
		offset + size == buffer->size ? VK_WHOLE_SIZE : size
	};
	VkResult result = DS_VK_CALL(device->vkFlushMappedMemoryRanges)(device->device, 1, &range);
	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
	return DS_HANDLE_VK_RESULT(result, "Couldn't flush buffer memory");
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
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be invalidated.");
		return false;
	}

	VkMappedMemoryRange range =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		NULL,
		bufferData->hostMemory,
		offset,
		offset + size == buffer->size ? VK_WHOLE_SIZE : size
	};
	VkResult result = DS_VK_CALL(device->vkInvalidateMappedMemoryRanges)(device->device, 1, &range);
	DS_VERIFY(dsSpinlock_unlock(&vkBuffer->lock));
	return DS_HANDLE_VK_RESULT(result, "Couldn't invalidate buffer memory");
}

bool dsVkGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = resourceManager->renderer;
	const size_t maxCommandBufferSize = 65536;
	// NOTE: Qualcomm's driver is obscenely slow with vkCmdUpdateBuffer().
	if (size > maxCommandBufferSize || renderer->vendorID == DS_VENDOR_ID_QUALCOMM)
		return copyDataTempBuffer(commandBuffer, buffer, offset, data, size);
	else
		return copyDataCommandBuffer(commandBuffer, buffer, offset, data, size);
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

	dsVkRenderer_processGfxBuffer(renderer, srcBufferData);
	dsVkRenderer_processGfxBuffer(renderer, dstBufferData);

	VkBuffer srcCopyBuffer = dsVkGfxBufferData_getBuffer(srcBufferData);
	VkBuffer dstCopyBuffer = dsVkGfxBufferData_getBuffer(dstBufferData);

	bool srcCanMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(srcBufferData);
	bool dstCanMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(dstBufferData);
	VkBufferMemoryBarrier barriers[2] =
	{
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			dsVkWriteBufferAccessFlags(dstBufferData->usage, dstCanMapMainBuffer) |
				dsVkReadBufferAccessFlags(dstBuffer->usage),
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			dstCopyBuffer,
			dstOffset,
			size
		},
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			dsVkWriteBufferAccessFlags(srcBufferData->usage, srcCanMapMainBuffer),
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			srcCopyBuffer,
			srcOffset,
			size
		}
	};
	uint32_t barrierCount = 1;
	VkPipelineStageFlags stages = dsVkReadBufferStageFlags(renderer, dstBuffer->usage) |
		dsVkWriteBufferStageFlags(renderer, dstBufferData->usage, dstCanMapMainBuffer);
	if (!dsVkGfxBufferData_isStatic(srcBufferData))
	{
		++barrierCount;
		stages |= dsVkWriteBufferStageFlags(renderer, srcBufferData->usage, srcCanMapMainBuffer);
	}
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, stages,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, barrierCount, barriers, 0, NULL);

	VkBufferCopy bufferCopy = {srcOffset, dstOffset, size};
	DS_VK_CALL(device->vkCmdCopyBuffer)(vkCommandBuffer, srcCopyBuffer, dstCopyBuffer, 1,
		&bufferCopy);

	barriers[0].dstAccessMask = barriers[0].srcAccessMask;
	barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].dstAccessMask = barriers[1].srcAccessMask;
	barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stages, 0, 0, NULL, 2, barriers, 0, NULL);
	return true;
}

bool dsVkGfxBuffer_copyToTexture(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, dsTexture* dstTexture, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	dsVkGfxBufferData* srcBufferData = dsVkGfxBuffer_getData(srcBuffer, commandBuffer);
	dsVkTexture* dstVkTexture = (dsVkTexture*)dstTexture;
	if (!srcBufferData || !dsVkCommandBuffer_addResource(commandBuffer, &dstVkTexture->resource))
		return false;

	dsVkRenderer_processGfxBuffer(renderer, srcBufferData);
	dsVkRenderer_processTexture(renderer, dstTexture);

	bool srcCanMapMainBuffer = dsVkGfxBufferData_canMapMainBuffer(srcBufferData);

	VkImageAspectFlags dstAspectMask = dsVkImageAspectFlags(dstTexture->info.format);
	uint32_t dstFaceCount = dstTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;
	bool dstIsDepthStencil = dsGfxFormat_isDepthStencil(dstTexture->info.format);

	if (!addCopyToImageBarriers(commandBuffer, regions, regionCount, srcBufferData,
			srcCanMapMainBuffer, dstTexture, false))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}

	// 1024 regions is ~56 KB of stack space. After that use heap space.
	bool heapRegions = regionCount > 1024;
	dsAllocator* scratchAllocator = resourceManager->allocator;
	VkBufferImageCopy* imageCopies;
	if (heapRegions)
	{
		imageCopies = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, VkBufferImageCopy, regionCount);
		if (!imageCopies)
			return false;
	}
	else
		imageCopies = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkBufferImageCopy, regionCount);

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;
		uint32_t dstLayer, dstDepth, layerCount, depthCount;
		if (dstIs3D)
		{
			dstLayer = 0;
			dstDepth = region->texturePosition.depth;
			layerCount = 1;
			depthCount = region->layers;
		}
		else
		{
			dstLayer = region->texturePosition.depth*dstFaceCount + region->texturePosition.face;
			dstDepth = 0;
			layerCount = region->layers;
			depthCount = 1;
		}

		VkBufferImageCopy* imageCopy = imageCopies + i;
		imageCopy->bufferOffset = region->bufferOffset;
		imageCopy->bufferRowLength = region->bufferWidth;
		imageCopy->bufferImageHeight = region->bufferHeight;
		imageCopy->imageSubresource.aspectMask = dstAspectMask;
		imageCopy->imageSubresource.mipLevel = region->texturePosition.mipLevel;
		imageCopy->imageSubresource.baseArrayLayer = dstLayer;
		imageCopy->imageSubresource.layerCount = layerCount;
		imageCopy->imageOffset.x = region->texturePosition.x;
		imageCopy->imageOffset.y = region->texturePosition.y;
		imageCopy->imageOffset.z = dstDepth;
		imageCopy->imageExtent.width = region->textureWidth;
		imageCopy->imageExtent.height = region->textureHeight;
		imageCopy->imageExtent.depth = depthCount;
	}

	VkPipelineStageFlags srcStageFlags = dsVkReadBufferStageFlags(renderer, srcBuffer->usage) |
		dsVkWriteBufferStageFlags(renderer, srcBuffer->usage, srcCanMapMainBuffer);
	VkPipelineStageFlags dstStageFlags = dsVkReadImageStageFlags(renderer, dstTexture->usage,
			dstTexture->offscreen && dstIsDepthStencil && !dstTexture->resolve) |
		dsVkWriteImageStageFlags(renderer, dstTexture->usage, dstTexture->offscreen,
			dstIsDepthStencil);
	VkPipelineStageFlags stageFlags = srcStageFlags | dstStageFlags;
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, stageFlags,
		VK_PIPELINE_STAGE_TRANSFER_BIT);
	DS_VK_CALL(device->vkCmdCopyBufferToImage)(vkCommandBuffer,
		dsVkGfxBufferData_getBuffer(srcBufferData), dstVkTexture->deviceImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, imageCopies);

	if (heapRegions)
		DS_VERIFY(dsAllocator_free(scratchAllocator, imageCopies));

	if (!addCopyToImageBarriers(commandBuffer, regions, regionCount, srcBufferData,
			srcCanMapMainBuffer, dstTexture, true))
	{
		dsVkCommandBuffer_resetMemoryBarriers(commandBuffer);
		return false;
	}
	dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stageFlags);

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
