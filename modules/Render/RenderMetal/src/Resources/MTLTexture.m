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

#include "Resources/MTLGfxBuffer.h"
#include "Resources/MTLResourceManager.h"
#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"
#include "MTLShared.h"

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
	dsMTLResourceManager* mtlResourceManager = (dsMTLResourceManager*)resourceManager;
	DS_UNUSED(mtlResourceManager);
	dsRenderer* renderer = resourceManager->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, info->format);
	MTLPixelFormat stencilPixelFormat = MTLPixelFormatInvalid;
	if (pixelFormat == MTLPixelFormatInvalid)
	{
		// Need to have separate depth and stencil surfaces.
		switch (info->format)
		{
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
			case dsGfxFormat_D16S8:
				pixelFormat = MTLPixelFormatDepth16Unorm;
				stencilPixelFormat = MTLPixelFormatStencil8;
				break;
#endif
			case dsGfxFormat_D32S8_Float:
				pixelFormat = MTLPixelFormatDepth32Float;
				stencilPixelFormat = MTLPixelFormatStencil8;
				break;
			default:
				break;
		}
	}
	else if (pixelFormat == MTLPixelFormatStencil8)
	{
		stencilPixelFormat = pixelFormat;
		pixelFormat = MTLPixelFormatInvalid;
	}

	bool sharedStencil = info->format == dsGfxFormat_D16S8 || info->format == dsGfxFormat_D24S8 ||
		info->format == dsGfxFormat_D32S8_Float;

	if (pixelFormat == MTLPixelFormatInvalid && stencilPixelFormat == MTLPixelFormatInvalid)
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
	texture->copyTexture = NULL;
	texture->stencilTexture = NULL;
	texture->resolveTexture = NULL;
	texture->resolveStencilTexture = NULL;
	texture->lastUsedSubmit = DS_NOT_SUBMITTED;
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

#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101400
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
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
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
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
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

	descriptor.width = info->width;
	if (info->dimension == dsTextureDim_1D)
		descriptor.height = 1;
	else
		descriptor.height = info->height;
	descriptor.mipmapLevelCount = info->mipLevels;
	descriptor.sampleCount = resolve ? 1 : info->samples;

	MTLResourceOptions resourceOptions;
	if (offscreen && (memoryHints & dsGfxMemory_Read))
	{
		resourceOptions = MTLResourceCPUCacheModeDefaultCache;
#if DS_MAC
		if (mtlResourceManager->appleGpu)
			resourceOptions |= MTLResourceStorageModeShared;
		else
			resourceOptions |= MTLResourceStorageModeManaged;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModeShared;
#endif
	}
	else
	{
		resourceOptions = MTLResourceCPUCacheModeWriteCombined;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModePrivate;
#endif
	}

	descriptor.resourceOptions = resourceOptions;

	MTLTextureUsage textureUsage = MTLTextureUsageUnknown;
	if (usage & (dsTextureUsage_Texture | dsTextureUsage_Image | dsTextureUsage_SubpassInput))
		textureUsage |= MTLTextureUsageShaderRead;
	if (usage & dsTextureUsage_Image)
		textureUsage |= MTLTextureUsageShaderWrite;
	if (usage & dsTextureUsage_CopyFrom)
		textureUsage |= MTLTextureUsagePixelFormatView;
	if (offscreen)
		textureUsage |= MTLTextureUsageRenderTarget;
	descriptor.usage = textureUsage;

	if (pixelFormat != MTLPixelFormatInvalid)
	{
		descriptor.pixelFormat = pixelFormat;
		id<MTLTexture> mtlTexture = [device newTextureWithDescriptor: descriptor];
		if (!mtlTexture)
		{
			dsMTLTexture_destroy(resourceManager, baseTexture);
			errno = ENOMEM;
			return NULL;
		}
		texture->mtlTexture = CFBridgingRetain(mtlTexture);
	}

	if (stencilPixelFormat != MTLPixelFormatInvalid)
	{
		descriptor.pixelFormat = stencilPixelFormat;
		id<MTLTexture> mtlTexture = [device newTextureWithDescriptor: descriptor];
		if (!mtlTexture)
		{
			dsMTLTexture_destroy(resourceManager, baseTexture);
			errno = ENOMEM;
			return NULL;
		}
		texture->stencilTexture = CFBridgingRetain(mtlTexture);
	}
	else if (sharedStencil)
		texture->stencilTexture = CFRetain(texture->mtlTexture);

	if (offscreen && resolve)
	{
		DS_ASSERT(info->samples > 1);
		descriptor.textureType = MTLTextureType2DMultisample;
		descriptor.mipmapLevelCount = 1;
		descriptor.arrayLength = 1;
		descriptor.depth = 1;
		descriptor.sampleCount = info->samples;

		resourceOptions = MTLResourceCPUCacheModeDefaultCache;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModePrivate;
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		if (mtlResourceManager->appleGpu &&
			!(usage & (dsTextureUsage_CopyTo | dsTextureUsage_OffscreenContinue)))
		{
			resourceOptions |= MTLResourceStorageModeMemoryless;
		}
#endif
		descriptor.resourceOptions = resourceOptions;
		descriptor.usage = MTLTextureUsageRenderTarget;

		if (pixelFormat != MTLPixelFormatInvalid)
		{
			descriptor.pixelFormat = pixelFormat;
			id<MTLTexture>mtlTexture = [device newTextureWithDescriptor: descriptor];
			if (!mtlTexture)
			{
				dsMTLTexture_destroy(resourceManager, baseTexture);
				errno = ENOMEM;
				return NULL;
			}
			texture->resolveTexture = CFBridgingRetain(mtlTexture);
		}

		if (stencilPixelFormat != MTLPixelFormatInvalid)
		{
			descriptor.pixelFormat = stencilPixelFormat;
			id<MTLTexture>mtlTexture = [device newTextureWithDescriptor: descriptor];
			if (!mtlTexture)
			{
				dsMTLTexture_destroy(resourceManager, baseTexture);
				errno = ENOMEM;
				return NULL;
			}
			texture->resolveStencilTexture = CFBridgingRetain(mtlTexture);
		}
		else if (sharedStencil)
			texture->resolveStencilTexture = CFRetain(texture->resolveTexture);
	}

	return baseTexture;
}

