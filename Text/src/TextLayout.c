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
#include <string.h>

size_t dsTextLayout_fullAllocSize(const dsText* text, uint32_t styleCount)
{
	if (!text || styleCount == 0)
		return 0;

	return DS_ALIGNED_SIZE(sizeof(dsTextLayout)) +
		DS_ALIGNED_SIZE(text->glyphCount*sizeof(dsGlyphLayout)) +
		DS_ALIGNED_SIZE(styleCount*sizeof(dsTextStyleRange));
}

dsTextLayout* dsTextLayout_create(dsAllocator* allocator, const dsText* text,
	const dsTextStyleRange* styles, uint32_t styleCount)
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

	layout->styles = (dsTextStyleRange*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		styleCount*sizeof(dsTextStyleRange));
	DS_ASSERT(layout->styles);
	memcpy(layout->styles, styles, styleCount*sizeof(dsTextStyleRange));
	layout->styleCount = styleCount;

	return layout;
}

bool dsTextLayout_layout(dsAlignedBox2f* outBounds, dsTextLayout* layout,
	dsCommandBuffer* commandBuffer, dsTextJustification justification, float maxWidth,
	float lineScale)
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

	DS_ASSERT(layout->styleCount > 0);
	DS_ASSERT(text->rangeCount > 0);
	const dsTextRange* textRange = text->ranges;
	const dsTextStyleRange* style = layout->styles;
	uint32_t textRangeLimit = textRange->glyphCount;
	uint32_t styleRangeLimit = style->count;
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

		while (i >= styleRangeLimit)
		{
			++style;
			DS_ASSERT(style < layout->styles + layout->styleCount);
			DS_ASSERT(style->start == styleRangeLimit);
			styleRangeLimit += style->count;
		}

		float scale = style->style.scale;
		dsGlyphInfo* glyphInfo = dsFont_getGlyphInfo(font, commandBuffer, textRange->face,
			text->glyphs[i].glyphId);

		dsGlyphLayout* curGlyph = (dsGlyphLayout*)(layout->glyphs + i);
		dsVector2_scale(glyphs[i].geometry.min, glyphInfo->glyphBounds.min, scale);
		dsVector2_scale(glyphs[i].geometry.max, glyphInfo->glyphBounds.max, scale);

		dsTexturePosition texturePos;
		dsFont_getGlyphTexturePos(&texturePos, dsFont_getGlyphIndex(font, glyphInfo),
			font->glyphSize);
		curGlyph->mipLevel = texturePos.mipLevel;
		dsFont_getGlyphTextureBounds(&curGlyph->texCoords, &texturePos, &glyphInfo->texSize,
			font->glyphSize);
		curGlyph->styleIndex = (uint32_t)(style - layout->styles);
	}

	dsFaceGroup_unlock(font->group);

	// Second pass: find line breaks
	dsVector2f position = {{0.0f, 0.0f}};
	uint32_t lastNonWhitespace = 0;
	bool lastIsWhitespace = false;
	unsigned int wordCount = 0;
	const float baseBoundsPadding = (float)DS_BASE_WINDOW_SIZE/(float)DS_LOW_SIZE;
	float maxScale = 0.0f;
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		float scale = layout->styles[glyphs[i].styleIndex].style.scale;
		maxScale = dsMax(scale, maxScale);
		position.x += fabsf(text->glyphs[i].advance)*scale;

		float glyphWidth = 0.0f;
		uint32_t charcode = text->characters[text->glyphs[i].charIndex];
		if (isspace(charcode))
			lastIsWhitespace = true;
		else
		{
			// Not 100% correct for an embolden value other than 0, but should be close.
			style = layout->styles + glyphs[i].styleIndex;
			float boundsPadding = baseBoundsPadding*style->style.scale*
				(1.0f - style->style.embolden);
			dsVector2f offset;
			dsVector2_scale(offset, text->glyphs[i].offset, scale);
			glyphWidth = offset.x + glyphs[i].geometry.max.x - boundsPadding;
			if (style->style.slant < 0)
				glyphWidth += (offset.y + glyphs[i].geometry.min.y)*style->style.slant;
			else
				glyphWidth += (offset.y + glyphs[i].geometry.max.y)*style->style.slant;

			if (lastIsWhitespace)
			{
				++wordCount;
				lastNonWhitespace = i;
				lastIsWhitespace = false;
			}
		}

		// Split on newline or on word boundaries that go over the limit.
		// Don't split on the first word in case the width is too small.
		if (charcode == '\n' ||
			(position.x + glyphWidth > maxWidth && lastIsWhitespace && wordCount > 1))
		{
			position.x = 0.0f;
			position.y += maxScale*lineScale;
			maxScale = 0.0f;
			wordCount = 0;
			if (charcode != '\n')
			{
				i = lastNonWhitespace;
				continue;
			}
		}

		// X will be re-computed later, but Y is still useful.
		glyphs->position = position;
	}

	// Third pass: find the offsets, resolving boundaries between forward and reverse text.
	dsAlignedBox2f lineBounds;
	dsAlignedBox2f_makeInvalid(&lineBounds);
	float lastY = 0.0f;
	uint32_t sectionStart = 0;
	uint32_t reverseSectionEnd = 0;
	float offset = 0;
	float maxPosition = 0;
	for (uint32_t i = 0; i < text->glyphCount; ++i)
	{
		if (glyphs[i].position.y != lastY)
		{
			if (dsAlignedBox2f_isValid(&lineBounds))
			{
				// Handle justification.
				switch (justification)
				{
					case dsTextJustification_Left:
						break;
					case dsTextJustification_Right:
						for (uint32_t j = sectionStart; j < i; ++j)
							glyphs[j].position.x -= lineBounds.max.x;
						break;
					case dsTextJustification_Center:
					{
						float justificationOffset = lineBounds.max.x/2.0f;
						for (uint32_t j = sectionStart; j < i; ++j)
							glyphs[j].position.x -= justificationOffset;
						break;
					}
				}

				if (outBounds)
				{
					// Negate Y
					float temp = lineBounds.min.y;
					lineBounds.min.y = -lineBounds.max.y;
					lineBounds.max.y = -temp;
					dsAlignedBox2_addBox(*outBounds, lineBounds);
				}

				dsAlignedBox2f_makeInvalid(&lineBounds);
			}

			lastY = glyphs[i].position.y;
			sectionStart = i;
			offset = 0;
			maxPosition = 0;
		}

		float scale = layout->styles[glyphs[i].styleIndex].style.scale;

		// Calculate the offset for reverse sections.
		if (i >= reverseSectionEnd && text->glyphs[i].advance < 0)
		{
			// Add size for the first glyph in the series.
			float sectionSize = text->glyphs[i].offset.x*scale + glyphs->geometry.max.x -
				baseBoundsPadding*scale;
			reverseSectionEnd = i;
			for (uint32_t j = i; j < text->glyphCount && glyphs[j].position.y == lastY &&
				text->glyphs[j].advance < 0; ++j, ++reverseSectionEnd)
			{
				float curScale = layout->styles[glyphs[j].styleIndex].style.scale;
				sectionSize += text->glyphs[i].advance*curScale;
			}

			maxPosition += sectionSize;
			offset += sectionSize;
		}
		else if (i == reverseSectionEnd)
			offset = maxPosition;

		float advance = text->glyphs[i].advance*scale;
		glyphs[i].position.x = offset;
		offset += advance;
		if (advance > 0)
			maxPosition += advance;
		else
			glyphs[i].position.x += advance;

		glyphs[i].position.x += text->glyphs[i].offset.x*scale;
		glyphs[i].position.y += text->glyphs[i].offset.y*scale;

		// Add to the line bounds.
		dsAlignedBox2f glyphBounds;
		dsVector2_add(glyphBounds.min, glyphs[i].position, glyphs[i].geometry.min);
		glyphBounds.min.x += baseBoundsPadding*scale;
		glyphBounds.min.y += baseBoundsPadding*scale;
		dsVector2_add(glyphBounds.max, glyphs[i].position, glyphs[i].geometry.max);
		glyphBounds.max.x -= baseBoundsPadding*scale;
		glyphBounds.max.y -= baseBoundsPadding*scale;
		dsAlignedBox2_addBox(lineBounds, glyphBounds);
	}

	// Last line.
	if (dsAlignedBox2f_isValid(&lineBounds))
	{
		// Handle justification.
		switch (justification)
		{
			case dsTextJustification_Left:
				break;
			case dsTextJustification_Right:
				for (uint32_t j = sectionStart; j < text->glyphCount; ++j)
					glyphs[j].position.x -= lineBounds.max.x;
				break;
			case dsTextJustification_Center:
			{
				float justificationOffset = lineBounds.max.x/2.0f;
				for (uint32_t j = sectionStart; j < text->glyphCount; ++j)
					glyphs[j].position.x -= justificationOffset;
				break;
			}
		}

		if (outBounds)
		{
			// Negate Y
			float temp = lineBounds.min.y;
			lineBounds.min.y = -lineBounds.max.y;
			lineBounds.max.y = -temp;
			dsAlignedBox2_addBox(*outBounds, lineBounds);
		}
	}

	DS_PROFILE_FUNC_RETURN(true);
}

void dsLayout_destroy(dsTextLayout* layout)
{
	if (!layout || !layout->allocator)
		return;

	dsAllocator_free(layout->allocator, layout);
}
