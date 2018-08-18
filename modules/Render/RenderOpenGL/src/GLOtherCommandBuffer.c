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

#include "Resources/GLDrawGeometry.h"
#include "Resources/GLFramebuffer.h"
#include "Resources/GLGfxBuffer.h"
#include "Resources/GLGfxFence.h"
#include "Resources/GLGfxQueryPool.h"
#include "Resources/GLRenderbuffer.h"
#include "Resources/GLShader.h"
#include "Resources/GLTexture.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLRenderPass.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <string.h>

typedef enum CommandType
{
	CommandType_CopyBufferData,
	CommandType_CopyBuffer,
	CommandType_CopyTextureData,
	CommandType_CopyTexture,
	CommandType_GenerateTextureMipmaps,
	CommandType_BeginQuery,
	CommandType_EndQuery,
	CommandType_QueryTimestamp,
	CommandType_CopyQueryValues,
	CommandType_BindShader,
	CommandType_SetTexture,
	CommandType_SetTextureBuffer,
	CommandType_SetShaderBuffer,
	CommandType_SetUniform,
	CommandType_UnbindShader,
	CommandType_BindComputeShader,
	CommandType_UnbindComputeShader,
	CommandType_BeginRenderSurface,
	CommandType_EndRenderSurface,
	CommandType_BeginRenderPass,
	CommandType_NextRenderSubpass,
	CommandType_EndRenderPass,
	CommandType_ClearColorSurface,
	CommandType_ClearDepthStencilSurface,
	CommandType_Draw,
	CommandType_DrawIndexed,
	CommandType_DrawIndirect,
	CommandType_DrawIndexedIndirect,
	CommandType_DispatchCompute,
	CommandType_DispatchComputeIndirect,
	CommandType_BlitSurface
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

typedef struct CopyTextureDataCommand
{
	Command command;
	dsTexture* texture;
	dsTexturePosition position;
	uint32_t width;
	uint32_t height;
	uint32_t layers;
	size_t size;
	uint8_t data[];
} CopyTextureDataCommand;

typedef struct CopyTextureCommand
{
	Command command;
	dsTexture* srcTexture;
	dsTexture* dstTexture;
	size_t regionCount;
	dsTextureCopyRegion regions[];
} CopyTextureCommand;

typedef struct GenerateTextureMipmapsCommand
{
	Command command;
	dsTexture* texture;
} GenerateTextureMipmapsCommand;

typedef struct BeginEndQueryCommand
{
	Command command;
	dsGfxQueryPool* queries;
	uint32_t query;
} BeginEndQueryCommand;

typedef struct QueryTimestampCommand
{
	Command command;
	dsGfxQueryPool* queries;
	uint32_t query;
} QueryTimestampCommand;

typedef struct CopyQueryValuesCommand
{
	Command command;
	dsGfxQueryPool* queries;
	uint32_t first;
	uint32_t count;
	dsGfxBuffer* buffer;
	size_t offset;
	size_t stride;
	size_t elementSize;
	bool checkAvailability;
} CopyQueryValuesCommand;

typedef struct BindShaderCommand
{
	Command command;
	const dsShader* shader;
	dsDynamicRenderStates renderStates;
} BindShaderCommand;

typedef struct SetTextureCommand
{
	Command command;
	const dsShader* shader;
	dsTexture* texture;
	uint32_t element;
} SetTextureCommand;

typedef struct SetTextureBufferCommand
{
	Command command;
	const dsShader* shader;
	dsGfxBuffer* buffer;
	uint32_t element;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} SetTextureBufferCommand;

typedef struct SetShaderBufferCommand
{
	Command command;
	const dsShader* shader;
	dsGfxBuffer* buffer;
	uint32_t element;
	size_t offset;
	size_t size;
} SetShaderBufferCommand;

typedef struct SetUniformCommand
{
	Command command;
	GLint location;
	dsMaterialType type;
	uint32_t count;
	// Type with the most strict alignment.
	double data[];
} SetUniformCommand;

typedef struct UnbindShaderCommand
{
	Command command;
	const dsShader* shader;
} UnbindShaderCommand;

typedef struct BindComputeShaderCommand
{
	Command command;
	const dsShader* shader;
} BindComputeShaderCommand;

typedef struct RenderSurfaceCommand
{
	Command command;
	void* glSurface;
} RenderSurfaceCommand;

typedef struct BeginRenderPassCommand
{
	Command command;
	const dsRenderPass* renderPass;
	const dsFramebuffer* framebuffer;
	dsAlignedBox3f viewport;
	bool viewportSet;
	uint32_t clearValueCount;
	dsSurfaceClearValue clearValues[];
} BeginRenderPassCommand;

typedef struct NextRenderSubpassCommand
{
	Command command;
	const dsRenderPass* renderPass;
	uint32_t subpassIndex;
} NextRenderSubpassCommand;

typedef struct EndRenderPassCommand
{
	Command command;
	const dsRenderPass* renderPass;
} EndRenderPassCommand;

typedef struct ClearColorSurfaceCommand
{
	Command command;
	dsFramebufferSurface surface;
	dsSurfaceColorValue value;
} ClearColorSurfaceCommand;

typedef struct ClearDepthStencilSurfaceCommand
{
	Command command;
	dsFramebufferSurface surface;
	dsClearDepthStencil surfaceParts;
	dsDepthStencilValue value;
} ClearDepthStencilSurfaceCommand;

typedef struct DrawCommand
{
	Command command;
	const dsDrawGeometry* geometry;
	dsDrawRange drawRange;
} DrawCommand;

typedef struct DrawIndexedCommand
{
	Command command;
	const dsDrawGeometry* geometry;
	dsDrawIndexedRange drawRange;
} DrawIndexedCommand;

typedef struct DrawIndirectCommand
{
	Command command;
	const dsDrawGeometry* geometry;
	const dsGfxBuffer* indirectBuffer;
	size_t offset;
	uint32_t count;
	uint32_t stride;
} DrawIndirectCommand;

typedef struct DispatchComputeCommand
{
	Command command;
	uint32_t x;
	uint32_t y;
	uint32_t z;
} DispatchComputeCommand;

typedef struct DispatchComputeIndirectCommand
{
	Command command;
	const dsGfxBuffer* indirectBuffer;
	size_t offset;
} DispatchComputeIndirectCommand;

typedef struct BlitSurfaceCommand
{
	Command command;
	void* srcSurface;
	void* dstSurface;
	dsGfxSurfaceType srcSurfaceType;
	dsGfxSurfaceType dstSurfaceType;
	dsBlitFilter filter;
	size_t regionCount;
	dsSurfaceBlitRegion regions[];
} BlitSurfaceCommand;

struct dsGLOtherCommandBuffer
{
	dsGLCommandBuffer commandBuffer;
	dsBufferAllocator buffer;

	dsGLFenceSyncRef** fenceSyncs;
	uint32_t curFenceSyncs;
	uint32_t maxFenceSyncs;
	bool bufferReadback;
};

static void addSurfaceRef(dsGfxSurfaceType type, void* surface)
{
	switch (type)
	{
		case dsGfxSurfaceType_Texture:
			dsGLTexture_addInternalRef((dsTexture*)surface);
			break;
		case dsGfxSurfaceType_Renderbuffer:
			dsGLRenderbuffer_addInternalRef((dsRenderbuffer*)surface);
			break;
		default:
			break;
	}
}

static void freeSurfaceRef(dsGfxSurfaceType type, void* surface)
{
	switch (type)
	{
		case dsGfxSurfaceType_Texture:
			dsGLTexture_freeInternalRef((dsTexture*)surface);
			break;
		case dsGfxSurfaceType_Renderbuffer:
			dsGLRenderbuffer_freeInternalRef((dsRenderbuffer*)surface);
			break;
		default:
			break;
	}
}

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
		void* newBuffer = dsAllocator_reallocWithFallback(commandBuffer->allocator,
			glCommandBuffer->buffer.buffer, ((dsAllocator*)&glCommandBuffer->buffer)->size,
			newBufferSize);
		if (!newBuffer)
			return NULL;

		DS_VERIFY(dsBufferAllocator_initialize(&glCommandBuffer->buffer, newBuffer, newBufferSize));
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
	size_t commandSize = sizeof(CopyBufferDataCommand) + size;
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

bool dsGLOtherCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	size_t commandSize = sizeof(CopyTextureDataCommand) + size;
	CopyTextureDataCommand* command = (CopyTextureDataCommand*)allocateCommand(commandBuffer,
		CommandType_CopyTextureData, commandSize);
	if (!command)
		return false;

	dsGLTexture_addInternalRef(texture);
	command->texture = texture;
	command->position = *position;
	command->width = width;
	command->height = height;
	command->layers = layers;
	command->size = size;
	memcpy(command->data, data, size);
	return true;
}

