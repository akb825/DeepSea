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

#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#define DS_NONE ((size_t)-1)
#define DS_BASE_PTR(allocator, index) \
	((uint8_t*)(allocator)->buffer + (index)*(allocator)->chunkSize)

size_t dsPoolAllocator_bufferSize(size_t chunkSize, size_t chunkCount)
{
	return DS_ALIGNED_SIZE(chunkSize)*chunkCount;
}

bool dsPoolAllocator_initialize(dsPoolAllocator* allocator, size_t chunkSize, size_t chunkCount,
	void* buffer, size_t bufferSize)
{
	if (!allocator || !chunkSize || !chunkCount || !buffer ||
		(uintptr_t)buffer % DS_ALLOC_ALIGNMENT != 0 ||
		bufferSize != dsPoolAllocator_bufferSize(chunkSize, chunkCount))
	{
		errno = EINVAL;
		return false;
	}

	if (!dsSpinlock_initialize(&allocator->lock))
		return false;

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;
	((dsAllocator*)allocator)->allocFunc = (dsAllocatorAllocFunction)&dsPoolAllocator_alloc;
	((dsAllocator*)allocator)->freeFunc = (dsAllocatorFreeFunction)&dsPoolAllocator_free;

	allocator->buffer = buffer;
	allocator->bufferSize = bufferSize;
	allocator->chunkSize = DS_ALIGNED_SIZE(chunkSize);
	allocator->chunkCount = chunkCount;
	allocator->head = 0;
	allocator->freeCount = chunkCount;
	allocator->initializedCount = 0;
	*(size_t*)allocator->buffer = DS_NONE;
	return true;
}

void* dsPoolAllocator_alloc(dsPoolAllocator* allocator, size_t size, unsigned int alignment)
{
	if (!allocator || !allocator->buffer || !size || alignment > DS_ALLOC_ALIGNMENT)
	{
		errno = EINVAL;
		return NULL;
	}

	// Check for tampering.
	DS_ASSERT(allocator->bufferSize > 0 &&
		allocator->bufferSize == allocator->chunkSize*allocator->chunkCount &&
		allocator->chunkSize % DS_ALLOC_ALIGNMENT == 0);

	// Size is above the chunk size.
	if (size > allocator->chunkSize)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsSpinlock_lock(&allocator->lock))
		return NULL;

	// Check if there are any free nodes available.
	void* retVal;
	if (allocator->freeCount > 0)
	{
		DS_ASSERT(allocator->head < allocator->chunkCount);
		retVal = DS_BASE_PTR(allocator, allocator->head);
		// Hook up the next head.
		size_t nextHead = *(size_t*)retVal;
		if (nextHead == DS_NONE)
		{
			if (allocator->initializedCount == allocator->chunkCount)
			{
				DS_ASSERT(allocator->freeCount == 1);
				nextHead = DS_NONE;
			}
			else
			{
				nextHead = ++allocator->initializedCount;
				if (nextHead < allocator->chunkCount)
					*(size_t*)DS_BASE_PTR(allocator, nextHead) = DS_NONE;
				else
				{
					DS_ASSERT(allocator->initializedCount == allocator->chunkCount);
					nextHead = DS_NONE;
				}
			}
		}
		--allocator->freeCount;
		DS_ASSERT((nextHead == DS_NONE && allocator->freeCount == 0) ||
			nextHead <= allocator->chunkCount);
		allocator->head = nextHead;
	}
	else
	{
		DS_ASSERT(allocator->head == DS_NONE);
		DS_ASSERT(allocator->initializedCount == allocator->chunkCount);
		errno = ENOMEM;
		retVal = NULL;
	}

	if (retVal)
	{
		((dsAllocator*)allocator)->size += allocator->chunkSize;
		++((dsAllocator*)allocator)->totalAllocations;
		++((dsAllocator*)allocator)->currentAllocations;
		DS_ASSERT(((dsAllocator*)allocator)->size <= allocator->bufferSize);
	}

	DS_VERIFY(dsSpinlock_unlock(&allocator->lock));
	return retVal;
}

