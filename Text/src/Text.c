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

#include <DeepSea/Text/Text.h>

#include "FontImpl.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Unicode.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <string.h>

typedef uint32_t (*dsNextCodepointFunction)(const void* string, uint32_t* index);
typedef uint32_t (*dsCodepointCountFunction)(const void* string);

typedef struct dsScriptInfo
{
	uint32_t firstCodepoint;
	uint32_t start;
	uint32_t count;
} dsScriptInfo;

// Will also convert runs to codepoint indices from characters.
static uint32_t countTextRanges(const dsFaceGroup* group, const uint32_t* codepoints,
	uint32_t length, dsRunInfo* runInfos, uint32_t runCount)
{
	DS_UNUSED(length);
	uint32_t rangeCount = runCount;
	for (uint32_t i = 0; i < runCount; ++i)
	{
		// Also find the direction.
		runInfos[i].direction = dsTextDirection_Either;
		for (uint32_t j = 0;
			j < runInfos[i].count && runInfos[i].direction == dsTextDirection_Either; ++j)
		{
			uint32_t index = runInfos[i].start + j;
			DS_ASSERT(index < length);
			runInfos[i].direction = dsFaceGroup_textDirection(dsFaceGroup_codepointScript(group,
				codepoints[index]));
		}

		uint32_t lastScript = 0;
		bool hasLastScript = false;
		for (uint32_t j = 0; j < runInfos[i].count; ++j)
		{
			uint32_t index = runInfos[i].start + j;
			DS_ASSERT(index < length);
			uint32_t script = dsFaceGroup_codepointScript(group, codepoints[index]);
			if (!dsFaceGroup_isScriptUnique(script) ||
				(hasLastScript && dsFaceGroup_areScriptsEqual(lastScript, script)))
			{
				continue;
			}

			if (hasLastScript)
				++rangeCount;
			lastScript = script;
			hasLastScript = true;
		}
	}

	return rangeCount;
}

static dsScriptInfo* getScriptInfo(dsText* text, uint32_t i)
{
	DS_STATIC_ASSERT(sizeof(dsScriptInfo) <= sizeof(dsTextRange), invalid_script_info_size);
	return (dsScriptInfo*)(text->ranges + i);
}

