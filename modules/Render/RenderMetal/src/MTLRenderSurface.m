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

#include "MTLRenderSurface.h"

#include "Resources/MTLResourceManager.h"
#include "MTLRendererInternal.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

#import <Metal/MTLCommandQueue.h>
#import <QuartzCore/CAMetalLayer.h>

#if DS_IOS
#import <UIKit/UIView.h>
typedef UIView ViewType;
#else
#import <AppKit/NSView.h>
typedef NSView ViewType;
#endif

static bool createExtraSurfaces(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;
	if (renderer->surfaceSamples > 1)
	{
		id<MTLTexture> resolveSurface = (__bridge id<MTLTexture>)mtlRenderSurface->resolveSurface;
		if (!resolveSurface || resolveSurface.width != renderSurface->width ||
			resolveSurface.height != renderSurface->height)
		{
			if (mtlRenderSurface->resolveSurface)
			{
				CFRelease(mtlRenderSurface->resolveSurface);
				mtlRenderSurface->resolveSurface = NULL;
			}

			MTLPixelFormat pixelFormat =
				dsMTLResourceManager_getPixelFormat(renderer->resourceManager,
					renderer->surfaceColorFormat);
			MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
			if (!descriptor)
			{
				errno = ENOMEM;
				return false;
			}

			descriptor.textureType = MTLTextureType2DMultisample;
			descriptor.pixelFormat = pixelFormat;
			descriptor.width = renderSurface->width;
			descriptor.height = renderSurface->height;
			descriptor.sampleCount = renderer->surfaceSamples;
#if DS_MAC || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
			descriptor.storageMode = MTLStorageModePrivate;
			descriptor.usage = MTLTextureUsageRenderTarget;
#endif

			resolveSurface = [device newTextureWithDescriptor: descriptor];
			if (!resolveSurface)
			{
				errno = ENOMEM;
				return false;
			}

			mtlRenderSurface->resolveSurface = CFBridgingRetain(resolveSurface);
		}
	}

	if (renderer->surfaceDepthStencilFormat != dsGfxFormat_Unknown)
	{
		id<MTLTexture> depthSurface = (__bridge id<MTLTexture>)mtlRenderSurface->depthSurface;

		if (!depthSurface || depthSurface.width != renderSurface->width ||
			depthSurface.height != renderSurface->height)
		{
			MTLPixelFormat depthPixelFormat =
				dsMTLResourceManager_getPixelFormat(renderer->resourceManager,
					renderer->surfaceDepthStencilFormat);
			MTLPixelFormat stencilPixelFormat = depthPixelFormat;
			if (depthPixelFormat == MTLPixelFormatInvalid)
			{
				// Need to have separate depth and stencil surfaces.
				switch (renderer->surfaceDepthStencilFormat)
				{
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
					case dsGfxFormat_D16S8:
						depthPixelFormat = MTLPixelFormatDepth16Unorm;
						stencilPixelFormat = MTLPixelFormatStencil8;
						break;
#endif
					case dsGfxFormat_D32S8_Float:
						depthPixelFormat = MTLPixelFormatDepth32Float;
						stencilPixelFormat = MTLPixelFormatStencil8;
						break;
					default:
						DS_ASSERT(false);
						break;
				}
			}
			else
			{
				switch (renderer->surfaceDepthStencilFormat)
				{
					case dsGfxFormat_D16:
					case dsGfxFormat_X8D24:
					case dsGfxFormat_D32_Float:
						stencilPixelFormat = MTLPixelFormatInvalid;
						break;
					default:
						break;
				}
			}

			if (mtlRenderSurface->depthSurface)
			{
				CFRelease(mtlRenderSurface->depthSurface);
				mtlRenderSurface->depthSurface = NULL;
			}
			if (mtlRenderSurface->stencilSurface)
			{
				CFRelease(mtlRenderSurface->stencilSurface);
				mtlRenderSurface->stencilSurface = NULL;
			}

			MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
			if (!descriptor)
			{
				errno = ENOMEM;
				return false;
			}

			descriptor.textureType = MTLTextureType2DMultisample;
			descriptor.pixelFormat = depthPixelFormat;
			descriptor.width = renderSurface->width;
			descriptor.height = renderSurface->height;
			descriptor.sampleCount = renderer->surfaceSamples;
#if DS_MAC || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
			descriptor.storageMode = MTLStorageModePrivate;
			descriptor.usage = MTLTextureUsageRenderTarget;
#endif

			depthSurface = [device newTextureWithDescriptor: descriptor];
			if (!depthSurface)
			{
				errno = ENOMEM;
				return false;
			}

			mtlRenderSurface->depthSurface = CFBridgingRetain(depthSurface);

			if (stencilPixelFormat == depthPixelFormat)
				mtlRenderSurface->stencilSurface = CFBridgingRetain(depthSurface);
			else if (stencilPixelFormat != MTLPixelFormatInvalid)
			{
				descriptor.pixelFormat = stencilPixelFormat;
				id<MTLTexture> stencilSurface = [device newTextureWithDescriptor: descriptor];
				if (!stencilSurface)
				{
					errno = ENOMEM;
					return false;
				}
			}
		}
	}

	return true;
}

