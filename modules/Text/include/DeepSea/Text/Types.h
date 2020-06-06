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
 * @brief The number of slots available for glyphs in a small cache.
 *
 * This is based on the number of slots available in different mip levels of the texture used for
 * storage. (16*16 + 8*8 + 4*4 + 2*2 + 1, using mip levels large enough for glyphs) This is the
 * number of unique glyphs that can be drawn before slots are overwritten.
 */
#define DS_SMALL_CACHE_GLYPH_SLOTS 341

/**
 * @brief The number of slots available for glyphs in a large cache.
 *
 * This is based on the number of slots available in different mip levels of the texture used for
 * storage. (32*32 + 16*16 + 8*8 + 4*4 + 2*2 + 1, using mip levels large enough for glyphs) This is
 * the number of unique glyphs that can be drawn before slots are overwritten.
 */
#define DS_LARGE_CACHE_GLYPH_SLOTS 1365

/**
 * @brief Enum for the size of the text cache.
 */
typedef enum dsTextCache
{
	dsTextCache_Small, ///< Small text cache, saving memory when few glyphs are required.
	dsTextCache_Large  ///< Large text cache, more suitable for international text.
} dsTextCache;

/**
 * @brief Enum for the quality of the text.
 *
 * The text quality will directly correlate to how much texture memory is required for each font.
 * When the large cache size is used, the memory used will be:
 * - Low: 333 KB
 * - Medium: 1.33 MB
 * - High: 3 MB
 * - VeryHigh: 5.32 MB
 *
 * Memory usage is 1/4 of the above values when the small cache size is used.
 */
typedef enum dsTextQuality
{
	dsTextQuality_Low,     ///< The lowest cost, best used for small text.
	dsTextQuality_Medium,  ///< Tradeoff for cost and quality, suitable for most text.
	dsTextQuality_High,    ///< High quality. This has higher cost, but will give better results for
	                       ///  large text or highly detailed glyphs. (e.g. Chinese)
	dsTextQuality_VeryHigh ///< Highest quality, but also highest cost.
} dsTextQuality;

/**
 * @brief Enum for the alignment of text.
 * @remark Start will be the same as left for left to right text, and the same as right for right
 * to left text. End is the same but in reverse.
 */
typedef enum dsTextAlign
{
	dsTextAlign_Start, ///< Align in the side of the bounds at the start of the text.
	dsTextAlign_End,   ///< Align in the side of the bounds at the end of the text.
	dsTextAlign_Left,  ///< Align in the left of the bounds.
	dsTextAlign_Right, ///< Align in the right of the bounds.
	dsTextAlign_Center ///< Align in the center of the bounds.
} dsTextAlign;

/**
 * @brief Enum for the type of unicode.
 */
typedef enum dsUnicodeType
{
	dsUnicodeType_UTF8,  ///< UTF-8
	dsUnicodeType_UTF16, ///< UTF-16
	dsUnicodeType_UTF32  ///< UTF-32
} dsUnicodeType;

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
 * @brief Mapping from a character to the glyphs it corresponds to.
 */
typedef struct dsCharMapping
{
	/**
	 * @brief The index of the first glyph that's associated with the character.
	 */
	uint32_t firstGlyph;

	/**
	 * @brief The number of lgyphs associated with the character.
	 */
	uint32_t glyphCount;
} dsCharMapping;

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
	 * @brief Mapping from characters to the corresponding glyphs.
	 */
	const dsCharMapping* charMappings;

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

	/**
	 * @brief Static offset to apply to the glyphs. (e.g. for superscript)
	 */
	float verticalOffset;
} dsTextStyle;

/**
 * @brief Struct containing information about a glyph in the layout.
 * @see TextLayout.h
 */
typedef struct dsGlyphLayout
{
	/**
	 * @brief The position of the glyph.
	 * @remark The position will be set to FLT_MAX for spaces beyond the end of line. Note that this
	 * may be in the middle of the line when there's a transition between right to left and left to
	 * right text. (which is the motivation of disabling them)
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
	 * @brief The index for the style.
	 *
	 * This indexes into the styles array in dsTextLayout.
	 */
	uint32_t styleIndex;
} dsGlyphLayout;

/**
 * @brief Struct defining a line of text.
 * @see TextLayout.h
 */
typedef struct dsTextLine
{
	/**
	 * @brief The first character in the range for the line.
	 */
	uint32_t start;

	/**
	 * @brief The number of characters in the range for the line.
	 */
	uint32_t count;

	/**
	 * @brief The bounds for the line.
	 */
	dsAlignedBox2f bounds;
} dsTextLine;

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
	 * @brief List of glyph indicies ordered such that glyphs on the same line are grouped together.
	 */
	const uint32_t* glyphsLineOrdered;

	/**
	 * @brief The lines of text.
	 *
	 * This will only be computed if the allocator supports freeing memory. Only lines that contain
	 * characters (i.e. non-empty lines) are included.
	 */
	dsTextLine* lines;

	/**
	 * @brief The styles that are used with the text.
	 *
	 * The style values may be changed after the layout has been created. However, the ranges should
	 * remain the same.
	 */
	dsTextStyle* styles;

	/**
	 * @brief The number of lines.
	 */
	uint32_t lineCount;

	/**
	 * @brief The maximum number of lines.
	 */
	uint32_t maxLines;

	/**
	 * @brief The number of styles.
	 */
	uint32_t styleCount;

	/**
	 * @brief The bounds of the layed out text.
	 *
	 * This will be the logical size of the text, not including embolding or slanting. The origin
	 * will be on the bottom of the first line at the base alignment position.
	 * (i.e. left, right, or center of text) Positive Y points down. The intent for this box is to
	 * place the block of text on the screen.
	 */
	dsAlignedBox2f bounds;
} dsTextLayout;

/**
 * @brief Function for getting the data for a glyph.
 * @param userData The user data for the function.
 * @param layout The text layout that will be added.
 * @param layoutUserData The user data provided with the layout.
 * @param glyphIndex The index of the glyph to add.
 * @param vertexData The vertex data to write to. You should write vertexCount vertices to this
 *     array depending on if it's 4 vertices for a quad or 1 for a tessellation shader. When writing
 *     4 vertices for a quad, it will typically be a clockwise loop. (since Y points down, the
 *     shader will typically flip it to become counter-clockwise)
 * @param format The vertex format.
 * @param vertexCount The number of vertices to write. This will either be 4 vertices for a quad,
 *     which should follow winding order, or 1 vertex when using the tessellation shader.
 */
typedef void (*dsGlyphDataFunction)(void* userData, const dsTextLayout* layout,
	void* layoutUserData, uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format,
	uint32_t vertexCount);

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
