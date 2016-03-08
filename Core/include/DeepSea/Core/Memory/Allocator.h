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
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Structure that defines a memory allocator.
 *
 * The base memory allocator only supports allocating and deallocating an object. Most systems
 * should use dsAllocator rather than the more general dsGeneralAllocator. This allows objects to be
 * created with special-purpose allocators based on how memory is managed by the system managers.
 */

/**
 * @brief Structure that defines a memory allocator.
 *
 * This can be "subclassed" by having it as the first member of other allocator structures. This can
 * be done to add additional data to the allocator and have it be freely casted between the
 * dsAllocator and the true allocator type.
 */
typedef struct dsAllocator dsAllocator;

/**
 * @brief Function for allocating from the allocator.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsAllocatorAllocFunction)(dsAllocator* allocator, size_t size);

/**
 * @brief Function for freeing memory from the allocator.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
typedef bool (*dsAllocatorFreeFunction)(dsAllocator* allocator, void* ptr);

/** @copydoc dsAllocator */
struct dsAllocator
{
	/**
	 * @brief The current size of allocated memory.
	 */
	size_t size;

	/**
	 * @brief The allocation function.
	 */
	dsAllocatorAllocFunction allocFunc;

	/**
	 * @brief The free function.
	 */
	dsAllocatorFreeFunction freeFunc;
};

/**
 * @brief Allocates memory from the allocator.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsAllocator_alloc(dsAllocator* allocator, size_t size);

/**
 * @brief Frees memory from the allocator.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
bool dsAllocator_free(dsAllocator* allocator, void* ptr);

inline void* dsAllocator_alloc(dsAllocator* allocator, size_t size)
{
	if (!allocator || !allocator->allocFunc)
		return NULL;
	return allocator->allocFunc(allocator, size);
}

inline bool dsAllocator_free(dsAllocator* allocator, void* ptr)
{
	if (!allocator || !allocator->freeFunc)
		return false;
	return allocator->freeFunc(allocator, ptr);
}

#ifdef __cplusplus
}
#endif
