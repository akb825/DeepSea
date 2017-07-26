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

#include "Resources/MockTexture.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <limits.h>
#include <string.h>

typedef struct dsMockTexture
{
	dsTexture texture;
	size_t dataSize;
	uint8_t data[];
} dsMockTexture;

dsTexture* dsMockTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	size_t textureSize = dsTexture_size(format, dimension, width, height, depth, mipLevels, 1);
	dsMockTexture* texture = (dsMockTexture*)dsAllocator_alloc(allocator,
		sizeof(dsMockTexture) + textureSize);
	if (!texture)
		return NULL;

	texture->texture.resourceManager = resourceManager;
	texture->texture.allocator = dsAllocator_keepPointer(allocator);
	texture->texture.usage = (dsTextureUsage)usage;
	texture->texture.memoryHints = (dsGfxMemory)memoryHints;
	texture->texture.format = format;
	texture->texture.dimension = dimension;
	texture->texture.width = width;
	texture->texture.height = height;
	texture->texture.depth = depth;
	texture->texture.mipLevels = mipLevels;
	texture->texture.offscreen = false;
	texture->texture.resolve = false;
	texture->texture.samples = 1;
	((dsMockTexture*)texture)->dataSize = textureSize;
	if (data)
	{
		DS_ASSERT(size == textureSize);
		memcpy(((dsMockTexture*)texture)->data, data, size);
	}

	return &texture->texture;
}

dsOffscreen* dsMockTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t samples,
	bool resolve)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	size_t textureSize = dsTexture_size(format, dimension, width, height, depth, mipLevels,
		samples);
	dsMockTexture* texture = (dsMockTexture*)dsAllocator_alloc(allocator,
		sizeof(dsMockTexture) + textureSize);
	if (!texture)
		return NULL;

	texture->texture.resourceManager = resourceManager;
	texture->texture.allocator = dsAllocator_keepPointer(allocator);
	texture->texture.usage = (dsTextureUsage)usage;
	texture->texture.memoryHints = (dsGfxMemory)memoryHints;
	texture->texture.format = format;
	texture->texture.dimension = dimension;
	texture->texture.width = width;
	texture->texture.height = height;
	texture->texture.depth = depth;
	texture->texture.mipLevels = mipLevels;
	texture->texture.offscreen = true;
	texture->texture.resolve = resolve;
	DS_ASSERT(samples < USHRT_MAX);
	texture->texture.samples = (uint16_t)samples;
	((dsMockTexture*)texture)->dataSize = textureSize;

	return &texture->texture;
}

bool dsMockTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(texture);
	DS_ASSERT(position);
	DS_ASSERT(data);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(size);

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texture->format));
	unsigned int blockSize = dsGfxFormat_size(texture->format);
	DS_ASSERT(blockSize > 0);

	DS_ASSERT(position->x % blockX == 0 && position->y % blockY == 0);
	uint32_t posBlockX = position->x/blockX;
	uint32_t posBlockY = position->y/blockY;
	uint32_t blockWidth = (width + blockX - 1)/blockX;
	uint32_t blockHeight = (height + blockY - 1)/blockY;
	uint32_t dataPitch = blockWidth*blockSize;
	const uint8_t* dataBytes = (const uint8_t*)data;

	for (uint32_t i = 0; i < layers; ++i)
	{
		size_t textureOffset = dsTexture_surfaceOffset(texture->format,
			texture->dimension, texture->width, texture->height,
			texture->depth, texture->mipLevels, position->face, position->depth + i,
			position->mipLevel);
		uint32_t mipWidth = texture->width >> position->mipLevel;
		uint32_t surfacePitch = (mipWidth + blockX - 1)/blockX*blockSize;
		textureOffset += surfacePitch*posBlockY + posBlockX*blockSize;
		for (uint32_t y = 0; y < blockHeight; ++y, textureOffset += surfacePitch,
			dataBytes += dataPitch)
		{
			DS_ASSERT(textureOffset + dataPitch <= ((dsMockTexture*)texture)->dataSize);
			memcpy(((dsMockTexture*)texture)->data + textureOffset, dataBytes, dataPitch);
		}
	}

	return true;
}

