/*
 * Copyright 2017-2025 Aaron Barany
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
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Sort.h>

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/CubicCurve.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>

#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/TextIcons.h>
#include <DeepSea/Text/Unicode.h>

#include <SheenBidi/SheenBidi.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_OUTLINE_H
#include <hb.h>
#include <hb-ft.h>

#ifdef HB_VERSION_ATLEAST
#define HAS_FONT_CHANGED HB_VERSION_ATLEAST(1, 6, 0)
#else
#define HAS_FONT_CHANGED 0
#endif

// Handle deprecated Harfbuzz function names.
#if HB_VERSION_MAJOR < 10 || (HB_VERSION_MAJOR == 10 && HB_VERSION_MINOR < 4)
#define hb_ft_font_get_ft_face hb_ft_font_get_face
#endif

struct dsFontFace
{
	dsHashTableNode node;
	char name[DS_MAX_FACE_NAME_LENGTH];
	dsAllocator* bufferAllocator;
	void* buffer;
	hb_font_t* font;
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

	dsText scratchText;

	uint32_t* scratchCodepoints;
	dsTextRange* scratchRanges;
	uint32_t scratchMaxCodepoints;
	uint32_t scratchMaxRanges;

	dsGlyph* scratchGlyphs;
	uint32_t scratchGlyphCount;
	uint32_t scratchMaxGlyphs;

	dsParagraphInfo* paragraphs;
	uint32_t maxParagraphs;

	dsRunInfo* runs;
	uint32_t maxRuns;

	uint32_t* codepointMapping;
	uint32_t maxCodepointMappingCount;

	uint32_t maxFaces;
	uint32_t faceCount;
	dsFontFace faces[];
};

#define CHORDAL_TOLERANCE 0.1f
#define EQUAL_EPSILON 1e-7f
#define FIXED_SCALE (1 << 6)
#define INV_FIXED_SCALE (1.0f/(float)FIXED_SCALE)

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
	return dsAllocator_reallocWithFallback((dsAllocator*)memory->user, block, curSize, newSize);
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

	size_t nameLength = strlen(name) + 1;
	if (nameLength > DS_MAX_FACE_NAME_LENGTH)
	{
		errno = EINVAL;
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

	DS_ASSERT(group->faceHashTable->list.length < group->maxFaces);
	hb_font_t* hbFont = hb_ft_font_create(ftFace, (hb_destroy_func_t)&FT_Done_Face);
	if (!hbFont)
		return NULL;

	dsFontFace* face = group->faces + group->faceCount++;
	memcpy(face->name, name, nameLength);
	face->bufferAllocator = NULL;
	face->buffer = NULL;
	face->font = hbFont;
	DS_VERIFY(dsHashTable_insert(group->faceHashTable, face->name, (dsHashTableNode*)face, NULL));
	return face;
}

static bool dsFaceGroup_loadFaceImpl(
	dsFaceGroup* group, dsAllocator* allocator, dsStream* stream, const char* name)
{
	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, allocator);
	if (!buffer)
		return false;

	DS_VERIFY(dsMutex_lock(group->mutex));
	FT_Open_Args args;
	args.flags = FT_OPEN_MEMORY;
	args.memory_base = (const FT_Byte*)buffer;
	args.memory_size = (FT_Long)size;
	FT_Face ftFace;
	if (setFontLoadErrno(FT_Open_Face(group->library, &args, 0, &ftFace)))
	{
		DS_VERIFY(dsMutex_unlock(group->mutex));
		if (dsAllocator_keepPointer(allocator))
			dsAllocator_free(allocator, buffer);
		return false;
	}

	dsFontFace* face = insertFace(group, name, ftFace);
	if (!face)
	{
		FT_Done_Face(ftFace);
		DS_VERIFY(dsMutex_unlock(group->mutex));
		if (dsAllocator_keepPointer(allocator))
			dsAllocator_free(allocator, buffer);
		return false;
	}

	if (dsAllocator_keepPointer(allocator))
	{
		face->bufferAllocator = allocator;
		face->buffer = buffer;
	}

	DS_VERIFY(dsMutex_unlock(group->mutex));
	return true;
}

static uint32_t* createCodepointMapping(dsFaceGroup* group, uint32_t length)
{
	uint32_t codepointMappingCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(group->scratchAllocator, group->codepointMapping,
			codepointMappingCount, group->maxCodepointMappingCount, length))
	{
		return NULL;
	}

	DS_ASSERT(codepointMappingCount == length);
	return group->codepointMapping;
}

static bool addGlyphPoint(dsGlyphGeometry* geometry, const dsVector2f* position)
{
	uint32_t curPointIndex = geometry->pointCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(geometry->allocator, geometry->points, geometry->pointCount,
			geometry->maxPoints, 1))
	{
		return false;
	}

	DS_ASSERT(geometry->loopCount > 0);
	dsGlyphLoop* loop = geometry->loops + geometry->loopCount - 1;
	++loop->pointCount;
	dsAlignedBox2_addPoint(loop->bounds, *position);

	dsGlyphPoint* point = geometry->points + curPointIndex;
	point->position = *position;
	return true;
}

static bool addGlyphLine(dsGlyphGeometry* geometry, const dsVector2f* end)
{
	DS_ASSERT(geometry->pointCount > 0);
	dsGlyphPoint* start = geometry->points + geometry->pointCount - 1;
	float distance = dsVector2f_dist(&start->position, end);
	if (distance < EQUAL_EPSILON)
		return true;

	return addGlyphPoint(geometry, end);
}

static int compareGlyphEdge(const void* left, const void* right)
{
	const dsOrderedGlyphEdge* leftEdge = (const dsOrderedGlyphEdge*)left;
	const dsOrderedGlyphEdge* rightEdge = (const dsOrderedGlyphEdge*)right;
	return dsCombineCmp(DS_CMP(leftEdge->minPoint.y, rightEdge->minPoint.y),
		DS_CMP(leftEdge->minPoint.x, rightEdge->minPoint.x));
}

static bool sortGlyphEdges(dsGlyphGeometry* geometry)
{
	// Reserve space for sorted edges. It might be less for invalid loops.
	uint32_t dummyEdgeCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(geometry->allocator, geometry->sortedEdges, dummyEdgeCount,
			geometry->maxEdges, geometry->pointCount))
	{
		return false;
	}

	uint32_t edgeIndex = 0;
	for (uint32_t i = 0; i < geometry->loopCount; ++i)
	{
		const dsGlyphLoop* loop = geometry->loops + i;
		if (loop->pointCount < 3)
			continue;

		for (uint32_t j = 0; j < loop->pointCount; ++j, ++edgeIndex)
		{
			const dsGlyphPoint* point = geometry->points + loop->firstPoint + j;
			const dsVector2f* firstPos = &point->position;
			const dsVector2f* secondPos = &point->nextPos;

			dsOrderedGlyphEdge* edge = geometry->sortedEdges + edgeIndex;
			if (firstPos->y < secondPos->y)
			{
				edge->minPoint = *firstPos;
				edge->maxPoint = *secondPos;
			}
			else
			{
				edge->minPoint = *secondPos;
				edge->maxPoint = *firstPos;
			}
		}
	}

	DS_ASSERT(edgeIndex <= geometry->pointCount);
	geometry->edgeCount = edgeIndex;
	qsort(geometry->sortedEdges, geometry->edgeCount, sizeof(dsOrderedGlyphEdge),
		&compareGlyphEdge);
	return true;
}

static bool endGlyphLoop(dsGlyphGeometry* geometry)
{
	if (geometry->loopCount == 0)
		return true;

	dsGlyphLoop* loop = geometry->loops + geometry->loopCount - 1;
	// Should at least be a triangle.
	if (loop->pointCount < 3)
		return true;

	// Remove duplicate ending point.
	DS_ASSERT(loop->firstPoint + loop->pointCount == geometry->pointCount);
	const dsGlyphPoint* firstPoint = geometry->points + loop->firstPoint;
	dsGlyphPoint* lastPoint = geometry->points + loop->firstPoint + loop->pointCount - 1;
	if (dsVector2f_epsilonEqual(&firstPoint->position, &lastPoint->position, EQUAL_EPSILON))
	{
		--loop->pointCount;
		--geometry->pointCount;
		--lastPoint;
	}

	// Should still at least be a triangle. Do second check here since it may or may not have
	// removed a point, and we need at least 2 points to perform the equality check.
	if (loop->pointCount < 3)
		return true;

	for (uint32_t i = 0; i < loop->pointCount; ++i)
	{
		dsGlyphPoint* point = geometry->points + loop->firstPoint + i;
		const dsGlyphPoint* nextPoint = geometry->points + loop->firstPoint +
			((i + 1) % loop->pointCount);

		point->nextPos = nextPoint->position;
		dsVector2_sub(point->edgeDir, nextPoint->position, point->position);
		point->edgeLength = dsVector2f_len(&point->edgeDir);
	}

	dsAlignedBox2_addBox(geometry->bounds, loop->bounds);
	return true;
}

static bool addGlyphBezierPoint(void* userData, const void* point, uint32_t axisCount, float t)
{
	DS_UNUSED(axisCount);
	if (t == 0.0f)
		return true;

	dsGlyphGeometry* geometry = (dsGlyphGeometry*)userData;
	return addGlyphLine(geometry, (const dsVector2f*)point);
}

static int glyphMoveTo(const FT_Vector* to, void* user)
{
	dsGlyphGeometry* geometry = (dsGlyphGeometry*)user;
	if (!endGlyphLoop(geometry))
	{
		if (errno == EPERM)
			return FT_Err_Invalid_Argument;
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	uint32_t loopIndex = geometry->loopCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(geometry->allocator, geometry->loops, geometry->loopCount,
			geometry->maxLoops, 1))
	{
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	dsGlyphLoop* loop = geometry->loops + loopIndex;
	loop->firstPoint = geometry->pointCount;
	loop->pointCount = 0;
	dsAlignedBox2f_makeInvalid(&loop->bounds);

	dsVector2f position = {{(float)to->x*INV_FIXED_SCALE, (float)-to->y*INV_FIXED_SCALE}};
	if (!addGlyphPoint(geometry, &position))
	{
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	return 0;
}

static int glyphLineTo(const FT_Vector* to, void* user)
{
	dsGlyphGeometry* geometry = (dsGlyphGeometry*)user;
	dsVector2f position = {{(float)to->x*INV_FIXED_SCALE, (float)-to->y*INV_FIXED_SCALE}};
	if (!addGlyphLine(geometry, &position))
	{
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	return 0;
}

static int glyphConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
	dsGlyphGeometry* geometry = (dsGlyphGeometry*)user;
	DS_ASSERT(geometry->pointCount > 0);

	const dsVector2f* p0 = &geometry->points[geometry->pointCount - 1].position;
	dsVector2f p1 = {{(float)control->x*INV_FIXED_SCALE, -(float)control->y*INV_FIXED_SCALE}};
	dsVector2f p2 = {{(float)to->x*INV_FIXED_SCALE, -(float)to->y*INV_FIXED_SCALE}};
	dsCubicCurvef curve;
	DS_VERIFY(dsCubicCurvef_initializeQuadratic(&curve, 2, p0, &p1, &p2));

	if (!dsCubicCurvef_tessellate(&curve, CHORDAL_TOLERANCE, 10, &addGlyphBezierPoint, geometry))
	{
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	return 0;
}

static int glyphCubicTo(
	const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
	dsGlyphGeometry* geometry = (dsGlyphGeometry*)user;
	DS_ASSERT(geometry->pointCount > 0);

	const dsVector2f* p0 = &geometry->points[geometry->pointCount - 1].position;
	dsVector2f p1 = {{(float)control1->x*INV_FIXED_SCALE, -(float)control1->y*INV_FIXED_SCALE}};
	dsVector2f p2 = {{(float)control2->x*INV_FIXED_SCALE, (float)-control2->y*INV_FIXED_SCALE}};
	dsVector2f p3 = {{(float)to->x*INV_FIXED_SCALE, -(float)to->y*INV_FIXED_SCALE}};
	dsCubicCurvef curve;
	DS_VERIFY(dsCubicCurvef_initializeBezier(&curve, 2, p0, &p1, &p2, &p3));

	if (!dsCubicCurvef_tessellate(&curve, CHORDAL_TOLERANCE, 10, &addGlyphBezierPoint, geometry))
	{
		DS_ASSERT(errno == ENOMEM);
		return FT_Err_Out_Of_Memory;
	}

	return 0;
}

static bool shapeFaceRange(const dsFont* font, uint32_t face, dsText* text, dsTextRange* range,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction)
{
	hb_font_t* hbFont = font->faces[face]->font;
	FT_Set_Pixel_Sizes(hb_ft_font_get_ft_face(hbFont), 0, font->glyphSize);
#if HAS_FONT_CHANGED
	hb_ft_font_changed(hbFont);
#else
	// This is the portion of hb_ft_font_changed() that we need to support older versions of
	// HarfBuzz.
	FT_Face ftFace = hb_ft_font_get_ft_face(hbFont);
	hb_font_set_scale(hbFont,
		(int)(((uint64_t)ftFace->size->metrics.x_scale*(uint64_t)ftFace->units_per_EM +
			(1u<<15)) >> 16),
		(int)(((uint64_t)ftFace->size->metrics.y_scale*(uint64_t)ftFace->units_per_EM +
			(1u<<15)) >> 16));
#endif

	hb_buffer_t* shapeBuffer = font->group->shapeBuffer;
	hb_buffer_add_utf32(shapeBuffer, text->characters, text->characterCount, start, count);
	if (direction == dsTextDirection_RightToLeft)
		hb_buffer_set_direction(shapeBuffer, HB_DIRECTION_RTL);
	else
		hb_buffer_set_direction(shapeBuffer, HB_DIRECTION_LTR);
	hb_buffer_set_script(shapeBuffer, (hb_script_t)dsFont_codepointScript(font, firstCodepoint));
	hb_buffer_set_language(shapeBuffer, hb_language_get_default());
	hb_shape(hbFont, shapeBuffer, NULL, 0);
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
	if (!dsFaceGroup_scratchGlyphs(font->group, glyphOffset + glyphCount))
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

	float scale = 1.0f/(float)(FIXED_SCALE*font->glyphSize);
	dsGlyph* glyphs = (dsGlyph*)(text->glyphs + glyphOffset);
	for (unsigned int i = 0; i < glyphCount; ++i)
	{
		dsGlyph* glyph = glyphs + i;
		const hb_glyph_info_t* glyphInfo = glyphInfos + i;
		const hb_glyph_position_t* curGlyphPos = glyphPos + i;
		glyph->glyphID = glyphInfo->codepoint;
		glyph->charIndex = glyphInfo->cluster;
		glyph->offset.x = (float)curGlyphPos->x_offset*scale;
		glyph->offset.y = -(float)curGlyphPos->y_offset*scale;
		// Special handling for newlines, since they are used in layout but will have an invalid
		// glyph.
		if (text->characters[glyph->charIndex] == '\n')
			glyph->advance = 0;
		else
			glyph->advance = (float)curGlyphPos->x_advance*scale;
		DS_ASSERT(curGlyphPos->y_advance == 0);
	}

	hb_buffer_reset(shapeBuffer);
	return true;
}

static bool shapeIconRange(const dsFont* font, dsText* text, dsTextRange* range,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction)
{
	DS_ASSERT(font->icons);
	uint32_t glyphOffset = text->glyphCount;
	if (!dsFaceGroup_scratchGlyphs(font->group, glyphOffset + count))
		return false;

	range->face = DS_ICON_FACE;
	range->firstChar = start;
	range->charCount = count;
	range->firstGlyph = glyphOffset;
	range->glyphCount = count;
	range->newlineCount = newlineCount;
	range->backward = direction == dsTextDirection_RightToLeft;

	dsGlyph* glyphs = (dsGlyph*)(text->glyphs + glyphOffset);
	if (range->backward)
	{
		// Reverse the order for shaping.
		uint32_t relativeIndex = count + start - 1;
		for (unsigned int i = 0; i < count; ++i)
		{
			dsGlyph* glyph = glyphs + i;
			uint32_t codepoint = text->characters[relativeIndex - i];
			const dsIconGlyph* icon = dsTextIcons_findIcon(font->icons, codepoint);
			if (!icon)
				return false;

			glyph->glyphID = codepoint;
			glyph->charIndex = start + i;
			glyph->offset.x = glyph->offset.y = 0.0f;
			glyph->advance = icon->advance;
		}
	}
	else
	{
		for (unsigned int i = 0; i < count; ++i)
		{
			dsGlyph* glyph = glyphs + i;
			uint32_t codepoint = text->characters[start + i];
			const dsIconGlyph* icon = dsTextIcons_findIcon(font->icons, codepoint);
			if (!icon)
				return false;

			glyph->glyphID = codepoint;
			glyph->charIndex = start + i;
			glyph->offset.x = glyph->offset.y = 0.0f;
			glyph->advance = icon->advance;
		}
	}

	return true;
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

uint32_t dsFontFace_getCodepointGlyph(const dsFontFace* face, uint32_t codepoint)
{
	FT_Face ftFace = hb_ft_font_get_ft_face(face->font);
	DS_ASSERT(ftFace);
	return FT_Get_Char_Index(ftFace, codepoint);
}

bool dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	dsFont* font)
{
	FT_Face ftFace = hb_ft_font_get_ft_face(face->font);
	DS_ASSERT(ftFace);
	FT_Set_Pixel_Sizes(ftFace, 0, font->glyphSize);
	FT_Load_Glyph(ftFace, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

	dsGlyphGeometry* geometry = &font->glyphGeometry;
	geometry->pointCount = 0;
	geometry->loopCount = 0;
	geometry->edgeCount = 0;
	dsAlignedBox2f_makeInvalid(&geometry->bounds);

	FT_Outline_Funcs outlineFuncs =
		{&glyphMoveTo, &glyphLineTo, &glyphConicTo, &glyphCubicTo, 0, 0};
	if (setFontLoadErrno(FT_Outline_Decompose(&ftFace->glyph->outline, &outlineFuncs, geometry)))
		return false;

	if (!endGlyphLoop(geometry) || !sortGlyphEdges(geometry))
		return false;

	*outBounds = geometry->bounds;
	return dsFont_writeGlyphToTexture(
		commandBuffer, texture, glyphIndex, font->glyphSize, font->texMultiplier, geometry);
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

dsRunInfo* dsFaceGroup_findBidiRuns(
	uint32_t* outCount, dsFaceGroup* group, const void* string, dsUnicodeType type)
{
	DS_PROFILE_FUNC_START();

	*outCount = DS_UNICODE_INVALID;
	if (!string)
	{
		*outCount = 0;
		DS_PROFILE_FUNC_RETURN(NULL);
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
		*outCount = 0;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	// Create a mapping between the characters and codepoints.
	uint32_t mappingSize = (uint32_t)sequence.stringLength + 1;
	uint32_t* codepointMapping = createCodepointMapping(group, mappingSize);
	if (!codepointMapping)
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t codepointIndex = 0;
	SBUInteger index = 0;
	do
	{
		SBUInteger prevIndex = index;
		uint32_t codepoint = SBCodepointSequenceGetCodepointAt(&sequence, &index);
		if (codepoint == SBCodepointInvalid)
		{
			codepointMapping[prevIndex] = codepointIndex;
			break;
		}

		for (SBUInteger i = prevIndex; i < index; ++i)
			codepointMapping[i] = codepointIndex;
		++codepointIndex;
	}
	while (true);

	SBAlgorithmRef algorithm = SBAlgorithmCreate(&sequence);
	if (!algorithm)
	{
		*outCount = DS_UNICODE_INVALID;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	// Count the paragraphs to allocate the array.
	SBUInteger offset = 0;
	unsigned int paragraphCount = 0;
	while (offset < sequence.stringLength)
	{
		SBUInteger length = 0;
		SBAlgorithmGetParagraphBoundary(
			algorithm, offset, sequence.stringLength - offset, &length, NULL);
		++paragraphCount;
		offset += length;
	}

	uint32_t tempArrayCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(group->scratchAllocator, group->paragraphs, tempArrayCount,
			group->maxParagraphs, paragraphCount))
	{
		SBAlgorithmRelease(algorithm);
		*outCount = DS_UNICODE_INVALID;
		DS_PROFILE_FUNC_RETURN(NULL);
	}
	DS_ASSERT(tempArrayCount == paragraphCount);

	// Create the paragraphs and lines.
	*outCount = 0;
	memset(group->paragraphs, 0, paragraphCount*sizeof(dsParagraphInfo));
	offset = 0;
	for (unsigned int i = 0; i < paragraphCount; ++i)
	{
		SBUInteger length = 0, separatorLength = 0;
		SBAlgorithmGetParagraphBoundary(
			algorithm, offset, sequence.stringLength - offset, &length, &separatorLength);
		dsParagraphInfo* paragraph = group->paragraphs + i;
		paragraph->paragraph = SBAlgorithmCreateParagraph(
			algorithm, offset, length, SBLevelDefaultLTR);
		if (!paragraph->paragraph)
		{
			++offset;
			paragraph->line = NULL;
			continue;
		}

		SBUInteger lineLength = length - separatorLength;
		if (lineLength > 0)
		{
			paragraph->line = SBParagraphCreateLine(paragraph->paragraph,
				offset, length - separatorLength);
			if (!paragraph->line)
			{
				SBParagraphRelease(paragraph->paragraph);
				for (unsigned int j = 0; j < i; ++j)
				{
					dsParagraphInfo* otherParagraph = group->paragraphs + j;
					SBLineRelease(otherParagraph->line);
					SBParagraphRelease(otherParagraph->paragraph);
				}
				SBAlgorithmRelease(algorithm);
				*outCount = DS_UNICODE_INVALID;
				DS_PROFILE_FUNC_RETURN(NULL);
			}
			*outCount += (uint32_t)SBLineGetRunCount(paragraph->line);
		}

		offset += length;
	}

	// Create the runs.
	tempArrayCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(
			group->scratchAllocator, group->runs, tempArrayCount, group->maxRuns, *outCount))
	{
		for (unsigned int i = 0; i < paragraphCount; ++i)
		{
			dsParagraphInfo* paragraph = group->paragraphs + i;
			if (!paragraph->paragraph)
				continue;

			SBLineRelease(paragraph->line);
			SBParagraphRelease(paragraph->paragraph);
		}
		SBAlgorithmRelease(algorithm);
		*outCount = DS_UNICODE_INVALID;
		DS_PROFILE_FUNC_RETURN(NULL);
	}
	DS_ASSERT(tempArrayCount == *outCount);

	uint32_t run = 0;
	for (uint32_t i = 0; i < paragraphCount; ++i)
	{
		dsParagraphInfo* paragraph = group->paragraphs + i;
		if (!paragraph->line)
		{
			// An empty paragraph indicates that there was an empty line. Add it to the last run if
			// it exists.
			if (run > 0)
				++group->runs[run - 1].newlineCount;
			continue;
		}

		SBUInteger curCount = SBLineGetRunCount(group->paragraphs[i].line);
		const SBRun* runArray = SBLineGetRunsPtr(group->paragraphs[i].line);
		for (SBUInteger j = 0; j < curCount; ++j, ++run)
		{
			// Convert from character run to codepoint run.
			const SBRun* curRun = runArray + j;
			dsRunInfo* groupRun = group->runs + run;
			DS_ASSERT(curRun->offset < mappingSize);
			DS_ASSERT(curRun->offset + curRun->length < mappingSize);
			groupRun->start = codepointMapping[curRun->offset];
			uint32_t end = codepointMapping[curRun->offset + curRun->length];
			groupRun->count = end - groupRun->start;

			// Odd levels indicate right to left text.
			groupRun->direction = curRun->level & 1 ? dsTextDirection_RightToLeft :
				dsTextDirection_LeftToRight;

			// Once we reach the end of the paragraph, mark as having a newline if more paragraphs
			// remain.
			groupRun->newlineCount = j == curCount - 1 && i != paragraphCount - 1;
		}
	}
	DS_ASSERT(run == *outCount);

	// Free temporary objects.
	for (unsigned int i = 0; i < paragraphCount; ++i)
	{
		dsParagraphInfo* paragraph = group->paragraphs + i;
		if (!paragraph->paragraph)
			continue;

		SBLineRelease(paragraph->line);
		SBParagraphRelease(paragraph->paragraph);
	}
	SBAlgorithmRelease(algorithm);

	DS_PROFILE_FUNC_RETURN(group->runs);
}

dsText* dsFaceGroup_scratchText(dsFaceGroup* group, uint32_t length)
{
	group->scratchText.characterCount = 0;
	group->scratchText.characters = NULL;
	group->scratchText.charMappings = NULL;
	group->scratchText.rangeCount = 0;
	group->scratchText.ranges = NULL;
	group->scratchText.glyphCount = 0;
	group->scratchText.glyphs = NULL;
	group->scratchGlyphCount = 0;

	if (length == 0)
		return &group->scratchText;

	if (!DS_RESIZEABLE_ARRAY_ADD(group->scratchAllocator, group->scratchCodepoints,
			group->scratchText.characterCount, group->scratchMaxCodepoints, length))
	{
		return NULL;
	}

	group->scratchText.characters = group->scratchCodepoints;
	return &group->scratchText;
}

bool dsFaceGroup_scratchRanges(dsFaceGroup* group, uint32_t rangeCount)
{
	if (rangeCount == 0)
		return true;

	group->scratchText.rangeCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(group->scratchAllocator, group->scratchRanges,
			group->scratchText.rangeCount, group->scratchMaxRanges, rangeCount))
	{
		return false;
	}

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

bool dsFaceGroup_isScriptUnique(uint32_t script)
{
	return script != HB_SCRIPT_INHERITED && script != HB_SCRIPT_UNKNOWN;
}

bool dsFaceGroup_isScriptCommon(uint32_t script)
{
	return script == HB_SCRIPT_COMMON;
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

bool dsFaceGroup_isScriptBoundary(
	uint32_t script, bool scriptUnique, bool hasLastScript, uint32_t lastScript)
{
	bool equal = !hasLastScript || dsFaceGroup_areScriptsEqual(script, lastScript);
	// When an invalid script (i.e. icon), treat non-unique scripts as a boundary.
	return (scriptUnique && !equal) ||
		(hasLastScript && !scriptUnique && lastScript == HB_SCRIPT_INVALID);
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
	return DS_ALIGNED_SIZE(sizeof(dsFaceGroup)) + DS_ALIGNED_SIZE(sizeof(dsFontFace)*maxFaces) +
		dsMutex_fullAllocSize() + dsHashTable_fullAllocSize(dsHashTable_tableSize(maxFaces));
}

dsFaceGroup* dsFaceGroup_create(
	dsAllocator* allocator, dsAllocator* scratchAllocator, uint32_t maxFaces)
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

	size_t hashTableSize = dsHashTable_tableSize(maxFaces);
	faceGroup->allocator = dsAllocator_keepPointer(allocator);
	faceGroup->scratchAllocator = scratchAllocator;
	faceGroup->faceHashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(hashTableSize));
	DS_ASSERT(faceGroup->faceHashTable);
	DS_VERIFY(dsHashTable_initialize(faceGroup->faceHashTable, hashTableSize, &dsHashString,
		dsHashStringEqual));

	faceGroup->mutex = dsMutex_create((dsAllocator*)&bufferAlloc, "Face Group");
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

	memset(&faceGroup->scratchText, 0, sizeof(dsText));
	faceGroup->scratchText.allocator = scratchAllocator;

	faceGroup->scratchCodepoints = NULL;
	faceGroup->scratchRanges = NULL;
	faceGroup->scratchMaxCodepoints = 0;
	faceGroup->scratchMaxRanges = 0;

	faceGroup->scratchGlyphs = NULL;
	faceGroup->scratchGlyphCount = 0;
	faceGroup->scratchMaxGlyphs = 0;

	faceGroup->paragraphs = NULL;
	faceGroup->maxParagraphs = 0;

	faceGroup->runs = NULL;
	faceGroup->maxRuns = 0;

	faceGroup->codepointMapping = NULL;
	faceGroup->maxCodepointMappingCount = 0;

	faceGroup->maxFaces = maxFaces;
	faceGroup->faceCount = 0;
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

bool dsFaceGroup_loadFaceResource(dsFaceGroup* group, dsAllocator* allocator,
	dsFileResourceType type, const char* fileName, const char* name)
{
	if (dsResourceStream_isFile(type))
	{
		char path[DS_PATH_MAX];
		if (!dsResourceStream_getPath(path, sizeof(path), type, fileName))
			return false;
		return dsFaceGroup_loadFaceFile(group, path, name);
	}

	if (!group || !allocator || !fileName || !name)
	{
		errno = EINVAL;
		return false;
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, fileName, "rb"))
	{
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Couldn't open font face file '%s'.", fileName);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool retVal = dsFaceGroup_loadFaceImpl(group, allocator, (dsStream*)&stream, name);
	dsResourceStream_close(&stream);
	return retVal;
}

bool dsFaceGroup_loadFaceArchive(dsFaceGroup* group, dsAllocator* allocator,
	const dsFileArchive* archive, const char* fileName, const char* name)
{
	if (!group || !allocator || !archive || !fileName || !name)
	{
		errno = EINVAL;
		return false;
	}

	dsStream* stream = dsFileArchive_openFile(archive, fileName);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Couldn't open font face file '%s'.", fileName);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool retVal = dsFaceGroup_loadFaceImpl(group, allocator, stream, name);
	dsStream_close(stream);
	return retVal;
}

bool dsFaceGroup_loadFaceStream(dsFaceGroup* group, dsAllocator* allocator,
	dsStream* stream, const char* name)
{
	if (!group || !allocator || !stream || !name)
	{
		errno = EINVAL;
		return false;
	}

	return dsFaceGroup_loadFaceImpl(group, allocator, stream, name);
}

bool dsFaceGroup_loadFaceData(dsFaceGroup* group, dsAllocator* allocator, const void* buffer,
	size_t size, const char* name)
{
	if (!group || !buffer || size == 0 || !name)
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
		if (loadBuffer != buffer && dsAllocator_keepPointer(allocator))
			dsAllocator_free(allocator, loadBuffer);
		return false;
	}

	dsFontFace* face = insertFace(group, name, ftFace);
	if (!face)
	{
		FT_Done_Face(ftFace);
		DS_VERIFY(dsMutex_unlock(group->mutex));
		if (loadBuffer != buffer && dsAllocator_keepPointer(allocator))
			dsAllocator_free(allocator, loadBuffer);
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

void dsFaceGroup_destroy(dsFaceGroup* group)
{
	if (!group)
		return;

	for (dsListNode* node = group->faceHashTable->list.head; node; node = node->next)
	{
		dsFontFace* face = (dsFontFace*)node;
		hb_font_destroy(face->font);
		if (face->bufferAllocator)
			DS_VERIFY(dsAllocator_free(face->bufferAllocator, face->buffer));
	}
	dsMutex_destroy(group->mutex);
	hb_buffer_destroy(group->shapeBuffer);

	FT_Done_Library(group->library);

	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->scratchCodepoints));
	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->scratchRanges));
	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->scratchGlyphs));
	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->paragraphs));
	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->runs));
	DS_VERIFY(dsAllocator_free(group->scratchAllocator, group->codepointMapping));
	DS_VERIFY(dsAllocator_free(group->allocator, group));
}

uint32_t dsFont_codepointScript(const dsFont* font, uint32_t codepoint)
{
	// Override whitepsace.
	if (dsIsSpace(codepoint))
		return HB_SCRIPT_INHERITED;

	if (dsTextIcons_isCodepointValid(font->icons, codepoint))
		return HB_SCRIPT_INVALID;

	return hb_unicode_script(font->group->unicode, codepoint);
}

uint32_t dsFont_findFaceForCodepoint(const dsFont* font, uint32_t codepoint)
{
	if (dsTextIcons_isCodepointValid(font->icons, codepoint))
		return DS_ICON_FACE;

	for (uint32_t i = 0; i < font->faceCount; ++i)
	{
		if (FT_Get_Char_Index(hb_ft_font_get_ft_face(font->faces[i]->font), codepoint))
			return i;
	}

	return 0;
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

	uint32_t face = dsFont_findFaceForCodepoint(font, firstCodepoint);
	if (face == DS_ICON_FACE)
	{
		return shapeIconRange(
			font, text, range, firstCodepoint, start, count, newlineCount, direction);
	}
	return shapeFaceRange(
		font, face, text, range, firstCodepoint, start, count, newlineCount, direction);
}
