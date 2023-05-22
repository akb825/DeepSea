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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if DS_APPLE
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#if DS_APPLE
#	define MALLOC_ALIGNMENT 16
#else
#	if DS_64BIT
#		define MALLOC_ALIGNMENT 16
#	else
#		define MALLOC_ALIGNMENT 8
#	endif
#endif

// Android has deprecated malloc_usable_size().
// _aligned_msize on Windows requires the alignment to be known, so need to track the size
// ourselves.
#define NO_MALLOC_SIZE DS_ANDROID || DS_WINDOWS

#if NO_MALLOC_SIZE
typedef struct dsMallocInfo
{
	uint64_t alignment : 16;
	uint64_t size : 48; // ~281 TB
} dsMallocInfo;

_Static_assert(sizeof(dsMallocInfo) == sizeof(uint64_t), "dsMallocInfo size mismatch.");
#endif

inline static size_t adjustSize(size_t size, unsigned int alignment)
{
#if NO_MALLOC_SIZE
	if (alignment < sizeof(dsMallocInfo))
		alignment = (unsigned int)sizeof(dsMallocInfo);
	return size + alignment;
#else
	DS_UNUSED(alignment);
	return size;
#endif
}

inline static void* adjustPointer(void* ptr, size_t size, unsigned int alignment)
{
#if NO_MALLOC_SIZE
	if (!ptr)
		return NULL;

#if DS_64BIT
	DS_ASSERT(size < (2ULL << 48));
#endif
	if (alignment < sizeof(dsMallocInfo))
		alignment = (unsigned int)sizeof(dsMallocInfo);
	DS_ASSERT(alignment < (2U << 16));

	dsMallocInfo* mallocInfo = (dsMallocInfo*)((uint8_t*)ptr + alignment - sizeof(dsMallocInfo));
	mallocInfo->alignment = alignment;
	mallocInfo->size = size;
	return (uint8_t*)ptr + alignment;
#else
	DS_UNUSED(size);
	DS_UNUSED(alignment);
	return ptr;
#endif
}

inline static void* originalPointer(void* ptr)
{
#if NO_MALLOC_SIZE
	if (!ptr)
		return NULL;

	size_t alignment = (size_t)((dsMallocInfo*)ptr)[-1].alignment;
	return (uint8_t*)ptr - alignment;
#else
	return ptr;
#endif
}

inline static size_t getMallocSize(void* ptr)
{
#if NO_MALLOC_SIZE
	return (size_t)((dsMallocInfo*)ptr)[-1].size;
#elif DS_APPLE
	return malloc_size(ptr);
#else
	return malloc_usable_size(ptr);
#endif
}

inline static void* mallocImpl(size_t size, unsigned int alignment)
{
	if (size == 0)
		return NULL;

	size_t allocSize = adjustSize(size, alignment);

	void* ptr;
#if DS_WINDOWS
	ptr = _aligned_malloc(allocSize, alignment);
#else
	if (alignment <= MALLOC_ALIGNMENT)
		ptr = malloc(allocSize);
	else
	{
		int errorCode = posix_memalign(&ptr, alignment, allocSize);
		if (errorCode != 0)
		{
			errno = errorCode;
			return NULL;
		}
	}
#endif

	return adjustPointer(ptr, size, alignment);
}

