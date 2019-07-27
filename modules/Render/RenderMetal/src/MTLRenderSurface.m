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
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

#import <Metal/MTLCommandQueue.h>
#import <QuartzCore/CAMetalLayer.h>

#if DS_MAC
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
typedef NSView ViewType;
#else
#import <UIKit/UIView.h>
#import <UIKit/UIScreen.h>
#import <UIKit/UIWindow.h>
typedef UIView ViewType;
#endif

@interface DSMetalView : ViewType
{
#if DS_IOS
	CGFloat contentsScale;
#endif
}

- (instancetype)initWithFrame:(CGRect)frame
#if DS_IOS
	contentsScale: (CGFloat)scale
#endif
	;

@end

@implementation DSMetalView

+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
#if DS_IOS
	contentsScale: (CGFloat)scale
#endif
{
	if ((self = [super initWithFrame: frame]))
	{
#if DS_MAC
		self.wantsLayer = true;
		self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
#else
		self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		self.multipleTouchEnabled = true;
		self->contentsScale = scale;
#endif
		[self updateDrawableSize];
	}

	return self;
}

- (void)updateDrawableSize
{
	CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
	CGSize size = self.bounds.size;

#if DS_MAC
	CGSize backingSize = [self convertSizeToBacking: size];
	metalLayer.contentsScale = backingSize.height/size.height;
	size = backingSize;
#else
	metalLayer.contentsScale = self->contentsScale;
	size.width *= self->contentsScale;
    size.height *= self->contentsScale;
#endif

	metalLayer.drawableSize = size;
}

#if DS_IOS

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self updateDrawableSize];
}

#else

- (BOOL)wantsUpdateLayer
{
	return true;
}

- (CALayer*)makeBackingLayer
{
	return [CAMetalLayer layer];
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
    [super resizeWithOldSuperviewSize: oldSize];
    [self updateDrawableSize];
}

- (void)viewWillMoveToWindow:(NSWindow*)window
{
	NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
	if (self.window)
	{
		[center removeObserver: self name: NSWindowDidChangeBackingPropertiesNotification
			object: self.window];
	}

	if (window)
	{
		[center addObserver: self selector: @selector(didChangeBackingProperties:)
			name: NSWindowDidChangeBackingPropertiesNotification object: window];
	}
}

- (void)viewDidMoveToWindow
{
	[super viewDidMoveToWindow];
	[self updateDrawableSize];
}

- (void)didChangeBackingProperties:(NSNotification*)notification
{
	DS_UNUSED(notification);
	[self updateDrawableSize];
}

#endif

@end

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
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
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
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
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

			if (renderer->surfaceSamples > 1)
				descriptor.textureType = MTLTextureType2DMultisample;
			else
				descriptor.textureType = MTLTextureType2D;
			descriptor.pixelFormat = depthPixelFormat;
			descriptor.width = renderSurface->width;
			descriptor.height = renderSurface->height;
			descriptor.sampleCount = renderer->surfaceSamples;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
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
	@autoreleasepool
	{
		dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
		if (type == dsRenderSurfaceType_Pixmap)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Pixmap not supported for Metal.");
			return NULL;
		}

		NSObject* handleObject = (__bridge NSObject*)osHandle;
		if (!handleObject || (![handleObject isKindOfClass: [ViewType class]] &&
				![handleObject isKindOfClass: [CAMetalLayer class]]))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "An NSView/UIView or CAMetalLayer must be passed "
				"as the OS handle to create a Metal render surface.");
			return NULL;
		}

		ViewType* view = NULL;
		CAMetalLayer* layer;
		if ([handleObject isKindOfClass: [ViewType class]])
		{
			view = (ViewType*)handleObject;
			if (view.layer.class != [CAMetalLayer class])
			{
				DSMetalView* metalView = [[DSMetalView alloc] initWithFrame: view.frame
#if DS_IOS
					contentsScale: view.window.screen.scale
#endif
					];
				if (!metalView)
				{
					errno = ENOMEM;
					return NULL;
				}

				[view addSubview: metalView];
				view = metalView;
			}

			layer = (CAMetalLayer*)view.layer;
		}
		else
			layer = (CAMetalLayer*)handleObject;

		layer.device = device;

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

		layer.pixelFormat = format;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
		layer.displaySyncEnabled = renderer->vsync;
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
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
		baseRenderSurface->width = (uint32_t)size.width;
		baseRenderSurface->height = (uint32_t)size.height;

		DS_VERIFY(dsSpinlock_initialize(&renderSurface->lock));
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
				"Render surface color format not supported for Metal surface.");
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
}

bool dsMTLRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;
		CAMetalLayer* layer = (__bridge CAMetalLayer*)mtlRenderSurface->layer;
		CGSize size = layer.drawableSize;
		renderSurface->width = (uint32_t)size.width;
		renderSurface->height = (uint32_t)size.height;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
		if (layer.displaySyncEnabled != renderer->vsync)
			layer.displaySyncEnabled = renderer->vsync;
#endif

		return createExtraSurfaces(renderer, renderSurface);
	}
}

bool dsMTLRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);

	dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;
	DS_VERIFY(dsSpinlock_lock(&mtlRenderSurface->lock));
	if (mtlRenderSurface->drawable)
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlRenderSurface->lock));
		return true;
	}

	CAMetalLayer* layer = (__bridge CAMetalLayer*)mtlRenderSurface->layer;
	mtlRenderSurface->drawable = CFBridgingRetain([layer nextDrawable]);
	DS_ASSERT(mtlRenderSurface->drawable);
	dsMTLRenderSurface_update(renderer, (dsRenderSurface*)renderSurface);
	DS_VERIFY(dsSpinlock_unlock(&mtlRenderSurface->lock));
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
	@autoreleasepool
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
			[commandBuffer
				presentDrawable: (__bridge id<CAMetalDrawable>)mtlRenderSurface->drawable];
			CFRelease(mtlRenderSurface->drawable);
			mtlRenderSurface->drawable = NULL;
		}

		dsMTLRenderer_flushImpl(renderer, commandBuffer);
		return true;
	}
}

bool dsMTLRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	dsMTLRenderSurface* mtlRenderSurface = (dsMTLRenderSurface*)renderSurface;

	dsSpinlock_shutdown(&mtlRenderSurface->lock);
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