dsTexture* dsMTLTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size)
{
	@autoreleasepool
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
	#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
			if (realTexture.storageMode == MTLStorageModePrivate)
			{
				dsMTLRenderer* renderer = (dsMTLRenderer*)resourceManager->renderer;
				id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
				MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
				if (!descriptor)
				{
					dsMTLTexture_destroy(resourceManager, texture);
					return NULL;
				}

				descriptor.textureType = realTexture.textureType;
				descriptor.pixelFormat = realTexture.pixelFormat;
				descriptor.width = realTexture.width;
				descriptor.height = realTexture.height;
				descriptor.depth = realTexture.depth;
				descriptor.mipmapLevelCount = realTexture.mipmapLevelCount;
				descriptor.arrayLength = realTexture.arrayLength;

				realTexture = [device newTextureWithDescriptor: descriptor];
				if (!realTexture)
				{
					dsMTLTexture_destroy(resourceManager, texture);
					return NULL;
				}

				mtlTexture->copyTexture = CFBridgingRetain(realTexture);
			}
	#endif

			uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
			bool is1D = info->dimension == dsTextureDim_1D;
			bool is3D = info->dimension == dsTextureDim_3D;
			bool isPVR = dsIsMTLFormatPVR(info->format);
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
						bytesPerRow: isPVR ? 0 : formatSize*blocksWide
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
							bytesPerRow: is1D || isPVR ? 0 : formatSize*blocksWide
							bytesPerImage: 0];

					}
				}
			}
		}

		return texture;
	}
}

dsOffscreen* dsMTLTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsTextureUsage usage, dsGfxMemory memoryHints,
	const dsTextureInfo* info, bool resolve)
{
	@autoreleasepool
	{
		return createTextureImpl(resourceManager, allocator, usage, memoryHints, info, true,
			resolve);
	}
}

bool dsMTLTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
		id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;

		dsMTLTexture_process(resourceManager, texture);
		return dsMTLCommandBuffer_copyTextureData(commandBuffer, realTexture, &texture->info,
			position, width, height, layers, data, size);
	}
}

bool dsMTLTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount)
{
	@autoreleasepool
	{
		dsMTLTexture* srcMtlTexture = (dsMTLTexture*)srcTexture;
		id<MTLTexture> realSrcTexture = (__bridge id<MTLTexture>)srcMtlTexture->mtlTexture;
		dsMTLTexture* dstMtlTexture = (dsMTLTexture*)dstTexture;
		id<MTLTexture> realDstTexture = (__bridge id<MTLTexture>)dstMtlTexture->mtlTexture;

		dsMTLTexture_process(resourceManager, srcTexture);
		dsMTLTexture_process(resourceManager, dstTexture);
		return dsMTLCommandBuffer_copyTexture(commandBuffer, realSrcTexture, realDstTexture,
			regions, regionCount);
	}
}

