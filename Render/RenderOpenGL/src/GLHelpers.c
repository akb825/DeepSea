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
#include "Resources/GLTexture.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

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

GLenum dsGetLastGLError(void)
{
	GLenum curError = GL_NO_ERROR, lastError;
	do
	{
		lastError = curError;
		curError = glGetError();
	} while (curError != GL_NO_ERROR);
	return lastError;
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

GLenum dsGetGLMinFilter(mslFilter minFilter, mslMipFilter mipFilter)
{
	switch (minFilter)
	{
		case mslFilter_Unset:
		case mslFilter_Nearest:
			switch (mipFilter)
			{
				case mslMipFilter_Unset:
				case mslMipFilter_None:
					return GL_NEAREST;
				case mslMipFilter_Nearest:
					return GL_NEAREST_MIPMAP_NEAREST;
				case mslMipFilter_Linear:
				case mslMipFilter_Anisotropic:
					return GL_NEAREST_MIPMAP_LINEAR;
			}
			break;
		case mslFilter_Linear:
			switch (mipFilter)
			{
				case mslMipFilter_Unset:
				case mslMipFilter_None:
					return GL_LINEAR;
				case mslMipFilter_Nearest:
					return GL_LINEAR_MIPMAP_NEAREST;
				case mslMipFilter_Linear:
				case mslMipFilter_Anisotropic:
					return GL_LINEAR_MIPMAP_LINEAR;
			}
			break;
	}

	DS_ASSERT(false);
	return GL_NEAREST;
}

GLenum dsGetGLMagFilter(mslFilter magFilter)
{
	switch (magFilter)
	{
		case mslFilter_Unset:
		case mslFilter_Nearest:
			return GL_NEAREST;
		case mslFilter_Linear:
			return GL_LINEAR;
	}

	DS_ASSERT(false);
	return GL_NEAREST;
}

GLenum dsGetGLAddressMode(mslAddressMode addressMode)
{
	switch (addressMode)
	{
		case mslAddressMode_Unset:
		case mslAddressMode_Repeat:
			return GL_REPEAT;
		case mslAddressMode_MirroredRepeat:
			return GL_MIRRORED_REPEAT;
		case mslAddressMode_ClampToEdge:
			return GL_CLAMP_TO_EDGE;
		case mslAddressMode_ClampToBorder:
		{
			if (AnyGL_atLeastVersion(1, 0, false) || AnyGL_OES_texture_border_clamp)
				return GL_CLAMP_TO_BORDER;
			else
				return GL_CLAMP_TO_EDGE;
		}
		case mslAddressMode_MirrorOnce:
		{
			if (AnyGL_atLeastVersion(4, 3, false) || AnyGL_EXT_texture_mirror_clamp)
				return GL_MIRROR_CLAMP_TO_EDGE;
			else
				return GL_MIRRORED_REPEAT;
		}
	}

	DS_ASSERT(false);
	return GL_REPEAT;
}

GLenum dsGetGLCompareOp(mslCompareOp compareOp)
{
	static const GLenum compareOpMap[] =
	{
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_GEQUAL,
		GL_ALWAYS
	};

	if (compareOp == mslCompareOp_Unset)
		compareOp = mslCompareOp_Less;
	DS_ASSERT((unsigned int)compareOp < DS_ARRAY_SIZE(compareOpMap));
	return compareOpMap[compareOp];
}

GLenum dsGetGLBufferType(int usage)
{
	if (usage & dsGfxBufferUsage_UniformBuffer)
		return GL_SHADER_STORAGE_BUFFER;
	else if (usage & dsGfxBufferUsage_UniformBlock)
		return GL_UNIFORM_BUFFER;
	else
		return GL_ARRAY_BUFFER;
}
