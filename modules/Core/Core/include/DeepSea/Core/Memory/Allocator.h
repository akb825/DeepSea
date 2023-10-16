/*
 * Copyright 2016-2023 Aaron Barany
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

#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with a base allocator.
 * @see dsAllocator
 */

/**
 * @brief Macro to allocate an object and return it as a pointer to that object type.
 * @remark errno will be set on failure.
 * @param allocator The allocator. This will be cast to dsAllocator* so specific allocator types
 *     (e.g. dsBufferAllocator, dsPoolAllocator) can be provided without an explicit cast.
 * @param type The type to allocate.
 * @return The allocated object, or NULL if the allocation failed.
 */
#define DS_ALLOCATE_OBJECT(allocator, type) ((type*)dsAllocator_alloc( \
		(dsAllocator*)(allocator), sizeof(type)))

/**
 * @brief Macro to allocate an array of objects and return it as a pointer to that object type.
 * @remark errno will be set on failure.
 * @param allocator The allocator. This will be cast to dsAllocator* so specific allocator types
 *     (e.g. dsBufferAllocator, dsPoolAllocator) can be provided without an explicit cast.
 * @param type The type to allocate.
 * @param count The number of objects to allocate.
 * @return The allocated array, or NULL if the allocation failed.
 */
#define DS_ALLOCATE_OBJECT_ARRAY(allocator, type, count) ((type*)dsAllocator_alloc( \
		(dsAllocator*)(allocator), sizeof(type)*count))

/**
 * @brief Allocates memory from the allocator.
 *
 * The alignment of the returned pointer will be aligned by DS_ALLOC_ALIGNMENT.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occurred or size is 0.
 */
DS_CORE_EXPORT inline void* dsAllocator_alloc(dsAllocator* allocator, size_t size);

/**
 * @brief Allocates aligned memory from the allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @param alignment The alignment to allocate. This must be a power of two.
 * @return The allocated memory or NULL if an error occurred or size is 0.
 */
DS_CORE_EXPORT inline void* dsAllocator_alignedAlloc(dsAllocator* allocator, size_t size,
	unsigned int alignment);

/**
 * @brief Re-allocates memory from the allocator.
 *
 * The alignment of the returned pointer will be aligned by DS_ALLOC_ALIGNMENT.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param ptr The pointer to reallocate. If NULL, a new pointer will be allocated.
 * @param size The size to allocate. If 0, it will free the pointer.
 * @return The allocated memory or NULL if an error occurred or size is 0. The original pointer will
 *     remain intact if an error occurred.
 */
DS_CORE_EXPORT inline void* dsAllocator_realloc(dsAllocator* allocator, void* ptr, size_t size);

/**
 * @brief Re-allocates memory from the allocator, falling back to an allocate + copy + free if
 * reallocation isn't supported by the allocator.
 *
 * The alignment of the returned pointer will be aligned by DS_ALLOC_ALIGNMENT.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param ptr The pointer to reallocate. If NULL, a new pointer will be allocated.
 * @param origSize The original size of the allocated memory. This may be smaller than the actual
 *     size to avoid unnecessary copying, but not larger.
 * @param newSize The new size to allocate. If 0, it will free the pointer.
 * @return The allocated memory or NULL if an error occurred or newSize is 0. The original pointer
 *     will remain intact if an error occurred.
 */
DS_CORE_EXPORT void* dsAllocator_reallocWithFallback(dsAllocator* allocator, void* ptr,
	size_t origSize, size_t newSize);

/**
 * @brief Frees memory from the allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free. This may be NULL.
 * @return True if the memory could be freed.
 */
DS_CORE_EXPORT inline bool dsAllocator_free(dsAllocator* allocator, void* ptr);

/**
 * @brief Gets the pointer to keep for an allocator.
 *
 * If the allocator doesn't have a free function, the allocator shouldn't be kept around with
 * an allocated object to free the object.
 *
 * @return The allocator pointer to keep in order to free memory, or NULL if the memory shouldn't
 *     be freed.
 */
DS_CORE_EXPORT inline dsAllocator* dsAllocator_keepPointer(dsAllocator* allocator);

inline void* dsAllocator_alloc(dsAllocator* allocator, size_t size)
{
	if (!allocator || !allocator->allocFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (size == 0)
		return NULL;

	return allocator->allocFunc(allocator, size, DS_ALLOC_ALIGNMENT);
}

inline void* dsAllocator_alignedAlloc(dsAllocator* allocator, size_t size, unsigned int alignment)
{
	if (!allocator || !allocator->allocFunc || alignment == 0 ||
		(unsigned int)(1 << (32 - dsClz(alignment - 1))) != alignment)
	{
		errno = EINVAL;
		return NULL;
	}

	if (size == 0)
		return NULL;

	return allocator->allocFunc(allocator, size, alignment);
}

inline void* dsAllocator_realloc(dsAllocator* allocator, void* ptr, size_t size)
{
	if (!allocator || !allocator->reallocFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	return allocator->reallocFunc(allocator, ptr, size, DS_ALLOC_ALIGNMENT);
}

inline bool dsAllocator_free(dsAllocator* allocator, void* ptr)
{
	if (!allocator || !allocator->freeFunc)
	{
		errno = EINVAL;
		return false;
	}

	return allocator->freeFunc(allocator, ptr);
}

inline dsAllocator* dsAllocator_keepPointer(dsAllocator* allocator)
{
	return allocator && allocator->freeFunc ? allocator : NULL;
}

#ifdef __cplusplus
}
#endif
