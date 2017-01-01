/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <errno.h>

uint32_t dsTexture_maxMipmapLevels(uint32_t width, uint32_t height)
{
	uint32_t levelCount = 1;
	for (; width > 1 && height > 1; width /= 2, height /= 2, ++levelCount)
		/* empty */;

	return levelCount;
}

size_t dsTexture_size(dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, uint16_t samples)
{
	if (width == 0 || height == 0)
		return 0;

	depth = dsMax(1U, depth);
	samples = dsMax(1U, samples);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(width, height);
	mipLevels = dsMax(1U, dsMin(mipLevels, maxMipLevels));

	unsigned int blockX, blockY, minX, minY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, format))
		return 0;
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, format));
	unsigned int formatSize = dsGfxFormat_size(format);
	DS_ASSERT(formatSize > 0);

	size_t size = 0;
	for (uint32_t curWidth = width, curHeight = height, i = 0; i < mipLevels;
		curWidth = dsMax(1U, curWidth/2), curHeight = dsMax(1U, curHeight/2), ++i)
	{
		uint32_t curBlocksX = (dsMax(curWidth, minX) + blockX - 1)/blockX;
		uint32_t curBlocksY = (dsMax(curHeight, minY) + blockY - 1)/blockY;
		size += curBlocksX*curBlocksY*formatSize;
	}

	size *= depth*mipLevels*samples;
	if (dimension == dsTextureDim_Cube)
		size *= 6;
	return size;
}

size_t dsTexture_surfaceOffset(dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, dsCubeFace cubeFace, uint32_t depthIndex,
	uint32_t mipIndex)
{
	if (width == 0 || height == 0)
		return 0;

	depth = dsMax(1U, depth);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(width, height);
	mipLevels = dsMax(1U, dsMin(mipLevels, maxMipLevels));

	if (depthIndex >= depth || mipIndex >= mipLevels)
		return 0;

	unsigned int blockX, blockY, minX, minY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, format))
		return 0;
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, format));
	unsigned int formatSize = dsGfxFormat_size(format);
	DS_ASSERT(formatSize > 0);
	unsigned int faces = dimension == dsTextureDim_Cube ? 6 : 1;

	size_t size = 0;
	for (uint32_t curWidth = width, curHeight = height, mip = 0; mip <= mipIndex;
		curWidth = dsMax(1U, curWidth/2), curHeight = dsMax(1U, curHeight/2), ++mip)
	{
		uint32_t curBlocksX = (dsMax(curWidth, minX) + blockX - 1)/blockX;
		uint32_t curBlocksY = (dsMax(curHeight, minY) + blockY - 1)/blockY;

		size_t baseMipSize = curBlocksX*curBlocksY*formatSize;
		// Add all the depth and face levels until we reach the requested mip.
		if (mip < mipIndex)
		{
			size += baseMipSize*depth*faces;
			continue;
		}

		// Offset to the current depth and face index.
		size += baseMipSize*(depthIndex*faces + cubeFace);
		return size;
	}

	DS_ASSERT(false);
	return 0;
}