bool dsPoolAllocator_free(dsPoolAllocator* allocator, void* ptr)
{
	if (!allocator || !allocator->buffer)
	{
		errno = EINVAL;
		return false;
	}

	// Check for tampering.
	DS_ASSERT(allocator->bufferSize > 0 &&
		allocator->bufferSize == allocator->chunkSize*allocator->chunkCount &&
		allocator->chunkSize % DS_ALLOC_ALIGNMENT == 0);

	// Make sure that the pointer is valid.
	uintptr_t bufferOffset = (uintptr_t)ptr - (uintptr_t)allocator->buffer;
	if (bufferOffset % allocator->chunkSize != 0)
	{
		errno = EINVAL;
		return false;
	}

	size_t index = bufferOffset/allocator->chunkSize;
	if (index >= allocator->chunkCount)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsSpinlock_lock(&allocator->lock))
		return false;

	// Add the node back to the free list.
	DS_ASSERT(allocator->initializedCount > 0);
	DS_ASSERT(allocator->freeCount < allocator->chunkCount);
	*(size_t*)ptr = allocator->head;
	allocator->head = index;
	++allocator->freeCount;

	DS_ASSERT(((dsAllocator*)allocator)->size >= allocator->chunkSize);
	((dsAllocator*)allocator)->size -= allocator->chunkSize;
	--((dsAllocator*)allocator)->currentAllocations;

	DS_VERIFY(dsSpinlock_unlock(&allocator->lock));
	return true;
}

bool dsPoolAllocator_reset(dsPoolAllocator* allocator)
{
	if (!allocator || !allocator->buffer || !allocator->chunkCount ||
		allocator->bufferSize != allocator->chunkCount*allocator->chunkSize)
	{
		errno = EINVAL;
		return false;
	}

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;

	allocator->head = 0;
	allocator->freeCount = allocator->chunkCount;
	allocator->initializedCount = 0;
	*(size_t*)allocator->buffer = DS_NONE;
	return true;
}

bool dsPoolAllocator_validate(dsPoolAllocator* allocator)
{
	if (!allocator || !allocator->buffer)
		return false;

	if (allocator->bufferSize == 0 ||
		allocator->bufferSize != allocator->chunkSize*allocator->chunkCount ||
		allocator->chunkSize % DS_ALLOC_ALIGNMENT != 0)
	{
		return false;
	}

	if (!dsSpinlock_lock(&allocator->lock))
		return false;

	bool valid = true;
	if (allocator->initializedCount > allocator->chunkCount ||
		allocator->freeCount > allocator->chunkCount)
	{
		valid = false;
	}
	else
	{
		if (allocator->head == DS_NONE)
		{
			valid = allocator->initializedCount == allocator->chunkCount &&
				allocator->freeCount == 0;
		}
		else
		{
			size_t foundNodes = 0;
			size_t next = allocator->head;
			do
			{
				if (next > allocator->chunkCount || foundNodes > allocator->freeCount)
				{
					valid = false;
					break;
				}

				next = *(size_t*)DS_BASE_PTR(allocator, next);
				foundNodes += next != DS_NONE;
			} while (next != DS_NONE);

			size_t allocatedNodes = allocator->chunkCount - allocator->freeCount;
			size_t adjustedInitialized = allocator->initializedCount;

			// The last node is considered uninitialized since it points to DS_NONE. As a result,
			// don't treat the last node in the pool as initialized.
			if (adjustedInitialized == allocator->chunkCount)
				--adjustedInitialized;

			size_t initializedFreeNodes = adjustedInitialized - allocatedNodes;
			if (initializedFreeNodes != foundNodes)
				valid = false;
		}
	}

	DS_VERIFY(dsSpinlock_unlock(&allocator->lock));
	return valid;
}

void dsPoolAllocator_destroy(dsPoolAllocator* allocator)
{
	if (!allocator || !allocator->buffer)
		return;

	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;

	allocator->buffer = NULL;
	allocator->bufferSize = 0;
	allocator->chunkSize = 0;
	allocator->head = 0;
	allocator->freeCount = 0;
	allocator->initializedCount = 0;
	dsSpinlock_destroy(&allocator->lock);
}
