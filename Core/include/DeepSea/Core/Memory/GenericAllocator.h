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

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Structure that defines a generic memory allocator.
 */

/**
 * @brief Structure that defines a generic memory allocator.
 */
typedef struct dsGenericAllocator dsGenericAllocator;

/**
 * @brief Function for allocating from the allocator.
 *
 * This should update the size for the allocator.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsGenericAllocFunction)(dsGenericAllocator* allocator, size_t size);

/**
 * @brief Function for allocating an aligned pointer from the allocator.
 *
 * This should update the size for the allocator.
 * @param allocator The allocator to allocate from.
 * @param alignment The alignement of the pointer.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsGenericAlignedAllocFunction)(dsGenericAllocator* allocator, size_t alignment,
	size_t size);

/**
 * @brief Function for re-allocating memory frm the allocator with the same semantics as realloc().
 *
 * This should update the size for the allocator.
 * @param allocator The allocator to allocate from.
 * @param ptr The original memory pointer.
 * @param size The new size.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsGenericReallocFunction)(dsGenericAllocator* allocator, void* ptr, size_t size);

/**
 * @brief Function for freeing memory from the allocator.
 *
 * This should update the size for the allocator.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
typedef bool (*dsGenericFreeFunction)(dsGenericAllocator* allocator, void* ptr);

/**
 * @brief Function for destroying the user data for the allocator.
 * @param userData The user data for the allocator.
 */
typedef void (*dsGenericAllocatorDestroyFunction)(void* userData);

/** @copydoc dsGenericAllocator */
struct dsGenericAllocator
{
	/**
	 * @brief User data associated with the allocator.
	 */
	void* userData;

	/**
	 * @brief The current size of allocated memory.
	 */
	size_t size;

	/**
	 * @brief The allocation function.
	 */
	dsGenericAllocFunction allocFunc;

	/**
	 * @brief The aligned allocation function.
	 */
	dsGenericAlignedAllocFunction alignedAllocFunc;

	/**
	 * @brief The re-allocation function.
	 */
	dsGenericReallocFunction reallocFunc;

	/**
	 * @brief The free function.
	 */
	dsGenericFreeFunction freeFunc;

	/**
	 * @brief The destructor function.
	 */
	dsGenericAllocatorDestroyFunction destroyFunc;
};

/**
 * @brief Initializes an allocator.
 * @param[out] allocator The allocator to initialize.
 * @param userData The user data for the allocator. This may be NULL.
 * @param allocFunc The allocation function.
 * @param alignedAllocFunc The aligned allocation function.
 * @param reallocFunc The re-allocation function.
 * @param freeFunc The free function.
 * @param destroyFunc The destructor function for the user data. This may be NULL.
 * @return True if the allocator is valid.
 */
bool dsGenericAllocator_initialize(dsGenericAllocator* allocator, void* userData,
	dsGenericAllocFunction allocFunc, dsGenericAlignedAllocFunction alignedAllocFunc,
	dsGenericReallocFunction reallocFunc, dsGenericFreeFunction freeFunc,
	dsGenericAllocatorDestroyFunction destroyFunc);

/**
 * @brief Allocates memory from the allocator.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGenericAllocator_alloc(dsGenericAllocator* allocator, size_t size);

/**
 * @brief Allocates aligned memory from the allocator.
 * @param allocator The allocator to allocate from.
 * @param alignment The alignement of the pointer.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGenericAllocator_alignedAlloc(dsGenericAllocator* allocator, size_t alignment, size_t size);

/**
 * @brief Reallocates memory from the allocator with the same semantics as realloc().
 * @param allocator The allocator to allocate from.
 * @param ptr The original memory pointer.
 * @param size The new size.
 * @return The allocated memory or NULL if an error occured.
 */
void* dsGenericAllocator_realloc(dsGenericAllocator* allocator, void* ptr, size_t size);

/**
 * @brief Frees memory from the allocator.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
bool dsGenericAllocator_free(dsGenericAllocator* allocator, void* ptr);

/**
 * @brief Destroys an allocator.
 * @param allocator The allocator to destroy.
 */
void dsGenericAllocator_destroy(dsGenericAllocator* allocator);

inline bool dsGenericAllocator_initialize(dsGenericAllocator* allocator, void* userData,
	dsGenericAllocFunction allocFunc, dsGenericAlignedAllocFunction alignedAllocFunc,
	dsGenericReallocFunction reallocFunc, dsGenericFreeFunction freeFunc,
	dsGenericAllocatorDestroyFunction destroyFunc)
{
	if (!allocator || !allocFunc || !alignedAllocFunc || !reallocFunc)
		return false;

	allocator->userData = userData;
	allocator->size = 0;
	allocator->allocFunc = allocFunc;
	allocator->alignedAllocFunc = alignedAllocFunc;
	allocator->reallocFunc = reallocFunc;
	allocator->freeFunc = freeFunc;
	allocator->destroyFunc = destroyFunc;

	return true;
}

inline void* dsGenericAllocator_alloc(dsGenericAllocator* allocator, size_t size)
{
	if (!allocator || !allocator->allocFunc)
		return NULL;
	return allocator->allocFunc(allocator, size);
}

inline void* dsGenericAllocator_alignedAlloc(dsGenericAllocator* allocator, size_t alignment,
	size_t size)
{
	if (!allocator || !allocator->alignedAllocFunc)
		return NULL;
	return allocator->alignedAllocFunc(allocator, alignment, size);
}

inline void* dsGenericAllocator_realloc(dsGenericAllocator* allocator, void* ptr, size_t size)
{
	if (!allocator || !allocator->reallocFunc)
		return NULL;
	return allocator->reallocFunc(allocator, ptr, size);
}

inline bool dsGenericAllocator_free(dsGenericAllocator* allocator, void* ptr)
{
	if (!allocator || !allocator->freeFunc)
		return false;
	return allocator->freeFunc(allocator, ptr);
}

inline void dsGenericAllocator_destroy(dsGenericAllocator* allocator)
{
	if (!allocator || !allocator->destroyFunc)
		return;
	allocator->destroyFunc(allocator->userData);
}

#ifdef __cplusplus
}
#endif
