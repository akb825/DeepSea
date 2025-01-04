/*
 * Copyright 2025 Aaron Barany
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
#include <DeepSea/Core/Types.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Defines and functions for checking platform byte order and performing byte swapping.
 */

#if DS_MSC
// Assume always little endian.
#define DS_BIG_ENDIAN 0
#define DS_LITTLE_ENDIAN 1
#else
#if !defined(__BYTE_ORDER__)
#error __BYTE_ORDER__ macros not defined.
#endif
#if defined(__FLOAT_WORD_ORDER__) && __FLOAT_WORD_ORDER__ != __BYTE_ORDER__
#error Inconsistent byte ordering for floating point.
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
/**
 * @brief Macro set to 1 for big-endian CPUs, 0 for little-endian CPUs.
 */
#define DS_BIG_ENDIAN 1

/**
 * @brief Macro set to 1 for little-endian CPUs, 0 for big-endian CPUs.
 */
#define DS_LITTLE_ENDIAN 0
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DS_BIG_ENDIAN 0
#define DS_LITTLE_ENDIAN 1
#else
#error Unsupported byte ordering.
#endif
#endif

/**
 * @brief Byte swaps a uint16_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint16_t dsEndian_swapUInt16(uint16_t value)
{
#if DS_GCC || DS_CLANG
	return __builtin_bswap16(value);
#elif DS_MSC
	return _byteswap_ushort(value);
#endif
}

/**
 * @brief Byte swaps an int16_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int16_t dsEndian_swapInt16(int16_t value)
{
	return (int16_t)dsEndian_swapUInt16((uint16_t)value);
}

/**
 * @brief Byte swaps a uint32_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint32_t dsEndian_swapUInt32(uint32_t value)
{
#if DS_GCC || DS_CLANG
	return __builtin_bswap32(value);
#elif DS_MSC
	return _byteswap_ulong(value);
#endif
}

/**
 * @brief Byte swaps an int32_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int32_t dsEndian_swapInt32(int32_t value)
{
	return (int32_t)dsEndian_swapUInt32((uint32_t)value);
}

/**
 * @brief Byte swaps a uint64_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint64_t dsEndian_swapUInt64(uint64_t value)
{
#if DS_GCC || DS_CLANG
	return __builtin_bswap64(value);
#elif DS_MSC
	return _byteswap_uint64(value);
#endif
}

/**
 * @brief Byte swaps an int64_t value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int64_t dsEndian_swapInt64(int64_t value)
{
	return (int64_t)dsEndian_swapUInt64((uint64_t)value);
}

/**
 * @brief Byte swaps a float value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE float dsEndian_swapFloat(float value)
{
	union {float f; uint32_t i;} swapHelper;
	swapHelper.f = value;
	swapHelper.i = dsEndian_swapUInt32(swapHelper.i);
	return swapHelper.f;
}

/**
 * @brief Byte swaps a double value.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE double dsEndian_swapDouble(double value)
{
	union {double f; uint64_t i;} swapHelper;
	swapHelper.f = value;
	swapHelper.i = dsEndian_swapUInt64(swapHelper.i);
	return swapHelper.f;
}

/**
 * @brief Byte swaps a uint16_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint16_t dsEndian_swapUInt16OnBig(uint16_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapUInt16(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int16_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int16_t dsEndian_swapInt16OnBig(int16_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapInt16(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a uint32_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint32_t dsEndian_swapUInt32OnBig(uint32_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapUInt32(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int32_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int32_t dsEndian_swapInt32OnBig(int32_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapInt32(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a uint64_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint64_t dsEndian_swapUInt64OnBig(uint64_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapUInt64(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int64_t value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int64_t dsEndian_swapInt64OnBig(int64_t value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapInt64(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a float value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE float dsEndian_swapFloatOnBig(float value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapFloat(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a double value only on big-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE double dsEndian_swapDoubleOnBig(double value)
{
#if DS_BIG_ENDIAN
	return dsEndian_swapDouble(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a uint16_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint16_t dsEndian_swapUInt16OnLittle(uint16_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapUInt16(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int16_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int16_t dsEndian_swapInt16OnLittle(int16_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapInt16(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a uint32_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint32_t dsEndian_swapUInt32OnLittle(uint32_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapUInt32(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int32_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int32_t dsEndian_swapInt32OnLittle(int32_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapInt32(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a uint64_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE uint64_t dsEndian_swapUInt64OnLittle(uint64_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapUInt64(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps an int64_t value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE int64_t dsEndian_swapInt64OnLittle(int64_t value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapInt64(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a float value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE float dsEndian_swapFloatOnLittle(float value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapFloat(value);
#else
	return value;
#endif
}

/**
 * @brief Byte swaps a double value only on little-endian systems.
 * @param value The value to swap.
 * @return The byte swapped value.
 */
DS_ALWAYS_INLINE double dsEndian_swapDoubleOnLittle(double value)
{
#if DS_LITTLE_ENDIAN
	return dsEndian_swapDouble(value);
#else
	return value;
#endif
}

#ifdef __cplusplus
}
#endif
