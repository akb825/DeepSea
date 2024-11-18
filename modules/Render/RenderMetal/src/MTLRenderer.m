/*
 * Copyright 2019-2023 Aaron Barany
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

#include <DeepSea/RenderMetal/MTLRenderer.h>
#include "MTLRendererInternal.h"

#include "Resources/MTLGfxBuffer.h"
#include "Resources/MTLResourceManager.h"
#include "Resources/MTLShader.h"
#include "MTLCommandBuffer.h"
#include "MTLCommandBufferPool.h"
#include "MTLHardwareCommandBuffer.h"
#include "MTLRenderPass.h"
#include "MTLRenderSurface.h"
#include "MTLShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Math/Core.h>

#include <limits.h>
#include <string.h>

#import <Metal/MTLBlitCommandEncoder.h>
#import <Metal/MTLCommandQueue.h>
#import <Metal/MTLLibrary.h>
#import <QuartzCore/CAMetalLayer.h>

#if DS_IOS && __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
#error iPhone target version must be >= 8.0 to support metal.
#endif

#if DS_MAC && __MAC_OS_X_VERSION_MIN_REQUIRED < 101100
#error macOS target version must be >= 10.11 to support metal.
#endif

static size_t dsMTLRenderer_fullAllocSize(size_t deviceNameLen)
{
	return DS_ALIGNED_SIZE(sizeof(dsMTLRenderer)) + DS_ALIGNED_SIZE(deviceNameLen + 1) +
		dsConditionVariable_fullAllocSize() + dsMutex_fullAllocSize();
}

static uint32_t getShaderVersion(void)
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 180000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 150000
	return DS_ENCODE_VERSION(3, 2, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 170000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 140000
	return DS_ENCODE_VERSION(3, 1, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 160000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 130000
	return DS_ENCODE_VERSION(3, 0, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 150000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
	return DS_ENCODE_VERSION(2, 4, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 140000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000
	return DS_ENCODE_VERSION(2, 3, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500
	return DS_ENCODE_VERSION(2, 2, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 120000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
	return DS_ENCODE_VERSION(2, 1, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	return DS_ENCODE_VERSION(2, 0, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
	return DS_ENCODE_VERSION(1, 2, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
	return DS_ENCODE_VERSION(1, 1, 0);
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
	return DS_ENCODE_VERSION(1, 0, 0);
#else
#error Metal not supported on this macOS/iOS version!
#endif
}

static uint32_t getMaxColorAttachments(id<MTLDevice> device)
{
	DS_UNUSED(device);
#if __IPHONE_OS_VERSION_MIN_REQUIRED == 80000
	return 4;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000
	return [device supportsFamily: MTLGPUFamilyApple2] ? 8 : 4;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	return [device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily2_v1] ? 8 : 4;
#else
	return 8;
#endif
}

static uint32_t getMaxSurfaceSamples(id<MTLDevice> device)
{
	if ([device supportsTextureSampleCount: 8])
		return 8;
	return 4;
}

static uint32_t hasTessellationShaders(id<MTLDevice> device)
{
	/*
	 * Tessellation shaders on Metal are... strange... Apple in their infinite wisdom
	 * decided instead of making adjustments to their render pipeline structures to have
	 * it work like literally every other graphics API, they decided to shoehorn it in a
	 * very strange way that requires multiple manual pipieline stages:
	 * 1. Have a graphics pipeline with just a vertex shader for the initial vertex shader.
	 *    Use a buffer to capture the output to use later.
	 * 2. Run a compute shader for the tessellation control. This will require ending the
	 *    render encoding and starting a compute encoding to run the compute shader. The
	 *    vertex shader output is passed as an input, and the patch output is captured in
	 *    another buffer.
	 * 3. A new render encoding needs to be created, and uses the tessellation evaluation
	 *    shader as the "vertex" function. This pipeline has some properties set to run the
	 *    tessellation stage.
	 *
	 * Note that stopping/restartng the render encoder can make some optimizations
	 * impossible, such as memoryless render targets since the render contents need to be
	 * preserved between the render pass invocations.
	 *
	 * *Can* this be implemented? Sure, MoltenVK does it and it's obvious why it took so
	 * long to add tessellation shader support. *Will* this be implemented? Currently no.
	 *
	 * I don't see this as critical enough to spend the time to implement this, especially
	 * given that some Vulkan drivers (e.g. Qualcomm) don't implement tessellation.
	 * Tessellation already has its own set of performance issues (e.g. drawing text by
	 * tessellating points into quads is slower even on desktop GPUs), and having to
	 * stop/restart the rendering encoder and manage the buffers will make this even slower
	 * compared to the driver providing a proper interface.
	 *
	 * So for the time being, no tessellation on Metal.
	 */

	DS_UNUSED(device);
#if DS_IOS && __IPHONE_OS_VERSION_MIN_REQUIRED < 100000
	return false;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
	//return [device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily3_v2];
	return false;
#elif __MAC_OS_X_VERSION_MIN_REQUIRED == 101000
	return false;
#else
	//return true;
	return false;
#endif
}

