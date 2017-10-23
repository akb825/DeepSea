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

	NSOpenGLPixelFormatAttribute versions[] = {0x4100, 0x3200, 0x1000};

	unsigned int optionCount = 0;
	NSOpenGLPixelFormatAttribute attr[MAX_OPTION_SIZE];
	addOption2(attr, &optionCount, NSOpenGLPFAOpenGLProfile, 0);
	addOption2(attr, &optionCount, NSOpenGLPFAColorSize, options->redBits + options->greenBits +
		options->blueBits);
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
			return format;
	}

	errno = EPERM;
	return NULL;
}

void dsDestroyGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	if (!config)
		return;

	[(NSOpenGLPixelFormat*)config release];
}

void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
{
	DS_UNUSED(allocator);
	if (!config)
		return NULL;

	return [[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)config
		shareContext: (NSOpenGLContext*)shareContext];
}

void dsDestroyGLContext(void* display, void* context)
{
	DS_UNUSED(display);
	if (!context)
		return;

	[(NSOpenGLContext*)context release];
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
		case dsRenderSurfaceType_Window:
			return [(NSView*)handle retain];
		case dsRenderSurfaceType_Pixmap:
			return NULL;
		default:
			return [(NSView*)handle retain];
	}
}

bool dsGetGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!outWidth || !outHeight || !surface)
		return false;

	NSRect bounds = [(NSView*)surface bounds];
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

	glFlush();
	NSOpenGLContext* context = [NSOpenGLContext currentContext];
	[context setView: (NSView*)surface];
	[context flushBuffer];
}

void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	[(NSView*)surface release];
}

bool dsBindGLContext(void* display, void* context, void* surface)
{
	if ([NSOpenGLContext currentContext])
		glFlush();

	if (context)
	{
		NSOpenGLContext* nsContext = (NSOpenGLContext*)context;
		[nsContext makeCurrentContext];
		if (surface == &dummyValue)
			[nsContext setView: NULL];
		else
		{
			[nsContext setView: (NSView*)surface];
			[nsContext update];
		}
	}
	else
		[NSOpenGLContext clearCurrentContext];

	return true;
}

void* dsGetCurrentGLContext(void* display)
{
	DS_UNUSED(display);
	return [NSOpenGLContext currentContext];
}

#endif // DS_MAC
