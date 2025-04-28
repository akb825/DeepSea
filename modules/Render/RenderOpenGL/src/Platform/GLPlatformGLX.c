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

#include "Platform/GLPlatformGLX.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>
#include <string.h>

#if ANYGL_HAS_GLX
#include "AnyGL/glx.h"

#define MAX_OPTION_SIZE 32

typedef struct Config
{
	dsAllocator* allocator;
	XVisualInfo* visualInfo;
	GLXFBConfig config;
	bool debug;
	GLint major;
	GLint minor;
} Config;

static void addOption(GLint* attr, unsigned int* size, GLint option)
{
	DS_ASSERT(*size + 1 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
}

static void addOption2(GLint* attr, unsigned int* size, GLint option, GLint value)
{
	DS_ASSERT(*size + 2 < MAX_OPTION_SIZE);
	attr[(*size)++] = option;
	attr[(*size)++] = value;
}

static bool hasExtension(const char* extensions, const char* extension)
{
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

static GLint glVersions[][2] =
{
	{4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
	{3, 3}, {3, 2}, {3, 1}, {3, 0}
};

typedef int (*X11ErrorHandler)(Display*, XErrorEvent*);
static bool gX11Error;
static int emptyErrorHandler(Display* display, XErrorEvent* event)
{
	DS_UNUSED(display);
	DS_UNUSED(event);
	gX11Error = true;
	return 0;
}

void* dsGetGLXDisplay(void* osDisplay)
{
	return XOpenDisplay(osDisplay);
}

void dsReleaseGLXDisplay(void* osDisplay, void* gfxDisplay)
{
	DS_UNUSED(osDisplay);
	XCloseDisplay(gfxDisplay);
}

void* dsCreateGLXConfig(
	dsAllocator* allocator, void* display, const dsRendererOptions* options,
	GLContextType contextType)
{
	if (!allocator || !display)
	{
		errno = EINVAL;
		return NULL;
	}

	int screen = DefaultScreen(display);
	const char* extensions = glXQueryExtensionsString(display, screen);
	DS_ASSERT(extensions);

	unsigned int optionCount = 0;
	GLint attr[MAX_OPTION_SIZE];
	if (ANYGL_SUPPORTED(glXChooseFBConfig))
	{
		addOption2(attr, &optionCount, GLX_RENDER_TYPE, GLX_RGBA_BIT);
		addOption2(attr, &optionCount, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT);
	}
	else
		addOption(attr, &optionCount, GLX_RGBA);
	addOption2(attr, &optionCount, GLX_RED_SIZE, options->redBits);
	addOption2(attr, &optionCount, GLX_GREEN_SIZE, options->greenBits);
	addOption2(attr, &optionCount, GLX_BLUE_SIZE, options->blueBits);
	addOption2(attr, &optionCount, GLX_ALPHA_SIZE, options->alphaBits);
	addOption2(attr, &optionCount, GLX_DEPTH_SIZE, options->depthBits);
	addOption2(attr, &optionCount, GLX_STENCIL_SIZE, options->stencilBits);
	if (!options->singleBuffer)
	{
		if (ANYGL_SUPPORTED(glXChooseFBConfig))
			addOption2(attr, &optionCount, GLX_DOUBLEBUFFER, true);
		else
			addOption(attr, &optionCount, GLX_DOUBLEBUFFER);
	}
	if (options->stereoscopic)
	{
		if (ANYGL_SUPPORTED(glXChooseFBConfig))
			addOption2(attr, &optionCount, GLX_STEREO, true);
		else
			addOption(attr, &optionCount, GLX_STEREO);
	}

	int major, minor;
	glXQueryVersion(display, &major, &minor);
	if (contextType == GLContextType_Render && ((major > 1 || (major == 1 && minor >= 4)) ||
		hasExtension(extensions, "GLX_ARB_multisample")))
	{
		if (options->surfaceSamples > 1)
		{
			addOption2(attr, &optionCount, GLX_SAMPLE_BUFFERS, 1);
			addOption2(attr, &optionCount, GLX_SAMPLES, options->surfaceSamples);
		}
		else
		{
			addOption2(attr, &optionCount, GLX_SAMPLE_BUFFERS, 0);
			addOption2(attr, &optionCount, GLX_SAMPLES, 0);
		}
	}

	if (options->srgb && hasExtension(extensions, "GLX_EXT_framebuffer_sRGB"))
		addOption2(attr, &optionCount, GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, true);

	addOption(attr, &optionCount, None);

	XVisualInfo* visualInfo = NULL;
	GLXFBConfig fbConfig = NULL;
	if (ANYGL_SUPPORTED(glXChooseFBConfig))
	{
		int configCount = 0;
		GLXFBConfig* configs = glXChooseFBConfig(display, screen, attr, &configCount);
		if (configs && configCount > 0)
		{
			fbConfig = configs[0];
			visualInfo = glXGetVisualFromFBConfig(display, fbConfig);
			XFree(configs);
		}
	}
	else
		visualInfo = glXChooseVisual(display, screen, attr);

	if (!visualInfo)
	{
		errno = EPERM;
		return NULL;
	}

	Config* config = DS_ALLOCATE_OBJECT(allocator, Config);
	if (!config)
	{
		XFree(visualInfo);
		return NULL;
	}

	config->allocator = dsAllocator_keepPointer(allocator);
	config->visualInfo = visualInfo;
	config->config = fbConfig;
	config->debug = options->debug;
	config->major = 1;
	config->minor = 0;

	if (ANYGL_SUPPORTED(glXCreateContextAttribsARB))
	{
		GLint contextAttr[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 0,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			None
		};

		// Set an empty error handler since the implementation may throw an error for unsupported
		// GL versions. Sync first to ensure any existing X11 calls have gone through.
		XSync(display, false);
		X11ErrorHandler prevHandler = XSetErrorHandler(&emptyErrorHandler);
		unsigned int versionCount = DS_ARRAY_SIZE(glVersions);
		for (unsigned int i = 0; i < versionCount; ++i)
		{
			contextAttr[1] = glVersions[i][0];
			contextAttr[3] = glVersions[i][1];
			gX11Error = false;
			GLXContext context = glXCreateContextAttribsARB(display, fbConfig, NULL, true,
				contextAttr);
			XSync(display, false);
			if (!gX11Error && context)
			{
				config->major = glVersions[i][0];
				config->minor = glVersions[i][1];
				glXDestroyContext(display, context);
				break;
			}

			if (context)
				glXDestroyContext(display, context);
		}
		// One more sync to make sure the destroy is processed before clearing out the handler.
		XSync(display, false);
		XSetErrorHandler(prevHandler);
	}

	return config;
}

void* dsGetPublicGLXConfig(void* display, void* config)
{
	DS_UNUSED(display);
	Config* configPtr = (Config*)config;
	if (!configPtr)
		return NULL;

	return (void*)configPtr->visualInfo->visualid;
}

void dsDestroyGLXConfig(void* display, void* config)
{
	DS_UNUSED(display);
	Config* configPtr = (Config*)config;
	if (!configPtr)
		return;

	XFree(configPtr->visualInfo);
	if (configPtr->allocator)
		dsAllocator_free(configPtr->allocator, configPtr);
}

void* dsCreateGLXContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr)
		return NULL;

	if (ANYGL_SUPPORTED(glXCreateContextAttribsARB))
	{
		GLint flags = configPtr->debug ? GLX_CONTEXT_DEBUG_BIT_ARB : 0;
		GLint attr[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, configPtr->major,
			GLX_CONTEXT_MINOR_VERSION_ARB, configPtr->minor,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			GLX_CONTEXT_FLAGS_ARB, flags,
			None
		};

		return glXCreateContextAttribsARB(display, configPtr->config, shareContext, true, attr);
	}
	else
		return glXCreateContext(display, configPtr->visualInfo, shareContext, true);
}

void dsDestroyGLXContext(void* display, void* context)
{
	if (!context)
		return;

	glXDestroyContext(display, context);
}

void* dsCreateDummyGLXSurface(dsAllocator* allocator, void* display, void* config, void** osSurface)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !osSurface)
		return NULL;

	Window root = DefaultRootWindow(display);
	XSetWindowAttributes attr;
	attr.colormap = XCreateColormap(display, root, configPtr->visualInfo->visual, AllocNone);
	Window window = XCreateWindow(display, root, 0, 0, 1, 1, 0, configPtr->visualInfo->depth,
		InputOutput, configPtr->visualInfo->visual, CWColormap, &attr);
	XFreeColormap(display, attr.colormap);

	if (configPtr->config)
	{
		DS_ASSERT(ANYGL_SUPPORTED(glXCreateWindow));
		GLXWindow glxWindow = glXCreateWindow(display, configPtr->config, window, NULL);
		if (!glxWindow)
		{
			XDestroyWindow(display, window);
			return NULL;
		}

		*osSurface = (void*)window;
		return (void*)glxWindow;
	}
	else
	{
		*osSurface = NULL;
		return (void*)window;
	}
}