static bool createSharedResources(dsMTLRenderer* renderer, id<MTLDevice> device,
	id<MTLCommandQueue> commandQueue)
{
	MTLDepthStencilDescriptor* depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	if (!depthStencilDescriptor)
	{
		errno = ENOMEM;
		return false;
	}

	id<MTLDepthStencilState> clearNoDepthStencilState =
		[device newDepthStencilStateWithDescriptor: depthStencilDescriptor];
	if (!clearNoDepthStencilState)
	{
		errno = ENOMEM;
		return false;
	}

	depthStencilDescriptor.depthWriteEnabled = true;
	id<MTLDepthStencilState> clearDepthState =
		[device newDepthStencilStateWithDescriptor: depthStencilDescriptor];
	if (!clearDepthState)
	{
		errno = ENOMEM;
		return false;
	}

	// Set both front and back so the cull state doesn't need to be set.
	depthStencilDescriptor.frontFaceStencil.depthStencilPassOperation =
		MTLStencilOperationReplace;
	depthStencilDescriptor.backFaceStencil.depthStencilPassOperation =
		MTLStencilOperationReplace;
	id<MTLDepthStencilState> clearDepthStencilState =
		[device newDepthStencilStateWithDescriptor: depthStencilDescriptor];
	if (!clearDepthStencilState)
	{
		errno = ENOMEM;
		return false;
	}

	depthStencilDescriptor.depthWriteEnabled = false;
	id<MTLDepthStencilState> clearStencilState =
		[device newDepthStencilStateWithDescriptor: depthStencilDescriptor];
	if (!clearStencilState)
	{
		errno = ENOMEM;
		return false;
	}

	dsVector2f clearVertexData[] =
	{
		{{-1.0f, -1.0f}},
		{{ 1.0f, -1.0f}},
		{{-1.0f,  1.0f}},
		{{ 1.0f, -1.0f}},
		{{ 1.0f,  1.0f}},
		{{-1.0f,  1.0f}}
	};
	id<MTLBuffer> clearVertices = [device newBufferWithBytes: clearVertexData
		length: sizeof(clearVertexData) options: MTLResourceCPUCacheModeDefaultCache];
	if (!clearVertices)
	{
		errno = ENOMEM;
		return false;
	}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	id<MTLCommandBuffer> processCommandBuffer = [commandQueue commandBuffer];
	if (!processCommandBuffer)
	{
		errno = ENOMEM;
		return false;
	}

	id<MTLBlitCommandEncoder> encoder = [processCommandBuffer blitCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return false;
	}

	id<MTLBuffer> deviceClearVertices = [device newBufferWithLength: sizeof(clearVertexData)
		options: MTLResourceStorageModePrivate];
	if (!deviceClearVertices)
	{
		errno = ENOMEM;
		return false;
	}

	[encoder copyFromBuffer: clearVertices sourceOffset: 0 toBuffer: deviceClearVertices
		destinationOffset: 0 size: sizeof(clearVertexData)];
	[encoder endEncoding];
	[processCommandBuffer commit];
	clearVertices = deviceClearVertices;
#else
	DS_UNUSED(commandQueue);
#endif

	renderer->clearNoDepthStencilState = CFBridgingRetain(clearNoDepthStencilState);
	renderer->clearDepthState = CFBridgingRetain(clearDepthState);
	renderer->clearStencilState = CFBridgingRetain(clearStencilState);
	renderer->clearDepthStencilState = CFBridgingRetain(clearDepthStencilState);
	renderer->clearVertices = CFBridgingRetain(clearVertices);
	return true;
}

static id<MTLCommandBuffer> processResources(dsMTLRenderer* renderer)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)renderer->commandQueue;
	id<MTLCommandBuffer> resourceCommandBuffer = nil;
	id<MTLBlitCommandEncoder> encoder = nil;

	DS_VERIFY(dsSpinlock_lock(&renderer->processBuffersLock));
	if (renderer->processBufferCount > 0)
	{
		if (!encoder)
		{
			resourceCommandBuffer = [queue commandBuffer];
			if (resourceCommandBuffer)
				encoder = [resourceCommandBuffer blitCommandEncoder];
		}

		if (!encoder)
		{
			for (uint32_t i = 0; i < renderer->processBufferCount; ++i)
				dsLifetime_freeRef(renderer->processBuffers[i]);
			renderer->processBufferCount = 0;
			DS_VERIFY(dsSpinlock_unlock(&renderer->processBuffersLock));
			return nil;
		}

		for (uint32_t i = 0; i < renderer->processBufferCount; ++i)
		{
			dsLifetime* lifetime = renderer->processBuffers[i];
			dsMTLGfxBufferData* buffer = (dsMTLGfxBufferData*)dsLifetime_acquire(lifetime);
			if (buffer)
			{
				id<MTLBuffer> srcBuffer = (__bridge id<MTLBuffer>)buffer->copyBuffer;
				id<MTLBuffer> dstBuffer = (__bridge id<MTLBuffer>)buffer->mtlBuffer;
				DS_ASSERT(srcBuffer);
				DS_ASSERT(dstBuffer);
				[encoder copyFromBuffer: srcBuffer sourceOffset: 0 toBuffer: dstBuffer
					destinationOffset: 0 size: srcBuffer.length];
				CFRelease(buffer->copyBuffer);
				buffer->copyBuffer = NULL;
				dsLifetime_release(lifetime);
			}
			dsLifetime_freeRef(lifetime);
		}
		renderer->processBufferCount = 0;

	}
	DS_VERIFY(dsSpinlock_unlock(&renderer->processBuffersLock));

	DS_VERIFY(dsSpinlock_lock(&renderer->processTexturesLock));
	if (renderer->processTextureCount > 0)
	{
		if (!encoder)
		{
			resourceCommandBuffer = [queue commandBuffer];
			if (resourceCommandBuffer)
				encoder = [resourceCommandBuffer blitCommandEncoder];
		}

		if (!encoder)
		{
			for (uint32_t i = 0; i < renderer->processTextureCount; ++i)
				dsLifetime_freeRef(renderer->processTextures[i]);
			renderer->processTextureCount = 0;
			DS_VERIFY(dsSpinlock_unlock(&renderer->processTexturesLock));
			return nil;
		}

		for (uint32_t i = 0; i < renderer->processTextureCount; ++i)
		{
			dsLifetime* lifetime = renderer->processTextures[i];
			dsTexture* texture = (dsTexture*)dsLifetime_acquire(lifetime);
			if (texture)
			{
				dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;
				id<MTLTexture> srcTexture = (__bridge id<MTLTexture>)mtlTexture->copyTexture;
				id<MTLTexture> dstTexture = (__bridge id<MTLTexture>)mtlTexture->mtlTexture;
				DS_ASSERT(srcTexture);
				DS_ASSERT(dstTexture);

				const dsTextureInfo* info = &texture->info;
				uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
				uint32_t slices = (uint32_t)srcTexture.arrayLength*faceCount;
				MTLOrigin origin = {0, 0, 0};
				for (uint32_t i = 0; i < info->mipLevels; ++i)
				{
					MTLSize size = {dsMax(1U, info->width >> i), dsMax(1U, info->height >> i),
						dsMax(1U, srcTexture.depth >> i)};
					for (uint32_t j = 0; j < slices; ++j)
					{
						[encoder copyFromTexture: srcTexture sourceSlice: j sourceLevel: i
							sourceOrigin: origin sourceSize: size toTexture:
							dstTexture destinationSlice: j destinationLevel: i
							destinationOrigin: origin];
					}
				}
				dsLifetime_release(lifetime);
			}
			dsLifetime_freeRef(lifetime);
		}
		renderer->processTextureCount = 0;

	}
	DS_VERIFY(dsSpinlock_unlock(&renderer->processTexturesLock));

	if (encoder)
		[encoder endEncoding];
	return resourceCommandBuffer;
