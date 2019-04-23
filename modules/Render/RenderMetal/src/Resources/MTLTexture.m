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

#include "Resources/MTLTexture.h"

#include "Resources/MTLResourceManager.h"
#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

#import <Metal/MTLBlitCommandEncoder.h>

static dsTexture* createTextureImpl(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, bool offscreen,
	bool resolve)
{
	dsMTLRenderer* renderer = (dsMTLRenderer*)resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
	MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, info->format);
	if (pixelFormat == MTLPixelFormatInvalid)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Unknown format.");
		return nil;
	}

	dsMTLTexture* texture = DS_ALLOCATE_OBJECT(allocator, dsMTLTexture);
	if (!texture)
		return NULL;

	dsTexture* baseTexture = (dsTexture*)texture;
	baseTexture->resourceManager = resourceManager;
	baseTexture->allocator = dsAllocator_keepPointer(allocator);
	baseTexture->usage = usage;
	baseTexture->memoryHints = memoryHints;
	baseTexture->info = *info;
	baseTexture->offscreen = offscreen;
	baseTexture->resolve = resolve;

	texture->mtlTexture = NULL;
	texture->resolveTexture = NULL;
	texture->processed = false;

	texture->lifetime = dsLifetime_create(allocator, texture);
	if (!texture->lifetime)
	{
		dsMTLTexture_destroy(resourceManager, baseTexture);
		return NULL;
	}

	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
	if (!descriptor)
	{
		dsMTLTexture_destroy(resourceManager, baseTexture);
		errno = ENOMEM;
		return NULL;
	}

	if (info->samples > 1 && !resolve)
	{
		if (info->dimension != dsTextureDim_2D)
		{
			dsMTLTexture_destroy(resourceManager, baseTexture);
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG,
				"Multisampled textures may only be used with dsTextureDim_2D.");
			return NULL;
		}

#if MAC_OS_X_VERSION_MIN_REQUIRED < 101400
		if (info->depth > 0)
		{
			dsMTLTexture_destroy(resourceManager, baseTexture);
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Multisampled texture areays aren't supported.");
			return NULL;
		}
#endif

		return NULL;
	}

	switch (info->dimension)
	{
		case dsTextureDim_1D:
			if (info->depth > 0)
			{
				descriptor.textureType = MTLTextureType1DArray;
				descriptor.arrayLength = info->depth;
			}
			else
				descriptor.textureType = MTLTextureType1D;
			break;
		case dsTextureDim_2D:
			if (info->depth > 0)
			{
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
				if (info->samples > 1 && !resolve)
					descriptor.textureType = MTLTextureType2DMultisampleArray;
				else
					descriptor.textureType = MTLTextureType2DArray;
#else
				DS_ASSERT(info->samples == 1 || resolve);
				descriptor.textureType = MTLTextureType2DArray;
#endif
				descriptor.arrayLength = info->depth;
			}
			else
			{
				if (info->samples > 1 && !resolve)
					descriptor.textureType = MTLTextureType2DMultisample;
				else
					descriptor.textureType = MTLTextureType2D;
			}
			break;
		case dsTextureDim_3D:
			descriptor.textureType = MTLTextureType3D;
			descriptor.depth = info->depth;
			break;
		case dsTextureDim_Cube:
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
			if (info->depth > 0)
			{
				DS_ASSERT(resourceManager->hasCubeArrays);
				descriptor.textureType = MTLTextureTypeCubeArray;
				descriptor.arrayLength = info->depth;
			}
			else
				descriptor.textureType = MTLTextureTypeCube;
#else
			DS_ASSERT(info->depth == 0);
			descriptor.textureType = MTLTextureTypeCube;
#endif
			break;
		default:
			DS_ASSERT(false);
			break;
	}

	descriptor.pixelFormat = pixelFormat;
	descriptor.width = info->width;
	if (info->dimension == dsTextureDim_1D)
		descriptor.height = 1;
	else
		descriptor.height = info->height;
	descriptor.mipmapLevelCount = info->mipLevels;
	descriptor.sampleCount = resolve ? info->samples : 1;

	MTLResourceOptions resourceOptions;
	if (!(memoryHints & dsGfxMemory_GPUOnly) && (memoryHints & dsGfxMemory_Read))
	{
		resourceOptions = MTLResourceOptionCPUCacheModeDefault;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModeShared;
#endif
	}
	else
	{
		resourceOptions = MTLResourceOptionCPUCacheModeWriteCombined;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModePrivate;
#endif
	}

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	if (!offscreen && !(usage & dsGfxBufferUsage_CopyTo))
		resourceOptions |= MTLResourceHazardTrackingModeUntracked;
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
	// TODO: Set to memoryless for tiled rendering if/when supported.
	/*if (usage == dsTextureUsage_SubpassInput)
		resourceOptions |= MTLResourceStorageModeMemoryless;*/
