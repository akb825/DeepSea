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

#include "Resources/GLFramebuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLRenderbuffer.h"
#include "Resources/GLResource.h"
#include "Resources/GLTexture.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool bindFramebufferSurface(GLenum attachment, const dsFramebufferSurface* surface,
	uint32_t layers, GLuint* curAttachment)
{
	if (!surface)
	{
		if (*curAttachment == 0)
			return false;

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0);
		*curAttachment = 0;
		return true;
	}

	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_Texture:
		{
			dsTexture* texture = (dsTexture*)surface->surface;
			dsGLTexture* glTexture = (dsGLTexture*)texture;
			if (glTexture->drawBufferId)
			{
				if (*curAttachment == glTexture->drawBufferId)
					return false;

				*curAttachment = glTexture->drawBufferId;
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
					GL_RENDERBUFFER, glTexture->drawBufferId);
			}
			else
			{
				if (*curAttachment == glTexture->textureId)
					return false;

				*curAttachment = glTexture->textureId;
				if (layers > 1)
				{
					DS_ASSERT(!glTexture->drawBufferId);
					glFramebufferTexture(GL_FRAMEBUFFER, attachment, glTexture->textureId,
						surface->mipLevel);
				}
				else
				{
					uint32_t layer = surface->layer;
					if (texture->info.dimension == dsTextureDim_Cube)
						layer = layer*6 + surface->cubeFace;
					dsGLTexture_bindFramebufferAttachment(texture, GL_FRAMEBUFFER, attachment,
						surface->mipLevel, layer);
				}
			}
			return true;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsGLRenderbuffer* renderbuffer = (dsGLRenderbuffer*)surface->surface;
			if (*curAttachment == renderbuffer->renderbufferId)
				return false;

			*curAttachment = renderbuffer->renderbufferId;
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER,
				renderbuffer->renderbufferId);
			return true;
		}
		default:
			DS_ASSERT(false);
			return false;
	}
}

dsFramebuffer* dsGLFramebuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsFramebufferSurface* surfaces, uint32_t surfaceCount,
	uint32_t width, uint32_t height, uint32_t layers)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(surfaces || surfaceCount == 0);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*surfaceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, fullSize));
	dsGLFramebuffer* framebuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
		dsGLFramebuffer);
	DS_ASSERT(framebuffer);

	dsFramebuffer* baseFramebuffer = (dsFramebuffer*)framebuffer;
	baseFramebuffer->resourceManager = resourceManager;
	baseFramebuffer->allocator = dsAllocator_keepPointer(allocator);
	if (surfaceCount > 0)
	{
		baseFramebuffer->surfaces = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator,
			dsFramebufferSurface, surfaceCount);
		DS_ASSERT(baseFramebuffer->surfaces);
		memcpy(baseFramebuffer->surfaces, surfaces, sizeof(dsFramebufferSurface)*surfaceCount);
	}
	else
		baseFramebuffer->surfaces = NULL;
	baseFramebuffer->surfaceCount = surfaceCount;
	baseFramebuffer->width = width;
	baseFramebuffer->height = height;
	baseFramebuffer->layers = layers;

	dsGLResource_initialize(&framebuffer->resource);
	framebuffer->framebufferId = 0;
	framebuffer->fboContext = 0;
	memset(framebuffer->curColorAttachments, 0, sizeof(framebuffer->curColorAttachmentCount));
	framebuffer->curColorAttachmentCount = 0;
	framebuffer->curDepthAttachment = DS_NO_ATTACHMENT;
	framebuffer->curDefaultSamples = 0;
	framebuffer->framebufferError = false;
	framebuffer->defaultFramebuffer = true;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		switch (surfaces[i].surfaceType)
		{
			case dsGfxSurfaceType_Texture:
			case dsGfxSurfaceType_Renderbuffer:
				framebuffer->defaultFramebuffer = false;
				break;
			default:
				break;
		}
	}

	return baseFramebuffer;
}

static bool destroyImpl(dsFramebuffer* framebuffer)
{
	dsGLFramebuffer* glFramebuffer = (dsGLFramebuffer*)framebuffer;
	dsGLRenderer_destroyFbo(framebuffer->resourceManager->renderer, glFramebuffer->framebufferId,
		glFramebuffer->fboContext);

	if (framebuffer->allocator)
		return dsAllocator_free(framebuffer->allocator, framebuffer);

	return true;
}