#else
	return nil;
#endif
}

static bool bindVertexBuffers(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDrawGeometry* geometry, int32_t vertexOffset)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	if (mtlCommandBuffer->boundGeometry == geometry &&
		mtlCommandBuffer->firstVertexBuffer == mtlShader->firstVertexBuffer &&
		mtlCommandBuffer->vertexOffset == vertexOffset)
	{
		return true;
	}

	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		const dsVertexBuffer* vertexBuffer = geometry->vertexBuffers + i;
		if (!vertexBuffer->buffer)
			continue;

		uint32_t bufferIndex = mtlShader->firstVertexBuffer + i;
		id<MTLBuffer> mtlBuffer = dsMTLGfxBuffer_getBuffer(vertexBuffer->buffer, commandBuffer);
		if (!mtlBuffer)
			return false;

		size_t offset = vertexBuffer->offset + vertexBuffer->format.size*vertexOffset;
		if (!dsMTLCommandBuffer_bindBufferUniform(commandBuffer, mtlBuffer, offset, bufferIndex,
				DS_MATERIAL_UNKNOWN))
		{
			return false;
		}
	}

	mtlCommandBuffer->boundGeometry = geometry;
	mtlCommandBuffer->firstVertexBuffer = mtlShader->firstVertexBuffer;
	mtlCommandBuffer->vertexOffset = vertexOffset;
	return true;
}

bool dsMTLRenderer_destroy(dsRenderer* renderer)
{
	@autoreleasepool
	{
		dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;

		dsRenderer_shutdownResources(renderer);

		// Check the function manually so we only wait if initialization completed.
		if (renderer->waitUntilIdleFunc)
			dsRenderer_waitUntilIdle(renderer);

		if (mtlRenderer->clearNoDepthStencilState)
			CFRelease(mtlRenderer->clearNoDepthStencilState);
		if (mtlRenderer->clearDepthState)
			CFRelease(mtlRenderer->clearDepthState);
		if (mtlRenderer->clearStencilState)
			CFRelease(mtlRenderer->clearStencilState);
		if (mtlRenderer->clearDepthStencilState)
			CFRelease(mtlRenderer->clearDepthStencilState);
		if (mtlRenderer->clearVertices)
			CFRelease(mtlRenderer->clearVertices);

		if (mtlRenderer->submitMutex)
		{
			DS_ASSERT(mtlRenderer->submitCondition);
			DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
			mtlRenderer->finishedSubmitCount = DS_NOT_SUBMITTED;
			DS_VERIFY(dsConditionVariable_notifyAll(mtlRenderer->submitCondition));
			DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));

			dsConditionVariable_destroy(mtlRenderer->submitCondition);
			dsMutex_destroy(mtlRenderer->submitMutex);
		}

		for (uint32_t i = 0; i < mtlRenderer->processBufferCount; ++i)
			dsLifetime_freeRef(mtlRenderer->processBuffers[i]);
		DS_VERIFY(dsAllocator_free(renderer->allocator, mtlRenderer->processBuffers));
		dsSpinlock_shutdown(&mtlRenderer->processBuffersLock);

		for (uint32_t i = 0; i < mtlRenderer->processTextureCount; ++i)
			dsLifetime_freeRef(mtlRenderer->processTextures[i]);
		DS_VERIFY(dsAllocator_free(renderer->allocator, mtlRenderer->processTextures));
		dsSpinlock_shutdown(&mtlRenderer->processTexturesLock);

		for (uint32_t i = 0; i < mtlRenderer->clearPipelineCount; ++i)
			CFRelease(mtlRenderer->clearPipelines[i].pipeline);
		DS_VERIFY(dsAllocator_free(renderer->allocator, mtlRenderer->clearPipelines));
		dsSpinlock_shutdown(&mtlRenderer->clearPipelinesLock);

		dsMTLHardwareCommandBuffer_shutdown(&mtlRenderer->mainCommandBuffer);
		dsMTLResourceManager_destroy(renderer->resourceManager);

		if (mtlRenderer->device)
			CFRelease(mtlRenderer->device);
		if (mtlRenderer->commandQueue)
			CFRelease(mtlRenderer->commandQueue);
		DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));
		return true;
	}
}

bool dsMTLRenderer_isSupported(void)
{
#if DS_MAC
	@autoreleasepool
	{
		return [MTLCopyAllDevices() count] > 0;
	}
#else
	return true;
#endif
}

bool dsMTLRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	DS_UNUSED(outDevices);
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	*outDeviceCount = 0;
	return true;
}

bool dsMTLRenderer_beginFrame(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	return true;
}

bool dsMTLRenderer_endFrame(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	return true;
}

bool dsMTLRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	renderer->surfaceSamples = samples;
	return true;
}

bool dsMTLRenderer_setDefaultSamples(dsRenderer* renderer, uint32_t samples)
{
	renderer->defaultSamples = samples;
	return true;
}

bool dsMTLRenderer_setVSync(dsRenderer* renderer, dsVSync vsync)
{
	renderer->vsync = vsync;
	return true;
}

bool dsMTLRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy)
{
	renderer->defaultAnisotropy = anisotropy;
	return true;
}

