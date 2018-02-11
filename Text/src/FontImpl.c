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

#include "FontImpl.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Unicode.h>
#include <SheenBidi.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <hb.h>
#include <hb-ft.h>

struct dsFontFace
{
	dsHashTableNode node;
	char name[DS_MAX_FACE_NAME_LENGTH];
	dsAllocator* bufferAllocator;
	void* buffer;
	hb_font_t* font;
	uint32_t maxWidth;
	uint32_t maxHeight;
};

typedef struct dsParagraphInfo
{
	SBParagraphRef paragraph;
	SBLineRef line;
} dsParagraphInfo;

struct dsFaceGroup
{
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsMutex* mutex;
	dsHashTable* faceHashTable;
	struct FT_MemoryRec_ memory;
	FT_Library library;
	hb_unicode_funcs_t* unicode;
	hb_buffer_t* shapeBuffer;
	dsTextQuality quality;

	dsText scratchText;

	uint32_t* scratchCodepoints;
	uint32_t scratchMaxCodepoints;

	dsTextRange* scratchRanges;
	uint32_t scratchMaxRanges;

	dsGlyph* scratchGlyphs;
	uint32_t scratchGlyphCount;
	uint32_t scratchMaxGlyphs;

	dsParagraphInfo* paragraphs;
	uint32_t maxParagraphs;

	dsRunInfo* runs;
	uint32_t maxRuns;

	uint32_t* charMapping;
	uint32_t maxCharMappingCount;

	dsGlyphMapping* glyphMapping;
	uint32_t maxGlyphMappingCount;

	uint32_t maxFaces;
	uint32_t faceCount;
	dsFontFace faces[];
};

static const unsigned int fixedScale = 1 << 6;

static void* ftAlloc(FT_Memory memory, long size)
{
	return dsAllocator_alloc((dsAllocator*)memory->user, size);
}

static void ftFree(FT_Memory memory, void* block)
{
	dsAllocator_free((dsAllocator*)memory->user, block);
}

static void* ftRealloc(FT_Memory memory, long curSize, long newSize, void* block)
{
	if (newSize == 0)
	{
		dsAllocator_free((dsAllocator*)memory->user, block);
		return NULL;
	}

	void* newBuffer = dsAllocator_alloc((dsAllocator*)memory->user, newSize);
	if (!newBuffer)
		return NULL;

	memcpy(newBuffer, block, dsMin(curSize, newSize));
	dsAllocator_free((dsAllocator*)memory->user, block);
	return newBuffer;
}

static unsigned int getTableSize(unsigned int maxValues)
{
	const float loadFactor = 0.75f;
	return (unsigned int)((float)maxValues/loadFactor);
}

static bool setFontLoadErrno(FT_Error error)
{
	if (error == FT_Err_Cannot_Open_Resource)
		errno = ENOTFOUND;
	else if (error == FT_Err_Invalid_File_Format)
		errno = EFORMAT;
	else if (error == FT_Err_Out_Of_Memory)
		errno = ENOMEM;
	else if (error != 0)
		errno = EPERM;

	return error != 0;
}