bool dsGLOtherCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount)
{
	size_t commandSize = sizeof(CopyTextureCommand) + sizeof(dsTextureCopyRegion)*regionCount;
	CopyTextureCommand* command = (CopyTextureCommand*)allocateCommand(commandBuffer,
		CommandType_CopyTexture, commandSize);
	if (!command)
		return false;

	dsGLTexture_addInternalRef(srcTexture);
	dsGLTexture_addInternalRef(dstTexture);
	command->srcTexture = srcTexture;
	command->dstTexture = dstTexture;
	command->regionCount = regionCount;
	memcpy(command->regions, regions, sizeof(dsTextureCopyRegion)*regionCount);
	return true;
}

bool dsGLOtherCommandBuffer_generateTextureMipmaps(dsCommandBuffer* commandBuffer,
	dsTexture* texture)
{
	GenerateTextureMipmapsCommand* command = (GenerateTextureMipmapsCommand*)allocateCommand(
		commandBuffer, CommandType_GenerateTextureMipmaps, sizeof(GenerateTextureMipmapsCommand));
	if (!command)
		return false;

	dsGLTexture_addInternalRef(texture);
	command->texture = texture;
	return true;
}

bool dsGLOtherCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback)
{
	dsGLOtherCommandBuffer* glCommandBuffer = (dsGLOtherCommandBuffer*)commandBuffer;
	size_t index = glCommandBuffer->curFenceSyncs;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, glCommandBuffer->fenceSyncs,
		glCommandBuffer->curFenceSyncs, glCommandBuffer->maxFenceSyncs, syncCount))
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

bool dsGLOtherCommandBuffer_beginQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	BeginEndQueryCommand* command = (BeginEndQueryCommand*)allocateCommand(commandBuffer,
		CommandType_BeginQuery, sizeof(BeginEndQueryCommand));
	if (!command)
		return false;

	dsGLGfxQueryPool_addInternalRef(queries);
	command->queries = queries;
	command->query = query;
	return true;
}