bool dsGLFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(framebuffer);

	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	if (dsGLResource_destroy(&glBuffer->resource))
		return destroyImpl(framebuffer);

	return true;
}

GLSurfaceType dsGLFramebuffer_getSurfaceType(dsGfxSurfaceType framebufferSurfaceType)
{
	switch (framebufferSurfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			return GLSurfaceType_Left;
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			return GLSurfaceType_Right;
		case dsGfxSurfaceType_Texture:
		case dsGfxSurfaceType_Renderbuffer:
			return GLSurfaceType_Framebuffer;
		default:
			DS_ASSERT(false);
			return GLSurfaceType_Framebuffer;
	}
}

GLSurfaceType dsGLFramebuffer_bind(const dsFramebuffer* framebuffer,
	const dsColorAttachmentRef* colorAttachments, uint32_t colorAttachmentCount,
	uint32_t depthStencilAttachment)
{
	dsRenderer* renderer = framebuffer->resourceManager->renderer;
	dsGLFramebuffer* glFramebuffer = (dsGLFramebuffer*)framebuffer;
	if (!glFramebuffer->defaultFramebuffer)
	{
		// Framebuffer objects are tied to specific contexts.
		dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
		if (!glFramebuffer->framebufferId || glFramebuffer->fboContext != glRenderer->contextCount)
		{
			glGenFramebuffers(1, &glFramebuffer->framebufferId);
			memset(glFramebuffer->curColorAttachments, 0,
				sizeof(glFramebuffer->curColorAttachmentCount));
			glFramebuffer->curColorAttachmentCount = 0;
			glFramebuffer->curDepthAttachment = DS_NO_ATTACHMENT;
			glFramebuffer->curDefaultSamples = 0;
			glFramebuffer->framebufferError = false;

			if (!renderer->resourceManager->requiresAnySurface)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer->framebufferId);
				glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
					framebuffer->width);
				glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
					framebuffer->height);
				glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS,
					framebuffer->layers);
				dsGLRenderer_restoreFramebuffer(renderer);
			}
		}
	}

	DS_ASSERT(colorAttachmentCount > 0);
	GLSurfaceType surfaceType;
	if (colorAttachmentCount == 1 && colorAttachments[0].attachmentIndex != DS_NO_ATTACHMENT)
	{
		surfaceType = dsGLFramebuffer_getSurfaceType(
			framebuffer->surfaces[colorAttachments[0].attachmentIndex].surfaceType);
	}
	else
		surfaceType = GLSurfaceType_Framebuffer;

	dsGLRenderer_bindFramebuffer(renderer, surfaceType, glFramebuffer->framebufferId,
		GLFramebufferFlags_Default);

	if (surfaceType == GLSurfaceType_Framebuffer)
	{
		// Bind the surfaces to the framebuffer.
		bool hasChanges = false;
		DS_ASSERT(colorAttachmentCount < MSL_MAX_ATTACHMENTS);
		for (uint32_t i = 0; i < colorAttachmentCount; ++i)
		{
			const dsFramebufferSurface* surface = NULL;
			if (colorAttachments[i].attachmentIndex != DS_NO_ATTACHMENT)
				surface = framebuffer->surfaces + colorAttachments[i].attachmentIndex;
			if (bindFramebufferSurface(GL_COLOR_ATTACHMENT0 + i, surface, framebuffer->layers,
				glFramebuffer->curColorAttachments + i))
			{
				hasChanges = true;
			}
		}

		if ((colorAttachmentCount != glFramebuffer->curColorAttachmentCount || hasChanges) &&
			ANYGL_SUPPORTED(glDrawBuffers))
		{
			hasChanges = true;
			GLenum drawBuffers[MSL_MAX_ATTACHMENTS];
			for (uint32_t i = 0; i < colorAttachmentCount; ++i)
			{
				if (colorAttachments[i].attachmentIndex == DS_NO_ATTACHMENT)
					drawBuffers[i] = GL_NONE;
				else
					drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			glDrawBuffers(colorAttachmentCount, drawBuffers);

			// Remove the binding for any remaining previous attachments to avoid holding onto
			// resources.
			for (uint32_t i = colorAttachmentCount; i < glFramebuffer->curColorAttachmentCount; ++i)
			{
				if (glFramebuffer->curColorAttachments[i])
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
						0, 0);
					glFramebuffer->curColorAttachments[i] = 0;
				}
			}
			glFramebuffer->curColorAttachmentCount = colorAttachmentCount;
		}

		if (depthStencilAttachment == DS_NO_ATTACHMENT)
		{
			if (glFramebuffer->curDepthAttachment != depthStencilAttachment)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
					0, 0);
				glFramebuffer->curDepthAttachment = depthStencilAttachment;
				hasChanges = true;
			}
		}
		else
		{
			const dsFramebufferSurface* surface = framebuffer->surfaces + depthStencilAttachment;
			dsGfxFormat format;
			if (surface->surfaceType == dsGfxSurfaceType_Texture)
				format = ((dsTexture*)surface->surface)->info.format;
			else
				format = ((dsRenderbuffer*)surface->surface)->format;
			if (bindFramebufferSurface(dsGLTexture_attachment(format), surface, framebuffer->layers,
				&glFramebuffer->curDepthAttachment))
			{
				hasChanges = true;
			}

			if (AnyGL_atLeastVersion(3, 0, false) || AnyGL_ARB_framebuffer_sRGB ||
				AnyGL_EXT_framebuffer_sRGB || AnyGL_EXT_sRGB_write_control)
			{
				if (format & dsGfxFormat_SRGB)
					glEnable(GL_FRAMEBUFFER_SRGB);
				else
					glDisable(GL_FRAMEBUFFER_SRGB);
			}
		}

		if (hasChanges)
		{
			// Check for completeness if we changed the binding.
			GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (result != GL_FRAMEBUFFER_COMPLETE)
			{
				glFramebuffer->framebufferError = true;
				DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Framebuffer error: %s",
					AnyGL_errorString(result));
				return GLSurfaceType_None;
			}

			glFramebuffer->framebufferError = false;
		}
		else if (glFramebuffer->framebufferError)
			return GLSurfaceType_None;
	}

	return surfaceType;
}

