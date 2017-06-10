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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

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