bool dsMTLRenderer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		if (primitiveType == dsPrimitiveType_TriangleFan)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Metal doesn't support drawing triangle fans.");
			return false;
		}

		dsShader* shader = (dsShader*)commandBuffer->boundShader;
		DS_ASSERT(shader);
		id<MTLRenderPipelineState> pipeline = dsMTLShader_getPipeline(shader, commandBuffer,
			primitiveType, geometry);
		if (!pipeline)
			return false;

		if (!bindVertexBuffers(commandBuffer, shader, geometry, 0))
			return false;

		return dsMTLCommandBuffer_draw(commandBuffer, pipeline, drawRange, primitiveType);
	}
}

bool dsMTLRenderer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		if (primitiveType == dsPrimitiveType_TriangleFan)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Metal doesn't support drawing triangle fans.");
			return false;
		}

		dsShader* shader = (dsShader*)commandBuffer->boundShader;
		DS_ASSERT(shader);
		id<MTLRenderPipelineState> pipeline = dsMTLShader_getPipeline(shader, commandBuffer,
			primitiveType, geometry);
		if (!pipeline)
			return false;

		DS_ASSERT(geometry->indexBuffer.buffer);
		id<MTLBuffer> indexBuffer = dsMTLGfxBuffer_getBuffer(geometry->indexBuffer.buffer,
			commandBuffer);
		if (!indexBuffer)
			return false;

		int32_t vertexOffset = renderer->hasStartInstance ? 0 : drawRange->vertexOffset;
		if (!bindVertexBuffers(commandBuffer, shader, geometry, vertexOffset))
			return false;

		return dsMTLCommandBuffer_drawIndexed(commandBuffer, pipeline, indexBuffer,
			geometry->indexBuffer.offset, geometry->indexBuffer.indexSize, drawRange,
			primitiveType);
	}
}

bool dsMTLRenderer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		if (primitiveType == dsPrimitiveType_TriangleFan)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Metal doesn't support drawing triangle fans.");
			return false;
		}

		dsShader* shader = (dsShader*)commandBuffer->boundShader;
		DS_ASSERT(shader);
		id<MTLRenderPipelineState> pipeline = dsMTLShader_getPipeline(shader, commandBuffer,
			primitiveType, geometry);
		if (!pipeline)
			return false;

		id<MTLBuffer> mtlIndirectBuffer = dsMTLGfxBuffer_getBuffer((dsGfxBuffer*)indirectBuffer,
			commandBuffer);
		if (!mtlIndirectBuffer)
			return false;

		if (!bindVertexBuffers(commandBuffer, shader, geometry, 0))
			return false;

		return dsMTLCommandBuffer_drawIndirect(commandBuffer, pipeline, mtlIndirectBuffer, offset,
			count, stride, primitiveType);
	}
}

bool dsMTLRenderer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		if (primitiveType == dsPrimitiveType_TriangleFan)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Metal doesn't support drawing triangle fans.");
			return false;
		}

		dsShader* shader = (dsShader*)commandBuffer->boundShader;
		DS_ASSERT(shader);
		id<MTLRenderPipelineState> pipeline = dsMTLShader_getPipeline(shader, commandBuffer,
			primitiveType, geometry);
		if (!pipeline)
			return false;

		DS_ASSERT(geometry->indexBuffer.buffer);
		id<MTLBuffer> indexBuffer = dsMTLGfxBuffer_getBuffer(geometry->indexBuffer.buffer,
			commandBuffer);
		if (!indexBuffer)
			return false;

		id<MTLBuffer> mtlIndirectBuffer = dsMTLGfxBuffer_getBuffer((dsGfxBuffer*)indirectBuffer,
			commandBuffer);
		if (!mtlIndirectBuffer)
			return false;

		if (!bindVertexBuffers(commandBuffer, shader, geometry, 0))
			return false;

		return dsMTLCommandBuffer_drawIndexedIndirect(commandBuffer, pipeline, indexBuffer,
			geometry->indexBuffer.offset, geometry->indexBuffer.indexSize, mtlIndirectBuffer,
			offset, count, stride, primitiveType);
	}
}

bool dsMTLRenderer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	uint32_t x, uint32_t y, uint32_t z)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		const dsMTLShader* shader = (const dsMTLShader*)commandBuffer->boundComputeShader;
		DS_ASSERT(shader);

		id<MTLComputePipelineState> computePipeline =
			(__bridge id<MTLComputePipelineState>)shader->computePipeline;
		DS_ASSERT(computePipeline);

		const uint32_t* computeLocalSize = shader->pipeline.computeLocalSize;
		return dsMTLCommandBuffer_dispatchCompute(commandBuffer, computePipeline, x, y, z,
			computeLocalSize[0], computeLocalSize[1], computeLocalSize[2]);
	}
}

bool dsMTLRenderer_dispatchComputeIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		const dsMTLShader* shader = (const dsMTLShader*)commandBuffer->boundComputeShader;
		DS_ASSERT(shader);

		id<MTLComputePipelineState> computePipeline =
			(__bridge id<MTLComputePipelineState>)shader->computePipeline;
		DS_ASSERT(computePipeline);

		id<MTLBuffer> mtlIndirectBuffer = dsMTLGfxBuffer_getBuffer((dsGfxBuffer*)indirectBuffer,
			commandBuffer);
		if (!mtlIndirectBuffer)
			return false;

		const uint32_t* computeLocalSize = shader->pipeline.computeLocalSize;
		return dsMTLCommandBuffer_dispatchComputeIndirect(commandBuffer, computePipeline,
			mtlIndirectBuffer, offset, computeLocalSize[0], computeLocalSize[1],
			computeLocalSize[2]);
	}
}

bool dsMTLRenderer_pushDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const char* name)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		return dsMTLCommandBuffer_pushDebugGroup(commandBuffer, name);
	}
}

bool dsMTLRenderer_popDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		return dsMTLCommandBuffer_popDebugGroup(commandBuffer);
	}
}

bool dsMTLRenderer_flush(dsRenderer* renderer)
{
	@autoreleasepool
	{
		dsMTLRenderer_flushImpl(renderer, nil);
		return true;
	}
}

