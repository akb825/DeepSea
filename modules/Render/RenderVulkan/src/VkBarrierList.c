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

#include "VkBarrierList.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkBarrierList_initialize(dsVkBarrierList* barriers, dsAllocator* allocator,
	dsVkDevice* device)
{
	memset(barriers, 0, sizeof(*barriers));
	DS_ASSERT(allocator->freeFunc);
	barriers->allocator = allocator;
	barriers->device = device;
}

bool dsVkBarrierList_addBufferBarrier(dsVkBarrierList* barriers, VkBuffer buffer,
	size_t offset, size_t size, dsGfxBufferUsage usage)
{
	uint32_t index = barriers->bufferBarrierCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(barriers->allocator, barriers->bufferBarriers,
		barriers->bufferBarrierCount, barriers->maxBufferBarriers, 1))
	{
		return false;
	}

	VkBufferMemoryBarrier* barrier = barriers->bufferBarriers + index;
	barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier->pNext = NULL;
	barrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier->dstAccessMask = 0;
	if (usage & dsGfxBufferUsage_Index)
		barrier->dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
	if (usage & dsGfxBufferUsage_Vertex)
		barrier->dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (usage & (dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch))
		barrier->dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_MutableImage))
	{
		barrier->dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyFrom)
		barrier->dstAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
	barrier->srcQueueFamilyIndex = barriers->device->queueFamilyIndex;
	barrier->dstQueueFamilyIndex = barriers->device->queueFamilyIndex;
	barrier->buffer = buffer;
	barrier->offset = offset;
	barrier->size = size;

	return true;
}

void dsVkBarrierList_clear(dsVkBarrierList* barriers)
{
	barriers->bufferBarrierCount = 0;
}

void dsVkBarrierList_shutdown(dsVkBarrierList* barriers)
{
	dsAllocator_free(barriers->allocator, barriers->bufferBarriers);
}
