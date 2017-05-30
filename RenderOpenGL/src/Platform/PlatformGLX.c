#include "Platform/Platform.h"
#include "AnyGL/AnyGL.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

#if ANYGL_LOAD == ANYGL_LOAD_GLX
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
			/* empty */
		if (strncmp(extensions + begin, extension, end - begin) == 0)
			return true;

		begin = extensions[end] == ' ' ? end + 1 : end;
	}

	return false;
}

void* dsGetGLDisplay(void)
{
	return XOpenDisplay(NULL);
}

void dsReleaseGLDisplay(void* display)
{
	XCloseDisplay(display);
}

void* dsCreateGLConfig(dsAllocator* allocator, void* display, const dsOpenGLOptions* options,
	bool render)
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
		addOption2(attr, &optionCount, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PBUFFER_BIT);
	}
	else
		addOption(attr, &optionCount, GLX_RGBA);
	addOption2(attr, &optionCount, GLX_RED_SIZE, options->redBits);
	addOption2(attr, &optionCount, GLX_GREEN_SIZE, options->greenBits);
	addOption2(attr, &optionCount, GLX_BLUE_SIZE, options->blueBits);
	addOption2(attr, &optionCount, GLX_ALPHA_SIZE, options->alphaBits);
	addOption2(attr, &optionCount, GLX_DEPTH_SIZE, options->depthBits);
	addOption2(attr, &optionCount, GLX_STENCIL_SIZE, options->stencilBits);
	if (options->doubleBuffer)
		addOption(attr, &optionCount, GLX_DOUBLEBUFFER);
	if (options->stereo)
		addOption(attr, &optionCount, GLX_STEREO);

	int samplesIdx = -1;
	int major, minor;
	glXQueryVersion(display, &major, &minor);
	if (render && options->samples > 1 && ((major > 1 || (major == 1 && minor >= 4)) ||
		hasExtension(extensions, "GLX_ARB_multisample")))
	{
		samplesIdx = optionCount;
		addOption2(attr, &optionCount, GLX_SAMPLE_BUFFERS, 1);
		addOption2(attr, &optionCount, GLX_SAMPLES, options->samples);
	}

	if (options->srgb && hasExtension(extensions, "GLX_EXT_framebuffer_sRGB"))
		addOption(attr, &optionCount, GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT);

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
		else if (samplesIdx >= 0)
		{
			// Try without multisampling.
			attr[samplesIdx + 1] = 0;
			attr[samplesIdx + 3] = 0;
			configs = glXChooseFBConfig(display, screen, attr, &configCount);
			if (configs && configCount > 0)
			{
				fbConfig = configs[0];
				visualInfo = glXGetVisualFromFBConfig(display, fbConfig);
				XFree(configs);
			}
		}
	}
	else
	{
		DS_ASSERT(samplesIdx < 0);
		visualInfo = glXChooseVisual(display, screen, attr);
	}

	if (!visualInfo)
	{
		errno = EPERM;
		return NULL;
	}

	Config* config = (Config*)dsAllocator_alloc(allocator, sizeof(Config));
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
		static GLint versions[][2] =
		{
			{4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
			{3, 3}, {3, 2}, {3, 1}, {3, 0}
		};

		GLint contextAttr[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 0,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			None
		};

		unsigned int versionCount = (unsigned int)DS_ARRAY_SIZE(versions);
		for (unsigned int i = 0; i < versionCount; ++i)
		{
			contextAttr[1] = versions[i][0];
			contextAttr[3] = versions[i][1];
			GLXContext context = glXCreateContextAttribsARB(display, fbConfig, NULL, true,
				contextAttr);
			if (context)
			{
				config->major = versions[i][0];
				config->minor = versions[i][1];
				glXDestroyContext(display, context);
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

	XFree(configPtr->visualInfo);
	if (configPtr->allocator)
		dsAllocator_free(configPtr->allocator, configPtr);
}

void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext)
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

void dsDestroyGLContext(void* display, void* context)
{
	if (!context)
		return;

	glXDestroyContext(display, context);
}

void* dsCreateDummyGLSurface(dsAllocator* allocator, void* display, void* config, void** osSurface)
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

void dsDestroyDummyGLSurface(void* display, void* surface, void* osSurface)
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
			if (configPtr->config)
			{
				DS_ASSERT(ANYGL_SUPPORTED(glXCreateWindow));
				return (void*)glXCreateWindow(display, configPtr->config, (GLXWindow)handle, NULL);
			}
			else
				return handle;
		default:
			return handle;
	}
}

void dsSwapGLBuffers(void* display, void* surface)
{
	if (!surface)
		return;

	glXSwapBuffers(display, (GLXDrawable)surface);
}

void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface)
{
	if (!surface)
		return;

	if (surfaceType == dsRenderSurfaceType_Window && ANYGL_SUPPORTED(glXDestroyWindow))
		glXDestroyWindow(display, (GLXWindow)surface);
}

bool dsBindGLContext(void* display, void* context, void* surface)
{
	if (!glXMakeCurrent(display, (GLXDrawable)surface, context))
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Couldn't bind GL context.");
		return false;
	}

	return true;
}

#endif // ANYGL_LOAD
