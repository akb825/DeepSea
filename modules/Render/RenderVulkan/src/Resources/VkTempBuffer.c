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

#include "Resources/VkTempBuffer.h"

#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "Resources/VkTexture.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsVkTempBuffer* dsVkTempBuffer_create(dsAllocator* allocator, dsVkDevice* device, size_t size)
{
	dsVkInstance* instance = &device->instance;

	dsVkTempBuffer* buffer = DS_ALLOCATE_OBJECT(allocator, dsVkTempBuffer);
	DS_ASSERT(buffer);
	dsVkResource_initialize(&buffer->resource);
	buffer->allocator = dsAllocator_keepPointer(allocator);
	buffer->device = device;
	buffer->buffer = 0;
	buffer->memory = 0;
	buffer->size = 0;
	buffer->capacity = size;

	VkBufferCreateInfo bufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, NULL
	};
	VkResult result = DS_VK_CALL(device->vkCreateBuffer)(device->device, &bufferCreateInfo,
		instance->allocCallbacksPtr, &buffer->buffer);
	if (!dsHandleVkResult(result))
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	VkMemoryRequirements memoryRequirements;
	DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->buffer,
		&memoryRequirements);
	uint32_t memoryIndex = dsVkMemoryIndex(device, &memoryRequirements,
		dsGfxMemory_Stream | dsGfxMemory_Coherent);
	if (memoryIndex == DS_INVALID_HEAP)
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	buffer->memory = dsAllocateVkMemory(device, &memoryRequirements, memoryIndex);
	if (!buffer->memory)
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	result = DS_VK_CALL(device->vkMapMemory)(
		device->device, buffer->memory, 0, VK_WHOLE_SIZE, 0, (void**)&buffer->contents);
	if (!dsHandleVkResult(result))
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	result =
		DS_VK_CALL(device->vkBindBufferMemory)(device->device, buffer->buffer, buffer->memory, 0);
	if (!dsHandleVkResult(result))
	{
		dsVkTempBuffer_destroy(buffer);
		return NULL;
	}

	return buffer;
}

void* dsVkTempBuffer_allocate(size_t* outOffset, dsVkTempBuffer* buffer, size_t size,
	uint32_t alignment)
{
	size_t offset = DS_CUSTOM_ALIGNED_SIZE(buffer->size, alignment);
	if (!DS_IS_BUFFER_RANGE_VALID(offset, size, DS_TEMP_BUFFER_CAPACITY))
		return NULL;

	buffer->size = offset + size;
	*outOffset = offset;
	return buffer->contents + offset;
}

bool dsVkTempBuffer_reset(dsVkTempBuffer* buffer, uint64_t finishedSubmit)
{
	if (dsVkResource_isInUse(&buffer->resource, finishedSubmit))
		return false;

	buffer->size = 0;
	return true;
}

void dsVkTempBuffer_destroy(dsVkTempBuffer* buffer)
{
	dsVkDevice* device = buffer->device;
	dsVkInstance* instance = &device->instance;
	if (buffer->buffer)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(
			device->device, buffer->buffer, instance->allocCallbacksPtr);
	}

	if (buffer->memory)
	{
		DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->memory);
		DS_VK_CALL(device->vkFreeMemory)(
			device->device, buffer->memory, instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&buffer->resource);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
}
