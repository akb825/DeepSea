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

#include "GLMainCommandBuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLGfxFence.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLTexture.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

struct dsGLMainCommandBuffer
{
	dsGLCommandBuffer commandBuffer;

	dsGLFenceSyncRef** fenceSyncs;
	size_t curFenceSyncs;
	size_t maxFenceSyncs;
	bool bufferReadback;

	bool insideRenderPass;
};

static bool setFences(dsRenderer* renderer, dsGLFenceSyncRef** fenceSyncs, size_t fenceCount,
	bool bufferReadback)
{
	if (ANYGL_SUPPORTED(glMemoryBarrier) && bufferReadback)
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

	GLsync glSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	if (!glSync)
	{
		GLenum lastError = dsGetLastGLError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error setting fence: %s",
			AnyGL_errorString(lastError));
		errno = dsGetGLErrno(lastError);
	}
	glFlush();

	dsGLFenceSync* sync = dsGLRenderer_createSync(renderer, glSync);
	if (!sync)
	{
		glDeleteSync(glSync);
		return false;
	}

	for (size_t i = 0; i < fenceCount; ++i)
	{
		dsGLFenceSync_addRef(sync);
		DS_ASSERT(!fenceSyncs[i]->sync);
		DS_ATOMIC_STORE_PTR(&fenceSyncs[i]->sync, &sync);
	}

	dsGLFenceSync_freeRef(sync);
	return true;
}

bool dsGLMainCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->bufferId);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool dsGLMainCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glSrcBuffer = (dsGLGfxBuffer*)srcBuffer;
	dsGLGfxBuffer* glDstBuffer = (dsGLGfxBuffer*)dstBuffer;
	glCopyBufferSubData(glSrcBuffer->bufferId, glDstBuffer->bufferId, srcOffset, dstOffset, size);
	return true;
}

bool dsGLMainCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glTexture = (dsGLTexture*)texture;
	GLenum target = dsGLTexture_target(texture);

	bool compressed = dsGfxFormat_compressedIndex(texture->format) > 0;
	GLenum internalFormat;
	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, &glFormat, &type,
		texture->resourceManager, texture->format));

	glBindTexture(target, glTexture->textureId);
	switch (texture->dimension)
	{
		case dsTextureDim_1D:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x,
						width, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x, width, glFormat,
						type, data);
				}
			}
			break;
		case dsTextureDim_2D:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, internalFormat,
						(GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x,
						position->y, width, height, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x, position->y,
						width, height, glFormat, type, data);
				}
			}
			break;
		case dsTextureDim_3D:
			if (compressed)
			{
				glCompressedTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x,
					position->y, position->depth, width, height, layers, internalFormat,
					(GLsizei)size, data);
			}
			else
			{
				glTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x, position->y,
					position->depth, width, height, layers, glFormat, type, data);
			}
			break;
		case dsTextureDim_Cube:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel,
						position->x, position->y, position->depth*6 + position->face, width, height,
						layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel, position->x,
						position->y, position->depth*6 + position->face, width, height, layers,
						glFormat, type, data);
				}
			}
			else
			{
				for (unsigned int j = 0; j < layers; ++j)
				{
					if (compressed)
					{
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height,
							internalFormat, (GLsizei)size, data);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height, glFormat,
							type, data);
					}
				}
			}
			break;
		default:
			DS_ASSERT(false);
	}
	glBindTexture(target, 0);

	return true;
}

bool dsGLMainCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glSrcTexture = (dsGLTexture*)srcTexture;
	dsGLTexture* glDstTexture = (dsGLTexture*)dstTexture;
	if (ANYGL_SUPPORTED(glCopyImageSubData))
	{
		GLenum srcTarget = dsGLTexture_copyTarget(srcTexture);
		GLenum dstTarget = dsGLTexture_copyTarget(dstTexture);

		for (size_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			glCopyImageSubData(glSrcTexture->textureId, srcTarget, regions[i].srcPosition.mipLevel,
				regions[i].srcPosition.x, regions[i].srcPosition.y, srcLayer,
				glDstTexture->textureId, dstTarget, regions[i].dstPosition.mipLevel,
				regions[i].dstPosition.x, regions[i].dstPosition.y, dstLayer, regions[i].width,
				regions[i].height, regions[i].layers);
		}
	}
	else
	{
		dsRenderer* renderer = commandBuffer->renderer;
		GLuint tempFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
		GLuint tempCopyFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
		if (!tempFramebuffer || !tempCopyFramebuffer)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Texture blitting may only be done during rendering.");
			return false;
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempCopyFramebuffer);

		GLbitfield buffers = dsGLTexture_buffers(srcTexture);
		for (uint32_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			for (uint32_t j = 0; j < regions[i].layers; ++j)
			{
				dsGLBindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture,
					regions[i].srcPosition.mipLevel, srcLayer + j);
				dsGLBindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture,
					regions[i].dstPosition.mipLevel, dstLayer + j);
				glBlitFramebuffer(regions[i].srcPosition.x, regions[i].srcPosition.y,
					regions[i].srcPosition.x + regions[i].width,
					regions[i].srcPosition.y + regions[i].height,
					regions[i].dstPosition.x, regions[i].dstPosition.y,
					regions[i].dstPosition.x + regions[i].width,
					regions[i].dstPosition.y + regions[i].height, buffers, GL_NEAREST);
			}
		}

		dsGLUnbindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture);
		dsGLUnbindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	return true;
}