static dsFontFace* insertFace(dsFaceGroup* group, const char* name, FT_Face ftFace)
{
	if (group->faceCount >= group->maxFaces)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Exceeded maximum number of faces.");
		return NULL;
	}
	if (!(ftFace->face_flags & FT_FACE_FLAG_SCALABLE))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Face '%s' isn't a vector font.", name);
		return NULL;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_FACE_NAME_LENGTH)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Face name '%s' exceeds maximum size of %u.", name,
			DS_MAX_FACE_NAME_LENGTH);
		return NULL;
	}

	if (dsHashTable_find(group->faceHashTable, name))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Face '%s' has already been loaded.", name);
		return NULL;
	}
	else if (group->faceHashTable->list.length == group->faceHashTable->tableSize)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Maximum number of faces has been exceeded.");
		return NULL;
	}

	switch (group->quality)
	{
		case dsTextQuality_Low:
			FT_Set_Pixel_Sizes(ftFace, 0, DS_LOW_SIZE);
			break;
		case dsTextQuality_High:
			FT_Set_Pixel_Sizes(ftFace, 0, DS_HIGH_SIZE);
			break;
		case dsTextQuality_VeryHigh:
			FT_Set_Pixel_Sizes(ftFace, 0, DS_VERY_HIGH_SIZE);
			break;
		case dsTextQuality_Medium:
		default:
			FT_Set_Pixel_Sizes(ftFace, 0, DS_MEDIUM_SIZE);
			break;
	}
	hb_font_t* hbFont = hb_ft_font_create_referenced(ftFace);
	if (!hbFont)
		return NULL;

	FT_Pos widthU = ftFace->bbox.xMax - ftFace->bbox.xMin;
	FT_Pos heightU = ftFace->bbox.yMax - ftFace->bbox.yMin;
	FT_Fixed width = widthU*ftFace->size->metrics.x_scale;
	FT_Fixed height = heightU*ftFace->size->metrics.y_scale;

	dsFontFace* face = group->faces + group->faceCount++;
	strncpy(face->name, name, sizeof(face->name));
	face->bufferAllocator = NULL;
	face->buffer = NULL;
	face->font = hbFont;
	face->maxWidth = (uint32_t)FT_CeilFix(width) >> 16;
	face->maxHeight = (uint32_t)FT_CeilFix(height) >> 16;
	DS_VERIFY(dsHashTable_insert(group->faceHashTable, face->name, (dsHashTableNode*)face, NULL));
	return face;
}

bool dsIsSpace(uint32_t charcode)
{
	// Work around assert on Windows.
	return charcode <= 128 && isspace(charcode);
}

const char* dsFontFace_getName(const dsFontFace* face)
{
	if (!face)
		return NULL;

	return face->name;
}

bool dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsVector2i* outTexSize, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	uint32_t glyphSize, dsFont* font)
{
	FT_Face ftFace = hb_ft_font_get_face(face->font);
	DS_ASSERT(ftFace);
	FT_Load_Glyph(ftFace, glyph, FT_LOAD_NO_HINTING | FT_LOAD_RENDER);

	float scale = 1.0f/(float)glyphSize;
	FT_Bitmap* bitmap = &ftFace->glyph->bitmap;
	DS_ASSERT(bitmap->width <= face->maxWidth);
	DS_ASSERT(bitmap->rows <= face->maxHeight);
	outBounds->min.x = (float)ftFace->glyph->bitmap_left*scale;
	outBounds->min.y = ((float)ftFace->glyph->bitmap_top - (float)bitmap->rows)*scale;
	outBounds->max.x = outBounds->min.x + (float)bitmap->width*scale;
	outBounds->max.y = outBounds->min.y + (float)bitmap->rows*scale;

	outTexSize->x = bitmap->width;
	outTexSize->y = bitmap->rows;

	// May need to re-allocate the temporary images.
	if (bitmap->width > font->maxWidth || bitmap->rows > font->maxHeight)
	{
		font->maxWidth = dsMax(bitmap->width, font->maxWidth);
		font->maxHeight = dsMax(bitmap->rows, font->maxHeight);

		dsAllocator* scratchAllocator = font->group->scratchAllocator;
		dsAllocator_free(scratchAllocator, font->tempImage);
		dsAllocator_free(scratchAllocator, font->tempSdf);
		font->tempImage = NULL;
		font->tempSdf = NULL;

		font->tempImage = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, uint8_t,
			font->maxWidth*font->maxHeight);
		if (!font->tempImage)
		{
			font->maxWidth = font->maxHeight = 0;
			return false;
		}

		uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
		uint32_t sdfWidth = font->maxWidth + windowSize*2;
		uint32_t sdfHeight= font->maxHeight + windowSize*2;
		font->tempSdf = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, float, sdfWidth*sdfHeight);
		if (!font->tempSdf)
		{
			dsAllocator_free(scratchAllocator, font->tempImage);
			font->tempImage = NULL;
			font->maxWidth = font->maxHeight = 0;
			return false;
		}
	}

	DS_ASSERT(bitmap->pixel_mode == FT_PIXEL_MODE_GRAY ||
		(bitmap->rows == 0 && bitmap->width == 0));
	for (unsigned int y = 0; y < bitmap->rows; ++y)
	{
		const uint8_t* row = bitmap->buffer + abs(bitmap->pitch)*y;
		unsigned int destY = bitmap->pitch > 0 ? y : bitmap->rows - y - 1;
		for (unsigned int x = 0; x < bitmap->width; ++x)
			font->tempImage[destY*bitmap->width + x] = row[x];
	}

	return dsFont_writeGlyphToTexture(commandBuffer, texture, glyphIndex, glyphSize,
		font->tempImage, bitmap->width, bitmap->rows, font->tempSdf);
}

