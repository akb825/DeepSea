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
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating text to be displayed later.
 * @remark Opertions on dsText are thread-safe and mutex protected with the parent dsFont and
 *     dsFaceGroup.
 * @see dsText
 */

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-8. This will be copied.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF8(dsFont* font, dsAllocator* allocator, const char* string);

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-16. This will be copied.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF16(dsFont* font, dsAllocator* allocator,
	const uint16_t* string);

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-32. This will be copied.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF32(dsFont* font, dsAllocator* allocator,
	const uint32_t* string);

/**
 * @brief Destroys a text object.
 * @param text The text to destroy.
 */
DS_TEXT_EXPORT void dsText_destroy(dsText* text);

#ifdef __cplusplus
}
#endif
