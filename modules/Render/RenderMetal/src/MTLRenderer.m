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

#include <DeepSea/RenderMetal/MTLRenderer.h>
#include "MTLRendererInternal.h"

#include "Resources/MTLResourceManager.h"
#include "MTLCommandBuffer.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

#include <string.h>

#import <Metal/MTLBlitCommandEncoder.h>
#import <Metal/MTLCommandQueue.h>

#if defined(IPHONE_OS_VERSION_MIN_REQUIRED) && IPHONE_OS_VERSION_MIN_REQUIRED < 80000
#error iPhone target version must be >= 8.0 to support metal.
#endif

#if defined(MAC_OS_X_VERSION_MIN_REQUIRED) && MAC_OS_X_VERSION_MIN_REQUIRED < 101100
#error macOS target version must be >= 10.11 to support metal.
#endif

static MTLFeatureSet mtlFeatures[] =
{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
	MTLFeatureSet_iOS_GPUFamily1_v1,
	MTLFeatureSet_iOS_GPUFamily2_v1,
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	MTLFeatureSet_iOS_GPUFamily1_v2,
	MTLFeatureSet_iOS_GPUFamily2_v2,
	MTLFeatureSet_iOS_GPUFamily3_v1,
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
	MTLFeatureSet_iOS_GPUFamily1_v3,
	MTLFeatureSet_iOS_GPUFamily2_v3,
	MTLFeatureSet_iOS_GPUFamily3_v2,
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
	MTLFeatureSet_iOS_GPUFamily1_v4,
	MTLFeatureSet_iOS_GPUFamily2_v4,
	MTLFeatureSet_iOS_GPUFamily3_v3,
	MTLFeatureSet_iOS_GPUFamily4_v1,
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
	MTLFeatureSet_iOS_GPUFamily1_v5,
	MTLFeatureSet_iOS_GPUFamily2_v5,
	MTLFeatureSet_iOS_GPUFamily3_v4,
	MTLFeatureSet_iOS_GPUFamily4_v2,
	MTLFeatureSet_iOS_GPUFamily5_v1,
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
	MTLFeatureSet_macOS_GPUFamily1_v1,
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
	MTLFeatureSet_macOS_GPUFamily1_v2,
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	MTLFeatureSet_macOS_GPUFamily1_v3,
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
	MTLFeatureSet_macOS_GPUFamily1_v4,
	MTLFeatureSet_macOS_GPUFamily2_v1,
#endif
};

static size_t dsMTLRenderer_fullAllocSize(size_t deviceNameLen)
{
	return DS_ALIGNED_SIZE(sizeof(dsMTLRenderer)) + DS_ALIGNED_SIZE(deviceNameLen + 1) +
		dsConditionVariable_fullAllocSize() + dsMutex_fullAllocSize();
}

