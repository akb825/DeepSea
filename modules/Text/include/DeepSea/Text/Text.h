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
 *
 * @remark Opertions on dsText are thread-safe and mutex protected with the parent dsFont and
 * dsFaceGroup.
 *
 * @remark When uniformScript is false, an external library will compute the bi-directional runs,
 * which will allocate memory on the heap outside of the allocator provided. (i.e. calling malloc
 * directly) The memory is temporary and will be freed by the time the function returns.
 *
 * @see dsText
 */

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string. This will be copied.
 * @param type The type of Unicode the string is encoded in.
 * @param uniformScript True if the text will have a uniform script. This will skip bi-directional
 *     text detection and using different faces based on different scripts within the same string.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_create(dsFont* font, dsAllocator* allocator, const void* string,
	dsUnicodeType type,  bool uniformScript);

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-8. This will be copied.
 * @param uniformScript True if the text will have a uniform script. This will skip bi-directional
 *     text detection and using different faces based on different scripts within the same string.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF8(dsFont* font, dsAllocator* allocator, const char* string,
	bool uniformScript);

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-16. This will be copied.
 * @param uniformScript True if the text will have a uniform script. This will skip bi-directional
 *     text detection and using different faces based on different scripts within the same string.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF16(dsFont* font, dsAllocator* allocator,
	const uint16_t* string, bool uniformScript);

/**
 * @brief Creates a text object from a string.
 * @remark errno will be set on failure.
 * @param font The font to draw the text with.
 * @param allocator The allocator to create the text with. If NULL, it will use the same allocator
 *     as the font.
 * @param string The string in UTF-32. This will be copied.
 * @param uniformScript True if the text will have a uniform script. This will skip bi-directional
 *     text detection and using different faces based on different scripts within the same string.
 * @return The text, or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsText* dsText_createUTF32(dsFont* font, dsAllocator* allocator,
	const uint32_t* string, bool uniformScript);

/**
 * @brief Destroys a text object.
 * @param text The text to destroy.
 */
DS_TEXT_EXPORT void dsText_destroy(dsText* text);

#ifdef __cplusplus
}
#endif
