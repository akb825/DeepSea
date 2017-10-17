#include "Platform/Platform.h"
#include "AnyGL/AnyGL.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

#if ANYGL_LOAD == ANYGL_LOAD_WGL
#include "AnyGL/wgl.h"

#define MAX_OPTION_SIZE 32

typedef struct Config
{
	dsAllocator* allocator;
	HINSTANCE hinst;
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat;
	HWND window;
	HDC dc;
	bool debug;
	GLint major;
	GLint minor;
} Config;

// Declared by AnyGL for its internal management.
static const char* windowClass = "AnyGLDummyWindow";

static void addOption(GLint* attr, unsigned int* size, GLint option, GLint value)
{
	DS_ASSERT(*size + 2 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
	attr[(*size)++] = value;
}

static bool hasExtension(const char* extensions, const char* extension)
{
	if (!extensions)
		return false;

	size_t begin = 0, end = 0;
	while (extensions[begin])
	{
		for (end = begin; extensions[end] && extensions[end] != ' '; ++end)
			; /* empty */
		if (strncmp(extensions + begin, extension, end - begin) == 0)
			return true;

		begin = extensions[end] == ' ' ? end + 1 : end;
	}

	return false;
}

void* dsGetGLDisplay(void)
{
	return GetDC(NULL);
}

void dsReleaseGLDisplay(void* display)
{
	ReleaseDC(NULL, display);;
}

void* dsCreateGLConfig(dsAllocator* allocator, void* display, const dsOpenGLOptions* options,
	bool render)
{
	if (!allocator || !display)
	{
		errno = EINVAL;
		return NULL;
	}

	const char* extensions = NULL;
	if (ANYGL_SUPPORTED(wglGetExtensionsStringARB))
		extensions = wglGetExtensionsStringARB(display);

	unsigned int pfdFlags = 0;
	if (options->doubleBuffer)
		pfdFlags |= PFD_DOUBLEBUFFER;
	if (options->stereoscopic)
		pfdFlags |= PFD_STEREO;
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | pfdFlags,
		PFD_TYPE_RGBA,
		0,
		options->redBits, 0, options->greenBits, 0, options->blueBits, 0,
		options->alphaBits,
		0,
		0,
		0, 0, 0, 0,
		options->depthBits,
		options->stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pixelFormat;
	if (ANYGL_SUPPORTED(wglChoosePixelFormatARB))
	{
		unsigned int optionCount = 0;
		GLint attr[MAX_OPTION_SIZE];
		addOption(attr, &optionCount, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
		addOption(attr, &optionCount, WGL_DRAW_TO_WINDOW_ARB, true);
		addOption(attr, &optionCount, WGL_RED_BITS_ARB, options->redBits);
		addOption(attr, &optionCount, WGL_GREEN_BITS_ARB, options->greenBits);
		addOption(attr, &optionCount, WGL_BLUE_BITS_ARB, options->blueBits);
		addOption(attr, &optionCount, WGL_ALPHA_BITS_ARB, options->alphaBits);
		addOption(attr, &optionCount, WGL_DEPTH_BITS_ARB, options->depthBits);
		addOption(attr, &optionCount, WGL_STENCIL_BITS_ARB, options->stencilBits);
		addOption(attr, &optionCount, WGL_DOUBLE_BUFFER_ARB, options->doubleBuffer);
		addOption(attr, &optionCount, WGL_STEREO_ARB, options->stereoscopic);
		if (hasExtension(extensions, "WGL_ARB_multisample"))
		{
			if (render && options->samples > 1)
			{
				addOption(attr, &optionCount, WGL_SAMPLE_BUFFERS_ARB, 1);
				addOption(attr, &optionCount, WGL_SAMPLES_ARB, options->samples);
			}
			else
			{
				addOption(attr, &optionCount, WGL_SAMPLE_BUFFERS_ARB, 0);
				addOption(attr, &optionCount, WGL_SAMPLES_ARB, 0);
			}
		}

		if (options->srgb && hasExtension(extensions, "WGL_ARB_framebuffer_sRGB"))
			addOption(attr, &optionCount, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, true);

		DS_ASSERT(optionCount < MAX_OPTION_SIZE);
		attr[optionCount] = 0;

		UINT formatCount;
		if (!wglChoosePixelFormatARB(display, attr, NULL, 1, &pixelFormat, &formatCount) ||
			formatCount == 0)
		{
			return NULL;
			errno = EPERM;
		}
	}
	else
	{
		pixelFormat = ChoosePixelFormat(display, &pfd);
		if (!pixelFormat)
		{
			errno = EPERM;
			return NULL;
		}
	}

	HINSTANCE hinst = GetModuleHandle(NULL);
	HWND window = CreateWindowA(windowClass, "Dummy", 0, 0, 0, 0, 0, NULL, NULL, hinst, NULL);
	if (!window)
	{
		errno = EPERM;
		return NULL;
	}

	HDC dc = GetDC(window);
	if (!SetPixelFormat(dc, pixelFormat, &pfd))
	{
		ReleaseDC(window, dc);
		DestroyWindow(window);
		errno = EPERM;
		return NULL;
	}

	Config* config = (Config*)dsAllocator_alloc(allocator, sizeof(Config));
	if (!config)
	{
		ReleaseDC(window, dc);
		DestroyWindow(window);
		return NULL;
	}

	config->allocator = dsAllocator_keepPointer(allocator);
	config->hinst = hinst;
	config->pfd = pfd;
	config->pixelFormat = pixelFormat;
	config->window = window;
	config->dc = dc;
	config->debug = options->debug;
	config->major = 1;
	config->minor = 0;

	if (ANYGL_SUPPORTED(wglCreateContextAttribsARB))
	{
		static GLint versions[][2] =
		{
			{4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
			{3, 3}, {3, 2}, {3, 1}, {3, 0}
		};

		GLint contextAttr[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 0,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		unsigned int versionCount = (unsigned int)DS_ARRAY_SIZE(versions);
		for (unsigned int i = 0; i < versionCount; ++i)
		{
			contextAttr[1] = versions[i][0];
			contextAttr[3] = versions[i][1];
			HGLRC context = wglCreateContextAttribsARB(config->dc, NULL, contextAttr);
			if (context)
			{
				config->major = versions[i][0];
				config->minor = versions[i][1];
				wglDeleteContext(context);
				break;
			}
		}
	}

	return config;
}

void dsDestroyGLConfig(void* display, void* config)
{
	DS_UNUSED(display);
	Config* configPtr = (Config*)config;
	if (!configPtr)
		return;

	ReleaseDC(configPtr->window, configPtr->dc);
	DestroyWindow(configPtr->window);

	if (configPtr->allocator)
		dsAllocator_free(configPtr->allocator, configPtr);
}

void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!configPtr)
		return NULL;

	if (ANYGL_SUPPORTED(wglCreateContextAttribsARB))
	{
		GLint flags = configPtr->debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;
		GLint attr[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, configPtr->major,
			WGL_CONTEXT_MINOR_VERSION_ARB, configPtr->minor,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, flags,
			0
		};

		return wglCreateContextAttribsARB(configPtr->dc, shareContext, attr);
	}
	else
	{
		HGLRC context = wglCreateContext(configPtr->dc);
		if (!context)
			return NULL;

		if (shareContext)
			wglShareLists(shareContext, context);
		return context;
	}
}

void dsDestroyGLContext(void* display, void* context)
{
	DS_UNUSED(display);
	if (!context)
		return;

	wglDeleteContext(context);
}

void* dsCreateDummyGLSurface(dsAllocator* allocator, void* display, void* config, void** osSurface)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !osSurface)
		return NULL;

	HWND window = CreateWindowA(windowClass, "Dummy", 0, 0, 0, 0, 0, NULL, NULL, configPtr->hinst,
		NULL);
	if (!window)
		return NULL;

	HDC dc = GetDC(window);
	if (!SetPixelFormat(dc, configPtr->pixelFormat, &configPtr->pfd))
	{
		ReleaseDC(window, dc);
		DestroyWindow(window);
		return NULL;
	}

	*osSurface = window;
	return dc;
}

