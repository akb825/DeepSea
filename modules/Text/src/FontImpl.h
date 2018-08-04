/*
 * Copyright 2017-2018 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/Text/Types.h>

#define DS_LOW_SIZE 16
#define DS_MEDIUM_SIZE 32
#define DS_HIGH_SIZE 48
#define DS_VERY_HIGH_SIZE 64
// 512 for low, 1024 for medium, 1536 for high, 2048 for very high when used with large cache size.
// 1/2 for small cache size.
#define DS_SMALL_CACHE_TEX_MULTIPLIER 16
#define DS_SMALL_CACHE_TEX_MIP_LEVELS 5
#define DS_LARGE_CACHE_TEX_MULTIPLIER 32
#define DS_LARGE_CACHE_TEX_MIP_LEVELS 6
#define DS_TABLE_SIZE 1823
#define DS_BASE_WINDOW_SIZE 4

typedef enum dsTextDirection
{
	dsTextDirection_Either,
	dsTextDirection_LeftToRight,
	dsTextDirection_RightToLeft
} dsTextDirection;

typedef struct dsRunInfo
{
	uint32_t start;
	uint32_t count;
	uint32_t newlineCount;
	dsTextDirection direction;
} dsRunInfo;

typedef struct dsGlyphKey
{
	uint32_t face;
	uint32_t glyph;
} dsGlyphKey;

typedef struct dsGlyphInfo
{
	dsHashTableNode node;
	dsGlyphKey key;
	dsAlignedBox2f glyphBounds;
} dsGlyphInfo;

typedef struct dsGlyphPoint
{
	dsVector2f position;

	dsVector2f nextPos;
	dsVector2f edgeDir;
	float edgeLength;
} dsGlyphPoint;

typedef struct dsGlyphLoop
{
	uint32_t firstPoint;
	uint32_t pointCount;
	dsAlignedBox2f bounds;
} dsGlyphLoop;

typedef struct dsOrderedGlyphEdge
{
	dsVector2f minPoint;
	dsVector2f maxPoint;
} dsOrderedGlyphEdge;

typedef struct dsGlyphGeometry
{
	dsAllocator* allocator;

	dsGlyphPoint* points;
	uint32_t pointCount;
	uint32_t maxPoints;

	dsGlyphLoop* loops;
	uint32_t loopCount;
	uint32_t maxLoops;

	dsOrderedGlyphEdge* sortedEdges;
	uint32_t edgeCount;
	uint32_t maxEdges;

	dsAlignedBox2f bounds;
} dsGlyphGeometry;

struct dsFont
{
	dsAllocator* allocator;
	dsFaceGroup* group;
	dsFontFace** faces;
	dsTextQuality quality;
	uint32_t faceCount;
	uint16_t glyphSize;
	uint16_t cacheSize;
	uint16_t texMultiplier;
	uint16_t usedGlyphCount;

	// State of currently loaded glyph. This gives up thread safety, but is already not an option
	// for FreeType.
	dsGlyphGeometry glyphGeometry;

	dsTexture* texture;
	dsGlyphInfo glyphPool[DS_LARGE_CACHE_GLYPH_SLOTS];
	DS_STATIC_HASH_TABLE(DS_TABLE_SIZE) glyphTable;
};

bool dsIsSpace(uint32_t charcode);
const char* dsFontFace_getName(const dsFontFace* face);
uint32_t dsFontFace_getCodepointGlyph(const dsFontFace* face, uint32_t codepoint);
bool dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	uint32_t glyphSize, dsFont* font);

void dsFaceGroup_lock(const dsFaceGroup* group);
void dsFaceGroup_unlock(const dsFaceGroup* group);
dsAllocator* dsFaceGroup_getScratchAllocator(const dsFaceGroup* group);
dsFontFace* dsFaceGroup_findFace(const dsFaceGroup* group, const char* name);
// Runs are in characters rather than codepoints.
dsRunInfo* dsFaceGroup_findBidiRuns(uint32_t* outCount, dsFaceGroup* group, const void* string,
	dsUnicodeType type);
dsText* dsFaceGroup_scratchText(dsFaceGroup* group, uint32_t length);
bool dsFaceGroup_scratchRanges(dsFaceGroup* group, uint32_t rangeCount);
bool dsFaceGroup_scratchGlyphs(dsFaceGroup* group, uint32_t length);

// Locking not needed for this function.
uint32_t dsFaceGroup_codepointScript(const dsFaceGroup* group, uint32_t codepoint);

bool dsFaceGroup_isScriptUnique(uint32_t script);
bool dsFaceGroup_isScriptCommon(uint32_t script);
bool dsFaceGroup_areScriptsEqual(uint32_t script1, uint32_t script2);
dsTextDirection dsFaceGroup_textDirection(uint32_t script);

dsGlyphInfo* dsFont_getGlyphInfo(dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face,
	uint32_t glyph);
uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph);
uint32_t dsFont_findFaceForCodepoint(const dsFont* font, uint32_t codepoint);
bool dsFont_shapeRange(const dsFont* font, dsText* text, uint32_t rangeIndex,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction);

// Pixel values are 0 or 1, +Y points down.
bool dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, uint32_t texMultiplier,
	const dsGlyphGeometry* geometry);
void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex,
	uint32_t glyphSize, uint32_t texMultiplier);
void dsFont_getGlyphTextureBounds(dsAlignedBox2f* outBounds, const dsTexturePosition* texturePos,
	const dsVector2f* glyphBoundsSize, uint32_t glyphSize, uint32_t texMultiplier);
