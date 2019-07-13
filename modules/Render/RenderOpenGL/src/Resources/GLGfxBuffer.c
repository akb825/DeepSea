/*
 * Copyright 2017-2019 Aaron Barany
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
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLResource.h"
#include "GLTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>

#include <string.h>

static bool needsMapEmulation(const dsResourceManager* resourceManager, dsGfxBufferMap flags)
{
	// Emulate persistent mapping.
	if ((flags & dsGfxBufferMap_Persistent) &&
			resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		return true;
	}

	// Emulate mapping all together.
	if (!ANYGL_SUPPORTED(glMapBuffer) && !ANYGL_SUPPORTED(glMapBufferRange))
		return true;

	// Emulate orphaning of buffers.
	if ((flags & dsGfxBufferMap_Orphan) && !ANYGL_SUPPORTED(glMapBufferRange))
		return true;

	return false;
}

static bool readBufferData(void* outData, dsGfxBuffer* buffer, GLenum bufferType, size_t offset,
	size_t size)
{
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;

	void* ptr = NULL;
	glBindBuffer(bufferType, glBuffer->bufferId);
	if (ANYGL_SUPPORTED(glMapBufferRange))
	{
		GLbitfield access = GL_MAP_READ_BIT;
		if (!(buffer->memoryHints & dsGfxMemory_Synchronize))
			access |= GL_MAP_UNSYNCHRONIZED_BIT;
		ptr = glMapBufferRange(bufferType, offset, size, access);
	}
	else if (ANYGL_SUPPORTED(glMapBuffer))
	{
		ptr = glMapBuffer(bufferType, GL_READ_ONLY);
		if (ptr)
			ptr = (uint8_t*)ptr + offset;
	}
	else if (ANYGL_SUPPORTED(glGetBufferSubData))
	{
		glGetBufferSubData(bufferType, offset, size, outData);
		glBindBuffer(bufferType, 0);
		return true;
	}
	else
	{
		glBindBuffer(bufferType, 0);
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot read from buffers when no mapping or copying is supported.");
		return false;
	}

	if (!ptr)
	{
		glBindBuffer(bufferType, 0);
		errno = EPERM;
		return false;
	}

	memcpy(outData, ptr, size);
	glUnmapBuffer(bufferType);
	glBindBuffer(bufferType, 0);

	return true;
}

static bool writeBufferData(dsGfxBuffer* buffer, GLenum bufferType, size_t offset, size_t size,
	const void* data)
{
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;

	void* ptr = NULL;
	glBindBuffer(bufferType, glBuffer->bufferId);
	bool synchronize = (buffer->memoryHints & dsGfxMemory_Synchronize) ||
		(glBuffer->mapFlags & dsGfxBufferMap_Orphan);
	if (ANYGL_SUPPORTED(glMapBufferRange) && !synchronize)
	{
		ptr = glMapBufferRange(bufferType, offset, size,
			GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}
	else if (ANYGL_SUPPORTED(glMapBuffer) && !synchronize)
	{
		ptr = glMapBuffer(bufferType, GL_WRITE_ONLY);
		if (ptr)
			ptr = (uint8_t*)ptr + offset;
	}
	else
	{
		glBufferSubData(bufferType, offset, size, data);
		glBindBuffer(bufferType, 0);
		return true;
	}

	if (!ptr)
	{
		glBindBuffer(bufferType, 0);
		errno = EPERM;
		return false;
	}

	memcpy(ptr, data, size);
	glUnmapBuffer(bufferType);
	glBindBuffer(bufferType, 0);

	return true;
}

dsGfxBuffer* dsGLGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLGfxBuffer* buffer = DS_ALLOCATE_OBJECT(allocator, dsGLGfxBuffer);
	if (!buffer)
		return NULL;

	dsGfxBuffer* baseBuffer = (dsGfxBuffer*)buffer;
	baseBuffer->resourceManager = resourceManager;
	baseBuffer->allocator = dsAllocator_keepPointer(allocator);
	baseBuffer->usage = usage;
	baseBuffer->memoryHints = memoryHints;
	baseBuffer->size = size;

	buffer->bufferId = 0;
	dsGLResource_initialize(&buffer->resource);

	DS_VERIFY(dsSpinlock_initialize(&buffer->mapLock));
	buffer->mapFlags = 0;
	buffer->emulatedMap = false;
	buffer->scratchAllocator = resourceManager->allocator;
	buffer->mappedBuffer = NULL;
	buffer->mappedOffset = 0;
	buffer->mappedSize = 0;
	buffer->mappedBufferCapacity = 0;

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

		// Explicitly copy or emulate persistent mapping.
		if ((usage & dsGfxBufferUsage_CopyTo) || ((memoryHints & dsGfxMemory_Persistent) &&
				resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent))
		{
			flags |= GL_DYNAMIC_STORAGE_BIT;
		}

		if (!(memoryHints & dsGfxMemory_GPUOnly))
		{
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

	// Make sure it's visible from the main render thread.
	if (!dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
		glFlush();

	return baseBuffer;
}

void* dsGLGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	DS_VERIFY(dsSpinlock_lock(&glBuffer->mapLock));

	if (glBuffer->mappedSize > 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Buffer is already mapped.");
		return NULL;
	}

	size = dsMin(size, buffer->size - offset);
	bool emulate = needsMapEmulation(resourceManager, flags);
	if (emulate && (!glBuffer->mappedBuffer || glBuffer->mappedBufferCapacity < size))
	{
		DS_VERIFY(dsAllocator_free(glBuffer->scratchAllocator, glBuffer->mappedBuffer));
		glBuffer->mappedBuffer = dsAllocator_alloc(glBuffer->scratchAllocator, size);
		glBuffer->mappedBufferCapacity = size;
		if (!glBuffer->mappedBuffer)
		{
			DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
			return NULL;
		}
	}

	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	void* ptr = NULL;
	if (emulate)
	{
		if ((flags & dsGfxBufferMap_Read) &&
			!readBufferData(glBuffer->mappedBuffer, buffer, bufferType, offset, size))
		{
			DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
			return NULL;
		}
		ptr = glBuffer->mappedBuffer;
	}
	else if (ANYGL_SUPPORTED(glMapBufferRange))
	{
		GLbitfield access = 0;
		if (flags & dsGfxBufferMap_Read)
			access |= GL_MAP_READ_BIT;
		if (flags & dsGfxBufferMap_Write)
			access |= GL_MAP_WRITE_BIT;
		if (flags & dsGfxBufferMap_Orphan)
			access |= GL_MAP_INVALIDATE_BUFFER_BIT;
		if (resourceManager->bufferMapSupport == dsGfxBufferMapSupport_Persistent &&
			flags & dsGfxBufferMap_Persistent)
		{
			access |= GL_MAP_PERSISTENT_BIT;
			if (buffer->memoryHints & dsGfxMemory_Coherent)
				access |= GL_MAP_COHERENT_BIT;
			else if (flags & dsGfxBufferMap_Write)
				access |= GL_MAP_FLUSH_EXPLICIT_BIT;
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
		if (ptr)
			ptr = (uint8_t*)ptr + offset;
	}

	if (ptr)
	{
		glBuffer->mapFlags = flags;
		glBuffer->emulatedMap = emulate;
		glBuffer->mappedOffset = offset;
		glBuffer->mappedSize = size;
	}
	else
		errno = EPERM;

	DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));

	return ptr;
}

bool dsGLGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	DS_VERIFY(dsSpinlock_lock(&glBuffer->mapLock));

	if (glBuffer->mappedSize == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Buffer isn't mapped.");
		return false;
	}

	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	bool success = true;
	if (glBuffer->emulatedMap)
	{
		if ((glBuffer->mapFlags & dsGfxBufferMap_Write) &&
			!(glBuffer->mapFlags & dsGfxBufferMap_Persistent))
		{
			success = writeBufferData(buffer, bufferType, glBuffer->mappedOffset,
				glBuffer->mappedSize, glBuffer->mappedBuffer);
		}
	}
	else
	{
		DS_ASSERT(ANYGL_SUPPORTED(glUnmapBuffer));
		glBindBuffer(bufferType, glBuffer->bufferId);
		success = glUnmapBuffer(bufferType);
		glBindBuffer(bufferType, 0);
	}

	// Make sure it's visible from the main render thread.
	if (success && !dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
		glFlush();

	glBuffer->mapFlags = 0;
	glBuffer->mappedOffset = 0;
	glBuffer->mappedSize = 0;

	DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));

	return success;
}

bool dsGLGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer, size_t offset,
	size_t size)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	DS_VERIFY(dsSpinlock_lock(&glBuffer->mapLock));

	if (glBuffer->mappedSize == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Buffer isn't mapped.");
		return false;
	}

	bool success;
	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	if (glBuffer->emulatedMap || !ANYGL_SUPPORTED(glFlushMappedBufferRange))
	{
		size = dsMin(size, buffer->size - offset);
		size_t end = offset + size;
		offset = dsMax(offset, glBuffer->mappedOffset);
		end = dsMin(end, glBuffer->mappedOffset + glBuffer->mappedSize);
		success = writeBufferData(buffer, bufferType, offset, end - offset,
			(uint8_t*)glBuffer->mappedBuffer + offset - glBuffer->mappedOffset);
	}
	else
	{
		glBindBuffer(bufferType, glBuffer->bufferId);
		glFlushMappedBufferRange(bufferType, offset, size);
		glBindBuffer(bufferType, 0);
		success = true;
	}

	// Make sure it's visible from the main render thread.
	if (!dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
		glFlush();

	DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));

	return success;
}

bool dsGLGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	DS_ASSERT(glBuffer && glBuffer->bufferId);

	DS_VERIFY(dsSpinlock_lock(&glBuffer->mapLock));

	if (glBuffer->mappedSize == 0)
	{
		DS_VERIFY(dsSpinlock_unlock(&glBuffer->mapLock));
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Buffer isn't mapped.");
		return false;
	}

	bool success = true;
	if (glBuffer->emulatedMap)
	{
		size = dsMin(size, buffer->size - offset);
		size_t end = offset + size;
		offset = dsMax(offset, glBuffer->mappedOffset);
		end = dsMin(end, glBuffer->mappedOffset + glBuffer->mappedSize);
		success = readBufferData((uint8_t*)glBuffer->mappedBuffer + offset - glBuffer->mappedOffset,
			buffer, dsGetGLBufferType(buffer->usage), offset, end - offset);
	}

	return success;
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
	DS_VERIFY(dsAllocator_free(glBuffer->scratchAllocator, glBuffer->mappedBuffer));
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));

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
