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

#include "MTLHardwareCommandBuffer.h"

#include "Resources/MTLResourceManager.h"
#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"
#include "MTLShared.h"
#include "MTLTempBuffer.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

#import <Metal/MTLCommandQueue.h>

typedef struct ClearVertexData
{
	dsVector4f bounds;
	float depth;
	uint32_t layer;
	float padding[2];
} ClearVertexData;

inline static void assertIsHardwareCommandBuffer(dsCommandBuffer* commandBuffer);

static id<MTLCommandBuffer> getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
		return (__bridge id<MTLCommandBuffer>)(mtlCommandBuffer->mtlCommandBuffer);

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)commandBuffer->renderer;
	id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)mtlRenderer->commandQueue;
	id<MTLCommandBuffer> newCommandBuffer = [commandQueue commandBuffer];
	if (!newCommandBuffer)
	{
		errno = ENOMEM;
		return nil;
	}

	uint32_t index = mtlCommandBuffer->submitBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
			mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers, 1))
	{
		return nil;
	}

	mtlCommandBuffer->mtlCommandBuffer = CFBridgingRetain(newCommandBuffer);
	mtlCommandBuffer->submitBuffers[index] = CFBridgingRetain(newCommandBuffer);
	return newCommandBuffer;
}

static id<MTLBlitCommandEncoder> getBlitCommandEncoder(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->blitCommandEncoder)
		return (__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);

	id<MTLBlitCommandEncoder> encoder = [realCommandBuffer blitCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->blitCommandEncoder = CFBridgingRetain(encoder);
	return encoder;
}

static bool needsDynamicDepthStencil(const mslStencilOpState* state)
{
	return state->compareMask == MSL_UNKNOWN || state->writeMask == MSL_UNKNOWN;
}

static void setRasterizationState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, const dsDynamicRenderStates* dynamicStates,
	bool dynamicOnly)
{
	dsColor4f blendConstants;
	memcpy(&blendConstants, renderStates->blendState.blendConstants, sizeof(blendConstants));
	if (blendConstants.x == MSL_UNKNOWN_FLOAT)
	{
		if (dynamicStates)
			blendConstants = dynamicStates->blendConstants;
		else
		{
			blendConstants.r = 0.0f;
			blendConstants.g = 0.0f;
			blendConstants.b = 0.0f;
			blendConstants.a = 1.0f;
		}
	}
	[encoder setBlendColorRed: blendConstants.r green: blendConstants.g blue: blendConstants.b
		alpha: blendConstants.a];

	if (dynamicOnly)
		return;

	const mslRasterizationState* rasterState = &renderStates->rasterizationState;
	[encoder setTriangleFillMode: rasterState->polygonMode == mslPolygonMode_Line ?
		MTLTriangleFillModeLines : MTLTriangleFillModeFill];
	[encoder setFrontFacingWinding: rasterState->frontFace == mslFrontFace_Clockwise ?
		MTLWindingClockwise : MTLWindingCounterClockwise];

	MTLCullMode cullMode = MTLCullModeNone;
	switch (rasterState->cullMode)
	{
		case mslCullMode_Front:
			cullMode = MTLCullModeFront;
			break;
		case mslCullMode_Back:
			cullMode = MTLCullModeBack;
			break;
		case mslCullMode_None:
		default:
			cullMode = MTLCullModeNone;
			break;
	}
	[encoder setCullMode: cullMode];
}

static id<MTLDepthStencilState> setDepthStencilState(uint32_t* outFrontStencilRef,
	uint32_t* outBackStencilRef, id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly)
{
	if (renderStates->depthStencilState.stencilTestEnable == mslBool_True &&
		dynamicStates &&
			(needsDynamicDepthStencil(&renderStates->depthStencilState.frontStencil) ||
			needsDynamicDepthStencil(&renderStates->depthStencilState.backStencil)))
	{
		MTLDepthStencilDescriptor* descriptor = [MTLDepthStencilDescriptor new];
		if (!descriptor)
			return nil;

		bool depthEnabled = renderStates->depthStencilState.depthTestEnable == mslBool_True;
		descriptor.depthCompareFunction = depthEnabled ?
			dsGetMTLCompareFunction(renderStates->depthStencilState.depthCompareOp,
				MTLCompareFunctionLess) :
			MTLCompareFunctionAlways;
		descriptor.depthWriteEnabled = depthEnabled &&
			renderStates->depthStencilState.depthWriteEnable != mslBool_False;
		descriptor.frontFaceStencil = dsCreateMTLStencilDescriptor(
			&renderStates->depthStencilState.frontStencil, dynamicStates->frontStencilCompareMask,
			dynamicStates->frontStencilWriteMask);
		descriptor.backFaceStencil = dsCreateMTLStencilDescriptor(
			&renderStates->depthStencilState.backStencil, dynamicStates->backStencilCompareMask,
			dynamicStates->backStencilWriteMask);

		depthStencilState = [[encoder device] newDepthStencilStateWithDescriptor: descriptor];
		if (!depthStencilState)
			return nil;
	}

	if (renderStates->depthStencilState.stencilTestEnable == mslBool_True)
	{
		uint32_t frontReference = renderStates->depthStencilState.frontStencil.reference;
		if (frontReference == MSL_UNKNOWN)
		{
			if (dynamicStates)
				frontReference = dynamicStates->frontStencilReference;
			else
				frontReference = 0;
		}
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		uint32_t backReference = renderStates->depthStencilState.backStencil.reference;
		if (backReference == MSL_UNKNOWN)
		{
			if (dynamicStates)
				backReference = dynamicStates->backStencilReference;
			else
				backReference = 0;
		}
		[encoder setStencilFrontReferenceValue: frontReference backReferenceValue: backReference];
#else
		[encoder setStencilReferenceValue: frontReference];
		uint32_t backReference = frontReference;
#endif
		*outFrontStencilRef = frontReference;
		*outBackStencilRef = backReference;
	}

	if (dynamicOnly)
		return nil;

	[encoder setDepthStencilState: depthStencilState];
	return depthStencilState;
}

static void setDynamicDepthState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, const dsDynamicRenderStates* dynamicStates,
	bool dynamicOnly, bool hasDepthClip)
{
	DS_UNUSED(hasDepthClip);
	if (renderStates->depthStencilState.depthWriteEnable == mslBool_False)
		return;

	float constBias = 0.0f, slopeBias = 0.0f, clamp = 0.0f;
	if (renderStates->rasterizationState.depthBiasEnable == mslBool_True)
	{
		if (renderStates->rasterizationState.depthBiasConstantFactor != MSL_UNKNOWN_FLOAT)
			constBias = renderStates->rasterizationState.depthBiasConstantFactor;
		else if (dynamicStates)
			constBias = dynamicStates->depthBiasConstantFactor;

		if (renderStates->rasterizationState.depthBiasSlopeFactor != MSL_UNKNOWN_FLOAT)
			slopeBias = renderStates->rasterizationState.depthBiasSlopeFactor;
		else if (dynamicStates)
			slopeBias = dynamicStates->depthBiasSlopeFactor;

		if (renderStates->rasterizationState.depthBiasClamp != MSL_UNKNOWN_FLOAT)
			clamp = renderStates->rasterizationState.depthBiasClamp;
		else if (dynamicStates)
			clamp = dynamicStates->depthBiasClamp;
	}
	[encoder setDepthBias: constBias slopeScale: slopeBias clamp: clamp];

	if (dynamicOnly)
		return;

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
	if (hasDepthClip)
	{
		[encoder setDepthClipMode:
			renderStates->rasterizationState.depthClampEnable == mslBool_True ?
				MTLDepthClipModeClamp : MTLDepthClipModeClip];
	}
#endif
}

