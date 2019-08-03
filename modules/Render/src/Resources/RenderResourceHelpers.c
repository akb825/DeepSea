/*
 * Copyright 2019 Aaron Barany
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

#include "Resources/RenderResourceHelpers.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

bool dsIsGfxBufferTextureCopyRegionValid(const dsGfxBufferTextureCopyRegion* region,
	const dsTextureInfo* info, size_t bufferSize)
{
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, info->format));
	unsigned int formatSize = dsGfxFormat_size(info->format);

	const dsTexturePosition* position = &region->texturePosition;
	if (position->x % blockX != 0 || position->y % blockY != 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture position must be a multiple of the block size.");
		return false;
	}

	if (position->mipLevel >= info->mipLevels)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		return false;
	}

	uint32_t mipWidth = dsMax(1U, info->width >> position->mipLevel);
	uint32_t mipHeight = dsMax(1U, info->height >> position->mipLevel);
	uint32_t mipLayers = dsMax(1U, info->depth);
	uint32_t layerOffset = position->depth;
	if (info->dimension == dsTextureDim_3D)
		mipLayers = dsMax(1U, mipLayers >> position->mipLevel);
	else if (info->dimension == dsTextureDim_Cube)
	{
		mipLayers *= 6;
		layerOffset = layerOffset*6 + position->face;
	}

	uint32_t textureEndX = position->x + region->textureWidth;
	uint32_t textureEndY = position->y + region->textureHeight;
	uint32_t textureEndLayer = layerOffset + region->layers;
	if (textureEndX > mipWidth || textureEndY > mipHeight || textureEndLayer > mipLayers)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		return false;
	}

	uint32_t bufferWidth = region->bufferWidth;
	uint32_t bufferHeight = region->bufferHeight;
	if (bufferWidth == 0)
		bufferWidth = region->textureWidth;
	if (bufferHeight == 0)
		bufferHeight = region->textureHeight;

	if (bufferWidth < region->textureWidth || bufferHeight < region->textureHeight)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer dimensions must be at least as large as texture dimensions.");
		return false;
	}

	size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
	size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
	size_t textureXBlocks = (region->textureWidth + blockX - 1)/blockX;
	size_t remainderXBlocks = bufferXBlocks - textureXBlocks;
	size_t copySize = (bufferXBlocks*bufferYBlocks*region->layers - remainderXBlocks)*formatSize;
	if (!DS_IS_BUFFER_RANGE_VALID(region->bufferOffset, copySize, bufferSize))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy buffer data out of range.");
		return false;
	}

	if ((textureEndX % blockX != 0 && textureEndX != mipWidth) ||
		(textureEndY % blockY != 0 && textureEndY != mipHeight))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture data width and height must be a multiple of the block size or reach the "
			"edge of the image.");
		return false;
	}

	if (position->mipLevel >= info->mipLevels)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		return false;
	}

	if (info->dimension != dsTextureDim_Cube && position->face != dsCubeFace_None)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot copy a specific cube face when not a cube map.");
		return false;
	}

	return true;
}
