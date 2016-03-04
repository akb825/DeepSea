#pragma once

#include <DeepSea/Core/Config.h>

/**
 * @file
 * @brief Debug utilities.
 */

#if DS_WINDOWS
#define DS_DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>

/**
 * @brief Breaks in the debugger.
 *
 * If a debugger isn't attached, the application may exit.
 */
#define DS_DEBUG_BREAK() raise(SIGTRAP)
#endif
