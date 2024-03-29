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
 * @brief Includes all of the types used for touch events.
 */

/// @cond
typedef struct dsGameInput dsGameInput;
/// @endcond

/**
 * @brief Struct containing information about a touch event.
 */
typedef struct dsTouchEvent
{
	/**
	 * @brief The game input that the touch is associated with, if any.
	 */
	const dsGameInput* gameInput;

	/**
	 * @brief ID for the touch.
	 */
	uint64_t touchID;

	/**
	 * @brief ID for the finger.
	 */
	uint64_t fingerID;

	/**
	 * @brief The position of the finger in normalized [0, 1] coordinates.
	 */
	dsVector2f position;

	/**
	 * @brief The amount moved in the X direction in normalized [-1, 1] coordinates.
	 *
	 * This may be 0 if the underlying implementation doesn't track the delta between all events.
	 */
	dsVector2f delta;

	/**
	 * @brief The pressure of the touch in normalized [0, 1] coordinates.
	 */
	float pressure;
} dsTouchEvent;

/**
 * @brief Struct containing information about a multi-touch event, supporting pinching and rotating.
 */
typedef struct dsMultiTouchEvent
{
	/**
	 * @brief ID for the touch.
	 */
	uint64_t touchID;

	/**
	 * @brief The rotation of the gesture in radians.
	 */
	float rotation;

	/**
	 * @brief The amount the fingers were pinched.
	 */
	float pinch;

	/**
	 * @brief The position of the gesture in normalized [0, 1] coordinates.
	 */
	dsVector2f position;

	/**
	 * @brief The number of fingers in the gesture.
	 */
	uint32_t fingerCount;
} dsMultiTouchEvent;

#ifdef __cplusplus
}
#endif
