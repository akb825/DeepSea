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

#include "Resources/VkGfxBufferData.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

dsVkGfxBufferData* dsVkGfxBufferData_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* scratchAllocator, dsGfxBufferUsage usage,
	dsGfxMemory memoryHints, const void* data, size_t size)
{
	DS_ASSERT(scratchAllocator->freeFunc);
	dsVkGfxBufferData* buffer = DS_ALLOCATE_OBJECT(allocator, dsVkGfxBufferData);
	if (!buffer)
		return NULL;

	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	memset(buffer, 0, sizeof(*buffer));
	buffer->resourceManager = resourceManager;
	buffer->allocator = dsAllocator_keepPointer(allocator);
	buffer->scratchAllocator = scratchAllocator;
	dsVkResource_initialize(&buffer->resource);
	DS_VERIFY(dsSpinlock_initialize(&buffer->bufferViewLock));

	buffer->lifetime = dsLifetime_create(allocator, buffer);
	if (!buffer->lifetime)
	{
		dsVkGfxBufferData_destroy(buffer);
		return NULL;
	}

	// Based on the flags, see what's required both for host and device access.
	bool needsDeviceMemory, needsHostMemory, keepHostMemory;
	dsGfxMemory deviceHints, hostHints;
	hostHints = memoryHints & (~dsGfxMemory_GPUOnly);
	bool canHaveOnGPU = !(memoryHints & (dsGfxMemory_Read | dsGfxMemory_Persistent));
	if ((memoryHints & dsGfxMemory_GPUOnly) || ((memoryHints & dsGfxMemory_Static) && canHaveOnGPU))
	{
		needsDeviceMemory = true;
		needsHostMemory = data != NULL || !(memoryHints & dsGfxMemory_GPUOnly);
		keepHostMemory = (memoryHints & dsGfxMemory_GPUOnly) == 0;
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
	if (usage & dsGfxBufferUsage_CopyTo || (data && needsDeviceMemory))
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
			0, NULL
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->deviceBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->deviceBuffer,
			&deviceRequirements);
		deviceMemoryIndex = dsVkMemoryIndex(device, &deviceRequirements, deviceHints);
		if (deviceMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(buffer);
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
			0, NULL
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->hostBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		// Check if the device memory index is supported. If so, use it explicitly since
		// dsVkMemoryIndex() may not return the same value.
		if (dsVkMemoryIndexCompatible(device, &hostRequirements, hostHints, deviceMemoryIndex))
			hostMemoryIndex = deviceMemoryIndex;
		else
			hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, hostHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		buffer->hostMemoryCoherent = dsVkHeapIsCoherent(device, hostMemoryIndex);
	}

	// Check if the device and host memory are the same. If so, only create a single buffer.
	// This is generally the case on devices with a shared memory model.
	if (deviceMemoryIndex == hostMemoryIndex)
	{
		DS_ASSERT(needsDeviceMemory && needsHostMemory);
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
			0, NULL
		};
		VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
			instance->allocCallbacksPtr, &buffer->hostBuffer);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, memoryHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}
	}

	// Create the memory to use with the buffers.
	if (needsDeviceMemory)
	{
		buffer->deviceMemory = dsAllocateVkMemory(device, &deviceRequirements, deviceMemoryIndex);
		if (!buffer->deviceMemory)
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device,
			buffer->deviceBuffer, buffer->deviceMemory, 0);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}
	}

	if (needsHostMemory)
	{
		buffer->hostMemory = dsAllocateVkMemory(device, &hostRequirements, hostMemoryIndex);
		if (!buffer->hostMemory)
		{
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device, buffer->hostBuffer,
			buffer->hostMemory, 0);
		if (!dsHandleVkResult(result))
		{
			dsVkGfxBufferData_destroy(buffer);
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
			dsVkGfxBufferData_destroy(buffer);
			return NULL;
		}

		memcpy(mappedData, data, size);
		if (!buffer->hostMemoryCoherent)
		{
			VkMappedMemoryRange range =
			{
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				NULL,
				buffer->hostMemory,
				0,
				VK_WHOLE_SIZE
			};
			DS_VK_CALL(device->vkFlushMappedMemoryRanges)(device->device, 1, &range);
		}
		DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->hostMemory);
		buffer->needsInitialCopy = true;
	}

	buffer->usage = usage;
	buffer->memoryHints = memoryHints;
	buffer->size = size;
	buffer->uploadedSubmit = DS_NOT_SUBMITTED;
	buffer->keepHost = keepHostMemory;
	buffer->used = false;
	return buffer;
}

