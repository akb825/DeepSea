/*
 * Copyright 2017-2023 Aaron Barany
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

#include <DeepSea/Text/Font.h>

#include "FontImpl.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/List.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Unicode.h>
#include <float.h>
#include <string.h>

#define INTERSECT_EPSILON 1e-5f

typedef uint32_t (*NextCodepointFunction)(const void* string, uint32_t* index);

typedef enum PolygonResult
{
	PolygonResult_Inside,
	PolygonResult_Outside,
	PolygonResult_OnEdge
} PolygonResult;

static float distanceToLine(const dsVector2f* point, const dsVector2f* start, const dsVector2f* end,
	const dsVector2f* lineDir, float lineLength)
{
	dsVector2f pointDir;
	dsVector2_sub(pointDir, *point, *start);

	float projDist2 = dsVector2_dot(pointDir, *lineDir);
	if (projDist2 < 0.0f)
		return dsVector2f_len(&pointDir);

	if (projDist2 >= dsPow2(lineLength))
		return dsVector2f_dist(point, end);

	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	return fabsf(lineDir->y*point->x - lineDir->x*point->y + end->x*start->y - end->y*start->x)/
		lineLength;
}

static PolygonResult intersectScanline(const dsVector2f* point, const dsVector2f* from,
	const dsVector2f* to)
{
	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	// Optimized for a ray in the direction (1, 0).
	float slightlyLeftX = point->x - INTERSECT_EPSILON;
	if (from->x < slightlyLeftX && to->x < slightlyLeftX)
		return PolygonResult_Outside;

	// Check if bottom Y is equal. This is inside if point is to the left of the edge, on edge if
	// point is on the right and top Y is also equal, othewise outside. If parallel and the point
	// is to the left of the point, consider outside since it enters and leaves the edge.
	float slightlyRightX = point->x + INTERSECT_EPSILON;
	if (dsEpsilonEqualf(point->y, from->y, INTERSECT_EPSILON))
	{
		if (dsEpsilonEqualf(point->x, from->x, INTERSECT_EPSILON))
			return PolygonResult_OnEdge;

		if (dsEpsilonEqualf(point->y, to->y, INTERSECT_EPSILON))
		{
			// Check against both from and to points for parallel lines since we can't be sure of
			// the ordering of X values, since the Y values could be off by an epsilon. We only need
			// to check for the left boundary since we know that the point isn't to the right of
			// both edges from the first check in the function.
			if (from->x < slightlyRightX || to->x < slightlyRightX)
				return PolygonResult_OnEdge;
			return PolygonResult_Outside;
		}

		if (from->x < slightlyLeftX)
			return PolygonResult_Outside;
		return PolygonResult_Inside;
	}

	// Since we explicitly check for the bottm Y for intersection, don't intersect if top Y is equal
	// to avoid double intersection in neighboring edge. An exception is made for when the point is
	// exactly equal.
	if (dsEpsilonEqualf(point->y, to->y, INTERSECT_EPSILON))
	{
		if (dsEpsilonEqualf(point->x, to->x, INTERSECT_EPSILON))
			return PolygonResult_OnEdge;
		return PolygonResult_Outside;
	}

	// Parallel lines. Above code already handled if parallel lines intersected.
	if (dsEpsilonEqualf(from->y, to->y, INTERSECT_EPSILON))
		return PolygonResult_Outside;

	// Only care about the X coordinate, we already know the Y coordinate of the intersection.
	// Take advantage of edge ordering and use a lerp to reduce precision loss.
	float t = (point->y - from->y)/(to->y - from->y);
	float intersectX = dsLerp(from->x, to->x, t);

	if (intersectX < slightlyLeftX)
		return PolygonResult_Outside;
	else if (intersectX <= slightlyRightX)
		return PolygonResult_OnEdge;
	return PolygonResult_Inside;
}

static PolygonResult pointInsideGlyph(const dsVector2f* point, const dsGlyphGeometry* geometry)
{
	if (!dsAlignedBox2_containsPoint(geometry->bounds, *point))
		return PolygonResult_Outside;

	// Use scanlines to determine if the point is inside of the polygon loop.
	// https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
	uint32_t intersectCount = 0;

	// Start at the bottom, end once the min point is above the current point. Skip any edges where
	// the max point is below. (since it won't hit a horizontal line originating at the current
	// point)
	for (uint32_t i = 0; i < geometry->edgeCount; ++i)
	{
		const dsOrderedGlyphEdge* edge = geometry->sortedEdges + i;
		if (edge->maxPoint.y + INTERSECT_EPSILON < point->y)
			continue;
		else if (edge->minPoint.y > point->y + INTERSECT_EPSILON)
			break;

		// Shoot a ray to the right to see if it intersects.
		PolygonResult result = intersectScanline(point, &edge->minPoint, &edge->maxPoint);
		if (result == PolygonResult_OnEdge)
			return PolygonResult_OnEdge;
		else if (result == PolygonResult_Inside)
			++intersectCount;
	}

	if (intersectCount & 1)
		return PolygonResult_Inside;
	return PolygonResult_Outside;
}

static float computeSignedDistance(const dsVector2f* pos, const dsGlyphGeometry* geometry)
{
	// Find the closest point.
	PolygonResult insideResult = pointInsideGlyph(pos, geometry);
	if (insideResult == PolygonResult_OnEdge)
		return 0.0f;

	float inside = insideResult == PolygonResult_Inside ? 1.0f : -1.0f;
	float curDistance = FLT_MAX;
	for (uint32_t i = 0; i < geometry->loopCount; ++i)
	{
		const dsGlyphLoop* loop = geometry->loops + i;
		if (loop->pointCount < 3)
			continue;

		if (curDistance != FLT_MAX && !dsAlignedBox2_containsPoint(geometry->loops[i].bounds, *pos))
		{
			dsVector2f boxPoint;
			dsAlignedBox2_closestPoint(boxPoint, geometry->loops[i].bounds, *pos);
			if (dsVector2_dist2(*pos, boxPoint) > dsPow2(curDistance))
				continue;
		}

		for (uint32_t j = 0; j < loop->pointCount; ++j)
		{
			const dsGlyphPoint* curPoint = geometry->points + loop->firstPoint + j;
			float distance = distanceToLine(pos, &curPoint->position, &curPoint->nextPos,
				&curPoint->edgeDir, curPoint->edgeLength);
			if (distance < curDistance)
				curDistance = distance;
		}
	}

	return curDistance*inside;
}

static bool preloadGlyphs(dsFont* font, dsCommandBuffer* commandBuffer, const void* string,
	NextCodepointFunction nextCodepointFunc)
{
	DS_PROFILE_FUNC_START();
	if (!font || !commandBuffer || !string)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsFaceGroup_lock(font->group);

	uint32_t index = 0;
	do
	{
		uint32_t codepoint = nextCodepointFunc(string, &index);
		if (codepoint == DS_UNICODE_END)
			break;
		else if (codepoint == DS_UNICODE_INVALID)
		{
			errno = EINVAL;
			dsFaceGroup_unlock(font->group);
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (dsIsSpace(codepoint))
			break;

		uint32_t face = dsFont_findFaceForCodepoint(font, codepoint);
		uint32_t glyph = dsFontFace_getCodepointGlyph(font->faces[face], codepoint);
		if (!dsFont_getGlyphInfo(font, commandBuffer, face, glyph))
		{
			dsFaceGroup_unlock(font->group);
			DS_PROFILE_FUNC_RETURN(false);
		}
	} while (true);

	dsFaceGroup_unlock(font->group);
	DS_PROFILE_FUNC_RETURN(true);
}

dsGlyphInfo* dsFont_getGlyphInfo(
	dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face, uint32_t glyph)
{
	dsGlyphKey key = {face, glyph};
	dsGlyphInfo* glyphInfo = (dsGlyphInfo*)dsHashTable_find(&font->glyphTable.hashTable, &key);
	if (glyphInfo)
	{
		// Move to the end of the list to consider the most recently used.
		DS_VERIFY(dsList_remove(&font->glyphTable.hashTable.list, (dsListNode*)glyphInfo));
		DS_VERIFY(dsList_append(&font->glyphTable.hashTable.list, (dsListNode*)glyphInfo));
		return glyphInfo;
	}

	if (font->usedGlyphCount < font->cacheSize)
	{
		glyphInfo = font->glyphPool + font->usedGlyphCount++;
		glyphInfo->key = key;
		DS_VERIFY(dsHashTable_insert(&font->glyphTable.hashTable, &glyphInfo->key,
			(dsHashTableNode*)glyphInfo, NULL));
	}
	else
	{
		// Re-purpose the last recently used glyph slot at the front of the list.
		// This could cause incorrect glyphs to be used if > 1365 glyphs are drawn at once, but this
		// should be incredibly unlikely.
		glyphInfo = (dsGlyphInfo*)font->glyphTable.hashTable.list.head;
		DS_VERIFY(dsHashTable_remove(&font->glyphTable.hashTable, &glyphInfo->key) ==
			(dsHashTableNode*)glyphInfo);

		glyphInfo->key = key;
		DS_VERIFY(dsHashTable_insert(&font->glyphTable.hashTable, &glyphInfo->key,
			(dsHashTableNode*)glyphInfo, NULL));
	}

	dsFontFace_cacheGlyph(&glyphInfo->glyphBounds, font->faces[face], commandBuffer, font->texture,
		glyph, dsFont_getGlyphIndex(font, glyphInfo), font);
	return glyphInfo;
}

uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph)
{
	return (uint32_t)(size_t)(glyph - font->glyphPool);
}

bool dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, uint32_t texMultiplier,
	const dsGlyphGeometry* geometry)
{
	float windowSize = (float)(glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE);
	dsVector2f windowSize2f = {{windowSize, windowSize}};
	dsAlignedBox2f paddedBounds;
	dsVector2_sub(paddedBounds.min, geometry->bounds.min, windowSize2f);
	dsVector2_add(paddedBounds.max, geometry->bounds.max, windowSize2f);

	dsVector2f size;
	dsAlignedBox2_extents(size, paddedBounds);

	// Scale down if needed, but not up.
	float maxSize = (float)(glyphSize - 1);
	float scaleX = 1.0f;
	if (size.x > maxSize)
		scaleX = size.x/maxSize;

	float scaleY = 1.0f;
	if (size.y > maxSize)
		scaleY = size.y/maxSize;

	// Compute the signed distance field into the final glyph texture.
	DS_ASSERT(glyphSize <= DS_VERY_HIGH_SIZE);
	uint8_t textureData[DS_VERY_HIGH_SIZE*DS_VERY_HIGH_SIZE];
	memset(textureData, 0, glyphSize*glyphSize);
	for (uint32_t y = 0; y < glyphSize; ++y)
	{
		float glyphY = (float)y*scaleY + paddedBounds.min.y;
		if (glyphY > paddedBounds.max.y)
			break;

		for (uint32_t x = 0; x < glyphSize; ++x)
		{
			dsVector2f glyphPos = {{(float)x*scaleX + paddedBounds.min.x, glyphY}};
			if (glyphPos.x > paddedBounds.max.x)
				break;

			float distance = computeSignedDistance(&glyphPos, geometry);
			distance = dsClamp(distance, -windowSize, windowSize);
			textureData[y*glyphSize + x] = (uint8_t)(((distance/windowSize)*0.5f + 0.5f)*255.0f);
		}
	}

	dsTexturePosition texturePos;
	dsFont_getGlyphTexturePos(&texturePos, glyphIndex, glyphSize, texMultiplier);
	return dsTexture_copyData(texture, commandBuffer, &texturePos, glyphSize, glyphSize, 1,
		textureData, glyphSize*glyphSize);
}

void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex, uint32_t glyphSize,
	uint32_t texMultiplier)
{
	outPos->face = dsCubeFace_None;
	outPos->depth = 0;

	static const uint32_t smallLimits[DS_SMALL_CACHE_TEX_MIP_LEVELS] =
	{
		dsPow2(DS_SMALL_CACHE_TEX_MULTIPLIER),
		dsPow2(DS_SMALL_CACHE_TEX_MULTIPLIER/2),
		dsPow2(DS_SMALL_CACHE_TEX_MULTIPLIER/4),
		dsPow2(DS_SMALL_CACHE_TEX_MULTIPLIER/8),
		dsPow2(DS_SMALL_CACHE_TEX_MULTIPLIER/16)
	};

	static const uint32_t largeLimits[DS_LARGE_CACHE_TEX_MIP_LEVELS] =
	{
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER),
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER/2),
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER/4),
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER/8),
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER/16),
		dsPow2(DS_LARGE_CACHE_TEX_MULTIPLIER/32)
	};

	DS_ASSERT(texMultiplier == DS_SMALL_CACHE_TEX_MULTIPLIER ||
		texMultiplier == DS_LARGE_CACHE_TEX_MULTIPLIER);
	const uint32_t* limits;
	uint32_t mipLevels;
	if (texMultiplier == DS_SMALL_CACHE_TEX_MULTIPLIER)
	{
		limits = smallLimits;
		mipLevels = DS_SMALL_CACHE_TEX_MIP_LEVELS;
	}
	else
	{
		limits = largeLimits;
		mipLevels = DS_LARGE_CACHE_TEX_MIP_LEVELS;
	}

	uint32_t prevLimit = 0;
	for (uint32_t i = 0; i < mipLevels; ++i)
	{
		uint32_t curLimit = prevLimit + limits[i];
		if (glyphIndex < curLimit)
		{
			uint32_t index = glyphIndex - prevLimit;
			outPos->mipLevel = i;
			outPos->x = index%(texMultiplier >> i)*glyphSize;
			outPos->y = index/(texMultiplier >> i)*glyphSize;
			return;
		}

		prevLimit = curLimit;
	}
	DS_ASSERT(false);
}

void dsFont_getGlyphTextureBounds(dsAlignedBox2f* outBounds, const dsTexturePosition* texturePos,
	const dsVector2f* glyphBoundsSize, uint32_t glyphSize, uint32_t texMultiplier)
{
	uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
	float levelSize = 1.0f/(float)(texMultiplier*glyphSize >> texturePos->mipLevel);
	outBounds->min.x = ((float)texturePos->x + 0.5f)*levelSize;
	outBounds->min.y = ((float)texturePos->y + 0.5f)*levelSize;

	dsVector2f offset = {{glyphBoundsSize->x + (float)windowSize*2.0f,
		glyphBoundsSize->y + (float)windowSize*2.0f}};
	offset.x = dsMin(offset.x, (float)(glyphSize - 1));
	offset.y = dsMin(offset.y, (float)(glyphSize - 1));

	dsVector2f levelSize2 = {{levelSize, levelSize}};
	dsVector2_mul(offset, offset, levelSize2);
	dsVector2_add(outBounds->max, outBounds->min, offset);
}

uint8_t dsFont_sizeForQuality(dsTextQuality quality)
{
	switch (quality)
	{
		case dsTextQuality_Low:
			return DS_LOW_SIZE;
		case dsTextQuality_High:
			return DS_HIGH_SIZE;
		case dsTextQuality_VeryHigh:
			return DS_VERY_HIGH_SIZE;
		case dsTextQuality_Medium:
		default:
			return DS_MEDIUM_SIZE;
	}
}

dsFont* dsFont_create(dsFaceGroup* group, dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* const* faceNames, uint32_t faceCount, dsTextQuality quality,
	dsTextCache cacheSize)
{
	if (!group || !resourceManager || (!allocator && !dsFaceGroup_getAllocator(group)) ||
		!faceNames || faceCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFaceGroup_lock(group);
	for (uint32_t i = 0; i < faceCount; ++i)
	{
		if (!faceNames[i])
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Empty face name.");
			dsFaceGroup_unlock(group);
			return NULL;
		}

		dsFontFace* face = dsFaceGroup_findFace(group, faceNames[i]);
		if (!face)
		{
			DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Face '%s' not found.", faceNames[i]);
			errno = ENOTFOUND;
			dsFaceGroup_unlock(group);
			return NULL;
		}
	}

	if (!allocator)
		allocator = dsFaceGroup_getAllocator(group);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsFont)) +
		DS_ALIGNED_SIZE(sizeof(dsFontFace*)*faceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		dsFaceGroup_unlock(group);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsFont* font = DS_ALLOCATE_OBJECT(&bufferAlloc, dsFont);
	DS_ASSERT(font);
	font->allocator = dsAllocator_keepPointer(allocator);
	font->group = group;
	font->faces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFontFace*, faceCount);
	DS_ASSERT(font->faces);
	for (uint32_t i = 0; i < faceCount; ++i)
	{
		font->faces[i] = dsFaceGroup_findFace(group, faceNames[i]);
		DS_ASSERT(font->faces[i]);
	}

	font->faceCount = faceCount;
	font->glyphSize = dsFont_sizeForQuality(quality);
	font->quality = quality;
	uint32_t mipLevels;
	if (cacheSize == dsTextCache_Small)
	{
		font->cacheSize = DS_SMALL_CACHE_GLYPH_SLOTS;
		font->texMultiplier = DS_SMALL_CACHE_TEX_MULTIPLIER;
		mipLevels = DS_SMALL_CACHE_TEX_MIP_LEVELS;
	}
	else
	{
		font->cacheSize = DS_LARGE_CACHE_GLYPH_SLOTS;
		font->texMultiplier = DS_LARGE_CACHE_TEX_MULTIPLIER;
		mipLevels = DS_SMALL_CACHE_TEX_MIP_LEVELS;
	}
	font->usedGlyphCount = 0;

	dsGlyphGeometry* geometry = &font->glyphGeometry;
	memset(geometry, 0, sizeof(dsGlyphGeometry));
	geometry->allocator = dsFaceGroup_getScratchAllocator(font->group);

	_Static_assert(sizeof(dsGlyphKey) == sizeof(uint64_t), "Unexpected glyph key size.");
	DS_VERIFY(dsHashTable_initialize(&font->glyphTable.hashTable, DS_TABLE_SIZE,
		&dsHash64, &dsHash64Equal));

	uint32_t texSize = font->glyphSize*font->texMultiplier;
	if (!resourceManager->hasArbitraryMipmapping)
		mipLevels = DS_ALL_MIP_LEVELS;
	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm),
		dsTextureDim_2D, texSize, texSize, 0, mipLevels, 0};
	font->texture = dsTexture_create(resourceManager, allocator,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Dynamic, &texInfo, NULL, 0);
	if (!font->texture)
	{
		if (font->allocator)
			dsAllocator_free(font->allocator, font);
		dsFaceGroup_unlock(group);
		return NULL;
	}

	dsFaceGroup_unlock(group);
	return font;
}

dsAllocator* dsFont_getAllocator(const dsFont* font)
{
	if (!font)
		return NULL;

	return font->allocator;
}

const dsFaceGroup* dsFont_getFaceGroup(const dsFont* font)
{
	if (!font)
		return NULL;

	return font->group;
}

uint32_t dsFont_getFaceCount(const dsFont* font)
{
	if (!font)
		return 0;

	return font->faceCount;
}

const char* dsFont_getFaceName(const dsFont* font, uint32_t face)
{
	if (!font || face >= font->faceCount)
		return NULL;

	return dsFontFace_getName(font->faces[face]);
}

dsTextQuality dsFont_getTextQuality(const dsFont* font)
{
	if (!font)
		return dsTextQuality_Medium;

	return font->quality;
}

dsTexture* dsFont_getTexture(const dsFont* font)
{
	if (!font)
		return NULL;

	return font->texture;
}

bool dsFont_applyHintingAndAntiAliasing(
	const dsFont* font, dsTextStyle* style, float pixelScale, float fuziness)
{
	if (!font || !style)
	{
		errno = EINVAL;
		return false;
	}

	float hintingStart, hintingEnd;
	float smallEmbolding, largeEmbolding;
	float antiAliasFactor;
	if (font->quality == dsTextQuality_Low)
	{
		hintingStart = 9.0f;
		hintingEnd = 32.0f;
		smallEmbolding = 0.1f;
		largeEmbolding = 0.05f;
	}
	else
	{
		hintingStart = 9.0f;
		hintingEnd = 32.0f;
		smallEmbolding = 0.1f;
		largeEmbolding = 0.0f;
	}
	antiAliasFactor = 1.5f*fuziness;

	float pixels = pixelScale*style->size;
	float size = dsClamp(pixels, hintingStart, hintingEnd);
	float t = (size - hintingStart)/(hintingEnd - hintingStart);
	float embolding = dsLerp(smallEmbolding, largeEmbolding, t);
	style->embolden += embolding;
	if (style->outlineThickness > 0.0f)
	{
		style->outlinePosition += embolding*0.5f;
		style->outlineThickness += embolding*0.5f;
	}

	t = 1.0f/pixels;
	style->antiAlias = t*antiAliasFactor;
	return true;
}

bool dsFont_preloadGlyphs(
	dsFont* font, dsCommandBuffer* commandBuffer, const void* string, dsUnicodeType type)
{
	if (!font || !commandBuffer || !string)
	{
		errno = EINVAL;
		return false;
	}

	NextCodepointFunction nextCodepoint;
	switch (type)
	{
		case dsUnicodeType_UTF8:
			nextCodepoint = (NextCodepointFunction)&dsUTF8_nextCodepoint;
			break;
		case dsUnicodeType_UTF16:
			nextCodepoint = (NextCodepointFunction)&dsUTF16_nextCodepoint;
			break;
		case dsUnicodeType_UTF32:
			nextCodepoint = (NextCodepointFunction)&dsUTF32_nextCodepoint;
			break;
		default:
			errno = EINVAL;
			return false;
	}

	return preloadGlyphs(font, commandBuffer, string, nextCodepoint);
}

bool dsFont_preloadGlyphsUTF8(dsFont* font, dsCommandBuffer* commandBuffer, const char* string)
{
	if (!font || !commandBuffer || !string)
	{
		errno = EINVAL;
		return false;
	}

	return preloadGlyphs(font, commandBuffer, string, (NextCodepointFunction)&dsUTF8_nextCodepoint);
}

bool dsFont_preloadGlyphsUTF16(dsFont* font, dsCommandBuffer* commandBuffer, const uint16_t* string)
{
	if (!font || !commandBuffer || !string)
	{
		errno = EINVAL;
		return false;
	}

	return preloadGlyphs(font, commandBuffer, string,
		(NextCodepointFunction)&dsUTF16_nextCodepoint);
}

bool dsFont_preloadGlyphsUTF32(dsFont* font, dsCommandBuffer* commandBuffer, const uint32_t* string)
{
	if (!font || !commandBuffer || !string)
	{
		errno = EINVAL;
		return false;
	}

	return preloadGlyphs(font, commandBuffer, string,
		(NextCodepointFunction)&dsUTF32_nextCodepoint);
}

bool dsFont_preloadASCII(dsFont* font, dsCommandBuffer* commandBuffer)
{
	const char* ascii = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	return dsFont_preloadGlyphsUTF8(font, commandBuffer, ascii);
}

bool dsFont_destroy(dsFont* font)
{
	if (!font)
		return true;

	if (!dsTexture_destroy(font->texture))
		return false;

	dsAllocator* scratchAllocator = dsFaceGroup_getScratchAllocator(font->group);
	dsAllocator_free(scratchAllocator, font->glyphGeometry.points);
	dsAllocator_free(scratchAllocator, font->glyphGeometry.loops);
	dsAllocator_free(scratchAllocator, font->glyphGeometry.sortedEdges);

	if (font->allocator)
		return dsAllocator_free(font->allocator, font);
	return true;
}
