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
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used for mouse events.
 */

/**
 * @brief Enum for a mouse button.
 *
 * This is for the most common buttons. (left, right, middle) Additional buttons may be used by
 * setting the bit directly.
 */
typedef enum dsMouseButton
{
	dsMouseButton_Left = 0x1,  ///< Left mouse button.
	dsMouseButton_Right = 0x2, ///< Right mouse button.
	dsMouseButton_Middle = 0x4 ///< Middle mouse button.
} dsMouseButton;

/**
 * @brief Gets a mouse button by index.
 * @param x The mouse button index.
 * @return The mouse button bit.
 */
#define DS_MOUSE_BUTTON(x) (dsMouseButton)(1 << x)

/**
 * @brief Structure containing information about a mouse button event.
 */
typedef struct dsMouseButtonEvent
{
	/**
	 * @brief The ID for the mouse.
	 */
	uint32_t mouseID;

	/**
	 * @brief The button that was pressed or released.
	 *
	 * Only a single bit will be set.
	 */
	dsMouseButton button;

	/**
	 * @brief The position of the mouse relative to the window in display coordinates.
	 */
	dsVector2i position;
} dsMouseButtonEvent;

/**
 * @brief Structure containing information about a mouse move event.
 */
typedef struct dsMouseMoveEvent
{
	/**
	 * @brief The ID for the mouse.
	 */
	uint32_t mouseID;

	/**
	 * @brief The position of the mouse relative to the window in display coordinates.
	 */
	dsVector2i position;

	/**
	 * @brief The amount scrolled in the X direction in display coordinates.
	 */
	dsVector2i delta;
} dsMouseMoveEvent;

/**
 * @brief Structure containing information about a mouse wheel moved event.
 */
typedef struct dsMouseWheelEvent
{
	/**
	 * @brief The ID for the mouse.
	 */
	uint32_t mouseID;

	/**
	 * @brief The position of the mouse relative to the window in display coordinates.
	 */
	dsVector2i position;

	/**
	 * @brief The amount scrolled in the direction.
	 */
	dsVector2i delta;

	/**
	 * @brief True if the Y direction is flipped due to "natural" scrolling.
	 */
	bool yFlipped;
} dsMouseWheelEvent;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsMouseButton);
/// @endcond
