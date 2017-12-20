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
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Text/FaceGroup.h>
#include <string.h>

static float computeSignedDistance(const uint8_t* pixels, uint32_t width, uint32_t height, int x,
	int y, uint32_t windowSize)
{
	bool inside = false;
	if (x >= 0 && y >= 0 && x < (int)width && y < (int)height)
		inside = pixels[y*width + x] != 0;

	// Compute the closest distance to a pixel that is the opposite state.
	float maxDistance = sqrtf((float)(dsPow2(windowSize) + dsPow2(windowSize)));
	float distance = maxDistance;
	for (uint32_t j = 0; j < windowSize*2 + 1; ++j)
	{
		for (uint32_t i = 0; i < windowSize*2 + 1; ++i)
		{
			int thisX = x + i - windowSize;
			int thisY = y + j - windowSize;
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

dsGlyphInfo* dsFont_getGlyphInfo(dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face,
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

	dsFontFace_cacheGlyph(&glyphInfo->glyphBounds, &glyphInfo->texSize, font->faces[face],
		commandBuffer, font->texture, glyph, dsFont_getGlyphIndex(font, glyphInfo), font->glyphSize,
		font);
	return glyphInfo;
}

uint32_t dsFont_getGlyphIndex(dsFont* font, dsGlyphInfo* glyph)
{
	return (uint32_t)(size_t)(glyph - font->glyphPool);
}

bool dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, const uint8_t* pixels, unsigned int width,
	unsigned int height, float* tempSdf)
{
	uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;

	// Pad by the window size on each side.
	uint32_t adjustedWidth = width + windowSize*2;
	uint32_t adjustedHeight = height + windowSize*2;

	// Scale down if needed, but not up.
	float scaleX = 1.0f;
	float offsetX = 1.0f;
	if (adjustedWidth > glyphSize)
	{
		scaleX = (float)glyphSize/(float)adjustedWidth;
		offsetX = 1.0f/scaleX;
	}

	float scaleY = 1.0f;
	float offsetY = 1.0f;
	if (adjustedHeight > glyphSize)
	{
		scaleY = (float)glyphSize/(float)adjustedHeight;
		offsetY = 1.0f/scaleY;
	}

	// Compute signed distnace field.
	for (uint32_t y = 0; y < adjustedHeight; ++y)
	{
		for (uint32_t x = 0; x < adjustedWidth; ++x)
		{
			tempSdf[y*adjustedWidth + x] = computeSignedDistance(pixels, width, height,
				x - windowSize, y - windowSize, windowSize);
		}
	}

	// Scale the glyph into the texture.
	DS_ASSERT(glyphSize <= DS_VERY_HIGH_SIZE);
	uint8_t textureData[DS_VERY_HIGH_SIZE*DS_VERY_HIGH_SIZE];
	memset(textureData, 0, sizeof(textureData));
	for (uint32_t y = 0; y < glyphSize; ++y)
	{
		float origY = (float)(int)(y - windowSize)*scaleY + (float)windowSize;
		int startY = (int)origY;

		// Calculate factors for linear filter.
		float centerY = ((float)y + 0.5f)/scaleY;
		unsigned int bottom = (int)(centerY - offsetY + 0.5f);
		bottom = dsMax((int)bottom, 0);
		unsigned int top = (unsigned int)(centerY + offsetY + 0.5f);
		top = dsMin(top, adjustedHeight);

		for (uint32_t x = 0; x < glyphSize; ++x)
		{
			float origX = (float)(int)(x - windowSize)*scaleX + (float)windowSize;
			int startX = (int)origX;

			uint32_t dstIndex = y*glyphSize + x;
			if (scaleX == 1.0f && scaleY == 1.0f)
			{
				if (startX >= 0 && startY >= 0 && startX < (int)adjustedWidth &&
					startY < (int)adjustedHeight)
				{
					uint32_t srcIndex = startY*adjustedWidth + startX;
					textureData[dstIndex] = (uint8_t)roundf(tempSdf[srcIndex]*255.0f);
				}
				else
					textureData[dstIndex] = 0;
			}
			else
			{
				// Linear filter.
				float centerX = ((float)x + 0.5f)/scaleX;
				unsigned int left = (int)(centerX - offsetX + 0.5f);
				left = dsMax((int)left, 0);
				unsigned int right = (unsigned int)(centerX + offsetX + 0.5f);
				right = dsMax(right, adjustedWidth);

				float weightedDistance = 0.0f;
				float totalWeight = 0.0f;
				for (unsigned int y2 = bottom; y2 < top; ++y2)
				{
					float weightY = 1.0f - fabsf((float)y2 + 0.5f - centerY)*scaleY;
					weightY = dsMax(weightY, 0.0f);
					if (weightY == 0.0)
						continue;

					for (unsigned int x2 = left; x2 < right; ++x2)
					{
						float weightX = 1.0f - fabsf((float)x2 + 0.5f - centerX)*scaleX;
						weightX = dsMax(weightX, 0.0f);
						if (weightX == 0.0)
							continue;

						uint32_t srcIndex = y2*adjustedWidth + x2;
						float weight = weightX*weightY;
						weightedDistance += tempSdf[srcIndex]*weight;
						totalWeight += weight;
					}
				}

				if (totalWeight == 0)
					textureData[dstIndex] = 0;
				else
					textureData[dstIndex] = (uint8_t)roundf(weightedDistance*255.0f/totalWeight);
			}
		}
	}

	dsTexturePosition texturePos;
	dsFont_getGlyphTexturePos(&texturePos, glyphIndex, glyphSize);
	return dsTexture_copyData(texture, commandBuffer, &texturePos, glyphSize, glyphSize, 1,
		textureData, glyphSize*glyphSize);
}

void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex, uint32_t glyphSize)
{
	outPos->face = dsCubeFace_None;
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
			outPos->x = index%(DS_TEX_MULTIPLIER >> i)*glyphSize;
			outPos->y = index/(DS_TEX_MULTIPLIER >> i)*glyphSize;
			return;
		}

		prevLimit = curLimit;
	}
	DS_ASSERT(false);
}

