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

#include <DeepSea/Text/TextLayout.h>

#include "FontImpl.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Text.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int rangeCompare(const void* key, const void* member)
{
	uint32_t glyph = *(const uint32_t*)key;
	const dsTextRange* range = (const dsTextRange*)member;
	if (glyph >= range->firstGlyph && glyph < range->firstGlyph + range->glyphCount)
		return 0;

	if (glyph < range->firstGlyph)
		return -1;
	return 1;
}

static const dsTextRange* findRange(const dsTextRange* ranges, uint32_t rangeCount, uint32_t glyph)
{
	return (const dsTextRange*)bsearch(&glyph, ranges, rangeCount, sizeof(dsTextRange),
		&rangeCompare);
}

static void finishLine(dsTextLayout* layout, dsAlignedBox2f* lineBounds, float lineY,
	dsTextAlign alignment, dsGlyphLayout* glyphs, const uint32_t* glyphsLineOrdered,
	uint32_t sectionStart, uint32_t sectionEnd, uint32_t* lineCount)
{
	if (!dsAlignedBox2f_isValid(lineBounds))
		return;

	dsTextLine* line = layout->lines ? layout->lines + *lineCount : NULL;
	++*lineCount;

	// Handle alignment.
	uint32_t minCharIndex = UINT_MAX;
	uint32_t maxCharIndex = 0;
	switch (alignment)
	{
		case dsTextAlign_Right:
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
			{
				uint32_t glyphIndex = glyphsLineOrdered[i];
				dsGlyphLayout* glyph = glyphs + glyphIndex;
				glyph->position.x -= lineBounds->max.x;
				glyph->position.y = lineY;

				uint32_t charIndex = layout->text->glyphs[glyphIndex].charIndex;
				minCharIndex = dsMin(minCharIndex, charIndex);
				maxCharIndex = dsMax(maxCharIndex, charIndex);
			}
			lineBounds->min.x -= lineBounds->max.x;
			lineBounds->max.x -= 0.0f;
			break;
		case dsTextAlign_Center:
		{
			float alignmentOffset = lineBounds->max.x/2.0f;
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
			{
				uint32_t glyphIndex = glyphsLineOrdered[i];
				dsGlyphLayout* glyph = glyphs + glyphIndex;
				glyph->position.x -= alignmentOffset;
				glyph->position.y = lineY;

				uint32_t charIndex = layout->text->glyphs[glyphIndex].charIndex;
				minCharIndex = dsMin(minCharIndex, charIndex);
				maxCharIndex = dsMax(maxCharIndex, charIndex);
			}
			lineBounds->min.x -= alignmentOffset;
			lineBounds->max.x -= alignmentOffset;
			break;
		}
		default:
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
			{
				uint32_t glyphIndex = glyphsLineOrdered[i];
				dsGlyphLayout* glyph = glyphs + glyphIndex;
				glyph->position.y = lineY;

				uint32_t charIndex = layout->text->glyphs[glyphIndex].charIndex;
				minCharIndex = dsMin(minCharIndex, charIndex);
				maxCharIndex = dsMax(maxCharIndex, charIndex);
			}
			break;
	}

	lineBounds->min.y += lineY;
	lineBounds->max.y += lineY;
	dsAlignedBox2_addBox(layout->bounds, *lineBounds);

	if (line)
	{
		line->start = minCharIndex;
		line->count = maxCharIndex - minCharIndex + 1;
		line->bounds = *lineBounds;
	}

	dsAlignedBox2f_makeInvalid(lineBounds);
}

static void updateGlyph(dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face,
	uint32_t glyphId, dsGlyphLayout* glyph)
{
	// Skip empty glyphs.
	if (glyph->geometry.min.x == glyph->geometry.max.x &&
		glyph->geometry.min.y == glyph->geometry.max.y)
	{
		return;
	}

	dsGlyphInfo* glyphInfo = dsFont_getGlyphInfo(font, commandBuffer, face, glyphId);
	dsTexturePosition texturePos;
	dsFont_getGlyphTexturePos(&texturePos, dsFont_getGlyphIndex(font, glyphInfo),
		font->glyphSize, font->texMultiplier);
	glyph->mipLevel = texturePos.mipLevel;

	dsVector2f glyphSize;
	dsAlignedBox2_extents(glyphSize, glyphInfo->glyphBounds);
	dsFont_getGlyphTextureBounds(&glyph->texCoords, &texturePos, &glyphSize,
		font->glyphSize, font->texMultiplier);
}