bool dsMTLRenderer_waitUntilIdle(dsRenderer* renderer)
{
	@autoreleasepool
	{
		uint64_t submit = dsMTLRenderer_flushImpl(renderer, nil);
		return dsMTLRenderer_waitForSubmit(renderer, submit, 0xFFFFFFFF) ==
			dsGfxFenceResult_Success;
	}
}

dsRenderer* dsMTLRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
{
	@autoreleasepool
	{
		if (!allocator || !options)
		{
			errno = EINVAL;
			return NULL;
		}

		if (!allocator->freeFunc)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG,
				"Renderer allocator must support freeing memory.");
			return NULL;
		}

		dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options, true, true);
		if (!dsGfxFormat_isValid(colorFormat))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Invalid color format.");
			return NULL;
		}

		dsGfxFormat depthFormat = dsRenderer_optionsDepthFormat(options);

		id<MTLDevice> device = MTLCreateSystemDefaultDevice();
		if (!device)
		{
			errno = EAGAIN;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Couldn't create Metal device.");
			return NULL;
		}

		const char* deviceName = device.name.UTF8String;
		size_t deviceNameLen = strlen(deviceName);

		size_t bufferSize = dsMTLRenderer_fullAllocSize(deviceNameLen);
		void* buffer = dsAllocator_alloc(allocator, bufferSize);
		if (!buffer)
			return NULL;

		dsBufferAllocator bufferAlloc;
		DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
		dsMTLRenderer* renderer = DS_ALLOCATE_OBJECT(&bufferAlloc, dsMTLRenderer);
		DS_ASSERT(renderer);
		memset(renderer, 0, sizeof(*renderer));

		dsRenderer* baseRenderer = (dsRenderer*)renderer;
		DS_VERIFY(dsRenderer_initialize(baseRenderer));

		baseRenderer->allocator = dsAllocator_keepPointer(allocator);

		DS_VERIFY(dsSpinlock_initialize(&renderer->processBuffersLock));
		DS_VERIFY(dsSpinlock_initialize(&renderer->processTexturesLock));
		DS_VERIFY(dsSpinlock_initialize(&renderer->clearPipelinesLock));

		renderer->device = CFBridgingRetain(device);

		id<MTLCommandQueue> commandQueue = [device newCommandQueue];
		if (!commandQueue)
		{
			dsMTLRenderer_destroy(baseRenderer);
			errno = ENOMEM;
			return NULL;
		}

		renderer->commandQueue = CFBridgingRetain(commandQueue);

		if (!createSharedResources(renderer, device, commandQueue))
		{
			dsMTLRenderer_destroy(baseRenderer);
			errno = ENOMEM;
			return NULL;
		}

		// Start at submit count 1 so it's ahead of the finished index.
		renderer->submitCount = 1;
		renderer->submitCondition = dsConditionVariable_create((dsAllocator*)&bufferAlloc,
			"Render Submit Condition");
		renderer->submitMutex = dsMutex_create((dsAllocator*)&bufferAlloc,
			"Render Submit Mutex");

		dsMTLHardwareCommandBuffer_initialize(&renderer->mainCommandBuffer, baseRenderer, allocator,
			dsCommandBufferUsage_Standard);
		baseRenderer->mainCommandBuffer = (dsCommandBuffer*)&renderer->mainCommandBuffer;

		baseRenderer->platform = dsGfxPlatform_Default;
		baseRenderer->rendererID = DS_MTL_RENDERER_ID;
		baseRenderer->platformID = 0;
		baseRenderer->name = "Metal";
#if DS_IOS
		baseRenderer->shaderLanguage = "metal-ios";
#else
		baseRenderer->shaderLanguage = "metal-macos";
#endif

		char* deviceNameCopy = (char*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			deviceNameLen + 1);
		DS_ASSERT(deviceNameCopy);
		memcpy(deviceNameCopy, deviceName, deviceNameLen + 1);
		baseRenderer->deviceName = deviceNameCopy;
		baseRenderer->shaderVersion = getShaderVersion();
		baseRenderer->vendorID = 0;
		baseRenderer->deviceID = 0;
		baseRenderer->driverVersion = 0;

		baseRenderer->maxColorAttachments = getMaxColorAttachments(device);
		baseRenderer->maxSurfaceSamples = getMaxSurfaceSamples(device);
		baseRenderer->maxAnisotropy = 16.0f;
		baseRenderer->surfaceSamples = options->surfaceSamples;
		baseRenderer->defaultSamples = options->defaultSamples;
		baseRenderer->defaultAnisotropy = 1.0f;

		baseRenderer->maxComputeWorkGroupSize[0] = UINT_MAX;
		baseRenderer->maxComputeWorkGroupSize[1] = UINT_MAX;
		baseRenderer->maxComputeWorkGroupSize[2] = UINT_MAX;

		baseRenderer->projectionOptions = dsProjectionMatrixOptions_HalfZRange;
		if (options->reverseZ)
			baseRenderer->projectionOptions |= dsProjectionMatrixOptions_InvertZ;
		baseRenderer->singleBuffer = false;
		baseRenderer->stereoscopic = false;
		baseRenderer->vsync = dsVSync_Disabled;
		baseRenderer->hasGeometryShaders = false;
		baseRenderer->hasTessellationShaders = hasTessellationShaders(device);

		// Enough optimizations that might as well consider multidraw native.
		baseRenderer->hasNativeMultidraw = true;
		baseRenderer->hasInstancedDrawing = true;

#if DS_MAC
		baseRenderer->hasStartInstance = true;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000
		baseRenderer->hasStartInstance = [device supportsFamily: MTLGPUFamilyApple3];
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		baseRenderer->hasStartInstance =
			[device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily3_v1];
#else
		baseRenderer->hasStartInstance = true;
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
		baseRenderer->hasDualSrcBlend = true;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000
		baseRenderer->hasDualSrcBlend = baseRenderer->hasDepthClamp =
			[device supportsFamily: MTLGPUFamilyApple4];
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		baseRenderer->hasDualSrcBlend = baseRenderer->hasDepthClamp =
			[device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily4_v1];
