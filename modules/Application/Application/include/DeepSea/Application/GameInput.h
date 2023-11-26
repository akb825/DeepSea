/*
 * Copyright 2017-2023 Aaron Barany
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
 * @brief Functions for working with game input devices.
 *
 * These functions are used to query the state of the game input and control rumble. Multiple types
 * of game input devices, including standard controllers, racing wheels, joysticks, and flight
 * sticks are supported.
 *
 * @see dsGameInput
 */

/**
 * @brief Checks whether or not a controller mapping exists.
 * @param gameInput The game input device to check the mapping for.
 * @param mapping The controller mapping.
 * @return Whether or not the mapping is available.
 */
DS_APPLICATION_EXPORT bool dsGameInput_hasControllerMapping(const dsGameInput* gameInput,
	dsGameControllerMap mapping);

/**
 * @brief Checks whether or not an input is controller mapped.
 * @param gameInput The game input device to find the mapping for.
 * @param method The input method.
 * @param index The index of the input method. (button, axis, or dpad)
 * @return Whether or not the input is controller mapped.
 */
DS_APPLICATION_EXPORT bool dsGameInput_isInputControllerMapped(const dsGameInput* gameInput,
	dsGameInputMethod method, uint32_t index);

/**
 * @brief Finds the controller mapping for an input map.
 * @param gameInput The game input device to find the mapping for.
 * @param inputMap The input map to find the controller map for. The D-pad axis members will be
 *     ignored for the compare when not a D-pad input method.
 * @return The controller mapping or dsGameControllerMap_Invalid if not found.
 */
DS_APPLICATION_EXPORT dsGameControllerMap dsGameInput_findControllerMapping(
	const dsGameInput* gameInput, const dsGameInputMap* inputMap);

/**
 * @brief Gets the game input battery level.
 * @param gameInput The game input device to get the battery level from.
 * @return The battery level of the device.
 */
DS_APPLICATION_EXPORT dsGameInputBattery dsGameInput_getBattery(const dsGameInput* gameInput);

/**
 * @brief Gets the value for a game input axis.
 * @param gameInput The game input device to get the axis from.
 * @param axis The index of the axis.
 * @return The axis value.
 */
DS_APPLICATION_EXPORT float dsGameInput_getAxis(const dsGameInput* gameInput, uint32_t axis);

/**
 * @brief Gets the value for a game input axis based on the game controller mapping.
 *
 * If the axis doesn't exist the value will be 0. If the mapping is a button, a value of 1 will be
 * returned. More information about the mapping cam be queried from gameInput->controllerMapping.
 *
 * @param gameInput The game input device to get the axis from.
 * @param mapping The controller mapping.
 * @return The axis value.
 */
DS_APPLICATION_EXPORT float dsGameInput_getControllerAxis(const dsGameInput* gameInput,
	dsGameControllerMap mapping);

/**
 * @brief Gets whether or not a game input button is pressed.
 * @param gameInput The game input device to get the button state from.
 * @param button The button to check.
 * @return True if the button is pressed.
 */
DS_APPLICATION_EXPORT bool dsGameInput_isButtonPressed(const dsGameInput* gameInput,
	uint32_t button);

/**
 * @brief Gets whether or not a game input button is pressed based on the game controller mapping.
 *
 * If the button doesn't exist the value will be false. If the mapping is an axis, true will be
 * returned if the axis value is at least 0.5. More information about the mapping cam be queried
 * from gameInput->controllerMapping.
 *
 * @param gameInput The game input device to get the button state from.
 * @param mapping The controller mapping.
 * @return True if the button is pressed.
 */
DS_APPLICATION_EXPORT bool dsGameInput_isControllerButtonPressed(const dsGameInput* gameInput,
	dsGameControllerMap mapping);

/**
 * @brief Gets the the D-pad direction for a game input.
 * @remark errno will be set on failure.
 * @param[out] outDirection The direction the D-pad is in.
 * @param gameInput The game input device to get the hat direction from.
 * @param dpad The D-pad to check.
 * @return False if the D-pad state couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsGameInput_getDPadDirection(dsVector2i* outDirection,
	const dsGameInput* gameInput, uint32_t dpad);

/**
 * @brief Sets rumble on a game input.
 * @remark errno will be set on failure.
 * @param gameInput The game input device to start the rumble on.
 * @param lowFrequencyStrength The strength of the low frequency rumble, typically on the left, in
 *     the range [0, 1].
 * @param highFrequencyStrength The strength of the high frequency rumble, typically on the right,
 *     in the range [0, 1].
 * @param duration The duration to rumble for in seconds. This shouldn't be set to long values, as
 *     values larger than 1 minute may be ignored.
 * @return False if rumble couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsGameInput_setRumble(dsGameInput* gameInput, float lowFrequencyStrength,
	float highFrequencyStrength, float duration);

/**
 * @brief Sets rumble on the triggers of a game input.
 * @remark errno will be set on failure.
 * @param gameInput The game input device to start the rumble on.
 * @param leftStrength The strength of the left trigger in the range [0, 1].
 * @param rightStrength The strength of the right trigger in the range [0, 1].
 * @param duration The duration to rumble for in seconds. This shouldn't be set to long values, as
 *     values larger than 1 minute may be ignored.
 * @return False if rumble couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsGameInput_setTriggerRumble(dsGameInput* gameInput, float leftStrength,
	float rightStrength, float duration);

/**
 * @brief Sets the color of the LED on a game input.
 * @remark errno will be set on failure.
 * @param gameInput The game input to set the LED color on.
 * @param color The color of the LED.
 * @return False if the LED color couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsGameInput_setLEDColor(dsGameInput* gameInput, dsColor color);

/**
 * @brief Gets wwhether or not a game input has a motion sensor.
 * @param gameInput The game input to check.
 * @param type The motion sensor type to check.
 * @return Whether or not the game input has the motion sensor.
 */
DS_APPLICATION_EXPORT bool dsGameInput_hasMotionSensor(const dsGameInput* gameInput,
	dsMotionSensorType type);

/**
 * @brief Gets the data for a game input motion sensor.
 * @remark errno will be set on failure.
 * @param[out] outData The data to populate.
 * @param gameInput The game input to get the data for.
 * @param type The type of motion sensor to get the data for.
 * @return False if the data couldn't be retrieved.
 */
DS_APPLICATION_EXPORT bool dsGameInput_getMotionSensorData(dsVector3f* outData,
	const dsGameInput* gameInput, dsMotionSensorType type);

#ifdef __cplusplus
}
#endif
