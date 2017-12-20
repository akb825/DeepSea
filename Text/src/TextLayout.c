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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Text/Font.h>
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
	dsTextJustification justification, dsGlyphLayout* glyphs, uint32_t sectionStart,
	uint32_t sectionEnd)
{
	if (!dsAlignedBox2f_isValid(lineBounds))
		return;

	// Handle justification.
	switch (justification)
	{
		case dsTextJustification_Right:
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
			{
				glyphs[i].position.x -= lineBounds->max.x;
				glyphs[i].position.y = lineY;
			}
			lineBounds->min.x -= lineBounds->max.x;
			lineBounds->max.x -= 0.0f;
			break;
		case dsTextJustification_Center:
		{
			float justificationOffset = lineBounds->max.x/2.0f;
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
			{
				glyphs[i].position.x -= justificationOffset;
				glyphs[i].position.y = lineY;
			}
			lineBounds->min.x -= justificationOffset;
			lineBounds->max.x -= justificationOffset;
			break;
		}
		default:
			for (uint32_t i = sectionStart; i < sectionEnd; ++i)
				glyphs[i].position.y = lineY;
			break;
	}

	lineBounds->min.y += lineY;
	lineBounds->max.y += lineY;
	dsAlignedBox2_addBox(layout->bounds, *lineBounds);
	dsAlignedBox2f_makeInvalid(lineBounds);
}

size_t dsTextLayout_fullAllocSize(const dsText* text, uint32_t styleCount)
{
	if (!text || styleCount == 0)
		return 0;

	return DS_ALIGNED_SIZE(sizeof(dsTextLayout)) +
		DS_ALIGNED_SIZE(text->glyphCount*sizeof(dsGlyphLayout)) +
		DS_ALIGNED_SIZE(styleCount*sizeof(dsTextStyle));
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
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Text style ranges must be monotomically increasing, "
			"non-overlapping, and cover the full range of text.");
		errno = EPERM;
		return NULL;
	}

	size_t fullSize = dsTextLayout_fullAllocSize(text, styleCount);
	DS_ASSERT(fullSize > 0);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsTextLayout* layout = (dsTextLayout*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsTextLayout));
	layout->allocator = dsAllocator_keepPointer(allocator);
	layout->text = text;
	if (text->glyphCount > 0)
	{
		layout->glyphs = (dsGlyphLayout*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			text->glyphCount*sizeof(dsGlyphLayout));
		DS_ASSERT(layout->glyphs);
	}
	else
		layout->glyphs = NULL;

	layout->styles = (dsTextStyle*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		styleCount*sizeof(dsTextStyle));
	DS_ASSERT(layout->styles);
	memcpy(layout->styles, styles, styleCount*sizeof(dsTextStyle));
	layout->styleCount = styleCount;

	dsAlignedBox2f_makeInvalid(&layout->bounds);

	return layout;
}

