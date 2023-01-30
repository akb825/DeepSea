/*
 * Copyright 2016-2023 Aaron Barany
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
#	if TARGET_OS_OSX
#		define DS_MAC 1
#	else
#		define DS_IOS 1
#	endif
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || \
	defined(__DragonFly__)
#	define DS_BSD 1
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
 * @brief Define for whether the platform is BSD.
 */
#ifndef DS_BSD
#	define DS_BSD 0
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
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || \
	defined(__arm64__) || defined(__aarch64__)
#define DS_64BIT 1
#else
#define DS_64BIT 0
#endif

/**
 * @brief Macro defined to whether or not the system is 64-bit x86.
 */
#if defined(__x86_64__) || defined(_M_AMD64)
#define DS_X86_64 1
#else
#define DS_X86_64 0
#endif

/**
 * @brief Macro defined to whether or not the system is 32-bit x86.
 */
#if defined(__i386__) || defined(_M_IX86)
#define DS_X86_32 1
#else
#define DS_X86_32 0
#endif

/**
 * @brief Macro defined to whether or not the system is 64-bit ARM.
 */
#if defined(__arm64__) || defined(__aarch64__)
#define DS_ARM_64 1
#else
#define DS_ARM_64 0
#endif

/**
 * @brief Macro defined to whether or not the system is 32-bit ARM.
 */
#if defined(__arm__) || defined(_M_ARM)
#define DS_ARM_32 1
#else
#define DS_ARM_32 0
#endif

/**
 * @brief Macro defined to whether or not the system is 64-bit PPC.
 */
#if defined(__ppc64__)
#define DS_PPC_64 1
#else
#define DS_PPC_64 0
#endif

/**
 * @brief Macro defined to whether or not the system is 32-bit PPC.
 */
#if defined(__ppc__)
#define DS_PPC_32 1
#else
#define DS_PPC_32 0
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
 * @brief Macro to always inline a function.
 */
#if DS_MSC
#define DS_ALWAYS_INLINE static __forceinline
#elif DS_GCC
#define DS_ALWAYS_INLINE extern __attribute__((always_inline, gnu_inline)) inline
#else
#define DS_ALWAYS_INLINE static __attribute__((always_inline)) inline
#endif

/**
 * @brief Macro for an unused variable.
 *
 * This can be used to work around compiler warnings.
 * @param x The unused variable.
 */
#define DS_UNUSED(x) (void)(&x)

/**
 * @brief Macro to mark a branch of code as unreachable.
 */
#if DS_MSC
#define DS_UNREACHABLE() __assume(0)
#else
#define DS_UNREACHABLE() __builtin_unreachable()
#endif

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

/**
 * @brief Encodes a version number into a 32-bit integer.
 * @param major The major version number. This must fit within 10 bits.
 * @param minor The minor version number. This must fit within 10 bits.
 * @param patch The patch version number. This must fit within 12 bits.
 */
#define DS_ENCODE_VERSION(major, minor, patch) ((((unsigned int)(major) & 0x3FF) << 22) | \
	(((unsigned int)(minor) & 0x3FF) << 12) | (((unsigned int)(patch) & 0xFFF)))

/**
 * @brief Decodes a version number.
 * @param[out] outMajor The major version.
 * @param[out] outMinor The minor version.
 * @param[out] outPatch The patch version.
 * @param version The encoded version number
 */
#define DS_DECODE_VERSION(outMajor, outMinor, outPatch, version) \
	do \
	{ \
		(outMajor) = ((unsigned int)(version) >> 22) & 0x3FF; \
		(outMinor) = ((unsigned int)(version) >> 12) & 0x3FF; \
		(outPatch) = (unsigned int)(version) & 0xFFF; \
	} while (0)

/**
 * @brief The major version of DeepSea.
 */
#ifndef DS_MAJOR_VERSION
#define DS_MAJOR_VERSION 0
#endif

/**
 * @brief The minor version of DeepSea.
 */
#ifndef DS_MINOR_VERSION
#define DS_MINOR_VERSION 0
#endif

/**
 * @brief The patch version of DeepSea.
 */
#ifndef DS_PATCH_VERSION
#define DS_PATCH_VERSION 0
#endif

/**
 * @brief The encoded version number of DeepSea.
 */
#define DS_VERSION DS_ENCODE_VERSION(DS_MAJOR_VERSION, DS_MINOR_VERSION, DS_PATCH_VERSION)

/**
 * @brief Macro for defining bitwise operators for enums.
 * @param type Enum type.
 */
#ifdef __cplusplus
#define DS_ENUM_BITMASK_OPERATORS(type) \
	inline type operator~(type x) {return (type)~(unsigned int)x;} \
	inline type operator|(type l, type r) {return (type)((unsigned int)l | (unsigned int)r);} \
	inline type operator&(type l, type r) {return (type)((unsigned int)l & (unsigned int)r);} \
	inline type operator^(type l, type r) {return (type)((unsigned int)l ^ (unsigned int)r);} \
	inline type& operator|=(type& l, type r) {return l = l | r;} \
	inline type& operator&=(type& l, type r) {return l = l & r;} \
	inline type& operator^=(type& l, type r) {return l = l ^ r;}
#else
#define DS_ENUM_BITMASK_OPERATORS(type)
#endif

#if DS_MSC
#pragma warning(disable: 4200) // nonstandard extension used : zero-sized array in struct/union
#endif
