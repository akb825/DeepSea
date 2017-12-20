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
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Types.h>

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
 * @brief The number of slots available for glyphs.
 *
 * This is based on the number of slots available in different mip levels of the texture used for
 * storage. (32*32 + 16*16 + 8*8 + 4*4 + 2*2 + 1, using mip levels large enough for glyphs) This is
 * the number of unique glyphs that can be drawn before slots are overwritten.
 */
#define DS_GLYPH_SLOTS 1365

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
	dsTextQuality_Low,     ///< Low quality with lower memory and CPU usage.
	dsTextQuality_Medium,  ///< Tradeoff between quality and memory and CPU usage.
	dsTextQuality_High,    ///< High quality with more memory and CPU usage.
	dsTextQuality_VeryHigh ///< Even higher quality with more memory usage.
} dsTextQuality;

/**
 * @brief Enum for the justification of text.
 */
typedef enum dsTextJustification
{
	dsTextJustification_Left,  ///< Align in the left of the bounds.
	dsTextJustification_Right, ///< Align in the right of the bounds.
	dsTextJustification_Center ///< Align in the center of the bounds.
} dsTextJustification;

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
	 *
	 * Positive Y points down.
	 */
	dsVector2f offset;

	/**
	 * @brief The amount to advance to the next glyph.
	 */
	float advance;
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
	uint32_t face;

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
	 * @brief The number of newlines following this range.
	 */
	uint32_t newlineCount;

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

/**
 * @brief Struct containing the style of the text.
 * @see TextLayout.h
 */
typedef struct dsTextStyle
{
	/**
	 * @brief The first character in the range for this style.
	 */
	uint32_t start;

	/**
	 * @brief The number of characters in the range for this style.
	 */
	uint32_t count;

	/**
	 * @brief The scale of the text.
	 */
	float scale;

	/**
	 * @brief The amount to embolden the text.
	 *
	 * This should be in the range [-1, 1], where 0 is a standard thickness.
	 */
	float embolden;

	/**
	 * @brief The amount to slant the text.
	 *
	 * A value of -1 will slant 45 degrees to the left, and a value of 1 will be 45 degrees to the
	 * right.
	 */
	float slant;

	/**
	 * @brief The position of the outline in the range [0, 1], where 1 is further from the center.
	 */
	float outlinePosition;

	/**
	 * @brief The thickness of the outline in the range [0, 1].
	 *
	 * Set to 0 to have no outline.
	 */
	float outlineThickness;

	/**
	 * @brief The amount to anti-alias the text in a range [0, 1].
	 *
	 * Set to 0 for no anti-aliasing.
	 */
	float antiAlias;

	/**
	 * @brief The color of the text.
	 */
	dsColor color;

	/**
	 * @brief The color of the text outline.
	 */
	dsColor outlineColor;
} dsTextStyle;

/**
 * @brief Struct containing information about a glyph in the layout.
 * @see TextLayout.h
 */
typedef struct dsGlyphLayout
{
	/**
	 * @brief The position of the glyph.
	 */
	dsVector2f position;

	/**
	 * @brief The geometry of the glyph.
	 *
	 * This will not have any slanting applied. The origin as at the origin of the glyph, and
	 * positive Y points down.
	 */
	dsAlignedBox2f geometry;

	/**
	 * @brief The texture coordinates for the glyph.
	 */
	dsAlignedBox2f texCoords;

	/**
	 * @brief The mip level in the texture that contains the glyph.
	 */
	uint32_t mipLevel;

	/**
	 * The index of the glyph in the text.
	 *
	 * This may not match in some cases such as right to left text with text wrapping.
	 */
	uint32_t textGlyphIndex;

	/**
	 * @brief The index for the style.
	 *
	 * This indexes into the styles array in dsTextLayout.
	 */
	uint32_t styleIndex;
} dsGlyphLayout;

/**
 * @brief Struct containing layout information for a piece of text
 * @see TextLayout.h
 */
typedef struct dsTextLayout
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The text to lay out.
	 */
	const dsText* text;

	/**
	 * @brief The list of laid out glyphs.
	 *
	 * This will be of size text->glyphCount.
	 */
	const dsGlyphLayout* glyphs;

	/**
	 * @brief The styles that are used with the text.
	 *
	 * The style values may be changed after the layout has been created. However, the ranges should
	 * remain the same.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of styles.
	 */
	uint32_t styleCount;

	/**
	 * @brief The bounds of the layed out text.
	 *
	 * This will be the logical size of the text, not including embolding or slanting. The origin
	 * will be on the bottom of the first line at the base justification position.
	 * (i.e. left, right, or center of text) Positive Y points down. The intent for this box is to
	 * place the block of text on the screen.
	 */
	dsAlignedBox2f bounds;
} dsTextLayout;

/**
 * @brief Function for getting the data for a glyph.
 * @param userData The user data for the function.
 * @param layout The text layout that will be added.
 * @param glyphIndex The index of the glyph to add.
 * @param data The vertex data to write to.
 * @param format The vertex format.
 * @param vertexCount The number of vertices to write. This will either be 4 vertices for a quad,
 *     which should follow winding order, or 1 vertex when using the tessellation shader.
 */
typedef void (*dsGlyphDataFunction)(void* userData, const dsTextLayout* layout, uint32_t glyphIndex,
	void* vertexData, const dsVertexFormat* format, uint32_t vertexCount);

/**
 * @brief Struct containing a buffer for rendering text.
 * @see TextRenderBuffer.h
 */
typedef struct dsTextRenderBuffer
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The geometry that will be drawn.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief Function for getting the data for a glyph.
	 */
	dsGlyphDataFunction glyphDataFunc;

	/**
	 * @brief The user data for getting the glyph data.
	 */
	void* userData;

	/**
	 * @brief The maximum number of glyphs that can be drawn at once.
	 */
	uint32_t maxGlyphs;

	/**
	 * @brief The number of glyphs that have been queued so far.
	 */
	uint32_t queuedGlyphs;

	/**
	 * @brief Temporary data.
	 */
	void* tempData;
} dsTextRenderBuffer;

#ifdef __cplusplus
}
#endif