dsTexture* dsTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator, int usage,
	int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width, uint32_t height,
	uint32_t depth, uint32_t mipLevels, const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createTextureFunc || !resourceManager->destroyTextureFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!usage)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one texture usage flag must be set when creating a texture.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (memoryHints == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one memory hint flag must be set when creating a texture.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((dimension == dsTextureDim_3D &&
		(depth == 0 || depth > resourceManager->maxTextureDepth)) ||
		(dimension != dsTextureDim_3D && depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture depth.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsGfxFormat_textureSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for textures.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(width, height);
	mipLevels = dsMax(1U, dsMin(mipLevels, maxMipLevels));
	if (!resourceManager->arbitraryMipmapping && mipLevels != 1 && mipLevels != maxMipLevels)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"The current target requires textures to be fully mipmapped or not mipmapped at all.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	unsigned int minWidth, minHeight;
	DS_VERIFY(dsGfxFormat_minDimensions(&minWidth, &minHeight, format));
	if (width < minWidth || height < minHeight || width > resourceManager->maxTextureSize ||
		height > resourceManager->maxTextureSize || (dimension == dsTextureDim_3D ?
		depth > resourceManager->maxTextureDepth : depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture dimensions.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t textureSize = dsTexture_size(format, dimension, width, height, depth, mipLevels, 1);
	if (data && size != textureSize)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTexture* texture = resourceManager->createTextureFunc(resourceManager, allocator, usage,
		memoryHints, format, dimension, width, height, depth, mipLevels, data, size);
	if (texture)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, 1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, textureSize);
	}
	DS_PROFILE_FUNC_RETURN(texture);
}

dsOffscreen* dsTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, uint16_t samples, bool resolve)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createTextureFunc || !resourceManager->destroyTextureFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!usage)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one texture usage flag must be set when creating an offscreen.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (memoryHints == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one memory hint flag must be set when creating an offscreen.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((dimension == dsTextureDim_3D &&
		(depth == 0 || depth > resourceManager->maxTextureDepth)) ||
		(dimension != dsTextureDim_3D && depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture depth.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsGfxFormat_offscreenSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for offscreens.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(width, height);
	mipLevels = dsMax(1U, dsMin(mipLevels, maxMipLevels));
	if (!resourceManager->arbitraryMipmapping && mipLevels != 1 && mipLevels != maxMipLevels)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"The current target requires textures to be fully mipmapped or not mipmapped at all.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	unsigned int minWidth, minHeight;
	DS_VERIFY(dsGfxFormat_minDimensions(&minWidth, &minHeight, format));
	if (width < minWidth || height < minHeight || width > resourceManager->maxTextureSize ||
		height > resourceManager->maxTextureSize || (dimension == dsTextureDim_3D ?
		depth > resourceManager->maxTextureDepth : depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture dimensions.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsOffscreen* offscreen = resourceManager->createOffscreenFunc(resourceManager, allocator, usage,
		memoryHints, format, dimension, width, height, depth, mipLevels, samples, resolve);
	if (offscreen)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, 1);
		size_t textureSize = dsTexture_size(format, dimension, width, height, depth, mipLevels,
			samples);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, textureSize);
	}
	DS_PROFILE_FUNC_RETURN(offscreen);
}

bool dsTexture_copyData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, const void* data,
	size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !texture || !texture->resourceManager ||
		!texture->resourceManager->copyTextureDataFunc || !position || !data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->usage & dsTextureUsage_CopyTo))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((position->depth > 0 && position->depth >= texture->depth) ||
		position->mipLevel >= texture->mipLevels)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t mipWidth = dsMax(1U, texture->width/(1 << position->mipLevel));
	uint32_t mipHeight = dsMax(1U, texture->height/(1 << position->mipLevel));
	if (position->x + width > mipWidth || position->y + height > mipHeight)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (size != dsTexture_size(texture->format, texture->dimension, width, height, 1, 1,
		texture->samples))
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	bool success = resourceManager->copyTextureDataFunc(resourceManager, commandBuffer, texture,
		position, width, height, data, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_copy(dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsTexture* dstTexture,
	const dsTextureCopyRegion* regions, size_t regionCount)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !srcTexture || !dstTexture || !srcTexture->resourceManager ||
		!srcTexture->resourceManager->copyTextureFunc || srcTexture != dstTexture || !regions)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcTexture->usage & dsTextureUsage_CopyFrom))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a texture without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstTexture->usage & dsTextureUsage_CopyTo))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	for (size_t i = 0; i < regionCount; ++i)
	{
		uint32_t depthCount = (srcTexture->dimension != dsTextureDim_3D &&
			dstTexture->dimension != dsTextureDim_3D) ? regions[i].arrayLevelCount : 1;
		depthCount = dsMax(1U, depthCount);

		const dsTexturePosition* srcPosition = &regions[i].srcPosition;
		uint32_t maxSrcDepth = srcPosition->depth + depthCount - 1;
		if ((maxSrcDepth > 0 && maxSrcDepth >= srcTexture->depth) ||
			srcPosition->mipLevel >= srcTexture->mipLevels)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t srcMipWidth = dsMax(1U, srcTexture->width/(1 << srcPosition->mipLevel));
		uint32_t srcMipHeight = dsMax(1U, srcTexture->height/(1 << srcPosition->mipLevel));
		if (srcPosition->x + regions[i].width > srcMipWidth ||
			srcPosition->y + regions[i].height > srcMipHeight)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* dstPosition = &regions[i].dstPosition;
		uint32_t maxDstDepth = dstPosition->depth + depthCount - 1;
		if ((maxDstDepth > 0 && maxDstDepth >= srcTexture->depth) ||
			dstPosition->mipLevel >= dstTexture->mipLevels)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t dstMipWidth = dsMax(1U, dstTexture->width/(1 << dstPosition->mipLevel));
		uint32_t dstMipHeight = dsMax(1U, dstTexture->height/(1 << dstPosition->mipLevel));
		if (dstPosition->x + regions[i].width > dstMipWidth ||
			dstPosition->y + regions[i].height > dstMipHeight)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsResourceManager* resourceManager = srcTexture->resourceManager;
	bool success = resourceManager->copyTextureFunc(resourceManager, commandBuffer, srcTexture,
		dstTexture, regions, regionCount);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_blit(dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsTexture* dstTexture,
	const dsTextureBlitRegion* regions, size_t regionCount, dsFilter filter)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !srcTexture || !dstTexture || !srcTexture->resourceManager ||
		!srcTexture->resourceManager->blitTextureFunc || srcTexture != dstTexture || !regions)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcTexture->usage & dsTextureUsage_CopyFrom))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a texture without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstTexture->usage & dsTextureUsage_CopyTo))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	for (size_t i = 0; i < regionCount; ++i)
	{
		if ((srcTexture->dimension != dsTextureDim_3D ||
			dstTexture->dimension != dsTextureDim_3D) &&
			regions[i].srcDepthRange != regions[i].dstDepthRange)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Source and destination depth ranges must match when blitting texture arrays.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Avoid underflow during checks.
		if (regions[i].srcDepthRange == 0 || regions[i].dstDepthRange == 0)
			continue;

		const dsTexturePosition* srcPosition = &regions[i].srcPosition;
		uint32_t maxSrcDepth = srcPosition->depth + regions[i].srcDepthRange - 1;
		if ((maxSrcDepth > 0 && maxSrcDepth >= srcTexture->depth) ||
			srcPosition->mipLevel >= srcTexture->mipLevels)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t srcMipWidth = dsMax(1U, srcTexture->width/(1 << srcPosition->mipLevel));
		uint32_t srcMipHeight = dsMax(1U, srcTexture->height/(1 << srcPosition->mipLevel));
		if (srcPosition->x + regions[i].srcWidth > srcMipWidth ||
			srcPosition->y + regions[i].srcHeight > srcMipHeight)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* dstPosition = &regions[i].dstPosition;
		uint32_t maxDstDepth = dstPosition->depth + regions[i].dstDepthRange - 1;
		if ((maxDstDepth > 0 && maxDstDepth >= srcTexture->depth) ||
			dstPosition->mipLevel >= dstTexture->mipLevels)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t dstMipWidth = dsMax(1U, dstTexture->width/(1 << dstPosition->mipLevel));
		uint32_t dstMipHeight = dsMax(1U, dstTexture->height/(1 << dstPosition->mipLevel));
		if (dstPosition->x + regions[i].dstWidth > dstMipWidth ||
			dstPosition->y + regions[i].dstHeight > dstMipHeight)
		{
			errno = ERANGE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsResourceManager* resourceManager = srcTexture->resourceManager;
	bool success = resourceManager->blitTextureFunc(resourceManager, commandBuffer, srcTexture,
		dstTexture, regions, regionCount, filter);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_getData(void* result, size_t size, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	if (!result || !texture || !texture->resourceManager ||
		!texture->resourceManager->getTextureDataFunc || !position)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->usage & dsTextureUsage_CopyFrom))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a texture without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	if (!texture->offscreen && !resourceManager->texturesReadable)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"The target doesn't support reading from a non-offscreen texture.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((position->depth > 0 && position->depth >= texture->depth) ||
		position->mipLevel >= texture->mipLevels)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t mipWidth = dsMax(1U, texture->width/(1 << position->mipLevel));
	uint32_t mipHeight = dsMax(1U, texture->height/(1 << position->mipLevel));
	if (position->x + width > mipWidth || position->y + height > mipHeight)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (size != dsTexture_size(texture->format, texture->dimension, width, height, 1, 1,
		texture->samples))
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	bool success = resourceManager->getTextureDataFunc(result, size, resourceManager, texture,
		position, width, height);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_destroy(dsTexture* texture)
{
	DS_PROFILE_FUNC_START();

	if (!texture || !texture->resourceManager || !texture->resourceManager->destroyTextureFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t size = dsTexture_size(texture->format, texture->dimension, texture->width,
		texture->height, texture->depth, texture->mipLevels, texture->samples);
	bool success = resourceManager->destroyTextureFunc(resourceManager, texture);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, -1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, -size);
	}
	DS_PROFILE_FUNC_RETURN(success);
}