bool dsGLOtherCommandBuffer_endQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	BeginEndQueryCommand* command = (BeginEndQueryCommand*)allocateCommand(commandBuffer,
		CommandType_EndQuery, sizeof(BeginEndQueryCommand));
	if (!command)
		return false;

	dsGLGfxQueryPool_addInternalRef(queries);
	command->queries = queries;
	command->query = query;
	return true;
}

bool dsGLOtherCommandBuffer_queryTimestamp(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	QueryTimestampCommand* command = (QueryTimestampCommand*)allocateCommand(commandBuffer,
		CommandType_QueryTimestamp, sizeof(QueryTimestampCommand));
	if (!command)
		return false;

	dsGLGfxQueryPool_addInternalRef(queries);
	command->queries = queries;
	command->query = query;
	return true;
}

bool dsGLOtherCommandBuffer_copyQueryValues(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability)
{
	CopyQueryValuesCommand* command = (CopyQueryValuesCommand*)allocateCommand(commandBuffer,
		CommandType_CopyQueryValues, sizeof(CopyQueryValuesCommand));
	if (!command)
		return false;

	dsGLGfxQueryPool_addInternalRef(queries);
	dsGLGfxBuffer_addInternalRef(buffer);
	command->queries = queries;
	command->first = first;
	command->count = count;
	command->buffer = buffer;
	command->offset = offset;
	command->stride = stride;
	command->elementSize = elementSize;
	command->checkAvailability = checkAvailability;
	return true;
}