size_t dsTextLayout_fullAllocSize(const dsText* text, uint32_t styleCount)
{
	if (!text || styleCount == 0)
		return 0;

	return DS_ALIGNED_SIZE(sizeof(dsTextLayout)) +
		DS_ALIGNED_SIZE(text->glyphCount*sizeof(dsGlyphLayout)) +
		DS_ALIGNED_SIZE(text->glyphCount*sizeof(uint32_t)) +
		DS_ALIGNED_SIZE(styleCount*sizeof(dsTextStyle));
}

bool dsTextLayout_applySlantToBounds(dsAlignedBox2f* outBounds, const dsAlignedBox2f* glyphBounds,
	float slant)
{
	if (!outBounds || !glyphBounds)
	{
		errno = EINVAL;
		return false;
	}

	if (outBounds != glyphBounds)
		*outBounds = *glyphBounds;

	// Positive y points down, so positive slant will go to the left for +y and right for -y.
	if (slant > 0.0f)
	{
		float minOffset = -glyphBounds->max.y*slant;
		float maxOffset = -glyphBounds->min.y*slant;
		outBounds->min.x += minOffset;
		outBounds->max.x += maxOffset;
	}
	else if (slant < 0.0f)
	{
		float maxOffset = -glyphBounds->max.y*slant;
		float minOffset = -glyphBounds->min.y*slant;
		outBounds->min.x += minOffset;
		outBounds->max.x += maxOffset;
	}
	return true;
}

dsTextLayout* dsTextLayout_create(dsAllocator* allocator, const dsText* text,
	const dsTextStyle* styles, uint32_t styleCount)
{
	if (!allocator || !text || !styles || styleCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	bool rangesValid = true;
	if (styles[0].start != 0)
		rangesValid = false;
	uint32_t maxRangeChar = styles[0].start + styles[0].count;
	for (uint32_t i = 1; i < styleCount && rangesValid; ++i)
	{
		if (styles[i].start != maxRangeChar)
		{
			rangesValid = false;
			break;
		}

		maxRangeChar += styles[i].count;
	}

	if (maxRangeChar < text->characterCount)
		rangesValid = false;

	if (!rangesValid)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Text style ranges must be monotomically increasing, "
			"non-overlapping, and cover the full range of text.");
		return NULL;
	}

	size_t fullSize = dsTextLayout_fullAllocSize(text, styleCount);
	DS_ASSERT(fullSize > 0);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsTextLayout* layout = DS_ALLOCATE_OBJECT(&bufferAlloc, dsTextLayout);
	layout->allocator = dsAllocator_keepPointer(allocator);
	layout->text = text;
	if (text->glyphCount > 0)
	{
		layout->glyphs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsGlyphLayout, text->glyphCount);
		DS_ASSERT(layout->glyphs);

		layout->glyphsLineOrdered = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t,
			text->glyphCount);
		DS_ASSERT(layout->glyphsLineOrdered);
	}
	else
	{
		layout->glyphs = NULL;
		layout->glyphsLineOrdered = NULL;
	}

	layout->lines = NULL;
	layout->styles = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsTextStyle, styleCount);
	DS_ASSERT(layout->styles);
	memcpy(layout->styles, styles, styleCount*sizeof(dsTextStyle));
	layout->lineCount = 0;
	layout->maxLines = 0;
	layout->styleCount = styleCount;

	dsAlignedBox2f_makeInvalid(&layout->bounds);

	return layout;
}

dsTextAlign dsTextLayout_resolveAlign(const dsTextLayout* layout,
	dsTextAlign alignment)
{
	bool valid = layout && layout->text && layout->text->ranges && layout->text->rangeCount > 0;
	switch (alignment)
	{
		case dsTextAlign_Start:
			if (valid && layout->text->ranges[0].backward)
				return dsTextAlign_Right;
			return dsTextAlign_Left;
		case dsTextAlign_End:
			if (valid && layout->text->ranges[0].backward)
				return dsTextAlign_Left;
			return dsTextAlign_Right;
		default:
			return alignment;
	}
}

