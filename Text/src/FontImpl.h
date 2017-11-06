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
#define DS_HIGH_SIZE 64
// 512 for low, 1024 for medium, 2048 for high
#define DS_TEX_MULTIPLIER 32
#define DS_TEX_MIP_LEVELS 6
// 32*32 + 16*16 + 8*8 + 4*4 + 2*2 + 1 (mip levels large enough for glyphs)
#define DS_GLYPH_SLOTS 1365
#define DS_TABLE_SIZE 1823
#define DS_BASE_WINDOW_SIZE 2

typedef struct dsFontFace dsFontFace;

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

struct dsFont
{
	dsAllocator* allocator;
	dsFaceGroup* group;
	dsFontFace** faces;
	uint32_t faceCount;
	uint16_t glyphSize;
	uint16_t usedGlyphCount;
	dsTexture* texture;
	dsGlyphInfo glyphPool[DS_GLYPH_SLOTS];
	DS_STATIC_HASH_TABLE(DS_TABLE_SIZE) glyphTable;
};

const char* dsFontFace_getName(const dsFontFace* face);
void dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	uint32_t glyphSize);

dsAllocator* dsFaceGroup_getAllocator(const dsFaceGroup* group);
dsFontFace* dsFaceGroup_findFace(const dsFaceGroup* group, const char* name);

dsGlyphInfo* dsFont_getGlyphInfo(dsCommandBuffer* commandBuffer, dsFont* font, uint32_t face,
	uint32_t glyph);
uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph);

// Pixel values are 0 or 1, +Y points down.
void dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, const uint8_t* pixels, unsigned int width,
	unsigned int height);
void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex, uint32_t glyphSize);