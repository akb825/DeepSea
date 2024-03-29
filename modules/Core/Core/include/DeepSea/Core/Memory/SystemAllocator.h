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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Memory/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Implementation of dsAllocator that uses the system allocator. (usually malloc)
 * @see dsSystemAllocator
 */

/**
 * @brief Initializes the system system allocator.
 * @remark errno will be set on failure.
 * @param[out] allocator The allocator to initialize.
 * @param limit The limit for the allocator. Set to DS_ALLOCATOR_NO_LIMIT to have no limit.
 * @return False if allocator is NULL.
 */
DS_CORE_EXPORT bool dsSystemAllocator_initialize(dsSystemAllocator* allocator, size_t limit);

/**
 * @brief Allocates memory from the system allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @param alignment The minimum alignment for the allocation.
 * @return The allocated memory or NULL if an error occurred.
 */
DS_CORE_EXPORT void* dsSystemAllocator_alloc(dsSystemAllocator* allocator, size_t size,
	unsigned int alignment);

/**
 * @brief Re-allocates memory from the system allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param ptr The original pointer to reallocate.
 * @param size The size to allocate.
 * @param alignment The minimum alignment for the allocation.
 * @return The allocated memory or NULL. If NULL and size isn't 0, an error occurred.
 */
DS_CORE_EXPORT void* dsSystemAllocator_realloc(dsSystemAllocator* allocator, void* ptr, size_t size,
	unsigned int alignment);

/**
 * @brief Frees memory from the system allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
DS_CORE_EXPORT bool dsSystemAllocator_free(dsSystemAllocator* allocator, void* ptr);

#ifdef __cplusplus
}
#endif