bool dsGLMainCommandBuffer_blitTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureBlitRegion* regions, size_t regionCount,
	dsBlitFilter filter)
{
	DS_UNUSED(commandBuffer);

	dsRenderer* renderer = commandBuffer->renderer;
	GLuint tempFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
	GLuint tempCopyFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
	if (!tempFramebuffer || !tempCopyFramebuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Texture blitting may only be done during rendering.");
		return false;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFramebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempCopyFramebuffer);

	GLbitfield buffers = dsGLTexture_buffers(srcTexture);
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		uint32_t srcLayer = regions[i].srcPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			srcLayer = srcLayer*6 + regions[i].dstPosition.face;
		uint32_t dstLayer = regions[i].dstPosition.depth;
		if (dstTexture->dimension == dsTextureDim_Cube)
			dstLayer = dstLayer*6 + regions[i].dstPosition.face;

		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			dsGLBindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture,
				regions[i].srcPosition.mipLevel, srcLayer + j);
			dsGLBindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture,
				regions[i].dstPosition.mipLevel, dstLayer + j);
			glBlitFramebuffer(regions[i].srcPosition.x, regions[i].srcPosition.y,
				regions[i].srcPosition.x + regions[i].srcWidth,
				regions[i].srcPosition.y + regions[i].srcHeight,
				regions[i].dstPosition.x, regions[i].dstPosition.y,
				regions[i].dstPosition.x + regions[i].dstWidth,
				regions[i].dstPosition.y + regions[i].dstHeight, buffers,
				filter == dsBlitFilter_Linear ? GL_LINEAR : GL_NEAREST);
		}
	}

	dsGLUnbindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture);
	dsGLUnbindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

bool dsGLMainCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	size_t syncCount, bool bufferReadback)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (glCommandBuffer->insideRenderPass)
	{
		size_t index = glCommandBuffer->curFenceSyncs;
		if (!dsGLAddToBuffer(commandBuffer->allocator, (void**)&glCommandBuffer->fenceSyncs,
			&glCommandBuffer->curFenceSyncs, &glCommandBuffer->maxFenceSyncs,
			sizeof(dsGLFenceSyncRef*), syncCount))
		{
			return false;
		}

		DS_ASSERT(index + syncCount <= glCommandBuffer->maxFenceSyncs);
		for (size_t i = 0; i < syncCount; ++i)
		{
			glCommandBuffer->fenceSyncs[index + i] = syncs[i];
			dsGLFenceSyncRef_addRef(syncs[i]);
		}
		glCommandBuffer->curFenceSyncs += syncCount;

		if (bufferReadback)
			glCommandBuffer->bufferReadback = bufferReadback;

		return true;
	}
	else
		return setFences(commandBuffer->renderer, syncs, syncCount, bufferReadback);
}

bool dsGLMainCommandBuffer_submit(dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(submitBuffer);
	return false;
}

static CommandBufferFunctionTable functionTable =
{
	&dsGLMainCommandBuffer_copyBufferData,
	&dsGLMainCommandBuffer_copyBuffer,
	&dsGLMainCommandBuffer_copyTextureData,
	&dsGLMainCommandBuffer_copyTexture,
	&dsGLMainCommandBuffer_blitTexture,
	&dsGLMainCommandBuffer_setFenceSyncs,
	&dsGLMainCommandBuffer_submit
};

dsGLMainCommandBuffer* dsGLMainCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator)
{
	DS_ASSERT(allocator->freeFunc);
	dsGLMainCommandBuffer* commandBuffer = (dsGLMainCommandBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsGLMainCommandBuffer));
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = dsCommandBufferUsage_Standard;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;
	commandBuffer->fenceSyncs = NULL;
	commandBuffer->curFenceSyncs = 0;
	commandBuffer->maxFenceSyncs = 0;
	commandBuffer->bufferReadback = false;
	commandBuffer->insideRenderPass = false;

	return commandBuffer;
}

bool dsGLMainCommandBuffer_destroy(dsGLMainCommandBuffer* commandBuffer)
{
	if (!commandBuffer)
		return true;

	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;

	if (commandBuffer->fenceSyncs)
	{
		for (size_t i = 0; i < commandBuffer->curFenceSyncs; ++i)
			dsGLFenceSyncRef_freeRef(commandBuffer->fenceSyncs[i]);
		DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fenceSyncs));
	}

	DS_VERIFY(dsAllocator_free(allocator, commandBuffer));
	return true;
}
