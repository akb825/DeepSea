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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Thread/Types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used by the Memory portion of the DeepSea/Coroe library.
 */

/**
 * @brief Define for no limit during allocations.
 */
#define DS_ALLOCATOR_NO_LIMIT (size_t)-1

/**
 * @brief Structure that defines a memory allocator.
 *
 * This can be "subclassed" by having it as the first member of other allocator structures. This can
 * be done to add additional data to the allocator and have it be freely casted between the
 * dsAllocator and the true allocator type.
 *
 * @see Allocator.h
 */
typedef struct dsAllocator dsAllocator;

/**
 * @brief Function for allocating from the allocator.
 *
 * The allocated memory must be at least 16-byte aligned.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @param alignment The minimum alignment of the allocation. When called by DeepSea (such as with
 *     dsAllocator_alloc()) it will be DS_ALLOC_ALIGNMENT. If the allocator is interfaced with
 *     external libraries (e.g. Vulkan, physics libraries) it may have a different minimum
 *     alignment.
 * @return The allocated memory or NULL if an error occured. errno should be set if an error
 *     occurred.
 */
typedef void* (*dsAllocatorAllocFunction)(dsAllocator* allocator, size_t size,
	unsigned int alignment);

/**
 * @brief Function for freeing memory from the allocator.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed. errno should be set if false is returned.
 */
typedef bool (*dsAllocatorFreeFunction)(dsAllocator* allocator, void* ptr);

/** @copydoc dsAllocator */
struct dsAllocator
{
	/**
	 * @brief The current size of allocated memory.
	 *
	 * If this is used to allocate across different threads, it should be accessed with
	 * DS_ATOMIC_LOAD_SIZE.
	 */
	size_t size;

	/**
	 * @brief The total number of allocations over the lifetime of the allocator.
	 *
	 * If this is used to allocate across different threads, it should be accessed with
	 * DS_ATOMIC_LOAD_SIZE.
	 */
	uint32_t totalAllocations;

	/**
	 * @brief The current number of active allocations.
	 *
	 * If this is used to allocate across different threads, it should be accessed with
	 * DS_ATOMIC_LOAD_SIZE.
	 */
	uint32_t currentAllocations;

	/**
	 * @brief The allocation function.
	 */
	dsAllocatorAllocFunction allocFunc;

	/**
	 * @brief The free function.
	 *
	 * If this function is NULL, then the memory allocated from allocFunc should not be freed. The
	 * allocator itself may also be temporary (such as with dsBufferAllocator) and pointers
	 * shouldn't be kept for later use.
	 */
	dsAllocatorFreeFunction freeFunc;
};

/**
 * @brief Structure for a system allocator.
 *
 * This is effectively a subclass of dsAllocator and a pointer to dsSystemAllocator can be freely
 * cast between the two types.
 *
 * @remark This allows alignments of any size that is a power of two, so it may be used with
 * external libraries with greater alignment requirements.
 *
 * @see SystemAllocator.h
 */
typedef struct dsSystemAllocator
{
	/**
	 * @brief The base allocator.
	 */
	dsAllocator allocator;

	/**
	 * @brief The limit for the allocator.
	 *
	 * If an allocation would bring the size over the limit, the allocation will fail. Set to
	 * DS_ALLOCATOR_NO_LIMIT to have no limit.
	 */
	size_t limit;
} dsSystemAllocator;

/**
 * @brief Structure for a buffer allocator.
 *
 * This is effectively a subclass of dsAllocator and a pointer to dsSystemAllocator can be freely
 * cast between the two types.
 *
 * A buffer is pre-allocated, then memory is taken from it sequentially. Memory is never freed back
 * to the buffer.
 *
 * The size member of dsAllocator will be used for the offset of the next memory allocated from the
 * buffer. Adjusting the size member can be used to reset where future allocations occur.
 *
 * @see BufferAllocator.h
 */
typedef struct dsBufferAllocator
{
	/**
	 * @brief The base allocator.
	 */
	dsAllocator allocator;

	/**
	 * @brief The buffer that memory is taken from.
	 */
	void* buffer;

	/**
	 * @brief The full size of the buffer.
	 */
	size_t bufferSize;
} dsBufferAllocator;

/**
 * @brief Structure for a pool allocator, which allocates fixed chunks from a pool of memory.
 *
 * This is effectively a subclass of dsAllocator and a pointer to dsSystemAllocator can be freely
 * cast between the two types.
 *
 * @remark Manually changing the values in this structure can cause bad memory access.
 *
 * @see PoolAllocator.h
 */
typedef struct dsPoolAllocator
{
	/**
	 * @brief The base allocator.
	 */
	dsAllocator allocator;

	/**
	 * @brief The buffer that memory is taken from.
	 */
	void* buffer;

	/**
	 * @brief The full size of the buffer.
	 */
	size_t bufferSize;

	/**
	 * @brief The size of a chunk.
	 */
	size_t chunkSize;

	/**
	 * @brief The number of available chunks.
	 */
	size_t chunkCount;

	/**
	 * @brief Index of the head chunk.
	 */
	size_t head;

	/**
	 * @brief The number of free chunks.
	 */
	size_t freeCount;

	/**
	 * @brief The number of initialized chunks.
	 */
	size_t initializedCount;

	/**
	 * @brief Lock used to protect allocation.
	 */
	dsSpinlock lock;
} dsPoolAllocator;

#ifdef __cplusplus
}
#endif
