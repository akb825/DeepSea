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

#include "Resources/MTLGfxBufferData.h"

#include "Resources/MTLResourceManager.h"
#include "MTLRendererInternal.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

#import <Metal/MTLBuffer.h>
#import <Metal/MTLTexture.h>

dsMTLGfxBufferData* dsMTLGfxBufferData_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* scratchAllocator, dsGfxBufferUsage usage,
	dsGfxMemory memoryHints, const void* data, size_t size)
{
	DS_UNUSED(usage);
	DS_ASSERT(scratchAllocator->freeFunc);
	dsMTLGfxBufferData* buffer = DS_ALLOCATE_OBJECT(allocator, dsMTLGfxBufferData);
	if (!buffer)
		return NULL;

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)(mtlRenderer->device);

	memset(buffer, 0, sizeof(*buffer));
	buffer->resourceManager = resourceManager;
	buffer->allocator = dsAllocator_keepPointer(allocator);
	buffer->scratchAllocator = scratchAllocator;
	buffer->processed = false;
	buffer->usage = usage;
	DS_VERIFY(dsSpinlock_initialize(&buffer->bufferTextureLock));

	buffer->lifetime = dsLifetime_create(allocator, buffer);
	if (!buffer->lifetime)
	{
		dsMTLGfxBufferData_destroy(buffer);
		errno = ENOMEM;
		return NULL;
	}

	MTLResourceOptions resourceOptions;
	if (memoryHints & dsGfxMemory_Read)
		resourceOptions = MTLResourceCPUCacheModeDefaultCache;
	else
		resourceOptions = MTLResourceCPUCacheModeWriteCombined;

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	if (memoryHints & dsGfxMemory_GPUOnly)
		resourceOptions |= MTLResourceStorageModePrivate;
	else if ((memoryHints & dsGfxMemory_Read) || (memoryHints & dsGfxMemory_Stream) ||
		(memoryHints & dsGfxMemory_Coherent))
	{
		resourceOptions |= MTLResourceStorageModeShared;
	}
	else
	{
#if DS_MAC
		resourceOptions |= MTLResourceStorageModeManaged;
		buffer->managed = true;
#else
		resourceOptions |= MTLResourceStorageModeShared;
#endif
	}
#endif

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	if ((memoryHints & dsGfxMemory_GPUOnly) && !(usage & (dsGfxBufferUsage_CopyTo |
			dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Image)))
	{
		resourceOptions |= MTLResourceHazardTrackingModeUntracked;
	}
#endif

	id<MTLBuffer> mtlBuffer;
	id<MTLBuffer> copyBuffer;
	if (data)
	{
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		if (resourceOptions & MTLResourceStorageModePrivate)
		{
			copyBuffer = [device newBufferWithBytes: data length: size
				options: MTLResourceCPUCacheModeWriteCombined];
			mtlBuffer = [device newBufferWithLength: size options: resourceOptions];
		}
		else
#endif
			mtlBuffer = [device newBufferWithBytes: data length: size options: resourceOptions];
	}
	else
		mtlBuffer = [device newBufferWithLength: size options: resourceOptions];

	if (!mtlBuffer)
	{
		dsMTLGfxBufferData_destroy(buffer);
		return NULL;
	}

	buffer->mtlBuffer = CFBridgingRetain(mtlBuffer);
	buffer->copyBuffer = CFBridgingRetain(copyBuffer);
	return buffer;
}

void dsMTLGfxBufferData_markAsUsed(dsMTLGfxBufferData* buffer)
{
	uint32_t used = true;
	DS_ATOMIC_STORE32(&buffer->used, &used);
}

