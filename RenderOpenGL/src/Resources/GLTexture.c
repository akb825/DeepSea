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

#include "Resources/GLTexture.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLResource.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

dsTexture* dsGLTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator, int usage,
	int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width, uint32_t height,
	uint32_t depth, uint32_t mipLevels, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLTexture* texture = (dsGLTexture*)dsAllocator_alloc(allocator, sizeof(dsGLTexture));
	if (!texture)
		return NULL;

	dsTexture* baseTexture = (dsTexture*)texture;
	baseTexture->resourceManager = resourceManager;
	baseTexture->allocator = dsAllocator_keepPointer(allocator);
	baseTexture->usage = (dsTextureUsage)usage;
	baseTexture->memoryHints = (dsGfxMemory)memoryHints;
	baseTexture->format = format;
	baseTexture->dimension = dimension;
	baseTexture->width = width;
	baseTexture->height = height;
	baseTexture->depth = depth;
	baseTexture->mipLevels = mipLevels;
	baseTexture->offscreen = false;
	baseTexture->resolve = 0;
	baseTexture->samples = 0;

	texture->textureId = 0;
	texture->drawBufferId = 0;
	dsGLResource_initialize(&texture->resource);

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	glGenTextures(1, &texture->textureId);
	if (!texture->textureId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating texture: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLTexture_destroy(resourceManager, baseTexture);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	GLenum target = dsGLTexture_target(baseTexture);
	glBindTexture(target, texture->textureId);

	// Format should have been validated earlier.
	GLenum internalFormat;
	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, &glFormat, &type,
		resourceManager, format));

	bool compressed = dsGfxFormat_compressedIndex(format) > 0;
	const uint8_t* dataBytes = (const uint8_t*)data;
	unsigned int faces = dimension == dsTextureDim_Cube ? 6 : 1;
	DS_ASSERT(mipLevels > 0);
	if (ANYGL_SUPPORTED(glTexStorage2D))
	{
		switch (dimension)
		{
			case dsTextureDim_1D:
				if (depth > 0)
					glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, internalFormat, width, depth);
				else
					glTexStorage1D(GL_TEXTURE_1D, mipLevels, internalFormat, width);
				break;
			case dsTextureDim_2D:
				if (depth > 0)
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, internalFormat, width, height,
						depth);
				}
				else
					glTexStorage2D(GL_TEXTURE_1D, mipLevels, internalFormat, width, height);
				break;
			case dsTextureDim_3D:
				glTexStorage3D(GL_TEXTURE_3D, mipLevels, internalFormat, width, height, depth);
				break;
			case dsTextureDim_Cube:
				if (depth > 0)
				{
					glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevels, internalFormat, width,
						height, depth);
				}
				else
					glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipLevels, internalFormat, width, height);
				break;
		}

		if (data)
		{
			for (uint32_t i = 0; i < mipLevels; ++i)
			{
				uint32_t mipWidth = dsMax(1U, width >> i);
				uint32_t mipHeight = dsMax(1U, height >> i);
				size_t offset = dsTexture_surfaceOffset(format, dimension, width, height,
					depth, mipLevels, dsCubeFace_PosX, 0, i);

				switch (dimension)
				{
					case dsTextureDim_1D:
						if (depth > 0)
						{
							if (compressed)
							{
								glCompressedTexSubImage2D(GL_TEXTURE_1D_ARRAY, i, 0, 0,
									mipWidth, depth, internalFormat, (GLsizei)size,
									dataBytes + offset);
							}
							else
							{
								glTexSubImage2D(GL_TEXTURE_1D_ARRAY, i, 0, 0, mipWidth, depth,
									glFormat, type, dataBytes + offset);
							}
						}
						else
						{
							if (compressed)
							{
								glCompressedTexSubImage1D(GL_TEXTURE_1D, i, 0, mipWidth,
									internalFormat, (GLsizei)size, dataBytes + offset);
							}
							else
							{
								glTexSubImage1D(GL_TEXTURE_1D, i, 0, mipWidth, glFormat, type,
									dataBytes + offset);
							}
						}
						break;
					case dsTextureDim_2D:
						if (depth > 0)
						{
							if (compressed)
							{
								glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, 0,
									mipWidth, mipHeight, depth, internalFormat, (GLsizei)size,
									dataBytes + offset);
							}
							else
							{
								glTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, 0, mipWidth,
									mipHeight, depth, glFormat, type, dataBytes + offset);
							}
						}
						else
						{
							if (compressed)
							{
								glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipWidth,
									mipHeight, internalFormat, (GLsizei)size,
									dataBytes + offset);
							}
							else
							{
								glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipWidth, mipHeight,
									glFormat, type, dataBytes + offset);
							}
						}
						break;
					case dsTextureDim_3D:
					{
						uint32_t mipDepth = dsMax(1U, depth >> i);
						if (compressed)
						{
							glCompressedTexSubImage3D(GL_TEXTURE_3D, i, 0, 0, 0, mipWidth,
								mipHeight, mipDepth, internalFormat, (GLsizei)size,
								dataBytes + offset);
						}
						else
						{
							glTexSubImage3D(GL_TEXTURE_3D, i, 0, 0, 0, mipWidth, mipHeight,
								mipDepth, glFormat, type, dataBytes + offset);
						}
						break;
					}
					case dsTextureDim_Cube:
						if (depth > 0)
						{
							if (compressed)
							{
								glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, i, 0, 0, 0,
									mipWidth, mipHeight, depth*faces, internalFormat, (GLsizei)size,
									dataBytes + offset);
							}
							else
							{
								glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, i, 0, 0, 0, mipWidth,
									mipHeight, depth*faces, glFormat, type, dataBytes + offset);
							}
						}
						else
						{
							for (unsigned int j = 0; j < faces; ++j)
							{
								if (compressed)
								{
									glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i,
										0, 0, mipWidth, mipHeight, internalFormat, (GLsizei)size,
										dataBytes + offset);
								}
								else
								{
									glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, 0, 0,
										mipWidth, mipHeight, glFormat, type, dataBytes + offset);
								}
							}
						}
						break;
					default:
						DS_ASSERT(false);
				}
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < mipLevels; ++i)
		{
			uint32_t mipWidth = dsMax(1U, width >> i);
			uint32_t mipHeight = dsMax(1U, height >> i);
			for (unsigned int j = 0; j < faces; ++j)
			{
				size_t offset = 0;
				if (data)
				{
					offset = dsTexture_surfaceOffset(format, dimension, width, height, depth,
						mipLevels, (dsCubeFace)j, 0, i);
				}

				switch (dimension)
				{
					case dsTextureDim_1D:
						if (depth > 0)
						{
							if (compressed)
							{
								glCompressedTexImage2D(GL_TEXTURE_1D_ARRAY, i, internalFormat,
									mipWidth, depth, 0, (GLsizei)size, dataBytes + offset);
							}
							else
							{
								glTexImage2D(GL_TEXTURE_1D_ARRAY, i, internalFormat, mipWidth,
									depth, 0, glFormat, type, dataBytes + offset);
							}
						}
						else
						{
							if (compressed)
							{
								glCompressedTexImage1D(GL_TEXTURE_1D, i, internalFormat, mipWidth,
									0, (GLsizei)size, dataBytes + offset);
							}
							else
							{
								glTexImage1D(GL_TEXTURE_1D, i, internalFormat, mipWidth, 0,
									glFormat, type, dataBytes + offset);
							}
						}
						break;
					case dsTextureDim_2D:
						if (depth > 0)
						{
							if (compressed)
							{
								glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat,
									mipWidth, mipHeight, depth, 0, (GLsizei)size,
									dataBytes + offset);
							}
							else
							{
								glTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat, mipWidth,
									mipHeight, depth, 0, glFormat, type, dataBytes + offset);
							}
						}
						else
						{
							if (compressed)
							{
								glCompressedTexImage2D(GL_TEXTURE_2D, i, internalFormat, mipWidth,
									mipHeight, 0, (GLsizei)size, dataBytes + offset);
							}
							else
							{
								glTexImage2D(GL_TEXTURE_2D, i, internalFormat, mipWidth, mipHeight,
									0, glFormat, type, dataBytes + offset);
							}
						}
						break;
					case dsTextureDim_3D:
					{
						uint32_t mipDepth = dsMax(1U, depth >> i);
						if (compressed)
						{
							glCompressedTexImage3D(GL_TEXTURE_3D, i, internalFormat, mipWidth,
								mipHeight, mipDepth, 0, (GLsizei)size, dataBytes + offset);
						}
						else
						{
							glTexImage3D(GL_TEXTURE_3D, i, internalFormat, mipWidth, mipHeight,
								mipDepth, 0, glFormat, type, dataBytes + offset);
						}
						break;
					}
					case dsTextureDim_Cube:
						DS_ASSERT(depth == 0);
						if (compressed)
						{
							glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i,
								internalFormat, mipWidth, mipHeight, 0, (GLsizei)size,
								dataBytes + offset);
						}
						else
						{
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, internalFormat,
								mipWidth, mipHeight, 0, glFormat, type, dataBytes + offset);
						}
						break;
					default:
						DS_ASSERT(false);
				}
			}
		}

		if (resourceManager->hasArbitraryMipmapping)
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
	}
	glBindTexture(target, 0);

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating texture: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsClearGLErrors();
		dsGLTexture_destroy(resourceManager, baseTexture);
		return NULL;
	}

	return baseTexture;
}