dsRenderSurface* dsMTLRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* osHandle, dsRenderSurfaceType type)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	if (type == dsRenderSurfaceType_Pixmap)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Pixmap not supported for Metal.");
		return NULL;
	}

	if (!osHandle)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG,
			"An NSView/UIView must be passed as the OS handle to create a Metal render surface.");
		return NULL;
	}

	ViewType* view = (__bridge ViewType*)osHandle;
	if (view.layer.class != [CAMetalLayer class])
	{
		CALayer* oldLayer = view.layer;
		CALayer* newLayer = [CAMetalLayer new];
		if (!newLayer)
		{
			errno = ENOMEM;
			return NULL;
		}

		newLayer.frame = oldLayer.frame;
		view.layer = newLayer;
#if DS_MAC
		view.wantsLayer = true;
#endif
	}
	((CAMetalLayer*)view.layer).device = device;

	MTLPixelFormat format = MTLPixelFormatBGRA8Unorm;
	if (renderer->surfaceColorFormat ==
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB))
	{
		format = MTLPixelFormatBGRA8Unorm_sRGB;
	}
	else if (renderer->surfaceColorFormat ==
		dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SRGB))
	{
		format = MTLPixelFormatRGBA16Float;
	}

	CAMetalLayer* layer = (CAMetalLayer*)view.layer;
	layer.pixelFormat = format;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	layer.displaySyncEnabled = renderer->vsync;
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
	layer.wantsExtendedDynamicRangeContent = format == MTLPixelFormatRGBA16Float;
#endif

	dsMTLRenderSurface* renderSurface = DS_ALLOCATE_OBJECT(allocator, dsMTLRenderSurface);
	if (!renderSurface)
		return NULL;

	dsRenderSurface* baseRenderSurface = (dsRenderSurface*)renderSurface;
	baseRenderSurface->renderer = renderer;
	baseRenderSurface->allocator = dsAllocator_keepPointer(allocator);
	baseRenderSurface->name = name;
	baseRenderSurface->surfaceType = type;

	CGSize size = layer.drawableSize;
	if (size.width == 0 || size.height == 0)
	{
		size = view.bounds.size;
		size.width *= layer.contentsScale;
		size.height *= layer.contentsScale;
	}
	baseRenderSurface->width = (uint32_t)size.width;
	baseRenderSurface->height = (uint32_t)size.height;

	renderSurface->view = CFBridgingRetain(view);
	renderSurface->layer = CFBridgingRetain(layer);
	renderSurface->drawable = NULL;
	renderSurface->resolveSurface = NULL;
	renderSurface->depthSurface = NULL;
	renderSurface->stencilSurface = NULL;

	renderSurface->drawable = CFBridgingRetain([layer nextDrawable]);
	if (!renderSurface->drawable)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG,
			"Render surface color format not supported for Metal view.");
		dsMTLRenderSurface_destroy(renderer, baseRenderSurface);
		return NULL;
	}

	if (!createExtraSurfaces(renderer, baseRenderSurface))
	{
		dsMTLRenderSurface_destroy(renderer, baseRenderSurface);
		return NULL;
	}

	return baseRenderSurface;
}

bool dsMTLRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;
	CAMetalLayer* layer = (__bridge CAMetalLayer*)mtlRenderSurface->layer;
	CGSize size = layer.drawableSize;
	renderSurface->width = (uint32_t)size.width;
	renderSurface->height = (uint32_t)size.height;

	// If the size has changed, need a new image.
	id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)mtlRenderSurface->drawable;
	id<MTLTexture> surface = drawable.texture;
	if (surface.width != renderSurface->width && surface.height != renderSurface->height)
	{
		CFRelease(mtlRenderSurface->drawable);
		mtlRenderSurface->drawable = CFBridgingRetain([layer nextDrawable]);
		DS_ASSERT(mtlRenderSurface->drawable);
	}

	return createExtraSurfaces(renderer, renderSurface);
}

bool dsMTLRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderSurface);
	return true;
}

bool dsMTLRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderSurface);
	return true;
}

bool dsMTLRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	uint32_t count)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtlRenderer->commandQueue;
	id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
	if (!commandBuffer)
		return false;

	for (uint32_t i = 0; i < count; ++i)
	{
		dsRenderSurface* renderSurface = renderSurfaces[i];
		dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;
		[commandBuffer presentDrawable: (__bridge id<CAMetalDrawable>)mtlRenderSurface->drawable];
		CFRelease(mtlRenderSurface->drawable);

		CAMetalLayer* layer = (__bridge CAMetalLayer*)mtlRenderSurface->layer;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
		if (layer.displaySyncEnabled != renderer->vsync)
			layer.displaySyncEnabled = renderer->vsync;
#endif
		mtlRenderSurface->drawable = CFBridgingRetain([layer nextDrawable]);
		DS_ASSERT(mtlRenderSurface->drawable);
		dsMTLRenderSurface_update(renderer, renderSurface);
	}

	dsMTLRenderer_flushImpl(renderer, commandBuffer);
	return true;
}

bool dsMTLRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;

	if (mtlRenderSurface->view)
		CFRelease(mtlRenderSurface->view);
	if (mtlRenderSurface->layer)
		CFRelease(mtlRenderSurface->layer);
	if (mtlRenderSurface->drawable)
		CFRelease(mtlRenderSurface->drawable);
	if (mtlRenderSurface->resolveSurface)
		CFRelease(mtlRenderSurface->resolveSurface);
	if (mtlRenderSurface->depthSurface)
		CFRelease(mtlRenderSurface->depthSurface);
	if (mtlRenderSurface->stencilSurface)
		CFRelease(mtlRenderSurface->stencilSurface);

	if (renderSurface->allocator)
		DS_VERIFY(dsAllocator_free(renderSurface->allocator, renderSurface));
	return true;
}