static bool needToBindTexture(dsMTLBoundTextureSet* boundTextures, dsAllocator* allocator,
	uint32_t index, CFTypeRef texture, CFTypeRef sampler)
{
	if (index >= boundTextures->textureCount)
	{
		uint32_t prevCount = boundTextures->textureCount;
		uint32_t addCount = index + 1 - prevCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(allocator, boundTextures->textures,
				boundTextures->textureCount, boundTextures->maxTextures, addCount))
		{
			return true;
		}

		memset(boundTextures->textures + prevCount, 0, addCount*sizeof(dsMTLBoundTexture));
	}

	if (boundTextures->textures[index].texture == texture &&
		boundTextures->textures[index].sampler == sampler)
	{
		return false;
	}

	boundTextures->textures[index].texture = texture;
	boundTextures->textures[index].sampler = sampler;
	return true;
}

static bool needToBindBuffer(dsMTLBoundBufferSet* boundBuffers, dsAllocator* allocator,
	uint32_t index, CFTypeRef buffer, size_t offset)
{
	if (index >= boundBuffers->bufferCount)
	{
		uint32_t prevCount = boundBuffers->bufferCount;
		uint32_t addCount = index + 1 - prevCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(allocator, boundBuffers->buffers,
				boundBuffers->bufferCount, boundBuffers->maxBuffers, addCount))
		{
			return true;
		}

		memset(boundBuffers->buffers + prevCount, 0, addCount*sizeof(dsMTLBoundBuffer));
	}

	if (boundBuffers->buffers[index].buffer == buffer &&
		boundBuffers->buffers[index].offset == offset)
	{
		return false;
	}

	boundBuffers->buffers[index].buffer = buffer;
	boundBuffers->buffers[index].offset = offset;
	return true;
}

static MTLPrimitiveType getPrimitiveType(dsPrimitiveType type)
{
	switch (type)
	{
		case dsPrimitiveType_PointList:
			return MTLPrimitiveTypePoint;
		case dsPrimitiveType_LineList:
		case dsPrimitiveType_LineListAdjacency:
			return MTLPrimitiveTypeLine;
		case dsPrimitiveType_LineStrip:
			return MTLPrimitiveTypeLineStrip;
		case dsPrimitiveType_TriangleList:
		case dsPrimitiveType_TriangleListAdjacency:
			return MTLPrimitiveTypeTriangle;
		case dsPrimitiveType_TriangleStrip:
		case dsPrimitiveType_TriangleStripAdjacency:
			return MTLPrimitiveTypeTriangleStrip;
		default:
			DS_ASSERT(false);
			return MTLPrimitiveTypeTriangle;
	}
}

static MTLIndexType getIndexType(uint32_t size)
{
	return size == sizeof(uint32_t) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
}

static void* getTempBufferData(uint32_t* outOffset, id<MTLBuffer>* outMTLBuffer,
	dsCommandBuffer* commandBuffer, uint32_t size, uint32_t alignment)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->curTempBuffer)
	{
		void* data = dsMTLTempBuffer_allocate(
			outOffset, outMTLBuffer, mtlCommandBuffer->curTempBuffer, size, alignment);
		if (data)
			return data;
	}

	// Try to find a finished buffer.
	uint64_t finishedSubmitCount = dsMTLRenderer_getFinishedSubmitCount(commandBuffer->renderer);
	for (uint32_t i = 0; i < mtlCommandBuffer->tempBufferPoolCount; ++i)
	{
		dsMTLTempBuffer* tempBuffer = mtlCommandBuffer->tempBufferPool[i];
		if (!dsMTLTempBuffer_reset(tempBuffer, finishedSubmitCount))
			continue;

		mtlCommandBuffer->curTempBuffer = tempBuffer;
		dsMTLCommandBuffer_addTempBuffer(commandBuffer, tempBuffer);
		return dsMTLTempBuffer_allocate(outOffset, outMTLBuffer, tempBuffer, size, alignment);
	}

	uint32_t index = mtlCommandBuffer->tempBufferPoolCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->tempBufferPool,
			mtlCommandBuffer->tempBufferPoolCount, mtlCommandBuffer->maxTempBufferPools, 1))
	{
		return NULL;
	}

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)commandBuffer->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	dsMTLTempBuffer* tempBuffer = dsMTLTempBuffer_create(commandBuffer->allocator, device);
	if (!tempBuffer)
	{
		--mtlCommandBuffer->tempBufferPoolCount;
		return NULL;
	}

	mtlCommandBuffer->tempBufferPool[index] = tempBuffer;
	mtlCommandBuffer->curTempBuffer = tempBuffer;
	dsMTLCommandBuffer_addTempBuffer(commandBuffer, tempBuffer);
	return dsMTLTempBuffer_allocate(outOffset, outMTLBuffer, tempBuffer, size, alignment);
}

void dsMTLHardwareCommandBuffer_clear(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlCommandBuffer->submitBuffers[i])
			CFRelease(mtlCommandBuffer->submitBuffers[i]);
	}
	mtlCommandBuffer->submitBufferCount = 0;
	mtlCommandBuffer->curTempBuffer = NULL;
}

void dsMTLHardwareCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);

	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
	{
		CFRelease(mtlCommandBuffer->mtlCommandBuffer);
		mtlCommandBuffer = NULL;
	}
}

bool dsMTLHardwareCommandBuffer_submit(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	assertIsHardwareCommandBuffer(commandBuffer);
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	dsMTLHardwareCommandBuffer* mtlSubmitBuffer = (dsMTLHardwareCommandBuffer*)submitBuffer;

	if (mtlCommandBuffer->mtlCommandBuffer)
	{
		dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);
		CFRelease(mtlCommandBuffer->mtlCommandBuffer);
		mtlCommandBuffer->mtlCommandBuffer = NULL;
	}

	if (mtlSubmitBuffer->mtlCommandBuffer)
	{
		dsMTLHardwareCommandBuffer_endEncoding(submitBuffer);
		CFRelease(mtlSubmitBuffer->mtlCommandBuffer);
		mtlSubmitBuffer->mtlCommandBuffer = NULL;
	}

	uint32_t index = mtlCommandBuffer->submitBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
			mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers,
			mtlSubmitBuffer->submitBufferCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < mtlSubmitBuffer->submitBufferCount; ++i)
		mtlCommandBuffer->submitBuffers[index + i] = mtlSubmitBuffer->submitBuffers[i];
	mtlSubmitBuffer->submitBufferCount = 0;

	return true;
}

