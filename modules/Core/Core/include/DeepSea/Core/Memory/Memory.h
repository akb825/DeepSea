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
 * used directly.
 *
 * TODO: If 256 bit SIMD operations become commonplace to use, should set this to 32 for platforms
 * where such instructions are possible (currently just x86 32/64), allowing them to be used without
 * explicit alignment. For now this isn't done to reduce waste for extra padding, since
 * single-precsion floats are almost exclusively used..
 */
#define DS_ALLOC_ALIGNMENT 16

/**
 * @brief Attributes a type to use a specific alignment.
 * @param x The alignment to use. This will most commonly be 16.
 */
#if DS_GCC || DS_CLANG
#	define DS_ALIGN(x) __attribute__((aligned(x)))
#elif DS_MSC
#	define DS_ALIGN(x) __declspec(align(x))
#else
#error Need to provide alignment implementation for this compiler.
#endif

/**
 * @brief Attributes a type to use a specific alignment for a parameter.
 * @param x The alignment to use. This will most commonly be 16.
 */
#if DS_GCC || DS_CLANG
#	define DS_ALIGN_PARAM(x) __attribute__((aligned(x)))
#else
#	define DS_ALIGN_PARAM(x)
#endif

/**
 * @brief Gets the aligned size for a custom alignment.
 * @param x The original size.
 * @param alignment The alignment. This must be a power of two.
 * @return The aligned size.
 */
#define DS_CUSTOM_ALIGNED_SIZE(x, alignment) \
	(((x) + (alignment) - 1) & ~(((size_t)(alignment)) - 1))

/**
 * @brief Gets the aligned size of an object.
 *
 * This can be used to calculate the amount of space to allocate to preserve alignment according to
 * DS_ALLOC_ALIGNMENT. For example, when pre-allocating a pool of memory for a group of objects
 * and advancing the memory pointer for each object.
 *
 * @param x The original size.
 * @return The aligned size.
 */
#define DS_ALIGNED_SIZE(x) DS_CUSTOM_ALIGNED_SIZE(x, DS_ALLOC_ALIGNMENT)

/**
 * @brief Gets the size when re-aligning from the default alignment to a custom alignment.
 * @param x The original size.
 * @param alignment The alignment. This must be a power of two and >= DS_ALLOC_ALIGNMENT.
 * @return The aligned size.
 */
#define DS_REALIGNED_SIZE(x, alignment) \
	(DS_CUSTOM_ALIGNED_SIZE(x, alignment) + ((alignment) - DS_ALLOC_ALIGNMENT))

#ifdef __cplusplus
}
#endif