void dsFaceGroup_lock(const dsFaceGroup* group)
{
	DS_VERIFY(dsMutex_lock(group->mutex));
}

void dsFaceGroup_unlock(const dsFaceGroup* group)
{
	DS_VERIFY(dsMutex_unlock(group->mutex));
}

dsAllocator* dsFaceGroup_getScratchAllocator(const dsFaceGroup* group)
{
	return group->scratchAllocator;
}

dsFontFace* dsFaceGroup_findFace(const dsFaceGroup* group, const char* name)
{
	if (!group || !name)
		return NULL;

	return (dsFontFace*)dsHashTable_find(group->faceHashTable, name);
}

dsRunInfo* dsFaceGroup_findBidiRuns(uint32_t* outCount, dsFaceGroup* group, const void* string,
	dsUnicodeType type)
{
	*outCount = DS_UNICODE_INVALID;
	if (!string)
	{
		*outCount = 0;
		return NULL;
	}

	SBCodepointSequence sequence;
	sequence.stringBuffer = (void*)string;
	sequence.stringLength = 0;
	switch (type)
	{
		case dsUnicodeType_UTF8:
			sequence.stringEncoding = SBStringEncodingUTF8;
			sequence.stringLength = dsUTF8_length((const char*)string);
			break;
		case dsUnicodeType_UTF16:
			sequence.stringEncoding = SBStringEncodingUTF16;
			sequence.stringLength = dsUTF16_length((const uint16_t*)string);
			break;
		case dsUnicodeType_UTF32:
			sequence.stringEncoding = SBStringEncodingUTF32;
			sequence.stringLength = dsUTF32_length((const uint32_t*)string);
			break;
	}
	if (sequence.stringLength == 0)
	{
		outCount = 0;
		return NULL;
	}

	// Create a mapping between the characters and codepoinds.
	uint32_t mappingSize = (uint32_t)sequence.stringLength + 1;
	uint32_t* charMapping = dsFaceGroup_charMapping(group, mappingSize);
	if (!charMapping)
		return NULL;

	uint32_t codepointIndex = 0;
	SBUInteger index = 0;
	do
	{
		SBUInteger prevIndex = index;
		uint32_t codepoint = SBCodepointSequenceGetCodepointAt(&sequence, &index);
		if (codepoint == SBCodepointInvalid)
		{
			charMapping[prevIndex] = codepointIndex;
			break;
		}

		for (SBUInteger i = prevIndex; i < index; ++i)
			charMapping[i] = codepointIndex;
		++codepointIndex;
	}
	while (true);

	SBAlgorithmRef algorithm = SBAlgorithmCreate(&sequence);
	if (!algorithm)
		return NULL;

	// Count the paragraphs to allocate the array.
	SBUInteger offset = 0;
	unsigned int paragraphCount = 0;
	while (offset < sequence.stringLength)
	{
		SBUInteger length = 0, separatorLength = 0;
		SBAlgorithmGetParagraphBoundary(algorithm, offset, sequence.stringLength - offset, &length,
			&separatorLength);
		++paragraphCount;
		offset += length + separatorLength;
	}

	if (!group->paragraphs || paragraphCount > group->maxParagraphs)
	{
		dsAllocator_free(group->scratchAllocator, group->paragraphs);
		group->paragraphs = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, dsParagraphInfo,
			paragraphCount);
		if (!group->paragraphs)
		{
			SBAlgorithmRelease(algorithm);
			return NULL;
		}
		group->maxParagraphs = paragraphCount;
	}

	// Create the paragraphs and lines.
	*outCount = 0;
	memset(group->paragraphs, 0, paragraphCount*sizeof(dsParagraphInfo));
	offset = 0;
	for (unsigned int i = 0; i < paragraphCount; ++i)
	{
		SBUInteger length, separatorLength;
		SBAlgorithmGetParagraphBoundary(algorithm, offset, sequence.stringLength - offset, &length,
			&separatorLength);
		group->paragraphs[i].paragraph = SBAlgorithmCreateParagraph(algorithm, offset, length,
			SBLevelDefaultLTR);
		if (!group->paragraphs[i].paragraph)
		{
			++offset;
			group->paragraphs[i].line = NULL;
			continue;
		}

		group->paragraphs[i].line = SBParagraphCreateLine(group->paragraphs[i].paragraph,
			offset, length);
		if (!group->paragraphs[i].line)
		{
			SBParagraphRelease(group->paragraphs[i].paragraph);
			group->paragraphs[i].paragraph = NULL;
		}

		if (!group->paragraphs[i].paragraph)
		{
			for (unsigned int j = 0; j < i; ++j)
			{
				SBLineRelease(group->paragraphs[j].line);
				SBParagraphRelease(group->paragraphs[j].paragraph);
			}
			SBAlgorithmRelease(algorithm);
			return NULL;
		}

		offset += length + separatorLength;
		*outCount += (uint32_t)SBLineGetRunCount(group->paragraphs[i].line);
	}

	// Create the runs.
	if (!group->runs || group->maxRuns < *outCount)
	{
		if (group->runs)
			dsAllocator_free(group->scratchAllocator, group->runs);
		group->runs = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, dsRunInfo, *outCount);
		if (!group->runs)
		{
			for (unsigned int i = 0; i < paragraphCount; ++i)
			{
				if (!group->paragraphs[i].paragraph)
					continue;

				SBLineRelease(group->paragraphs[i].line);
				SBParagraphRelease(group->paragraphs[i].paragraph);
			}
			SBAlgorithmRelease(algorithm);
			return NULL;
		}
		group->maxRuns = *outCount;
	}

	uint32_t run = 0;
	for (uint32_t i = 0; i < paragraphCount; ++i)
	{
		if (!group->paragraphs[i].line)
		{
			if (run > 0)
				++group->runs[run - 1].newlineCount;
			continue;
		}

		SBUInteger curCount = SBLineGetRunCount(group->paragraphs[i].line);
		const SBRun* runArray = SBLineGetRunsPtr(group->paragraphs[i].line);
		for (SBUInteger j = 0; j < curCount; ++j, ++run)
		{
			// Convert from character run to codepoint run.
			DS_ASSERT(runArray[j].offset < mappingSize);
			DS_ASSERT(runArray[j].offset + runArray[j].length < mappingSize);
			group->runs[run].start = charMapping[runArray[j].offset];
			uint32_t end = charMapping[runArray[j].offset + runArray[j].length];
			group->runs[run].count = end - group->runs[run].start;
			group->runs[run].newlineCount = j == curCount - 1 && i != paragraphCount - 1;
		}
	}
	DS_ASSERT(run == *outCount);

	// Free temporary objects.
	for (unsigned int i = 0; i < paragraphCount; ++i)
	{
		if (!group->paragraphs[i].paragraph)
			continue;

		SBLineRelease(group->paragraphs[i].line);
		SBParagraphRelease(group->paragraphs[i].paragraph);
	}
	SBAlgorithmRelease(algorithm);

	return group->runs;
}