bool dsMTLHardwareCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	id<MTLBlitCommandEncoder> encoder = getBlitCommandEncoder(commandBuffer);
	if (!encoder)
		return false;

	if (size > DS_MAX_TEMP_BUFFER_ALLOC)
	{
		id<MTLBuffer> tempBuffer = [device newBufferWithBytes: data length:
			size options: MTLResourceCPUCacheModeDefaultCache];
		if (!tempBuffer)
		{
			errno = ENOMEM;
			return false;
		}

		[encoder copyFromBuffer: tempBuffer sourceOffset: 0 toBuffer: buffer
			destinationOffset: offset size: size];
	}
	else
	{
		uint32_t tempOffset = 0;
		id<MTLBuffer> tempBuffer = nil;
		void* tempData =
			getTempBufferData(&tempOffset, &tempBuffer, commandBuffer, (uint32_t)size, 4);
		if (!tempData)
			return false;

		memcpy(tempData, data, size);
		[encoder copyFromBuffer: tempBuffer sourceOffset: tempOffset toBuffer: buffer
			destinationOffset: offset size: size];
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> srcBuffer, size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset,
	size_t size)
{
	id<MTLBlitCommandEncoder> encoder = getBlitCommandEncoder(commandBuffer);
	if (!encoder)
		return false;

	[encoder copyFromBuffer: srcBuffer sourceOffset: srcOffset toBuffer: dstBuffer
		destinationOffset: dstOffset size: size];
	return true;
}

bool dsMTLHardwareCommandBuffer_copyBufferToTexture(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> srcBuffer, id<MTLTexture> dstTexture, dsGfxFormat format,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	id<MTLBlitCommandEncoder> blitEncoder = getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	uint32_t dstFaceCount = dstTexture.textureType == MTLTextureTypeCube ? 6 : 1;
	bool dstIs3D = dstTexture.textureType == MTLTextureType3D;

	unsigned int formatSize = dsGfxFormat_size(format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, format));

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;
		const dsTexturePosition* position = &region->texturePosition;
		uint32_t dstLayer, dstDepth;
		if (dstIs3D)
		{
			dstLayer = 0;
			dstDepth = position->depth;
		}
		else
		{
			dstLayer = position->depth*dstFaceCount + position->face;
			dstDepth = 0;
		}

		uint32_t bufferWidth = region->bufferWidth;
		if (bufferWidth == 0)
			bufferWidth = region->textureWidth;
		uint32_t bufferHeight = region->bufferHeight;
		if (bufferHeight == 0)
			bufferHeight = region->textureHeight;
		size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
		size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
		size_t bufferPitch = bufferXBlocks*formatSize;
		size_t bufferImageSize = bufferPitch*bufferYBlocks;

		MTLOrigin dstOrigin = {position->x, position->y, dstDepth};
		MTLSize size = {region->textureWidth, region->textureHeight, 1};
		if (dstIs3D)
		{
			size.depth = region->layers;
			[blitEncoder copyFromBuffer: srcBuffer sourceOffset: region->bufferOffset
				sourceBytesPerRow: bufferPitch sourceBytesPerImage: bufferImageSize
				sourceSize: size toTexture: dstTexture destinationSlice: 0
				destinationLevel: position->mipLevel destinationOrigin: dstOrigin];
		}
		else
		{
			for (uint32_t j = 0; j < region->layers; ++j)
			{
				[blitEncoder copyFromBuffer: srcBuffer
					sourceOffset: region->bufferOffset + bufferImageSize*j
					sourceBytesPerRow: bufferPitch sourceBytesPerImage: bufferImageSize
					sourceSize: size toTexture: dstTexture destinationSlice: dstLayer + j
					destinationLevel: position->mipLevel destinationOrigin: dstOrigin];
			}
		}
	}

	return true;
}

bool dsMTLHardwareCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, const dsTextureInfo* textureInfo, const dsTexturePosition* position,
	uint32_t width, uint32_t height, uint32_t layers, const void* data, size_t size)
{
	DS_UNUSED(size);
	dsRenderer* renderer = commandBuffer->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	MTLTextureDescriptor* descriptor =
		[MTLTextureDescriptor texture2DDescriptorWithPixelFormat: texture.pixelFormat width: width
			height: height mipmapped: false];
	if (!descriptor)
	{
		errno = ENOMEM;
		return false;
	}

	id<MTLBlitCommandEncoder> blitEncoder = getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	unsigned int formatSize = dsGfxFormat_size(textureInfo->format);
	unsigned int blocksX, blocksY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, textureInfo->format));

	size_t blocksWide = (width + blocksX - 1)/blocksX;
	size_t blocksHigh = (height + blocksY - 1)/blocksY;
	size_t baseSize = blocksWide*blocksHigh*formatSize;
	DS_ASSERT(baseSize*layers == size);

	uint32_t faceCount = textureInfo->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = textureInfo->dimension == dsTextureDim_3D;
	bool is1D = textureInfo->dimension == dsTextureDim_1D;
	bool isPVR = dsIsMTLFormatPVR(textureInfo->format);
	uint32_t iterations = is3D ? 1 : layers;
	uint32_t baseSlice = is3D ? 0 : position->depth*faceCount + position->face;
	size_t iterationSize = is3D ? baseSize*layers : baseSize;
	const uint8_t* bytes = (const uint8_t*)data;

	MTLRegion region =
	{
		{0, 0, 0},
		{width, height, is3D ? layers: 1}
	};
	MTLOrigin dstOrigin = {position->x, position->y, is3D ? position->depth : 0};
	if (isPVR)
	{
		for (uint32_t i = 0; i < iterations; ++i)
		{
			id<MTLTexture> tempImage = [device newTextureWithDescriptor: descriptor];
			if (!tempImage)
			{
				errno = ENOMEM;
				return false;
			}

			[tempImage replaceRegion: region mipmapLevel: 0 slice: 0 withBytes: bytes + i*baseSize
				bytesPerRow: is1D || isPVR ? 0 : formatSize*blocksWide
				bytesPerImage: is3D ? baseSize : 0];
			[blitEncoder copyFromTexture: tempImage sourceSlice: 0 sourceLevel: 0
				sourceOrigin: region.origin sourceSize: region.size toTexture: texture
				destinationSlice: baseSlice + i destinationLevel: position->mipLevel
				destinationOrigin: dstOrigin];
		}
	}
	else
	{
		if (iterationSize > DS_MAX_TEMP_BUFFER_ALLOC)
		{
			id<MTLBuffer> tempBuffer = [device newBufferWithBytes: data length: size
				options: MTLResourceCPUCacheModeDefaultCache];
			if (!tempBuffer)
				return false;

			for (uint32_t i = 0; i < iterations; ++i)
			{
				[blitEncoder copyFromBuffer: tempBuffer sourceOffset: baseSize*i
					sourceBytesPerRow: blocksWide*formatSize sourceBytesPerImage: baseSize
					sourceSize: region.size toTexture: texture destinationSlice: baseSlice + i
					destinationLevel: position->mipLevel destinationOrigin: dstOrigin];
			}
		}
		else
		{
			for (uint32_t i = 0; i < iterations; ++i)
			{
				uint32_t offset = 0;
				id<MTLBuffer> tempBuffer = nil;
				void* tempData = getTempBufferData(&offset, &tempBuffer, commandBuffer,
					(uint32_t)iterationSize, formatSize);
				if (!tempData)
					return false;

				memcpy(tempData, bytes + i*iterationSize, iterationSize);
				[blitEncoder copyFromBuffer: tempBuffer sourceOffset: offset
					sourceBytesPerRow: blocksWide*formatSize sourceBytesPerImage: baseSize
					sourceSize: region.size toTexture: texture destinationSlice: baseSlice + i
					destinationLevel: position->mipLevel destinationOrigin: dstOrigin];
			}
		}
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount)
{
	id<MTLBlitCommandEncoder> blitEncoder = getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	uint32_t srcFaceCount = srcTexture.textureType == MTLTextureTypeCube ? 6 : 1;
	bool srcIs3D = srcTexture.textureType == MTLTextureType3D;

	uint32_t dstFaceCount = dstTexture.textureType == MTLTextureTypeCube ? 6 : 1;
	bool dstIs3D = dstTexture.textureType == MTLTextureType3D;

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
			[blitEncoder copyFromTexture: srcTexture sourceSlice: srcLayer
				sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin sourceSize: size
				toTexture: dstTexture destinationSlice: dstLayer
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

				[blitEncoder copyFromTexture: srcTexture sourceSlice: srcIs3D ? 0 : srcLayer + j
					sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin
					sourceSize: size toTexture: dstTexture
					destinationSlice: dstIs3D ? 0 : dstLayer + j
					destinationLevel: region->dstPosition.mipLevel destinationOrigin: dstOrigin];
			}
		}
	}

	return true;
}

