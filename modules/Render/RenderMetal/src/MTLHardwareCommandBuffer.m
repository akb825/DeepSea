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

#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"
#include "MTLShared.h"

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

inline static void assertIsHardwareCommandBuffer(dsCommandBuffer* commandBuffer);

static id<MTLCommandBuffer> getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
		return (__bridge id<MTLCommandBuffer>)(mtlCommandBuffer);

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

static id<MTLComputeCommandEncoder> getComputeCommandEncoder(dsCommandBuffer* commandBuffer)
{
	dsMTLHardwareCommandBuffer* mtlCommandBuffer = (dsMTLHardwareCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->computeCommandEncoder)
		return (__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLHardwareCommandBuffer_endEncoding(commandBuffer);

	id<MTLComputeCommandEncoder> encoder = [realCommandBuffer computeCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->computeCommandEncoder = CFBridgingRetain(encoder);
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

static void setDepthStencilState(id<MTLRenderCommandEncoder> encoder,
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
			return;

		descriptor.depthCompareFunction =
			renderStates->depthStencilState.depthTestEnable == mslBool_True ?
				dsGetMTLCompareFunction(renderStates->depthStencilState.depthCompareOp) :
				mslCompareOp_Always;
		descriptor.depthWriteEnabled =
			renderStates->depthStencilState.depthWriteEnable != mslBool_False;
		descriptor.frontFaceStencil = dsCreateMTLStencilDescriptor(
			&renderStates->depthStencilState.frontStencil, dynamicStates->frontStencilCompareMask,
			dynamicStates->frontStencilWriteMask);
		descriptor.backFaceStencil = dsCreateMTLStencilDescriptor(
			&renderStates->depthStencilState.backStencil, dynamicStates->backStencilCompareMask,
			dynamicStates->backStencilWriteMask);

		depthStencilState = [[encoder device] newDepthStencilStateWithDescriptor: descriptor];
		if (!depthStencilState)
			return;
	}

	if (renderStates->depthStencilState.stencilTestEnable == mslBool_True)
	{
		uint32_t frontReference = renderStates->depthStencilState.frontStencil.reference;
		if (frontReference == MSL_UNKNOWN && dynamicStates)
			frontReference = dynamicStates->frontStencilReference;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		uint32_t backReference = renderStates->depthStencilState.backStencil.reference;
		if (backReference == MSL_UNKNOWN && dynamicStates)
			backReference = dynamicStates->backStencilReference;
		[encoder setStencilFrontReferenceValue: frontReference backReferenceValue: backReference];
#else
		[encoder setStencilReferenceValue: frontReference];
#endif
	}

	if (dynamicOnly)
		return;

	[encoder setDepthStencilState: depthStencilState];
}

static void setDynamicDepthState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, const dsDynamicRenderStates* dynamicStates,
	bool dynamicOnly)
{
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

	if (dynamicOnly)
		return;

	[encoder setDepthBias: constBias slopeScale: slopeBias clamp: clamp];
	[encoder setDepthClipMode:
		renderStates->rasterizationState.depthClampEnable == mslBool_True ?
			MTLDepthClipModeClamp : MTLDepthClipModeClip];
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
		CFRelease(mtlCommandBuffer->mtlCommandBuffer);
		mtlCommandBuffer = NULL;
	}

	if (mtlSubmitBuffer->mtlCommandBuffer)
	{
		uint32_t index = mtlCommandBuffer->submitBufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
				mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers, 1))
		{
			return false;
		}

		mtlCommandBuffer->submitBuffers[index] = CFRetain(mtlSubmitBuffer->mtlCommandBuffer);
	}

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

	id<MTLBuffer> tempBuffer = [device newBufferWithBytes: data length:
		size options: MTLResourceCPUCacheModeDefaultCache];
	if (!tempBuffer)
	{
		errno = ENOMEM;
		return false;
	}

	[encoder copyFromBuffer: tempBuffer sourceOffset: 0 toBuffer: buffer
		destinationOffset: offset size: size];
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

	uint32_t blocksWide = (width + blocksX - 1)/blocksX;
	uint32_t blocksHigh = (width + blocksY - 1)/blocksY;
	uint32_t sliceSize = blocksWide*blocksHigh*formatSize;

	uint32_t faceCount = textureInfo->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = textureInfo->dimension == dsTextureDim_3D;
	bool is1D = textureInfo->dimension == dsTextureDim_1D;
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
			sourceOrigin: region.origin sourceSize: region.size toTexture: texture
			destinationSlice: baseSlice + i destinationLevel: position->mipLevel
			destinationOrigin: dstOrigin];
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
			[blitEncoder copyFromTexture: srcTexture sourceSlice: 0
				sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin sourceSize: size
				toTexture: dstTexture destinationSlice: 0
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

				[blitEncoder copyFromTexture: srcTexture sourceSlice: srcIs3D ? srcLayer + j : 0
					sourceLevel: region->srcPosition.mipLevel sourceOrigin: srcOrigin
					sourceSize: size toTexture: dstTexture
					destinationSlice: dstIs3D ? dstLayer + j : 0
					destinationLevel: region->dstPosition.mipLevel destinationOrigin: dstOrigin];
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

	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
	if (vertex)
		[encoder setVertexBytes: data length: size atIndex: 0];
	if (fragment)
		[encoder setFragmentBytes: data length: size atIndex: 0];
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
		[encoder setVertexBuffer: buffer offset: offset atIndex: vertexIndex];
	if (fragmentIndex != DS_MATERIAL_UNKNOWN)
		[encoder setFragmentBuffer: buffer offset: offset atIndex: vertexIndex];
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
		[encoder setVertexTexture: texture atIndex: vertexIndex];
		[encoder setVertexSamplerState: sampler atIndex: vertexIndex];
	}
	if (fragmentIndex != DS_MATERIAL_UNKNOWN)
	{
		[encoder setFragmentTexture: texture atIndex: vertexIndex];
		[encoder setFragmentSamplerState: sampler atIndex: vertexIndex];
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
	setDepthStencilState(encoder, renderStates, depthStencilState, dynamicStates, dynamicOnly);
	setDynamicDepthState(encoder, renderStates, dynamicStates, dynamicOnly);
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer,
	const void* data, uint32_t size)
{
	id<MTLComputeCommandEncoder> encoder = getComputeCommandEncoder(commandBuffer);
	if (!encoder)
		return false;

	[encoder setBytes: data length: size atIndex: 0];
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index)
{
	id<MTLComputeCommandEncoder> encoder = getComputeCommandEncoder(commandBuffer);
	if (!encoder)
		return false;

	[encoder setBuffer: buffer offset: offset atIndex: index];
	return true;
}