static uint32_t getShaderVersion(MTLFeatureSet feature)
{
	switch (feature)
	{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
		case MTLFeatureSet_iOS_GPUFamily1_v1:
		case MTLFeatureSet_iOS_GPUFamily2_v1:
			return DS_ENCODE_VERSION(1, 0, 0);
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		case MTLFeatureSet_iOS_GPUFamily1_v2:
		case MTLFeatureSet_iOS_GPUFamily2_v2:
		case MTLFeatureSet_iOS_GPUFamily3_v1:
			return DS_ENCODE_VERSION(1, 1, 0);
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		case MTLFeatureSet_iOS_GPUFamily1_v3:
		case MTLFeatureSet_iOS_GPUFamily2_v3:
		case MTLFeatureSet_iOS_GPUFamily3_v2:
			return DS_ENCODE_VERSION(1, 2, 0);
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		case MTLFeatureSet_iOS_GPUFamily1_v4:
		case MTLFeatureSet_iOS_GPUFamily2_v4:
		case MTLFeatureSet_iOS_GPUFamily3_v3:
		case MTLFeatureSet_iOS_GPUFamily4_v1:
			return DS_ENCODE_VERSION(2, 0, 0);
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
		case MTLFeatureSet_iOS_GPUFamily1_v5:
		case MTLFeatureSet_iOS_GPUFamily2_v5:
		case MTLFeatureSet_iOS_GPUFamily3_v4:
		case MTLFeatureSet_iOS_GPUFamily4_v2:
		case MTLFeatureSet_iOS_GPUFamily5_v1:
			return DS_ENCODE_VERSION(2, 1, 0);
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
		case MTLFeatureSet_macOS_GPUFamily1_v1:
			return DS_ENCODE_VERSION(1, 1, 0);
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
		case MTLFeatureSet_macOS_GPUFamily1_v2:
			return DS_ENCODE_VERSION(1, 2, 0);
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
		case MTLFeatureSet_macOS_GPUFamily1_v3:
			return DS_ENCODE_VERSION(2, 0, 0);
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
		case MTLFeatureSet_macOS_GPUFamily1_v4:
		case MTLFeatureSet_macOS_GPUFamily2_v1:
			return DS_ENCODE_VERSION(2, 1, 0);
#endif

		default:
			DS_ASSERT(false);
			return 0;
	}
}

static uint32_t getMaxColorAttachments(MTLFeatureSet feature)
{
	switch (feature)
	{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
		case MTLFeatureSet_iOS_GPUFamily1_v1:
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		case MTLFeatureSet_iOS_GPUFamily1_v2:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		case MTLFeatureSet_iOS_GPUFamily1_v3:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		case MTLFeatureSet_iOS_GPUFamily1_v4:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
		case MTLFeatureSet_iOS_GPUFamily1_v5:
#endif
			return 4;
#endif
		default:
			return 8;
	}
}

static uint32_t getMaxSurfaceSamples(id<MTLDevice> device)
{
	if ([device supportsTextureSampleCount: 8])
		return 8;
	return 4;
}

static uint32_t hasTessellationShaders(MTLFeatureSet feature)
{
	switch (feature)
	{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
		case MTLFeatureSet_iOS_GPUFamily1_v1:
		case MTLFeatureSet_iOS_GPUFamily2_v1:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		case MTLFeatureSet_iOS_GPUFamily1_v2:
		case MTLFeatureSet_iOS_GPUFamily2_v2:
		case MTLFeatureSet_iOS_GPUFamily3_v1:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		case MTLFeatureSet_iOS_GPUFamily1_v3:
		case MTLFeatureSet_iOS_GPUFamily2_v3:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		case MTLFeatureSet_iOS_GPUFamily1_v4:
		case MTLFeatureSet_iOS_GPUFamily2_v4:
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
		case MTLFeatureSet_iOS_GPUFamily1_v5:
		case MTLFeatureSet_iOS_GPUFamily2_v5:
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
		case MTLFeatureSet_macOS_GPUFamily1_v1:
#endif
			return false;
		default:
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
			//return true;
			return false;
	}
}

static id<MTLCommandBuffer> processTextures(dsMTLRenderer* renderer)
{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
	id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)renderer->commandQueue;
	id<MTLCommandBuffer> textureCommandBuffer = nil;
	DS_VERIFY(dsSpinlock_lock(&renderer->processTexturesLock));
	if (renderer->processTextureCount > 0)
	{
		textureCommandBuffer = [queue commandBuffer];
		id<MTLBlitCommandEncoder> encoder;
		if (textureCommandBuffer)
			encoder = [textureCommandBuffer blitCommandEncoder];
		else
			encoder = nil;
		if (!encoder)
		{
			for (uint32_t i = 0; i < renderer->processTextureCount; ++i)
				dsLifetime_freeRef(renderer->processTextures[i]);
			renderer->processTextureCount = 0;
			DS_VERIFY(dsSpinlock_unlock(&renderer->processTexturesLock));
		}

		for (uint32_t i = 0; i < renderer->processTextureCount; ++i)
		{
			dsLifetime* lifetime = renderer->processTextures[i];
			dsMTLTexture* texture = (dsMTLTexture*)dsLifetime_acquire(lifetime);
			if (texture)
			{
				id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)texture->mtlTexture;
				[encoder optimizeContentsForGPUAccess: mtlTexture];
				dsLifetime_release(lifetime);
			}
			dsLifetime_freeRef(lifetime);
		}
		renderer->processTextureCount = 0;

		[textureCommandBuffer commit];
	}
	DS_VERIFY(dsSpinlock_unlock(&renderer->processTexturesLock));
	return textureCommandBuffer;
