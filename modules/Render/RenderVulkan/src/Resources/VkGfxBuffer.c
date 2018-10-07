/*
 * Copyright 2018 Aaron Barany
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
#include "VkRendererInternal.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

static dsVkGfxBufferData* createBufferData(dsVkDevice* device, dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsGfxBufferUsage usage, dsGfxMemory memoryHints,
	const void* data, size_t size)
{
	DS_ASSERT(scratchAllocator->freeFunc);
	dsVkGfxBufferData* buffer = DS_ALLOCATE_OBJECT(allocator, dsVkGfxBufferData);
	if (!buffer)
		return NULL;

	memset(buffer, 0, sizeof(dsVkGfxBuffer));
	buffer->allocator = dsAllocator_keepPointer(allocator);
	buffer->scratchAllocator = scratchAllocator;
	DS_VERIFY(dsSpinlock_initialize(&buffer->lock));

	dsVkInstance* instance = &device->instance;

	// Based on the flags, see what's required both for host and device access.
	bool needsDeviceMemory, needsHostMemory, keepHostMemory;
	dsGfxMemory deviceHints, hostHints;
	hostHints = memoryHints & (~dsGfxMemory_GPUOnly);
	bool canHaveOnGPU = !(memoryHints & (dsGfxMemory_Read | dsGfxMemory_Persistent));
	if ((memoryHints & dsGfxMemory_GPUOnly) || ((memoryHints & dsGfxMemory_Static) && canHaveOnGPU))
	{
		needsDeviceMemory = true;
		needsHostMemory = data != NULL || !(memoryHints & dsGfxMemory_GPUOnly);
		keepHostMemory = (memoryHints & dsGfxMemory_GPUOnly) != 0;
		deviceHints = dsGfxMemory_GPUOnly;
	}
	else
	{
		needsDeviceMemory = false;
		needsHostMemory = true;
		keepHostMemory = true;
		deviceHints = hostHints;
	}

	// Base flags determined from the usage flags passed in.
	VkBufferUsageFlags baseCreateFlags = 0;
	if (usage & dsGfxBufferUsage_Index)
		baseCreateFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_Vertex)
		baseCreateFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (usage & (dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch))
		baseCreateFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_UniformBlock)
		baseCreateFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_UniformBuffer)
		baseCreateFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_Image)
		baseCreateFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_MutableImage)
		baseCreateFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_CopyFrom)
		baseCreateFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (usage & dsGfxBufferUsage_CopyTo)
		baseCreateFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	// Create device buffer for general usage.
	uint32_t deviceMemoryIndex = DS_INVALID_HEAP;
	VkMemoryRequirements deviceRequirements;
	if (needsDeviceMemory)
	{
		VkBufferUsageFlags createFlags = baseCreateFlags;
		if (needsHostMemory)
			createFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VkBufferCreateInfo bufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			NULL,
			0,
			size,
			createFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			1, &device->queueFamilyIndex
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->deviceBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->deviceBuffer,
			&deviceRequirements);
		deviceMemoryIndex = dsVkMemoryIndex(device, &deviceRequirements, deviceHints);
		if (deviceMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}
	}

	// Create host buffer for access on the host.
	uint32_t hostMemoryIndex = DS_INVALID_HEAP;
	VkMemoryRequirements hostRequirements;
	if (needsHostMemory)
	{
		VkBufferUsageFlags createFlags;
		if (needsDeviceMemory)
			createFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		else
			createFlags = baseCreateFlags;
		VkBufferCreateInfo bufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			NULL,
			0,
			size,
			createFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			1, &device->queueFamilyIndex
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->hostBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, hostHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}
	}

	// Check if the device and host memory are the same. If so, only create a single buffer.
	// This is generally the case on devices with a shared memory model.
	if (deviceMemoryIndex == hostMemoryIndex)
	{
		DS_ASSERT(needsDeviceMemory && needsDeviceMemory);
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->deviceBuffer,
			instance->allocCallbacksPtr);
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->hostBuffer,
			instance->allocCallbacksPtr);
		buffer->deviceBuffer = 0;
		needsDeviceMemory = false;
		keepHostMemory = true;

		VkBufferCreateInfo bufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			NULL,
			0,
			size,
			baseCreateFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			1, &device->queueFamilyIndex
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->hostBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, memoryHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}
	}

	// Create the memory to use with the buffers.
	if (needsDeviceMemory)
	{
		buffer->deviceMemory = dsAllocateVkMemory(device, &deviceRequirements, deviceMemoryIndex);
		if (!buffer->deviceMemory)
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device,
			buffer->deviceBuffer, buffer->deviceMemory, 0);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}
	}

	if (needsHostMemory)
	{
		buffer->hostMemory = dsAllocateVkMemory(device, &hostRequirements, hostMemoryIndex);
		if (!buffer->hostMemory)
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device, buffer->hostBuffer,
			buffer->hostMemory, 0);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}
	}

	// Set the initial data.
	if (data)
	{
		DS_ASSERT(buffer->hostMemory);
		void* mappedData;
		VkResult result = DS_VK_CALL(device->vkMapMemory)(device->device, buffer->hostMemory, 0,
			size, 0, &mappedData);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(device, buffer);
			return NULL;
		}

		memcpy(mappedData, data, size);
		DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->hostMemory);
		buffer->needsInitialCopy = true;
	}

	buffer->size = size;
	buffer->lastUsedSubmit = DS_NOT_SUBMITTED;
	buffer->uploadedSubmit = DS_NOT_SUBMITTED;
	buffer->keepHost = keepHostMemory;
	buffer->used = false;
	return buffer;
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

	dsVkResourceManager* vkResourceManager = (dsVkResourceManager*)resourceManager;
	dsVkDevice* device = vkResourceManager->device;
	buffer->bufferData = createBufferData(device, allocator, resourceManager->allocator, usage,
		memoryHints, data, size);
	if (!buffer->bufferData)
	{
		if (baseBuffer->allocator)
			dsAllocator_free(baseBuffer->allocator, buffer);
		return NULL;
	}

	return baseBuffer;
}

void* dsVkGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	dsVkGfxBufferData* bufferData;
	DS_ATOMIC_LOAD_PTR(&vkBuffer->bufferData, &bufferData);

	DS_VERIFY(dsSpinlock_lock(&bufferData->lock));
	if (bufferData->mappedSize > 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer is already mapped.");
		return NULL;
	}

	if (!bufferData->keepHost)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be mapped.");
		return NULL;
	}

	// Orphan the data if invalidated.
	if ((flags & dsGfxBufferMap_Invalidate) && bufferData->used)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		dsVkGfxBufferData* newBufferData = createBufferData(device, buffer->allocator,
			resourceManager->allocator, buffer->usage, buffer->memoryHints, NULL, buffer->size);
		if (!newBufferData)
			return NULL;

		// Delete the previous buffer data and replace with the new one. Note that it might have
		// been re-assigned, so do an exchange to get the newest one associated with the buffer.
		DS_ATOMIC_EXCHANGE_PTR(&vkBuffer->bufferData, &newBufferData, &bufferData);
		dsVkRenderer_deleteGfxBuffer(renderer, bufferData);
		bufferData = newBufferData;
		DS_VERIFY(dsSpinlock_lock(&bufferData->lock));
		DS_ASSERT(bufferData->keepHost);
		DS_ASSERT(bufferData->hostMemory);
	}

	bufferData->mappedStart = offset;
	bufferData->mappedSize = size;
	bufferData->mappedWrite = (flags & dsGfxBufferMap_Write) != 0;
	uint64_t lastUsedSubmit = bufferData->lastUsedSubmit;
	DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));

	// Wait for the submitted command to be finished when reading.
	if ((flags & dsGfxBufferMap_Read) && (buffer->memoryHints & dsGfxMemory_Synchronize) &&
		lastUsedSubmit != DS_NOT_SUBMITTED)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));

		// 10 seconds in nanoseconds
		const uint64_t timeout = 10000000000;
		dsGfxFenceResult fenceResult = dsVkRenderer_waitForSubmit(renderer, lastUsedSubmit,
			timeout);

		DS_VERIFY(dsSpinlock_lock(&bufferData->lock));

		if (fenceResult == dsGfxFenceResult_WaitingToQueue)
		{
			bufferData->mappedStart = 0;
			bufferData->mappedSize = 0;
			bufferData->mappedWrite = false;
			DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));

			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer still queued to be rendered.");
			return NULL;
		}

		if (bufferData->mappedSize == 0)
		{
			DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
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
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		return NULL;
	}

	DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
	return memory;
}

bool dsVkGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	dsVkGfxBufferData* bufferData;
	DS_ATOMIC_LOAD_PTR(&vkBuffer->bufferData, &bufferData);

	DS_VERIFY(dsSpinlock_lock(&bufferData->lock));
	if (bufferData->mappedSize == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
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
	DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));

	return true;
}

bool dsVkGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;

	dsVkGfxBufferData* bufferData;
	DS_ATOMIC_LOAD_PTR(&vkBuffer->bufferData, &bufferData);

	if (!bufferData->keepHost)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be flushed.");
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		return false;
	}

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

	dsVkGfxBufferData* bufferData;
	DS_ATOMIC_LOAD_PTR(&vkBuffer->bufferData, &bufferData);

	if (!bufferData->keepHost)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Buffer memory not accessible to be flushed.");
		DS_VERIFY(dsSpinlock_unlock(&bufferData->lock));
		return false;
	}

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

bool dsVkGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	dsVkGfxBuffer* vkBuffer = (dsVkGfxBuffer*)buffer;
	dsVkRenderer_deleteGfxBuffer(resourceManager->renderer, vkBuffer->bufferData);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
	return true;
}

void dsVkGfxBufferData_destroy(dsVkDevice* device, dsVkGfxBufferData* buffer)
{
	if (!buffer)
		return;

	dsVkInstance* instance = &device->instance;
	if (buffer->deviceBuffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->deviceBuffer,
			instance->allocCallbacksPtr);
	}
	if (buffer->hostMemory)
	{
		if (buffer->mappedSize > 0)
			DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->hostMemory);
		DS_VK_CALL(device->vkFreeMemory)(device->device, buffer->deviceMemory,
			instance->allocCallbacksPtr);
	}
	if (buffer->hostBuffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->hostBuffer,
			instance->allocCallbacksPtr);
	}
	if (buffer->hostMemory)
	{
		DS_VK_CALL(device->vkFreeMemory)(device->device, buffer->hostMemory,
			instance->allocCallbacksPtr);
	}

	DS_VERIFY(dsAllocator_free(buffer->scratchAllocator, buffer->dirtyRanges));

	dsSpinlock_shutdown(&buffer->lock);
	if (buffer->allocator)
		dsAllocator_free(buffer->allocator, buffer);
}