bool dsMTLHardwareCommandBuffer_copyTextureToBuffer(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLBuffer> dstBuffer, dsGfxFormat format,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	id<MTLBlitCommandEncoder> blitEncoder = getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	uint32_t srcFaceCount = srcTexture.textureType == MTLTextureTypeCube ? 6 : 1;
	bool srcIs3D = srcTexture.textureType == MTLTextureType3D;

	unsigned int formatSize = dsGfxFormat_size(format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, format));

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;
		const dsTexturePosition* position = &region->texturePosition;
		uint32_t srcLayer, srcDepth;
		if (srcIs3D)
		{
			srcLayer = 0;
			srcDepth = position->depth;
		}
		else
		{
			srcLayer = position->depth*srcFaceCount + position->face;
			srcDepth = 0;
		}

		uint32_t bufferWidth = region->bufferWidth;
		if (bufferWidth == 0)
			bufferWidth = region->textureWidth;
		uint32_t bufferHeight = region->bufferHeight;
		if (bufferHeight == 0)
			bufferHeight = region->textureHeight;
		size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
		size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
		size_t bufferPitch = bufferXBlocks*formatSize;
		size_t bufferImageSize = bufferPitch*bufferYBlocks;

		MTLOrigin srcOrigin = {position->x, position->y, srcDepth};
		MTLSize size = {region->textureWidth, region->textureHeight, 1};
		if (srcIs3D)
		{
			size.depth = region->layers;
			[blitEncoder copyFromTexture: srcTexture sourceSlice: 0 sourceLevel: position->mipLevel
				sourceOrigin: srcOrigin sourceSize: size toBuffer: dstBuffer
				destinationOffset: region->bufferOffset destinationBytesPerRow: bufferPitch
				destinationBytesPerImage: bufferImageSize];
		}
		else
		{
			for (uint32_t j = 0; j < region->layers; ++j)
			{
				[blitEncoder copyFromTexture: srcTexture sourceSlice: srcLayer + j
					sourceLevel: position->mipLevel sourceOrigin: srcOrigin sourceSize: size
					toBuffer: dstBuffer destinationOffset: region->bufferOffset + bufferImageSize*j
					destinationBytesPerRow: bufferPitch destinationBytesPerImage: bufferImageSize];
			}
		}
	}

	return true;
}

bool dsMTLHardwareCommandBuffer_generateMipmaps(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture)
{
	id<MTLBlitCommandEncoder> blitEncoder = getBlitCommandEncoder(commandBuffer);
	if (!blitEncoder)
		return false;

	[blitEncoder generateMipmapsForTexture: texture];
	return true;
}

bool dsMTLHardwareCommandBuffer_bindPushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	uint32_t alignment = commandBuffer->renderer->resourceManager->minUniformBlockAlignment;
	uint32_t offset = 0;
	id<MTLBuffer> mtlBuffer = nil;
	void* tempData = getTempBufferData(&offset, &mtlBuffer, commandBuffer, size, alignment);
	if (!tempData)
		return false;

	memcpy(tempData, data, size);

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (vertex)
		[encoder setVertexBuffer: mtlBuffer offset: offset atIndex: 0];
	if (fragment)
		[encoder setFragmentBuffer: mtlBuffer offset: offset atIndex: 0];
	return true;
}

