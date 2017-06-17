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

void dsGLBindFramebufferTexture(GLenum framebuffer, dsTexture* texture, uint32_t mipLevel,
	uint32_t layer)
{
	dsGLTexture* glTexture = (dsGLTexture*)texture;
	GLenum target = dsGLTexture_target(texture);
	GLenum attachment = dsGLTexture_attachment(texture);
	switch (texture->dimension)
	{
		case dsTextureDim_1D:
			if (texture->depth > 0)
			{
				glFramebufferTextureLayer(framebuffer, attachment, glTexture->textureId, mipLevel,
					layer);
			}
			else
			{
				glFramebufferTexture1D(framebuffer, attachment, target, glTexture->textureId,
					mipLevel);
			}
			break;
		case dsTextureDim_2D:
			if (texture->depth > 0)
			{
				glFramebufferTextureLayer(framebuffer, attachment, glTexture->textureId, mipLevel,
					layer);
			}
			else
			{
				glFramebufferTexture2D(framebuffer, attachment, target, glTexture->textureId,
					mipLevel);
			}
			break;
		case dsTextureDim_3D:
			glFramebufferTexture3D(framebuffer, attachment, target, glTexture->textureId, mipLevel,
				layer);
			break;
		case dsTextureDim_Cube:
			if (texture->depth > 0)
			{
				glFramebufferTextureLayer(framebuffer, attachment, glTexture->textureId, mipLevel,
					layer);
			}
			else
			{
				glFramebufferTexture2D(framebuffer, attachment,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, glTexture->textureId, mipLevel);
			}
			break;
		default:
			DS_ASSERT(false);
	}
}

void dsGLUnbindFramebufferTexture(GLenum framebuffer, dsTexture* texture)
{
	GLenum attachment = dsGLTexture_attachment(texture);
	glFramebufferTexture2D(framebuffer, attachment, GL_TEXTURE_2D, 0, 0);
}

bool dsGLAddToBuffer(dsAllocator* allocator, void** buffer, size_t* curElems, size_t* maxElems,
	size_t elemSize, size_t addElems)
{
	DS_ASSERT(allocator);
	DS_ASSERT(buffer);
	DS_ASSERT(curElems);
	DS_ASSERT(maxElems);
	DS_ASSERT(*buffer || *curElems == 0);

	if (*curElems + addElems <= *maxElems)
	{
		*curElems += addElems;
		return true;
	}

	size_t newMaxElems = dsMax(16U, addElems);
	newMaxElems = dsMax(newMaxElems, *maxElems*2);
	DS_ASSERT(newMaxElems >= *curElems + addElems);
	void* newBuffer = dsAllocator_alloc(allocator, newMaxElems*elemSize);
	if (!newBuffer)
		return false;

	if (*buffer)
	{
		memcpy(newBuffer, *buffer, *curElems*elemSize);
		DS_VERIFY(dsAllocator_free(allocator, buffer));
	}
	*curElems += addElems;
	*maxElems = newMaxElems;
	*buffer = newBuffer;
	return true;
}
