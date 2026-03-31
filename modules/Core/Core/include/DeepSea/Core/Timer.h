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

#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to get a high resolution time in seconds.
 */

/**
 * @brief Gets the current number of ticks for a timer.
 *
 * This uses the highest precision timer available on the system. It is monotonically increasing and
 * relative to an implementation-specific epoch. While not a strict guarantee, the start time is
 * typically reset on reboot. The timer may or may not be incremented when the system is asleep.
 *
 * Typically a difference between two timepoints is used with dsTimer_ticksToSeconds() to get the
 * final relative time in seconds
 *
 * @return The current ticks.
 */
DS_CORE_EXPORT uint64_t dsTimer_currentTicks(void);

/**
 * @brief Creates a timer.
 * @return The timer.
 */
DS_CORE_EXPORT dsTimer dsTimer_create(void);

/**
 * @brief Converts ticks to seconds for a timer.
 * @param timer The timer.
 * @param ticks The number of ticks.
 * @return The number of seconds based on the ticks.
 */
DS_CORE_EXPORT inline double dsTimer_ticksToSeconds(dsTimer timer, int64_t ticks)
{
	return timer.scale*(double)ticks;
}

/**
 * @brief Converts ticks from one timer to another.
 *
 * This can be used if the total number of ticks (e.g. total runtime) is saved and loaded on a
 * machine with a different scale from ticks to seconds.
 *
 * @param timer The timer to convert the ticks to.
 * @param origScale The original scale the ticks were queried with.
 * @param ticks The number of ticks according to origScale.
 * @return The new number of ticks.
 */
DS_CORE_EXPORT uint64_t dsTimer_convertTicks(dsTimer timer, double origScale, uint64_t ticks);

#ifdef __cplusplus
}
#endif