dsOffscreen* dsGLTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, uint16_t samples, bool resolve)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLTexture* texture = (dsGLTexture*)dsAllocator_alloc(allocator, sizeof(dsGLTexture));
	if (!texture)
		return NULL;

	dsTexture* baseTexture = (dsTexture*)texture;
	baseTexture->resourceManager = resourceManager;
	baseTexture->allocator = dsAllocator_keepPointer(allocator);
	baseTexture->usage = (dsTextureUsage)usage;
	baseTexture->memoryHints = (dsGfxMemory)memoryHints;
	baseTexture->format = format;
	baseTexture->dimension = dimension;
	baseTexture->width = width;
	baseTexture->height = height;
	baseTexture->depth = depth;
	baseTexture->mipLevels = mipLevels;
	baseTexture->offscreen = false;
	baseTexture->resolve = resolve;
	baseTexture->samples = samples;

	texture->textureId = 0;
	texture->drawBufferId = 0;
	dsGLResource_initialize(&texture->resource);

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	// Format should have been validated earlier.
	GLenum internalFormat;
	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, &glFormat, &type,
		resourceManager, format));

	if (samples > 1 && resolve && ANYGL_SUPPORTED(glRenderbufferStorageMultisample))
	{
		glGenRenderbuffers(1, &texture->drawBufferId);
		if (!texture->drawBufferId)
		{
			GLenum error = glGetError();
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating offscreen: %s",
				AnyGL_errorString(error));
			errno = dsGetGLErrno(error);
			dsGLTexture_destroy(resourceManager, baseTexture);
			AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
			return NULL;
		}

		glBindRenderbuffer(GL_RENDERBUFFER, texture->drawBufferId);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating offscreen: %s",
				AnyGL_errorString(error));
			errno = dsGetGLErrno(error);
			dsClearGLErrors();
			dsGLTexture_destroy(resourceManager, baseTexture);
			AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
			return NULL;
		}
	}

	glGenTextures(1, &texture->textureId);
	if (!texture->textureId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating texture: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLTexture_destroy(resourceManager, baseTexture);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	GLenum target = dsGLTexture_target(baseTexture);
	glBindTexture(target, texture->textureId);

	DS_ASSERT(mipLevels > 0);
	if (ANYGL_SUPPORTED(glTexStorage2D))
	{
		switch (dimension)
		{
			case dsTextureDim_1D:
				if (depth > 0)
					glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, internalFormat, width, depth);
				else
					glTexStorage1D(GL_TEXTURE_1D, mipLevels, internalFormat, width);
				break;
			case dsTextureDim_2D:
				if (depth > 0)
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, internalFormat, width, height,
						depth);
				}
				else
					glTexStorage2D(GL_TEXTURE_1D, mipLevels, internalFormat, width, height);
				break;
			case dsTextureDim_3D:
				glTexStorage3D(GL_TEXTURE_3D, mipLevels, internalFormat, width, height, depth);
				break;
			case dsTextureDim_Cube:
				if (depth > 0)
				{
					glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevels, internalFormat, width,
						height, depth);
				}
				else
					glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipLevels, internalFormat, width, height);
				break;
		}
	}
	else
	{
		DS_ASSERT(dsGfxFormat_compressedIndex(format) == 0);
		unsigned int faces = dimension == dsTextureDim_Cube ? 6 : 1;
		for (uint32_t i = 0; i < mipLevels; ++i)
		{
			uint32_t mipWidth = dsMax(1U, width >> i);
			uint32_t mipHeight = dsMax(1U, height >> i);
			for (unsigned int j = 0; j < faces; ++j)
			{
				switch (dimension)
				{
					case dsTextureDim_1D:
						if (depth > 0)
						{
							glTexImage2D(GL_TEXTURE_1D_ARRAY, i, internalFormat, mipWidth, depth, 0,
								glFormat, type, NULL);
						}
						else
						{
							glTexImage1D(GL_TEXTURE_1D, i, internalFormat, mipWidth, 0, glFormat,
								type, NULL);
						}
						break;
					case dsTextureDim_2D:
						if (depth > 0)
						{
							glTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat, mipWidth,
								mipHeight, depth, 0, glFormat, type, NULL);
						}
						else
						{
							glTexImage2D(GL_TEXTURE_2D, i, internalFormat, mipWidth, mipHeight, 0,
								glFormat, type, NULL);
						}
						break;
					case dsTextureDim_3D:
					{
						uint32_t mipDepth = dsMax(1U, depth >> i);
						glTexImage3D(GL_TEXTURE_3D, i, internalFormat, mipWidth, mipHeight,
							mipDepth, 0, glFormat, type, NULL);
						break;
					}
					case dsTextureDim_Cube:
						DS_ASSERT(depth == 0);
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, internalFormat,
							mipWidth, mipHeight, 0, glFormat, type, NULL);
						break;
					default:
						DS_ASSERT(false);
				}
			}
		}

		if (resourceManager->hasArbitraryMipmapping)
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
	}
	glBindTexture(target, 0);

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating texture: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsClearGLErrors();
		dsGLTexture_destroy(resourceManager, baseTexture);
		return NULL;
	}

	return baseTexture;
}

