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
#include <DeepSea/Core/Types.h>

#if DS_MSC
#include <intrin.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Helper functions for manipulating bitmasks.
 */

/**
 * @brief Counts the leading zeros in a 32-bit integer.
 * @param x The bitmask.
 * @return The number of leading zeros.
 */
DS_CORE_EXPORT inline uint32_t dsClz(uint32_t x);

/**
 * @brief Counts the trailing zeros in a 32-bit integer.
 * @param x The bitmask.
 * @return The number of trailing zeros.
 */
DS_CORE_EXPORT inline uint32_t dsCtz(uint32_t x);

/**
 * @brief Gets the index of the next item in a bitmask.
 *
 * This is used when iterating over a bitmask. For example:
 * @code
 * for (uint32_t curBitmask = bitmask; curBitmask; curBitmask = dsRemoveLastBit(curBitmask))
 * {
 *     uint32_t i = dsBitmaskIndex(curBitmask);
 *     ...
 * }
 * @endcode
 *
 * @param x The bitmask.
 * @return The first set bit.
 */
DS_CORE_EXPORT inline uint32_t dsBitmaskIndex(uint32_t x);

/**
 * @brief Removes the last bit in a bitmask.
 *
 * This is used when iterating over a bitmask. For example:
 * @code
 * for (uint32_t curBitmask = bitmask; curBitmask; curBitmask = dsRemoveLastBit(curBitmask))
 * {
 *     uint32_t i = dsBitmaskIndex(curBitmask);
 *     ...
 * }
 * @endcode
 *
 * @param x The bitmask.
 * @return The bitmask with the last bit removed.
 */
DS_CORE_EXPORT inline uint32_t dsRemoveLastBit(uint32_t x);

inline uint32_t dsClz(uint32_t x)
{
#if DS_MSC
	if (!x)
		return 32;

	unsigned long leading = 0;
	_BitScanReverse( &leading, x);
	return 31 - leading;
#elif DS_GCC || DS_CLANG
	return x ? __builtin_clz(x) : 32;
#else
#error Need to implement clz for current compiler.
#endif
}

inline uint32_t dsCtz(uint32_t x)
{
#if DS_MSC
	if (!x)
		return 32;

	unsigned long trailing = 0;
	_BitScanForward( &trailing, x);
	return trailing;
#elif DS_GCC || DS_CLANG
	return x ? __builtin_ctz(x) : 32;
#else
#error Need to implement ctz for current compiler.
#endif
}

inline uint32_t dsBitmaskIndex(uint32_t x)
{
	return dsCtz(x);
}

inline uint32_t dsRemoveLastBit(uint32_t x)
{
	return x & (x - 1);
}

#ifdef __cplusplus
}
#endif
