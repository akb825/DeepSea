#pragma once

#include <DeepSea/Core/Export.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to get a high resolution time in seconds.
 */

/**
 * @brief Structure that holds the system data for a timer.
 */
typedef struct
{
	/** Implementation specific scale. */
	double scale;
} dsTimer;

/**
 * @brief Initializes a timer.
 * @param timer The timer to initialize.
 */
DS_CORE_EXPORT void dsTimer_initialize(dsTimer* timer);

/**
 * @brief Gets the current time in seconds.
 *
 * This time is not guaranteed to be relative to any specific epoch. It is meant to be high
 * precision and used for relataive times.
 *
 * @param timer The timer.
 * @return The current time in seconds.
 */
DS_CORE_EXPORT double dsTimer_getTime(dsTimer* timer);

#ifdef __cplusplus
}
#endif
