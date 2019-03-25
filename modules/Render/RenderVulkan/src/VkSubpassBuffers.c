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

#include "VkSubpassBuffers.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

void dsVkSubpassBuffers_initialize(dsVkSubpassBuffers* buffers, dsAllocator* allocator)
{
	buffers->allocator = allocator;
	buffers->commandBuffers = NULL;
	buffers->commandBufferCount = 0;
	buffers->maxCommandBuffers = 0;
	buffers->subpasses = NULL;
	buffers->subpassCount = 0;
	buffers->maxSubpasses = 0;
}

bool dsVkSubpassBuffers_addSubpass(dsVkSubpassBuffers* buffers)
{
	uint32_t index = buffers->subpassCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(buffers->allocator, buffers->subpasses, buffers->subpassCount,
			buffers->maxSubpasses, 1))
	{
		return false;
	}

	buffers->subpasses[index].start = buffers->commandBufferCount;
	buffers->subpasses[index].count = 0;
	return true;
}

bool dsVkSubpassBuffers_addCommandBuffer(dsVkSubpassBuffers* buffers, VkCommandBuffer commandBuffer)
{
	if (buffers->subpassCount == 0)
		return false;

	uint32_t index = buffers->commandBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(buffers->allocator, buffers->commandBuffers,
			buffers->commandBufferCount, buffers->maxCommandBuffers, 1))
	{
		return false;
	}

	buffers->commandBuffers[index] = commandBuffer;
	++buffers->subpasses[buffers->subpassCount - 1].count;
	return true;
}

void dsVkSubpassBuffers_reset(dsVkSubpassBuffers* buffers)
{
	buffers->commandBufferCount = 0;
	buffers->subpassCount = 0;
}

void dsVkSubpassBuffers_shutdown(dsVkSubpassBuffers* buffers)
{
	if (!buffers->allocator)
		return;

	DS_VERIFY(dsAllocator_free(buffers->allocator, buffers->commandBuffers));
	DS_VERIFY(dsAllocator_free(buffers->allocator, buffers->subpasses));
}