static bool shapeText(dsText* text, const dsRunInfo* runs, uint32_t runCount, bool uniformScript)
{
	if (text->characterCount == 0)
		return true;

	const dsFaceGroup* group = dsFont_getFaceGroup(text->font);

	// Store the script info in the text ranges memory. This will be converted in place to the final
	// text ranges later.
	dsScriptInfo* scriptInfo = NULL;
	uint32_t curInfo = 0;
	for (uint32_t i = 0; i < runCount; ++i)
	{
		uint32_t startInfo = curInfo;
		scriptInfo = getScriptInfo(text, curInfo++);
		// First codepoint may be set again later.
		scriptInfo->firstCodepoint = text->characters[runs[i].start];
		scriptInfo->start = runs[i].start;
		scriptInfo->count = runs[i].count;

		// Loop backward for right to left text.
		uint32_t start, end;
		int preIncr, postIncr;
		uint32_t firstNewlineCount, lastNewlineCount;
		if (runs[i].direction == dsTextDirection_RightToLeft)
		{
			start = runs[i].count;
			end = 0;
			preIncr = -1;
			postIncr = 0;
			firstNewlineCount = runs[i].newlineCount;
			lastNewlineCount = 0;
		}
		else
		{
			start = 0;
			end = runs[i].count;
			preIncr = 0;
			postIncr = 1;
			firstNewlineCount = 0;
			lastNewlineCount = runs[i].newlineCount;
		}

		uint32_t lastScript = 0;
		bool hasLastScript = false;
		for (uint32_t j = start; j != end; j += postIncr)
		{
			j += preIncr;
			uint32_t index = runs[i].start + j;
			uint32_t script = dsFaceGroup_codepointScript(group, text->characters[index]);
			if (!dsFaceGroup_isScriptUnique(script) ||
				(hasLastScript && dsFaceGroup_areScriptsEqual(lastScript, script)))
			{
				continue;
			}

			if (hasLastScript)
			{
				// Shape the current range and move onto the next one.
				if (runs[i].direction == dsTextDirection_RightToLeft)
				{
					DS_ASSERT(index + 1 < scriptInfo->start + scriptInfo->count);
					scriptInfo->count = scriptInfo->start + scriptInfo->count - (index + 1);
					scriptInfo->start = index + 1;
				}
				else
					scriptInfo->count = index - scriptInfo->start;
				DS_ASSERT(curInfo < text->rangeCount);
				uint32_t newlineCount = curInfo - 1 == startInfo ? firstNewlineCount : 0;
				if (!dsFont_shapeRange(text->font, text, curInfo - 1, scriptInfo->firstCodepoint,
					scriptInfo->start, scriptInfo->count, newlineCount, runs[i].direction))
				{
					return false;
				}

				scriptInfo = getScriptInfo(text, curInfo++);
				scriptInfo->firstCodepoint = text->characters[index];
				if (runs[i].direction == dsTextDirection_RightToLeft)
				{
					scriptInfo->start = runs[i].start;
					scriptInfo->count = index + 1 - runs[i].start;
				}
				else
				{
					scriptInfo->start = index;
					scriptInfo->count = runs[i].start + runs[i].count - index;
				}
			}
			else
			{
				// The first info for a run will wait until the first unique script to grab the
				// first codepoint.
				scriptInfo->firstCodepoint = text->characters[index];
				hasLastScript = true;

				// When we know the script will be uniform, just need to find the first unique
				// script codepoint.
				if (uniformScript)
					break;
			}

			lastScript = script;
		}

		// Shape the last range.
		uint32_t newlineCount = lastNewlineCount;
		if (curInfo - 1 == startInfo && firstNewlineCount)
			newlineCount = firstNewlineCount;
		if (!dsFont_shapeRange(text->font, text, curInfo - 1, scriptInfo->firstCodepoint,
			scriptInfo->start, scriptInfo->count, newlineCount, runs[i].direction))
		{
			return false;
		}
	}
	DS_ASSERT(curInfo == text->rangeCount);
	return true;
}

