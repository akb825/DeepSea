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
#	define DS_APPLE
#endif

/**
 * @brief Define for whether or not the platform is Windows.
 */
#ifndef DS_WINDOWS
#	define DS_WINDOWS 0
#endif

/**
 * @brief Define for whether or not the platform is Linux.
 */
#ifndef DS_LINUX
#	define DS_LINUX 0
#endif

/**
 * @brief Define for whether or not the platform is Apple.
 */
#ifndef DS_APPLE
#	define DS_APPLE 0
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
