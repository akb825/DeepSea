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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used by the DeepSea/Text library.
 */

/**
 * @brief Log tag used by the text library.
 */
#define DS_TEXT_LOG_TAG "text"

/**
 * @brief Enum for the quality of the text.
 *
 * The text quality will directly correlate to how much texture memory is required for each font.
 * - Low: 256 KB
 * - Medium: 1 MB
 * - High: 4 MB
 */
typedef enum dsTextQuality
{
	dsTextQuality_Low,    ///< Low quality with lower memory and CPU usage.
	dsTextQuality_Medium, ///< Tradeoff between quality and memory and CPU usage.
	dsTextQuality_High    ///< High quality with more memory and CPU usage.
} dsTextQuality;

/**
 * @brief Struct containing a shared group of faces for fonts.
 * @see FaceGroup.h
 */
typedef struct dsFaceGroup dsFaceGroup;

/**
 * @brief Struct describing a single face wtihin the font.
 */
typedef struct dsFontFace dsFontFace;

/**
 * @brief Struct containing information about a font.
 * @see Font.h
 */
typedef struct dsFont dsFont;

/**
 * @brief Struct containing information about a glyph.
 */
typedef struct dsGlyph
{
	/**
	 * @brief The ID of the glpyh.
	 */
	uint32_t glyphId;

	/**
	 * @brief The index of the character in the string.
	 */
	uint32_t charIndex;

	/**
	 * @brief True if a line break is safe on this glyph.
	 */
	bool canBreak;

	/**
	 * @brief The offset before drawing the glyph.
	 */
	dsVector2f offset;

	/**
	 * @brief The amount to advance to the next glyph.
	 */
	dsVector2f advance;
} dsGlyph;

/**
 * @brief Struct containing information about a range of text.
 *
 * Each range that has different properties will have an entry. This is largely for internal use,
 * but information such as right to left ranges are important for external use.
 */
typedef struct dsTextRange
{
	/**
	 * @brief The face that the range will be drawn with.
	 */
	dsFontFace* face;

	/**
	 * @brief The first character in the range.
	 */
	uint32_t firstChar;

	/**
	 * @brief The number of characters in the range.
	 */
	uint32_t charCount;

	/**
	 * @brief The first glyph in the range.
	 */
	uint32_t firstGlyph;

	/**
	 * @brief The number of glyphes in the range.
	 */
	uint32_t glyphCount;

	/**
	 * @brief True if the text is vertical.
	 */
	bool vertical;

	/**
	 * @brief True if the text goes backward.
	 */
	bool backward;
} dsTextRange;

/**
 * @brief Struct containing information about text.
 * @see Text.h
 */
typedef struct dsText
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The font that this text will be drawn with.
	 */
	dsFont* font;

	/**
	 * @brief The characters in string as UTF-32.
	 */
	const uint32_t* characters;

	/**
	 * @brief The glyphs used with the string.
	 */
	const dsGlyph* glyphs;

	/**
	 * @brief The ranges with unique properties for the text.
	 */
	const dsTextRange* ranges;

	/**
	 * @brief The number of characters in the string.
	 */
	uint32_t characterCount;

	/**
	 * @brief The number of glyphs;
	 */
	uint32_t glyphCount;

	/**
	 * @brief The number of ranges.
	 */
	uint32_t rangeCount;
} dsText;

#ifdef __cplusplus
}
#endif