bool dsMTLHardwareCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index)
{
	id<MTLComputeCommandEncoder> encoder = getComputeCommandEncoder(commandBuffer);
	if (!encoder)
		return false;

	[encoder setTexture: texture atIndex: index];
	[encoder setSamplerState: sampler atIndex: index];
	return true;
}

static dsMTLCommandBufferFunctionTable hardwareCommandBufferFunctions =
{
	&dsMTLHardwareCommandBuffer_clear,
	&dsMTLHardwareCommandBuffer_end,
	&dsMTLHardwareCommandBuffer_submit,
	&dsMTLHardwareCommandBuffer_copyBufferData,
	&dsMTLHardwareCommandBuffer_copyBuffer,
	&dsMTLHardwareCommandBuffer_copyTextureData,
	&dsMTLHardwareCommandBuffer_copyTexture,
	&dsMTLHardwareCommandBuffer_generateMipmaps,
	&dsMTLHardwareCommandBuffer_bindPushConstants,
	&dsMTLHardwareCommandBuffer_bindBufferUniform,
	&dsMTLHardwareCommandBuffer_bindTextureUniform,
	&dsMTLHardwareCommandBuffer_setRenderStates,
	&dsMTLHardwareCommandBuffer_bindComputePushConstants,
	&dsMTLHardwareCommandBuffer_bindComputeBufferUniform,
	&dsMTLHardwareCommandBuffer_bindComputeTextureUniform
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
	}
}

void dsMTLHardwareCommandBuffer_submitted(dsCommandBuffer* commandBuffer, uint64_t submitCount)
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

	dsMTLCommandBuffer_shutdown((dsMTLCommandBuffer*)commandBuffer);
}