bool dsTextLayout_layout(dsTextLayout* layout, dsCommandBuffer* commandBuffer,
	dsTextAlign alignment, float maxWidth, float lineScale)
{
	DS_PROFILE_FUNC_START();
	if (!layout || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	alignment = dsTextLayout_resolveAlign(layout, alignment);

	const dsText* text = layout->text;
	dsFont* font = text->font;
	dsGlyphLayout* glyphs = (dsGlyphLayout*)layout->glyphs;
	uint32_t* glyphsLineOrdered = (uint32_t*)layout->glyphsLineOrdered;

	if (text->glyphCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	// First pass: get the initial cached glyph info and base positions.
	// This part accesses shared resources and needs to be locked.
	dsFaceGroup_lock(font->group);

	DS_ASSERT(layout->styleCount > 0);
	DS_ASSERT(text->rangeCount > 0);
	DS_PROFILE_SCOPE_START("Cache Glyphs");
	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		const dsTextRange* range = text->ranges + i;
		const dsTextStyle* style = layout->styles;
		uint32_t styleRangeLimit = style->count;
		while (range->firstChar >= styleRangeLimit)
		{
			++style;
			DS_ASSERT(style < layout->styles + layout->styleCount);
			DS_ASSERT(style->start == styleRangeLimit);
			styleRangeLimit += style->count;
		}
		DS_ASSERT(style < layout->styles + layout->styleCount);

		for (uint32_t j = 0; j < range->glyphCount; ++j)
		{
			uint32_t index = range->firstGlyph + j;

			// Advance the style.
			uint32_t charIndex = text->glyphs[index].charIndex;
			DS_ASSERT(charIndex < text->characterCount);

			while (charIndex >= styleRangeLimit)
			{
				++style;
				DS_ASSERT(style < layout->styles + layout->styleCount);
				DS_ASSERT(style->start == styleRangeLimit);
				styleRangeLimit += style->count;
			}
			DS_ASSERT(style < layout->styles + layout->styleCount);

			// Skip whitespace.
			uint32_t charcode = text->characters[text->glyphs[index].charIndex];
			if (dsIsSpace(charcode))
			{
				glyphs[index].geometry.min.x = glyphs[index].geometry.min.y = 0;
				glyphs[index].geometry.max = glyphs[index].geometry.min;
				glyphs[index].texCoords = glyphs[index].geometry;
				glyphs[index].mipLevel = 0;
				glyphs[index].styleIndex = (uint32_t)(style - layout->styles);
				continue;
			}

			float scale = style->scale/(float)font->glyphSize;
			dsGlyphInfo* glyphInfo = dsFont_getGlyphInfo(font, commandBuffer, range->face,
				text->glyphs[index].glyphId);

			dsVector2_scale(glyphs[index].geometry.min, glyphInfo->glyphBounds.min, scale);
			dsVector2_scale(glyphs[index].geometry.max, glyphInfo->glyphBounds.max, scale);

			// Add the offset to the base glyph position.
			glyphs[index].geometry.min.y += style->verticalOffset;
			glyphs[index].geometry.max.y += style->verticalOffset;

			dsTexturePosition texturePos;
			dsFont_getGlyphTexturePos(&texturePos, dsFont_getGlyphIndex(font, glyphInfo),
				font->glyphSize, font->texMultiplier);
			glyphs[index].mipLevel = texturePos.mipLevel;
			// Store the base information in texCoords for now so we can use it for later
			// calculations.
			glyphs[index].texCoords.min.x = (float)texturePos.x;
			glyphs[index].texCoords.min.y = (float)texturePos.y;
			dsAlignedBox2_extents(glyphs[index].texCoords.max, glyphInfo->glyphBounds);
			glyphs[index].styleIndex = (uint32_t)(style - layout->styles);
		}
	}
	DS_PROFILE_SCOPE_END();

	// Second pass: resolve line breaks from ranges, which were detected when finding BiDi runs.
	uint32_t line = 0;
	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		const dsTextRange* range = text->ranges + i;
		for (uint32_t j = 0; j < range->glyphCount; ++j)
			glyphs[range->firstGlyph + j].position.y = (float)line;
		line += range->newlineCount;
	}

	// Third pass: find line breaks from newlines and wrapping. Do this based on the original
	// character order.
	dsVector2f position = {{0.0f, 0.0f}};
	const unsigned int windowSize = DS_BASE_WINDOW_SIZE*font->glyphSize/DS_LOW_SIZE;
	const float basePadding = (float)windowSize/(float)font->glyphSize;
	unsigned int wordCount = 0;
	bool lastIsWhitespace = false;
	float curWordOffset = 0.0f;
	uint32_t curWord = 0;
	uint32_t firstWhitespaceBeforeWord = 0;
	line = 0;
	for (uint32_t i = 0; i < text->characterCount; ++i)
	{
		const dsCharMapping* charMapping = text->charMappings + i;
		if (charMapping->glyphCount == 0)
			continue;

		bool isWhitespace = dsIsSpace(text->characters[i]);
		float scale = layout->styles[glyphs[charMapping->firstGlyph].styleIndex].scale;
		float glyphWidth = 0.0f;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			uint32_t index = charMapping->firstGlyph + j;

			// Handle line change from previous pass.
			if (glyphs[index].position.y != line)
			{
				// For right to left text, the characters will be reversed within a line, but
				// shouldn't pass a line boundary.
				DS_ASSERT(glyphs[index].position.y > line);
				position.x = 0.0f;
				position.y += glyphs[index].position.y - (float)line;
				wordCount = 0;
				curWord = i;
				curWordOffset = 0.0f;
				lastIsWhitespace = false;
				line = (uint32_t)glyphs[index].position.y;
			}

			position.x += text->glyphs[index].advance*scale;

			if (!isWhitespace)
			{
				const dsTextStyle* style = layout->styles + glyphs[index].styleIndex;
				float glyphImageWidth = glyphs[index].texCoords.max.x + (float)windowSize*2.0f;
				glyphImageWidth = dsMax(glyphImageWidth, font->glyphSize);
				float glyphScale = (float)font->glyphSize/glyphImageWidth;
				float boundsPadding = glyphScale*basePadding*style->embolden*scale;
				dsVector2f offset;
				dsVector2_scale(offset, text->glyphs[index].offset, scale);
				glyphWidth += offset.x + glyphs[index].geometry.max.x + boundsPadding;
				// Positive y points down, so need to subtract the slant from the width for a
				// positive effect.
				if (style->slant > 0)
					glyphWidth -= (offset.y + glyphs[index].geometry.min.y)*style->slant;
				else
					glyphWidth -= (offset.y + glyphs[index].geometry.max.y)*style->slant;
			}
		}

		// Detect word boundaries.
		if (lastIsWhitespace && !isWhitespace)
		{
			curWordOffset = position.x;
			curWord = i;
			++wordCount;
		}
		else if (!lastIsWhitespace && isWhitespace)
			firstWhitespaceBeforeWord = i;

		// Split on newline or on word boundaries that go over the limit.
		// Don't split on the first word in case the width is too small. (word count starts at 0
		// for the first word)
		if (text->characters[i] == '\n' ||
			(position.x + glyphWidth > maxWidth && !isWhitespace && wordCount > 0))
		{
			if (text->characters[i] == '\n')
			{
				firstWhitespaceBeforeWord = curWord = i;
				lastIsWhitespace = true;
				position.x = 0.0f;
			}
			else
				position.x -= curWordOffset;
			position.y += 1.0f;
			wordCount = 0;

			// Invalidate the positions for whitespace directly before the word, since it will cause
			// be drawn incorrectly for right to left text.
			for (uint32_t j = firstWhitespaceBeforeWord; j < curWord; ++j)
			{
				const dsCharMapping* otherMapping = text->charMappings + j;
				for (uint32_t k = 0; k < otherMapping->glyphCount; ++k)
				{
					glyphs[otherMapping->firstGlyph + k].position.x = FLT_MAX;
					glyphs[otherMapping->firstGlyph + k].position.y = FLT_MAX;
				}
			}

			// Offset the current word.
			for (uint32_t j = curWord; j < i; ++j)
			{
				const dsCharMapping* otherMapping = text->charMappings + j;
				for (uint32_t k = 0; k < otherMapping->glyphCount; ++k)
				{
					glyphs[otherMapping->firstGlyph + k].position.x -= curWordOffset;
					glyphs[otherMapping->firstGlyph + k].position.y += 1.0f;
				}
			}
		}
		lastIsWhitespace = isWhitespace;

		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
			glyphs[charMapping->firstGlyph + j].position = position;
	}

	// Allocate lines if there's an allocator.
	if (layout->allocator)
	{
		uint32_t lineCount = (uint32_t)position.y + 1;
		layout->lineCount = 0;
		if (!DS_RESIZEABLE_ARRAY_ADD(layout->allocator, layout->lines, layout->lineCount,
				layout->maxLines, lineCount))
		{
			DS_PROFILE_FUNC_RETURN(true);
		}
	}

	// Done with the lock at this point.
	dsFaceGroup_unlock(font->group);

	// Re-order any glyphs so they are in descending order. This could be an issue with line
	// wrapping for right to left text. Use bubble sort since it's stable (i.e. equal elements have
	// the same ordering) and should be fast given that it should be close to sorted.
	for (uint32_t i = 0; i < text->glyphCount; ++i)
		glyphsLineOrdered[i] = i;

	for (uint32_t i = 1; i < text->glyphCount; ++i)
	{
		for (uint32_t j = i; j > 0 &&
			glyphs[glyphsLineOrdered[j - 1]].position.y > glyphs[glyphsLineOrdered[j]].position.y;
			--j)
		{
			uint32_t temp = glyphsLineOrdered[j - 1];
			glyphsLineOrdered[j - 1] = glyphsLineOrdered[j];
			glyphsLineOrdered[j] = temp;
		}
	}

	// Fourth pass: find the offsets.
	dsAlignedBox2f lineBounds;
	dsAlignedBox2f_makeInvalid(&lineBounds);
	dsAlignedBox2f_makeInvalid(&layout->bounds);
	float lastY = 0.0f;
	float lastScale = 0.0f;
	float maxScale = 0.0f;
	float lineY = 0.0f;
	uint32_t sectionStart = 0;
	float offset = 0;
	uint32_t lineCount = 0;
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		// Skip glyphs on an invalid line.
		uint32_t textIndex = glyphsLineOrdered[i];
		dsGlyphLayout* glyph = glyphs + textIndex;
		if (glyph->position.y == FLT_MAX)
			continue;

		if (glyph->position.y != lastY)
		{
			if (maxScale != 0.0f)
				lastScale = maxScale;
			if (sectionStart != 0)
				lineY += lastScale*lineScale;
			float curYIndex = glyph->position.y;
			float lastYIndex = lastY;
			lastY = glyph->position.y;
			finishLine(layout, &lineBounds, lineY, alignment, glyphs, glyphsLineOrdered,
				sectionStart, i, &lineCount);

			lineY += lastScale*lineScale*(curYIndex - lastYIndex - 1.0f);
			sectionStart = i;
			offset = 0;
			maxScale = 0.0f;
		}

		float scale = layout->styles[glyph->styleIndex].scale;
		maxScale = dsMax(scale, maxScale);
		glyph->position.x = offset;
		DS_ASSERT(text->glyphs[textIndex].advance >= 0);
		offset += text->glyphs[textIndex].advance*scale;

		glyph->position.x += text->glyphs[textIndex].offset.x*scale;
		glyph->position.y += text->glyphs[textIndex].offset.y*scale;

		// Add to the line bounds.
		if (glyph->geometry.min.x != glyph->geometry.max.x &&
			glyph->geometry.min.y != glyph->geometry.max.y)
		{
			dsAlignedBox2f glyphBounds;
			dsVector2f relativePosition = {{glyph->position.x, 0.0f}};
			dsVector2_add(glyphBounds.min, relativePosition, glyph->geometry.min);
			dsVector2_add(glyphBounds.max, relativePosition, glyph->geometry.max);
			dsAlignedBox2_addBox(lineBounds, glyphBounds);
		}
		else
		{
			// This will catch leading whitespace for right to left text.
			if (text->characters[text->glyphs[textIndex].charIndex] != '\n')
			{
				dsVector2f point = {{offset, glyph->position.y}};
				dsAlignedBox2_addPoint(lineBounds, point);
			}
		}
	}

	// Last line.
	if (maxScale != 0.0f)
		lastScale = maxScale;
	if (sectionStart != 0)
		lineY += lastScale*lineScale;
	finishLine(layout, &lineBounds, lineY, alignment, glyphs, glyphsLineOrdered, sectionStart,
		text->glyphCount, &lineCount);
	if (layout->lines)
	{
		// Actual line count may be less due to empty lines.
		DS_ASSERT(lineCount <= layout->lineCount);
		layout->lineCount = lineCount;
	}

	// Fifth pass: add padding to the bounds and compute the final texture coordinates.
	uint32_t paddedGlyphSize = font->glyphSize + windowSize*2;
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		dsTexturePosition texturePos = {dsCubeFace_None, (uint32_t)glyphs[i].texCoords.min.x,
			(uint32_t)glyphs[i].texCoords.min.y, 0, glyphs[i].mipLevel};
		const dsVector2f* glyphSize = &glyphs[i].texCoords.max;

		// Don't add padding if the geometry is degenerate.
		if (glyphs[i].geometry.min.x < glyphs[i].geometry.max.x &&
			glyphs[i].geometry.min.y < glyphs[i].geometry.max.y)
		{
			float scale = layout->styles[glyphs[i].styleIndex].scale;
			float glyphWidth = glyphSize->x + (float)windowSize*2.0f;
			float glyphHeight = glyphSize->y + (float)windowSize*2.0f;
			glyphWidth = dsMax(glyphWidth, (float)paddedGlyphSize);
			glyphHeight = dsMax(glyphHeight, (float)paddedGlyphSize);
			dsVector2f glyphScale = {{(float)paddedGlyphSize/glyphWidth,
				(float)paddedGlyphSize/glyphHeight}};
			dsVector2f padding = {{basePadding, basePadding}};
			dsVector2_mul(padding, padding, glyphScale);
			dsVector2_scale(padding, padding, scale);

			dsVector2_sub(glyphs[i].geometry.min, glyphs[i].geometry.min, padding);
			dsVector2_add(glyphs[i].geometry.max, glyphs[i].geometry.max, padding);
		}

		dsFont_getGlyphTextureBounds(&glyphs[i].texCoords, &texturePos, glyphSize, font->glyphSize,
			font->texMultiplier);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsTextLayout_refresh(dsTextLayout* layout, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();
	if (!layout || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	const dsText* text = layout->text;
	if (text->characterCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsFont* font = text->font;
	dsGlyphLayout* glyphs = (dsGlyphLayout*)layout->glyphs;

	dsFaceGroup_lock(font->group);

	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		const dsTextRange* range = text->ranges + i;
		for (uint32_t j = 0; j < range->glyphCount; ++j)
		{
			uint32_t glyphIndex = range->firstGlyph + j;
			updateGlyph(font, commandBuffer, range->face, text->glyphs[glyphIndex].glyphId,
				glyphs + glyphIndex);
		}
	}

	dsFaceGroup_unlock(font->group);

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsTextLayout_refreshRange(dsTextLayout* layout, dsCommandBuffer* commandBuffer,
	uint32_t firstChar, uint32_t charCount)
{
	if (!layout || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	const dsText* text = layout->text;
	if (firstChar == 0 && charCount == text->characterCount)
		return dsTextLayout_refresh(layout, commandBuffer);

	DS_PROFILE_FUNC_START();

	if (!DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, text->characterCount))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (charCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsFont* font = text->font;
	dsGlyphLayout* glyphs = (dsGlyphLayout*)layout->glyphs;

	dsFaceGroup_lock(font->group);

	for (uint32_t i = 0; i < charCount; ++i)
	{
		const dsCharMapping* charMapping = text->charMappings + firstChar + i;
		const dsTextRange* range = findRange(text->ranges, text->rangeCount,
			charMapping->firstGlyph);
		DS_ASSERT(range);
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			uint32_t glyphIndex = charMapping->firstGlyph + j;
			updateGlyph(font, commandBuffer, range->face, text->glyphs[glyphIndex].glyphId,
				glyphs + glyphIndex);
		}
	}

	dsFaceGroup_unlock(font->group);

	DS_PROFILE_FUNC_RETURN(true);
}

void dsTextLayout_destroy(dsTextLayout* layout)
{
	if (!layout || !layout->allocator)
		return;

	dsAllocator_free(layout->allocator, layout->lines);
	dsAllocator_free(layout->allocator, layout);
}

void dsTextLayout_destroyLayoutAndText(dsTextLayout* layout)
{
	if (!layout)
		return;

	dsText_destroy((dsText*)layout->text);
	if (layout->allocator)
	{
		dsAllocator_free(layout->allocator, layout->lines);
		dsAllocator_free(layout->allocator, layout);
	}
}