dsText* dsFaceGroup_scratchText(dsFaceGroup* group, uint32_t length)
{
	group->scratchText.characterCount = 0;
	group->scratchText.characters = NULL;
	group->scratchText.rangeCount = 0;
	group->scratchText.ranges = NULL;
	group->scratchText.glyphCount = 0;
	group->scratchText.glyphs = NULL;
	group->scratchGlyphCount = 0;

	if (length == 0)
		return &group->scratchText;

	if (group->scratchCodepoints && length <= group->scratchMaxCodepoints)
	{
		group->scratchText.characters = group->scratchCodepoints;
		group->scratchText.characterCount = length;
		return &group->scratchText;
	}

	if (group->scratchCodepoints)
		dsAllocator_free(group->scratchAllocator, group->scratchCodepoints);
	group->scratchMaxCodepoints = length;
	group->scratchCodepoints = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, uint32_t, length);
	if (!group->scratchCodepoints)
	{
		group->scratchMaxCodepoints = 0;
		return NULL;
	}

	group->scratchText.characterCount = length;
	group->scratchText.characters = group->scratchCodepoints;
	return &group->scratchText;
}

bool dsFaceGroup_scratchRanges(dsFaceGroup* group, uint32_t rangeCount)
{
	if (rangeCount == 0)
		return true;

	if (group->scratchRanges && rangeCount <= group->scratchMaxRanges)
	{
		group->scratchText.rangeCount = rangeCount;
		group->scratchText.ranges = group->scratchRanges;
		return true;
	}

	if (group->scratchRanges)
		dsAllocator_free(group->scratchAllocator, group->scratchRanges);
	group->scratchMaxRanges = rangeCount;
	group->scratchRanges = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, dsTextRange, rangeCount);
	if (!group->scratchRanges)
	{
		group->scratchMaxRanges = 0;
		return false;
	}

	group->scratchText.rangeCount = rangeCount;
	group->scratchText.ranges = group->scratchRanges;
	return true;
}

