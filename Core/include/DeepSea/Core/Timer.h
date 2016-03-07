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
typedef struct dsTimer
{
	/** Implementation specific scale. */
	double scale;
} dsTimer;

/**
 * @brief Creates a timer.
 * @return The timer.
 */
DS_CORE_EXPORT dsTimer dsTimer_create();

/**
 * @brief Gets the current time in seconds.
 *
 * This time is not guaranteed to be relative to any specific epoch. It is meant to be high
 * precision and used for relataive times.
 *
 * @param timer The timer.
 * @return The current time in seconds.
 */
DS_CORE_EXPORT double dsTimer_getTime(dsTimer timer);

#ifdef __cplusplus
}
#endif