#endif

	descriptor.resourceOptions = resourceOptions;

	MTLTextureUsage textureUsage = MTLTextureUsageUnknown;
	if (usage & (dsTextureUsage_Texture | dsTextureUsage_Image))
		textureUsage |= MTLTextureUsageShaderRead;
	if (usage & dsTextureUsage_Image)
		textureUsage |= MTLTextureUsageShaderWrite;
	if (usage & dsTextureUsage_CopyFrom)
		textureUsage |= MTLTextureUsagePixelFormatView;
	if (offscreen && (!resolve || info->samples == 1))
		textureUsage |= MTLTextureUsageRenderTarget;
	descriptor.usage = textureUsage;

	id<MTLTexture> mtlTexture = [device newTextureWithDescriptor: descriptor];
	if (!mtlTexture)
	{
		dsMTLTexture_destroy(resourceManager, baseTexture);
		errno = ENOMEM;
		return NULL;
	}
	texture->mtlTexture = CFBridgingRetain(mtlTexture);

	if (offscreen && resolve)
	{
		DS_ASSERT(info->samples > 1);
		descriptor.mipmapLevelCount = 1;
		descriptor.arrayLength = 1;
		descriptor.depth = 1;
		descriptor.sampleCount = info->samples;

		resourceOptions = MTLResourceOptionCPUCacheModeDefault;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModePrivate;
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		if (!(usage & dsTextureUsage_OffscreenContinue))
			resourceOptions |= MTLResourceStorageModeMemoryless;
#endif

		mtlTexture = [device newTextureWithDescriptor: descriptor];
		if (!mtlTexture)
		{
			dsMTLTexture_destroy(resourceManager, baseTexture);
			errno = ENOMEM;
			return NULL;
		}
		texture->resolveTexture = CFBridgingRetain(mtlTexture);
	}

	return baseTexture;
}

dsTexture* dsMTLTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size)
{
	DS_UNUSED(size);
	dsTexture* texture = createTextureImpl(resourceManager, allocator, usage, memoryHints, info,
		false, false);
	if (!texture)
		return NULL;

	if (data)
	{
		dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
		id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
		uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
		bool is1D = info->dimension == dsTextureDim_1D;
		bool is3D = info->dimension == dsTextureDim_3D;
		unsigned int formatSize = dsGfxFormat_size(info->format);
		unsigned int blocksX, blocksY;
		DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, info->format));

		const uint8_t* bytes = (const uint8_t*)data;
		for (uint32_t i = 0; i < info->mipLevels; ++i)
		{
			uint32_t width = info->width >> i;
			uint32_t height = info->height >> i;
			uint32_t depth = is3D ? info->depth >> i : info->depth;

			width = dsMax(width, 1U);
			height = dsMax(height, 1U);
			depth = dsMax(depth, 1U)*faceCount;

			uint32_t blocksWide = (width + blocksX - 1)/blocksX;
			uint32_t blocksHigh = (height + blocksX - 1)/blocksX;

			if (is3D)
			{
				MTLRegion region =
				{
					{0, 0, 0},
					{width, height, depth}
				};
				[realTexture replaceRegion: region mipmapLevel: i slice: 0
					withBytes: bytes + dsTexture_layerOffset(info, 0, i)
					bytesPerRow: is1D ? 0 : formatSize*blocksWide
					bytesPerImage: formatSize*blocksWide*blocksHigh];
			}
			else
			{
				MTLRegion region =
				{
					{0, 0, 0},
					{width, height, 1}
				};
				for (uint32_t j = 0; j < depth; ++j)
				{
					[realTexture replaceRegion: region mipmapLevel: i slice: j
						withBytes: bytes + dsTexture_layerOffset(info, j, i)
						bytesPerRow: is1D ? 0 : formatSize*blocksWide bytesPerImage: 0];

				}
			}
		}
	}

	return texture;
}

