/*
 * Copyright 2016-2019 Aaron Barany
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
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

uint32_t dsTexture_maxMipmapLevels(uint32_t width, uint32_t height, uint32_t depth)
{
	uint32_t levelCountWidth = 32 - dsClz(width);
	uint32_t levelCountHeight = 32 - dsClz(height);
	uint32_t levelCountDepth = 32 - dsClz(depth);
	uint32_t maxWidthHeight = dsMax(levelCountWidth, levelCountHeight);
	return dsMax(maxWidthHeight, levelCountDepth);
}

size_t dsTexture_size(const dsTextureInfo* info)
{
	if (!info || info->width == 0 || info->height == 0)
		return 0;

	uint32_t depth = dsMax(1U, info->depth);
	uint32_t samples = dsMax(1U, info->samples);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(info->width, info->height,
		DS_MIP_DEPTH(info->dimension, depth));
	uint32_t mipLevels = dsMin(info->mipLevels, maxMipLevels);
	mipLevels = dsMax(1U, mipLevels);

	unsigned int blockX, blockY, minX, minY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
		return 0;
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, info->format));
	unsigned int formatSize = dsGfxFormat_size(info->format);
	DS_ASSERT(formatSize > 0);

	size_t size = 0;
	for (uint32_t curWidth = info->width, curHeight = info->height, curDepth = depth, i = 0;
		i < mipLevels;
		curWidth = dsMax(1U, curWidth/2), curHeight = dsMax(1U, curHeight/2),
		curDepth = info->dimension == dsTextureDim_3D ? dsMax(1U, curDepth/2) : depth, ++i)
	{
		uint32_t curBlocksX = (dsMax(curWidth, minX) + blockX - 1)/blockX;
		uint32_t curBlocksY = (dsMax(curHeight, minY) + blockY - 1)/blockY;
		size += curBlocksX*curBlocksY*formatSize*curDepth;
	}

	size *= samples;
	if (info->dimension == dsTextureDim_Cube)
		size *= 6;
	return size;
}

uint32_t dsTexture_surfaceCount(const dsTextureInfo* info)
{
	if (!info)
		return 0;

	uint32_t depth = dsMax(1U, info->depth);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(info->width, info->height,
		DS_MIP_DEPTH(info->dimension, depth));
	uint32_t mipLevels = dsMin(info->mipLevels, maxMipLevels);
	mipLevels = dsMax(1U, mipLevels);

	unsigned int faces = info->dimension == dsTextureDim_Cube ? 6 : 1;
	if (info->dimension == dsTextureDim_3D)
	{
		uint32_t count = 0;
		for (uint32_t i = 0, curDepth = depth; i < mipLevels; ++i, curDepth = dsMax(1U, curDepth/2))
			count += curDepth*faces;
		return count;
	}

	return mipLevels*depth*faces;
}

uint32_t dsTexture_surfaceIndex(const dsTextureInfo* info, dsCubeFace cubeFace, uint32_t depthIndex,
	uint32_t mipIndex)
{
	if (!info)
		return DS_INVALID_TEXTURE_SURFACE;

	uint32_t depth = dsMax(1U, info->depth);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(info->width, info->height,
		DS_MIP_DEPTH(info->dimension, depth));
	uint32_t mipLevels = dsMin(info->mipLevels, maxMipLevels);
	mipLevels = dsMax(1U, mipLevels);

	if (depthIndex >= depth || mipIndex >= mipLevels ||
		(info->dimension != dsTextureDim_Cube && cubeFace != dsCubeFace_None))
	{
		return DS_INVALID_TEXTURE_SURFACE;
	}

	unsigned int faces = info->dimension == dsTextureDim_Cube ? 6 : 1;
	if (info->dimension == dsTextureDim_3D)
	{
		for (uint32_t i = 0, curDepth = depth, index = 0; i < mipLevels;
			++i, curDepth = info->dimension == dsTextureDim_3D ? dsMax(1U, curDepth/2) : depth)
		{
			if (i < mipIndex)
			{
				index += curDepth*faces;
				continue;
			}

			if (depthIndex >= curDepth)
				return DS_INVALID_TEXTURE_SURFACE;

			return index + depthIndex;
		}

		DS_ASSERT(false);
		return DS_INVALID_TEXTURE_SURFACE;
	}

	return mipIndex*depth*faces + depthIndex*faces + cubeFace;
}

size_t dsTexture_surfaceOffset(const dsTextureInfo* info, dsCubeFace cubeFace, uint32_t depthIndex,
	uint32_t mipIndex)
{
	if (!info || info->width == 0 || info->height == 0)
		return DS_INVALID_TEXTURE_OFFSET;

	uint32_t depth = dsMax(1U, info->depth);
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(info->width, info->height,
		DS_MIP_DEPTH(info->dimension, depth));
	uint32_t mipLevels = dsMin(info->mipLevels, maxMipLevels);
	mipLevels = dsMax(1U, mipLevels);

	if (depthIndex >= depth || mipIndex >= mipLevels)
		return DS_INVALID_TEXTURE_OFFSET;

	unsigned int blockX, blockY, minX, minY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
		return 0;
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, info->format));
	unsigned int formatSize = dsGfxFormat_size(info->format);
	DS_ASSERT(formatSize > 0);
	unsigned int faces = info->dimension == dsTextureDim_Cube ? 6 : 1;

	size_t size = 0;
	for (uint32_t curWidth = info->width, curHeight = info->height, curDepth = depth, mip = 0;
		mip <= mipIndex;
		curWidth = dsMax(1U, curWidth/2), curHeight = dsMax(1U, curHeight/2),
		curDepth = info->dimension == dsTextureDim_3D ? dsMax(1U, curDepth/2) : depth, ++mip)
	{
		uint32_t curBlocksX = (dsMax(curWidth, minX) + blockX - 1)/blockX;
		uint32_t curBlocksY = (dsMax(curHeight, minY) + blockY - 1)/blockY;

		size_t baseMipSize = curBlocksX*curBlocksY*formatSize;
		// Add all the depth and face levels until we reach the requested mip.
		if (mip < mipIndex)
		{
			size += baseMipSize*curDepth*faces;
			continue;
		}

		if (depthIndex >= curDepth)
			return DS_INVALID_TEXTURE_OFFSET;

		// Offset to the current depth and face index.
		size += baseMipSize*(depthIndex*faces + cubeFace);
		return size;
	}

	DS_ASSERT(false);
	return DS_INVALID_TEXTURE_OFFSET;
}

size_t dsTexture_layerOffset(const dsTextureInfo* info, uint32_t layerIndex, uint32_t mipIndex)
{
	if (!info || info->width == 0 || info->height == 0)
		return 0;

	unsigned int faces = info->dimension == dsTextureDim_Cube ? 6 : 1;
	uint32_t depth = dsMax(1U, info->depth);
	uint32_t layers = dsMax(1U, depth)*faces;
	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(info->width, info->height,
		DS_MIP_DEPTH(info->dimension, info->depth));
	uint32_t mipLevels = dsMin(info->mipLevels, maxMipLevels);
	mipLevels = dsMax(1U, mipLevels);

	if (layerIndex >= layers || mipIndex >= mipLevels)
		return DS_INVALID_TEXTURE_OFFSET;

	unsigned int blockX, blockY, minX, minY;
	if (!dsGfxFormat_blockDimensions(&blockX, &blockY, info->format))
		return 0;
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, info->format));
	unsigned int formatSize = dsGfxFormat_size(info->format);
	DS_ASSERT(formatSize > 0);

	size_t size = 0;
	for (uint32_t curWidth = info->width, curHeight = info->height, curDepth = depth, mip = 0;
		mip <= mipIndex;
		curWidth = dsMax(1U, curWidth/2), curHeight = dsMax(1U, curHeight/2),
		curDepth = info->dimension == dsTextureDim_3D ? dsMax(1U, curDepth/2) : depth, ++mip)
	{
		uint32_t curBlocksX = (dsMax(curWidth, minX) + blockX - 1)/blockX;
		uint32_t curBlocksY = (dsMax(curHeight, minY) + blockY - 1)/blockY;

		size_t baseMipSize = curBlocksX*curBlocksY*formatSize;
		// Add all the depth and face levels until we reach the requested mip.
		if (mip < mipIndex)
		{
			size += baseMipSize*curDepth*faces;
			continue;
		}

		// Offset to the current depth and face index.
		size += baseMipSize*layerIndex;
		return size;
	}

	DS_ASSERT(false);
	return DS_INVALID_TEXTURE_OFFSET;
}

dsTexture* dsTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createTextureFunc || !resourceManager->destroyTextureFunc || !info)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureInfo texInfo = *info;

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

	if ((texInfo.dimension == dsTextureDim_3D &&
		(texInfo.depth == 0 || texInfo.depth > resourceManager->maxTextureDepth)) ||
		(texInfo.dimension != dsTextureDim_3D &&
			(texInfo.depth > resourceManager->maxTextureArrayLevels)) ||
		(texInfo.dimension == dsTextureDim_Cube &&
			texInfo.depth > 0 && !resourceManager->hasCubeArrays))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture depth.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsGfxFormat_textureSupported(resourceManager, texInfo.format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for textures.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	texInfo.samples = dsMax(1U, texInfo.samples);
	if (texInfo.samples > 1)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot create a non-offscreen texture with anti-alias samples.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(texInfo.width, texInfo.height,
		DS_MIP_DEPTH(texInfo.dimension, texInfo.depth));
	texInfo.mipLevels = dsMin(texInfo.mipLevels, maxMipLevels);
	texInfo.mipLevels = dsMax(1U, texInfo.mipLevels);
	if (!resourceManager->hasArbitraryMipmapping && texInfo.mipLevels != 1 &&
		texInfo.mipLevels != maxMipLevels)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"The current target requires textures to be fully mipmapped or not mipmapped at all.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	unsigned int minWidth, minHeight;
	DS_VERIFY(dsGfxFormat_minDimensions(&minWidth, &minHeight, texInfo.format));
	if (texInfo.dimension == dsTextureDim_1D)
		texInfo.height = minHeight;

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texInfo.format));
	if (texInfo.width > resourceManager->maxTextureSize ||
		texInfo.height > resourceManager->maxTextureSize ||
		(texInfo.dimension == dsTextureDim_3D ? texInfo.depth > resourceManager->maxTextureDepth :
	texInfo.depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture dimensions.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t textureSize = dsTexture_size(&texInfo);
	if (data && size != textureSize)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTexture* texture = resourceManager->createTextureFunc(resourceManager, allocator, usage,
		memoryHints, &texInfo, data, size);
	if (texture)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, 1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, textureSize);
	}
	DS_PROFILE_FUNC_RETURN(texture);
}

dsOffscreen* dsTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, bool resolve)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createTextureFunc || !resourceManager->destroyTextureFunc || !info)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureInfo texInfo = *info;

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

	if ((texInfo.dimension == dsTextureDim_3D &&
		(texInfo.depth == 0 || texInfo.depth > resourceManager->maxTextureDepth)) ||
		(texInfo.dimension != dsTextureDim_3D &&
			(texInfo.depth > resourceManager->maxTextureArrayLevels)) ||
		(texInfo.dimension == dsTextureDim_Cube &&
			texInfo.depth > 0 && !resourceManager->hasCubeArrays))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture depth.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsGfxFormat_offscreenSupported(resourceManager, texInfo.format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for offscreens.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t maxMipLevels = dsTexture_maxMipmapLevels(texInfo.width, texInfo.height,
		DS_MIP_DEPTH(texInfo.dimension, texInfo.depth));
	texInfo.mipLevels = dsMin(texInfo.mipLevels, maxMipLevels);
	texInfo.mipLevels = dsMax(1U, texInfo.mipLevels);
	if (!resourceManager->hasArbitraryMipmapping && texInfo.mipLevels != 1 &&
		texInfo.mipLevels != maxMipLevels)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"The current target requires textures to be fully mipmapped or not mipmapped at all.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (texInfo.samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		texInfo.samples = resourceManager->renderer->surfaceSamples;
	texInfo.samples = dsMax(1U, texInfo.samples);
	if (texInfo.samples == 1)
		resolve = false;

	unsigned int minWidth, minHeight;
	DS_VERIFY(dsGfxFormat_minDimensions(&minWidth, &minHeight, texInfo.format));
	if (texInfo.dimension == dsTextureDim_1D)
		texInfo.height = minHeight;

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texInfo.format));
	if (texInfo.width > resourceManager->maxTextureSize ||
		texInfo.height > resourceManager->maxTextureSize ||
		(texInfo.dimension == dsTextureDim_3D ? texInfo.depth > resourceManager->maxTextureDepth :
			texInfo.depth > resourceManager->maxTextureArrayLevels))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture dimensions.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (texInfo.samples > resourceManager->renderer->maxSurfaceSamples)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Surface samples is above the maximum.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (texInfo.samples > resourceManager->maxTextureSamples && !resolve)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Too many samples for multisampled texture.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsOffscreen* offscreen = resourceManager->createOffscreenFunc(resourceManager, allocator,
		usage, memoryHints, &texInfo, resolve);
	if (offscreen)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, 1);
		dsTextureInfo curInfo = texInfo;
		if (resolve)
			curInfo.samples = 1;
		size_t textureSize = dsTexture_size(&curInfo);
		if (resolve)
		{
			curInfo.depth = 1;
			curInfo.mipLevels = 1;
			curInfo.samples = texInfo.samples;
			textureSize += dsTexture_size(&curInfo);
		}
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, textureSize);
	}
	DS_PROFILE_FUNC_RETURN(offscreen);
}

bool dsTexture_copyData(dsTexture* texture, dsCommandBuffer* commandBuffer,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !texture || !texture->resourceManager ||
		!texture->resourceManager->copyTextureDataFunc || !position || layers == 0 || !data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->usage & dsTextureUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texture->info.format));
	if (position->x % blockX != 0 || position->y % blockY != 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture data position must be a multiple of the block size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t mipWidth = dsMax(1U, texture->info.width >> position->mipLevel);
	uint32_t mipHeight = dsMax(1U, texture->info.height >> position->mipLevel);
	uint32_t mipLayers = dsMax(1U, texture->info.depth);
	uint32_t layerOffset = position->depth;
	if (texture->info.dimension == dsTextureDim_3D)
		mipLayers = dsMax(1U, mipLayers >> position->mipLevel);
	else if (texture->info.dimension == dsTextureDim_Cube)
	{
		mipLayers *= 6;
		layerOffset = layerOffset*6 + position->face;
	}
	uint32_t endX = position->x + width;
	uint32_t endY = position->y + height;
	uint32_t endLayer = layerOffset + mipLayers;
	if (endX > mipWidth || endY > mipHeight || endLayer > mipLayers)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((endX % blockX != 0 && endX != mipWidth) || (endY % blockY && endY != mipHeight))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture data width and height must be a multiple of the block size or reach the edge "
			"of the image.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (texture->info.dimension != dsTextureDim_Cube && position->face != dsCubeFace_None)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot copy to a specific cube face when not a cube map.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsTextureDim dimension = texture->info.dimension;
	if (dimension == dsTextureDim_Cube)
		dimension = dsTextureDim_2D;
	dsTextureInfo surfaceInfo = texture->info;
	surfaceInfo.dimension = dimension;
	surfaceInfo.width = width;
	surfaceInfo.height = height;
	surfaceInfo.depth = layers;
	surfaceInfo.mipLevels = 1;
	if (size != dsTexture_size(&surfaceInfo))
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Texture copying must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture copying must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	bool success = resourceManager->copyTextureDataFunc(resourceManager, commandBuffer, texture,
		position, width, height, layers, data, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_copy(dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsTexture* dstTexture,
	const dsTextureCopyRegion* regions, uint32_t regionCount)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !srcTexture || !dstTexture || !srcTexture->resourceManager ||
		!srcTexture->resourceManager->copyTextureFunc ||
		srcTexture->resourceManager != dstTexture->resourceManager ||
		srcTexture->info.format != dstTexture->info.format || (!regions && regionCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = srcTexture->resourceManager;
	if (!dsGfxFormat_textureCopySupported(resourceManager, srcTexture->info.format,
		dstTexture->info.format))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Textures cannot be copied between each other on the current target.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcTexture->usage & dsTextureUsage_CopyFrom))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a texture without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstTexture->usage & dsTextureUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, srcTexture->info.format));

	for (size_t i = 0; i < regionCount; ++i)
	{
		if (regions[i].srcPosition.x % blockX != 0 || regions[i].srcPosition.y % blockY != 0 ||
			regions[i].dstPosition.x % blockX != 0 || regions[i].dstPosition.y % blockY != 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data position must be a multiple of the block size.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* srcPosition = &regions[i].srcPosition;
		if (srcPosition->mipLevel >= srcTexture->info.mipLevels)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t srcMipWidth = dsMax(1U, srcTexture->info.width >> srcPosition->mipLevel);
		uint32_t srcMipHeight = dsMax(1U, srcTexture->info.height >> srcPosition->mipLevel);
		uint32_t srcMipLayers = dsMax(1U, srcTexture->info.depth);
		uint32_t srcLayerOffset = srcPosition->depth;
		if (srcTexture->info.dimension == dsTextureDim_3D)
			srcMipLayers = dsMax(1U, srcMipLayers >> srcPosition->mipLevel);
		else if (srcTexture->info.dimension == dsTextureDim_Cube)
		{
			srcMipLayers *= 6;
			srcLayerOffset = srcLayerOffset*6 + srcPosition->face;
		}
		uint32_t srcEndX = regions[i].srcPosition.x + regions[i].width;
		uint32_t srcEndY = regions[i].srcPosition.y + regions[i].height;
		uint32_t srcEndLayer = srcLayerOffset + regions[i].layers;
		if (srcEndX > srcMipWidth || srcEndY > srcMipHeight || srcEndLayer > srcMipLayers)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if ((srcEndX % blockX != 0 && srcEndX != srcMipWidth) ||
			(srcEndY % blockY != 0 && srcEndY != srcMipHeight))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data width and height must be a multiple of the block size or reach the "
				"edge of the image.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		const dsTexturePosition* dstPosition = &regions[i].dstPosition;
		if (dstPosition->mipLevel >= dstTexture->info.mipLevels)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t dstMipWidth = dsMax(1U, dstTexture->info.width >> dstPosition->mipLevel);
		uint32_t dstMipHeight = dsMax(1U, dstTexture->info.height >> dstPosition->mipLevel);
		uint32_t dstMipLayers = dsMax(1U, dstTexture->info.depth);
		uint32_t dstLayerOffset = dstPosition->depth;
		if (dstTexture->info.dimension == dsTextureDim_3D)
			dstMipLayers = dsMax(1U, dstMipLayers >> dstPosition->mipLevel);
		else if (dstTexture->info.dimension == dsTextureDim_Cube)
		{
			dstMipLayers *= 6;
			dstLayerOffset = dstLayerOffset*6 + dstPosition->face;
		}
		uint32_t dstEndX = regions[i].dstPosition.x + regions[i].width;
		uint32_t dstEndY = regions[i].dstPosition.y + regions[i].height;
		uint32_t dstEndLayer = dstLayerOffset + regions[i].layers;
		if (dstEndX > dstMipWidth || dstEndY > dstMipHeight || dstEndLayer > dstMipLayers)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if ((dstEndX % blockX != 0 && dstEndX != dstMipWidth) ||
			(dstEndY % blockY != 0 && dstEndY != dstMipHeight))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture data width and height must be a multiple of the block size or reach the "
				"edge of the image.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if ((srcTexture->info.dimension != dsTextureDim_Cube &&
				regions[i].srcPosition.face != dsCubeFace_None) ||
			(dstTexture->info.dimension != dsTextureDim_Cube &&
				regions[i].dstPosition.face != dsCubeFace_None))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Cannot copy to a specific cube face when not a cube map.");
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Texture copying must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture copying must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->copyTextureFunc(resourceManager, commandBuffer, srcTexture,
		dstTexture, regions, regionCount);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_generateMipmaps(dsTexture* texture, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !texture || !texture->resourceManager ||
		!texture->resourceManager->generateTextureMipmapsFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	if (!dsGfxFormat_generateMipmapsSupported(resourceManager, texture->info.format))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture cannot have mipmaps generated on the current target.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (texture->info.samples > 1)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot generate mipmaps for multisampled textures.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->usage & dsTextureUsage_CopyFrom) || !(texture->usage & dsTextureUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Generating mipmaps requires both the copy from and copy to usage bits to be set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (texture->info.mipLevels == 1)
		DS_PROFILE_FUNC_RETURN(true);

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Generating mipmaps must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Generating mipmaps must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->generateTextureMipmapsFunc(resourceManager, commandBuffer,
		texture);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsTexture_getData(void* result, size_t size, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	DS_PROFILE_FUNC_START();

	if (!result || !texture || !texture->resourceManager ||
		!texture->resourceManager->getTextureDataFunc || !position)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->usage & dsTextureUsage_CopyFrom))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a texture without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(texture->memoryHints & dsGfxMemory_Read))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting read from a texture without the read memory flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	if (!texture->offscreen && !resourceManager->texturesReadable)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Target doesn't support reading from a non-offscreen texture.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (texture->info.samples > 1 && !texture->resolve)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to read from a non-resolved multisampled offscreen.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texture->info.format));
	if (position->x % blockX != 0 || position->y % blockY != 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture data position must be a multiple of the block size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((position->depth > 0 && position->depth >= texture->info.depth) ||
		position->mipLevel >= texture->info.mipLevels)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t mipWidth = dsMax(1U, texture->info.width >> position->mipLevel);
	uint32_t mipHeight = dsMax(1U, texture->info.height >> position->mipLevel);
	uint32_t endX = position->x + width;
	uint32_t endY = position->y + height;
	if (endX > mipWidth || endY > mipHeight)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy texture data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((endX % blockX != 0 && endX != mipWidth) || (endY % blockY && endY != mipHeight))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture data width and height must be a multiple of the block size or reach the edge "
			"of the image.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsTextureInfo surfaceInfo = texture->info;
	surfaceInfo.width = width;
	surfaceInfo.height = height;
	surfaceInfo.depth = 1;
	surfaceInfo.mipLevels = 1;
	if (size != dsTexture_size(&surfaceInfo))
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid texture data size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->getTextureDataFunc(result, size, resourceManager, texture,
		position, width, height);
	DS_PROFILE_FUNC_RETURN(success);
}

void dsTexture_process(dsTexture* texture)
{
	DS_PROFILE_FUNC_START();

	if (!texture || !texture->resourceManager || !texture->resourceManager->processTextureFunc)
		DS_PROFILE_FUNC_RETURN_VOID();

	dsResourceManager* resourceManager = texture->resourceManager;
	resourceManager->processTextureFunc(resourceManager, texture);
	DS_PROFILE_FUNC_RETURN_VOID();
}

bool dsTexture_destroy(dsTexture* texture)
{
	if (!texture)
		return true;

	DS_PROFILE_FUNC_START();

	if (!texture->resourceManager || !texture->resourceManager->destroyTextureFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = texture->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsTextureInfo curInfo = texture->info;
	if (texture->resolve)
		curInfo.samples = 1;
	size_t size = dsTexture_size(&curInfo);
	if (texture->resolve)
	{
		curInfo.depth = 1;
		curInfo.mipLevels = 1;
		curInfo.samples = texture->info.samples;
		size += dsTexture_size(&curInfo);
	}

	bool success = resourceManager->destroyTextureFunc(resourceManager, texture);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->textureCount, -1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->textureMemorySize, -size);
	}
	DS_PROFILE_FUNC_RETURN(success);
}