bool dsGLOtherCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	BindShaderCommand* command = (BindShaderCommand*)allocateCommand(commandBuffer,
		CommandType_BindShader, sizeof(BindShaderCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	command->shader = shader;
	command->renderStates = *renderStates;
	return true;
}

bool dsGLOtherCommandBuffer_setTexture(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture)
{
	SetTextureCommand* command = (SetTextureCommand*)allocateCommand(commandBuffer,
		CommandType_SetTexture, sizeof(SetTextureCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	dsGLTexture_addInternalRef(texture);
	command->shader = shader;
	command->texture = texture;
	command->element = element;
	return true;
}

bool dsGLOtherCommandBuffer_setTextureBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	SetTextureBufferCommand* command = (SetTextureBufferCommand*)allocateCommand(commandBuffer,
		CommandType_SetTextureBuffer, sizeof(SetTextureBufferCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	dsGLGfxBuffer_addInternalRef(buffer);
	command->shader = shader;
	command->buffer = buffer;
	command->element = element;
	command->format = format;
	command->offset = offset;
	command->count = count;
	return true;
}

bool dsGLOtherCommandBuffer_setShaderBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size)
{
	SetShaderBufferCommand* command = (SetShaderBufferCommand*)allocateCommand(commandBuffer,
		CommandType_SetShaderBuffer, sizeof(SetShaderBufferCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	dsGLGfxBuffer_addInternalRef(buffer);
	command->shader = shader;
	command->buffer = buffer;
	command->element = element;
	command->offset = offset;
	command->size = size;
	return true;
}

bool dsGLOtherCommandBuffer_setUniform(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data)
{
	size_t dataSize = dsMaterialType_cpuSize(type)*dsMax(1U, count);
	size_t commandSize = sizeof(SetUniformCommand) + dataSize;
	SetUniformCommand* command = (SetUniformCommand*)allocateCommand(commandBuffer,
		CommandType_SetUniform, commandSize);
	if (!command)
		return false;

	command->location = location;
	command->type = type;
	command->count = count;
	memcpy(command->data, data, dataSize);
	return true;
}

bool dsGLOtherCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	UnbindShaderCommand* command = (UnbindShaderCommand*)allocateCommand(commandBuffer,
		CommandType_UnbindShader, sizeof(UnbindShaderCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	command->shader = shader;
	return true;
}

bool dsGLOtherCommandBuffer_bindComputeShader(dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	BindComputeShaderCommand* command = (BindComputeShaderCommand*)allocateCommand(commandBuffer,
		CommandType_BindComputeShader, sizeof(BindComputeShaderCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	command->shader = shader;
	return true;
}

bool dsGLOtherCommandBuffer_unbindComputeShader(dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	UnbindShaderCommand* command = (UnbindShaderCommand*)allocateCommand(commandBuffer,
		CommandType_UnbindComputeShader, sizeof(UnbindShaderCommand));
	if (!command)
		return false;

	dsGLShader_addInternalRef((dsShader*)shader);
	command->shader = shader;
	return true;
}

bool dsGLOtherCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	RenderSurfaceCommand* command = (RenderSurfaceCommand*)allocateCommand(commandBuffer,
		CommandType_BeginRenderSurface, sizeof(RenderSurfaceCommand));
	if (!command)
		return false;

	command->glSurface = glSurface;
	return true;
}

bool dsGLOtherCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	RenderSurfaceCommand* command = (RenderSurfaceCommand*)allocateCommand(commandBuffer,
		CommandType_EndRenderSurface, sizeof(RenderSurfaceCommand));
	if (!command)
		return false;

	command->glSurface = glSurface;
	return true;
}

bool dsGLOtherCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	size_t commandSize = sizeof(BeginRenderPassCommand) +
		sizeof(dsSurfaceClearValue)*clearValueCount;
	BeginRenderPassCommand* command = (BeginRenderPassCommand*)allocateCommand(commandBuffer,
		CommandType_BeginRenderPass, commandSize);
	if (!command)
		return false;

	dsGLRenderPass_addInternalRef((dsRenderPass*)renderPass);
	dsGLFramebuffer_addInternalRef((dsFramebuffer*)framebuffer);
	command->renderPass = renderPass;
	command->framebuffer = framebuffer;
	if (viewport)
	{
		command->viewport = *viewport;
		command->viewportSet = true;
	}
	else
		command->viewportSet = false;
	command->clearValueCount = clearValueCount;
	memcpy(command->clearValues, clearValues, sizeof(dsSurfaceClearValue)*clearValueCount);
	return true;
}

bool dsGLOtherCommandBuffer_nextRenderSubpass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex)
{
	NextRenderSubpassCommand* command = (NextRenderSubpassCommand*)allocateCommand(commandBuffer,
		CommandType_NextRenderSubpass, sizeof(NextRenderSubpassCommand));
	if (!command)
		return false;

	dsGLRenderPass_addInternalRef((dsRenderPass*)renderPass);
	command->renderPass = renderPass;
	command->subpassIndex = subpassIndex;
	return true;
}

bool dsGLOtherCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	EndRenderPassCommand* command = (EndRenderPassCommand*)allocateCommand(commandBuffer,
		CommandType_EndRenderPass, sizeof(EndRenderPassCommand));
	if (!command)
		return false;

	dsGLRenderPass_addInternalRef((dsRenderPass*)renderPass);
	command->renderPass = renderPass;
	return true;
}

bool dsGLOtherCommandBuffer_clearColorSurface(dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, const dsSurfaceColorValue* colorValue)
{
	ClearColorSurfaceCommand* command = (ClearColorSurfaceCommand*)allocateCommand(commandBuffer,
		CommandType_ClearColorSurface, sizeof(ClearColorSurfaceCommand));
	if (!command)
		return false;

	command->surface = *surface;
	if (surface->surfaceType == dsGfxSurfaceType_Texture)
		dsGLTexture_addInternalRef((dsTexture*)surface->surface);
	else if (surface->surfaceType == dsGfxSurfaceType_Renderbuffer)
		dsGLRenderbuffer_addInternalRef((dsRenderbuffer*)surface->surface);
	command->value = *colorValue;
	return true;
}

bool dsGLOtherCommandBuffer_clearDepthStencilSurface(dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, dsClearDepthStencil surfaceParts,
	const dsDepthStencilValue* depthStencilValue)
{
	ClearDepthStencilSurfaceCommand* command = (ClearDepthStencilSurfaceCommand*)allocateCommand(
		commandBuffer, CommandType_ClearDepthStencilSurface,
		sizeof(ClearDepthStencilSurfaceCommand));
	if (!command)
		return false;

	command->surface = *surface;
	if (surface->surfaceType == dsGfxSurfaceType_Texture)
		dsGLTexture_addInternalRef((dsTexture*)surface->surface);
	else if (surface->surfaceType == dsGfxSurfaceType_Renderbuffer)
		dsGLRenderbuffer_addInternalRef((dsRenderbuffer*)surface->surface);
	command->surfaceParts = surfaceParts;
	command->value = *depthStencilValue;
	return true;
}

bool dsGLOtherCommandBuffer_draw(dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsDrawRange* drawRange)
{
	DrawCommand* command = (DrawCommand*)allocateCommand(commandBuffer, CommandType_Draw,
		sizeof(DrawCommand));
	if (!command)
		return false;

	dsGLDrawGeometry_addInternalRef((dsDrawGeometry*)geometry);
	command->geometry = geometry;
	command->drawRange = *drawRange;
	return true;
}

bool dsGLOtherCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange)
{
	DrawIndexedCommand* command = (DrawIndexedCommand*)allocateCommand(commandBuffer,
		CommandType_DrawIndexed, sizeof(DrawIndexedCommand));
	if (!command)
		return false;

	dsGLDrawGeometry_addInternalRef((dsDrawGeometry*)geometry);
	command->geometry = geometry;
	command->drawRange = *drawRange;
	return true;
}

bool dsGLOtherCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DrawIndirectCommand* command = (DrawIndirectCommand*)allocateCommand(commandBuffer,
		CommandType_DrawIndirect, sizeof(DrawIndirectCommand));
	if (!command)
		return false;

	dsGLDrawGeometry_addInternalRef((dsDrawGeometry*)geometry);
	dsGLGfxBuffer_addInternalRef((dsGfxBuffer*)indirectBuffer);
	command->geometry = geometry;
	command->indirectBuffer = indirectBuffer;
	command->offset = offset;
	command->count = count;
	command->stride = stride;
	return true;
}

bool dsGLOtherCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DrawIndirectCommand* command = (DrawIndirectCommand*)allocateCommand(commandBuffer,
		CommandType_DrawIndexedIndirect, sizeof(DrawIndirectCommand));
	if (!command)
		return false;

	dsGLDrawGeometry_addInternalRef((dsDrawGeometry*)geometry);
	dsGLGfxBuffer_addInternalRef((dsGfxBuffer*)indirectBuffer);
	command->geometry = geometry;
	command->indirectBuffer = indirectBuffer;
	command->offset = offset;
	command->count = count;
	command->stride = stride;
	return true;
}

bool dsGLOtherCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer, uint32_t x, uint32_t y,
	uint32_t z)
{
	DispatchComputeCommand* command = (DispatchComputeCommand*)allocateCommand(commandBuffer,
		CommandType_DispatchCompute, sizeof(DispatchComputeCommand));
	if (!command)
		return false;

	command->x = x;
	command->y = y;
	command->z = z;
	return true;
}

