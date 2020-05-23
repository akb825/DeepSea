/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Application/Export.h>
#include <DeepSea/Application/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with controllers.
 *
 * These functions are used to query the state of controller and control rumble. Multiple types
 * of controllers, including racing wheels, joysticks, and flight sticks are supported.
 *
 * @see dsController
 */

/**
 * @brief Gets the value for a controller axis.
 * @param controller The controller to get the axis from.
 * @param axis The index of the axis.
 * @return The axis value.
 */
DS_APPLICATION_EXPORT float dsController_getAxis(const dsController* controller, uint32_t axis);

/**
 * @brief Gets whether or not a controller button is pressed.
 * @param controller The controller to get the button state from.
 * @param button The button to check.
 * @return True if the button is pressed.
 */
DS_APPLICATION_EXPORT bool dsController_isButtonPressed(const dsController* controller,
	uint32_t button);

/**
 * @brief Gets the the hat direction for a controller.
 * @remark errno will be set on failure.
 * @param[out] outDirection The direction the hat is in.
 * @param controller The controller to get the hat direction from.
 * @param hat The hat to check.
 * @return False if the hat state couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsController_getHatDirection(dsVector2i* outDirection,
	const dsController* controller, uint32_t hat);

/**
 * @brief Starts rumble on a controller.
 * @remark errno will be set on failure.
 * @param controller The controller to start the rumble on.
 * @param strength The strength of the rumble, in the range [0, 1].
 * @param duration The duration to rumble for in seconds. If a value < 0, the rumble will continue
 *     until stopped.
 * @return False if rumble couldn't be started.
 */
DS_APPLICATION_EXPORT bool dsController_startRumble(dsController* controller, float strength,
	float duration);

/**
 * @brief Stops rumble on a controller.
 * @remark errno will be set on failure.
 * @param controller The controller to stopthe rumble on.
 * @return False if rumble couldn't be stopped.
 */
DS_APPLICATION_EXPORT bool dsController_stopRumble(dsController* controller);

#ifdef __cplusplus
}
#endif
