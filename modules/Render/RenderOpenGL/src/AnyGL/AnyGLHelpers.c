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

#include "AnyGL.h"
#include "gl.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#if ANYGL_HAS_FPTR
int AnyGL_FPTR_initialize(void);
int AnyGL_FPTR_load(void);
void AnyGL_FPTR_shutdown(void);
#endif

#if ANYGL_HAS_EGL
int AnyGL_EGL_initialize(void);
int AnyGL_EGL_load(void);
void AnyGL_EGL_shutdown(void);
#endif

#if ANYGL_HAS_WGL
int AnyGL_WGL_initialize(void);
int AnyGL_WGL_load(void);
void AnyGL_WGL_shutdown(void);
#endif

#if ANYGL_HAS_GLX
int AnyGL_GLX_initialize(void);
int AnyGL_GLX_load(void);
void AnyGL_GLX_shutdown(void);
#endif

static int majorVersion;
static int minorVersion;
static int (*loadFunc)(void);
static void (*shutdownFunc)(void);

int AnyGL_initialize(int loadLib)
{
	switch (loadLib)
	{
#if ANYGL_HAS_FPTR
		case ANYGL_LOAD_FPTR:
			if (AnyGL_FPTR_initialize())
			{
				loadFunc = &AnyGL_FPTR_load;
				shutdownFunc = &AnyGL_FPTR_shutdown;
				return 1;
			}
			return 0;
#endif
#if ANYGL_HAS_EGL
		case ANYGL_LOAD_EGL:
			if (AnyGL_EGL_initialize())
			{
				loadFunc = &AnyGL_EGL_load;
				shutdownFunc = &AnyGL_EGL_shutdown;
				return 1;
			}
			return 0;
#endif
#if ANYGL_HAS_WGL
		case ANYGL_LOAD_WGL:
			if (AnyGL_WGL_initialize())
			{
				loadFunc = &AnyGL_WGL_load;
				shutdownFunc = &AnyGL_WGL_shutdown;
				return 1;
			}
			return 0;
#endif
#if ANYGL_HAS_GLX
		case ANYGL_LOAD_GLX:
			if (AnyGL_GLX_initialize())
			{
				loadFunc = &AnyGL_GLX_load;
				shutdownFunc = &AnyGL_GLX_shutdown;
				return 1;
			}
			return 0;
#endif
	}
	return 0;
}

int AnyGL_load(void)
{
	return loadFunc && loadFunc();
}

void AnyGL_getGLVersion(int* major, int* minor, int* es)
{
	if (major)
		*major = majorVersion;
	if (minor)
		*minor = minorVersion;
	if (es)
		*es = ANYGL_GLES;
}

int AnyGL_atLeastVersion(int major, int minor, int es)
{
	if ((es != 0) != (ANYGL_GLES != 0))
		return 0;

	if (majorVersion > major || (majorVersion == major && minorVersion >= minor))
		return 1;

	return 0;
}

void AnyGL_shutdown(void)
{
	if (shutdownFunc)
		shutdownFunc();
	loadFunc = NULL;
	shutdownFunc = NULL;
}

int AnyGL_updateGLVersion(void)
{
	if (!ANYGL_SUPPORTED(glGetIntegerv))
		return 0;

	majorVersion = 0;
	minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	if (majorVersion == 0)
	{
		// This may have caused a GL error for older versions of OpenGL.
		glGetError();
		glGetError();
		const char* version = (const char*)glGetString(GL_VERSION);
		if (!version)
			return 0;
		if (sscanf(version, "%u.%u", &majorVersion, &minorVersion) != 2)
			return 0;
	}
	return 1;
}

int AnyGL_queryExtension(const char* name)
{
	if (ANYGL_SUPPORTED(glGetStringi))
	{
		GLint count = 0, i;
		glGetIntegerv(GL_NUM_EXTENSIONS, &count);
		for (i = 0; i < count; ++i)
		{
			if (strcmp((const char*)glGetStringi(GL_EXTENSIONS, i), name) == 0)
				return 1;
		}

		return 0;
	}
	else if (ANYGL_SUPPORTED(glGetString))
	{
		const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
		size_t begin = 0, end = 0;
		if (!extensions)
			return 0;

		while (extensions[begin])
		{
			for (end = begin; extensions[end] && extensions[end] != ' '; ++end)
				/* empty */;
			if (begin != end && strncmp(extensions + begin, name, end - begin) == 0)
				return 1;

			begin = extensions[end] == ' ' ? end + 1 : end;
		}

		return 0;
	}
	else
		return 0;
}

const char* AnyGL_errorString(unsigned int error)
{
	switch (error)
	{
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_FRAMEBUFFER_COMPLETE:
			return "GL_FRAMEBUFFER_COMPLETE";
		case GL_FRAMEBUFFER_UNDEFINED:
			return "GL_FRAMEBUFFER_UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:
			return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
		default:
			return "UNKNOWN";
	}
}
