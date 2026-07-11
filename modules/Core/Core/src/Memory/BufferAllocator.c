/*
 * Copyright 2016-2026 Aaron Barany
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

	dsAllocator* baseAllocator = (dsAllocator*)allocator;
	baseAllocator->size = 0;
	baseAllocator->totalAllocations = 0;
	baseAllocator->currentAllocations = 0;
	baseAllocator->allocFunc = (dsAllocatorAllocFunction)&dsBufferAllocator_alloc;
	baseAllocator->reallocFunc = NULL;
	baseAllocator->freeFunc = NULL;

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

	dsAllocator* baseAllocator = (dsAllocator*)allocator;
	size_t alignmentOffset = 0;
	uintptr_t bufferRem = ((uintptr_t)allocator->buffer & (alignment - 1));
	if (bufferRem > 0)
		alignmentOffset = alignment - bufferRem;

	// Use atomic operations to allow for thread safety.
	size_t curSize, offset, nextSize;
	DS_ATOMIC_LOAD_SIZE(&baseAllocator->size, &curSize);
	do
	{
		size_t alignedCurSize = DS_ALIGNED_SIZE(curSize, alignment);
		if (alignedCurSize < curSize || !DS_CAN_ADD_SIZES(alignmentOffset, alignedCurSize))
		{
			errno = ERANGE;
			return NULL;
		}

		offset = alignmentOffset + alignedCurSize;
		if (!DS_CAN_ADD_SIZES(offset, size) || offset + size > allocator->bufferSize)
		{
			errno = ENOMEM;
			return NULL;
		}

		nextSize = offset + size;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE_SIZE(&baseAllocator->size, &curSize, &nextSize, true));

	DS_ATOMIC_FETCH_ADD32(&baseAllocator->totalAllocations, 1);
	DS_ATOMIC_FETCH_ADD32(&baseAllocator->currentAllocations, 1);
	return (uint8_t*)allocator->buffer + offset;
}

bool dsBufferAllocator_reset(dsBufferAllocator* allocator)
{
	if (!allocator || !allocator->buffer || !allocator->bufferSize)
	{
		errno = EINVAL;
		return false;
	}

	dsAllocator* baseAllocator = (dsAllocator*)allocator;
	baseAllocator->size = 0;
	baseAllocator->totalAllocations = 0;
	baseAllocator->currentAllocations = 0;
	return true;
}
