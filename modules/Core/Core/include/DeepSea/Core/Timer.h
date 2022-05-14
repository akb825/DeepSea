/*
 * Copyright 2016-2022 Aaron Barany
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
 * @brief Creates a timer.
 * @return The timer.
 */
DS_CORE_EXPORT dsTimer dsTimer_create(void);

/**
 * @brief Gets the current time in seconds.
 *
 * This uses the highest precision timer available on the system. It is monotonically increasing and
 * relative to an implementation-specific epoc. While not a strict guarantee, the start time is
 * typically reset on reboot. The timer may or may not be incremented when the system is asleep.
 *
 * A double is used rather than integer number of ticks for ease of use. This technically loses
 * resolution as more times progress, but should provide nanosecond precision for over 100 years and
 * microsecond precision for over 100,000 years. (with these timescales reset on reboot)
 *
 * @param timer The timer.
 * @return The current time in seconds.
 */
DS_CORE_EXPORT double dsTimer_time(dsTimer timer);

#ifdef __cplusplus
}
#endif