void dsDestroyDummyGLSurface(void* display, void* surface, void* osSurface)
{
	if (!surface)
		return;

	ReleaseDC(osSurface, surface);
	DestroyWindow(osSurface);
}

void* dsCreateGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !handle)
		return NULL;

	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
		{
			HDC dc = GetDC(handle);
			if (!SetPixelFormat(dc, configPtr->pixelFormat, &configPtr->pfd))
			{
				ReleaseDC(handle, dc);
				return NULL;
			}
			return dc;
		}
		case dsRenderSurfaceType_Pixmap:
		{
			HDC dc = CreateCompatibleDC(configPtr->dc);
			if (!dc)
				return NULL;

			if (!SelectObject(dc, handle))
			{
				DeleteDC(dc);
				return NULL;
			}

			return dc;
		}
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

	HWND window = WindowFromDC(surface);
	if (window)
	{
		RECT rect;
		if (!GetClientRect(window, &rect))
			return false;

		*outWidth = rect.right - rect.left;
		*outHeight = rect.bottom - rect.top;
		return true;
	}

	HGDIOBJ hBitmap = GetCurrentObject(surface, OBJ_BITMAP);
	BITMAP bitmapHeader;
	if (GetObject(hBitmap, sizeof(bitmapHeader), &bitmapHeader) == 0)
		return false;

	*outWidth = bitmapHeader.bmWidth;
	*outHeight = bitmapHeader.bmHeight;
	return true;
}

void dsSetGLSurfaceVsync(void* display, dsRenderSurfaceType surfaceType, void* surface, bool vsync)
{
	DS_UNUSED(display);
	DS_UNUSED(surfaceType);
	DS_UNUSED(surface);
	if (!ANYGL_SUPPORTED(wglSwapIntervalEXT))
		return;

	wglSwapIntervalEXT(vsync);
}

void dsSwapGLBuffers(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!surface)
		return;

	wglSwapLayerBuffers(surface, WGL_SWAP_MAIN_PLANE);
}

void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	if (!surface)
		return;

	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
			ReleaseDC(WindowFromDC(surface), surface);
			break;
		case dsRenderSurfaceType_Pixmap:
		{
			// Replace the DC bitmap to prevent it from deleting the user bitmap.
			HBITMAP bitmap = CreateCompatibleBitmap(surface, 1, 1);
			SelectObject(surface, bitmap);
			DeleteDC(surface);
			break;
		}
		default:
			break;
	}
}

bool dsBindGLContext(void* display, void* context, void* surface)
{
	if (!wglMakeCurrent(surface, context))
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't bind GL context.");
		return false;
	}

	return true;
}

void* dsGetCurrentGLContext(void* display)
{
	DS_UNUSED(display);
	return wglGetCurrentContext();
}

#endif // ANYGL_LOAD