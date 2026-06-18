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
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneTick_initialize(dsSceneTick* outTick, float updatePeriod);

/**
 * @brief Updates a scene tick based on time progressing from the timer.
 * @remark errno will be set on failure.
 * @param tick The scene tick to update.
 * @param timerTicks The number of ticks from the the timer.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneTick_update(dsSceneTick* tick, uint64_t timerTicks);

#ifdef __cplusplus
}
#endif