inline static void freeImpl(void* ptr)
{
	ptr = originalPointer(ptr);
#if DS_WINDOWS
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

inline static void* reallocImpl(void* ptr, size_t size, unsigned int alignment)
{
	size_t allocSize = adjustSize(size, alignment);

#if DS_WINDOWS
	return adjustPointer(_aligned_realloc(originalPointer(ptr), allocSize, alignment), size,
		alignment);
#else
	if (alignment <= MALLOC_ALIGNMENT)
		return adjustPointer(realloc(originalPointer(ptr), allocSize), size, alignment);

	// Fall back to allocating a new pointer and copying the results. Can re-use the existing
	// pointer if smaller.
	if (ptr && getMallocSize(ptr) >= size)
		return ptr;

	void* newPtr;
	int errorCode = posix_memalign(&newPtr, alignment, allocSize);
	if (errorCode != 0)
	{
		errno = errorCode;
		return NULL;
	}

	newPtr = adjustPointer(newPtr, size, alignment);
	if (!newPtr)
		return NULL;

	if (ptr)
	{
		size_t origSize = getMallocSize(ptr);
		size_t copySize = origSize;
		if (size < origSize)
			copySize = size;
		memcpy(newPtr, ptr, copySize);
	}

	freeImpl(ptr);
	return newPtr;
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
	((dsAllocator*)allocator)->totalAllocations = 0;
	((dsAllocator*)allocator)->currentAllocations = 0;
	((dsAllocator*)allocator)->allocFunc = (dsAllocatorAllocFunction)&dsSystemAllocator_alloc;
	((dsAllocator*)allocator)->reallocFunc = (dsAllocatorReallocFunction)&dsSystemAllocator_realloc;
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

	if (size == 0)
		return NULL;

	// Check to see if the size will exceed the limit.
	size_t allocatorSize;
	DS_ATOMIC_LOAD_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize);
	if (allocatorSize + size > allocator->limit)
	{
		errno = ENOMEM;
		return NULL;
	}

	void* ptr = mallocImpl(size, alignment);
	if (!ptr)
		return NULL;

	// Check to see if the allocated size is over the limit. (e.g. alignment padding)
	// Protect against concurrent allocations here.
	size_t allocSize = getMallocSize(ptr);
	DS_ASSERT(allocSize >= size);
	size_t updatedSize;
	do
	{
		updatedSize = allocatorSize + allocSize;
		if (updatedSize > allocator->limit)
		{
			freeImpl(ptr);
			errno = ENOMEM;
			return NULL;
		}
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize,
		&updatedSize, true));

	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->totalAllocations, 1);
	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->currentAllocations, 1);
	return ptr;
}

void* dsSystemAllocator_realloc(dsSystemAllocator* allocator, void* ptr, size_t size,
	unsigned int alignment)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (size == 0)
	{
		dsSystemAllocator_free(allocator, ptr);
		return NULL;
	}

	size_t origSize = 0;
	if (ptr)
		origSize = getMallocSize(ptr);

	// Check to see if the size will exceed the limit.
	size_t allocatorSize;
	DS_ATOMIC_LOAD_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize);
	DS_ASSERT(allocatorSize >= origSize);
	if (allocatorSize + size - origSize > allocator->limit)
	{
		errno = ENOMEM;
		return NULL;
	}

	void* newPtr = reallocImpl(ptr, size, alignment);

	/*
	 * Update the allocated size. There's a chance that the initial check above could succeed, but
	 * the final added size fails due to padding increasing the allocated size or concurrent
	 * allocations. However, we cannot recover the original pointer, so we will have to accept that
	 * the limit could be exceeded in this case.
	 */
	size_t allocSize = getMallocSize(newPtr);
	DS_ASSERT(allocSize >= size);
	size_t updatedSize;
	do
	{
		updatedSize = allocatorSize + allocSize - origSize;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE_SIZE(&((dsAllocator*)allocator)->size, &allocatorSize,
		&updatedSize, true));

	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->totalAllocations, 1);
	if (!ptr)
		DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->currentAllocations, 1);

	return newPtr;
}

bool dsSystemAllocator_free(dsSystemAllocator* allocator, void* ptr)
{
	if (!allocator)
	{
		errno = EINVAL;
		return false;
	}

	if (!ptr)
		return true;

	DS_ATOMIC_FETCH_ADD_SIZE(&((dsAllocator*)allocator)->size, -getMallocSize(ptr));
	DS_ATOMIC_FETCH_ADD32(&((dsAllocator*)allocator)->currentAllocations, -1);
	freeImpl(ptr);
	return true;
}