void dsDestroyDummyGLXSurface(void* display, void* surface, void* osSurface)
{
	if (!surface)
		return;

	if (osSurface)
	{
		DS_ASSERT(ANYGL_SUPPORTED(glXDestroyWindow));
		glXDestroyWindow(display, (GLXWindow)surface);
		XDestroyWindow(display, (Window)osSurface);
	}
	else
		XDestroyWindow(display, (Window)surface);
}

void* dsCreateGLXSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle)
{
	DS_UNUSED(allocator);
	Config* configPtr = (Config*)config;
	if (!display || !configPtr || !handle)
		return NULL;

	GLXDrawable drawable;
	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
			if (configPtr->config)
			{
				DS_ASSERT(ANYGL_SUPPORTED(glXCreateWindow));
				drawable = glXCreateWindow(display, configPtr->config, (Window)handle, NULL);
				break;
			}
			drawable = (GLXDrawable)handle;
			break;
		case dsRenderSurfaceType_Pixmap:
			if (configPtr->config)
			{
				DS_ASSERT(ANYGL_SUPPORTED(glXCreatePixmap));
				drawable = glXCreatePixmap(display, configPtr->config, (Pixmap)handle, NULL);
				break;
			}
			drawable = (GLXDrawable)handle;
			break;
		default:
			drawable = (GLXDrawable)handle;
			break;
	}

	return (void*)drawable;
}

