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
 * @brief Implementation of dsAllocator that allocates memory from the buffer.
 *
 * The buffer is pre-allocated and once memory is allocated from the buffer it isn't freed.
 *
 * @see dsBufferAllocator
 */

/**
 * @brief Initializes the buffer allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to initialize.
 * @param buffer The buffer to initialize with. This must be aligned by DS_ALLOC_ALIGNMENT.
 * @param bufferSize The size of the buffer in bytes.
 * @return False if the allocator couldn't be iniitialized.
 */
DS_CORE_EXPORT bool dsBufferAllocator_initialize(dsBufferAllocator* allocator, void* buffer,
	size_t bufferSize);

/**
 * @brief Allocates memory from the buffer allocator.
 * @remark errno will be set on failure.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @param alignment The minimum alignment of the allocation. This may need to use additional space
 *     to align the memory. Use DS_REALIGNED_SIZE() to compute the size to take this into account.
 * @return The allocated memory or NULL if an error occurred.
 */
DS_CORE_EXPORT void* dsBufferAllocator_alloc(dsBufferAllocator* allocator, size_t size,
	unsigned int alignment);

/**
 * @brief Resets the buffer allocator to the beginning.
 * @remark errno will be set on failure.
 * @param allocator The allocator to reset.
 * @return True if the allocator is valid.
 */
DS_CORE_EXPORT bool dsBufferAllocator_reset(dsBufferAllocator* allocator);

#ifdef __cplusplus
}
#endif
