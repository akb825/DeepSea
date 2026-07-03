/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene ticks.
 * @see dsSceneTick
 */

/**
 * @brief Initializes a scene tick.
 * @remark errno will be set on failure.
 * @param[out] outTick The scene tick to initialize.
 * @param updatePeriod The update period in seconds. Updates will always be performed in increments
 *     of this time. A value of zero indicates that the previous frame's time should be used,
 *     allowing for dynamic time steps.
 * @param maxTime The maximum time in seconds beyond which this will discard the update entirely.
 *     For example, if the application was placed in the background or computer put to sleep, this
 *     can be used to avoid problematically long updates. A value of zero indicates to accept any
 *     update time, otherwise elapsed times that exceed this will be forced to zero.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneTick_initialize(
	dsSceneTick* outTick, float updatePeriod, float maxTime);

/**
 * @brief Updates a scene tick based on time progressing from the timer.
 * @remark errno will be set on failure.
 * @param tick The scene tick to update.
 * @param absoluteTicks The current absolute time in timer ticks.
 * @param elapsedTicks The amount of time that has elapsed in ticks.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneTick_update(
	dsSceneTick* tick, uint64_t absoluteTicks, uint64_t elapsedTicks);

/**
 * @brief Gets the total step number for a given step within a tick.
 *
 * tick->totalSteps is the total number of steps once the updates are complete, this allows getting
 * an absolute step number during an update within the tick.
 *
 * @param tick The current tick.
 * @param step The step number within the tick.
 * @return The absolute tick number from when the tick started keeping track.
 */
DS_SCENE_EXPORT uint64_t dsSceneTick_absoluteStepNumber(
	const dsSceneTick* tick, unsigned int step);

/**
 * @brief Gets the interpolation for a step within the tick.
 * @param tick The current tick.
 * @param step THe step within the tick.
 * @return The interpolation to use for this step.
 */
DS_SCENE_EXPORT inline float dsSceneTick_interpForStep(const dsSceneTick* tick, unsigned int step);

inline float dsSceneTick_interpForStep(const dsSceneTick* tick, unsigned int step)
{
	if (!tick || step != tick->stepCount - 1)
		return 1.0f;
	return tick->stepInterp;
}

#ifdef __cplusplus
}
#endif