bool dsGLTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(texture);
	DS_ASSERT(position);

	return dsGLCommandBuffer_copyTextureData(commandBuffer, texture, position, width, height,
		layers, data, size);
}

bool dsGLTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	size_t regionCount)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(srcTexture);
	DS_ASSERT(dstTexture);
	DS_ASSERT(regions || regionCount == 0);

	return dsGLCommandBuffer_copyTexture(commandBuffer, srcTexture, dstTexture, regions,
		regionCount);
}

bool dsGLTexture_blit(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureBlitRegion* regions,
	size_t regionCount, dsBlitFilter filter)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(srcTexture);
	DS_ASSERT(dstTexture);
	DS_ASSERT(regions || regionCount == 0);

	return dsGLCommandBuffer_blitTexture(commandBuffer, srcTexture, dstTexture, regions,
		regionCount, filter);
}

bool dsGLTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height)
{
	DS_UNUSED(size);
	DS_ASSERT(result);
	DS_ASSERT(texture);
	DS_ASSERT(position);

	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(NULL, &glFormat, &type, resourceManager,
		texture->format));
	if (texture->offscreen)
	{
		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);

		uint32_t layer = position->depth;
		if (texture->dimension == dsTextureDim_Cube)
			layer = layer*6 + position->face;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		if (ANYGL_SUPPORTED(glReadBuffer))
			glReadBuffer(GL_COLOR_ATTACHMENT0);
		dsGLBindFramebufferTexture(GL_READ_FRAMEBUFFER, texture, position->mipLevel, layer);

		glReadPixels(position->x, position->y, width, height, glFormat, type, result);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &framebuffer);
	}
	else
	{
		DS_ASSERT(ANYGL_SUPPORTED(glGetTexImage));
		dsGLTexture* glTexture = (dsGLTexture*)texture;
		GLenum target = dsGLTexture_target(texture);

		uint32_t mipWidth = dsMax(1U, texture->width >> position->mipLevel);
		uint32_t mipHeight = dsMax(1U, texture->height >> position->mipLevel);
		DS_ASSERT(mipWidth >= width && mipHeight >= height);
		void* buffer = result;
		if (mipWidth != width || mipHeight != height)
		{
			size_t levelSize = dsTexture_size(texture->format, dsTextureDim_2D, mipWidth, mipHeight,
				1, 1, 1);
			buffer = dsAllocator_alloc(resourceManager->allocator, levelSize);
			if (!buffer)
				return false;
		}

		glBindTexture(target, glTexture->textureId);
		glGetTexImage(target, position->mipLevel, glFormat, type, buffer);
		glBindTexture(target, 0);

		if (buffer != result)
		{
			unsigned int blockX, blockY;
			DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, texture->format));
			unsigned int formatSize = dsGfxFormat_size(texture->format);
			size_t offset = (position->y/blockY*mipWidth + position->x/blockX)*formatSize;
			size_t srcPitch = (mipWidth + blockX - 1)/blockX*formatSize;
			size_t dstPitch = (width + blockX - 1)/blockX*formatSize;
			uint8_t* srcBytes = (uint8_t*)buffer + offset;
			uint8_t* dstBytes = (uint8_t*)result;
			uint32_t yBlocks = (height + blockY - 1)/blockY;
			for (uint32_t y = 0; y < yBlocks; ++y, srcBytes += srcPitch, dstBytes += dstPitch)
				memcpy(dstBytes, srcBytes, dstPitch);
			DS_VERIFY(dsAllocator_free(resourceManager->allocator, buffer));
		}
	}

	return true;
}

