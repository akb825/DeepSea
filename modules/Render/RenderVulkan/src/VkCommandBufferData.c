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

#include "VkCommandBufferData.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkCommandBufferData_initialize(dsVkCommandBufferData* bufferData, dsAllocator* allocator,
	dsVkDevice* device, VkCommandPool commandPool, bool renderPass)
{
	DS_ASSERT(allocator->freeFunc);
	bufferData->allocator = allocator;
	bufferData->device = device;
	bufferData->commandPool = commandPool;
	bufferData->chunks = NULL;
	bufferData->chunkCount = 0;
	bufferData->maxChunks = 0;
	bufferData->activeChunk = 0;
	bufferData->renderPass = renderPass;
}

VkCommandBuffer dsVkCommandBufferData_getCommandBuffer(dsVkCommandBufferData* bufferData)
{
	if (bufferData->activeChunk < bufferData->chunkCount)
	{
		dsVkCommandBufferChunk* chunk = bufferData->chunks[bufferData->activeChunk];
		DS_ASSERT(chunk->nextBuffer < DS_COMMAND_BUFFER_CHUNK_SIZE);
		VkCommandBuffer commandBuffer = chunk->commandBuffers[chunk->nextBuffer];
		if (++chunk->nextBuffer == DS_COMMAND_BUFFER_CHUNK_SIZE)
			++bufferData->activeChunk;
		return commandBuffer;
	}

	uint32_t index = bufferData->chunkCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(bufferData->allocator, bufferData->chunks, bufferData->chunkCount,
		bufferData->maxChunks, 1))
	{
		return 0;
	}

	dsVkCommandBufferChunk* chunk = DS_ALLOCATE_OBJECT(bufferData->allocator,
		dsVkCommandBufferChunk);
	if (!chunk)
	{
		--bufferData->chunkCount;
		return 0;
	}

	dsVkDevice* device = bufferData->device;

	VkCommandBufferAllocateInfo allocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		bufferData->commandPool,
		bufferData->renderPass ? VK_COMMAND_BUFFER_LEVEL_SECONDARY :
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		DS_COMMAND_BUFFER_CHUNK_SIZE
	};

	VkResult result = DS_VK_CALL(device->vkAllocateCommandBuffers)(device->device, &allocateInfo,
		chunk->commandBuffers);
	if (!dsHandleVkResult(result))
	{
		DS_VERIFY(dsAllocator_free(bufferData->allocator, chunk));
		--bufferData->chunkCount;
		return 0;
	}

	chunk->nextBuffer = 1;
	VkCommandBuffer commandBuffer = chunk->commandBuffers[0];
	bufferData->chunks[index] = chunk;
	return commandBuffer;
}

void dsVkCommandBufferData_reset(dsVkCommandBufferData* bufferData)
{
	for (uint32_t i = 0; i < bufferData->chunkCount; ++i)
		bufferData->chunks[i]->nextBuffer = 0;
	bufferData->activeChunk = 0;
}

void dsVkCommandBufferData_shutdown(dsVkCommandBufferData* bufferData)
{
	for (uint32_t i = 0; i < bufferData->chunkCount; ++i)
		DS_VERIFY(dsAllocator_free(bufferData->allocator, bufferData->chunks[i]));
	DS_VERIFY(dsAllocator_free(bufferData->allocator, bufferData->chunks));
}
