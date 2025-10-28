/*
 * Copyright 2025 Aaron Barany
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
// Max X and Y offset at the minimum quality to check for signed distances. Apply a thickness scale
// to have the range [0, 1] be a reasonable amount for things like embolding and outlines.
#define DS_BASE_WINDOW_SIZE 2
#define DS_THICKNESS_SCALE 0.5f
#define DS_ICON_FACE (uint32_t)-1

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
	const dsTextIcons* icons;
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

typedef struct dsIconGlyphNode
{
	dsHashTableNode node;
	uint32_t codepoint;
	uint32_t index;
} dsIconGlyphNode;

struct dsTextIcons
{
	dsAllocator* allocator;
	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;
	dsPrepareDrawTextIconsFunction prepareFunc;
	dsPrepareDrawTextIconsFunction drawFunc;
	dsDestroyUserDataFunction destroyGlyphUserDataFunc;

	dsIndexRange* codepointRanges;
	dsIconGlyph* iconGlyphs;
	dsIconGlyphNode* iconNodes;
	uint32_t codepointRangeCount;
	uint32_t iconCount;
	uint32_t maxIcons;

	dsHashTable* iconTable;
};
