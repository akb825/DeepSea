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

#include "VkBarrierList.h"
#include "VkShared.h"
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
	VkDeviceSize offset, VkDeviceSize size, dsGfxBufferUsage srcUsage, dsGfxBufferUsage dstUsage,
	bool canMap)
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
	barrier->srcAccessMask = dsVkWriteBufferAccessFlags(srcUsage, canMap);
	barrier->dstAccessMask = dsVkReadBufferAccessFlags(dstUsage) |
		dsVkWriteBufferAccessFlags(dstUsage, canMap);
	barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->buffer = buffer;
	barrier->offset = offset;
	barrier->size = size;

	return true;
}

bool dsVkBarrierList_addImageBarrier(dsVkBarrierList* barriers, VkImage image,
	const VkImageSubresourceRange* range, dsTextureUsage srcUsage, bool host,
	bool offscreen, bool depthStencil, dsTextureUsage dstUsage, VkImageLayout oldLayout,
	VkImageLayout newLayout)
{
	uint32_t index = barriers->imageBarrierCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(barriers->allocator, barriers->imageBarriers,
		barriers->imageBarrierCount, barriers->maxImageBarriers, 1))
	{
		return false;
	}

	VkImageMemoryBarrier* barrier = barriers->imageBarriers + index;
	barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier->pNext = NULL;
	barrier->srcAccessMask = host ? VK_ACCESS_HOST_WRITE_BIT :
		dsVkWriteImageAccessFlags(srcUsage, false, false);
	barrier->dstAccessMask = dsVkReadImageAccessFlags(dstUsage);
	if (oldLayout != newLayout)
		barrier->dstAccessMask |= dsVkWriteImageAccessFlags(srcUsage, offscreen, depthStencil);
	barrier->oldLayout = oldLayout;
	barrier->newLayout = newLayout;
	barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->image = image;
	barrier->subresourceRange = *range;

	return true;
}

void dsVkBarrierList_clear(dsVkBarrierList* barriers)
{
	barriers->bufferBarrierCount = 0;
	barriers->imageBarrierCount = 0;
}

void dsVkBarrierList_shutdown(dsVkBarrierList* barriers)
{
	dsAllocator_free(barriers->allocator, barriers->bufferBarriers);
	dsAllocator_free(barriers->allocator, barriers->imageBarriers);
}
