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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/Text/Types.h>

#define DS_LOW_SIZE 16
#define DS_MEDIUM_SIZE 32
#define DS_HIGH_SIZE 48
#define DS_VERY_HIGH_SIZE 64
// 512 for low, 1024 for medium, 1536 for high, and 2048 for very high
#define DS_TEX_MULTIPLIER 32
#define DS_TEX_MIP_LEVELS 6
#define DS_TABLE_SIZE 1823
#define DS_BASE_WINDOW_SIZE 3

typedef enum dsUnicodeType
{
	dsUnicodeType_UTF8,
	dsUnicodeType_UTF16,
	dsUnicodeType_UTF32
} dsUnicodeType;

typedef enum dsTextDirection
{
	dsTextDirection_Either,
	dsTextDirection_LeftToRight,
	dsTextDirection_RightToLeft
} dsTextDirection;

typedef struct dsGlyphMapping
{
	uint32_t index;
	uint32_t count;
} dsGlyphMapping;

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
	dsVector2i texSize;
} dsGlyphInfo;

struct dsFont
{
	dsAllocator* allocator;
	dsFaceGroup* group;
	dsFontFace** faces;
	uint32_t faceCount;
	uint16_t glyphSize;
	uint16_t usedGlyphCount;

	uint32_t maxWidth;
	uint32_t maxHeight;
	// This gives up thread safety, but is already not an option for FreeType.
	uint8_t* tempImage;
	float* tempSdf;

	dsTexture* texture;
	dsGlyphInfo glyphPool[DS_GLYPH_SLOTS];
	DS_STATIC_HASH_TABLE(DS_TABLE_SIZE) glyphTable;
};

const char* dsFontFace_getName(const dsFontFace* face);
bool dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsVector2i* outTexSize, dsFontFace* face,
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
uint32_t* dsFaceGroup_charMapping(dsFaceGroup* group, uint32_t length);
dsGlyphMapping* dsFaceGroup_glyphMapping(dsFaceGroup* group, uint32_t length);

// Locking not needed for these two functions.
uint32_t dsFaceGroup_codepointScript(const dsFaceGroup* group, uint32_t codepoint);
bool dsFaceGroup_isScriptUnique(uint32_t script);
bool dsFaceGroup_areScriptsEqual(uint32_t script1, uint32_t script2);
dsTextDirection dsFaceGroup_textDirection(uint32_t script);

dsGlyphInfo* dsFont_getGlyphInfo(dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face,
	uint32_t glyph);
uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph);
bool dsFont_shapeRange(const dsFont* font, dsText* text, uint32_t rangeIndex,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction);

// Pixel values are 0 or 1, +Y points down.
bool dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, const uint8_t* pixels, unsigned int width,
	unsigned int height, float* tempSdf);
void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex,
	uint32_t glyphSize);
void dsFont_getGlyphTextureBounds(dsAlignedBox2f* outBounds, const dsTexturePosition* texturePos,
	const dsVector2i* texSize, uint32_t glyphSize);
