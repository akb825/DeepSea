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

#include "Resources/MTLRenderbuffer.h"

#include "Resources/MTLResourceManager.h"
#include "MTLCommandBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

#import <Metal/MTLTexture.h>

dsRenderbuffer* dsMTLRenderbuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsRenderbufferUsage usage, dsGfxFormat format, uint32_t width, uint32_t height,
	uint32_t samples)
{
	dsMTLRenderer* renderer = (dsMTLRenderer*)resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
	MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, format);
	if (pixelFormat == MTLPixelFormatInvalid)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Unknown format.");
		return nil;
	}

	dsMTLRenderbuffer* renderbuffer = DS_ALLOCATE_OBJECT(allocator, dsMTLRenderbuffer);
	if (!renderbuffer)
		return NULL;

	dsRenderbuffer* baseRenderbuffer = (dsRenderbuffer*)renderbuffer;
	baseRenderbuffer->resourceManager = resourceManager;
	baseRenderbuffer->allocator = dsAllocator_keepPointer(allocator);
	baseRenderbuffer->usage = usage;
	baseRenderbuffer->format = format;
	baseRenderbuffer->width = width;
	baseRenderbuffer->height = height;
	baseRenderbuffer->samples = samples;

	MTLTextureDescriptor* descriptor =
		[MTLTextureDescriptor texture2DDescriptorWithPixelFormat: pixelFormat
			width: width height: height mipmapped: false];
	if (!descriptor)
	{
		errno = ENOMEM;
		dsMTLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
		return NULL;
	}
	descriptor.sampleCount = samples;

	MTLResourceOptions resourceOptions = MTLResourceOptionCPUCacheModeWriteCombined;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	resourceOptions |= MTLResourceStorageModePrivate;
#endif
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
	if (!(usage & (dsRenderbufferUsage_Continue | dsRenderbufferUsage_Clear)))
		resourceOptions |= MTLResourceStorageModeMemoryless;
#endif
	descriptor.resourceOptions = resourceOptions;
	descriptor.usage = MTLTextureUsageRenderTarget;

	id<MTLTexture> surface = [device newTextureWithDescriptor: descriptor];
	if (!surface)
	{
		errno = ENOMEM;
		dsMTLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
		return NULL;
	}

	renderbuffer->surface = CFBridgingRetain(surface);
	return baseRenderbuffer;
}

bool dsMTLRenderbuffer_destroy(dsResourceManager* resourceManager, dsRenderbuffer* renderbuffer)
{
	DS_UNUSED(resourceManager);
	dsMTLRenderbuffer* mtlRenderbuffer = (dsMTLRenderbuffer*)renderbuffer;
	if (mtlRenderbuffer->surface)
		CFRelease(mtlRenderbuffer->surface);
	if (renderbuffer->allocator)
		DS_VERIFY(dsAllocator_free(renderbuffer->allocator, renderbuffer));
	return true;
}
