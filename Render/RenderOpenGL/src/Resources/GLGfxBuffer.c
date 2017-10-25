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

#include "Resources/GLGfxBuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLGfxBuffer.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLResource.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>

dsGfxBuffer* dsGLGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLGfxBuffer* buffer = (dsGLGfxBuffer*)dsAllocator_alloc(allocator, sizeof(dsGLGfxBuffer));
	if (!buffer)
		return NULL;

	dsGfxBuffer* baseBuffer = (dsGfxBuffer*)buffer;
	baseBuffer->resourceManager = resourceManager;
	baseBuffer->allocator = dsAllocator_keepPointer(allocator);
	baseBuffer->usage = (dsGfxBufferUsage)usage;
	baseBuffer->memoryHints = (dsGfxMemory)memoryHints;
	baseBuffer->size = size;

	buffer->bufferId = 0;
	dsGLResource_initialize(&buffer->resource);

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	glGenBuffers(1, &buffer->bufferId);
	if (!buffer->bufferId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating graphics buffer: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLGfxBuffer_destroy(resourceManager, baseBuffer);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	GLenum bufferType = dsGetGLBufferType(usage);
	glBindBuffer(bufferType, buffer->bufferId);
	if (ANYGL_SUPPORTED(glBufferStorage))
	{
		GLbitfield flags = 0;

		bool noUpdate = (memoryHints & dsGfxMemory_Static) != 0;
		if (!noUpdate)
			flags |= GL_DYNAMIC_STORAGE_BIT;

		if (!(memoryHints & dsGfxMemory_GpuOnly))
		{
			if (!noUpdate)
				flags |= GL_MAP_WRITE_BIT;
			if ((memoryHints & dsGfxMemory_Read))
				flags |= GL_MAP_READ_BIT;

			if (memoryHints & dsGfxMemory_Persistent)
			{
				flags |= GL_MAP_PERSISTENT_BIT;
				if (memoryHints & dsGfxMemory_Coherent)
					flags |= GL_MAP_COHERENT_BIT;
			}
		}

		glBufferStorage(bufferType, size, data, flags);
	}
	else
	{
		bool hasCopyRead = AnyGL_atLeastVersion(1, 5, false) || AnyGL_atLeastVersion(3, 0, true);
		GLenum glUsage;
		if ((memoryHints & dsGfxMemory_Draw) || (!(memoryHints & dsGfxMemory_Read) &&
			!(usage & dsGfxBufferUsage_CopyFrom)) || !hasCopyRead)
		{
			if (memoryHints & dsGfxMemory_Static)
				glUsage = GL_STATIC_DRAW;
			else if (memoryHints & dsGfxMemory_Stream)
				glUsage = GL_STREAM_DRAW;
			else
				glUsage = GL_DYNAMIC_DRAW;
		}
		else if (memoryHints & dsGfxMemory_Read)
		{
			if (memoryHints & dsGfxMemory_Static)
				glUsage = GL_STATIC_READ;
			else if (memoryHints & dsGfxMemory_Stream)
				glUsage = GL_STREAM_READ;
			else
				glUsage = GL_DYNAMIC_READ;
		}
		else
		{
			if (memoryHints & dsGfxMemory_Static)
				glUsage = GL_STATIC_COPY;
			else if (memoryHints & dsGfxMemory_Stream)
				glUsage = GL_STREAM_COPY;
			else
				glUsage = GL_DYNAMIC_COPY;
		}

		glBufferData(bufferType, size, data, glUsage);
	}

	glBindBuffer(bufferType, 0);

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating graphics buffer: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsClearGLErrors();
		dsGLGfxBuffer_destroy(resourceManager, baseBuffer);
		return NULL;
	}

	return baseBuffer;
}

void* dsGLGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer, unsigned int flags,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	void* ptr = NULL;
	if (ANYGL_SUPPORTED(glMapBufferRange))
	{
		size = dsMin(size, buffer->size - offset);
		GLbitfield access = 0;
		if (flags & dsGfxBufferMap_Read)
			access |= GL_MAP_READ_BIT;
		if (flags & dsGfxBufferMap_Write)
			access |= GL_MAP_WRITE_BIT;
		if (flags & dsGfxBufferMap_Persistent)
		{
			access |= GL_MAP_PERSISTENT_BIT;
			if (buffer->memoryHints & dsGfxMemory_Coherent)
				access |= GL_MAP_COHERENT_BIT;
			else
				access += GL_MAP_FLUSH_EXPLICIT_BIT;
		}
		if (!(buffer->memoryHints & dsGfxMemory_Synchronize))
			access |= GL_MAP_UNSYNCHRONIZED_BIT;

		glBindBuffer(bufferType, glBuffer->bufferId);
		ptr = glMapBufferRange(bufferType, offset, size, access);
		glBindBuffer(bufferType, 0);
	}
	else
	{
		GLenum access;
		if ((flags & dsGfxBufferMap_Read) && (flags & dsGfxBufferMap_Write))
			access = GL_READ_WRITE;
		else if (flags & dsGfxBufferMap_Read)
			access = GL_READ_ONLY;
		else
			access = GL_WRITE_ONLY;

		DS_ASSERT(ANYGL_SUPPORTED(glMapBuffer));
		glBindBuffer(bufferType, glBuffer->bufferId);
		ptr = glMapBuffer(bufferType, access);
		glBindBuffer(bufferType, 0);
	}

	return ptr;
}

bool dsGLGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	DS_ASSERT(ANYGL_SUPPORTED(glUnmapBuffer));
	glBindBuffer(bufferType, glBuffer->bufferId);
	bool success = glUnmapBuffer(bufferType);
	glBindBuffer(bufferType, 0);

	return success;
}

bool dsGLGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer, size_t offset,
	size_t size)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	DS_ASSERT(ANYGL_SUPPORTED(glFlushMappedBufferRange));
	glBindBuffer(bufferType, glBuffer->bufferId);
	glFlushMappedBufferRange(bufferType, offset, size);
	glBindBuffer(bufferType, 0);

	return true;
}

bool dsGLGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);

	return true;
}

bool dsGLGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_copyBufferData(commandBuffer, buffer, offset, data, size);
}

bool dsGLGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size)
{
	DS_UNUSED(resourceManager);
	return dsGLCommandBuffer_copyBuffer(commandBuffer, srcBuffer, srcOffset, dstBuffer, dstOffset,
		size);
}

static bool destroyImpl(dsGfxBuffer* buffer)
{
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	if (glBuffer->bufferId)
		glDeleteBuffers(1, &glBuffer->bufferId);
	if (buffer->allocator)
		return dsAllocator_free(buffer->allocator, buffer);

	return true;
}

bool dsGLGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(buffer);

	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	if (dsGLResource_destroy(&glBuffer->resource))
		return destroyImpl(buffer);

	return true;
}

void dsGLGfxBuffer_addInternalRef(dsGfxBuffer* buffer)
{
	DS_ASSERT(buffer);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	dsGLResource_addRef(&glBuffer->resource);
}

void dsGLGfxBuffer_freeInternalRef(dsGfxBuffer* buffer)
{
	DS_ASSERT(buffer);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	if (dsGLResource_freeRef(&glBuffer->resource))
		destroyImpl(buffer);
}
