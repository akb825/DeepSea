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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Text/FaceGroup.h>

static float computeSignedDistance(const uint8_t* pixels, unsigned int width, unsigned int height,
	int x, int y, uint32_t windowSize)
{
	bool inside = false;
	if (x >= 0 && y >= 0 && x < (int)width && y < (int)height)
		inside = pixels[y*width + x] != 0;

	// Compute the closest distance to a pixel that is the opposite state.
	float maxDistance = (float)(windowSize*windowSize);
	float distance = maxDistance;
	for (uint32_t j = 0; j < windowSize*2 + 1; ++j)
	{
		for (uint32_t i = 0; i < windowSize*2 + 1; ++i)
		{
			int thisX = x + i - windowSize - 1;
			int thisY = y + j - windowSize - 1;
			bool thisInside = false;
			if (thisX >= 0 && thisY >= 0 && thisX < (int)width && thisY < (int)height)
				thisInside = pixels[thisY*width + thisX] != 0;
			if (thisInside != inside)
			{
				float thisDistance = sqrtf((float)(dsPow2(x - thisX) + dsPow2(y - thisY)));
				distance = dsMin(thisDistance, distance);
			}
		}
	}

	// Normalize to a [0, 1] value to be placed into the texture.
	distance = distance/maxDistance;
	if (!inside)
		distance = -distance;
	return distance*0.5f + 0.5f;
}

dsGlyphInfo* dsFont_getGlyphInfo(dsCommandBuffer* commandBuffer, dsFont* font, uint32_t face,
	uint32_t glyph)
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

	if (font->usedGlyphCount < DS_GLYPH_SLOTS)
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
		glyph, dsFont_getGlyphIndex(font, glyphInfo), font->glyphSize, font->tempImage,
		font->tempSdf);
	return glyphInfo;
}

uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph)
{
	return (uint32_t)(size_t)(glyph - font->glyphPool);
}

void dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, const uint8_t* pixels, unsigned int width,
	unsigned int height, float* tempSdf)
{
	uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;

	// Pad by the window size on each side.
	unsigned int adjustedWidth = width + windowSize*2;
	unsigned int adjustedHeight = height + windowSize*2;
	float scaleX = (float)adjustedWidth/(float)(glyphSize + windowSize*2);
	float scaleY = (float)adjustedHeight/(float)(glyphSize + windowSize*2);

	// Compute signed distnace field.
	for (unsigned int y = 0; y < adjustedWidth; ++y)
	{
		for (unsigned int x = 0; x < adjustedHeight; ++x)
		{
			tempSdf[y*adjustedWidth + x] = computeSignedDistance(pixels, width, height,
				x - windowSize, y - windowSize, windowSize);
		}
	}

	// Scale the glyph into the texture.
	const dsVector2i offsets[4] = {{{0, 0}}, {{1, 0}}, {{0, 1}}, {{1, 1}}};
	DS_ASSERT(glyphSize <= DS_HIGH_SIZE);
	uint8_t textureData[DS_HIGH_SIZE*DS_HIGH_SIZE];
	for (uint32_t y = 0; y < glyphSize; ++y)
	{
		float origY = (float)(int)(y - windowSize)*scaleY + (float)windowSize;
		int startY = (int)origY;
		float tY = origY - (float)startY;
		for (uint32_t x = 0; x < glyphSize; ++x)
		{
			float origX = (float)(int)(x - windowSize)*scaleX + (float)windowSize;
			int startX = (int)origX;
			float tX = origX - (float)startX;

			float samples[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			for (unsigned int i = 0; i < 4; ++i)
			{
				int curX = startX + offsets[i].x;
				int curY = startY + offsets[i].y;
				if (curX >= 0 && curY >= 0 && curX < (int)adjustedWidth &&
					curY < (int)adjustedHeight)
				{
					samples[i] = tempSdf[curY*adjustedWidth + curX];
				}

				samples[0] = dsLerp(samples[0], samples[1], tX);
				samples[1] = dsLerp(samples[2], samples[3], tX);
				samples[0] = dsLerp(samples[0], samples[1], tY);
				textureData[y*glyphSize + x] = (uint8_t)roundf(samples[0]*255.0f);
			}
		}
	}

	dsTexturePosition texturePos;
	dsFont_getGlyphTexturePos(&texturePos, glyphIndex, glyphSize);
	dsTexture_copyData(commandBuffer, texture, &texturePos, glyphSize, glyphSize, 1, textureData,
		glyphSize*glyphSize);
}

void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex, uint32_t glyphSize)
{
	outPos->face = dsCubeFace_PosX;
	outPos->depth = 0;

	static const uint32_t limits[DS_TEX_MIP_LEVELS] =
	{
		dsPow2(DS_TEX_MULTIPLIER),
		dsPow2(DS_TEX_MULTIPLIER/2),
		dsPow2(DS_TEX_MULTIPLIER/4),
		dsPow2(DS_TEX_MULTIPLIER/8),
		dsPow2(DS_TEX_MULTIPLIER/16),
		dsPow2(DS_TEX_MULTIPLIER/32)
	};

	uint32_t prevLimit = 0;
	for (uint32_t i = 0; i < DS_TEX_MIP_LEVELS; ++i)
	{
		uint32_t curLimit = prevLimit + limits[i];
		if (glyphIndex < curLimit)
		{
			uint32_t index = glyphIndex - prevLimit;
			outPos->mipLevel = i;
			outPos->x = index/(DS_TEX_MULTIPLIER >> i)*glyphSize;
			outPos->y = index%(DS_TEX_MULTIPLIER >> i)*glyphSize;
			return;
		}

		prevLimit = curLimit;
	}
	DS_ASSERT(false);
}

