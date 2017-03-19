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
 * @brief Structure that defines a memory allocator.
 * @see dsAllocator
 */

/**
 * @brief Allocates memory from the allocator.
 *
 * The alignment of the returned pointer will be aligned by DS_ALLOC_ALIGNMENT.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
DS_CORE_EXPORT inline void* dsAllocator_alloc(dsAllocator* allocator, size_t size);

/**
 * @brief Frees memory from the allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
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

	return allocator->allocFunc(allocator, size, DS_ALLOC_ALIGNMENT);
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