void dsFont_getGlyphTextureBounds(dsAlignedBox2f* outBounds, const dsTexturePosition* texturePos,
	const dsVector2i* texSize, uint32_t glyphSize)
{
	uint32_t windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
	float levelSize = 1.0f/(float)(DS_TEX_MULTIPLIER*glyphSize >> texturePos->mipLevel);
	outBounds->min.x = (float)texturePos->x*levelSize;
	outBounds->min.y = (float)texturePos->y*levelSize;

	dsVector2f offset = {{(float)(texSize->x + windowSize*2), (float)(texSize->y + windowSize*2)}};
	offset.x = dsMin(offset.x, (float)glyphSize);
	offset.y = dsMin(offset.y, (float)glyphSize);

	dsVector2f levelSize2 = {{levelSize, levelSize}};
	dsVector2_mul(offset, offset, levelSize2);
	dsVector2_add(outBounds->max, outBounds->min, offset);

	// Add a half pixel to avoid leakage from other glyphs.
	dsVector2f halfPixel = {{0.5f*levelSize, 0.5f*levelSize}};
	dsVector2_add(outBounds->min, outBounds->min, halfPixel);
	dsVector2_add(outBounds->max, outBounds->max, halfPixel);
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

	uint16_t glyphSize;
	switch (dsFaceGroup_getTextQuality(group))
	{
		case dsTextQuality_Low:
			glyphSize = DS_LOW_SIZE;
			break;
		case dsTextQuality_High:
			glyphSize = DS_HIGH_SIZE;
			break;
		case dsTextQuality_VeryHigh:
			glyphSize = DS_VERY_HIGH_SIZE;
			break;
		case dsTextQuality_Medium:
		default:
			glyphSize = DS_MEDIUM_SIZE;
			break;
	}

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

	font->maxWidth = glyphSize*2;
	font->maxHeight = glyphSize*2;
	dsAllocator* scratchAllocator = dsFaceGroup_getScratchAllocator(group);
	font->tempImage = (uint8_t*)dsAllocator_alloc(scratchAllocator,
		font->maxWidth*font->maxHeight*sizeof(uint8_t));
	if (!font->tempImage)
	{
		if (font->allocator)
			dsAllocator_free(font->allocator, font);
		dsFaceGroup_unlock(group);
		return NULL;
	}

	unsigned int windowSize = glyphSize*DS_BASE_WINDOW_SIZE/DS_LOW_SIZE;
	uint32_t sdfWidth = font->maxWidth + windowSize*2;
	uint32_t sdfHeight= font->maxHeight + windowSize*2;
	font->tempSdf = (float*)dsAllocator_alloc(scratchAllocator, sdfWidth*sdfHeight*sizeof(float));
	if (!font->tempSdf)
	{
		dsAllocator_free(scratchAllocator, font->tempImage);
		if (font->allocator)
			dsAllocator_free(font->allocator, font);
		dsFaceGroup_unlock(group);
		return NULL;
	}

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
		dsAllocator_free(scratchAllocator, font->tempImage);
		dsAllocator_free(scratchAllocator, font->tempSdf);
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

	dsAllocator* scratchAllocator = dsFaceGroup_getScratchAllocator(font->group);
	if (font->tempImage)
		dsAllocator_free(scratchAllocator, font->tempImage);
	if (font->tempSdf)
		dsAllocator_free(scratchAllocator, font->tempSdf);

	if (font->allocator)
		return dsAllocator_free(font->allocator, font);
	return true;
}