dsFont* dsFont_create(dsFaceGroup* group, dsResourceManager* resourceManager,
	dsAllocator* allocator, const char** faceNames, uint32_t faceCount)
{
	if (!group || !resourceManager || (!allocator && !dsFaceGroup_getAllocator(group)) ||
		!faceNames || faceCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t maxWidth = 0, maxHeight = 0;
	for (uint32_t i = 0; i < faceCount; ++i)
	{
		if (!faceNames[i])
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Empty face name.");
			return NULL;
		}

		dsFontFace* face = dsFaceGroup_findFace(group, faceNames[i]);
		if (!face)
		{
			DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Face '%s' not found.", faceNames[i]);
			errno = ENOTFOUND;
			return NULL;
		}

		uint32_t curWidth, curHeight;
		dsFontFace_getMaxSize(&curWidth, &curHeight, face);
		maxWidth = dsMax(curWidth, maxWidth);
		maxHeight = dsMax(curHeight, maxHeight);
	}

	if (!allocator)
		allocator = dsFaceGroup_getAllocator(group);

	uint16_t glyphSize;
	switch (dsFaceGroup_getTextQuality(group))
	{
		case dsTextQuality_Low:
			glyphSize = DS_LOW_SIZE;
			break;
		case dsTextQuality_High:
			glyphSize = DS_HIGH_SIZE;
			break;
		case dsTextQuality_Medium:
		default:
			glyphSize = DS_MEDIUM_SIZE;
			break;
	}

	unsigned int windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
	size_t tempImageSize = maxWidth*maxHeight*sizeof(uint8_t);
	size_t tempSdfSize = (maxWidth + windowSize)*(maxHeight + windowSize)*sizeof(float);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsFont)) +
		DS_ALIGNED_SIZE(sizeof(dsFontFace*)*faceCount) + DS_ALIGNED_SIZE(tempImageSize) +
		DS_ALIGNED_SIZE(tempSdfSize);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsFont* font = (dsFont*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, sizeof(dsFont));
	DS_ASSERT(font);
	font->allocator = dsAllocator_keepPointer(allocator);
	font->group = group;
	font->faces = (dsFontFace**)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsFontFace*)*faceCount);
	DS_ASSERT(font->faces);
	for (uint32_t i = 0; i < faceCount; ++i)
	{
		font->faces[i] = dsFaceGroup_findFace(group, faceNames[i]);
		DS_ASSERT(font->faces[i]);
	}

	font->faceCount = faceCount;
	font->glyphSize = glyphSize;
	font->usedGlyphCount = 0;

	font->maxWidth = maxWidth;
	font->maxHeight = maxHeight;
	font->tempImage = (uint8_t*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, tempImageSize);
	font->tempSdf = (float*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, tempSdfSize);

	DS_STATIC_ASSERT(sizeof(dsGlyphKey) == sizeof(uint64_t), unexpected_glyph_key_size);
	DS_VERIFY(dsHashTable_initialize(&font->glyphTable.hashTable, DS_TABLE_SIZE,
		&dsHash64, &dsHash64Equal));

	uint32_t textureSize = font->glyphSize*DS_TEX_MULTIPLIER;
	uint32_t mipLevels = resourceManager->hasArbitraryMipmapping ?
		DS_TEX_MIP_LEVELS : DS_ALL_MIP_LEVELS;
	font->texture = dsTexture_create(resourceManager, allocator,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Dynamic,
		dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm), dsTextureDim_2D, textureSize,
		textureSize, 0, mipLevels, NULL, 0);
	if (!font->texture)
	{
		if (font->allocator)
			dsAllocator_free(font->allocator, font);
		return NULL;
	}

	return font;
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

dsTexture* dsFont_getTexture(dsFont* font)
{
	if (!font)
		return NULL;

	return font->texture;
}

bool dsFont_destroy(dsFont* font)
{
	if (!font)
		return false;

	if (!dsTexture_destroy(font->texture))
		return false;

	if (font->allocator)
		return dsAllocator_free(font->allocator, font);
	return true;
}