bool dsMTLTexture_copyToBuffer(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsGfxBuffer* dstBuffer,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		dsMTLTexture_process(resourceManager, srcTexture);
		dsMTLTexture* srcMtlTexture = (dsMTLTexture*)srcTexture;
		id<MTLTexture> realSrcTexture = (__bridge id<MTLTexture>)srcMtlTexture->mtlTexture;

		id<MTLBuffer> realDstBuffer = dsMTLGfxBuffer_getBuffer(dstBuffer, commandBuffer);
		if (!realDstBuffer)
			return false;

		return dsMTLCommandBuffer_copyTextureToBuffer(commandBuffer, realSrcTexture, realDstBuffer,
			srcTexture->info.format, regions, regionCount);
	}
}

bool dsMTLTexture_generateMipmaps(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture)
{
	@autoreleasepool
	{
		DS_UNUSED(resourceManager);
		dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
		id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
		return dsMTLCommandBuffer_generateMipmaps(commandBuffer, realTexture);
	}
}

bool dsMTLTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	@autoreleasepool
	{
		DS_UNUSED(size);
		DS_UNUSED(resourceManager);
		dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
		id<MTLTexture> realTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;

		uint64_t lastUsedSubmit;
		DS_ATOMIC_LOAD64(&mtlTexture->lastUsedSubmit, &lastUsedSubmit);
		if (lastUsedSubmit == DS_NOT_SUBMITTED)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG,
				"Trying to read to an offscreen that hasn't had a draw flushed yet.");
			return false;
		}

		dsGfxFenceResult fenceResult = dsMTLRenderer_waitForSubmit(resourceManager->renderer,
			lastUsedSubmit, DS_DEFAULT_WAIT_TIMEOUT);
		if (fenceResult == dsGfxFenceResult_WaitingToQueue)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Offscreen still queued to be rendered.");
			return false;
		}

		unsigned int formatSize = dsGfxFormat_size(texture->info.format);
		unsigned int blocksX, blocksY;
		DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, texture->info.format));

		uint32_t blocksWide = (width + blocksX - 1)/blocksX;
		uint32_t blocksHigh = (width + blocksY - 1)/blocksY;

		uint32_t faceCount = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
		bool is3D = texture->info.dimension == dsTextureDim_3D;
		bool is1D = texture->info.dimension == dsTextureDim_1D;
		bool isPVR = dsIsMTLFormatPVR(texture->info.format);

		MTLRegion region =
		{
			{position->x, position->y, is3D ? position->depth : 0},
			{width, height, 1}
		};
		[realTexture getBytes: result bytesPerRow: is1D || isPVR ? 0 : formatSize*blocksWide
			bytesPerImage: is3D ? formatSize*blocksWide*blocksHigh : 0 fromRegion: region
			mipmapLevel: position->mipLevel
			slice: is3D ? 0 : position->depth*faceCount + position->face];
		return true;
	}
}

void dsMTLTexture_process(dsResourceManager* resourceManager, dsTexture* texture)
{
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	uint32_t processed = true;
	uint32_t wasProcessed;
	DS_ATOMIC_EXCHANGE32(&mtlTexture->processed, &processed, &wasProcessed);
	if (!wasProcessed && mtlTexture->copyTexture)
		dsMTLRenderer_processTexture(resourceManager->renderer, texture);
}

bool dsMTLTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture)
{
	DS_UNUSED(resourceManager);
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
	dsLifetime_destroy(mtlTexture->lifetime);
	if (mtlTexture->mtlTexture)
		CFRelease(mtlTexture->mtlTexture);
	if (mtlTexture->copyTexture)
		CFRelease(mtlTexture->copyTexture);
	if (mtlTexture->resolveTexture)
		CFRelease(mtlTexture->resolveTexture);
	if (mtlTexture->stencilTexture)
		CFRelease(mtlTexture->stencilTexture);
	if (mtlTexture->resolveStencilTexture)
		CFRelease(mtlTexture->resolveStencilTexture);
	if (texture->allocator)
		DS_VERIFY(dsAllocator_free(texture->allocator, texture));
	return true;
}
