/*
 * Copyright 2017-2025 Aaron Barany
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
 * @brief Functions for creating and manipulating fonts.
 * @remark Opertions on dsFont are thread-safe and mutex protected with the parent dsFaceGroup and
 *     other font operations.
 * @see dsFont
 */

/**
 * @brief Gets the expected font size for a given quality.
 * @param quality The text quality.
 * @return The expected font size.
 */
DS_TEXT_EXPORT uint8_t dsFont_sizeForQuality(dsTextQuality quality);

/**
 * @brief Creates a font.
 * @remark errno will be set on failure.
 * @param group The face group to get the faces from.
 * @param resourceManager The resource manager to create the graphics resources.
 * @param allocator The allocator to create the font with. If NULL, it will use the same allocator
 *     as the face group.
 * @param faceNames The names of the faces to use from the face group. When a glyph is requested, it
 *     will use the first face that contains that glyph. This way multiple faces may be used
 *     for different language sets.
 * @param faceCount The number of faces in faceNames.
 * @param icons Icons to display as part of the font, or NULL if no icons are used.
 * @param quality The quality of the rendered text.
 * @param cacheSize The size of the text cache.
 * @return The created font, or NULL if the font couldn't be created.
 */
DS_TEXT_EXPORT dsFont* dsFont_create(dsFaceGroup* group, dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* const* faceNames, uint32_t faceCount,
	const dsTextIcons* icons, dsTextQuality quality, dsTextCache cacheSize);

/**
 * @brief Gets the allocator for a font.
 * @param font The font.
 * @return The allocator.
 */
DS_TEXT_EXPORT dsAllocator* dsFont_getAllocator(const dsFont* font);

/**
 * @brief Gets the face group for a font.
 * @param font The font.
 * @return The face group.
 */
DS_TEXT_EXPORT const dsFaceGroup* dsFont_getFaceGroup(const dsFont* font);

/**
 * @brief Gets the number of faces in the font.
 * @param font The font.
 * @return The number of faces.
 */
DS_TEXT_EXPORT uint32_t dsFont_getFaceCount(const dsFont* font);

/**
 * @brief Gets the name for a face within the font.
 * @param font The font.
 * @param face The index of the font.
 * @return The name of the face.
 */
DS_TEXT_EXPORT const char* dsFont_getFaceName(const dsFont* font, uint32_t face);

/**
 * @brief Gets the icons for a font.
 * @param font The font.
 * @return The icons for the font or NULL if no icons were set.
 */
DS_TEXT_EXPORT const dsTextIcons* dsFont_getIcons(const dsFont* font);

/**
 * @brief Gets the text rendering quality of a font.
 * @param font The font.
 * @return The quality.
 */
DS_TEXT_EXPORT dsTextQuality dsFont_getTextQuality(const dsFont* font);

/**
 * @brief Gets the texture for the font.
 * @param font The font.
 * @return The texture.
 */
DS_TEXT_EXPORT dsTexture* dsFont_getTexture(const dsFont* font);

/**
 * @brief Applies hinting and anti-aliasing to a style.
 *
 * For smaller sizes, this will "hint" the text, which will add to the embolden value of the style,
 * as well as a corresponding amount to outlinePosition and outlineThickness. It will also set the
 * antiAlias value to an appropriate value for the size.
 *
 * Since this may add to the embolden, outlinePosition, and outlineThickness values, it's not
 * advisable to call this multiple times on the same style.
 *
 * @remark errno will be set on failure.
 * @param font The font.
 * @param[inout] style The style to apply hinting and anti-aliasing to.
 * @param pixelScale The value to multiply by the scale of the style to get the size in pixels.
 * @param fuziness The amount to blur the text for anti-aliasing. A value < 1 will be sharper, while
 *     a value > 1 will be blurrier. A value of 1 is default.
 * @return False if the parameters are invalid.
 */
DS_TEXT_EXPORT bool dsFont_applyHintingAndAntiAliasing(const dsFont* font, dsTextStyle* style,
	float pixelScale, float fuziness);

/**
 * @brief Preloads glyphs from a string.
 * @remark This will load the direct mapping from codepoint to glyph. In cases where glyphs depend
 *     on the surrounding text (e.g. Arabic) this will usually not be the glyph used in actual text.
 * @remark errno will be set on failure.
 * @param font The font to pre-load glyphs for.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @param string The string containing code points to pre-load glyphs for.
 * @param type The unicode type for the string.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsFont_preloadGlyphs(dsFont* font, dsCommandBuffer* commandBuffer,
	const void* string, dsUnicodeType type);

/**
 * @brief Preloads glyphs from a string.
 * @remark This will load the direct mapping from codepoint to glyph. In cases where glyphs depend
 *     on the surrounding text (e.g. Arabic) this will usually not be the glyph used in actual text.
 * @remark errno will be set on failure.
 * @param font The font to pre-load glyphs for.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @param string The string containing code points to pre-load glyphs for in UTF-8.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsFont_preloadGlyphsUTF8(
	dsFont* font, dsCommandBuffer* commandBuffer, const char* string);

/**
 * @brief Preloads glyphs from a string.
 * @remark This will load the direct mapping from codepoint to glyph. In cases where glyphs depend
 *     on the surrounding text (e.g. Arabic) this will usually not be the glyph used in actual text.
 * @remark errno will be set on failure.
 * @param font The font to pre-load glyphs for.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @param string The string containing code points to pre-load glyphs for in UTF-16.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsFont_preloadGlyphsUTF16(
	dsFont* font, dsCommandBuffer* commandBuffer, const uint16_t* string);

/**
 * @brief Preloads glyphs from a string.
 * @remark This will load the direct mapping from codepoint to glyph. In cases where glyphs depend
 *     on the surrounding text (e.g. Arabic) this will usually not be the glyph used in actual text.
 * @remark errno will be set on failure.
 * @param font The font to pre-load glyphs for.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @param string The string containing code points to pre-load glyphs for in UTF-32.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsFont_preloadGlyphsUTF32(
	dsFont* font, dsCommandBuffer* commandBuffer, const uint32_t* string);

/**
 * @brief Preloads glyphs for ASCII characters.
 * @remark errno will be set on failure.
 * @param font The font to pre-load glyphs for.
 * @param commandBuffer The command buffer to place texture commands onto.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsFont_preloadASCII(dsFont* font, dsCommandBuffer* commandBuffer);

/**
 * @brief Destroyes the font.
 * @remark errno will be set on failure.
 * @param font The font o destroy.
 * @return False if the font couldn't be destroyed.
 */
DS_TEXT_EXPORT bool dsFont_destroy(dsFont* font);

#ifdef __cplusplus
}
#endif
