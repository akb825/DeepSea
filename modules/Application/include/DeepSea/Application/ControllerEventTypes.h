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

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used for controller and joystick events.
 */

/**
 * @brief Enum for the controller axis.
 */
typedef enum dsControllerAxis
{
	dsControllerAxis_LeftX,       ///< Left control stick X axis.
	dsControllerAxis_LeftY,       ///< Left control stick Y axis.
	dsControllerAxis_RightX,      ///< Right control stick X axis.
	dsControllerAxis_RightY,      ///< Right control stick Y axis.
	dsControllerAxis_LeftTrigger, ///< Left trigger button.
	dsControllerAxis_RightTrigger ///< Right trigger button.
} dsContrllerAxis;

/**
 * @brief Enum for the controller button.
 *
 * This corresponds to the standard XBox-style controller.
 */
typedef enum dsControllerButton
{
	dsControllerButton_A,
	dsControllerButton_B,
	dsControllerButton_X,
	dsControllerButton_Y,
	dsControllerButton_Back,
	dsControllerButton_Guide,
	dsControllerButton_Start,
	dsControllerButton_LeftStick,
	dsControllerButton_RightStick,
	dsControllerButton_LeftShoulder,
	dsControllerButton_RightShoulder,
	dsControllerButton_Up,
	dsControllerButton_Down,
	dsControllerButton_Left,
	dsControllerButton_Right
} dsControllerButton;

/// @cond
typedef struct dsController dsController;
/// @endcond

/**
 * @brief Struct containing information about connecting a controller.
 */
typedef struct dsControllerConnectEvent
{
	/**
	 * @brief The controller.
	 */
	const dsController* controller;
} dsControllerConnectEvent;

/**
 * @brief Struct containing information about moving an axis.
 */
typedef struct dsControllerAxisEvent
{
	/**
	 * @brief The controller.
	 */
	const dsController* controller;

	/**
	 * @brief The axis that was modified.
	 */
	uint32_t axis;

	/**
	 * @brief The value of the axis.
	 */
	float value;
} dsControllerAxisEvent;

/**
 * @brief Struct containing information about a controller button press or release.
 */
typedef struct dsControllerButtonEvent
{
	/**
	 * @brief The controller.
	 */
	const dsController* controller;

	/**
	 * @brief The button that was pressed or released.
	 */
	uint32_t button;
} dsControllerButtonEvent;

/**
 * @brief Struct containing information about a joystick trackball movement.
 */
typedef struct dsJoystickBallEvent
{
	/**
	 * @brief The controller.
	 */
	const dsController* controller;

	/**
	 * @brief The index of the ball.
	 */
	uint32_t ball;

	/**
	 * @brief The movement in the X direction.
	 */
	int32_t deltaX;

	/**
	 * @brief The movement in the Y direction.
	 */
	int32_t deltaY;
} dsJoystickBallEvent;

/**
 * @brief Struct containing information about a joystick hat movement.
 */
typedef struct dsJoystickHatEvent
{
	/**
	 * @brief The controller.
	 */
	const dsController* controller;

	/**
	 * @brief The index of the hat.
	 */
	uint32_t hat;

	/**
	 * @brief The X direction of the hat, -1 for left, 0 for center, and 1 for right.
	 */
	int8_t x;

	/**
	 * @brief The Y direction of the hat, -1 for down, 0 for center, and 1 for up.
	 */
	int8_t y;
} dsJoystickHatEvent;

#ifdef __cplusplus
}
#endif
