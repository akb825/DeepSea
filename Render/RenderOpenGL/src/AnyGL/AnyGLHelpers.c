#include "gl.h"
#include <string.h>

static int majorVersion;
static int minorVersion;

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

	if (major > majorVersion || (major == majorVersion && minor >= minorVersion))
		return 1;

	return 0;
}

int AnyGL_updateGLVersion(void)
{
	if (!ANYGL_SUPPORTED(glGetIntegerv))
		return 0;

	majorVersion = 0;
	minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	return majorVersion > 0;
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
				/* empty */
			if (strncmp(extensions + begin, name, end - begin) == 0)
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