bool dsMTLHardwareCommandBuffer_bindBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (vertexIndex != DS_MATERIAL_UNKNOWN)
	{
		if (needToBindBuffer(mtlCommandBuffer->boundBuffers + 0, commandBuffer->allocator,
				vertexIndex, (__bridge CFTypeRef)buffer, offset))
		{
			[encoder setVertexBuffer: buffer offset: offset atIndex: vertexIndex];
		}
	}
	if (fragmentIndex != DS_MATERIAL_UNKNOWN)
	{
		if (needToBindBuffer(mtlCommandBuffer->boundBuffers + 1, commandBuffer->allocator,
				fragmentIndex, (__bridge CFTypeRef)buffer, offset))
		{
			[encoder setFragmentBuffer: buffer offset: offset atIndex: fragmentIndex];
		}
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_bindTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t vertexIndex,
	uint32_t fragmentIndex)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (vertexIndex != DS_MATERIAL_UNKNOWN)
	{
		if (needToBindTexture(mtlCommandBuffer->boundTextures + 0, commandBuffer->allocator,
				vertexIndex, (__bridge CFTypeRef)texture, (__bridge CFTypeRef)sampler))
		{
			[encoder setVertexTexture: texture atIndex: vertexIndex];
			[encoder setVertexSamplerState: sampler atIndex: vertexIndex];
		}
	}
	if (fragmentIndex != DS_MATERIAL_UNKNOWN)
	{
		if (needToBindTexture(mtlCommandBuffer->boundTextures + 1, commandBuffer->allocator,
				fragmentIndex, (__bridge CFTypeRef)texture, (__bridge CFTypeRef)sampler))
		{
			[encoder setFragmentTexture: texture atIndex: fragmentIndex];
			[encoder setFragmentSamplerState: sampler atIndex: fragmentIndex];
		}
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_setRenderStates(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	setRasterizationState(encoder, renderStates, dynamicStates, dynamicOnly);
	id<MTLDepthStencilState> boundDepthStencilState =
		setDepthStencilState(&mtlCommandBuffer->curFrontStencilRef,
			&mtlCommandBuffer->curBackStencilRef, encoder, renderStates, depthStencilState,
			dynamicStates, dynamicOnly);
	if (boundDepthStencilState)
	{
		if (mtlCommandBuffer->boundDepthStencil)
			CFRelease(mtlCommandBuffer->boundDepthStencil);
		mtlCommandBuffer->boundDepthStencil = CFBridgingRetain(boundDepthStencilState);
	}

	setDynamicDepthState(encoder, renderStates, dynamicStates, dynamicOnly,
		commandBuffer->renderer->hasDepthClamp);
	return true;
}

bool dsMTLHardwareCommandBuffer_beginComputeShader(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->computeCommandEncoder)
		return true;

	id<MTLCommandBuffer> submitBuffer = getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);
	id<MTLComputeCommandEncoder> encoder = [submitBuffer computeCommandEncoder];
	if (!encoder)
		return false;

	mtlCommandBuffer->computeCommandEncoder = CFBridgingRetain(encoder);
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer,
	const void* data, uint32_t size)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->computeCommandEncoder)
		return false;

	uint32_t alignment = commandBuffer->renderer->resourceManager->minUniformBlockAlignment;
	uint32_t offset = 0;
	id<MTLBuffer> mtlBuffer = nil;
	void* tempData = getTempBufferData(&offset, &mtlBuffer, commandBuffer, size, alignment);
	if (!tempData)
		return false;

	memcpy(tempData, data, size);

	id<MTLComputeCommandEncoder> encoder =
		(__bridge id<MTLComputeCommandEncoder>)mtlCommandBuffer->computeCommandEncoder;
	[encoder setBuffer: mtlBuffer offset: offset atIndex: 0];
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->computeCommandEncoder)
		return false;

	id<MTLComputeCommandEncoder> encoder =
		(__bridge id<MTLComputeCommandEncoder>)mtlCommandBuffer->computeCommandEncoder;
	if (needToBindBuffer(&mtlCommandBuffer->boundComputeBuffers, commandBuffer->allocator, index,
			(__bridge CFTypeRef)buffer, offset))
	{
		[encoder setBuffer: buffer offset: offset atIndex: index];
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->computeCommandEncoder)
		return false;

	id<MTLComputeCommandEncoder> encoder =
		(__bridge id<MTLComputeCommandEncoder>)mtlCommandBuffer->computeCommandEncoder;
	if (needToBindTexture(&mtlCommandBuffer->boundComputeTextures, commandBuffer->allocator,
			index, (__bridge CFTypeRef)texture, (__bridge CFTypeRef)sampler))
	{
		[encoder setTexture: texture atIndex: index];
		[encoder setSamplerState: sampler atIndex: index];
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	MTLRenderPassDescriptor* renderPass, const dsAlignedBox3f* viewport)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	id<MTLCommandBuffer> submitBuffer = getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);
	id<MTLRenderCommandEncoder> encoder =
		[submitBuffer renderCommandEncoderWithDescriptor: renderPass];
	if (!encoder)
		return false;

	mtlCommandBuffer->renderCommandEncoder = CFBridgingRetain(encoder);
	MTLViewport mtlViewport = {viewport->min.x, viewport->min.y, viewport->max.x - viewport->min.x,
		viewport->max.y - viewport->min.y, viewport->min.z, viewport->max.z};
	[encoder setViewport: mtlViewport];
	return true;
}

bool dsMTLHardwareCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return true;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	[encoder endEncoding];
	CFRelease(mtlCommandBuffer->renderCommandEncoder);
	mtlCommandBuffer->renderCommandEncoder = NULL;

	for (uint32_t i = 0; i < 2; ++i)
	{
		dsMTLBoundTextureSet* boundTextures = mtlCommandBuffer->boundTextures + i;
		memset(boundTextures->textures, 0, sizeof(dsMTLBoundTexture)*boundTextures->textureCount);
		dsMTLBoundBufferSet* boundBuffers = mtlCommandBuffer->boundBuffers + i;
		memset(boundBuffers->buffers, 0, sizeof(dsMTLBoundBuffer)*boundBuffers->bufferCount);
	}
	mtlCommandBuffer->boundPipeline = NULL;
	if (mtlCommandBuffer->boundDepthStencil)
	{
		CFRelease(mtlCommandBuffer->boundDepthStencil);
		mtlCommandBuffer->boundDepthStencil = NULL;
	}

	return true;
}

bool dsMTLHardwareCommandBuffer_setViewport(dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport)
{
	// Hooked up directly to renderer function pointer, so need autorelease pool here.
	@autoreleasepool
	{
		dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
		if (!mtlCommandBuffer->renderCommandEncoder)
			return false;

		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
		MTLViewport mtlViewport = {viewport->min.x, viewport->min.y,
			viewport->max.x - viewport->min.x, viewport->max.y - viewport->min.y, viewport->min.z,
			viewport->max.z};
		[encoder setViewport: mtlViewport];
		return true;
	}
}