bool dsMockTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	size_t regionCount)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(srcTexture);
	DS_ASSERT(dstTexture);
	DS_ASSERT(regions);
	DS_UNUSED(commandBuffer);

	DS_ASSERT(srcTexture->format == dstTexture->format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, srcTexture->format));
	unsigned int blockSize = dsGfxFormat_size(srcTexture->format);
	DS_ASSERT(blockSize > 0);

	for (size_t i = 0; i < regionCount; ++i)
	{
		DS_ASSERT(regions[i].srcPosition.x % blockX == 0 && regions[i].srcPosition.y % blockY == 0);
		uint32_t srcPosBlockX = regions[i].srcPosition.x/blockX;
		uint32_t srcPosBlockY = regions[i].srcPosition.y/blockY;
		uint32_t srcPosLayer = regions[i].srcPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			srcPosLayer = srcPosLayer*6 + regions[i].srcPosition.face;
		uint32_t srcMipWidth = srcTexture->width >> regions[i].srcPosition.mipLevel;
		uint32_t srcPitch = (srcMipWidth + blockX - 1)/blockX*blockSize;

		DS_ASSERT(regions[i].dstPosition.x % blockX == 0 && regions[i].dstPosition.y % blockY == 0);
		uint32_t dstPosBlockX = regions[i].dstPosition.x/blockX;
		uint32_t dstPosBlockY = regions[i].dstPosition.y/blockY;
		uint32_t dstPosLayer = regions[i].dstPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			dstPosLayer = dstPosLayer*6 + regions[i].dstPosition.face;
		uint32_t dstMipWidth = dstTexture->width >> regions[i].dstPosition.mipLevel;
		uint32_t dstPitch = (dstMipWidth + blockX - 1)/blockX*blockSize;

		uint32_t copySize = (regions[i].width + blockX - 1)/blockX*blockSize;
		uint32_t blockHeight = (regions[i].height + blockY - 1)/blockY;
		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			size_t srcOffset = dsTexture_layerOffset(srcTexture->format,
				srcTexture->dimension, srcTexture->width, srcTexture->height, srcTexture->depth,
				srcTexture->mipLevels, srcPosLayer + j, regions[i].srcPosition.mipLevel);
			srcOffset += srcPosBlockY*srcPitch + srcPosBlockX*blockSize;

			size_t dstOffset = dsTexture_layerOffset(dstTexture->format, dstTexture->dimension,
				dstTexture->width, dstTexture->height, dstTexture->depth, dstTexture->mipLevels,
				dstPosLayer + j, regions[i].dstPosition.mipLevel);
			dstOffset += dstPosBlockY*dstPitch + dstPosBlockX*blockSize;

			for (uint32_t y = 0; y < blockHeight; ++y, srcOffset += srcPitch, dstOffset += dstPitch)
			{
				DS_ASSERT(srcOffset + copySize <= ((dsMockTexture*)srcTexture)->dataSize);
				DS_ASSERT(dstOffset + copySize <= ((dsMockTexture*)dstTexture)->dataSize);
				memcpy(((dsMockTexture*)dstTexture)->data + dstOffset,
					((dsMockTexture*)srcTexture)->data + srcOffset, copySize);
			}
		}
	}

	return true;
}

