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

static size_t fullAllocSizeImpl(const dsFaceGroup* group, const void* string,
	dsNextCodepointFunction nextCodepoint, uint32_t* outScriptRangeCount)
{
	uint32_t length = 0;
	uint32_t scriptRangeCount = 0;
	uint32_t lastScript = 0;
	uint32_t index = 0;
	bool hasLastScript = false;
	do
	{
		uint32_t codepoint = nextCodepoint(string, &index);
		if (codepoint == DS_UNICODE_INVALID)
			return 0;
		else if (codepoint == DS_UNICODE_END)
			break;

		++length;
		uint32_t script = dsFaceGroup_codepointScript(group, codepoint);
		if (!dsFaceGroup_isScriptUnique(script) || (hasLastScript && lastScript == script))
			continue;

		++scriptRangeCount;
		lastScript = script;
		hasLastScript = true;
	} while (true);

	if (scriptRangeCount == 0)
		scriptRangeCount = 1;
	if (outScriptRangeCount)
		*outScriptRangeCount = scriptRangeCount;

	return DS_ALIGNED_SIZE(sizeof(dsText)) + DS_ALIGNED_SIZE(sizeof(uint32_t)*length) +
		DS_ALIGNED_SIZE(sizeof(dsGlyph)*length) +
		DS_ALIGNED_SIZE(sizeof(dsScriptInfo)*scriptRangeCount);
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

	// Shape the ranges of text. Lock only needs to be acquired for this portion.
	dsFaceGroup_lock(group);
	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		scriptInfo = getScriptInfo(text, i);
		if (!dsFont_shapeRange(text->font, text, i, scriptInfo->firstCodepoint, scriptInfo->start,
			scriptInfo->count))
		{
			return false;
		}
	}
	dsFaceGroup_unlock(group);

	return true;
}

static dsText* createTextImpl(dsFont* font, dsAllocator* allocator, const void* string,
	dsNextCodepointFunction nextCodepoint, dsCodepointCountFunction codepointCount)
{
	DS_PROFILE_FUNC_START();
	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Allocator for text must support freeing memory.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t length = string ? codepointCount(string) : 0;
	if (length == DS_UNICODE_INVALID)
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Invalid Unicode string.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t scriptRangeCount = 0;
	size_t fullSize = fullAllocSizeImpl(dsFont_getFaceGroup(font), string, nextCodepoint,
		&scriptRangeCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsText* text = (dsText*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, sizeof(dsText));
	DS_ASSERT(text);
	text->font = font;
	text->allocator = dsAllocator_keepPointer(allocator);
	if (length > 0)
	{
		text->characters = (uint32_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(uint32_t)*length);
		DS_ASSERT(text->characters);
		text->glyphs = (dsGlyph*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(dsGlyph)*length);
		DS_ASSERT(text->glyphs);
		text->ranges = (dsTextRange*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(dsTextRange)*length);
		DS_ASSERT(text->ranges);

		text->characterCount = length;
		text->glyphCount = 0; // will be set later
		text->rangeCount = scriptRangeCount;

		uint32_t index = 0;
		for (uint32_t i = 0; i < length; ++i)
			((uint32_t*)text->characters)[i] = nextCodepoint(string, &index);
		if (!shapeText(text))
		{
			if (text->allocator)
				dsAllocator_free(text->allocator, text);
			return NULL;
		}
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

size_t dsText_fullAllocSizeUTF8(const dsFont* font, const char* string)
{
	return fullAllocSizeImpl(dsFont_getFaceGroup(font), string,
		(dsNextCodepointFunction)&dsUTF8_nextCodepoint, NULL);
}

size_t dsText_fullAllocSizeUTF16(const dsFont* font, const uint16_t* string)
{
	return fullAllocSizeImpl(dsFont_getFaceGroup(font), string,
		(dsNextCodepointFunction)&dsUTF16_nextCodepoint, NULL);
}

size_t dsText_fullAllocSizeUTF32(const dsFont* font, const uint32_t* string)
{
	return fullAllocSizeImpl(dsFont_getFaceGroup(font), string,
		(dsNextCodepointFunction)&dsUTF32_nextCodepoint, NULL);
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
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF8_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF8_codepointCount);
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
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF16_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF16_codepointCount);
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
	return createTextImpl(font, allocator, string, (dsNextCodepointFunction)&dsUTF32_nextCodepoint,
		(dsCodepointCountFunction)&dsUTF32_codepointCount);
}
