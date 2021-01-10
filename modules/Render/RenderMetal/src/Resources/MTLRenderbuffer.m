/*
 * Copyright 2019-2021 Aaron Barany
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
	@autoreleasepool
	{
		dsMTLRenderer* renderer = (dsMTLRenderer*)resourceManager->renderer;
		id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
		MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, format);
		MTLPixelFormat stencilPixelFormat = MTLPixelFormatInvalid;
		if (pixelFormat == MTLPixelFormatInvalid)
		{
			// Need to have separate depth and stencil surfaces.
			switch (format)
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

		bool sharedStencil = format == dsGfxFormat_D16S8 || format == dsGfxFormat_D24S8 ||
			format == dsGfxFormat_D32S8_Float;

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

		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
		if (!descriptor)
		{
			errno = ENOMEM;
			dsMTLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
			return NULL;
		}

		if (samples > 1)
			descriptor.textureType = MTLTextureType2DMultisample;
		descriptor.width = width;
		descriptor.height = height;
		descriptor.sampleCount = samples;

		MTLResourceOptions resourceOptions = MTLResourceCPUCacheModeWriteCombined;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		resourceOptions |= MTLResourceStorageModePrivate;
		descriptor.usage = MTLTextureUsageRenderTarget;
#endif
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		if (!(usage & dsRenderbufferUsage_Continue))
			resourceOptions |= MTLResourceStorageModeMemoryless;
#endif
		descriptor.resourceOptions = resourceOptions;

		if (pixelFormat != MTLPixelFormatInvalid)
		{
			descriptor.pixelFormat = pixelFormat;
			id<MTLTexture> surface = [device newTextureWithDescriptor: descriptor];
			if (!surface)
			{
				errno = ENOMEM;
				dsMTLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
				return NULL;
			}
			renderbuffer->surface = CFBridgingRetain(surface);
		}
		else
			renderbuffer->surface = NULL;

		if (stencilPixelFormat != MTLPixelFormatInvalid)
		{
			descriptor.pixelFormat = stencilPixelFormat;
			id<MTLTexture> surface = [device newTextureWithDescriptor: descriptor];
			if (!surface)
			{
				errno = ENOMEM;
				dsMTLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
				return NULL;
			}
			renderbuffer->stencilSurface = CFBridgingRetain(surface);
		}
		else if (sharedStencil)
			renderbuffer->stencilSurface = CFRetain(renderbuffer->surface);
		else
			renderbuffer->stencilSurface = NULL;

		return baseRenderbuffer;
	}
}

bool dsMTLRenderbuffer_destroy(dsResourceManager* resourceManager, dsRenderbuffer* renderbuffer)
{
	DS_UNUSED(resourceManager);
	dsMTLRenderbuffer* mtlRenderbuffer = (dsMTLRenderbuffer*)renderbuffer;
	if (mtlRenderbuffer->surface)
		CFRelease(mtlRenderbuffer->surface);
	if (mtlRenderbuffer->stencilSurface)
		CFRelease(mtlRenderbuffer->stencilSurface);
	if (renderbuffer->allocator)
		DS_VERIFY(dsAllocator_free(renderbuffer->allocator, renderbuffer));
	return true;
}