bool dsFaceGroup_scratchGlyphs(dsFaceGroup* group, uint32_t length)
{
	if (length <= group->scratchGlyphCount)
	{
		group->scratchText.glyphCount = length;
		group->scratchText.glyphs = group->scratchGlyphs;
		return true;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(group->scratchAllocator, group->scratchGlyphs,
		group->scratchGlyphCount, group->scratchMaxGlyphs, length - group->scratchGlyphCount))
	{
		return false;
	}

	group->scratchText.glyphCount = length;
	group->scratchText.glyphs = group->scratchGlyphs;
	return true;
}

uint32_t* dsFaceGroup_charMapping(dsFaceGroup* group, uint32_t length)
{
	if (group->charMapping && length <= group->maxCharMappingCount)
		return group->charMapping;

	if (group->charMapping)
		dsAllocator_free(group->scratchAllocator, group->charMapping);
	group->maxCharMappingCount = length;
	group->charMapping = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, uint32_t, length);
	if (!group->charMapping)
	{
		group->maxCharMappingCount = 0;
		return NULL;
	}

	return group->charMapping;
}

dsGlyphMapping* dsFaceGroup_glyphMapping(dsFaceGroup* group, uint32_t length)
{
	if (group->glyphMapping && length <= group->maxGlyphMappingCount)
		return group->glyphMapping;

	if (group->glyphMapping)
		dsAllocator_free(group->scratchAllocator, group->glyphMapping);
	group->maxGlyphMappingCount = length;
	group->glyphMapping = DS_ALLOCATE_OBJECT_ARRAY(group->scratchAllocator, dsGlyphMapping, length);
	if (!group->glyphMapping)
	{
		group->maxGlyphMappingCount = 0;
		return NULL;
	}

	return group->glyphMapping;
}

uint32_t dsFaceGroup_codepointScript(const dsFaceGroup* group, uint32_t codepoint)
{
	// Override whitepsace.
	if (dsIsSpace(codepoint))
		return HB_SCRIPT_INHERITED;
	return hb_unicode_script(group->unicode, codepoint);
}

bool dsFaceGroup_isScriptUnique(uint32_t script)
{
	return script != HB_SCRIPT_INHERITED && script != HB_SCRIPT_UNKNOWN;
}

bool dsFaceGroup_areScriptsEqual(uint32_t script1, uint32_t script2)
{
	// Treate common as latin to account for international fonts that only include the unique
	// scripts.
	if (script1 == HB_SCRIPT_COMMON)
		script1 = HB_SCRIPT_LATIN;
	if (script2 == HB_SCRIPT_COMMON)
		script2 = HB_SCRIPT_LATIN;
	return script1 == script2;
}

dsTextDirection dsFaceGroup_textDirection(uint32_t script)
{
	if (script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_UNKNOWN)
		return dsTextDirection_Either;

	if (hb_script_get_horizontal_direction((hb_script_t)script) == HB_DIRECTION_RTL)
		return dsTextDirection_RightToLeft;
	return dsTextDirection_LeftToRight;
}

