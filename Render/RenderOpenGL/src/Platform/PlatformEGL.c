#include "Platform/Platform.h"
#include "AnyGL/AnyGL.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

#if ANYGL_LOAD == ANYGL_LOAD_EGL
#include <EGL/egl.h>

#ifndef EGL_OPENGL_ES3_BIT
#define EGL_OPENGL_ES3_BIT 0x00000040
#endif

#define MAX_OPTION_SIZE 32

typedef struct Config
{
	dsAllocator* allocator;
	EGLConfig config;
	GLint version;
	bool srgb;
} Config;

static void addOption(GLint* attr, unsigned int* size, GLint option, GLint value)
{
	DS_ASSERT(*size + 2 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
	attr[(*size)++] = value;
}

void* dsGetGLDisplay(void)
{
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!display)
		return NULL;

	eglInitialize(display, NULL, NULL);
	return display;
}

void dsReleaseGLDisplay(void* display)
{
	eglTerminate((EGLDisplay)display);
}

void* dsCreateGLConfig(dsAllocator* allocator, void* display, const dsOpenGLOptions* options,
	bool render)
{
	if (!allocator || !display)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!options->doubleBuffer)
	{
		errno = EPERM;
		return NULL;
	}

	unsigned int optionCount = 0;
	GLint attr[MAX_OPTION_SIZE];
	addOption(attr, &optionCount, EGL_RENDERABLE_TYPE, 0);
	GLint surfaces = EGL_WINDOW_BIT;
	// Use pbuffer as a dummy surface.
	if (!render)
		surfaces |= EGL_PBUFFER_BIT;
	addOption(attr, &optionCount, EGL_SURFACE_TYPE, surfaces);
	addOption(attr, &optionCount, EGL_RED_SIZE, options->redBits);
	addOption(attr, &optionCount, EGL_GREEN_SIZE, options->greenBits);
	addOption(attr, &optionCount, EGL_BLUE_SIZE, options->blueBits);
	addOption(attr, &optionCount, EGL_ALPHA_SIZE, options->alphaBits);
	addOption(attr, &optionCount, EGL_DEPTH_SIZE, options->depthBits);
	addOption(attr, &optionCount, EGL_STENCIL_SIZE, options->stencilBits);
	if (render && options->samples > 1)
	{
		addOption(attr, &optionCount, EGL_SAMPLE_BUFFERS, 1);
		addOption(attr, &optionCount, EGL_SAMPLES, options->samples);
	}
	else
	{
		addOption(attr, &optionCount, EGL_SAMPLE_BUFFERS, 0);
		addOption(attr, &optionCount, EGL_SAMPLES, 0);
	}

	DS_ASSERT(optionCount < MAX_OPTION_SIZE);
	attr[optionCount] = EGL_NONE;

#if ANYGL_GLES_VERSION >= 30
	GLint versions[] = {EGL_OPENGL_ES3_BIT, EGL_OPENGL_ES2_BIT};
#else
	GLint versions[] = {EGL_OPENGL_ES3_BIT};
#endif
	GLint version = 0;
	EGLConfig eglConfig = NULL;
	for (size_t i = 0; i < DS_ARRAY_SIZE(versions); ++i)
	{
		version = versions[i];
		attr[1] = version;
		GLint configCount = 0;
		if (eglChooseConfig((EGLDisplay)display, attr, &eglConfig, 1, &configCount) &&
			configCount > 0)
		{
			break;
		}
	}

	if (!eglConfig)
		return NULL;

	Config* config = (Config*)dsAllocator_alloc(allocator, sizeof(Config));
	if (!config)
		return NULL;

	config->allocator = dsAllocator_keepPointer(allocator);
	config->config = eglConfig;
	config->version = version == EGL_OPENGL_ES3_BIT ? 3 : 2;
	config->srgb = options->srgb;
	return config;
}

void* dsGetPublicGLConfig(void* config)
{
	DS_UNUSED(config);
	return NULL;
}

void dsDestroyGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	Config* configPtr = (Config*)config;
	if (!configPtr)
		return;

	if (configPtr->allocator)
		dsAllocator_free(configPtr->allocator, configPtr);
}

void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr)
		return NULL;

	GLint attr[] = {EGL_CONTEXT_CLIENT_VERSION, configPtr->version, EGL_NONE};
	return eglCreateContext((EGLDisplay)display, configPtr->config, (EGLContext)shareContext, attr);
}

void dsDestroyGLContext(void* display, void* context)
{
	if (!context)
		return;

	eglDestroyContext((EGLDisplay)display, (EGLContext)context);
}

void* dsCreateDummyGLSurface(dsAllocator* allocator, void* display, void* config, void** osSurface)
{
	DS_UNUSED(allocator);
	DS_UNUSED(osSurface);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !osSurface)
		return NULL;

	GLint attr[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
	return eglCreatePbufferSurface((EGLDisplay)display, configPtr->config, attr);
}

void dsDestroyDummyGLSurface(void* display, void* surface, void* osSurface)
{
	DS_UNUSED(osSurface);
	if (!surface)
		return;

	eglDestroySurface((EGLDisplay)display, (EGLSurface)surface);
}

void* dsCreateGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !handle)
		return NULL;

	// If sRGB is requested, it means convert from linear to sRGB.
	GLint attr[] = {EGL_COLORSPACE, configPtr->srgb ? EGL_COLORSPACE_LINEAR : EGL_COLORSPACE_sRGB,
		EGL_NONE};
	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
			return eglCreateWindowSurface((EGLDisplay)display, configPtr->config,
				(NativeWindowType)handle, attr);
		case dsRenderSurfaceType_Pixmap:
			return eglCreatePixmapSurface((EGLDisplay)display, configPtr->config,
				(NativePixmapType)handle, attr);
		default:
			return handle;
	}
}

bool dsGetGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!outWidth || !outHeight || !surface)
		return false;

	GLint width, height;
	if (!eglQuerySurface((EGLDisplay)display, (EGLSurface)surface, EGL_WIDTH, &width))
		return false;
	if (!eglQuerySurface((EGLDisplay)display, (EGLSurface)surface, EGL_HEIGHT, &height))
		return false;

	*outWidth = width;
	*outHeight = height;
	return true;
}

void dsSetGLSurfaceVsync(void* display, dsRenderSurfaceType surfaceType, void* surface, bool vsync)
{
	DS_UNUSED(surfaceType);
	DS_UNUSED(surface);

	eglSwapInterval((EGLDisplay)display, vsync);
}

void dsSwapGLBuffers(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	eglSwapBuffers((EGLDisplay)display, (EGLSurface)surface);
}

void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	if (!surface)
		return;

	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
		case dsRenderSurfaceType_Pixmap:
			eglDestroySurface((EGLDisplay)display, (EGLSurface)surface);
			break;
		default:
			break;
	}
}

bool dsBindGLContext(void* display, void* context, void* surface)
{
	if (!eglMakeCurrent((EGLDisplay)display, (EGLSurface)surface, (EGLSurface)surface,
		(EGLContext)context))
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't bind GL context.");
		return false;
	}

	return true;
}

void* dsGetCurrentGLContext(void* display)
{
	DS_UNUSED(display);
	return eglGetCurrentContext();
}

#endif // ANYGL_LOAD
