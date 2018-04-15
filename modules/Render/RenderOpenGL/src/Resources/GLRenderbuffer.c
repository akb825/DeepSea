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

#include "Resources/GLRenderbuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLResource.h"
#include "GLHelpers.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsRenderbuffer* dsGLRenderbuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxFormat format, uint32_t width, uint32_t height, uint32_t samples)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLRenderbuffer* renderbuffer = DS_ALLOCATE_OBJECT(allocator, dsGLRenderbuffer);
	if (!renderbuffer)
		return NULL;

	dsRenderbuffer* baseRenderbuffer = (dsRenderbuffer*)renderbuffer;
	baseRenderbuffer->resourceManager = resourceManager;
	baseRenderbuffer->allocator = dsAllocator_keepPointer(allocator);
	baseRenderbuffer->format = format;
	baseRenderbuffer->width = width;
	baseRenderbuffer->height = height;
	baseRenderbuffer->samples = samples;

	renderbuffer->renderbufferId = 0;
	dsGLResource_initialize(&renderbuffer->resource);

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	glGenRenderbuffers(1, &renderbuffer->renderbufferId);
	if (!renderbuffer->renderbufferId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating renderbuffer: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	GLenum internalFormat;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, NULL, NULL, resourceManager,
		format));
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer->renderbufferId);
	if (samples > 1 && ANYGL_SUPPORTED(glRenderbufferStorageMultisample))
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating renderbuffer: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsClearGLErrors();
		dsGLRenderbuffer_destroy(resourceManager, baseRenderbuffer);
		return NULL;
	}

	return baseRenderbuffer;
}

static bool destroyImpl(dsRenderbuffer* renderbuffer)
{
	dsGLRenderbuffer* glRenderbuffer = (dsGLRenderbuffer*)renderbuffer;
	if (glRenderbuffer->renderbufferId)
		glDeleteRenderbuffers(1, &glRenderbuffer->renderbufferId);
	if (renderbuffer->allocator)
		return dsAllocator_free(renderbuffer->allocator, renderbuffer);

	return true;
}

bool dsGLRenderbuffer_destroy(dsResourceManager* resourceManager, dsRenderbuffer* renderbuffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(renderbuffer);

	dsGLRenderbuffer* glRenderbuffer = (dsGLRenderbuffer*)renderbuffer;
	if (dsGLResource_destroy(&glRenderbuffer->resource))
		return destroyImpl(renderbuffer);

	return true;
}

void dsGLRenderbuffer_addInternalRef(dsRenderbuffer* renderbuffer)
{
	DS_ASSERT(renderbuffer);
	dsGLRenderbuffer* glRenderbuffer = (dsGLRenderbuffer*)renderbuffer;
	dsGLResource_addRef(&glRenderbuffer->resource);
}

void dsGLRenderbuffer_freeInternalRef(dsRenderbuffer* renderbuffer)
{
	DS_ASSERT(renderbuffer);
	dsGLRenderbuffer* glRenderbuffer = (dsGLRenderbuffer*)renderbuffer;
	if (dsGLResource_freeRef(&glRenderbuffer->resource))
		destroyImpl(renderbuffer);
}
