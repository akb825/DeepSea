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

	dsText* scratchText;
	uint32_t scratchLength;
	uint32_t scratchRangeCount;

	dsGlyph* scratchGlyphs;
	uint32_t scratchMaxGlyphs;
	uint32_t scratchGlyphCount;

	dsParagraphInfo* paragraphs;
	uint32_t maxParagraphs;

	dsRunInfo* runs;
	uint32_t maxRuns;

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
	FT_Load_Glyph(ftFace, glyph, FT_LOAD_MONOCHROME | FT_LOAD_NO_HINTING | FT_LOAD_RENDER);

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

		font->tempImage = (uint8_t*)dsAllocator_alloc(scratchAllocator,
			font->maxWidth*font->maxHeight*sizeof(uint8_t));
		if (!font->tempImage)
		{
			font->maxWidth = font->maxHeight = 0;
			return false;
		}

		uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
		uint32_t sdfWidth = font->maxWidth + windowSize*2;
		uint32_t sdfHeight= font->maxHeight + windowSize*2;
		font->tempSdf = (float*)dsAllocator_alloc(scratchAllocator,
			sdfWidth*sdfHeight*sizeof(float));
		if (!font->tempSdf)
		{
			dsAllocator_free(scratchAllocator, font->tempImage);
			font->tempImage = NULL;
			font->maxWidth = font->maxHeight = 0;
			return false;
		}
	}

	DS_ASSERT(bitmap->pixel_mode == FT_PIXEL_MODE_MONO ||
		(bitmap->rows == 0 && bitmap->width == 0));
	for (unsigned int y = 0; y < bitmap->rows; ++y)
	{
		const uint8_t* row = bitmap->buffer + abs(bitmap->pitch)*y;
		unsigned int destY = bitmap->pitch > 0 ? y : bitmap->rows - y - 1;
		for (unsigned int x = 0; x < bitmap->width; ++x)
		{
			uint32_t mask = (1 << (7 - (x % 8)));
			font->tempImage[destY*bitmap->width + x] = (row[x/8] & mask) != 0;
		}
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

	SBAlgorithmRef algorithm = SBAlgorithmCreate(&sequence);
	if (!algorithm)
		return NULL;

	// Count the paragraphs to allocate the array.
	SBUInteger offset = 0;
	unsigned int paragraphCount = 0;
	while (offset < sequence.stringLength)
	{
		SBUInteger length, separatorLength;
		SBAlgorithmGetParagraphBoundary(algorithm, offset, sequence.stringLength - offset, &length,
			&separatorLength);
		++paragraphCount;
		offset += length + separatorLength;
	}

	if (!group->paragraphs || paragraphCount > group->maxParagraphs)
	{
		dsAllocator_free(group->scratchAllocator, group->paragraphs);
		group->paragraphs = (dsParagraphInfo*)dsAllocator_alloc(group->scratchAllocator,
			paragraphCount*sizeof(dsParagraphInfo));
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
		if (group->paragraphs[i].paragraph)
		{
			group->paragraphs[i].line = SBParagraphCreateLine(group->paragraphs[i].paragraph,
				offset, length);
			if (!group->paragraphs[i].line)
			{
				SBParagraphRelease(group->paragraphs[i].paragraph);
				group->paragraphs[i].paragraph = NULL;
			}
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
		dsAllocator_free(group->scratchAllocator, group->runs);
		group->runs = (dsRunInfo*)dsAllocator_alloc(group->scratchAllocator,
			*outCount*sizeof(dsRunInfo));
		if (!group->runs)
		{
			for (unsigned int i = 0; i < paragraphCount; ++i)
			{
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
		SBUInteger curCount = SBLineGetRunCount(group->paragraphs[i].line);
		const SBRun* runArray = SBLineGetRunsPtr(group->paragraphs[i].line);
		for (SBUInteger j = 0; j < curCount; ++j, ++run)
		{
			group->runs[run].start = (uint32_t)runArray[j].offset;
			// Include line endings with the runs, so don't use the length directly.
			if (run > 0)
				group->runs[run - 1].count = group->runs[run].start - group->runs[run - 1].start;
		}
	}

	// Set the length for the last run.
	DS_ASSERT(run == *outCount);
	group->runs[run - 1].count = (uint32_t)(sequence.stringLength - group->runs[run - 1].start);

	// Free temporary objects.
	for (unsigned int i = 0; i < paragraphCount; ++i)
	{
		SBLineRelease(group->paragraphs[i].line);
		SBParagraphRelease(group->paragraphs[i].paragraph);
	}
	SBAlgorithmRelease(algorithm);

	return group->runs;
}

dsText* dsFaceGroup_scratchText(dsFaceGroup* group, uint32_t length, uint32_t rangeCount)
{
	if (group->scratchText && length <= group->scratchLength &&
		rangeCount <= group->scratchRangeCount)
	{
		group->scratchText->characterCount = length;
		group->scratchText->rangeCount = rangeCount;
		group->scratchText->glyphCount = 0;
		group->scratchText->rangeCount = 0;
		return group->scratchText;
	}

	group->scratchLength = dsMax(length, group->scratchLength);
	group->scratchRangeCount = dsMax(rangeCount, group->scratchRangeCount);
	if (group->scratchText)
	{
		dsAllocator_free(group->scratchAllocator, group->scratchText);
		group->scratchText = NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsText)) +
		DS_ALIGNED_SIZE(group->scratchLength*sizeof(uint32_t)) +
		DS_ALIGNED_SIZE(group->scratchRangeCount*sizeof(dsTextRange));
	void* buffer = dsAllocator_alloc(group->scratchAllocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	group->scratchText = (dsText*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, sizeof(dsText));
	DS_ASSERT(group->scratchText);

	group->scratchText->allocator = group->scratchAllocator;
	group->scratchText->font = NULL;

	group->scratchText->characters = (uint32_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		group->scratchLength*sizeof(uint32_t));
	DS_ASSERT(group->scratchText->characters);
	group->scratchText->characterCount = length;

	group->scratchText->ranges = (dsTextRange*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		group->scratchRangeCount*sizeof(dsTextRange));
	DS_ASSERT(group->scratchText->ranges);
	group->scratchText->rangeCount = rangeCount;

	group->scratchText->glyphs = NULL;
	group->scratchText->glyphCount = 0;

	return group->scratchText;
}

dsGlyph* dsFaceGroup_scratchGlyphs(dsFaceGroup* group, uint32_t length)
{
	if (length <= group->scratchGlyphCount)
		return group->scratchGlyphs;

	if (!dsResizeableArray_add(group->scratchAllocator,
		(void**)&group->scratchGlyphs, &group->scratchGlyphCount, &group->scratchMaxGlyphs,
		sizeof(dsGlyph), length - group->scratchGlyphCount))
	{
		return NULL;
	}

	return group->scratchGlyphs;
}

uint32_t dsFaceGroup_codepointScript(const dsFaceGroup* group, uint32_t codepoint)
{
	return hb_unicode_script(group->unicode, codepoint);
}

bool dsFaceGroup_isScriptUnique(uint32_t script)
{
	return script != HB_SCRIPT_COMMON && script != HB_SCRIPT_INHERITED &&
		script != HB_SCRIPT_UNKNOWN;
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
		sizeof(dsFaceGroup) + sizeof(dsFontFace)*maxFaces);
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

	faceGroup->scratchText = NULL;
	faceGroup->scratchLength = 0;
	faceGroup->scratchGlyphCount = 0;
	faceGroup->scratchGlyphs = NULL;
	faceGroup->scratchMaxGlyphs = 0;
	faceGroup->scratchGlyphCount = 0;
	faceGroup->paragraphs = NULL;
	faceGroup->maxParagraphs = 0;
	faceGroup->runs = NULL;
	faceGroup->maxRuns = 0;

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

	if (group->scratchText)
		dsAllocator_free(group->scratchAllocator, group->scratchText);
	if (group->scratchGlyphs)
		dsAllocator_free(group->scratchAllocator, group->scratchGlyphs);
	if (group->paragraphs)
		dsAllocator_free(group->scratchAllocator, group->paragraphs);
	if (group->runs)
		dsAllocator_free(group->scratchAllocator, group->runs);

	if (group->allocator)
		dsAllocator_free(group->allocator, group);
}

bool dsFont_shapeRange(const dsFont* font, dsText* text, uint32_t rangeIndex,
	uint32_t firstCodepoint, uint32_t start, uint32_t count)
{
	DS_ASSERT(text == font->group->scratchText);
	if (count == 0)
		return true;

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
	hb_buffer_guess_segment_properties(shapeBuffer);
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
		hb_buffer_reset(shapeBuffer);
		return true;
	}

	// Make sure the glyph buffer is large enough.
	text->glyphs = dsFaceGroup_scratchGlyphs(font->group, text->glyphCount + glyphCount);
	if (!text->glyphs)
	{
		hb_buffer_reset(shapeBuffer);
		return false;
	}

	hb_segment_properties_t properties;
	hb_buffer_get_segment_properties(shapeBuffer, &properties);

	dsTextRange* range = (dsTextRange*)(text->ranges + rangeIndex);
	range->face = face;
	range->firstChar = start;
	range->charCount = count;
	range->firstGlyph = text->glyphCount;
	range->glyphCount = glyphCount;
	DS_ASSERT(!HB_DIRECTION_IS_VERTICAL(properties.direction));
	range->backward = HB_DIRECTION_IS_BACKWARD(properties.direction);

	float scale = 1.0f/(float)(fixedScale*font->glyphSize);
	dsGlyph* glyphs = (dsGlyph*)(text->glyphs + text->glyphCount);
	for (unsigned int i = 0; i < glyphCount; ++i)
	{
		glyphs[i].glyphId = glyphInfos[i].codepoint;
		glyphs[i].charIndex = glyphInfos[i].cluster;
		glyphs[i].canBreak = (glyphInfos[i].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK) == 0;
		glyphs[i].offset.x = (float)glyphPos[i].x_offset*scale;
		glyphs[i].offset.y = -(float)glyphPos[i].y_offset*scale;
		glyphs[i].advance = (float)glyphPos[i].x_advance*scale;
		DS_ASSERT(glyphPos[i].y_advance == 0);
	}

	text->glyphCount += glyphCount;
	hb_buffer_reset(shapeBuffer);
	return true;
}
