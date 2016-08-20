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
 * @brief Implementation of dsAllocator that allocates fixed chunks from a pool of memory.
 *
 * The pool of memory is pre-allocated and all allocations are the same size.
 */

/**
 * @brief Gets the size of a buffer based on the chunk size and count.
 * @param chunkSize The size of each chunk.
 * @param chunkCount The number of chunks to have available.
 * @return The buffer size.
 */
DS_CORE_EXPORT size_t dsPoolAllocator_bufferSize(size_t chunkSize, size_t chunkCount);

/**
 * @brief Initializes the pool allocator.
 * @param allocator The allcoator to initialize.
 * @param chunkSize The size of each chunk.
 * @param chunkCount The number of chunks to have available.
 * @param buffer The buffer of memory to allocate from. This must be aligned by DS_ALLOC_ALIGNMENT.
 * @param bufferSize The size of the buffer. This must be the same size as calling
 * dsPoolAllocator_bufferSize().
 * @return False if any of the parameters are invalid.
 */
DS_CORE_EXPORT bool dsPoolAllocator_initialize(dsPoolAllocator* allocator, size_t chunkSize,
	size_t chunkCount, void* buffer, size_t bufferSize);


/**
 * @brief Allocates memory from the pool allocator.
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate. This must be equal to or less than the pool size.
 * @return The allocated memory or NULL if an error occured.
 */
DS_CORE_EXPORT void* dsPoolAllocator_alloc(dsPoolAllocator* allocator, size_t size);

/**
 * @brief Frees memory from the pool allocator.
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
DS_CORE_EXPORT bool dsPoolAllocator_free(dsPoolAllocator* allocator, void* ptr);

/**
 * @brief Resets the pool allocator to be empty.
 *
 * This should only be used when no destruction is needed for the contents.
 *
 * @param allocator The allocator to reset.
 * @return True if the allocator is valid.
 */
DS_CORE_EXPORT bool dsPoolAllocator_reset(dsPoolAllocator* allocator);

/**
 * @brief Validates the consistency of the allocator.
 *
 * This can help make sure that there were no buffer overruns
 *
 * @param allocator The allocator to validate.
 * @return True if the allocator is valid.
 */
DS_CORE_EXPORT bool dsPoolAllocator_validate(dsPoolAllocator* allocator);

/**
 * @brief Destroys the pool allocator.
 * @remark The buffer itself will not be freed.
 * @param allocator The allocator to destroy. This will be cleared.
 */
DS_CORE_EXPORT void dsPoolAllocator_destroy(dsPoolAllocator* allocator);

#ifdef __cplusplus
}
#endif