bool dsMockTexture_blit(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureBlitRegion* regions,
	size_t regionCount, dsBlitFilter filter)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(srcTexture);
	DS_ASSERT(dstTexture);
	DS_ASSERT(regions);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(filter);

	if (srcTexture->format != dstTexture->format)
	{
		errno = EPERM;
		DS_LOG_ERROR("render-mock", "Mock render implementation requires textures to have the same "
			"format when blitting.");
		return false;
	}

	for (size_t i = 0; i < regionCount; ++i)
	{
		if (regions[i].srcWidth != regions[i].dstWidth ||
			regions[i].srcHeight != regions[i].dstHeight)
		{
			errno = EPERM;
			DS_LOG_ERROR("render-mock", "Mock render implementation requires texture regions to "
				" have the same source and destination dimensions when blitting.");
			return false;
		}
	}

	DS_ASSERT(srcTexture->format == dstTexture->format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, srcTexture->format));
	unsigned int blockSize = dsGfxFormat_size(srcTexture->format);
	DS_ASSERT(blockSize > 0);

	for (size_t i = 0; i < regionCount; ++i)
	{
		DS_ASSERT(regions[i].srcPosition.x % blockX == 0 && regions[i].srcPosition.y % blockY == 0);
		uint32_t srcPosBlockX = regions[i].srcPosition.x/blockX;
		uint32_t srcPosBlockY = regions[i].srcPosition.y/blockY;
		uint32_t srcPosLayer = regions[i].srcPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			srcPosLayer = srcPosLayer*6 + regions[i].srcPosition.face;
		uint32_t srcMipWidth = srcTexture->width >> regions[i].srcPosition.mipLevel;
		uint32_t srcPitch = (srcMipWidth + blockX - 1)/blockX*blockSize;

		DS_ASSERT(regions[i].dstPosition.x % blockX == 0 && regions[i].dstPosition.y % blockY == 0);
		uint32_t dstPosBlockX = regions[i].dstPosition.x/blockX;
		uint32_t dstPosBlockY = regions[i].dstPosition.y/blockY;
		uint32_t dstPosLayer = regions[i].dstPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			dstPosLayer = dstPosLayer*6 + regions[i].dstPosition.face;
		uint32_t dstMipWidth = dstTexture->width >> regions[i].dstPosition.mipLevel;
		uint32_t dstPitch = (dstMipWidth + blockX - 1)/blockX*blockSize;

		uint32_t copySize = (regions[i].srcWidth + blockX - 1)/blockX*blockSize;
		uint32_t blockHeight = (regions[i].srcHeight + blockY - 1)/blockY;
		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			size_t srcOffset = dsTexture_layerOffset(srcTexture->format, srcTexture->dimension,
				srcTexture->width, srcTexture->height, srcTexture->depth, srcTexture->mipLevels,
				srcPosLayer + j, regions[i].srcPosition.mipLevel);
			srcOffset += srcPosBlockY*srcPitch + srcPosBlockX*blockSize;

			size_t dstOffset = dsTexture_layerOffset(dstTexture->format, dstTexture->dimension,
				dstTexture->width, dstTexture->height, dstTexture->depth, dstTexture->mipLevels,
				dstPosLayer + j, regions[i].dstPosition.mipLevel);
			dstOffset += dstPosBlockY*dstPitch + dstPosBlockX*blockSize;

			for (uint32_t y = 0; y < blockHeight; ++y, srcOffset += srcPitch, dstOffset += dstPitch)
			{
				DS_ASSERT(srcOffset + copySize <= ((dsMockTexture*)srcTexture)->dataSize);
				DS_ASSERT(dstOffset + copySize <= ((dsMockTexture*)dstTexture)->dataSize);
				memcpy(((dsMockTexture*)dstTexture)->data + dstOffset,
					((dsMockTexture*)srcTexture)->data + srcOffset, copySize);
			}
		}
	}

	return true;
}

bool dsMockTexture_generateMipmaps(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(texture);
	DS_ASSERT(texture);
	return true;
}

bool dsMockTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	DS_ASSERT(result);
	DS_ASSERT(texture);
	DS_ASSERT(position);
	DS_UNUSED(size);
	DS_UNUSED(resourceManager);

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texture->format));
	unsigned int blockSize = dsGfxFormat_size(texture->format);
	DS_ASSERT(blockSize > 0);

	DS_ASSERT(position->x % blockX == 0 && position->y % blockY == 0);
	uint32_t posBlockX = position->x/blockX;
	uint32_t posBlockY = position->y/blockY;
	uint32_t blockWidth = (width + blockX - 1)/blockX;
	uint32_t blockHeight = (height + blockY - 1)/blockY;
	uint32_t dataPitch = blockWidth*blockSize;

	size_t textureOffset = dsTexture_surfaceOffset(texture->format,
		texture->dimension, texture->width, texture->height,
		texture->depth, texture->mipLevels, position->face, position->depth,
		position->mipLevel);
	uint32_t mipWidth = texture->width >> position->mipLevel;
	uint32_t surfacePitch = (mipWidth + blockX - 1)/blockX*blockSize;
	textureOffset += surfacePitch*posBlockY + posBlockX*blockSize;
	for (uint32_t y = 0; y < blockHeight; ++y, textureOffset += surfacePitch)
	{
		DS_ASSERT(textureOffset + dataPitch <= ((dsMockTexture*)texture)->dataSize);
		DS_ASSERT(y*dataPitch < size);
		memcpy((uint8_t*)result + dataPitch*y, ((dsMockTexture*)texture)->data + textureOffset,
			dataPitch);
	}

	return true;
}

bool dsMockTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture)
{
	DS_UNUSED(resourceManager);
	if (texture->allocator)
		return dsAllocator_free(texture->allocator, texture);
	return true;
}