bool dsMTLHardwareCommandBuffer_clearAttachments(dsCommandBuffer* commandBuffer,
	const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount)
{
	// Hooked up directly to renderer function pointer, so need autorelease pool here.
	@autoreleasepool
	{
		dsRenderer* renderer = commandBuffer->renderer;
		dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)commandBuffer->renderer;
		dsResourceManager* resourceManager = renderer->resourceManager;
		const dsRenderPass* renderPass = commandBuffer->boundRenderPass;
		const dsRenderSubpassInfo* subpass = renderPass->subpasses +
			commandBuffer->activeRenderSubpass;
		const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
		dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;

		uint32_t samples = DS_DEFAULT_ANTIALIAS_SAMPLES;
		MTLPixelFormat colorFormats[DS_MAX_ATTACHMENTS];
		uint32_t colorMask = 0;
		for (uint32_t i = 0; i < DS_MAX_ATTACHMENTS; ++i)
		{
			colorFormats[i] = MTLPixelFormatInvalid;
			if (i >= subpass->colorAttachmentCount ||
				subpass->colorAttachments[i].attachmentIndex == DS_NO_ATTACHMENT)
			{
				continue;
			}

			dsGfxFormat format =
				renderPass->attachments[subpass->colorAttachments[i].attachmentIndex].format;
			colorFormats[i] = dsMTLResourceManager_getPixelFormat(resourceManager, format);
		}
		MTLPixelFormat depthFormat = MTLPixelFormatInvalid;
		MTLPixelFormat stencilFormat = MTLPixelFormatInvalid;
		bool clearDepth = false;
		bool clearStencil = false;
		if (subpass->depthStencilAttachment.attachmentIndex != DS_NO_ATTACHMENT)
		{
			dsGfxFormat format =
				renderPass->attachments[subpass->depthStencilAttachment.attachmentIndex].format;
			depthFormat = dsGetMTLDepthFormat(resourceManager, format);
			stencilFormat = dsGetMTLDepthFormat(resourceManager, format);
		}

		dsVector4f colors[DS_MAX_ATTACHMENTS] = {};
		float depth = 1.0f;
		uint32_t stencil = 0;

		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			const dsClearAttachment* clearAttachment = attachments + i;
			uint32_t attachmentIndex = DS_NO_ATTACHMENT;
			if (clearAttachment->colorAttachment == DS_NO_ATTACHMENT)
			{
				attachmentIndex = subpass->depthStencilAttachment.attachmentIndex;
				depth = clearAttachment->clearValue.depthStencil.depth;
				stencil = clearAttachment->clearValue.depthStencil.stencil;
				switch (clearAttachment->clearDepthStencil)
				{
					case dsClearDepthStencil_Depth:
						clearDepth = true;
						break;
					case dsClearDepthStencil_Stencil:
						clearStencil = true;
						break;
					case dsClearDepthStencil_Both:
						clearDepth = true;
						clearStencil = true;
						break;
				}
			}
			else
			{
				attachmentIndex =
					subpass->colorAttachments[clearAttachment->colorAttachment].attachmentIndex;

				dsGfxFormat format = renderPass->attachments[attachmentIndex].format;
				colorMask |= 1 << i;
				switch (format & dsGfxFormat_DecoratorMask)
				{
					case dsGfxFormat_UInt:
						colors[i].r = (float)clearAttachment->clearValue.colorValue.uintValue[0];
						colors[i].g = (float)clearAttachment->clearValue.colorValue.uintValue[1];
						colors[i].b = (float)clearAttachment->clearValue.colorValue.uintValue[2];
						colors[i].a = (float)clearAttachment->clearValue.colorValue.uintValue[3];
						break;
					case dsGfxFormat_SInt:
						colors[i].r = (float)clearAttachment->clearValue.colorValue.intValue[0];
						colors[i].g = (float)clearAttachment->clearValue.colorValue.intValue[1];
						colors[i].b = (float)clearAttachment->clearValue.colorValue.intValue[2];
						colors[i].a = (float)clearAttachment->clearValue.colorValue.intValue[3];
						break;
					default:
						colors[i] = clearAttachment->clearValue.colorValue.floatValue;
						break;
				}
			}

			if (attachmentIndex != DS_NO_ATTACHMENT)
				samples = renderPass->attachments[attachmentIndex].samples;
		}
		if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			samples = renderer->surfaceSamples;

		id<MTLRenderPipelineState> pipeline = dsMTLRenderer_getClearPipeline(renderer, colorFormats,
			colorMask, depthFormat, stencilFormat, framebuffer->layers > 1, samples);
		if (!pipeline)
			return false;

		[encoder setRenderPipelineState: pipeline];
		if (clearDepth && clearStencil)
		{
			[encoder setDepthStencilState:
				(__bridge id<MTLDepthStencilState>)mtlRenderer->clearDepthStencilState];
		}
		else if (clearDepth && !clearStencil)
		{
			[encoder setDepthStencilState:
				(__bridge id<MTLDepthStencilState>)mtlRenderer->clearDepthState];
		}
		else if (!clearDepth && clearStencil)
		{
			[encoder setDepthStencilState:
				(__bridge id<MTLDepthStencilState>)mtlRenderer->clearStencilState];
		}
		else
		{
			[encoder setDepthStencilState:
				(__bridge id<MTLDepthStencilState>)mtlRenderer->clearNoDepthStencilState];
		}

		if (clearStencil)
			[encoder setStencilReferenceValue: stencil];
		[encoder setFragmentBytes: colors length: sizeof(colors) atIndex: 0];
		[encoder setVertexBuffer: (__bridge id<MTLBuffer>)mtlRenderer->clearVertices offset: 0
			atIndex: 1];

		for (uint32_t i = 0; i < regionCount; ++i)
		{
			const dsAttachmentClearRegion* region = regions + i;
			ClearVertexData data;
			data.bounds.x = ((float)region->x/(float)framebuffer->width)*2.0f - 1.0f;
			data.bounds.y = ((float)(framebuffer->height - (region->y + region->height))/
				(float)framebuffer->height)*2.0f - 1.0f;
			data.bounds.z = ((float)(region->x + region->height)/(float)framebuffer->width)*2.0f -
				1.0f;
			data.bounds.w = ((float)(framebuffer->height - region->y)/
				(float)framebuffer->height)*2.0f - 1.0f;
			data.depth = depth;
			for (uint32_t j = 0; j < region->layerCount; ++j)
			{
				data.layer = j;
				[encoder setVertexBytes: &data length: sizeof(data) atIndex: 0];
				[encoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart: 0 vertexCount: 6];
			}
		}

		// Dirty appropriate states.
		mtlCommandBuffer->boundPipeline = NULL;
		if (mtlCommandBuffer->boundDepthStencil)
		{
			[encoder setDepthStencilState:
				(__bridge id<MTLDepthStencilState>)mtlCommandBuffer->boundDepthStencil];
		}
		if (clearStencil)
		{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
			[encoder setStencilFrontReferenceValue: mtlCommandBuffer->curFrontStencilRef
				backReferenceValue: mtlCommandBuffer->curBackStencilRef];
#else
			[encoder setStencilReferenceValue: mtlCommandBuffer->curFrontStencilRef];
#endif
		}
		for (uint32_t i = 0; i < 2; ++i)
		{
			if (i < mtlCommandBuffer->boundBuffers[0].bufferCount)
				mtlCommandBuffer->boundBuffers[0].buffers[i].buffer = NULL;
		}
		if (mtlCommandBuffer->boundBuffers[1].bufferCount > 0)
			mtlCommandBuffer->boundBuffers[1].buffers[0].buffer = NULL;
		return true;
	}
}

bool dsMTLHardwareCommandBuffer_draw(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, const dsDrawRange* drawRange,
	dsPrimitiveType primitiveType)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (mtlCommandBuffer->boundPipeline != (__bridge CFTypeRef)pipeline)
	{
		[encoder setRenderPipelineState: pipeline];
		mtlCommandBuffer->boundPipeline = (__bridge CFTypeRef)pipeline;
	}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	if (commandBuffer->renderer->hasStartInstance || DS_MAC)
	{
		[encoder drawPrimitives: getPrimitiveType(primitiveType) vertexStart: drawRange->firstVertex
			vertexCount: drawRange->vertexCount instanceCount: drawRange->instanceCount
			baseInstance: drawRange->firstInstance];
	}
	else
#endif
	{
		[encoder drawPrimitives: getPrimitiveType(primitiveType) vertexStart: drawRange->firstVertex
			vertexCount: drawRange->vertexCount instanceCount: drawRange->instanceCount];
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, const dsDrawIndexedRange* drawRange, dsPrimitiveType primitiveType)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (mtlCommandBuffer->boundPipeline != (__bridge CFTypeRef)pipeline)
	{
		[encoder setRenderPipelineState: pipeline];
		mtlCommandBuffer->boundPipeline = (__bridge CFTypeRef)pipeline;
	}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	if (commandBuffer->renderer->hasStartInstance || DS_MAC)
	{
		[encoder drawIndexedPrimitives: getPrimitiveType(primitiveType)
			indexCount: drawRange->indexCount indexType: getIndexType(indexSize)
			indexBuffer: indexBuffer
			indexBufferOffset: indexOffset + drawRange->firstIndex*indexSize
			instanceCount: drawRange->instanceCount baseVertex: drawRange->vertexOffset
			baseInstance: drawRange->firstInstance];
	}
	else
