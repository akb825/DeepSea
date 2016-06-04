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
#include <DeepSea/Core/Debug.h>
#include <DeepSea/Core/Log.h>
#include <stdlib.h>

/**
 * @file
 * @brief Assert macros used for checking validity of values.
 */

/**
 * @brief Define for when asserts are enabled.
 *
 * Asserts will be enabled on debug builds.
 */
#if DS_DEBUG
#define DS_ASSERTS_ENABLED 1
#else
#define DS_ASSERTS_ENABLED 0
#endif

/**
 * @brief Asserts that a condition is true.
 *
 * If the condition is false, it will print a message to the console (according to
 * dsLog_defaultPrint()), and abort the program. The condition is only evaluated when asserts are
 * enabled.
 *
 * @remark This should be used instead of C's assert(), since it has two main advantages on Windows:
 * 1. It will actually break in the debugger.
 * 2. It will print to the output window and be double-clickable.
 *
 * @param x The expression to assert on.
 */
#if DS_ASSERTS_ENABLED
#define DS_ASSERT(x) \
	do \
	{ \
		if (!(x)) \
		{ \
			dsLog_defaultPrint(dsLogLevel_Fatal, "assertion failed", __FILE__, __LINE__, \
				__FUNCTION__, #x); \
			DS_DEBUG_BREAK(); \
			abort(); \
		} \
	} while (0)
#else
#define DS_ASSERT(x) do {} while(0)
#endif

/**
 * @brief Verifies that a condition is true.
 *
 * The condition is always evaluated.
 * @param x The expression to assert on.
 */
#if DS_ASSERTS_ENABLED
#define DS_VERIFY(x) DS_ASSERT(x)
#else
#define DS_VERIFY(x) (void)(x)
#endif

/**
* @brief Asserts at compile time that a condition is true.
* @param x The expression to assert on.
* @param message A message to include with the assertion. This must not contain whitespace.
*/
#if DS_GCC || DS_CLANG
#define DS_STATIC_ASSERT(x, message) \
	typedef __attribute__((unused)) char static_assertion_failed_ ## message[(x) ? 1 : -1]
#else
#define DS_STATIC_ASSERT(x, message) \
	typedef char static_assertion_failed_ ## message[(x) ? 1 : -1]
#endif
