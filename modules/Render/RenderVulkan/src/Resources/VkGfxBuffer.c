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

#include "VkGfxBuffer.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

static void destroyBufferData(dsVkDevice* device, dsVkGfxBufferData* buffer)
{
	dsVkInstance* instance = &device->instance;
	if (buffer->deviceBuffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->deviceBuffer,
			instance->allocCallbacksPtr);
	}
	if (buffer->hostMemory)
	{
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
	if (buffer->allocator)
		dsAllocator_free(buffer->allocator, buffer);
}

static dsVkGfxBufferData* createBufferData(dsVkDevice* device, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
{
	dsVkGfxBufferData* buffer = DS_ALLOCATE_OBJECT(allocator, dsVkGfxBufferData);
	if (!buffer)
		return NULL;

	memset(buffer, 0, sizeof(dsVkGfxBuffer));
	buffer->allocator = dsAllocator_keepPointer(allocator);

	dsVkInstance* instance = &device->instance;

	// Based on the flags, see what's required both for host and device access.
	bool needsDeviceMemory, needsHostMemory;
	dsGfxMemory deviceHints, hostHints;
	hostHints = memoryHints & (~dsGfxMemory_GPUOnly);
	bool canHaveOnGPU = !(memoryHints & (dsGfxMemory_Read | dsGfxMemory_Persistent));
	if ((memoryHints & dsGfxMemory_GPUOnly) || ((memoryHints & dsGfxMemory_Static) && canHaveOnGPU))
	{
		needsDeviceMemory = true;
		needsHostMemory = data != NULL || !(memoryHints & dsGfxMemory_GPUOnly);
		deviceHints = dsGfxMemory_GPUOnly;
	}
	else
	{
		needsDeviceMemory = false;
		needsHostMemory = true;
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
			destroyBufferData(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->deviceBuffer,
			&deviceRequirements);
		deviceMemoryIndex = dsVkMemoryIndex(device, &deviceRequirements, deviceHints);
		if (deviceMemoryIndex == DS_INVALID_HEAP)
		{
			destroyBufferData(device, buffer);
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
			destroyBufferData(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, hostHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			destroyBufferData(device, buffer);
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
			destroyBufferData(device, buffer);
			return NULL;
		}

		DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->hostBuffer,
			&hostRequirements);
		hostMemoryIndex = dsVkMemoryIndex(device, &hostRequirements, memoryHints);
		if (hostMemoryIndex == DS_INVALID_HEAP)
		{
			destroyBufferData(device, buffer);
			return NULL;
		}
	}

	// Create the memory to use with the buffers.
	if (needsDeviceMemory)
	{
		buffer->deviceMemory = dsAllocateVkMemory(device, &deviceRequirements, deviceMemoryIndex);
		if (!buffer->deviceMemory)
		{
			destroyBufferData(device, buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device,
			buffer->deviceBuffer, buffer->deviceMemory, 0);
		if (!dsHandleVkResult(result))
		{
			destroyBufferData(device, buffer);
			return NULL;
		}
	}

	if (needsHostMemory)
	{
		buffer->hostMemory = dsAllocateVkMemory(device, &hostRequirements, hostMemoryIndex);
		if (!buffer->hostMemory)
		{
			destroyBufferData(device, buffer);
			return NULL;
		}

		VkResult result = DS_VK_CALL(device->vkBindBufferMemory)(device->device, buffer->hostBuffer,
			buffer->hostMemory, 0);
		if (!dsHandleVkResult(result))
		{
			destroyBufferData(device, buffer);
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
			destroyBufferData(device, buffer);
			return NULL;
		}

		memcpy(mappedData, data, size);
		DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->hostMemory);
		buffer->needsUpload = true;
	}

	buffer->lastUsedSubmit = DS_NOT_SUBMITTED;
	buffer->uploadedSubmit = DS_NOT_SUBMITTED;
	buffer->keepHost = needsHostMemory;
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
	buffer->bufferData = createBufferData(device, allocator, usage, memoryHints, data, size);
	if (!buffer->bufferData)
	{
		if (baseBuffer->allocator)
			dsAllocator_free(baseBuffer->allocator, buffer);
	}

	DS_VERIFY(dsSpinlock_initialize(&buffer->lock));
	return baseBuffer;
}