#endif
	{
		[encoder drawIndexedPrimitives: getPrimitiveType(primitiveType)
			indexCount: drawRange->indexCount indexType: getIndexType(indexSize)
			indexBuffer: indexBuffer
			indexBufferOffset: indexOffset + drawRange->firstIndex*indexSize
			instanceCount: drawRange->instanceCount];
	}
	return true;
}

bool dsMTLHardwareCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (mtlCommandBuffer->boundPipeline != (__bridge CFTypeRef)pipeline)
	{
		[encoder setRenderPipelineState: pipeline];
		mtlCommandBuffer->boundPipeline = (__bridge CFTypeRef)pipeline;
	}

	for (uint32_t i = 0; i < count; ++i)
	{
		[encoder drawPrimitives: getPrimitiveType(primitiveType) indirectBuffer: indirectBuffer
			indirectBufferOffset: offset + i*stride];
	}
	return true;
#else
	DS_UNUSED(commandBuffer);
	DS_UNUSED(pipeline);
	DS_UNUSED(indirectBuffer);
	DS_UNUSED(offset);
	DS_UNUSED(count);
	DS_UNUSED(stride);
	DS_UNUSED(primitiveType);
	return false;
#endif
}

bool dsMTLHardwareCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, id<MTLBuffer> indirectBuffer, size_t indirectOffset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->renderCommandEncoder)
		return false;

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (mtlCommandBuffer->boundPipeline != (__bridge CFTypeRef)pipeline)
	{
		[encoder setRenderPipelineState: pipeline];
		mtlCommandBuffer->boundPipeline = (__bridge CFTypeRef)pipeline;
	}

	for (uint32_t i = 0; i < count; ++i)
	{
		[encoder drawIndexedPrimitives: getPrimitiveType(primitiveType)
			indexType:getIndexType(indexSize) indexBuffer: indexBuffer
			indexBufferOffset: indexOffset indirectBuffer: indirectBuffer
			indirectBufferOffset: indirectOffset + i*stride];
	}
	return true;
#else
	DS_UNUSED(commandBuffer);
	DS_UNUSED(pipeline);
	DS_UNUSED(indexBuffer);
	DS_UNUSED(indexOffset);
	DS_UNUSED(indexSize);
	DS_UNUSED(indirectBuffer);
	DS_UNUSED(indirectOffset);
	DS_UNUSED(count);
	DS_UNUSED(stride);
	DS_UNUSED(primitiveType);
	return false;
#endif
}

bool dsMTLHardwareCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, uint32_t x, uint32_t y, uint32_t z,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->computeCommandEncoder)
		return false;

	id<MTLComputeCommandEncoder> encoder =
		(__bridge id<MTLComputeCommandEncoder>)mtlCommandBuffer->computeCommandEncoder;
	CFTypeRef computePipelineRef = (__bridge CFTypeRef)computePipeline;
	if (computePipelineRef != mtlCommandBuffer->boundComputePipeline)
	{
		[encoder setComputePipelineState: computePipeline];
		mtlCommandBuffer->boundComputePipeline = computePipelineRef;
	}

	[encoder dispatchThreadgroups: MTLSizeMake(x, y, z)
		threadsPerThreadgroup: MTLSizeMake(groupX, groupY, groupZ)];
	return true;
}

bool dsMTLHardwareCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, id<MTLBuffer> buffer, size_t offset,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (!mtlCommandBuffer->computeCommandEncoder)
		return false;

	id<MTLComputeCommandEncoder> encoder =
		(__bridge id<MTLComputeCommandEncoder>)mtlCommandBuffer->computeCommandEncoder;
	CFTypeRef computePipelineRef = (__bridge CFTypeRef)computePipeline;
	if (computePipelineRef != mtlCommandBuffer->boundComputePipeline)
	{
		[encoder setComputePipelineState: computePipeline];
		mtlCommandBuffer->boundComputePipeline = computePipelineRef;
	}

	[encoder dispatchThreadgroupsWithIndirectBuffer: buffer indirectBufferOffset: offset
		threadsPerThreadgroup: MTLSizeMake(groupX, groupY, groupZ)];
	return true;
#else
	DS_UNUSED(commandBuffer);
	DS_UNUSED(computePipeline);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(groupX);
	DS_UNUSED(groupY);
	DS_UNUSED(groupZ);
	return false;
#endif
}

bool dsMTLHardwareCommandBuffer_pushDebugGroup(dsCommandBuffer* commandBuffer, const char* name)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
	id<MTLCommandBuffer> submitBuffer = getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	[submitBuffer pushDebugGroup: [NSString stringWithUTF8String: name]];
	return true;
#else
	DS_UNUSED(commandBuffer);
	DS_UNUSED(name);
	return true;
#endif
}

bool dsMTLHardwareCommandBuffer_popDebugGroup(dsCommandBuffer* commandBuffer)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
	id<MTLCommandBuffer> submitBuffer = getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	[submitBuffer popDebugGroup];
	return true;
#else
	DS_UNUSED(commandBuffer);
	return true;
#endif
}

static dsMTLCommandBufferFunctionTable hardwareCommandBufferFunctions =
{
	&dsMTLHardwareCommandBuffer_clear,
	&dsMTLHardwareCommandBuffer_end,
	&dsMTLHardwareCommandBuffer_submit,
	&dsMTLHardwareCommandBuffer_copyBufferData,
	&dsMTLHardwareCommandBuffer_copyBuffer,
	&dsMTLHardwareCommandBuffer_copyBufferToTexture,
	&dsMTLHardwareCommandBuffer_copyTextureData,
	&dsMTLHardwareCommandBuffer_copyTexture,
	&dsMTLHardwareCommandBuffer_copyTextureToBuffer,
	&dsMTLHardwareCommandBuffer_generateMipmaps,
	&dsMTLHardwareCommandBuffer_bindPushConstants,
	&dsMTLHardwareCommandBuffer_bindBufferUniform,
	&dsMTLHardwareCommandBuffer_bindTextureUniform,
	&dsMTLHardwareCommandBuffer_setRenderStates,
	&dsMTLHardwareCommandBuffer_beginComputeShader,
	&dsMTLHardwareCommandBuffer_bindComputePushConstants,
	&dsMTLHardwareCommandBuffer_bindComputeBufferUniform,
	&dsMTLHardwareCommandBuffer_bindComputeTextureUniform,
	&dsMTLHardwareCommandBuffer_beginRenderPass,
	&dsMTLHardwareCommandBuffer_endRenderPass,
	&dsMTLHardwareCommandBuffer_setViewport,
	&dsMTLHardwareCommandBuffer_clearAttachments,
	&dsMTLHardwareCommandBuffer_draw,
	&dsMTLHardwareCommandBuffer_drawIndexed,
	&dsMTLHardwareCommandBuffer_drawIndirect,
	&dsMTLHardwareCommandBuffer_drawIndexedIndirect,
	&dsMTLHardwareCommandBuffer_dispatchCompute,
	&dsMTLHardwareCommandBuffer_dispatchComputeIndirect,
	&dsMTLHardwareCommandBuffer_pushDebugGroup,
	&dsMTLHardwareCommandBuffer_popDebugGroup
};

