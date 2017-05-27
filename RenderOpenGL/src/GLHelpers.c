/*
 * Copyright 2017 Aaron Barany
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

#include "GLHelpers.h"
#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Types.h"
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Error.h>

void dsCheckGLErrors(void)
{
#if ANYGL_ALLOW_DEBUG
	dsClearGLErrors();
#endif
}

void dsClearGLErrors(void)
{
	GLenum error;
	do
	{
		error = glGetError();
		if (error != GL_NO_ERROR)
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "OpenGL error: %s", AnyGL_errorString(error));
	} while (error != GL_NO_ERROR);
}

int dsGetGLErrno(GLenum error)
{
	switch (error)
	{
		case GL_NO_ERROR:
			return 0;
		case GL_INVALID_ENUM:
			return EINVAL;
		case GL_INVALID_VALUE:
			return EINVAL;
		case GL_INVALID_OPERATION:
			return EPERM;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return EPERM;
		case GL_OUT_OF_MEMORY:
			return ENOMEM;
		default:
			return EINVAL;
	}
}
