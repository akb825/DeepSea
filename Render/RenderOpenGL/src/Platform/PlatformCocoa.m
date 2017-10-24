#include "Platform/Platform.h"
#include "AnyGL/AnyGL.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

#if DS_MAC
#include <AppKit/NSOpenGL.h>
#include <AppKit/NSView.h>

#define MAX_OPTION_SIZE 32

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

void* dsGetGLDisplay(void)
{
	return NULL;
}

void dsReleaseGLDisplay(void* display)
{
	DS_UNUSED(display);
}

void* dsCreateGLConfig(dsAllocator* allocator, void* display, const dsOpenGLOptions* options,
	bool render)
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
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1010
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

		if (render && options->samples > 1)
		{
			addOption2(attr, &optionCount, NSOpenGLPFASampleBuffers, 1);
			addOption2(attr, &optionCount, NSOpenGLPFASamples, options->samples);
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

void* dsGetPublicGLConfig(void* config)
{
	DS_UNUSED(config);
	return NULL;
}

void dsDestroyGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	if (!config)
		return;

	CFRelease(config);
}

void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
{
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

void dsDestroyGLContext(void* display, void* context)
{
	DS_UNUSED(display);
	if (!context)
		return;

	CFRelease(context);
}

void* dsCreateDummyGLSurface(dsAllocator* allocator, void* display, void* config, void** osSurface)
{
	DS_UNUSED(allocator);
	DS_UNUSED(display);
	DS_UNUSED(osSurface);

	return &dummyValue;
}

void dsDestroyDummyGLSurface(void* display, void* surface, void* osSurface)
{
	DS_UNUSED(display);
	DS_UNUSED(surface);
	DS_UNUSED(osSurface);
}

void* dsCreateGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle)
{
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

bool dsGetGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!outWidth || !outHeight || !surface)
		return false;

	NSRect bounds = [(__bridge NSView*)surface bounds];
	*outWidth = (uint32_t)bounds.size.width;
	*outHeight = (uint32_t)bounds.size.height;
	return true;
}

void dsSetGLSurfaceVsync(void* display, dsRenderSurfaceType surfaceType, void* surface, bool vsync)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	DS_UNUSED(surface);

	GLint parameter = vsync;
	[[NSOpenGLContext currentContext] setValues: &parameter
		forParameter: NSOpenGLContextParameterSwapInterval];
}

void dsSwapGLBuffers(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	NSView* view = (__bridge NSView*)surface;
	if ([context view] != view)
	{
		glFlush();
		[context setView: view];
	}
	[context flushBuffer];
}

void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	CFBridgingRelease(surface);
}

bool dsBindGLContext(void* display, void* context, void* surface)
{
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

	return true;
}

void* dsGetCurrentGLContext(void* display)
{
	DS_UNUSED(display);
	return (__bridge void*)[NSOpenGLContext currentContext];
}

#endif // DS_MAC
