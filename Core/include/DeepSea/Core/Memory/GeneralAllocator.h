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
#include <DeepSea/Core/Memory/Allocator.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Structure that defines a general memory allocator.
 *
 * This is for a full featured allocator that supports allocation, reallocation, aligned allocation,
 * and freeing. The most common implementation would be a general purpose heap. (such as malloc)
 * This should be used for allocating pools of memory for other allocator types or systems that
 * cannot have constraints based on pre-existing knowledge.
 */

/**
 * @brief Structure that defines a general memory allocator.
 *
 * This effectively subclasses from dsAllocator and a dsGenericAllocator pointer can be freely
 * cast between the two types.
 */
typedef struct dsGeneralAllocator dsGeneralAllocator;

/**
 * @brief Function for allocating an aligned pointer from the allocator.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to allocate from.
 * @param alignment The alignement of the pointer.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsGeneralAllocatorAlignedAllocFunction)(dsGeneralAllocator* allocator,
	size_t alignment, size_t size);

/**
 * @brief Function for re-allocating memory frm the allocator with the same semantics as realloc().
 *
 * This should update the size for the allocator.
 * @param allocator The allocator to allocate from.
 * @param ptr The original memory pointer.
 * @param size The new size.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsGeneralAllocatorReallocFunction)(dsGeneralAllocator* allocator, void* ptr,
	size_t size);

/** @copydoc dsGeneralAllocator */
struct dsGeneralAllocator
{
	/**
	 * @brief The base allocator.
	 *
	 * This effectively subclasses from dsAllocator.
	 */
	dsAllocator allocator;

	/**
	 * @brief The aligned allocation function.
	 */
	dsGeneralAllocatorAlignedAllocFunction alignedAllocFunc;

	/**
	 * @brief The re-allocation function.
	 */
	dsGeneralAllocatorReallocFunction reallocFunc;
};

/**
 * @brief Allocates memory from the allocator.
 * @remark This is the same as caclling dsAllocator_alloc() and is provided for convenience to avoid
 * casting.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGeneralAllocator_alloc(dsGeneralAllocator* allocator, size_t size);

/**
 * @brief Allocates aligned memory from the allocator.
 * @param allocator The allocator to allocate from.
 * @param alignment The alignement of the pointer.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGeneralAllocator_alignedAlloc(dsGeneralAllocator* allocator, size_t alignment, size_t size);

/**
 * @brief Reallocates memory from the allocator with the same semantics as realloc().
 * @param allocator The allocator to allocate from.
 * @param ptr The original memory pointer.
 * @param size The new size.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGeneralAllocator_realloc(dsGeneralAllocator* allocator, void* ptr, size_t size);

/**
 * @brief Frees memory from the allocator.
 * @remark This is the same as caclling dsAllocator_free() and is provided for convenience to avoid
 * casting.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
bool dsGeneralAllocator_free(dsGeneralAllocator* allocator, void* ptr);

inline void* dsGeneralAllocator_alloc(dsGeneralAllocator* allocator, size_t size)
{
	return dsAllocator_alloc((dsAllocator*)allocator, size);
}

inline void* dsGeneralAllocator_alignedAlloc(dsGeneralAllocator* allocator, size_t alignment,
	size_t size)
{
	if (!allocator || !allocator->alignedAllocFunc)
		return NULL;
	return allocator->alignedAllocFunc(allocator, alignment, size);
}

inline void* dsGeneralAllocator_realloc(dsGeneralAllocator* allocator, void* ptr, size_t size)
{
	if (!allocator || !allocator->reallocFunc)
		return NULL;
	return allocator->reallocFunc(allocator, ptr, size);
}

inline bool dsGeneralAllocator_free(dsGeneralAllocator* allocator, void* ptr)
{
	return dsAllocator_free((dsAllocator*)allocator, ptr);
}

#ifdef __cplusplus
}
#endif