dsOffscreen* dsMTLTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsTextureUsage usage, dsGfxMemory memoryHints,
	const dsTextureInfo* info, bool resolve)
{
	return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, true, resolve);
}

bool dsMTLTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size)
{
	DS_UNUSED(size);
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;

	dsMTLTexture_process(resourceManager, texture);

	MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
		texture->info.format);
	DS_ASSERT(pixelFormat != MTLPixelFormatInvalid);
	MTLTextureDescriptor* descriptor =
		[MTLTextureDescriptor texture2DDescriptorWithPixelFormat: pixelFormat width: width
			height: height mipmapped: false];
	if (!descriptor)
	{
		errno = ENOMEM;
		return false;
	}

	id<MTLBlitCommandEncoder> blitEncoder = dsMTLCommandBuffer_getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	unsigned int formatSize = dsGfxFormat_size(texture->info.format);
	unsigned int blocksX, blocksY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, texture->info.format));

	uint32_t blocksWide = (width + blocksX - 1)/blocksX;
	uint32_t blocksHigh = (width + blocksY - 1)/blocksY;
	uint32_t sliceSize = blocksWide*blocksHigh*formatSize;

	uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = texture->info.dimension == dsTextureDim_3D;
	bool is1D = texture->info.dimension == dsTextureDim_1D;
	uint32_t iterations = is3D ? 1 : layers;
	uint32_t baseSlice = is3D ? 0 : position->depth*faceCount + position->face;
	const uint8_t* bytes = (const uint8_t*)data;

	MTLRegion region =
	{
		{0, 0, 0},
		{width, height, is3D ? layers: 1}
	};
	MTLOrigin dstOrigin = {position->x, position->y, is3D ? position->depth : 0};
	for (uint32_t i = 0; i < iterations; ++i)
	{
		id<MTLTexture> tempImage = [device newTextureWithDescriptor: descriptor];
		if (!tempImage)
		{
			errno = ENOMEM;
			return false;
		}

		[tempImage replaceRegion: region mipmapLevel: 0 slice: 0 withBytes: bytes + i*sliceSize
			bytesPerRow: is1D ? 0 : formatSize*blocksWide bytesPerImage: is3D ? sliceSize : 0];
		[blitEncoder copyFromTexture: tempImage sourceSlice: 0 sourceLevel: 0
			sourceOrigin: region.origin sourceSize: region.size toTexture: realTexture
			destinationSlice: baseSlice + i destinationLevel: position->mipLevel
			destinationOrigin: dstOrigin];
	}
	return true;
}

