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

static void createCharMappings(dsCharMapping* charMappings, uint32_t length, const dsGlyph* glyphs,
	uint32_t glyphCount)
{
	memset(charMappings, 0, length*sizeof(dsCharMapping));
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		uint32_t charIndex = glyphs[i].charIndex;
		DS_ASSERT(charIndex < length);

		// Assume that the glyphs associated with a character are in sequence.
		if (charMappings[charIndex].glyphCount == 0)
		{
			charMappings[charIndex].firstGlyph = i;
			charMappings[charIndex].glyphCount = 1;
		}
		else
		{
			DS_ASSERT(charMappings[charIndex].firstGlyph + charMappings[charIndex].glyphCount == i);
			++charMappings[charIndex].glyphCount;
		}
	}
}

static dsScriptInfo* getScriptInfo(dsText* text, uint32_t i)
{
	DS_STATIC_ASSERT(sizeof(dsScriptInfo) <= sizeof(dsTextRange), invalid_script_info_size);
	return (dsScriptInfo*)(text->ranges + i);
}

static bool shapeText(dsText* text, const dsRunInfo* runs, uint32_t runCount, bool uniformScript)
{
	DS_PROFILE_FUNC_START();

	if (text->characterCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	const dsFaceGroup* group = dsFont_getFaceGroup(text->font);

	// Store the script info in the text ranges memory. This will be converted in place to the final
	// text ranges later.
	dsScriptInfo* scriptInfo = NULL;
	uint32_t curInfo = 0;
	for (uint32_t i = 0; i < runCount; ++i)
	{
		uint32_t startInfo = curInfo;
		scriptInfo = getScriptInfo(text, curInfo++);
		// Initialize for the entire run in case there's no script boundaries. First codepoint may
		// be set again later.
		scriptInfo->firstCodepoint = text->characters[runs[i].start];
		scriptInfo->start = runs[i].start;
		scriptInfo->count = runs[i].count;

		uint32_t lastScript = 0;
		bool hasLastScript = false;
		for (uint32_t j = 0; j < runs[i].count; ++j)
		{
			uint32_t index = runs[i].start + j;
			uint32_t script = dsFaceGroup_codepointScript(group, text->characters[index]);
			if (!dsFaceGroup_isScriptUnique(script) ||
				(hasLastScript && dsFaceGroup_areScriptsEqual(lastScript, script)))
			{
				continue;
			}

			if (hasLastScript)
			{
				// End the current range and move onto the next one.
				scriptInfo->count = index - scriptInfo->start;

				// Prepare the next range. Initialize the start and count to match the rest of the
				// text (end for left to right, or start for right to left) in case we don't
				// encounter another script boundary.
				scriptInfo = getScriptInfo(text, curInfo++);
				scriptInfo->firstCodepoint = text->characters[index];
				scriptInfo->start = index;
				scriptInfo->count = runs[i].start + runs[i].count - index;
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
				{
					// Keep searching if common script.
					if (dsFaceGroup_isScriptCommon(script))
						hasLastScript = false;
					else
						break;
				}
			}

			lastScript = script;
		}

		// Reverse the ranges for right to left text.
		if (runs[i].direction == dsTextDirection_RightToLeft)
		{
			uint32_t count = curInfo - startInfo;
			for (uint32_t j = 0; j < count/2; ++j)
			{
				dsScriptInfo* first = getScriptInfo(text, startInfo + j);
				dsScriptInfo* second = getScriptInfo(text, startInfo + count - j - 1);
				dsScriptInfo temp = *first;
				*first = *second;
				*second = temp;
			}
		}

		// Shape the ranges. Go backwards for right to left text.
		for (uint32_t j = startInfo; j < curInfo; ++j)
		{
			uint32_t infoIndex = j;
			uint32_t newlineCount = j == curInfo - 1 ? runs[i].newlineCount : 0;
			scriptInfo = getScriptInfo(text, infoIndex);
			if (!dsFont_shapeRange(text->font, text, infoIndex, scriptInfo->firstCodepoint,
				scriptInfo->start, scriptInfo->count, newlineCount, runs[i].direction))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
	}
	DS_ASSERT(curInfo == text->rangeCount);
	DS_PROFILE_FUNC_RETURN(true);
}

static dsText* createTextImpl(dsFont* font, dsAllocator* allocator, const void* string,
	dsNextCodepointFunction nextCodepoint, dsCodepointCountFunction codepointCount,
	dsUnicodeType type, bool uniformScript)
{
	DS_PROFILE_FUNC_START();
	if (!allocator->freeFunc)
	{
		errno = EINVAL;
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

	uint32_t* characters = (uint32_t*)scratchText->characters;
	uint32_t index = 0;
	for (uint32_t i = 0; i < length; ++i)
		characters[i] = nextCodepoint(string, &index);

	uint32_t rangeCount;
	if (uniformScript)
	{
		rangeCount = runCount;

		// Find the direction based on the first unique script.
		runs[0].direction = dsTextDirection_LeftToRight;
		for (uint32_t i = 0; i < length; ++i)
		{
			uint32_t script = dsFaceGroup_codepointScript(font->group, characters[i]);
			if (dsFaceGroup_isScriptUnique(script))
			{
				dsTextDirection direction = dsFaceGroup_textDirection(script);
				if (direction != dsTextDirection_Either)
				{
					runs[0].direction = direction;
					break;
				}
			}
		}
	}
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
		DS_ALIGNED_SIZE(length*sizeof(dsCharMapping)) +
		DS_ALIGNED_SIZE(scratchText->glyphCount*sizeof(dsGlyph)) +
		DS_ALIGNED_SIZE(rangeCount*sizeof(dsTextRange));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsText* text = DS_ALLOCATE_OBJECT(&bufferAlloc, dsText);
	DS_ASSERT(text);
	text->font = font;
	text->allocator = dsAllocator_keepPointer(allocator);
	if (scratchText->glyphCount > 0)
	{
		text->characters = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, length);
		DS_ASSERT(text->characters);
		text->characterCount = length;
		memcpy((void*)text->characters, scratchText->characters, length*sizeof(uint32_t));

		text->charMappings = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsCharMapping, length);
		DS_ASSERT(text->charMappings);

		// Ranges may not be in monotomic increasing order due to right to left text. Re-order the
		// glyphs so it is.
		text->glyphs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsGlyph, scratchText->glyphCount);
		DS_ASSERT(text->glyphs);
		text->glyphCount = scratchText->glyphCount;

		text->ranges = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsTextRange, rangeCount);
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

		createCharMappings((dsCharMapping*)text->charMappings, length, text->glyphs,
			text->glyphCount);
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