#else
		baseRenderer->hasDualSrcBlend = false;
		baseRenderer->hasDepthClamp = false;
#endif

#if DS_MAC
		baseRenderer->hasDepthClamp = true;
#endif

		baseRenderer->hasIndependentBlend = true;
		baseRenderer->hasLogicOps = false;
		baseRenderer->hasSampleShading = false;
		baseRenderer->hasDepthBounds = true;
		baseRenderer->hasDepthBiasClamp = true;
#if DS_MAC
		baseRenderer->hasDepthStencilMultisampleResolve = true;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000
		baseRenderer->hasDepthStencilMultisampleResolve =
			[device supportsFamily: MTLGPUFamilyApple3];
#else
		baseRenderer->hasDepthStencilMultisampleResolve =
			[device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily3_v1];
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 130000
		baseRenderer->hasFragmentInputs = [device supportsFamily: MTLGPUFamilyApple4];
#elif __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		baseRenderer->hasFragmentInputs =
			[device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily4_v1];
#else
		baseRenderer->hasFragmentInputs = false;
#endif
		baseRenderer->projectedTexCoordTInverted = true;

		baseRenderer->resourceManager = dsMTLResourceManager_create(allocator, baseRenderer);
		if (!baseRenderer->resourceManager)
		{
			dsMTLRenderer_destroy(baseRenderer);
			return NULL;
		}

		baseRenderer->surfaceColorFormat = colorFormat;

		// 16 and 24-bit depth not always supported.
		// First try 16 bit to fall back to 24 bit. Then try 24 bit to fall back to 32 bit.
		if (depthFormat == dsGfxFormat_D16S8 &&
			!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
		{
			depthFormat = dsGfxFormat_D24S8;
		}
		else if (depthFormat == dsGfxFormat_D16 &&
			!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
		{
			depthFormat = dsGfxFormat_X8D24;
		}

		if (depthFormat == dsGfxFormat_D24S8 &&
			!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
		{
			depthFormat = dsGfxFormat_D32S8_Float;
		}
		else if (depthFormat == dsGfxFormat_X8D24 &&
			!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
		{
			depthFormat = dsGfxFormat_D32_Float;
		}

		if (depthFormat != dsGfxFormat_Unknown &&
			!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Can't draw to surface depth format.");
			dsMTLRenderer_destroy(baseRenderer);
			return NULL;
		}

		baseRenderer->surfaceDepthStencilFormat = depthFormat;

		baseRenderer->destroyFunc = &dsMTLRenderer_destroy;

		// Render surfaces
		baseRenderer->createRenderSurfaceFunc = &dsMTLRenderSurface_create;
		baseRenderer->destroyRenderSurfaceFunc = &dsMTLRenderSurface_destroy;
		baseRenderer->updateRenderSurfaceFunc = &dsMTLRenderSurface_update;
		baseRenderer->beginRenderSurfaceFunc = &dsMTLRenderSurface_beginDraw;
		baseRenderer->endRenderSurfaceFunc = &dsMTLRenderSurface_endDraw;
		baseRenderer->swapRenderSurfaceBuffersFunc = &dsMTLRenderSurface_swapBuffers;

		// Command buffer pools
		baseRenderer->createCommandBufferPoolFunc = &dsMTLCommandBufferPool_create;
		baseRenderer->createCommandBuffersFunc = &dsMTLCommandBufferPool_createCommandBuffers;
		baseRenderer->destroyCommandBufferPoolFunc = &dsMTLCommandBufferPool_destroy;
		baseRenderer->resetCommandBufferPoolFunc = &dsMTLCommandBufferPool_reset;

		// Command buffers.
		baseRenderer->beginCommandBufferFunc = &dsMTLCommandBuffer_begin;
		baseRenderer->beginSecondaryCommandBufferFunc = &dsMTLCommandBuffer_beginSecondary;
		baseRenderer->endCommandBufferFunc = &dsMTLCommandBuffer_end;
		baseRenderer->submitCommandBufferFunc = &dsMTLCommandBuffer_submit;

		// Render passes.
		baseRenderer->createRenderPassFunc = &dsMTLRenderPass_create;
		baseRenderer->destroyRenderPassFunc = &dsMTLRenderPass_destroy;
		baseRenderer->beginRenderPassFunc = &dsMTLRenderPass_begin;
		baseRenderer->nextRenderSubpassFunc = &dsMTLRenderPass_nextSubpass;
		baseRenderer->endRenderPassFunc = &dsMTLRenderPass_end;

		// Rendering functions.
		baseRenderer->beginFrameFunc = &dsMTLRenderer_beginFrame;
		baseRenderer->endFrameFunc = &dsMTLRenderer_endFrame;
		baseRenderer->setSurfaceSamplesFunc = &dsMTLRenderer_setSurfaceSamples;
		baseRenderer->setDefaultSamplesFunc = &dsMTLRenderer_setDefaultSamples;
		baseRenderer->setVSyncFunc = &dsMTLRenderer_setVSync;
		baseRenderer->setDefaultAnisotropyFunc = &dsMTLRenderer_setDefaultAnisotropy;
		baseRenderer->setViewportFunc = &dsMTLCommandBuffer_setViewport;
		baseRenderer->clearAttachmentsFunc = &dsMTLCommandBuffer_clearAttachments;
		baseRenderer->drawFunc = &dsMTLRenderer_draw;
		baseRenderer->drawIndexedFunc = &dsMTLRenderer_drawIndexed;
		baseRenderer->drawIndirectFunc = &dsMTLRenderer_drawIndirect;
		baseRenderer->drawIndexedIndirectFunc = &dsMTLRenderer_drawIndexedIndirect;
		baseRenderer->dispatchComputeFunc = &dsMTLRenderer_dispatchCompute;
		baseRenderer->dispatchComputeIndirectFunc = &dsMTLRenderer_dispatchComputeIndirect;
		baseRenderer->pushDebugGroupFunc = &dsMTLRenderer_pushDebugGroup;
		baseRenderer->popDebugGroupFunc = &dsMTLRenderer_popDebugGroup;
		baseRenderer->flushFunc = &dsMTLRenderer_flush;
		baseRenderer->waitUntilIdleFunc = &dsMTLRenderer_waitUntilIdle;

		DS_VERIFY(dsRenderer_initializeResources(baseRenderer));

		return baseRenderer;
	}
}

uint64_t dsMTLRenderer_flushImpl(dsRenderer* renderer, id<MTLCommandBuffer> extraCommands)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;

	dsMTLHardwareCommandBuffer_endEncoding(renderer->mainCommandBuffer);
	dsMTLHardwareCommandBuffer* commandBuffer = &mtlRenderer->mainCommandBuffer;

	id<MTLCommandBuffer> lastCommandBuffer = processResources(mtlRenderer);
	for (uint32_t i = 0; i < commandBuffer->submitBufferCount; ++i)
	{
		if (lastCommandBuffer)
			[lastCommandBuffer commit];
		lastCommandBuffer = (__bridge id<MTLCommandBuffer>)commandBuffer->submitBuffers[i];
	}

	if (extraCommands)
	{
		if (lastCommandBuffer)
			[lastCommandBuffer commit];
		lastCommandBuffer = extraCommands;
	}

	DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
	uint64_t submit = mtlRenderer->submitCount;
	if (!lastCommandBuffer)
	{
		DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
		return submit - 1;
	}

	++mtlRenderer->submitCount;
	DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));

	id<MTLCommandBuffer> synchronizeCommands =
		dsMTLHardwareCommandBuffer_submitted(renderer->mainCommandBuffer, submit);
	if (synchronizeCommands)
	{
		[lastCommandBuffer commit];
		lastCommandBuffer = synchronizeCommands;
	}

	// Increment finished submit count at the end of the last command buffer.
	[lastCommandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> commandBuffer)
		{
			DS_UNUSED(commandBuffer);
			DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
			if (submit > mtlRenderer->finishedSubmitCount)
			{
				DS_ATOMIC_STORE64(&mtlRenderer->finishedSubmitCount, &submit);
				DS_VERIFY(dsConditionVariable_notifyAll(mtlRenderer->submitCondition));
			}
			DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
		}];
	[lastCommandBuffer commit];
	return submit;
}