VkBufferView dsVkGfxBufferData_getBufferView(dsVkGfxBufferData* buffer, dsGfxFormat format,
	size_t offset, size_t count)
{
	dsVkDevice* device = &((dsVkRenderer*)buffer->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	DS_VERIFY(dsSpinlock_lock(&buffer->bufferViewLock));

	for (uint32_t i = 0; i < buffer->bufferViewCount; ++i)
	{
		dsVkBufferView* view = buffer->bufferViews + i;
		if (view->format == format && view->offset == offset && view->count == count)
		{
			VkBufferView bufferView = view->bufferView;
			DS_VERIFY(dsSpinlock_unlock(&buffer->bufferViewLock));
			return bufferView;
		}
	}

	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(buffer->resourceManager,
		format);
	if (!formatInfo)
	{
		errno = EINVAL;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
		return 0;
	}

	uint32_t index = buffer->bufferViewCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(buffer->scratchAllocator, buffer->bufferViews,
		buffer->bufferViewCount, buffer->maxBufferViews, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&buffer->bufferViewLock));
		return 0;
	}

	dsVkBufferView* view = buffer->bufferViews + index;
	view->format = format;
	view->offset = offset;
	view->count = count;

	VkBufferViewCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		NULL,
		0,
		dsVkGfxBufferData_getBuffer(buffer),
		formatInfo->vkFormat,
		offset,
		count*dsGfxFormat_size(format)
	};

	VkBufferView bufferView;
	VkResult result = DS_VK_CALL(device->vkCreateBufferView)(device->device, &createInfo,
		instance->allocCallbacksPtr, &bufferView);
	if (!dsHandleVkResult(result))
	{
		DS_VERIFY(dsSpinlock_unlock(&buffer->bufferViewLock));
		--buffer->bufferViewCount;
		return 0;
	}

	view->bufferView = bufferView;

	DS_VERIFY(dsSpinlock_unlock(&buffer->bufferViewLock));

	return bufferView;
}

VkBuffer dsVkGfxBufferData_getBuffer(const dsVkGfxBufferData* buffer);

bool dsVkGfxBufferData_canMap(const dsVkGfxBufferData* buffer)
{
	return (buffer->memoryHints & dsGfxMemory_GPUOnly) == 0 && !buffer->deviceBuffer;
}

bool dsVkGfxBufferData_isStatic(const dsVkGfxBufferData* buffer)
{
	/*
	 * Check for:
	 * 1. Doesn't allow GPU usage that supports copying.
	 * 2. If access on host via mapping isn't allowed.
	 * 3. Device memory is used, in which case the data must be copied.
	 * 1 and either 2 or 3 must be met.
	 */
	return !(buffer->usage & (dsGfxBufferUsage_CopyTo | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_MutableImage)) &&
		((buffer->memoryHints & dsGfxMemory_GPUOnly) || buffer->deviceMemory);
}

bool dsVkGfxBufferData_addMemoryBarrier(dsVkGfxBufferData* buffer, VkDeviceSize offset,
	VkDeviceSize size, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(DS_IS_BUFFER_RANGE_VALID(offset, size, buffer->size));
	bool canMap = dsVkGfxBufferData_canMap(buffer);
	bool canWrite = buffer->usage & (dsGfxBufferUsage_CopyTo | dsGfxBufferUsage_MutableImage |
		dsGfxBufferUsage_UniformBuffer) || canMap;
	if (canWrite)
	{
		VkAccessFlags srcAccessMask = dsVkWriteBufferAccessFlags(buffer->usage, canMap);
		VkAccessFlags dstAccessMask = dsVkReadBufferAccessFlags(buffer->usage);
		VkBuffer vkBuffer = dsVkGfxBufferData_getBuffer(buffer);
		VkBufferMemoryBarrier bufferBarrier =
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			srcAccessMask,
			dstAccessMask,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			vkBuffer,
			offset,
			size
		};

		// If recently added, implies that the following parts have already been done.
		if (dsVkCommandBuffer_recentlyAddedBufferBarrier(commandBuffer, &bufferBarrier))
			return true;

		VkBufferMemoryBarrier* addedBarrier = dsVkCommandBuffer_addBufferBarrier(commandBuffer);
		if (!addedBarrier)
			return false;

		*addedBarrier = bufferBarrier;
	}

	// Make sure the buffer is renderable.
	dsVkRenderer_processGfxBuffer(commandBuffer->renderer, buffer);

	// Getting the buffer added to the command buffer resource list.
	return true;
}

void dsVkGfxBufferData_destroy(dsVkGfxBufferData* buffer)
{
	if (!buffer)
		return;

	dsVkDevice* device = &((dsVkRenderer*)buffer->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	dsLifetime_destroy(buffer->lifetime);

	if (buffer->deviceBuffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->deviceBuffer,
			instance->allocCallbacksPtr);
	}
	if (buffer->deviceMemory)
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

	for (uint32_t i = 0; i < buffer->bufferViewCount; ++i)
	{
		DS_VK_CALL(device->vkDestroyBufferView)(device->device, buffer->bufferViews[i].bufferView,
			instance->allocCallbacksPtr);
	}
	DS_VERIFY(dsAllocator_free(buffer->scratchAllocator, buffer->bufferViews));

	dsSpinlock_shutdown(&buffer->bufferViewLock);
	dsVkResource_shutdown(&buffer->resource);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
}
