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

#include "GLOtherCommandBuffer.h"

#include "Resources/GLGfxBuffer.h"
#include "GLCommandBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

typedef enum CommandType
{
	CommandType_CopyBufferData,
	CommandType_CopyBuffer
} CommandType;

typedef struct Command
{
	CommandType type;
	size_t size;
} Command;

typedef struct CopyBufferDataCommand
{
	Command command;
	dsGfxBuffer* buffer;
	size_t offset;
	size_t size;
	uint8_t data[];
} CopyBufferDataCommand;

typedef struct CopyBufferCommand
{
	Command command;
	dsGfxBuffer* srcBuffer;
	size_t srcOffset;
	dsGfxBuffer* dstBuffer;
	size_t dstOffset;
	size_t size;
} CopyBufferCommand;

struct dsGLOtherCommandBuffer
{
	dsGLCommandBuffer commandBuffer;
	dsBufferAllocator buffer;
};

static Command* allocateCommand(dsCommandBuffer* commandBuffer, CommandType type, size_t size)
{
	DS_ASSERT(size >= sizeof(Command));
	dsGLOtherCommandBuffer* glCommandBuffer = (dsGLOtherCommandBuffer*)commandBuffer;
	int prevErrno = errno;
	Command* command = (Command*)dsAllocator_alloc((dsAllocator*)&glCommandBuffer->buffer, size);
	if (!command)
	{
		// Allocate a new buffer.
		errno = prevErrno;
		size_t newBufferSize = dsMax(glCommandBuffer->buffer.bufferSize*2,
			glCommandBuffer->buffer.bufferSize + size);
		void* newBuffer = dsAllocator_alloc(commandBuffer->allocator, newBufferSize);
		if (!newBuffer)
			return NULL;

		void* oldBuffer = glCommandBuffer->buffer.buffer;
		memcpy(newBuffer, oldBuffer, ((dsAllocator*)&glCommandBuffer->buffer)->size);
		DS_VERIFY(dsBufferAllocator_initialize(&glCommandBuffer->buffer, newBuffer, newBufferSize));
		DS_VERIFY(dsAllocator_free(commandBuffer->allocator, oldBuffer));

		command = (Command*)dsAllocator_alloc((dsAllocator*)&glCommandBuffer->buffer, size);
		DS_ASSERT(command);
	}

	command->type = type;
	command->size = DS_ALIGNED_SIZE(size);
	return command;
}

bool dsGLOtherCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	size_t commandSize = DS_ALIGNED_SIZE(sizeof(CopyBufferCommand)) + DS_ALIGNED_SIZE(size);
	CopyBufferDataCommand* command = (CopyBufferDataCommand*)allocateCommand(commandBuffer,
		CommandType_CopyBufferData, commandSize);
	if (!command)
		return false;

	dsGLGfxBuffer_addInternalRef(buffer);
	command->buffer = buffer;
	command->offset = offset;
	command->size = size;
	memcpy(command->data, data, size);
	return true;
}

bool dsGLOtherCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	CopyBufferCommand* command = (CopyBufferCommand*)allocateCommand(commandBuffer,
		CommandType_CopyBuffer, sizeof(CopyBufferCommand));
	if (!command)
		return false;

	dsGLGfxBuffer_addInternalRef(srcBuffer);
	dsGLGfxBuffer_addInternalRef(dstBuffer);
	command->srcBuffer = srcBuffer;
	command->srcOffset = srcOffset;
	command->dstBuffer = dstBuffer;
	command->dstOffset = dstOffset;
	command->size = size;
	return true;
}

bool dsGLOtherCommandBuffer_submit(dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer)
{
	dsGLOtherCommandBuffer* glSubmitBuffer = (dsGLOtherCommandBuffer*)submitBuffer;
	uint8_t* buffer = (uint8_t*)glSubmitBuffer->buffer.buffer;
	size_t bufferSize = ((dsAllocator*)&glSubmitBuffer->buffer)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		switch (command->type)
		{
			case CommandType_CopyBufferData:
			{
				CopyBufferDataCommand* thisCommand = (CopyBufferDataCommand*)command;
				dsGLCommandBuffer_copyBufferData(commandBuffer, thisCommand->buffer,
					thisCommand->offset, thisCommand->data, thisCommand->size);
				break;
			}
			case CommandType_CopyBuffer:
			{
				CopyBufferCommand* thisCommand = (CopyBufferCommand*)command;
				dsGLCommandBuffer_copyBuffer(commandBuffer, thisCommand->srcBuffer,
					thisCommand->srcOffset, thisCommand->dstBuffer, thisCommand->dstOffset,
					thisCommand->size);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	// Reset immediately if not submitted multiple times. This frees any internal references to
	// resources.
	if (!(commandBuffer->usage &
		(dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame)))
	{
		dsGLOtherCommandBuffer_reset(glSubmitBuffer);
	}
	return true;
}

static CommandBufferFunctionTable functionTable =
{
	&dsGLOtherCommandBuffer_copyBufferData,
	&dsGLOtherCommandBuffer_copyBuffer,
	&dsGLOtherCommandBuffer_submit
};

dsGLOtherCommandBuffer* dsGLOtherCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Command buffer allocator must support freeing memory.");
		return NULL;
	}

	dsGLOtherCommandBuffer* commandBuffer = (dsGLOtherCommandBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsGLOtherCommandBuffer));
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;

	const size_t defaultBufferSize = 512*1024;
	void* bufferData = dsAllocator_alloc(allocator, defaultBufferSize);
	if (!bufferData)
	{
		dsAllocator_free(allocator, commandBuffer);
		return NULL;
	}

	DS_VERIFY(dsBufferAllocator_initialize(&commandBuffer->buffer, bufferData, defaultBufferSize));
	return commandBuffer;
}

void dsGLOtherCommandBuffer_reset(dsGLOtherCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);

	// Free any internal refs for resources.
	dsGLOtherCommandBuffer* glCommandBuffer = (dsGLOtherCommandBuffer*)commandBuffer;
	uint8_t* buffer = (uint8_t*)glCommandBuffer->buffer.buffer;
	size_t bufferSize = ((dsAllocator*)&glCommandBuffer->buffer)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		switch (command->type)
		{
			case CommandType_CopyBufferData:
			{
				CopyBufferDataCommand* thisCommand = (CopyBufferDataCommand*)command;
				dsGLGfxBuffer_freeInternalRef(thisCommand->buffer);
				break;
			}
			case CommandType_CopyBuffer:
			{
				CopyBufferCommand* thisCommand = (CopyBufferCommand*)command;
				dsGLGfxBuffer_freeInternalRef(thisCommand->srcBuffer);
				dsGLGfxBuffer_freeInternalRef(thisCommand->dstBuffer);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}
	DS_VERIFY(dsBufferAllocator_reset(&commandBuffer->buffer));
}

bool dsGLOtherCommandBuffer_destroy(dsGLOtherCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	dsGLOtherCommandBuffer_reset(commandBuffer);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->buffer.buffer));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer));
	return true;
}
