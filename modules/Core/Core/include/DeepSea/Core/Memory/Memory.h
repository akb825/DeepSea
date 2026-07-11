/*
 * Copyright 2016-2026 Aaron Barany
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

#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for core memory constants and operations.
 */

/**
 * @brief The alignment of allocated memory.
 *
 * This is the alignment used within the DeepSea libraries, and allows for 128 bit SIMD types to be
 * used directly. If DEEPSEA_HEAVY_DOUBLE_USAGE is set through CMake and AVX2 is supported, this
 * will be set to allow for 256 bit SIMD types at the expense for higher memory usage and slower
 * allocations in some situations.
 */
#if DS_HEAVY_DOUBLE_USAGE && defined(__AVX2__)
#define DS_ALLOC_ALIGNMENT 32
#else
#define DS_ALLOC_ALIGNMENT 16
#endif

/**
 * @brief Gets the aligned size.
 * @param x The original size.
 * @param alignment The alignment. This must be a power of two.
 * @return The aligned size.
 */
#define DS_ALIGNED_SIZE(x, alignment) \
	(((x) + (alignment) - 1) & ~(((size_t)(alignment)) - 1))

/**
 * @brief Gets the size when re-aligning from the default alignment to a custom alignment.
 * @param x The original size.
 * @param alignment The alignment. This must be a power of two and >= DS_ALLOC_ALIGNMENT.
 * @return The size to allow re-alignment.
 */
#define DS_REALIGNED_SIZE(x, alignment) \
	(DS_ALIGNED_SIZE(x, alignment) + ((alignment) - DS_ALLOC_ALIGNMENT))

/**
 * @brief Checks whether two sizes can be added, avoiding integer overflow.
 * @param firstSize The first size to add.
 * @param secondSize The seconds size to add.
 * @return Whether firstSize + secondSize can be performed without integer overflow.
 */
#define DS_CAN_ADD_SIZES(firstSize, secondSize) ((firstSize) <= (SIZE_MAX - (secondSize)))

/**
 * @brief Checks whether an array size is valid, avoiding integer overflow.
 * @param elemSize The size of each array element.
 * @param count The number of array elements.
 * @return False if count would cause an integer overflow for the full size.
 */
#define DS_ARRAY_SIZE_VALID(elemSize, count) ((size_t)(count) <= (SIZE_MAX/(elemSize)))

/**
 * @brief Adds one aligned size to another, checking for integer overflow.
 *
 * This is useful when computing the size to allocate a single buffer that is then chunked into
 * multiple sub-allocations.
 *
 * @remark errno will be set on failure.
 * @param[inout] fullSize The full allocation size. This will be aligned if not already.
 * @param newSize The size to add to fullSize. This will be aligned for allocation.
 * @param alignment The alignment to use. This must be a power of two.
 * @return Flase if the final memory size would result in an integer overflow.
 */
DS_CORE_EXPORT bool dsAddAlignedSize(size_t* fullSize, size_t newSize, unsigned int alignment);

/**
 * @brief Adds the aligned size for an array to the full allocation size, checking for integer
 *     overflow.
 *
 * The full array size will be aligned rather than each individual element. This is useful when
 * computing the size to allocate a single buffer that is then chunked into multiple
 * sub-allocations.
 *
 * @remark errno will be set on failure.
 * @param[inout] fullSize The full allocation size. This will be aligned if not already.
 * @param elemSize The size of each array element.
 * @param count The nubmer of array elements.
 * @param alignment The alignment to use. This must be a power of two.
 * @return Flase if the final memory size would result in an integer overflow.
 */
DS_CORE_EXPORT bool dsAddAlignedArraySize(
	size_t* fullSize, size_t elemSize, size_t count, unsigned int alignment);

/**
 * @brief Accumulates any number of aligned sizes into a full count, checking for integer overflow.
 *
 * This is useful when computing the size to allocate a single buffer that is then chunked into
 * multiple sub-allocations.
 *
 * @remark errno will be set on failure.
 * @param[inout] fullSize The full allocation size. This will be aligned if not already.
 * @param sizes The sizes to accumulate, either for arrays or non-arrays with the count set to 1.
 *     This will assume that any 0 elementSize when the count isn't also 0 resulted from a previous
 *     call to get the full size failing.
 * @param sizeCount The number of elements in sizes.
 * @param alignment The alignment to use. This must be a power of two.
 * @return Flase if the final memory size would result in an integer overflow.
 */
DS_CORE_EXPORT bool dsAccumulateAlignedSizes(
	size_t* fullSize, const dsMemorySize* sizes, unsigned int sizeCount, unsigned int alignment);

#ifdef __cplusplus
}
#endif