size_t dsFaceGroup_fullAllocSize(uint32_t maxFaces)
{
	return DS_ALIGNED_SIZE(sizeof(dsFaceGroup) + sizeof(dsFontFace)*maxFaces) +
		dsMutex_fullAllocSize() + dsHashTable_fullAllocSize(getTableSize(maxFaces));
}

dsFaceGroup* dsFaceGroup_create(dsAllocator* allocator, dsAllocator* scratchAllocator,
	uint32_t maxFaces, dsTextQuality quality)
{
	if (!allocator || maxFaces == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;
	if (!scratchAllocator->freeFunc)
	{
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Face group scratch allocator must support freeing memory.");
		errno = EPERM;
		return NULL;
	}

	size_t fullSize = dsFaceGroup_fullAllocSize(maxFaces);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsFaceGroup* faceGroup = (dsFaceGroup*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		DS_ALIGNED_SIZE(sizeof(dsFaceGroup)) + DS_ALIGNED_SIZE(sizeof(dsFontFace)*maxFaces));
	DS_ASSERT(faceGroup);

	uint32_t hashTableSize = getTableSize(maxFaces);
	faceGroup->allocator = dsAllocator_keepPointer(allocator);
	faceGroup->scratchAllocator = scratchAllocator;
	faceGroup->faceHashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(hashTableSize));
	DS_ASSERT(faceGroup->faceHashTable);
	DS_VERIFY(dsHashTable_initialize(faceGroup->faceHashTable, hashTableSize, &dsHashString,
		dsHashStringEqual));

	faceGroup->mutex = dsMutex_create((dsAllocator*)&bufferAlloc, "dsFaceGroup");
	faceGroup->unicode = hb_unicode_funcs_get_default();
	if (!faceGroup->unicode)
	{
		if (faceGroup->allocator)
			dsAllocator_free(allocator, faceGroup);
		return NULL;
	}

	faceGroup->shapeBuffer = hb_buffer_create();
	if (!faceGroup->shapeBuffer)
	{
		hb_unicode_funcs_destroy(faceGroup->unicode);
		if (faceGroup->allocator)
			dsAllocator_free(allocator, faceGroup);
		return NULL;
	}

	faceGroup->memory.user = scratchAllocator;
	faceGroup->memory.alloc = &ftAlloc;
	faceGroup->memory.free = &ftFree;
	faceGroup->memory.realloc = &ftRealloc;
	if (FT_New_Library(&faceGroup->memory, &faceGroup->library) != 0)
	{
		hb_unicode_funcs_destroy(faceGroup->unicode);
		hb_buffer_destroy(faceGroup->shapeBuffer);
		if (faceGroup->allocator)
			dsAllocator_free(allocator, faceGroup);
		return NULL;
	}

	FT_Add_Default_Modules(faceGroup->library);
	FT_Set_Default_Properties(faceGroup->library);

	memset(&faceGroup->scratchText, 0, sizeof(dsText));

	faceGroup->scratchCodepoints = NULL;
	faceGroup->scratchMaxCodepoints = 0;

	faceGroup->scratchRanges= NULL;
	faceGroup->scratchMaxRanges = 0;

	faceGroup->scratchGlyphs = NULL;
	faceGroup->scratchGlyphCount = 0;
	faceGroup->scratchMaxGlyphs = 0;;

	faceGroup->paragraphs = NULL;
	faceGroup->maxParagraphs = 0;

	faceGroup->runs = NULL;
	faceGroup->maxRuns = 0;

	faceGroup->charMapping = NULL;
	faceGroup->maxCharMappingCount = 0;

	faceGroup->glyphMapping = NULL;
	faceGroup->maxGlyphMappingCount = 0;

	faceGroup->maxFaces = maxFaces;
	faceGroup->faceCount = 0;
	faceGroup->quality = quality;
	return faceGroup;
}

dsAllocator* dsFaceGroup_getAllocator(const dsFaceGroup* group)
{
	if (!group)
		return NULL;

	return group->allocator;
}

uint32_t dsFaceGroup_getRemainingFaces(const dsFaceGroup* group)
{
	if (!group)
		return 0;

	DS_VERIFY(dsMutex_lock(group->mutex));
	uint32_t count = group->maxFaces - group->faceCount;
	DS_VERIFY(dsMutex_unlock(group->mutex));
	return count;
}

