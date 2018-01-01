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

/**
 * @file
 * @brief Configuration macros for the project.
 */

#if defined(_WIN32)
#	define DS_WINDOWS 1
#elif defined(linux)
#	define DS_LINUX 1
#elif defined(__APPLE__)
#	define DS_APPLE 1
#	include <TargetConditionals.h>
#	if TARGET_OS_MAC
#		define DS_MAC 1
#	else
#		define DS_IOS 1
#	endif
#endif

// Android is also Linux, so separate.
#if defined(__ANDROID__)
#	define DS_ANDROID 1
#endif

#if defined(_MSC_VER)
#	define DS_MSC 1
#elif defined(__clang__)
#	define DS_CLANG 1
#elif defined(__GNUC__)
#	define DS_GCC 1
#else
#error Unknown compiler.
#endif

/**
 * @brief Define for whether the platform is Windows.
 */
#ifndef DS_WINDOWS
#	define DS_WINDOWS 0
#endif

/**
 * @brief Define for whether the platform is Linux.
 */
#ifndef DS_LINUX
#	define DS_LINUX 0
#endif

/**
 * @brief Define for whether the platform is Apple.
 *
 * Either DS_MAC or DS_IOS will also be defined.
 */
#ifndef DS_APPLE
#	define DS_APPLE 0
#endif

/**
 * @brief Define for whether the platform is Mac OS.
 */
#ifndef DS_MAC
#	define DS_MAC 0
#endif

/**
 * @brief Define for whether the platform is iOS.
 */
#ifndef DS_IOS
#	define DS_IOS 0
#endif

/**
 * @brief Define for whether the platform is Android.
 *
 * DS_LINUX should also be 1 on Android.
 */
#ifndef DS_ANDROID
#	define DS_ANDROID 0
#endif

/**
 * @brief Define for whether the compler is Microsoft's C compiler.
 */
#ifndef DS_MSC
#	define DS_MSC 0
#endif

/**
 * @brief Define for whether the compiler is LLVM clang.
 */
#ifndef DS_CLANG
#	define DS_CLANG 0
#endif

/**
 * @def DS_GCC
 * @brief Define for whether the compiler is GCC.
 */
#ifndef DS_GCC
#	define DS_GCC 0
#endif

/**
 * @brief Macro defined to whether or not the system is 64-bit.
 */
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__arm64__)
#define DS_64BIT 1
#else
#define DS_64BIT 0
#endif

/**
 * @brief Define for whether or not this is a debug build.
 */
#ifdef NDEBUG
#define DS_DEBUG 0
#else
#define DS_DEBUG 1
#endif

/**
 * @brief Macro for an unused variable.
 *
 * This can be used to work around compiler warnings.
 * @param x The unused variable.
 */
#define DS_UNUSED(x) (void)(&x)

/**
 * @brief Gets the number of elements within an array.
 * @param array The array.
 */
#define DS_ARRAY_SIZE(array) ((unsigned int)(sizeof(array)/sizeof(*(array))))

/**
 * @brief Checks whether or not a range within a buffer is valid.
 *
 * This is designed to prevent cases where offset + rangeSize would cause an integer overflow.
 *
 * @param offset The offset into the buffer.
 * @param rangeSize The size of the range within the buffer.
 * @param bufferSize The size of the buffer.
 * @return True if the range is valid.
 */
#define DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize) \
	((offset) <= (bufferSize) && ((bufferSize) - (offset)) >= (rangeSize))

/**
 * @brief Packs four characters into an integer.
 * @param a The first character.
 * @param b The second character.
 * @param c The third character.
 * @param d The fourth character.
 * @return The combined integer.
 */
#define DS_FOURCC(a, b, c, d) ((unsigned int)(a) | ((unsigned int)(b) << 8) | \
	((unsigned int)(c) << 16) | ((unsigned int)(d) << 24 ))

#if DS_MSC
#pragma warning(disable: 4200) // nonstandard extension used : zero-sized array in struct/union
#endif
