/*
 * Copyright 2017 Aaron Barany
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
 * @brief Functions to aid in having an array that may be resized, similar to std::vector.
 */

/**
 * @brief Helper macro for adding elements to a resizeable array.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the array with.
 * @param[inout] array The array to resize. The value of array may be be NULL for when the array
 *     starts as empty. The value of array will be re-assigned if the array is re-allocated.
 * @param[inout] elementCount The number of elements in the buffer. (passed by value, not by
 *     pointer)
 * @param[inout] maxElements The naximum number of elements held by the buffer. (passed by value,
 *     not by pointer)
 * @param addCount The number of elements to add.
 * @return False if the parameters are invalid or the buffer couldn't be allocated.
 */
#define DS_RESIZEABLE_ARRAY_ADD(allocator, array, elementCount, maxElements, addCount) \
	dsResizeableArray_add(allocator, (void**)&(array), &(elementCount), &(maxElements), \
		sizeof(*(array)), (addCount))

/**
 * @brief Helper macro for removing elements from a resizeable array.
 * @param[inout] array The array to remove elements from.
 * @param[inout] elementCount The number of elements in th ebuffer. (passed by value, not by
 *     pointer)
 * @param index The index to remove at.
 * @param removeCount The number of elements to remove.
 * @return False if the parameters are invalid.
 */
#define DS_RESIZEABLE_ARRAY_REMOVE(array, elementCount, index, removeCount) \
	dsResizeableArray_remove(array, &(elementCount), sizeof(*(array)), (index), (removeCount))

/**
 * @brief Adds elements to a resizeable array.
 *
 * If the number of elements has exceeded the capacity, the buffer will be re-allocated. The size
 * will be doubled each time it's re-allocated to amortize the allocation cost, similar to
 * std::vector in C++.
 *
 * @remark It is best to first consider data structures with a fixed size to avoid re-allocations.
 * This should be used sparingly in cases where it isn't practical to know a limit ahead of time.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the buffer with.
 * @param[inout] buffer A pointer to the buffer to hold the data. The value of *buffer may be
 *     be NULL for when the buffer starts as empty. The pointer will be re-assigned if the buffer
 *     is re-allocated.
 * @param[inout] elementCount The number of elements in the buffer.
 * @param[inout] maxElements The naximum number of elements held by the buffer.
 * @param elementSize The size of each element in bytes.
 * @param addCount The number of elements to add.
 * @return False if the parameters are invalid or the buffer couldn't be allocated.
 */
DS_CORE_EXPORT bool dsResizeableArray_add(dsAllocator* allocator, void** buffer,
	uint32_t* elementCount, uint32_t* maxElements, size_t elementSize, uint32_t addCount);

/**
 * @brief Removes elements from a resizeable array.
 * @remark errno will be set on failure.
 * @param buffer The buffer to hold the data.
 * @param[inout] elementCount The number of elements in the buffer.
 * @param elementSize The size of each element in bytes.
 * @param index The index to begin removing items.
 * @param removeCount The number of elements to remove.
 * @return False if the parameters are invalid.
 */
DS_CORE_EXPORT bool dsResizeableArray_remove(void* buffer, uint32_t* elementCount,
	uint32_t elementSize, uint32_t index, uint32_t removeCount);

#ifdef __cplusplus
}
#endif