bool dsGetGLXSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(surfaceType);
	if (!outWidth || !outHeight || !surface)
		return false;

	glXQueryDrawable(display, (GLXDrawable)surface, GLX_WIDTH, outWidth);
	glXQueryDrawable(display, (GLXDrawable)surface, GLX_HEIGHT, outHeight);
	return true;
}

void dsSwapGLXBuffers(void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync)
{
	// Set the swap group when multiple vsynced surfaces are swapped to avoid waiting for multiple
	// vsyncs.
	bool setSwapGroup = vsync && count > 1 && ANYGL_SUPPORTED(glXJoinSwapGroupNV);
	if (setSwapGroup)
	{
		for (size_t i = 0; i < count; ++i)
		{
			GLXDrawable drawable =
				(GLXDrawable)((dsGLRenderSurface*)renderSurfaces[i])->glSurface;
			glXJoinSwapGroupNV(display, drawable, 1);
		}
	}

	for (size_t i = 0; i < count; ++i)
	{
		GLXDrawable drawable =
			(GLXDrawable)((dsGLRenderSurface*)renderSurfaces[i])->glSurface;
		glXSwapBuffers(display, drawable);
	}

	if (setSwapGroup)
	{
		for (size_t i = 0; i < count; ++i)
		{
			GLXDrawable drawable =
				(GLXDrawable)((dsGLRenderSurface*)renderSurfaces[i])->glSurface;
			glXJoinSwapGroupNV(display, drawable, 0);
		}
	}
}

void dsDestroyGLXSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	if (!surface)
		return;

	switch (surfaceType)
	{
		case dsRenderSurfaceType_Window:
			if (ANYGL_SUPPORTED(glXDestroyWindow))
				glXDestroyWindow(display, (GLXWindow)surface);
			break;
		case dsRenderSurfaceType_Pixmap:
			if (ANYGL_SUPPORTED(glXDestroyPixmap))
				glXDestroyPixmap(display, (GLXPixmap)surface);
			break;
		default:
			break;
	}
}

bool dsBindGLXContext(void* display, void* context, void* surface)
{
	DS_PROFILE_FUNC_START();
	GLXDrawable drawable = (GLXDrawable)surface;
	if (!glXMakeCurrent(display, drawable, context))
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't bind GL context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

void* dsGetCurrentGLXContext(void* display)
{
	DS_UNUSED(display);
	return glXGetCurrentContext();
}

void dsSetGLXVSync(void* display, void* surface, bool vsync)
{
	if (ANYGL_SUPPORTED(glXSwapIntervalEXT))
		glXSwapIntervalEXT(display, (GLXDrawable)surface, vsync);
}

#endif // ANYGL_HAS_GLX