bool dsGLOtherCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	DispatchComputeIndirectCommand* command = (DispatchComputeIndirectCommand*)allocateCommand(
		commandBuffer, CommandType_DispatchComputeIndirect, sizeof(DispatchComputeIndirectCommand));
	if (!command)
		return false;

	dsGLGfxBuffer_addInternalRef((dsGfxBuffer*)indirectBuffer);
	command->indirectBuffer = indirectBuffer;
	command->offset = offset;
	return true;
}

bool dsGLOtherCommandBuffer_blitSurface(dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter)
{
	size_t commandSize = sizeof(BlitSurfaceCommand) + sizeof(dsSurfaceBlitRegion)*regionCount;
	BlitSurfaceCommand* command = (BlitSurfaceCommand*)allocateCommand(commandBuffer,
		CommandType_BlitSurface, commandSize);
	if (!command)
		return false;

	addSurfaceRef(srcSurfaceType, srcSurface);
	addSurfaceRef(dstSurfaceType, dstSurface);
	command->srcSurface = srcSurface;
	command->dstSurface = dstSurface;
	command->srcSurfaceType = srcSurfaceType;
	command->dstSurfaceType = dstSurfaceType;
	command->filter = filter;
	command->regionCount = regionCount;
	memcpy(command->regions, regions, sizeof(dsSurfaceBlitRegion)*regionCount);
	return true;
}

bool dsGLOtherCommandBuffer_begin(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	uint32_t subpassIndex, const dsFramebuffer* framebuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpassIndex);
	DS_UNUSED(framebuffer);
	return true;
}

