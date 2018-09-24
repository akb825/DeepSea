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

	VkBufferUsageFlags createFlags = 0;
	if (usage & dsGfxBufferUsage_Index)
		createFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_Vertex)
		createFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (usage & (dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch))
		createFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_UniformBlock)
		createFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_UniformBuffer)
		createFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_Image)
		createFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_MutableImage)
		createFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	if (usage & dsGfxBufferUsage_CopyFrom)
		createFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (usage & dsGfxBufferUsage_CopyTo)
		createFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	dsVkResourceManager* vkResourceManager = (dsVkResourceManager*)resourceManager;
	dsVkDevice* device = vkResourceManager->device;
	dsVkInstance* instance = &device->instance;
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
		instance->allocCallbacksPtr, &buffer->vkBuffer);
	if (!dsHandleVkResult(result))
	{
		dsAllocator_free(allocator, buffer);
		return NULL;
	}

	VkMemoryRequirements requirements;
	DS_VK_CALL(device->vkGetBufferMemoryRequirements)(device->device, buffer->vkBuffer,
		&requirements);

	buffer->memory = dsAllocateVkMemory(device, &requirements, memoryHints);
	if (!buffer->memory)
	{
		DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->vkBuffer,
			instance->allocCallbacksPtr);
		dsAllocator_free(allocator, buffer);
		return NULL;
	}

	if (data)
	{
		void* mappedData;
		result = DS_VK_CALL(device->vkMapMemory)(device->device, buffer->memory, 0, size, 0,
			&mappedData);
		if (!dsHandleVkResult(result))
		{
			DS_VK_CALL(device->vkDestroyBuffer)(device->device, buffer->vkBuffer,
				instance->allocCallbacksPtr);
			DS_VK_CALL(device->vkFreeMemory)(device->device, buffer->memory,
				instance->allocCallbacksPtr);
			dsAllocator_free(allocator, buffer);
			return NULL;
		}

		memcpy(mappedData, data, size);
		DS_VK_CALL(device->vkUnmapMemory)(device->device, buffer->memory);
	}

	dsGfxBuffer* baseBuffer = (dsGfxBuffer*)buffer;
	baseBuffer->resourceManager = resourceManager;
	baseBuffer->allocator = allocator;
	baseBuffer->usage = usage;
	baseBuffer->memoryHints = memoryHints;
	baseBuffer->size = size;

	return baseBuffer;
}
