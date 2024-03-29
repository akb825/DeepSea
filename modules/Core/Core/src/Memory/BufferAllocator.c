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

#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

bool dsBufferAllocator_initialize(dsBufferAllocator* allocator, void* buffer, size_t bufferSize)
{
	if (!allocator || !buffer || !bufferSize || (uintptr_t)buffer % DS_ALLOC_ALIGNMENT != 0)
	{
		errno = EINVAL;
		return false;
	}

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;
	((dsAllocator*)allocator)->allocFunc = (dsAllocatorAllocFunction)&dsBufferAllocator_alloc;
	((dsAllocator*)allocator)->reallocFunc = NULL;
	((dsAllocator*)allocator)->freeFunc = NULL;

	allocator->buffer = buffer;
	allocator->bufferSize = bufferSize;
	return true;
}

void* dsBufferAllocator_alloc(dsBufferAllocator* allocator, size_t size, unsigned int alignment)
{
	if (!allocator || size == 0 || alignment == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t alignmentOffset = 0;
	uintptr_t bufferRem = ((uintptr_t)allocator->buffer & (alignment - 1));
	if (bufferRem > 0)
		alignmentOffset = alignment - bufferRem;

	// Use atomic operations to allow for thread safety.
	// PTR is the same size is size_t.
	size_t curSize, offset, nextSize;
	DS_ATOMIC_LOAD_SIZE(&((dsAllocator*)allocator)->size, &curSize);
	do
	{
		offset = alignmentOffset + DS_CUSTOM_ALIGNED_SIZE(curSize, alignment);
		if (offset + size > allocator->bufferSize)
		{
			errno = ENOMEM;
			return NULL;
		}

		nextSize = offset + size;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE_SIZE(&((dsAllocator*)allocator)->size, &curSize, &nextSize,
		true));

	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->totalAllocations, 1);
	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->currentAllocations, 1);
	return (uint8_t*)allocator->buffer + offset;
}

bool dsBufferAllocator_reset(dsBufferAllocator* allocator)
{
	if (!allocator || !allocator->buffer || !allocator->bufferSize)
	{
		errno = EINVAL;
		return false;
	}

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;
	return true;
}