bool dsTextLayout_layout(dsTextLayout* layout, dsCommandBuffer* commandBuffer,
	dsTextJustification justification, float maxWidth, float lineScale)
{
	DS_PROFILE_FUNC_START();
	if (!layout || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	const dsText* text = layout->text;
	dsFont* font = text->font;
	dsGlyphLayout* glyphs = (dsGlyphLayout*)layout->glyphs;

	if (text->glyphCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	// First pass: get the initial cached glyph info and base positions.
	// This part accesses shared resources and needs to be locked.
	dsFaceGroup_lock(font->group);

	dsGlyphMapping* glyphMapping = dsFaceGroup_glyphMapping(font->group, text->characterCount);
	if (!glyphMapping)
	{
		dsFaceGroup_unlock(font->group);
		DS_PROFILE_FUNC_RETURN(false);
	}
	memset(glyphMapping, 0, text->characterCount*sizeof(dsGlyphMapping));

	DS_ASSERT(layout->styleCount > 0);
	DS_ASSERT(text->rangeCount > 0);
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

			// Store the glyph mapping. Multiple glyphs might be used with the same character.
			if (glyphMapping[charIndex].count == 0)
			{
				glyphMapping[charIndex].index = index;
				glyphMapping[charIndex].count = 1;
			}
			else
			{
				DS_ASSERT(glyphMapping[charIndex].index + glyphMapping[charIndex].count == index);
				++glyphMapping[charIndex].count;
			}

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
			if (isspace(charcode))
			{
				glyphs[index].geometry.min.x = glyphs[index].geometry.min.y = 0;
				glyphs[index].geometry.max = glyphs[index].geometry.min;
				glyphs[index].texCoords = glyphs[index].geometry;
				glyphs[index].mipLevel = 0;
				glyphs[index].textGlyphIndex = index;
				glyphs[index].styleIndex = (uint32_t)(style - layout->styles);
				continue;
			}

			float scale = style->scale;
			dsGlyphInfo* glyphInfo = dsFont_getGlyphInfo(font, commandBuffer, range->face,
				text->glyphs[index].glyphId);

			dsVector2_scale(glyphs[index].geometry.min, glyphInfo->glyphBounds.min, scale);
			dsVector2_scale(glyphs[index].geometry.max, glyphInfo->glyphBounds.max, scale);
			// Convert to Y pointing down.
			float temp = glyphs[index].geometry.min.y;
			glyphs[index].geometry.min.y = -glyphs[index].geometry.max.y;
			glyphs[index].geometry.max.y = -temp;

			dsTexturePosition texturePos;
			dsFont_getGlyphTexturePos(&texturePos, dsFont_getGlyphIndex(font, glyphInfo),
				font->glyphSize);
			glyphs[index].mipLevel = texturePos.mipLevel;
			// Store the base information in texCoords for now so we can use it for later
			// calculations.
			glyphs[index].texCoords.min.x = (float)texturePos.x;
			glyphs[index].texCoords.min.y = (float)texturePos.y;
			glyphs[index].texCoords.max.x = (float)glyphInfo->texSize.x;
			glyphs[index].texCoords.max.y = (float)glyphInfo->texSize.y;
			glyphs[index].textGlyphIndex = index;
			glyphs[index].styleIndex = (uint32_t)(style - layout->styles);
		}
	}

	// Second pass: find line breaks. Do this based on the original character order.
	dsVector2f position = {{0.0f, 0.0f}};
	const unsigned int windowSize = DS_BASE_WINDOW_SIZE*font->glyphSize/DS_LOW_SIZE;
	const float basePadding = (float)windowSize/(float)font->glyphSize;
	unsigned int wordCount = 0;
	bool lastIsWhitespace = false;
	uint32_t lastNonWhitespace = 0;
	for (uint32_t i = 0; i < text->characterCount; ++i)
	{
		if (glyphMapping[i].count == 0)
			continue;

		bool isWhitespace = isspace(text->characters[i]);
		float scale = layout->styles[glyphs[glyphMapping[i].index].styleIndex].scale;
		float glyphWidth = 0.0f;
		for (uint32_t j = 0; j < glyphMapping[i].count; ++j)
		{
			uint32_t index = glyphMapping[i].index + j;
			uint32_t textIndex = glyphs[index].textGlyphIndex;
			position.x += text->glyphs[textIndex].advance*scale;

			if (!isWhitespace)
			{
				const dsTextStyle* style = layout->styles + glyphs[index].styleIndex;
				uint32_t glyphImageWidth = (uint32_t)glyphs[index].texCoords.max.x + windowSize*2;
				glyphImageWidth = dsMax(glyphImageWidth, font->glyphSize);
				float glyphScale = (float)font->glyphSize/(float)glyphImageWidth;
				float boundsPadding = glyphScale*basePadding*style->embolden*scale;
				dsVector2f offset;
				dsVector2_scale(offset, text->glyphs[textIndex].offset, scale);
				glyphWidth += offset.x + glyphs[index].geometry.max.x + boundsPadding;
				// Positive y points down, so need to subtract the slant from the width for a
				// positive effect.
				if (style->slant > 0)
					glyphWidth -= (offset.y + glyphs[index].geometry.min.y)*style->slant;
				else
					glyphWidth -= (offset.y + glyphs[index].geometry.max.y)*style->slant;
			}
		}

		if (lastIsWhitespace && !isWhitespace)
			++wordCount;

		// Split on newline or on word boundaries that go over the limit.
		// Don't split on the first word in case the width is too small.
		bool hasNewline = false;
		if (text->characters[i] == '\n' ||
			(position.x + glyphWidth > maxWidth && lastIsWhitespace && wordCount > 1))
		{
			hasNewline = true;
			lastIsWhitespace = false;
			isWhitespace = false;
			position.y += 1.0f;
			position.x = 0.0f;
			wordCount = 0;
		}
		else
			lastIsWhitespace = isWhitespace;

		for (uint32_t j = 0; j < glyphMapping[i].count; ++j)
			glyphs[glyphMapping[i].index + j].position = position;

		// Add any trailing newlines at the end of the range.
		const dsTextRange* range = findRange(text->ranges, text->rangeCount, glyphMapping[i].index);
		DS_ASSERT(range);
		if (range->newlineCount > 0 &&
			((range->backward && glyphMapping[i].index == range->firstGlyph) ||
			(!range->backward && glyphMapping[i].index + glyphMapping[i].count ==
				range->firstGlyph + range->glyphCount)))
		{
			hasNewline = true;
			lastIsWhitespace = false;
			position.y += (float)range->newlineCount;
			position.x = 0.0f;
			wordCount = 0;
		}

		// If a newline was added, mark any trailing spaces as invalid.
		if (hasNewline)
		{
			for (uint32_t j = lastNonWhitespace + 1; j < i; ++j)
			{
				DS_ASSERT(glyphMapping[j].count == 1);
				glyphs[glyphMapping[j].index].position.y = FLT_MAX;
				glyphs[glyphMapping[j].index].position.y = FLT_MAX;
			}
		}

		if (!isWhitespace)
			lastNonWhitespace = i;
	}

	// Done with the lock at this point.
	dsFaceGroup_unlock(font->group);

	// Re-order any glyphs so they are in descending order. This could be an issue with line
	// wrapping for right to left text.
	for (uint32_t i = 1; i < text->glyphCount; ++i)
	{
		for (uint32_t j = i; j > 0 && glyphs[j - 1].position.y > glyphs[j].position.y; --j)
		{
			dsGlyphLayout temp = glyphs[j - 1];
			glyphs[j - 1] = glyphs[j];
			glyphs[j] = temp;
		}
	}

	// Third pass: find the offsets.
	dsAlignedBox2f lineBounds;
	dsAlignedBox2f_makeInvalid(&lineBounds);
	dsAlignedBox2f_makeInvalid(&layout->bounds);
	float lastY = 0.0f;
	float lastScale = 0.0f;
	float maxScale = 0.0f;
	float lineY = 0.0f;
	uint32_t sectionStart = 0;
	float offset = 0;
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		// Skkip glyphs on an invalid line.
		if (glyphs[i].position.y == FLT_MAX)
			continue;

		if (glyphs[i].position.y != lastY)
		{
			if (maxScale != 0.0f)
				lastScale = maxScale;
			if (sectionStart != 0)
				lineY += lastScale*lineScale;
			float curYIndex = glyphs[i].position.y;
			float lastYIndex = lastY;
			lastY = glyphs[i].position.y;
			finishLine(layout, &lineBounds, lineY, justification, glyphs, sectionStart, i);

			lineY += lastScale*lineScale*(curYIndex - lastYIndex - 1.0f);
			sectionStart = i;
			offset = 0;
			maxScale = 0.0f;
		}

		uint32_t textIndex = glyphs[i].textGlyphIndex;
		float scale = layout->styles[glyphs[i].styleIndex].scale;
		maxScale = dsMax(scale, maxScale);
		glyphs[i].position.x = offset;
		DS_ASSERT(text->glyphs[textIndex].advance >= 0);
		offset += text->glyphs[textIndex].advance*scale;

		glyphs[i].position.x += text->glyphs[textIndex].offset.x*scale;
		glyphs[i].position.y += text->glyphs[textIndex].offset.y*scale;

		// Add to the line bounds.
		if (glyphs[i].geometry.min.x != glyphs[i].geometry.max.x &&
			glyphs[i].geometry.min.y != glyphs[i].geometry.max.y)
		{
			dsAlignedBox2f glyphBounds;
			dsVector2f relativePosition = {{glyphs[i].position.x, 0.0f}};
			dsVector2_add(glyphBounds.min, relativePosition, glyphs[i].geometry.min);
			dsVector2_add(glyphBounds.max, relativePosition, glyphs[i].geometry.max);
			dsAlignedBox2_addBox(lineBounds, glyphBounds);
		}
	}

	// Last line.
	if (maxScale != 0.0f)
		lastScale = maxScale;
	if (sectionStart != 0)
		lineY += lastScale*lineScale;
	finishLine(layout, &lineBounds, lineY, justification, glyphs, sectionStart, text->glyphCount);

	// Fifth pass: add padding to the bounds and compute the final texture coordinates.
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		// Don't add padding if the geometry is degenerate.
		if (glyphs[i].geometry.min.x != glyphs[i].geometry.max.x &&
			glyphs[i].geometry.min.y != glyphs[i].geometry.max.y)
		{
			float scale = layout->styles[glyphs[i].styleIndex].scale;
			uint32_t glyphWidth = (uint32_t)glyphs[i].texCoords.max.x + windowSize*2;
			uint32_t glyphHeight = (uint32_t)glyphs[i].texCoords.max.y + windowSize*2;
			glyphWidth = dsMax(glyphWidth, font->glyphSize);
			glyphHeight = dsMax(glyphHeight, font->glyphSize);
			dsVector2f glyphScale = {{(float)font->glyphSize/(float)glyphWidth,
				(float)font->glyphSize/(float)glyphHeight}};
			dsVector2f padding = {{basePadding, basePadding}};
			dsVector2_mul(padding, padding, glyphScale);
			dsVector2_scale(padding, padding, scale);

			dsVector2_sub(glyphs[i].geometry.min, glyphs[i].geometry.min, padding);
			dsVector2_add(glyphs[i].geometry.max, glyphs[i].geometry.max, padding);
		}

		dsTexturePosition texturePos = {dsCubeFace_None, (uint32_t)glyphs[i].texCoords.min.x,
			(uint32_t)glyphs[i].texCoords.min.y, 0, glyphs[i].mipLevel};
		dsVector2i texSize = {{(int)glyphs[i].texCoords.max.x, (int)glyphs[i].texCoords.max.y}};
		dsFont_getGlyphTextureBounds(&glyphs[i].texCoords, &texturePos, &texSize, font->glyphSize);
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
	dsFont* font = text->font;
	dsGlyphLayout* glyphs = (dsGlyphLayout*)layout->glyphs;

	if (text->glyphCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsFaceGroup_lock(font->group);

	DS_ASSERT(text->rangeCount > 0);
	const dsTextRange* textRange = text->ranges;
	uint32_t textRangeLimit = textRange->glyphCount;
	for (uint32_t i = 0; i < layout->text->glyphCount; ++i)
	{
		// Advance the text range and style.
		while (i >= textRangeLimit)
		{
			++textRange;
			DS_ASSERT(textRange < text->ranges + text->rangeCount);
			DS_ASSERT(textRange->firstGlyph == textRangeLimit);
			textRangeLimit += textRange->glyphCount;
		}

		dsGlyphInfo* glyphInfo = dsFont_getGlyphInfo(font, commandBuffer, textRange->face,
			text->glyphs[glyphs[i].textGlyphIndex].glyphId);

		dsTexturePosition texturePos;
		dsFont_getGlyphTexturePos(&texturePos, dsFont_getGlyphIndex(font, glyphInfo),
			font->glyphSize);
		glyphs[i].mipLevel = texturePos.mipLevel;
		dsFont_getGlyphTextureBounds(&glyphs[i].texCoords, &texturePos, &glyphInfo->texSize,
			font->glyphSize);
	}

	dsFaceGroup_unlock(font->group);

	DS_PROFILE_FUNC_RETURN(true);
}

void dsTextLayout_destroy(dsTextLayout* layout)
{
	if (!layout || !layout->allocator)
		return;

	dsAllocator_free(layout->allocator, layout);
}