static bool destroyImpl(dsTexture* texture)
{
	dsGLTexture* glTexture = (dsGLTexture*)texture;
	if (glTexture->textureId)
		glDeleteTextures(1, &glTexture->textureId);
	if (glTexture->drawBufferId)
		glDeleteRenderbuffers(1, &glTexture->drawBufferId);
	if (texture->allocator)
		return dsAllocator_free(texture->allocator, texture);

	return true;
}

bool dsGLTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(texture);

	dsGLTexture* glTexture = (dsGLTexture*)texture;
	if (dsGLResource_destroy(&glTexture->resource))
		return destroyImpl(texture);

	return true;
}

GLenum dsGLTexture_target(const dsTexture* texture)
{
	switch (texture->dimension)
	{
		case dsTextureDim_1D:
			return GL_TEXTURE_1D;
		case dsTextureDim_2D:
			return GL_TEXTURE_2D;
		case dsTextureDim_3D:
			return GL_TEXTURE_3D;
		case dsTextureDim_Cube:
			return GL_TEXTURE_CUBE_MAP;
		default:
			DS_ASSERT(false);
			return GL_TEXTURE_2D;
	}
}

GLenum dsGLTexture_copyTarget(const dsTexture* texture)
{
	switch (texture->dimension)
	{
		case dsTextureDim_1D:
			return texture->depth > 0 ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
		case dsTextureDim_2D:
			return texture->depth > 0 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
		case dsTextureDim_3D:
			return GL_TEXTURE_3D;
		case dsTextureDim_Cube:
			return texture->depth > 0 ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
		default:
			DS_ASSERT(false);
			return GL_TEXTURE_2D;
	}
}

GLenum dsGLTexture_attachment(const dsTexture* texture)
{
	switch (texture->format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
			return GL_DEPTH_ATTACHMENT;
		case dsGfxFormat_S8:
			return GL_STENCIL_ATTACHMENT;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			return GL_DEPTH_STENCIL_ATTACHMENT;
		default:
			return GL_COLOR_ATTACHMENT0;
	}
}

GLbitfield dsGLTexture_buffers(const dsTexture* texture)
{
	switch (texture->format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
			return GL_DEPTH_BUFFER_BIT;
		case dsGfxFormat_S8:
			return GL_STENCIL_BUFFER_BIT;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			return GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		default:
			return GL_COLOR_BUFFER_BIT;
	}
}

void dsGLTexture_addInternalRef(dsTexture* texture)
{
	DS_ASSERT(texture);
	dsGLTexture* glTexture = (dsGLTexture*)texture;
	dsGLResource_addRef(&glTexture->resource);
}

void dsGLTexture_freeInternalRef(dsTexture* texture)
{
	DS_ASSERT(texture);
	dsGLTexture* glTexture = (dsGLTexture*)texture;
	if (dsGLResource_freeRef(&glTexture->resource))
		destroyImpl(texture);
}