id<MTLTexture> dsMTLGfxBufferData_getBufferTexture(dsMTLGfxBufferData* buffer, dsGfxFormat format,
	size_t offset, size_t count)
{
#if DS_IOS || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	DS_VERIFY(dsSpinlock_lock(&buffer->bufferTextureLock));

	for (uint32_t i = 0; i < buffer->bufferTextureCount; ++i)
	{
		dsMTLBufferTexture* bufferTex = buffer->bufferTextures + i;
		if (bufferTex->format == format && bufferTex->offset == offset &&
			bufferTex->count == count)
		{
			id<MTLTexture> texture = (__bridge id<MTLTexture>)bufferTex->mtlTexture;
			DS_VERIFY(dsSpinlock_unlock(&buffer->bufferTextureLock));
			return texture;
		}
	}

	MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(buffer->resourceManager,
		format);
	if (pixelFormat == MTLPixelFormatInvalid)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Unknown format.");
		return nil;
	}

	uint32_t index = buffer->bufferTextureCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(buffer->scratchAllocator, buffer->bufferTextures,
		buffer->bufferTextureCount, buffer->maxBufferTextures, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&buffer->bufferTextureLock));
		return nil;
	}

	dsMTLBufferTexture* bufferTex = buffer->bufferTextures + index;
	bufferTex->format = format;
	bufferTex->offset = offset;
	bufferTex->count = count;

	uint32_t width = (uint32_t)dsMin(count, DS_IMAGE_BUFFER_WIDTH);
	id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)buffer->mtlBuffer;
	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat: pixelFormat width: width
		height: (count + DS_IMAGE_BUFFER_WIDTH - 1)/DS_IMAGE_BUFFER_WIDTH mipmapped: false];
	if (!descriptor)
	{
		--buffer->bufferTextureCount;
		DS_VERIFY(dsSpinlock_unlock(&buffer->bufferTextureLock));
		return nil;
	}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	descriptor.storageMode = mtlBuffer.storageMode;
	descriptor.cpuCacheMode = mtlBuffer.cpuCacheMode;
#endif

	MTLTextureUsage usage = MTLTextureUsageShaderRead;
	if (buffer->usage & dsGfxBufferUsage_Image)
		usage |= MTLTextureUsageShaderWrite;
	descriptor.usage = usage;

	id<MTLTexture> texture = [mtlBuffer newTextureWithDescriptor: descriptor offset: offset
		bytesPerRow: width*dsGfxFormat_size(format)];
	if (!texture)
	{
		--buffer->bufferTextureCount;
		DS_VERIFY(dsSpinlock_unlock(&buffer->bufferTextureLock));
		return nil;
	}

	bufferTex->mtlTexture = CFBridgingRetain(texture);
	DS_VERIFY(dsSpinlock_unlock(&buffer->bufferTextureLock));

	return texture;
#else
	DS_UNUSED(buffer);
	DS_UNUSED(format);
	DS_UNUSED(offset);
	DS_UNUSED(count);
	return nil;
#endif
}

void dsMTLGfxBufferData_process(dsMTLGfxBufferData* buffer, dsRenderer* renderer)
{
	uint32_t processed = true;
	uint32_t wasProcessed;
	DS_ATOMIC_EXCHANGE32(&buffer->processed, &processed, &wasProcessed);
	if (!wasProcessed && buffer->copyBuffer)
		dsMTLRenderer_processBuffer(renderer, buffer);
}

void dsMTLGfxBufferData_destroy(dsMTLGfxBufferData* buffer)
{
	if (!buffer)
		return;

	dsLifetime_destroy(buffer->lifetime);
	if (buffer->copyBuffer)
		CFRelease(buffer->copyBuffer);
	if (buffer->mtlBuffer)
		CFRelease(buffer->mtlBuffer);

	for (uint32_t i = 0; i < buffer->bufferTextureCount; ++i)
	{
		DS_ASSERT(buffer->bufferTextures[i].mtlTexture);
		CFRelease(buffer->bufferTextures[i].mtlTexture);
	}
	DS_VERIFY(dsAllocator_free(buffer->scratchAllocator, buffer->bufferTextures));

	dsSpinlock_shutdown(&buffer->bufferTextureLock);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
}
