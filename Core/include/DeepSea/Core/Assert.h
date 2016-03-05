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
 */
#define DS_STATIC_ASSERT(x) typedef char static_assertion_failed[(x) ? 1 : -1]
