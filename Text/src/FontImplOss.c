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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Text/FaceGroup.h>
#include <math.h>
#include <string.h>

#ifdef DS_OSS_TEXT
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
};

struct dsFaceGroup
{
	dsAllocator* allocator;
	dsHashTable* faceHashTable;
	dsPoolAllocator faces;
	struct FT_MemoryRec_ memory;
	FT_Library library;
	dsTextQuality quality;
};

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

	switch (group->quality)
	{
		case dsTextQuality_Low:
			FT_Set_Pixel_Sizes(ftFace, DS_LOW_SIZE, DS_LOW_SIZE);
			break;
		case dsTextQuality_High:
			FT_Set_Pixel_Sizes(ftFace, DS_HIGH_SIZE, DS_HIGH_SIZE);
			break;
		case dsTextQuality_Medium:
		default:
			FT_Set_Pixel_Sizes(ftFace, DS_MEDIUM_SIZE, DS_MEDIUM_SIZE);
			break;
	}
	hb_font_t* hbFont = hb_ft_font_create_referenced(ftFace);
	if (!hbFont)
		return NULL;

	dsFontFace* face = (dsFontFace*)dsAllocator_alloc((dsAllocator*)&group->faces,
		sizeof(dsFontFace));
	if (!face)
		return NULL;

	strncpy(face->name, name, sizeof(face->name));
	face->bufferAllocator = NULL;
	face->buffer = NULL;
	face->font= hbFont;
	DS_VERIFY(dsHashTable_insert(group->faceHashTable, face->name, (dsHashTableNode*)face, NULL));
	return face;
}

const char* dsFontFace_getName(const dsFontFace* face)
{
	if (!face)
		return NULL;

	return face->name;
}

void dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	uint32_t glyphSize)
{
	FT_Face ftFace = hb_ft_font_get_face(face->font);
	DS_ASSERT(ftFace);
	FT_Load_Glyph(ftFace, glyph, FT_LOAD_MONOCHROME | FT_LOAD_NO_HINTING | FT_LOAD_RENDER);

	float scale = 1.0f/(float)glyphSize;
	FT_Bitmap* bitmap = &ftFace->glyph->bitmap;
	outBounds->min.x = (float)ftFace->glyph->bitmap_left*scale;
	outBounds->min.y = (float)(ftFace->glyph->bitmap_top - bitmap->rows)*scale;
	outBounds->max.x = outBounds->min.x + (float)bitmap->width*scale;
	outBounds->min.y = outBounds->min.y + (float)bitmap->rows*scale;

	DS_ASSERT(bitmap->pixel_mode == FT_PIXEL_MODE_MONO);
	uint8_t* pixels = (uint8_t*)alloca(bitmap->width*bitmap->rows);
	for (unsigned int y = 0; y < bitmap->rows; ++y)
	{
		const uint8_t* row = bitmap->buffer + abs(bitmap->pitch)*y;
		unsigned int destY = bitmap->pitch > 0 ? y : bitmap->rows - y - 1;
		for (unsigned int x = 0; x < bitmap->width; ++x)
			pixels[destY*bitmap->width + x] = (row[x/8] & 1 << (7 - x)) != 0;
	}

	dsFont_writeGlyphToTexture(commandBuffer, texture, glyphIndex, glyphSize, pixels, bitmap->width,
		bitmap->rows);
}

dsAllocator* dsFaceGroup_getAllocator(const dsFaceGroup* group)
{
	return group->allocator;
}

dsFontFace* dsFaceGroup_findFace(const dsFaceGroup* group, const char* name)
{
	if (!group || !name)
		return NULL;

	return (dsFontFace*)dsHashTable_find(group->faceHashTable, name);
}

size_t dsFaceGroup_fullAllocSize(uint32_t maxFaces)
{
	return DS_ALIGNED_SIZE(sizeof(dsFaceGroup)) +
		dsHashTable_fullAllocSize(getTableSize(maxFaces)) +
		dsPoolAllocator_bufferSize(sizeof(dsFontFace), maxFaces);
}

dsFaceGroup* dsFaceGroup_create(dsAllocator* allocator, uint32_t maxFaces, dsTextQuality quality)
{
	if (!allocator || maxFaces == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsFaceGroup_fullAllocSize(maxFaces);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsFaceGroup* faceGroup = (dsFaceGroup*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsFaceGroup));
	DS_ASSERT(faceGroup);

	uint32_t hashTableSize = getTableSize(maxFaces);
	faceGroup->allocator = dsAllocator_keepPointer(allocator);
	faceGroup->faceHashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(hashTableSize));
	DS_ASSERT(faceGroup->faceHashTable);
	DS_VERIFY(dsHashTable_initialize(faceGroup->faceHashTable, hashTableSize, &dsHashString,
		dsHashStringEqual));

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(dsFontFace), maxFaces);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&faceGroup->faces, sizeof(dsFontFace), maxFaces,
		poolBuffer, poolSize));

	if (faceGroup->allocator)
	{
		faceGroup->memory.user = allocator;
		faceGroup->memory.alloc = &ftAlloc;
		faceGroup->memory.free = &ftFree;
		faceGroup->memory.realloc = &ftRealloc;
		if (FT_New_Library(&faceGroup->memory, &faceGroup->library) != 0)
		{
			if (faceGroup->allocator)
				dsAllocator_free(allocator, faceGroup);
			return NULL;
		}

		FT_Add_Default_Modules(faceGroup->library);
		FT_Set_Default_Properties(faceGroup->library);
	}
	else
	{
		if (FT_Init_FreeType(&faceGroup->library) != 0)
		{
			if (faceGroup->allocator)
				dsAllocator_free(allocator, faceGroup);
			return NULL;
		}
	}

	faceGroup->quality = quality;
	return faceGroup;
}

uint32_t dsFaceGroup_getRemainingFaces(const dsFaceGroup* group)
{
	if (!group)
		return 0;

	return (uint32_t)group->faces.freeCount;
}

bool dsFaceGroup_hasFace(const dsFaceGroup* group, const char* name)
{
	return dsFaceGroup_findFace(group, name) != NULL;
}

bool dsFaceGroup_loadFaceFile(dsFaceGroup* group, const char* fileName, const char* name)
{
	if (!group || !fileName || !name)
	{
		errno = EINVAL;
		return false;
	}

	FT_Face ftFace;
	if (setFontLoadErrno(FT_New_Face(group->library, fileName, 0, &ftFace)))
		return false;

	if (insertFace(group, name, ftFace) == NULL)
	{
		FT_Done_Face(ftFace);
		return false;
	}

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
		return false;

	dsFontFace* face = insertFace(group, name, ftFace);
	if (!face)
	{
		FT_Done_Face(ftFace);
		return false;
	}

	if (loadBuffer != buffer && dsAllocator_keepPointer(allocator))
	{
		face->bufferAllocator = allocator;
		face->buffer = loadBuffer;
	}

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

	if (group->allocator)
	{
		FT_Done_Library(group->library);
		DS_VERIFY(dsAllocator_free(group->allocator, group));
	}
	else
		FT_Done_FreeType(group->library);
}

#endif // DS_OSS_TEXT