inline static void assertIsHardwareCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_ASSERT(((dsMTLCommandBuffer*)commandBuffer)->functions == &hardwareCommandBufferFunctions);
}

void dsMTLHardwareCommandBuffer_initialize(dsMTLHardwareCommandBuffer* commandBuffer,
	dsRenderer* renderer, dsAllocator* allocator, dsCommandBufferUsage usage)
{
	memset(commandBuffer, 0, sizeof(dsMTLHardwareCommandBuffer));
	dsMTLCommandBuffer_initialize((dsMTLCommandBuffer*)commandBuffer, renderer, allocator, usage,
		&hardwareCommandBufferFunctions);
}

void dsMTLHardwareCommandBuffer_endEncoding(dsCommandBuffer* commandBuffer)
{
	// Render encoder is fully managed by render passes.
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	DS_ASSERT(!mtlCommandBuffer->renderCommandEncoder);

	if (mtlCommandBuffer->blitCommandEncoder)
	{
		id<MTLBlitCommandEncoder> encoder =
			(__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->blitCommandEncoder);
		mtlCommandBuffer->blitCommandEncoder = NULL;
	}

	if (mtlCommandBuffer->computeCommandEncoder)
	{
		id<MTLComputeCommandEncoder> encoder =
			(__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->computeCommandEncoder);
		mtlCommandBuffer->computeCommandEncoder = NULL;

		dsMTLBoundTextureSet* boundTextures = &mtlCommandBuffer->boundComputeTextures;
		memset(boundTextures->textures, 0, sizeof(dsMTLBoundTexture)*boundTextures->textureCount);
		dsMTLBoundBufferSet* boundBuffers = &mtlCommandBuffer->boundComputeBuffers;
		memset(boundBuffers->buffers, 0, sizeof(dsMTLBoundBuffer)*boundBuffers->bufferCount);
		mtlCommandBuffer->boundComputePipeline = NULL;
	}
}

id<MTLCommandBuffer> dsMTLHardwareCommandBuffer_submitted(dsCommandBuffer* commandBuffer,
	uint64_t submitCount)
{
	dsMTLHardwareCommandBuffer* mtlHardwareCommandBuffer =
		(dsMTLHardwareCommandBuffer*)commandBuffer;
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlHardwareCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlHardwareCommandBuffer->submitBuffers[i])
			CFRelease(mtlHardwareCommandBuffer->submitBuffers[i]);
	}
	mtlHardwareCommandBuffer->submitBufferCount = 0;

	if (mtlHardwareCommandBuffer->mtlCommandBuffer)
	{
		CFRelease(mtlHardwareCommandBuffer->mtlCommandBuffer);
		mtlHardwareCommandBuffer->mtlCommandBuffer = NULL;
	}

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->gfxBuffers[i];
		dsMTLGfxBufferData* bufferData = (dsMTLGfxBufferData*)dsLifetime_acquire(lifetime);
		if (bufferData)
		{
			DS_ATOMIC_STORE64(&bufferData->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->tempBufferCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->tempBuffers[i];
		dsMTLTempBuffer* tempBuffer = (dsMTLTempBuffer*)dsLifetime_acquire(lifetime);
		if (tempBuffer)
		{
			DS_ATOMIC_STORE64(&tempBuffer->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->tempBufferCount = 0;
	mtlHardwareCommandBuffer->curTempBuffer = NULL;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->fences[i];
		dsMTLGfxFence* fence = (dsMTLGfxFence*)dsLifetime_acquire(lifetime);
		if (fence)
		{
			DS_ATOMIC_STORE64(&fence->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->fenceCount = 0;

	if (mtlCommandBuffer->readbackOffscreenCount == 0)
		return nil;

#if DS_MAC
	dsMTLRenderer* renderer = (dsMTLRenderer*)commandBuffer->renderer;
	id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)renderer->commandQueue;
	id<MTLCommandBuffer> submitBuffer = [commandQueue commandBuffer];
	if (!submitBuffer)
		return nil;

	id<MTLBlitCommandEncoder> encoder = [submitBuffer blitCommandEncoder];
	if (!encoder)
		return nil;
#endif

	for (uint32_t i = 0; i < mtlCommandBuffer->readbackOffscreenCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->readbackOffscreens[i];
		dsMTLTexture* texture = (dsMTLTexture*)dsLifetime_acquire(lifetime);
		if (texture)
		{
#if DS_MAC
			id<MTLTexture> realTexture = (__bridge id<MTLTexture>)texture->mtlTexture;
			[encoder synchronizeResource: realTexture];
#endif
			DS_ATOMIC_STORE64(&texture->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->readbackOffscreenCount = 0;

#if DS_MAC
	[encoder endEncoding];
	return submitBuffer;
#else
	return nil;
#endif
}

void dsMTLHardwareCommandBuffer_shutdown(dsMTLHardwareCommandBuffer* commandBuffer)
{
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	// Not initialized yet.
	if (!allocator)
		return;

	if (commandBuffer->mtlCommandBuffer)
		CFRelease(commandBuffer->mtlCommandBuffer);
	if (commandBuffer->renderCommandEncoder)
		CFRelease(commandBuffer->renderCommandEncoder);
	if (commandBuffer->blitCommandEncoder)
		CFRelease(commandBuffer->blitCommandEncoder);
	if (commandBuffer->computeCommandEncoder)
		CFRelease(commandBuffer->computeCommandEncoder);

	for (uint32_t i = 0; i < commandBuffer->submitBufferCount; ++i)
	{
		if (commandBuffer->submitBuffers[i])
			CFRelease(commandBuffer->submitBuffers[i]);
	}
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->submitBuffers));

	for (uint32_t i = 0; i < 2; ++i)
	{
		DS_VERIFY(dsAllocator_free(allocator, commandBuffer->boundTextures[i].textures));
		DS_VERIFY(dsAllocator_free(allocator, commandBuffer->boundBuffers[i].buffers));
	}
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->boundComputeTextures.textures));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->boundComputeBuffers.buffers));

	if (commandBuffer->boundDepthStencil)
		CFRelease(commandBuffer->boundDepthStencil);

	for (uint32_t i = 0; i < commandBuffer->tempBufferPoolCount; ++i)
		dsMTLTempBuffer_destroy(commandBuffer->tempBufferPool[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->tempBufferPool));

	dsMTLCommandBuffer_shutdown((dsMTLCommandBuffer*)commandBuffer);
}
