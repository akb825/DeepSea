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
 * @return The number of leading zeros. The result is undefined if x is zero.
 */
DS_CORE_EXPORT inline unsigned int dsClz(uint32_t x);

/**
 * @brief Counts the leading zeros in a 64-bit integer.
 * @param x The bitmask.
 * @return The number of leading zeros. The result is undefined if x is zero.
 */
DS_CORE_EXPORT inline unsigned int dsClz64(uint64_t x);

/**
 * @brief Counts the trailing zeros in a 32-bit integer.
 * @param x The bitmask.
 * @return The number of trailing zeros. The result is undefined if x is zero.
 */
DS_CORE_EXPORT inline unsigned int dsCtz(uint32_t x);

/**
 * @brief Counts the trailing zeros in a 64-bit integer.
 * @param x The bitmask.
 * @return The number of trailing zeros. The result is undefined if x is zero.
 */
DS_CORE_EXPORT inline unsigned int dsCtz64(uint64_t x);

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
 * @return The first set bit. The result is undefined if x is zero.
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

/**
 * @brief Counts the number of bits in a bitmask.
 * @param x The bitmask.
 * @return The number of bits.
 */
DS_CORE_EXPORT inline uint32_t dsCountBits(uint32_t x);

inline unsigned int dsClz(uint32_t x)
{
#if DS_MSC
	unsigned long leading = 0;
	_BitScanReverse(&leading, x);
	return 31 - (unsigned int)leading;
#elif DS_GCC || DS_CLANG
	return __builtin_clz(x);
#else
#error Need to implement clz for current compiler.
#endif
}

inline unsigned int dsClz64(uint64_t x)
{
#if DS_MSC
	unsigned long leading = 0;
#if DS_64_BIT
	_BitScanReverse64(&leading, x);
	return 63 - (unsigned int)leading;
#else
	const uint32_t* words = (const uint32_t*)&x;
	if (words[1])
	{
		_BitScanReverse(&leading, words[1]);
		return 31 - (unsigned int)leading;
	}
	_BitScanReverse(&leading, words[0]);
	return 63 - (unsigned int)leading;
#endif
#elif DS_GCC || DS_CLANG
	return __builtin_clzll(x);
#else
#error Need to implement clz for current compiler.
#endif
}

inline unsigned int dsCtz(uint32_t x)
{
#if DS_MSC
	unsigned long trailing = 0;
	_BitScanForward(&trailing, x);
	return (unsigned int)trailing;
#elif DS_GCC || DS_CLANG
	return __builtin_ctz(x);
#else
#error Need to implement ctz for current compiler.
#endif
}

inline unsigned int dsCtz64(uint64_t x)
{
#if DS_MSC
	unsigned long trailing = 0;
#if DS_64_BIT
	_BitScanForward64(&trailing, x);
	return (unsigned int)trailing;
#else
	const uint32_t* words = (const uint32_t*)&x;
	if (words[0])
	{
		_BitScanForward(&trailing, words[0]);
		return (unsigned int)trailing;
	}
	_BitScanForward(&trailing, words[1]);
	return 32 + (unsigned int)trailing;
#endif
#elif DS_GCC || DS_CLANG
	return __builtin_ctzll(x);
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

inline uint32_t dsCountBits(uint32_t x)
{
#if DS_MSC
	return __popcnt(x);
#elif DS_GCC || DS_CLANG
	return __builtin_popcount(x);
#else
	// https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
	x = x - ((x >> 1) & 0x55555555U);
	x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
	return (((x + (x >> 4)) & 0x0F0F0F0FU) * 0x01010101U) >> 24;
#endif
}

#ifdef __cplusplus
}
#endif
