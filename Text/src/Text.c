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

typedef struct dsScriptInfo
{
	uint32_t firstCodepoint;
	uint32_t start;
	uint32_t count;
} dsScriptInfo;

static void getTextLengths(uint32_t* outLength, uint32_t* outRangeCount, const dsFaceGroup* group,
	const void* string, dsNextCodepointFunction nextCodepoint)
{
	*outLength = 0;
	*outRangeCount = 0;
	if (!string)
		return;

	uint32_t lastScript = 0;
	uint32_t index = 0;
	bool hasLastScript = false;
	do
	{
		uint32_t codepoint = nextCodepoint(string, &index);
		if (codepoint == DS_UNICODE_INVALID)
		{
			*outLength = DS_UNICODE_INVALID;
		}
		else if (codepoint == DS_UNICODE_END)
			break;

		++*outLength;
		uint32_t script = dsFaceGroup_codepointScript(group, codepoint);
		if (!dsFaceGroup_isScriptUnique(script) || (hasLastScript && lastScript == script))
			continue;

		++*outRangeCount;
		lastScript = script;
		hasLastScript = true;
	} while (true);

	if (*outRangeCount == 0)
		*outRangeCount = 1;
}

static dsScriptInfo* getScriptInfo(dsText* text, uint32_t i)
{
	DS_STATIC_ASSERT(sizeof(dsScriptInfo) <= sizeof(dsTextRange), invalid_script_info_size);
	return (dsScriptInfo*)(text->ranges + i);
}

static bool shapeText(dsText* text)
{
	/*
	 * Compute the boundaries between unique scripts. This ignores characters shared across
	 * different "common" scripts (such as @) and inhereted scripts (such as '"'). These unique
	 * script boundaries are used for two purposes:
	 * 1. Determine which face to draw wih, since not all faces contain all scripts.
	 * 2. Different text directions. (left to right vs. right to left)
	 *
	 * NOTE: It's expected that this will be sufficient for our use cases for bidirectional text.
	 * The use cases for this library aren't intended to be super complex, such as for a UI or a
	 * game, not general purpose document or web page rendering. If this algorithm breaks down in
	 * real-world situations, some other alternatives will need to be considered.
	 * - Enhance the algorithm. For example, this might need to have additional logic along
	 *   whitespace boundaries to have a better concept of words.
	 * - Integrate a more correct algorithm from an external library
	 *   (e.g. https://github.com/mta452/SheenBidi) and combine it with the script boundaries for
	 *   choosing the face.
	 */

	if (text->characterCount == 0)
		return true;

	const dsFaceGroup* group = dsFont_getFaceGroup(text->font);

	// Re-use glyph memory to store scripts. It's guaranteed to be large enough and will be
	// populated later.
	uint32_t* scripts = (uint32_t*)text->glyphs;
	for (uint32_t i = 0; i < text->characterCount; ++i)
		scripts[i] = dsFaceGroup_codepointScript(group, text->characters[i]);

	// Store the script info in the text ranges memory. This will be converted in place to the final text
	// ranges later.
	uint32_t lastScript = 0;
	bool hasLastScript = false;
	dsScriptInfo* scriptInfo = getScriptInfo(text, 0);
	scriptInfo->firstCodepoint = text->characters[0];
	scriptInfo->start = 0;
	dsScriptInfo* prevScriptInfo = NULL;
	uint32_t curInfo = 0;
	for (uint32_t i = 0; i < text->characterCount; ++i)
	{
		if (!dsFaceGroup_isScriptUnique(scripts[i]) || (hasLastScript && lastScript == scripts[i]))
			continue;

		DS_ASSERT(curInfo < text->rangeCount);
		if (prevScriptInfo)
			prevScriptInfo->count = i - prevScriptInfo->start;
		scriptInfo->firstCodepoint = text->characters[i];
		scriptInfo->start = i;

		prevScriptInfo = scriptInfo;
		scriptInfo = getScriptInfo(text, ++curInfo);

		lastScript = scripts[i];
		hasLastScript = true;
	}

	if (curInfo == 0)
		curInfo = 1;
	DS_ASSERT(curInfo == text->rangeCount);
	scriptInfo = getScriptInfo(text, curInfo - 1);
	scriptInfo->count = text->characterCount - scriptInfo->start;

	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		scriptInfo = getScriptInfo(text, i);
		if (!dsFont_shapeRange(text->font, text, i, scriptInfo->firstCodepoint, scriptInfo->start,
			scriptInfo->count))
		{
			return false;
		}
	}

	return true;
}

