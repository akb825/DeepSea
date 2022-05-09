/*
 * Copyright 2017-2022 Aaron Barany
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
#include <DeepSea/Math/Types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used for game input events.
 */

/**
 * @brief Enum for standard components of a game controller.
 *
 * This should map cleanly to most standard game controllers, such as XBox, PlayStation, and
 * Nintendo Switch.
 *
 * Face buttons are numbered rather than named. These correspond to a common location on the
 * controller, though the letters or symbols may differ depending on the model. For example,
 * A/B and X/Y are swapped on an XBox compared to Nintendo Switch controller.
 *
 * The layout is as follows:
 * ```
 *    3
 * 2     1
 *    0
 * ```
 */
typedef enum dsGameControllerMap
{
	dsGameControllerMap_Invalid = -1,  ///< Invalid mapping used for error results.
	dsGameControllerMap_LeftXAxis,     ///< The X axis controlled by the left control stick.
	dsGameControllerMap_LeftYAxis,     ///< The Y axis controlled by the left control stick.
	dsGameControllerMap_RightXAxis,    ///< The X axis controlled by the right control stick.
	dsGameControllerMap_RightYAxis,    ///< The Y axis controlled by the right control stick.
	dsGameControllerMap_DPadUp,        ///< The up button for the D-pad.
	dsGameControllerMap_DPadDown,      ///< The down button for the D-pad.
	dsGameControllerMap_DPadLeft,      ///< The left button for the D-pad.
	dsGameControllerMap_DPadRight,     ///< The right button for the D-pad.
	dsGameControllerMap_FaceButton0,   ///< The first face button.
	dsGameControllerMap_FaceButton1,   ///< The second face button.
	dsGameControllerMap_FaceButton2,   ///< The third face button.
	dsGameControllerMap_FaceButton3,   ///< The fourth face button.
	dsGameControllerMap_Start,         ///< The start/+ button.
	dsGameControllerMap_Select,        ///< The select/back/- button.
	dsGameControllerMap_Home,          ///< The home/guide button.
	dsGameControllerMap_LeftStick,     ///< Button for pressing the left control stick.
	dsGameControllerMap_RightStick,    ///< Button for pressing the right control stick.
	dsGameControllerMap_LeftShoulder,  ///< The left shoulder button.
	dsGameControllerMap_RightShoulder, ///< The right shoulder button.
	dsGameControllerMap_LeftTrigger,   ///< The left shoulder trigger.
	dsGameControllerMap_RightTrigger,  ///< The right shoulder trigger.
	dsGameControllerMap_Paddle0,       ///< First paddle.
	dsGameControllerMap_Paddle1,       ///< Second paddle.
	dsGameControllerMap_Paddle2,       ///< Third paddle.
	dsGameControllerMap_Paddle3,       ///< Fourth paddle.
	dsGameControllerMap_Touchpad,      ///< Touchpad button.
	dsGameControllerMap_MiscButton0,   ///< The share/microphone/camera button.
	dsGameControllerMap_Count          ///< The number of game controller input maps.
} dsGameControllerMap;

/**
 * @brief Enum for a method of input for a game input.
 */
typedef enum dsGameInputMethod
{
	dsGameInputMethod_Invalid = -1, ///< Input method isn't provided.
	dsGameInputMethod_Axis,         ///< Analog axis.
	dsGameInputMethod_Button,       ///< A button that is either pressed or not.
	dsGameInputMethod_DPad          ///< A D-pad or hat.
} dsGameInputMethod;

/**
 * @brief Struct defining a mapping to a specific input on a game input device.
 */
typedef struct dsGameInputMap
{
	/**
	 * @brief The input method.
	 */
	dsGameInputMethod method;

	/**
	 * @brief The index of the input method.
	 */
	uint32_t index;
} dsGameInputMap;

/// @cond
typedef struct dsGameInput dsGameInput;
/// @endcond

/**
 * @brief Struct containing information about connecting a game input.
 */
typedef struct dsGameInputConnectEvent
{
	/**
	 * @brief The game input device.
	 */
	const dsGameInput* gameInput;
} dsGameInputConnectEvent;

/**
 * @brief Struct containing information about moving a game input axis.
 */
typedef struct dsGameInputAxisEvent
{
	/**
	 * @brief The game input device.
	 */
	const dsGameInput* gameInput;

	/**
	 * @brief The game controller mapping, if available.
	 */
	dsGameControllerMap mapping;

	/**
	 * @brief The axis that was modified.
	 *
	 * This will be 0 if mapping is set since it may not map to an actual axis. See
	 * gameInput->controllerMapping for more detailed information.
	 */
	uint32_t axis;

	/**
	 * @brief The value of the axis.
	 */
	float value;
} dsGameInputAxisEvent;

/**
 * @brief Struct containing information about a game input button press or release.
 */
typedef struct dsGameInputButtonEvent
{
	/**
	 * @brief The game input device.
	 */
	const dsGameInput* gameInput;

	/**
	 * @brief The game controller mapping, if available.
	 */
	dsGameControllerMap mapping;

	/**
	 * @brief The button that was pressed or released.
	 *
	 * This will be 0 if mapping is set since it may not map to an actual button. See
	 * gameInput->controllerMapping for more detailed information.
	 */
	uint32_t button;
} dsGameInputButtonEvent;

/**
 * @brief Struct containing information about a game input trackball movement.
 */
typedef struct dsGameInputBallEvent
{
	/**
	 * @brief The game input device.
	 */
	const dsGameInput* gameInput;

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
} dsGameInputBallEvent;

/**
 * @brief Struct containing information about a game input D-pad movement.
 */
typedef struct dsGameInputDPadEvent
{
	/**
	 * @brief The game input device.
	 */
	const dsGameInput* gameInput;

	/**
	 * @brief The index of the D-pad.
	 */
	uint32_t dpad;

	/**
	 * @brief The direction the D-pad is in.
	 */
	dsVector2i direction;
} dsGameInputDPadEvent;

#ifdef __cplusplus
}
#endif
