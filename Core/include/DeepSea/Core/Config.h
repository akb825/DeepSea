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