static dsText* createTextImpl(dsFont* font, dsAllocator* allocator, const void* string,
	dsNextCodepointFunction nextCodepoint, dsCodepointCountFunction codepointCount,
	dsUnicodeType type, bool uniformScript)
{
	DS_PROFILE_FUNC_START();
	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Allocator for text must support freeing memory.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	// Do the initial work in a temporary dsText instance. This allows us to re-use temporary memory
	// and use a single allocation for the final object. We already need to mutex lock for shaping,
	// so it doesn't require additional locks.
	dsFaceGroup_lock(font->group);

	uint32_t length = codepointCount(string);
	if (length == DS_UNICODE_INVALID)
	{
		dsFaceGroup_unlock(font->group);
		errno = EFORMAT;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Invalid Unicode string.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t runCount;
	dsRunInfo* runs;
	dsRunInfo dummyRun;
	if (uniformScript)
	{
		// When using uniform script, has a single run and range that spans the full string.
		runCount = length == 0 ? 0 : 1;
		dummyRun.start = 0;
		dummyRun.count = length;
		runs = &dummyRun;
	}
	else
	{
		runs = dsFaceGroup_findBidiRuns(&runCount, font->group, string, type);
		if (runCount == DS_UNICODE_INVALID)
		{
			dsFaceGroup_unlock(font->group);
			if (errno != ENOMEM)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Invalid Unicode string.");
			}
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	dsText* scratchText = dsFaceGroup_scratchText(font->group, length);
	if (!scratchText)
	{
		dsFaceGroup_unlock(font->group);
		DS_PROFILE_FUNC_RETURN(NULL);
	}
	scratchText->font = font;

	uint32_t index = 0;
	for (uint32_t i = 0; i < length; ++i)
		((uint32_t*)scratchText->characters)[i] = nextCodepoint(string, &index);

	uint32_t rangeCount;
	if (uniformScript)
		rangeCount = runCount;
	else
	{
		rangeCount = countTextRanges(font->group, scratchText->characters,
			scratchText->characterCount, runs, runCount);
	}
	if (!dsFaceGroup_scratchRanges(font->group, rangeCount))
	{
		dsFaceGroup_unlock(font->group);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	bool shapeSucceeded = shapeText(scratchText, runs, runCount, uniformScript);
	dsFaceGroup_unlock(font->group);
	if (!shapeSucceeded)
		DS_PROFILE_FUNC_RETURN(NULL);

	// Now copy the contents into the final text object.
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsText)) + DS_ALIGNED_SIZE(length*sizeof(uint32_t)) +
		DS_ALIGNED_SIZE(scratchText->glyphCount*sizeof(dsGlyph)) +
		DS_ALIGNED_SIZE(rangeCount*sizeof(dsTextRange));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsText* text = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsText);
	DS_ASSERT(text);
	text->font = font;
	text->allocator = dsAllocator_keepPointer(allocator);
	if (scratchText->glyphCount > 0)
	{
		text->characters = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, uint32_t, length);
		DS_ASSERT(text->characters);
		text->characterCount = length;
		memcpy((void*)text->characters, scratchText->characters, length*sizeof(uint32_t));

		// Ranges may not be in monotomic increasing order due to right to left text. Re-order the
		// glyphs so it is.
		text->glyphs = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsGlyph,
			scratchText->glyphCount);
		DS_ASSERT(text->glyphs);
		text->glyphCount = scratchText->glyphCount;

		text->ranges = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsTextRange,
			rangeCount);
		DS_ASSERT(text->ranges);
		text->rangeCount = rangeCount;

		uint32_t curGlyphStart = 0;
		for (uint32_t i = 0; i < rangeCount; ++i)
		{
			dsTextRange* range = (dsTextRange*)text->ranges + i;
			*range = scratchText->ranges[i];
			memcpy((void*)(text->glyphs + curGlyphStart), scratchText->glyphs + range->firstGlyph,
				range->glyphCount*sizeof(dsGlyph));
			range->firstGlyph = curGlyphStart;
			curGlyphStart += text->ranges[i].glyphCount;
		}
		DS_ASSERT(curGlyphStart == text->glyphCount);
	}
	else
	{
		text->characters = NULL;
		text->glyphs = NULL;
		text->ranges = NULL;
		text->characterCount = 0;
		text->glyphCount = 0;
		text->rangeCount = 0;
	}

	DS_PROFILE_FUNC_RETURN(text);
}

dsText* dsText_create(dsFont* font, dsAllocator* allocator, const void* string, dsUnicodeType type,
	bool uniformScript)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);

	dsNextCodepointFunction nextCodepoint;
	dsCodepointCountFunction codepointCount;
	switch (type)
	{
		case dsUnicodeType_UTF8:
			nextCodepoint = (dsNextCodepointFunction)&dsUTF8_nextCodepoint;
			codepointCount = (dsCodepointCountFunction)&dsUTF8_codepointCount;
			break;
		case dsUnicodeType_UTF16:
			nextCodepoint = (dsNextCodepointFunction)&dsUTF16_nextCodepoint;
			codepointCount = (dsCodepointCountFunction)&dsUTF16_codepointCount;
			break;
		case dsUnicodeType_UTF32:
			nextCodepoint = (dsNextCodepointFunction)&dsUTF32_nextCodepoint;
			codepointCount = (dsCodepointCountFunction)&dsUTF32_codepointCount;
			break;
		default:
			errno = EINVAL;
			return NULL;
	}
	return createTextImpl(font, allocator, string, nextCodepoint, codepointCount, type,
		uniformScript);
}

dsText* dsText_createUTF8(dsFont* font, dsAllocator* allocator, const char* string,
	bool uniformScript)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF8_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF8_codepointCount, dsUnicodeType_UTF8, uniformScript);
}

dsText* dsText_createUTF16(dsFont* font, dsAllocator* allocator, const uint16_t* string,
	bool uniformScript)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF16_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF16_codepointCount, dsUnicodeType_UTF16, uniformScript);
}

dsText* dsText_createUTF32(dsFont* font, dsAllocator* allocator, const uint32_t* string,
	bool uniformScript)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF32_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF32_codepointCount, dsUnicodeType_UTF32, uniformScript);
}

void dsText_destroy(dsText* text)
{
	if (!text || !text->allocator)
		return;

	dsAllocator_free(text->allocator, text);
}
