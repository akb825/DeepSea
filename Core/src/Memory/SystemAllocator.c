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

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Atomic.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#if DS_APPLE
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

static size_t getMallocSize(void* ptr)
{
#if DS_WINDOWS
	return _aligned_msize(ptr, DS_ALLOC_ALIGNMENT, 0);
#elif DS_APPLE
	return malloc_size(ptr);
#else
	return malloc_usable_size(ptr);
#endif
}

bool dsSystemAllocator_initialize(dsSystemAllocator* allocator, size_t limit)
{
	if (!allocator || limit == 0)
	{
		errno = EINVAL;
		return false;
	}

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->allocFunc = (dsAllocatorAllocFunction)&dsSystemAllocator_alloc;
	((dsAllocator*)allocator)->freeFunc = (dsAllocatorFreeFunction)&dsSystemAllocator_free;
	allocator->limit = limit;
	return true;
}

void* dsSystemAllocator_alloc(dsSystemAllocator* allocator, size_t size, unsigned int alignment)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	// Check to see if the size will exceed the limit.
	size_t allocatorSize;
	DS_ATOMIC_LOAD_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize);
	if (allocatorSize + size > allocator->limit)
	{
		errno = ENOMEM;
		return NULL;
	}

	void* ptr;
#if DS_WINDOWS
	ptr = _aligned_malloc(size, alignment);
#else
	int errorCode = posix_memalign(&ptr, alignment, size);
	if (errorCode != 0)
	{
		errno = errorCode;
		return NULL;
	}
#endif

	if (!ptr)
		return NULL;

	// Check to see if the allocated size is over the limit. (e.g. alignment padding)
	// Protect against concurrent allocations here.
	size_t allocSize = getMallocSize(ptr);
	size_t updatedSize;
	do
	{
		updatedSize = allocatorSize + allocSize;
		if (updatedSize > allocator->limit)
		{
	#if DS_WINDOWS
			_aligned_free(ptr);
	#else
			free(ptr);
	#endif
			errno = ENOMEM;
			return NULL;
		}
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize,
		&updatedSize, true));
	return ptr;
}

bool dsSystemAllocator_free(dsSystemAllocator* allocator, void* ptr)
{
	if (!allocator)
	{
		errno = EINVAL;
		return false;
	}

	if (ptr)
	{
		DS_ATOMIC_FETCH_ADD_SIZE(&((dsAllocator*)allocator)->size, -getMallocSize(ptr));

#if DS_WINDOWS
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}

	return true;
}
