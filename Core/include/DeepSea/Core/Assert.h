#pragma once

#include <assert.h>

/**
 * @file
 * @brief Assert macros used for checking validity of values.
 */

/**
 * @brief Asserts that a condition is true.
 *
 * The condition is only evaluated when asserts are enabled.
 * @param x The expression to assert on.
 */
#ifdef NDEBUG
#define DS_ASSERTS_ENABLED 0
#else
#define DS_ASSERTS_ENABLED 1
#endif

#define DS_ASSERT(x) assert(x)

/**
 * @brief Verifies that a condition is true.
 *
 * The condition is always evaluated.
 * @param x The expression to assert on.
 */
#if DS_ASSERTS_ENABLED
#define DS_VERIFY(x) assert(x)
#else
#define DS_VERIFY(x) (x)
#endif