uint64_t dsMTLRenderer_getFinishedSubmitCount(const dsRenderer* renderer)
{
	const dsMTLRenderer* mtlRenderer = (const dsMTLRenderer*)renderer;
	uint64_t finishedSubmitCount;
	DS_ATOMIC_LOAD64(&mtlRenderer->finishedSubmitCount, &finishedSubmitCount);
	return finishedSubmitCount;
}

dsGfxFenceResult dsMTLRenderer_waitForSubmit(const dsRenderer* renderer, uint64_t submitCount,
	unsigned int milliseconds)
{
	const dsMTLRenderer* mtlRenderer = (const dsMTLRenderer*)renderer;
	if (dsMTLRenderer_getFinishedSubmitCount(renderer) >= submitCount)
		return dsGfxFenceResult_Success;

	DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
	if (mtlRenderer->submitCount <= submitCount)
	{
		DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
		return dsGfxFenceResult_WaitingToQueue;
	}

	while (submitCount > mtlRenderer->finishedSubmitCount)
	{
		dsConditionVariableResult result = dsConditionVariable_timedWait(
			mtlRenderer->submitCondition, mtlRenderer->submitMutex, milliseconds);
		switch (result)
		{
			case dsConditionVariableResult_Error:
				DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
				return dsGfxFenceResult_Error;
			case dsConditionVariableResult_Timeout:
				DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
				if (submitCount > mtlRenderer->finishedSubmitCount)
					return dsGfxFenceResult_Timeout;
				else
					return dsGfxFenceResult_Success;
			default:
				break;
		}
	}
	DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));

	return dsGfxFenceResult_Success;
}

void dsMTLRenderer_processBuffer(dsRenderer* renderer, dsMTLGfxBufferData* buffer)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;

	DS_VERIFY(dsSpinlock_lock(&mtlRenderer->processBuffersLock));

	uint32_t index = mtlRenderer->processBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderer->allocator, mtlRenderer->processBuffers,
		mtlRenderer->processBufferCount, mtlRenderer->maxProcessBuffers, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->processBuffersLock));
		return;
	}

	mtlRenderer->processBuffers[index] = dsLifetime_addRef(buffer->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->processBuffersLock));
#else
	DS_UNUSED(renderer);
	DS_UNUSED(buffer);
#endif
}

void dsMTLRenderer_processTexture(dsRenderer* renderer, dsTexture* texture)
{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	dsMTLTexture* mtlTexture = (dsMTLTexture*)texture;

	DS_VERIFY(dsSpinlock_lock(&mtlRenderer->processTexturesLock));

	uint32_t index = mtlRenderer->processTextureCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(renderer->allocator, mtlRenderer->processTextures,
		mtlRenderer->processTextureCount, mtlRenderer->maxProcessTextures, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->processTexturesLock));
		return;
	}

	mtlRenderer->processTextures[index] = dsLifetime_addRef(mtlTexture->lifetime);
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->processTexturesLock));
#else
	DS_UNUSED(renderer);
	DS_UNUSED(texture);
#endif
}