bool dsFaceGroup_hasFace(const dsFaceGroup* group, const char* name)
{
	if (!group)
		return false;

	DS_VERIFY(dsMutex_lock(group->mutex));
	bool found = dsFaceGroup_findFace(group, name) != NULL;
	DS_VERIFY(dsMutex_unlock(group->mutex));
	return found;
}

bool dsFaceGroup_loadFaceFile(dsFaceGroup* group, const char* fileName, const char* name)
{
	if (!group || !fileName || !name)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsMutex_lock(group->mutex));
	FT_Face ftFace;
	if (setFontLoadErrno(FT_New_Face(group->library, fileName, 0, &ftFace)))
	{
		DS_VERIFY(dsMutex_unlock(group->mutex));
		return false;
	}

	if (insertFace(group, name, ftFace) == NULL)
	{
		FT_Done_Face(ftFace);
		DS_VERIFY(dsMutex_unlock(group->mutex));
		return false;
	}

	DS_VERIFY(dsMutex_unlock(group->mutex));
	return true;
}

bool dsFaceGroup_loadFaceBuffer(dsFaceGroup* group, dsAllocator* allocator, const void* buffer,
	size_t size, const char* name)
{
	if (!group || buffer || size == 0 || !name)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsMutex_lock(group->mutex));
	void* loadBuffer;
	if (allocator)
	{
		loadBuffer = dsAllocator_alloc(allocator, size);
		if (!loadBuffer)
			return false;
		memcpy(loadBuffer, buffer, size);
	}
	else
		loadBuffer = (void*)buffer;

	FT_Open_Args args;
	args.flags = FT_OPEN_MEMORY;
	args.memory_base = (const FT_Byte*)loadBuffer;
	args.memory_size = (FT_Long)size;
	FT_Face ftFace;
	if (setFontLoadErrno(FT_Open_Face(group->library, &args, 0, &ftFace)))
	{
		DS_VERIFY(dsMutex_unlock(group->mutex));
		return false;
	}

	dsFontFace* face = insertFace(group, name, ftFace);
	if (!face)
	{
		FT_Done_Face(ftFace);
		DS_VERIFY(dsMutex_unlock(group->mutex));
		return false;
	}

	if (loadBuffer != buffer && dsAllocator_keepPointer(allocator))
	{
		face->bufferAllocator = allocator;
		face->buffer = loadBuffer;
	}

	DS_VERIFY(dsMutex_unlock(group->mutex));
	return true;
}

dsTextQuality dsFaceGroup_getTextQuality(const dsFaceGroup* group)
{
	if (!group)
		return dsTextQuality_Medium;

	return group->quality;
}

void dsFaceGroup_destroy(dsFaceGroup* group)
{
	for (dsListNode* node = group->faceHashTable->list.head; node; node = node->next)
	{
		dsFontFace* face = (dsFontFace*)node;
		hb_font_destroy(face->font);
		if (face->buffer)
			DS_VERIFY(dsAllocator_free(face->bufferAllocator, face->buffer));
	}
	dsMutex_destroy(group->mutex);
	hb_unicode_funcs_destroy(group->unicode);
	hb_buffer_destroy(group->shapeBuffer);

	FT_Done_Library(group->library);

	if (group->scratchCodepoints)
		dsAllocator_free(group->scratchAllocator, group->scratchCodepoints);
	if (group->scratchRanges)
		dsAllocator_free(group->scratchAllocator, group->scratchRanges);
	if (group->scratchGlyphs)
		dsAllocator_free(group->scratchAllocator, group->scratchGlyphs);
	if (group->paragraphs)
		dsAllocator_free(group->scratchAllocator, group->paragraphs);
	if (group->runs)
		dsAllocator_free(group->scratchAllocator, group->runs);
	if (group->charMapping)
		dsAllocator_free(group->scratchAllocator, group->charMapping);
	if (group->glyphMapping)
		dsAllocator_free(group->scratchAllocator, group->glyphMapping);

	if (group->allocator)
		dsAllocator_free(group->allocator, group);
}