void dsGLFramebuffer_setDefaultSamples(const dsFramebuffer* framebuffer, uint32_t samples)
{
	dsRenderer* renderer = framebuffer->resourceManager->renderer;
	dsGLRenderer* glRenderer = (dsGLRenderer*)renderer;
	if (glRenderer->curSurfaceType != GLSurfaceType_Framebuffer ||
		framebuffer->resourceManager->requiresAnySurface)
	{
		return;
	}

	if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = renderer->surfaceSamples;
	dsGLFramebuffer* glFramebuffer = (dsGLFramebuffer*)framebuffer;
	if (glFramebuffer->curDefaultSamples != samples)
	{
		glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples);
		glFramebuffer->curDefaultSamples = samples;
	}
}

void dsGLFramebuffer_addInternalRef(dsFramebuffer* framebuffer)
{
	DS_ASSERT(framebuffer);
	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	dsGLResource_addRef(&glBuffer->resource);

	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (framebuffer->surfaces[i].surfaceType == dsGfxSurfaceType_Texture)
			dsGLTexture_addInternalRef((dsTexture*)framebuffer->surfaces[i].surface);
		else if (framebuffer->surfaces[i].surfaceType == dsGfxSurfaceType_Renderbuffer)
			dsGLRenderbuffer_addInternalRef((dsRenderbuffer*)framebuffer->surfaces[i].surface);
	}
}

void dsGLFramebuffer_freeInternalRef(dsFramebuffer* framebuffer)
{
	DS_ASSERT(framebuffer);
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (framebuffer->surfaces[i].surfaceType == dsGfxSurfaceType_Texture)
			dsGLTexture_freeInternalRef((dsTexture*)framebuffer->surfaces[i].surface);
		else if (framebuffer->surfaces[i].surfaceType == dsGfxSurfaceType_Renderbuffer)
			dsGLRenderbuffer_freeInternalRef((dsRenderbuffer*)framebuffer->surfaces[i].surface);
	}

	dsGLFramebuffer* glBuffer = (dsGLFramebuffer*)framebuffer;
	if (dsGLResource_freeRef(&glBuffer->resource))
		destroyImpl(framebuffer);
}
