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
 * @see FontGroup.h
 */
typedef struct dsFaceGroup dsFaceGroup;

/**
 * @brief Struct containing information about a font.
 * @see Font.h
 */
typedef struct dsFont dsFont;

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
	 * @brief The characters in string as UTF-32.
	 */
	const uint32_t* characters;

	/**
	 * @brief The number of characters in the string.
	 */
	size_t characterCount;

	/**
	 * @brief The glyphs used with the string.
	 */
	const uint32_t* glyphs;

	/**
	 * @brief The number of glyphs;
	 */
	size_t glyphCount;
} dsText;

#ifdef __cplusplus
}
#endif