bool dsFont_shapeRange(const dsFont* font, dsText* text, uint32_t rangeIndex,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction)
{
	DS_ASSERT(text == &font->group->scratchText);
	dsTextRange* range = (dsTextRange*)(text->ranges + rangeIndex);
	if (count == 0)
	{
		range->face = 0;
		range->firstChar = start;
		range->charCount = count;
		range->firstGlyph = text->glyphCount;
		range->glyphCount = 0;
		range->newlineCount = newlineCount;
		range->backward = false;
		return true;
	}

	uint32_t face = 0;
	for (uint32_t i = 0; i < font->faceCount; ++i)
	{
		if (FT_Get_Char_Index(hb_ft_font_get_face(font->faces[i]->font), firstCodepoint))
		{
			face = i;
			break;
		}
	}

	hb_buffer_t* shapeBuffer = font->group->shapeBuffer;
	hb_buffer_add_codepoints(shapeBuffer, text->characters, text->characterCount, start,
		count);
	if (direction == dsTextDirection_RightToLeft)
		hb_buffer_set_direction(shapeBuffer, HB_DIRECTION_RTL);
	else
		hb_buffer_set_direction(shapeBuffer, HB_DIRECTION_LTR);
	hb_buffer_set_script(shapeBuffer, (hb_script_t)dsFaceGroup_codepointScript(font->group,
		firstCodepoint));
	hb_buffer_set_language(shapeBuffer, hb_language_get_default());
	hb_shape(font->faces[face]->font, shapeBuffer, NULL, 0);
	if (!hb_buffer_allocation_successful(shapeBuffer))
	{
		hb_buffer_reset(shapeBuffer);
		errno = ENOMEM;
		return false;
	}

	unsigned int glyphCount = 0;
	hb_glyph_info_t* glyphInfos = hb_buffer_get_glyph_infos(shapeBuffer, &glyphCount);
	unsigned int glyphPosCount;
	hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(shapeBuffer, &glyphPosCount);
	DS_ASSERT(glyphCount == glyphPosCount);
	if (glyphCount == 0)
	{
		range->face = 0;
		range->firstChar = start;
		range->charCount = count;
		range->firstGlyph = text->glyphCount;
		range->glyphCount = 0;
		range->newlineCount = newlineCount;
		range->backward = false;
		hb_buffer_reset(shapeBuffer);
		return true;
	}

	// Make sure the glyph buffer is large enough.
	uint32_t glyphOffset = text->glyphCount;
	if (!dsFaceGroup_scratchGlyphs(font->group, text->glyphCount + glyphCount))
	{
		hb_buffer_reset(shapeBuffer);
		return false;
	}

	hb_segment_properties_t properties;
	hb_buffer_get_segment_properties(shapeBuffer, &properties);

	range->face = face;
	range->firstChar = start;
	range->charCount = count;
	range->firstGlyph = glyphOffset;
	range->glyphCount = glyphCount;
	DS_ASSERT(!HB_DIRECTION_IS_VERTICAL(properties.direction));
	range->newlineCount = newlineCount;
	range->backward = HB_DIRECTION_IS_BACKWARD(properties.direction);

	float scale = 1.0f/(float)(fixedScale*font->glyphSize);
	dsGlyph* glyphs = (dsGlyph*)(text->glyphs + glyphOffset);
	for (unsigned int i = 0; i < glyphCount; ++i)
	{
		glyphs[i].glyphId = glyphInfos[i].codepoint;
		glyphs[i].charIndex = glyphInfos[i].cluster;
		glyphs[i].canBreak = (glyphInfos[i].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK) == 0;
		glyphs[i].offset.x = (float)glyphPos[i].x_offset*scale;
		glyphs[i].offset.y = -(float)glyphPos[i].y_offset*scale;
		// Special handling for newlines, since they are used in layout but will have an invalid
		// glyph.
		if (text->characters[glyphs[i].charIndex] == '\n')
			glyphs[i].advance = 0;
		else
			glyphs[i].advance = (float)glyphPos[i].x_advance*scale;
		DS_ASSERT(glyphPos[i].y_advance == 0);
	}

	hb_buffer_reset(shapeBuffer);
	return true;
}