#else
	DS_UNUSED(renderer);
#endif
}

bool dsMTLRenderer_destroy(dsRenderer* renderer)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;

	// Check the function manually so we only wait if initialization completed.
	if (renderer->waitUntilIdleFunc)
		dsRenderer_waitUntilIdle(renderer);

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

	for (uint32_t i = 0; i < mtlRenderer->processTextureCount; ++i)
		dsLifetime_freeRef(mtlRenderer->processTextures[i]);
	DS_VERIFY(dsAllocator_free(renderer->allocator, mtlRenderer->processTextures));
	dsSpinlock_shutdown(&mtlRenderer->processTexturesLock);

	if (mtlRenderer->device)
		CFRelease(mtlRenderer->device);
	if (mtlRenderer->commandQueue)
		CFRelease(mtlRenderer->commandQueue);
	DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));
	return true;
}

bool dsMTLRenderer_isSupported(void)
{
	return [MTLCopyAllDevices() count] > 0;
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

void dsMTLRenderer_flush(dsRenderer* renderer)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;

	dsMTLCommandBuffer_endEncoding(renderer->mainCommandBuffer);
	dsMTLCommandBuffer* commandBuffer = &mtlRenderer->mainCommandBuffer;

	id<MTLCommandBuffer> lastCommandBuffer = processTextures(mtlRenderer);
	for (uint32_t i = 0; i < commandBuffer->submitBufferCount; ++i)
	{
		lastCommandBuffer = (__bridge id<MTLCommandBuffer>)commandBuffer->submitBuffers[i];
		[lastCommandBuffer commit];
	}

	DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
	uint64_t submit = mtlRenderer->submitCount;
	if (lastCommandBuffer > 0)
		++mtlRenderer->submitCount;
	DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));

	// Increment finished submit count at the end of the last command buffer.
	[lastCommandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> commandBuffer)
		{
			DS_UNUSED(commandBuffer);
			DS_VERIFY(dsMutex_lock(mtlRenderer->submitMutex));
			if (submit > mtlRenderer->finishedSubmitCount)
			{
				mtlRenderer->finishedSubmitCount = submit;
				DS_VERIFY(dsConditionVariable_notifyAll(mtlRenderer->submitCondition));
			}
			DS_VERIFY(dsMutex_unlock(mtlRenderer->submitMutex));
		}];
	dsMTLCommandBuffer_submitted(renderer->mainCommandBuffer, submit);
}