bool dsGLOtherCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
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
			case CommandType_CopyTextureData:
			{
				CopyTextureDataCommand* thisCommand = (CopyTextureDataCommand*)command;
				dsGLCommandBuffer_copyTextureData(commandBuffer, thisCommand->texture,
					&thisCommand->position, thisCommand->width, thisCommand->height,
					thisCommand->layers, thisCommand->data, thisCommand->size);
				break;
			}
			case CommandType_CopyTexture:
			{
				CopyTextureCommand* thisCommand = (CopyTextureCommand*)command;
				dsGLCommandBuffer_copyTexture(commandBuffer, thisCommand->srcTexture,
					thisCommand->dstTexture, thisCommand->regions, thisCommand->regionCount);
				break;
			}
			case CommandType_GenerateTextureMipmaps:
			{
				GenerateTextureMipmapsCommand* thisCommand =
					(GenerateTextureMipmapsCommand*)command;
				dsGLCommandBuffer_generateTextureMipmaps(commandBuffer, thisCommand->texture);
				break;
			}
			case CommandType_BeginQuery:
			{
				BeginEndQueryCommand* thisCommand = (BeginEndQueryCommand*)command;
				dsGLCommandBuffer_beginQuery(commandBuffer, thisCommand->queries,
					thisCommand->query);
				break;
			}
			case CommandType_EndQuery:
			{
				BeginEndQueryCommand* thisCommand = (BeginEndQueryCommand*)command;
				dsGLCommandBuffer_endQuery(commandBuffer, thisCommand->queries,
					thisCommand->query);
				break;
			}
			case CommandType_QueryTimestamp:
			{
				QueryTimestampCommand* thisCommand = (QueryTimestampCommand*)command;
				dsGLCommandBuffer_queryTimestamp(commandBuffer, thisCommand->queries,
					thisCommand->query);
				break;
			}
			case CommandType_CopyQueryValues:
			{
				CopyQueryValuesCommand* thisCommand = (CopyQueryValuesCommand*)command;
				dsGLCommandBuffer_copyQueryValues(commandBuffer, thisCommand->queries,
					thisCommand->first, thisCommand->count, thisCommand->buffer,
					thisCommand->offset, thisCommand->stride, thisCommand->elementSize,
					thisCommand->checkAvailability);
				break;
			}
			case CommandType_BindShader:
			{
				BindShaderCommand* thisCommand = (BindShaderCommand*)command;
				dsGLCommandBuffer_bindShader(commandBuffer, thisCommand->shader,
					&thisCommand->renderStates);
				break;
			}
			case CommandType_SetTexture:
			{
				SetTextureCommand* thisCommand = (SetTextureCommand*)command;
				dsGLCommandBuffer_setTexture(commandBuffer, thisCommand->shader,
					thisCommand->element, thisCommand->texture);
				break;
			}
			case CommandType_SetTextureBuffer:
			{
				SetTextureBufferCommand* thisCommand = (SetTextureBufferCommand*)command;
				dsGLCommandBuffer_setTextureBuffer(commandBuffer, thisCommand->shader,
					thisCommand->element, thisCommand->buffer, thisCommand->format,
					thisCommand->offset, thisCommand->count);
				break;
			}
			case CommandType_SetShaderBuffer:
			{
				SetShaderBufferCommand* thisCommand = (SetShaderBufferCommand*)command;
				dsGLCommandBuffer_setShaderBuffer(commandBuffer, thisCommand->shader,
					thisCommand->element, thisCommand->buffer, thisCommand->offset,
					thisCommand->size);
			}
			case CommandType_SetUniform:
			{
				SetUniformCommand* thisCommand = (SetUniformCommand*)command;
				dsGLCommandBuffer_setUniform(commandBuffer, thisCommand->location,
					thisCommand->type, thisCommand->count, thisCommand->data);
				break;
			}
			case CommandType_UnbindShader:
			{
				UnbindShaderCommand* thisCommand = (UnbindShaderCommand*)command;
				dsGLCommandBuffer_unbindShader(commandBuffer, thisCommand->shader);
				break;
			}
			case CommandType_BindComputeShader:
			{
				BindComputeShaderCommand* thisCommand = (BindComputeShaderCommand*)command;
				dsGLCommandBuffer_bindComputeShader(commandBuffer, thisCommand->shader);
				break;
			}
			case CommandType_UnbindComputeShader:
			{
				UnbindShaderCommand* thisCommand = (UnbindShaderCommand*)command;
				dsGLCommandBuffer_unbindComputeShader(commandBuffer, thisCommand->shader);
				break;
			}
			case CommandType_BeginRenderSurface:
			{
				RenderSurfaceCommand* thisCommand = (RenderSurfaceCommand*)command;
				dsGLCommandBuffer_beginRenderSurface(commandBuffer, thisCommand->glSurface);
				break;
			}
			case CommandType_EndRenderSurface:
			{
				RenderSurfaceCommand* thisCommand = (RenderSurfaceCommand*)command;
				dsGLCommandBuffer_endRenderSurface(commandBuffer, thisCommand->glSurface);
				break;
			}
			case CommandType_BeginRenderPass:
			{
				BeginRenderPassCommand* thisCommand = (BeginRenderPassCommand*)command;
				dsAlignedBox3f* viewport = NULL;
				if (thisCommand->viewportSet)
					viewport = &thisCommand->viewport;
				dsGLCommandBuffer_beginRenderPass(commandBuffer, thisCommand->renderPass,
					thisCommand->framebuffer, viewport, thisCommand->clearValues,
					thisCommand->clearValueCount);
				break;
			}
			case CommandType_NextRenderSubpass:
			{
				NextRenderSubpassCommand* thisCommand = (NextRenderSubpassCommand*)command;
				dsGLCommandBuffer_nextRenderSubpass(commandBuffer, thisCommand->renderPass,
					thisCommand->subpassIndex);
				break;
			}
			case CommandType_EndRenderPass:
			{
				EndRenderPassCommand* thisCommand = (EndRenderPassCommand*)command;
				dsGLCommandBuffer_endRenderPass(commandBuffer, thisCommand->renderPass);
				break;
			}
			case CommandType_ClearColorSurface:
			{
				ClearColorSurfaceCommand* thisCommand = (ClearColorSurfaceCommand*)command;
				dsGLCommandBuffer_clearColorSurface(commandBuffer->renderer, commandBuffer,
					&thisCommand->surface, &thisCommand->value);
				break;
			}
			case CommandType_ClearDepthStencilSurface:
			{
				ClearDepthStencilSurfaceCommand* thisCommand =
					(ClearDepthStencilSurfaceCommand*)command;
				dsGLCommandBuffer_clearDepthStencilSurface(commandBuffer->renderer, commandBuffer,
					&thisCommand->surface, thisCommand->surfaceParts, &thisCommand->value);
				break;
			}
			case CommandType_Draw:
			{
				DrawCommand* thisCommand = (DrawCommand*)command;
				dsGLCommandBuffer_draw(commandBuffer->renderer, commandBuffer,
					thisCommand->geometry, &thisCommand->drawRange);
				break;
			}
			case CommandType_DrawIndexed:
			{
				DrawIndexedCommand* thisCommand = (DrawIndexedCommand*)command;
				dsGLCommandBuffer_drawIndexed(commandBuffer->renderer, commandBuffer,
					thisCommand->geometry, &thisCommand->drawRange);
				break;
			}
			case CommandType_DrawIndirect:
			{
				DrawIndirectCommand* thisCommand = (DrawIndirectCommand*)command;
				dsGLCommandBuffer_drawIndirect(commandBuffer->renderer, commandBuffer,
					thisCommand->geometry, thisCommand->indirectBuffer, thisCommand->offset,
					thisCommand->count, thisCommand->stride);
				break;
			}
			case CommandType_DrawIndexedIndirect:
			{
				DrawIndirectCommand* thisCommand = (DrawIndirectCommand*)command;
				dsGLCommandBuffer_drawIndexedIndirect(commandBuffer->renderer, commandBuffer,
					thisCommand->geometry, thisCommand->indirectBuffer, thisCommand->offset,
					thisCommand->count, thisCommand->stride);
				break;
			}
			case CommandType_DispatchCompute:
			{
				DispatchComputeCommand* thisCommand = (DispatchComputeCommand*)command;
				dsGLCommandBuffer_dispatchCompute(commandBuffer->renderer, commandBuffer,
					thisCommand->x, thisCommand->y, thisCommand->z);
				break;
			}
			case CommandType_DispatchComputeIndirect:
			{
				DispatchComputeIndirectCommand* thisCommand =
					(DispatchComputeIndirectCommand*)command;
				dsGLCommandBuffer_dispatchComputeIndirect(commandBuffer->renderer, commandBuffer,
					thisCommand->indirectBuffer, thisCommand->offset);
				break;
			}
			case CommandType_BlitSurface:
			{
				BlitSurfaceCommand* thisCommand = (BlitSurfaceCommand*)command;
				dsGLCommandBuffer_blitSurface(commandBuffer->renderer, commandBuffer,
					thisCommand->srcSurfaceType, thisCommand->srcSurface,
					thisCommand->dstSurfaceType, thisCommand->dstSurface, thisCommand->regions,
					thisCommand->regionCount, thisCommand->filter);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	if (glSubmitBuffer->curFenceSyncs > 0)
	{
		dsGLCommandBuffer_setFenceSyncs(commandBuffer, glSubmitBuffer->fenceSyncs,
			glSubmitBuffer->curFenceSyncs, glSubmitBuffer->bufferReadback);
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
	&dsGLOtherCommandBuffer_copyTextureData,
	&dsGLOtherCommandBuffer_copyTexture,
	&dsGLOtherCommandBuffer_generateTextureMipmaps,
	&dsGLOtherCommandBuffer_setFenceSyncs,
	&dsGLOtherCommandBuffer_beginQuery,
	&dsGLOtherCommandBuffer_endQuery,
	&dsGLOtherCommandBuffer_queryTimestamp,
	&dsGLOtherCommandBuffer_copyQueryValues,
	&dsGLOtherCommandBuffer_bindShader,
	&dsGLOtherCommandBuffer_setTexture,
	&dsGLOtherCommandBuffer_setTextureBuffer,
	&dsGLOtherCommandBuffer_setShaderBuffer,
	&dsGLOtherCommandBuffer_setUniform,
	&dsGLOtherCommandBuffer_unbindShader,
	&dsGLOtherCommandBuffer_bindComputeShader,
	&dsGLOtherCommandBuffer_unbindComputeShader,
	&dsGLOtherCommandBuffer_beginRenderSurface,
	&dsGLOtherCommandBuffer_endRenderSurface,
	&dsGLOtherCommandBuffer_beginRenderPass,
	&dsGLOtherCommandBuffer_nextRenderSubpass,
	&dsGLOtherCommandBuffer_endRenderPass,
	&dsGLOtherCommandBuffer_clearColorSurface,
	&dsGLOtherCommandBuffer_clearDepthStencilSurface,
	&dsGLOtherCommandBuffer_draw,
	&dsGLOtherCommandBuffer_drawIndexed,
	&dsGLOtherCommandBuffer_drawIndirect,
	&dsGLOtherCommandBuffer_drawIndexedIndirect,
	&dsGLOtherCommandBuffer_dispatchCompute,
	&dsGLOtherCommandBuffer_dispatchComputeIndirect,
	&dsGLOtherCommandBuffer_blitSurface,
	&dsGLOtherCommandBuffer_begin,
	&dsGLOtherCommandBuffer_end,
	&dsGLOtherCommandBuffer_submit
};

dsGLOtherCommandBuffer* dsGLOtherCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Command buffer allocator must support freeing memory.");
		return NULL;
	}

	dsGLOtherCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(allocator, dsGLOtherCommandBuffer);
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;
	baseCommandBuffer->frameActive = true;
	baseCommandBuffer->boundSurface = NULL;
	baseCommandBuffer->boundFramebuffer = NULL;
	baseCommandBuffer->boundRenderPass = NULL;
	baseCommandBuffer->activeRenderSubpass = 0;
	baseCommandBuffer->indirectCommands = false;
	baseCommandBuffer->boundShader = NULL;
	baseCommandBuffer->boundComputeShader = NULL;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;
	commandBuffer->fenceSyncs = NULL;
	commandBuffer->curFenceSyncs = 0;
	commandBuffer->maxFenceSyncs = 0;
	commandBuffer->bufferReadback = false;

	const size_t defaultBufferSize = 512*1024;
	void* bufferData = dsAllocator_alloc(allocator, defaultBufferSize);
	if (!bufferData)
	{
		dsAllocator_free(allocator, commandBuffer);
		return NULL;
	}

	DS_VERIFY(dsBufferAllocator_initialize(&commandBuffer->buffer, bufferData, defaultBufferSize));
	dsGLCommandBuffer_initialize(baseCommandBuffer, (usage & dsCommandBufferUsage_Subpass) != 0);
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
			case CommandType_CopyTextureData:
			{
				CopyTextureDataCommand* thisCommand = (CopyTextureDataCommand*)command;
				dsGLTexture_freeInternalRef(thisCommand->texture);
				break;
			}
			case CommandType_CopyTexture:
			{
				CopyTextureCommand* thisCommand = (CopyTextureCommand*)command;
				dsGLTexture_freeInternalRef(thisCommand->srcTexture);
				dsGLTexture_freeInternalRef(thisCommand->dstTexture);
				break;
			}
			case CommandType_GenerateTextureMipmaps:
			{
				GenerateTextureMipmapsCommand* thisCommand =
					(GenerateTextureMipmapsCommand*)command;
				dsGLTexture_freeInternalRef(thisCommand->texture);
				break;
			}
			case CommandType_BeginQuery:
			case CommandType_EndQuery:
			{
				BeginEndQueryCommand* thisCommand = (BeginEndQueryCommand*)command;
				dsGLGfxQueryPool_freeInternalRef(thisCommand->queries);
				break;
			}
			case CommandType_QueryTimestamp:
			{
				QueryTimestampCommand* thisCommand = (QueryTimestampCommand*)command;
				dsGLGfxQueryPool_freeInternalRef(thisCommand->queries);
				break;
			}
			case CommandType_CopyQueryValues:
			{
				CopyQueryValuesCommand* thisCommand = (CopyQueryValuesCommand*)command;
				dsGLGfxQueryPool_freeInternalRef(thisCommand->queries);
				dsGLGfxBuffer_freeInternalRef(thisCommand->buffer);
				break;
			}
			case CommandType_BindShader:
			{
				BindShaderCommand* thisCommand = (BindShaderCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				break;
			}
			case CommandType_SetTexture:
			{
				SetTextureCommand* thisCommand = (SetTextureCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				dsGLTexture_freeInternalRef(thisCommand->texture);
				break;
			}
			case CommandType_SetTextureBuffer:
			{
				SetTextureBufferCommand* thisCommand = (SetTextureBufferCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				dsGLGfxBuffer_freeInternalRef(thisCommand->buffer);
				break;
			}
			case CommandType_SetShaderBuffer:
			{
				SetShaderBufferCommand* thisCommand = (SetShaderBufferCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				dsGLGfxBuffer_freeInternalRef(thisCommand->buffer);
				break;
			}
			case CommandType_SetUniform:
				break;
			case CommandType_UnbindShader:
			case CommandType_UnbindComputeShader:
			{
				UnbindShaderCommand* thisCommand = (UnbindShaderCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				break;
			}
			case CommandType_BindComputeShader:
			{
				BindComputeShaderCommand* thisCommand = (BindComputeShaderCommand*)command;
				dsGLShader_freeInternalRef((dsShader*)thisCommand->shader);
				break;
			}
			case CommandType_BeginRenderSurface:
				break;
			case CommandType_EndRenderSurface:
				break;
			case CommandType_BeginRenderPass:
			{
				BeginRenderPassCommand* thisCommand = (BeginRenderPassCommand*)command;
				dsGLRenderPass_freeInternalRef((dsRenderPass*)thisCommand->renderPass);
				dsGLFramebuffer_freeInternalRef((dsFramebuffer*)thisCommand->framebuffer);
				break;
			}
			case CommandType_NextRenderSubpass:
			{
				NextRenderSubpassCommand* thisCommand = (NextRenderSubpassCommand*)command;
				dsGLRenderPass_freeInternalRef((dsRenderPass*)thisCommand->renderPass);
				break;
			}
			case CommandType_EndRenderPass:
			{
				EndRenderPassCommand* thisCommand = (EndRenderPassCommand*)command;
				dsGLRenderPass_freeInternalRef((dsRenderPass*)thisCommand->renderPass);
				break;
			}
			case CommandType_ClearColorSurface:
			{
				ClearColorSurfaceCommand* thisCommand = (ClearColorSurfaceCommand*)command;
				if (thisCommand->surface.surfaceType == dsGfxSurfaceType_Texture)
					dsGLTexture_freeInternalRef((dsTexture*)thisCommand->surface.surface);
				else if (thisCommand->surface.surfaceType == dsGfxSurfaceType_Renderbuffer)
					dsGLRenderbuffer_freeInternalRef((dsRenderbuffer*)thisCommand->surface.surface);
				break;
			}
			case CommandType_ClearDepthStencilSurface:
			{
				ClearDepthStencilSurfaceCommand* thisCommand =
					(ClearDepthStencilSurfaceCommand*)command;
				if (thisCommand->surface.surfaceType == dsGfxSurfaceType_Texture)
					dsGLTexture_freeInternalRef((dsTexture*)thisCommand->surface.surface);
				else if (thisCommand->surface.surfaceType == dsGfxSurfaceType_Renderbuffer)
					dsGLRenderbuffer_freeInternalRef((dsRenderbuffer*)thisCommand->surface.surface);
				break;
			}
			case CommandType_Draw:
			{
				DrawCommand* thisCommand = (DrawCommand*)command;
				dsGLDrawGeometry_freeInternalRef((dsDrawGeometry*)thisCommand->geometry);
				break;
			}
			case CommandType_DrawIndexed:
			{
				DrawIndexedCommand* thisCommand = (DrawIndexedCommand*)command;
				dsGLDrawGeometry_freeInternalRef((dsDrawGeometry*)thisCommand->geometry);
				break;
			}
			case CommandType_DrawIndirect:
			case CommandType_DrawIndexedIndirect:
			{
				DrawIndirectCommand* thisCommand = (DrawIndirectCommand*)command;
				dsGLDrawGeometry_freeInternalRef((dsDrawGeometry*)thisCommand->geometry);
				dsGLGfxBuffer_freeInternalRef((dsGfxBuffer*)thisCommand->indirectBuffer);
				break;
			}
			case CommandType_DispatchCompute:
				break;
			case CommandType_DispatchComputeIndirect:
			{
				DispatchComputeIndirectCommand* thisCommand =
					(DispatchComputeIndirectCommand*)command;
				dsGLGfxBuffer_freeInternalRef((dsGfxBuffer*)thisCommand->indirectBuffer);
				break;
			}
			case CommandType_BlitSurface:
			{
				BlitSurfaceCommand* thisCommand = (BlitSurfaceCommand*)command;
				freeSurfaceRef(thisCommand->srcSurfaceType, thisCommand->srcSurface);
				freeSurfaceRef(thisCommand->dstSurfaceType, thisCommand->dstSurface);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	for (size_t i = 0; i < glCommandBuffer->curFenceSyncs; ++i)
		dsGLFenceSyncRef_freeRef(glCommandBuffer->fenceSyncs[i]);
	glCommandBuffer->curFenceSyncs = 0;
	glCommandBuffer->bufferReadback = false;

	DS_VERIFY(dsBufferAllocator_reset(&commandBuffer->buffer));
}

bool dsGLOtherCommandBuffer_destroy(dsGLOtherCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	dsGLOtherCommandBuffer_reset(commandBuffer);
	dsGLCommandBuffer_shutdown((dsCommandBuffer*)commandBuffer);

	DS_ASSERT(commandBuffer->curFenceSyncs == 0);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fenceSyncs));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->buffer.buffer));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer));
	return true;
}
