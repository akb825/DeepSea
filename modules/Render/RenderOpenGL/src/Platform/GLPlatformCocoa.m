/*
 * Copyright 2017-2025 Aaron Barany
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

#include "Platform/GLPlatformCocoa.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>
#include <string.h>

#if DS_MAC
#include <AppKit/NSOpenGL.h>
#include <AppKit/NSView.h>

#define MAX_OPTION_SIZE 32

// Yes, I understand that OpenGL is deprecated. No, I cannot and will not use NSOpenGLView.
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static int dummyValue;

static void addOption(NSOpenGLPixelFormatAttribute* attr, unsigned int* size,
	NSOpenGLPixelFormatAttribute option)
{
	DS_ASSERT(*size + 1 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
}

static void addOption2(NSOpenGLPixelFormatAttribute* attr, unsigned int* size,
	NSOpenGLPixelFormatAttribute option, NSOpenGLPixelFormatAttribute value)
{
	DS_ASSERT(*size + 2 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
	attr[(*size)++] = value;
}

void* dsGetCocoaGLDisplay(void* osDisplay)
{
	DS_UNUSED(osDisplay);
	return NULL;
}

void dsReleaseCocoaGLDisplay(void* osDisplay, void* gfxDisplay)
{
	DS_UNUSED(osDisplay);
	DS_UNUSED(gfxDisplay);
}

void* dsCreateCocoaGLConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	GLContextType contextType)
{
	DS_UNUSED(allocator);
	DS_UNUSED(display);

	if (options->srgb || options->stereoscopic)
	{
		errno = EPERM;
		return NULL;
	}

	@autoreleasepool
	{
		NSOpenGLPixelFormatAttribute versions[] =
		{
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1010
			NSOpenGLProfileVersion4_1Core,
#endif
			NSOpenGLProfileVersion3_2Core, NSOpenGLProfileVersionLegacy
		};

		unsigned int optionCount = 0;
		NSOpenGLPixelFormatAttribute attr[MAX_OPTION_SIZE];
		addOption2(attr, &optionCount, NSOpenGLPFAOpenGLProfile, 0);
		addOption2(attr, &optionCount, NSOpenGLPFAColorSize, options->redBits +
			options->greenBits + options->blueBits);
		addOption2(attr, &optionCount, NSOpenGLPFAAlphaSize, options->alphaBits);
		addOption2(attr, &optionCount, NSOpenGLPFADepthSize, options->depthBits);
		addOption2(attr, &optionCount, NSOpenGLPFAStencilSize, options->stencilBits);
		if (options->doubleBuffer)
			addOption(attr, &optionCount, NSOpenGLPFADoubleBuffer);

		if (contextType == GLContextType_Render && options->surfaceSamples > 1)
		{
			addOption2(attr, &optionCount, NSOpenGLPFASampleBuffers, 1);
			addOption2(attr, &optionCount, NSOpenGLPFASamples, options->surfaceSamples);
		}
		else
		{
			addOption2(attr, &optionCount, NSOpenGLPFASampleBuffers, 0);
			addOption2(attr, &optionCount, NSOpenGLPFASamples, 0);
		}

		DS_ASSERT(optionCount < MAX_OPTION_SIZE);
		attr[optionCount] = 0;

		for (size_t i = 0; i < DS_ARRAY_SIZE(versions); ++i)
		{
			attr[1] = versions[i];
			NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attr];
			if (format)
				return (void*)CFBridgingRetain(format);
		}
	}

	errno = EPERM;
	return NULL;
}

void* dsGetPublicCocoaGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	DS_UNUSED(config);
	return NULL;
}

void dsDestroyCocoaGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	if (!config)
		return;

	CFRelease(config);
}

void* dsCreateCocoaGLContext(
	dsAllocator* allocator, void* display, void* config, void* shareContext)
{
	DS_UNUSED(display);
	DS_UNUSED(allocator);
	if (!config)
		return NULL;

	@autoreleasepool
	{
		return (void*)CFBridgingRetain([[NSOpenGLContext alloc]
			initWithFormat: (__bridge NSOpenGLPixelFormat*)config
			shareContext: (__bridge NSOpenGLContext*)shareContext]);
	}
}

void dsDestroyCocoaGLContext(void* display, void* context)
{
	DS_UNUSED(display);
	if (!context)
		return;

	CFRelease(context);
}

void* dsCreateDummyCocoaGLSurface(
	dsAllocator* allocator, void* display, void* config, void** osSurface)
{
	DS_UNUSED(allocator);
	DS_UNUSED(display);
	DS_UNUSED(config);
	DS_UNUSED(osSurface);

	return &dummyValue;
}

void dsDestroyDummyCocoaGLSurface(void* display, void* surface, void* osSurface)
{
	DS_UNUSED(display);
	DS_UNUSED(surface);
	DS_UNUSED(osSurface);
}

void* dsCreateCocoaGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle)
{
	DS_UNUSED(display);
	DS_UNUSED(config);
	DS_UNUSED(allocator);
	switch (surfaceType)
	{
		case dsRenderSurfaceType_Pixmap:
			return NULL;
		case dsRenderSurfaceType_Window:
		default:
			return (void*)CFBridgingRetain((__bridge NSView*)handle);
	}
}

bool dsGetCocoaGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	if (!outWidth || !outHeight || !surface)
		return false;

	NSView* view = (__bridge NSView*)surface;
	NSSize size = [view convertSizeToBacking: view.bounds.size];
	*outWidth = (uint32_t)size.width;
	*outHeight = (uint32_t)size.height;

	// Make sure the current size is reflected in the context.
	NSOpenGLContext* curContext = [NSOpenGLContext currentContext];
	if (curContext)
		[curContext update];

	return true;
}

void dsSwapCocoaGLBuffers(
	void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync)
{
	DS_UNUSED(display);
	DS_UNUSED(vsync);

	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	for (size_t i = 0; i < count; ++i)
	{
		NSView* view = (__bridge NSView*)((dsGLRenderSurface*)renderSurfaces[i])->glSurface;
		// Would normally call glFlush(), but dsGLRenderSurface_swapBuffers() already calls it.
		if ([context view] != view)
			[context setView: view];
		[context flushBuffer];
	}
}

void dsDestroyCocoaGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	CFRelease(surface);
}

bool dsBindCocoaGLContext(void* display, void* context, void* surface)
{
	DS_PROFILE_FUNC_START();
	DS_UNUSED(display);
	NSOpenGLContext* nsContext = (__bridge NSOpenGLContext*)context;
	NSOpenGLContext* curContext = [NSOpenGLContext currentContext];
	NSView* view = surface == &dummyValue ? NULL : (__bridge NSView*)surface;
	NSView* curView = nsContext ? [nsContext view] : NULL;

	// Flush if we're changing contexts or views. (assuming a context is bound)
	if (curContext && (curContext != nsContext || (nsContext && curView != view)))
		glFlush();

	// Update the context if different.
	if (curContext != nsContext)
	{
		if (nsContext)
			[nsContext makeCurrentContext];
		else
			[NSOpenGLContext clearCurrentContext];
	}

	// Update the view if different.
	if (nsContext && curView != view)
		[nsContext setView: view];

	// Update the context if set.
	if (nsContext)
		[nsContext update];

	DS_PROFILE_FUNC_RETURN(true);
}

void* dsGetCurrentCocoaGLContext(void* display)
{
	DS_UNUSED(display);
	return (__bridge void*)[NSOpenGLContext currentContext];
}

void dsSetCocoaGLVSync(void* display, void* surface, bool vsync)
{
	DS_UNUSED(display);
	DS_UNUSED(surface);

	GLint parameter = vsync;
	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	[context setValues: &parameter forParameter: NSOpenGLContextParameterSwapInterval];
}

#endif // DS_MAC
