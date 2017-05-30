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
#include "AnyGL/gl.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

struct dsGLMainCommandBuffer
{
	dsGLCommandBuffer commandBuffer;
};

bool dsGLMainCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->bufferId);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool dsGLMainCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	dsGLGfxBuffer* glSrcBuffer = (dsGLGfxBuffer*)srcBuffer;
	dsGLGfxBuffer* glDstBuffer = (dsGLGfxBuffer*)dstBuffer;
	glCopyBufferSubData(glSrcBuffer->bufferId, glDstBuffer->bufferId, srcOffset, dstOffset, size);
	return true;
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
	&dsGLMainCommandBuffer_submit
};

dsGLMainCommandBuffer* dsGLMainCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator)
{
	dsGLMainCommandBuffer* commandBuffer = (dsGLMainCommandBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsGLMainCommandBuffer));
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = dsAllocator_keepPointer(allocator);
	baseCommandBuffer->usage = dsCommandBufferUsage_Standard;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;

	return commandBuffer;
}

bool dsGLMainCommandBuffer_destroy(dsGLMainCommandBuffer* commandBuffer)
{
	if (!commandBuffer)
		return true;

	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	if (allocator)
		return dsAllocator_free(allocator, commandBuffer);
	return true;
}
