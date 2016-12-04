/*
 * Copyright 2016 Aaron Barany
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

#include "MockGfxBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

typedef struct dsMockGfxBuffer
{
	dsGfxBuffer buffer;
	uint8_t data[];
} dsMockGfxBuffer;

dsGfxBuffer* dsMockGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, size_t size, const void* data)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	dsMockGfxBuffer* buffer = (dsMockGfxBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsMockGfxBuffer) + size);
	if (!buffer)
		return NULL;

	buffer->buffer.resourceManager = resourceManager;
	if (allocator->freeFunc)
		buffer->buffer.allocator = allocator;
	buffer->buffer.usage = (dsGfxBufferUsage)usage;
	buffer->buffer.memoryHints = (dsGfxMemory)memoryHints;
	buffer->buffer.size = size;
	if (data)
		memcpy(buffer->data, data, size);

	return &buffer->buffer;
}

void* dsMockGfxBuffer_map(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer, int flags,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(flags);
	DS_ASSERT(buffer);
	if (size == DS_MAP_FULL_BUFFER)
		size = buffer->buffer.size;
	DS_ASSERT(offset + size <= buffer->buffer.size);
	return buffer->data + offset;
}

bool dsMockGfxBuffer_unmap(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	return true;
}

bool dsMockGfxBuffer_flush(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);
	return true;
}

bool dsMockGfxBuffer_invalidate(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);
	return true;
}

bool dsMockGfxBuffer_copyData(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer,
	size_t offset, size_t size, const void* data)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(buffer);
	DS_ASSERT(offset + size <= buffer->buffer.size);
	DS_ASSERT(data);
	memcpy(buffer->data + offset, data, size);
	return true;
}

bool dsMockGfxBuffer_copy(dsResourceManager* resourceManager, dsMockGfxBuffer* srcBuffer,
	size_t srcOffset, dsMockGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(srcBuffer);
	DS_ASSERT(srcOffset + size <= srcBuffer->buffer.size);
	DS_ASSERT(dstBuffer);
	DS_ASSERT(dstOffset + size <= dstBuffer->buffer.size);
	memcpy(dstBuffer->data + dstOffset, srcBuffer->data + srcOffset, size);
	return true;
}

bool dsMockGfxBuffer_destroy(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	if (buffer->buffer.allocator)
		return dsAllocator_free(buffer->buffer.allocator, buffer);
	return true;
}