id<MTLRenderPipelineState> dsMTLRenderer_getClearPipeline(dsRenderer* renderer,
	MTLPixelFormat colorFormats[DS_MAX_ATTACHMENTS], uint32_t colorMask, MTLPixelFormat depthFormat,
	MTLPixelFormat stencilFormat, bool layered, uint32_t samples)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderer->clearPipelinesLock));

	for (uint32_t i = 0; i < mtlRenderer->clearPipelineCount; ++i)
	{
		dsMTLClearPipeline* clearPipeline = mtlRenderer->clearPipelines + i;
		if (memcmp(clearPipeline->colorFormats, colorFormats,
				sizeof(clearPipeline->colorFormats)) == 0 &&
			clearPipeline->colorMask == colorMask && clearPipeline->depthFormat == depthFormat &&
			clearPipeline->stencilFormat == stencilFormat && clearPipeline->layered == layered &&
			clearPipeline->samples == samples)
		{
			return (__bridge id<MTLRenderPipelineState>)clearPipeline->pipeline;
		}
	}

	@autoreleasepool
	{
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
		MTLCompileOptions* options = [MTLCompileOptions new];

		NSMutableString* str = [NSMutableString stringWithCapacity: 2*1024];
		[str appendString: @"#include <metal_stdlib>\n"];
		[str appendString: @"using namespace metal;\n"];
		[str appendString: @"\n"];
		[str appendString: @"typedef struct {\n"];
		[str appendString: @"    float2 position [[attribute(0)]];\n"];
		[str appendString: @"} VertexInput;\n"];
		[str appendString: @"\n"];
		[str appendString: @"typedef struct {\n"];
		[str appendString: @"    float depth;\n"];
		[str appendString: @"    uint layer;\n"];
		[str appendString: @"} VertexData;\n"];
		[str appendString: @"\n"];
		[str appendString: @"typedef struct {\n"];
		[str appendFormat: @"    float4 colors[%u];\n", DS_MAX_ATTACHMENTS];
		[str appendString: @"} FragmentData;\n"];
		[str appendString: @"\n"];
		[str appendString: @"typedef struct {\n"];
		[str appendString: @"    float4 position [[position]];\n"];
		if (layered)
			[str appendString: @"    uint layer [[render_target_array_index]];\n"];
		[str appendString: @"} VertexOutput;\n"];
		[str appendString: @"\n"];
		[str appendString: @"typedef struct {\n"];
		for (uint32_t i = 0; i < DS_MAX_ATTACHMENTS; ++i)
		{
			if (colorFormats[i] != MTLPixelFormatInvalid)
				[str appendFormat: @"    float4 color%u [[color(%u)]];\n", i, i];
		}
		[str appendString: @"} FragmentOutput;\n"];
		[str appendString: @"\n"];
		[str appendString: @"vertex VertexOutput vertexShader(VertexInput attributes [[stage_in]], "
			@"constant VertexData& parameters [[buffer(0)]]) {\n"];
		[str appendString: @"    VertexOutput vertexOut;\n"];
		[str appendString:
			@"    vertexOut.position = float4(attributes.position, parameters.depth, 1.0f);\n"];
		if (layered)
			[str appendString: @"    vertexOut.layer = parameters.layer;\n"];
		[str appendString: @"    return vertexOut;\n"];
		[str appendString: @"}\n"];
		[str appendString: @"\n"];
		[str appendString: @"fragment FragmentOutput fragmentShader("
			@"VertexOutput vertexOut [[stage_in]], "
			@"constant FragmentData& parameters [[buffer(0)]]) {\n"];
		[str appendString: @"    FragmentOutput fragmentOut;\n"];
		for (uint32_t i = 0; i < DS_MAX_ATTACHMENTS; ++i)
		{
			if (colorFormats[i] != MTLPixelFormatInvalid)
				[str appendFormat: @"    fragmentOut.color%u = parameters.colors[%u];\n", i, i];
		}
		[str appendString: @"    return fragmentOut;\n"];
		[str appendString: @"}\n"];

		NSError* error = NULL;
		id<MTLLibrary> library = [device newLibraryWithSource: str options: options error: &error];
		if (error)
		{
			DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
				"Error creating clear shader: %s", error.localizedDescription.UTF8String);
			errno = EPERM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}
		else if (!library)
		{
			errno = EPERM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		id<MTLFunction> vertexFunction = [library newFunctionWithName: @"vertexShader"];
		if (!vertexFunction)
		{
			errno = ENOMEM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		id<MTLFunction> fragmentFunction = [library newFunctionWithName: @"fragmentShader"];
		if (!fragmentFunction)
		{
			errno = ENOMEM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];
		if (!descriptor)
		{
			errno = ENOMEM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		descriptor.vertexFunction = vertexFunction;
		descriptor.fragmentFunction = fragmentFunction;
		descriptor.vertexDescriptor.layouts[1].stride = sizeof(dsVector2f);
		descriptor.vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
		descriptor.vertexDescriptor.attributes[0].bufferIndex = 1;
		for (uint32_t i = 0; i < DS_MAX_ATTACHMENTS; ++i)
		{
			if (colorFormats[i] == MTLPixelFormatInvalid)
				continue;

			MTLRenderPipelineColorAttachmentDescriptor* colorDescriptor =
				descriptor.colorAttachments[i];
			colorDescriptor.pixelFormat = colorFormats[i];
			if (!(colorMask & (1 << i)))
				colorDescriptor.writeMask = MTLColorWriteMaskNone;
		}
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
		descriptor.rasterSampleCount = samples;
#else
		descriptor.sampleCount = samples;
#endif
		descriptor.depthAttachmentPixelFormat = depthFormat;
		descriptor.stencilAttachmentPixelFormat = stencilFormat;

		error = NULL;
		id<MTLRenderPipelineState> pipeline =
			[device newRenderPipelineStateWithDescriptor: descriptor error: &error];
		if (error)
		{
			DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
				"Error creating clear pipeline: %s", error.localizedDescription.UTF8String);
			errno = EPERM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}
		else if (!pipeline)
		{
			errno = EPERM;
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		uint32_t index = mtlRenderer->clearPipelineCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(renderer->allocator, mtlRenderer->clearPipelines,
				mtlRenderer->clearPipelineCount, mtlRenderer->maxClearPipelines, 1))
		{
			DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
			return nil;
		}

		dsMTLClearPipeline* clearPipeline = mtlRenderer->clearPipelines + index;
		memcpy(clearPipeline->colorFormats, colorFormats, sizeof(clearPipeline->colorFormats));
		clearPipeline->depthFormat = depthFormat;
		clearPipeline->stencilFormat = stencilFormat;
		clearPipeline->layered = layered;
		clearPipeline->samples = (uint8_t)samples;
		clearPipeline->pipeline = CFBridgingRetain(pipeline);

		DS_VERIFY(dsSpinlock_unlock(&mtlRenderer->clearPipelinesLock));
		return pipeline;
	}
}