static dsText* createTextImpl(dsFont* font, dsAllocator* allocator, const void* string,
	dsNextCodepointFunction nextCodepoint)
{
	DS_PROFILE_FUNC_START();
	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Allocator for text must support freeing memory.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t length, rangeCount;
	getTextLengths(&length, &rangeCount, font->group, string, nextCodepoint);
	if (length == DS_UNICODE_INVALID)
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Invalid Unicode string.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	// Do the initial work in a temporary dsText instance. This allows us to re-use temporary memory
	// and use a single allocation for the final object. We already need to mutex lock for shaping,
	// so it doesn't require additional locks.
	dsFaceGroup_lock(font->group);

	dsText* scratchText = dsFaceGroup_scratchText(font->group, length, rangeCount);
	if (!scratchText)
	{
		dsFaceGroup_unlock(font->group);
		DS_PROFILE_FUNC_RETURN(NULL);
	}
	scratchText->font = font;

	uint32_t index = 0;
	for (uint32_t i = 0; i < length; ++i)
		((uint32_t*)scratchText->characters)[i] = nextCodepoint(string, &index);
	bool shapeSucceeded = shapeText(scratchText);

	dsFaceGroup_unlock(font->group);
	if (!shapeSucceeded)
		DS_PROFILE_FUNC_RETURN(NULL);

	// Now copy the contents into the final text object.
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsText)) + DS_ALIGNED_SIZE(length*sizeof(uint32_t)) +
		DS_ALIGNED_SIZE(sizeof(dsGlyph)*scratchText->glyphCount) +
		DS_ALIGNED_SIZE(rangeCount*sizeof(dsTextRange));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsText* text = (dsText*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, sizeof(dsText));
	DS_ASSERT(text);
	text->font = font;
	text->allocator = dsAllocator_keepPointer(allocator);
	if (length > 0)
	{
		text->characters = (uint32_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			length*sizeof(uint32_t));
		DS_ASSERT(text->characters);
		text->characterCount = length;
		memcpy((void*)text->characters, scratchText->characters, length*sizeof(uint32_t));

		text->glyphs = (dsGlyph*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(dsGlyph)*scratchText->glyphCount);
		DS_ASSERT(text->glyphs);
		text->glyphCount = scratchText->glyphCount;
		memcpy((void*)text->glyphs, scratchText->glyphs, scratchText->glyphCount*sizeof(dsGlyph));

		text->ranges = (dsTextRange*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			rangeCount*sizeof(dsTextRange));
		DS_ASSERT(text->ranges);
		text->rangeCount = rangeCount;
		memcpy((void*)text->ranges, scratchText->ranges, rangeCount*sizeof(dsTextRange));
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

dsText* dsText_createUTF8(dsFont* font, dsAllocator* allocator, const char* string)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF8_nextCodepoint);
}

dsText* dsText_createUTF16(dsFont* font, dsAllocator* allocator, const uint16_t* string)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF16_nextCodepoint);
}

dsText* dsText_createUTF32(dsFont* font, dsAllocator* allocator, const uint32_t* string)
{
	if (!font || (!allocator && !dsFont_getAllocator(font)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = dsFont_getAllocator(font);
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF32_nextCodepoint);
}