bool dsMTLTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount)
{
	dsMTLTexture* srcMtlTexture = (dsMTLTexture*)srcTexture;
	id<MTLTexture> realSrcTexture = (__bridge id<MTLTexture>)srcMtlTexture->mtlTexture;
	dsMTLTexture* dstMtlTexture = (dsMTLTexture*)dstTexture;
	id<MTLTexture> realDstTexture = (__bridge id<MTLTexture>)dstMtlTexture->mtlTexture;

	dsMTLTexture_process(resourceManager, srcTexture);
	dsMTLTexture_process(resourceManager, dstTexture);

	id<MTLBlitCommandEncoder> blitEncoder = dsMTLCommandBuffer_getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	uint32_t srcFaceCount = srcTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;

	uint32_t dstFaceCount = dstTexture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsTextureCopyRegion* region = regions + i;
		uint32_t srcLayer, srcDepth;
		if (srcIs3D)
		{
			srcLayer = 0;
			srcDepth = region->srcPosition.depth;
		}
		else
		{
			srcLayer = region->srcPosition.depth*srcFaceCount + region->srcPosition.face;
			srcDepth = 0;
		}

		uint32_t dstLayer, dstDepth;
		if (dstIs3D)
		{
			dstLayer = 0;
			dstDepth = region->dstPosition.depth;
		}
		else
		{
			dstLayer = region->dstPosition.depth*dstFaceCount + region->dstPosition.face;
			dstDepth = 0;
		}

		MTLOrigin srcOrigin = {region->srcPosition.x, region->srcPosition.y, srcDepth};
		MTLOrigin dstOrigin = {region->dstPosition.x, region->dstPosition.y, dstDepth};
		MTLSize size = {region->width, region->height, 1};
		if (srcIs3D && dstIs3D)
		{
			size.depth = region->layers;
			[blitEncoder copyFromTexture: realSrcTexture sourceSlice: 0
				sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin sourceSize: size
				toTexture: realDstTexture destinationSlice: 0
				destinationLevel: region->dstPosition.mipLevel destinationOrigin: dstOrigin];
		}
		else
		{
			for (uint32_t j = 0; j < region->layers; ++j)
			{
				if (srcIs3D)
					srcOrigin.z = srcDepth + j;
				if (dstIs3D)
					dstOrigin.z = dstDepth + j;

				[blitEncoder copyFromTexture: realSrcTexture sourceSlice: srcIs3D ? srcLayer + j : 0
					sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin
					sourceSize: size toTexture: realDstTexture
					destinationSlice: dstIs3D ? dstLayer + j : 0
					destinationLevel: region->dstPosition.mipLevel destinationOrigin: dstOrigin];
			}
		}
	}

	return true;
}

bool dsMTLTexture_generateMipmaps(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture)
{
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
	dsMTLTexture_process(resourceManager, texture);

	id<MTLBlitCommandEncoder> blitEncoder = dsMTLCommandBuffer_getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	[blitEncoder generateMipmapsForTexture: realTexture];
	return true;
}

bool dsMTLTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	DS_UNUSED(size);
	DS_UNUSED(resourceManager);
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;

	unsigned int formatSize = dsGfxFormat_size(texture->info.format);
	unsigned int blocksX, blocksY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, texture->info.format));

	uint32_t blocksWide = (width + blocksX - 1)/blocksX;
	uint32_t blocksHigh = (width + blocksY - 1)/blocksY;

	uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = texture->info.dimension == dsTextureDim_3D;
	bool is1D = texture->info.dimension == dsTextureDim_1D;

	MTLRegion region =
	{
		{position->x, position->y, is3D ? position->depth : 0},
		{width, height, 1}
	};
	[realTexture getBytes: result bytesPerRow: is1D ? 0 : formatSize*blocksWide
		bytesPerImage: is3D ? formatSize*blocksWide*blocksHigh : 0 fromRegion: region
		mipmapLevel: position->mipLevel
		slice: is3D ? 0 : position->depth*faceCount + position->face];
	return true;
}

void dsMTLTexture_process(dsResourceManager* resourceManager, dsTexture* texture)
{
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	uint32_t processed = true;
	uint32_t wasProcessed;
	DS_ATOMIC_EXCHANGE32(&mtlTexture->processed, &processed, &wasProcessed);
	if (!wasProcessed)
		dsMTLRenderer_processTexture(resourceManager->renderer, texture);
}

bool dsMTLTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture)
{
	DS_UNUSED(resourceManager);
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	dsLifetime_destroy(mtlTexture->lifetime);
	if (mtlTexture->mtlTexture)
		CFRelease(mtlTexture->mtlTexture);
	if (mtlTexture->resolveTexture)
		CFRelease(mtlTexture->resolveTexture);
	if (texture->allocator)
		DS_VERIFY(dsAllocator_free(texture->allocator, texture));
	return true;
}