dsRenderer* dsMTLRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Renderer allocator must support freeing memory.");
		return NULL;
	}

	dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options, false, false);
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

	const char* deviceName = [device.name UTF8String];
	size_t deviceNameLen = strlen(deviceName);

	size_t bufferSize = dsMTLRenderer_fullAllocSize(deviceNameLen);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsMTLRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsMTLRenderer);
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	DS_VERIFY(dsSpinlock_initialize(&renderer->processTexturesLock));

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	renderer->device = CFBridgingRetain(device);
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(mtlFeatures); ++i)
	{
		if ([device supportsFeatureSet: mtlFeatures[i]])
			renderer->featureSet = mtlFeatures[i];
	}

	id<MTLCommandQueue> commandQueue = [device newCommandQueue];
	if (!commandQueue)
	{
		dsMTLRenderer_destroy(baseRenderer);
		errno = ENOMEM;
		return NULL;
	}

	renderer->commandQueue = CFBridgingRetain(commandQueue);

	// Start at submit count 1 so it's ahead of the finished index.
	renderer->submitCount = 1;
	renderer->submitCondition = dsConditionVariable_create((dsAllocator*)&bufferAlloc,
		"Render Submit Condition");
	renderer->submitMutex = dsMutex_create((dsAllocator*)&bufferAlloc,
		"Render Submit Mutex");

	baseRenderer->allocator = dsAllocator_keepPointer(allocator);

	dsMTLCommandBuffer_initialize(&renderer->mainCommandBuffer, baseRenderer, allocator,
		dsCommandBufferUsage_Standard);
	baseRenderer->mainCommandBuffer = (dsCommandBuffer*)&renderer->mainCommandBuffer;

	baseRenderer->platform = dsGfxPlatform_Default;
	baseRenderer->rendererID = DS_MTL_RENDERER_ID;
	baseRenderer->platformID = 0;
	baseRenderer->name = "Metal";
#if DS_IOS
	baseRenderer->shaderLanguage = "metal-ios";
#else
	baseRenderer->shaderLanguage = "metal-osx";
#endif

	char* deviceNameCopy = (char*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, deviceNameLen + 1);
	DS_ASSERT(deviceNameCopy);
	memcpy(deviceNameCopy, deviceName, deviceNameLen + 1);
	baseRenderer->deviceName = deviceNameCopy;
	baseRenderer->shaderVersion = getShaderVersion(renderer->featureSet);
	baseRenderer->vendorID = 0;
	baseRenderer->deviceID = 0;
	baseRenderer->driverVersion = 0;

	baseRenderer->maxColorAttachments = getMaxColorAttachments(renderer->featureSet);
	baseRenderer->maxSurfaceSamples = getMaxSurfaceSamples(device);
	baseRenderer->maxAnisotropy = 16.0f;
	baseRenderer->surfaceColorFormat = colorFormat;
	baseRenderer->surfaceDepthStencilFormat = depthFormat;
	baseRenderer->surfaceSamples = options->samples;
	baseRenderer->doubleBuffer = true;
	baseRenderer->stereoscopic = false;
	baseRenderer->vsync = false;
	baseRenderer->clipHalfDepth = true;
	baseRenderer->clipInvertY = false;
	baseRenderer->hasGeometryShaders = false;
	baseRenderer->hasTessellationShaders = hasTessellationShaders(renderer->featureSet);

	MTLSize maxComputeSize = device.maxThreadsPerThreadgroup;
	baseRenderer->maxComputeWorkGroupSize[0] = (uint32_t)maxComputeSize.width;
	baseRenderer->maxComputeWorkGroupSize[1] = (uint32_t)maxComputeSize.height;
	baseRenderer->maxComputeWorkGroupSize[2] = (uint32_t)maxComputeSize.depth;

	baseRenderer->hasNativeMultidraw = false;
	baseRenderer->supportsInstancedDrawing = true;
	baseRenderer->supportsStartInstance = true;
	baseRenderer->defaultAnisotropy = 1.0f;

	baseRenderer->resourceManager = dsMTLResourceManager_create(allocator, baseRenderer);
	if (!baseRenderer->resourceManager)
	{
		dsMTLRenderer_destroy(baseRenderer);
		return NULL;
	}

	baseRenderer->destroyFunc = &dsMTLRenderer_destroy;

	DS_VERIFY(dsRenderer_initializeResources(baseRenderer));

	return baseRenderer;
}

dsGfxFenceResult dsMTLRenderer_waitForSubmit(const dsRenderer* renderer, uint64_t submitCount,
	unsigned int milliseconds)
{
	const dsMTLRenderer* mtlRenderer = (const dsMTLRenderer*)renderer;
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

void dsMTLRenderer_processTexture(dsRenderer* renderer, dsTexture* texture)
{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
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
