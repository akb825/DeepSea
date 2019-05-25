/*
 * Copyright 2019 Aaron Barany
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

#include "MTLSoftwareCommandBuffer.h"

#include "MTLCommandBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

typedef enum CommandType
{
	CommandType_CopyBuffer,
	CommandType_CopyTexture,
	CommandType_GenerateMipmaps,
	CommandType_BindPushConstants,
	CommandType_BindBufferUniform,
	CommandType_BindTextureUniform,
	CommandType_SetRenderStates,
	CommandType_BeginComputeShader,
	CommandType_BindComputePushConstants,
	CommandType_BindComputeBufferUniform,
	CommandType_BindComputeTextureUniform,
	CommandType_BeginRenderPass,
	CommandType_EndRenderPass,
	CommandType_ClearColorSurface,
	CommandType_ClearDepthStencilSurface,
	CommandType_Draw,
	CommandType_DrawIndexed,
	CommandType_DrawIndirect,
	CommandType_DrawIndexedIndirect,
	CommandType_DispatchCompute,
	CommandType_DispatchComputeIndirect
} CommandType;

typedef struct Command
{
	CommandType type;
	uint32_t size;
} Command;

typedef struct CopyBufferCommand
{
	Command command;
	CFTypeRef srcBuffer;
	CFTypeRef dstBuffer;
	size_t srcOffset;
	size_t dstOffset;
	size_t size;
} CopyBufferCommand;

typedef struct CopyTextureCommand
{
	Command command;
	CFTypeRef srcTexture;
	CFTypeRef dstTexture;
	uint32_t regionCount;
	dsTextureCopyRegion regions[];
} CopyTextureCommand;

typedef struct GenerateMipmapsCommand
{
	Command command;
	CFTypeRef texture;
} GenerateMipmapsCommand;

typedef struct BindPushConstantsCommand
{
	Command command;
	bool vertex;
	bool fragment;
	uint32_t size;
	uint8_t data[];
} BindPushConstantsCommand;

typedef struct BindBufferUniformCommand
{
	Command command;
	CFTypeRef buffer;
	size_t offset;
	uint32_t vertexIndex;
	uint32_t fragmentIndex;
} BindBufferUniformCommand;

typedef struct BindTextureUniformCommand
{
	Command command;
	CFTypeRef texture;
	CFTypeRef sampler;
	size_t offset;
	uint32_t vertexIndex;
	uint32_t fragmentIndex;
} BindTextureUniformCommand;

typedef struct SetRenderStatesCommand
{
	Command command;
	mslRenderState renderStates;
	CFTypeRef depthStencilState;
	dsDynamicRenderStates dynamicStates;
	bool hasDynamicStates;
	bool dynamicOnly;
} SetRenderStatesCommand;

typedef struct BindComputePushConstantsCommand
{
	Command command;
	uint32_t size;
	uint8_t data[];
} BindComputePushConstantsCommand;

typedef struct BindComputeBufferUniformCommand
{
	Command command;
	CFTypeRef buffer;
	size_t offset;
	uint32_t index;
} BindComputeBufferUniformCommand;

typedef struct BindComputeTextureUniformCommand
{
	Command command;
	CFTypeRef texture;
	CFTypeRef sampler;
	size_t offset;
	uint32_t index;
} BindComputeTextureUniformCommand;

typedef struct BeginRenderPassCommand
{
	Command command;
	CFTypeRef renderPass;
	dsAlignedBox3f viewport;
} BeginRenderPassCommand;

typedef struct ClearColorSurfaceCommand
{
	Command command;
	CFTypeRef texture;
	CFTypeRef resolveTexture;
	MTLClearColor clearColor;
} ClearColorSurfaceCommand;

typedef struct ClearDepthStencilSurfaceCommand
{
	Command command;
	CFTypeRef depthTexture;
	CFTypeRef resolveDepthTexture;
	CFTypeRef stencilTexture;
	CFTypeRef resolveStencilTexture;
	float depthValue;
	uint32_t stencilValue;
} ClearDepthStencilSurfaceCommand;

typedef struct DrawCommand
{
	Command command;
	CFTypeRef pipeline;
	dsDrawRange drawRange;
	dsPrimitiveType primitiveType;
} DrawCommand;

typedef struct DrawIndexedCommand
{
	Command command;
	CFTypeRef pipeline;
	CFTypeRef indexBuffer;
	size_t indexOffset;
	uint32_t indexSize;
	dsDrawIndexedRange drawRange;
	dsPrimitiveType primitiveType;
} DrawIndexedCommand;

typedef struct DrawIndirectCommand
{
	Command command;
	CFTypeRef pipeline;
	CFTypeRef indirectBuffer;
	size_t offset;
	uint32_t count;
	uint32_t stride;
	dsPrimitiveType primitiveType;
} DrawIndirectCommand;

typedef struct DrawIndexedIndirectCommand
{
	Command command;
	CFTypeRef pipeline;
	CFTypeRef indexBuffer;
	CFTypeRef indirectBuffer;
	size_t indexOffset;
	size_t indirectOffset;
	uint32_t indexSize;
	uint32_t count;
	uint32_t stride;
	dsPrimitiveType primitiveType;
} DrawIndexedIndirectCommand;

typedef struct DispatchComputeCommand
{
	Command command;
	CFTypeRef computePipeline;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t groupX;
	uint32_t groupY;
	uint32_t groupZ;
} DispatchComputeCommand;

typedef struct DispatchComputeIndirectCommand
{
	Command command;
	CFTypeRef computePipeline;
	CFTypeRef buffer;
	size_t offset;
	uint32_t groupX;
	uint32_t groupY;
	uint32_t groupZ;
} DispatchComputeIndirectCommand;

static Command* allocateCommand(dsCommandBuffer* commandBuffer, CommandType type, size_t size)
{
	DS_ASSERT(size >= sizeof(Command));
	dsMTLSoftwareCommandBuffer* mtlCommandBuffer = (dsMTLSoftwareCommandBuffer*)commandBuffer;
	int prevErrno = errno;
	Command* command = (Command*)dsAllocator_alloc(
		(dsAllocator*)&mtlCommandBuffer->commands, size);
	if (!command)
	{
		// Allocate a new buffer.
		errno = prevErrno;
		size_t newBufferSize = dsMax(mtlCommandBuffer->commands.bufferSize*2,
			mtlCommandBuffer->commands.bufferSize + size);
		void* newBuffer = dsAllocator_reallocWithFallback(commandBuffer->allocator,
			mtlCommandBuffer->commands.buffer, ((dsAllocator*)&mtlCommandBuffer->commands)->size,
			newBufferSize);
		if (!newBuffer)
			return NULL;

		DS_VERIFY(dsBufferAllocator_initialize(&mtlCommandBuffer->commands, newBuffer,
			newBufferSize));
		command = (Command*)dsAllocator_alloc((dsAllocator*)&mtlCommandBuffer->commands,
			size);
		DS_ASSERT(command);
	}

	command->type = type;
	command->size = (uint32_t)DS_ALIGNED_SIZE(size);
	return command;
}

void dsMTLSoftwareCommandBuffer_clear(dsCommandBuffer* commandBuffer)
{
	dsMTLSoftwareCommandBuffer* mtlCommandBuffer = (dsMTLSoftwareCommandBuffer*)commandBuffer;

	// Free any internal refs for resources.
	uint8_t* buffer = (uint8_t*)mtlCommandBuffer->commands.buffer;
	size_t bufferSize = ((dsAllocator*)&mtlCommandBuffer->commands)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		switch (command->type)
		{
			case CommandType_CopyBuffer:
			{
				CopyBufferCommand* thisCommand = (CopyBufferCommand*)command;
				CFRelease(thisCommand->srcBuffer);
				CFRelease(thisCommand->dstBuffer);
				break;
			}
			case CommandType_CopyTexture:
			{
				CopyTextureCommand* thisCommand = (CopyTextureCommand*)command;
				CFRelease(thisCommand->srcTexture);
				CFRelease(thisCommand->dstTexture);
				break;
			}
			case CommandType_GenerateMipmaps:
			{
				GenerateMipmapsCommand* thisCommand = (GenerateMipmapsCommand*)command;
				CFRelease(thisCommand->texture);
				break;
			}
			case CommandType_BindPushConstants:
				break;
			case CommandType_BindBufferUniform:
			{
				BindBufferUniformCommand* thisCommand = (BindBufferUniformCommand*)command;
				if (thisCommand->buffer)
					CFRelease(thisCommand->buffer);
				break;
			}
			case CommandType_BindTextureUniform:
			{
				BindTextureUniformCommand* thisCommand = (BindTextureUniformCommand*)command;
				if (thisCommand->texture)
					CFRelease(thisCommand->texture);
				if (thisCommand->sampler)
					CFRelease(thisCommand->sampler);
				break;
			}
			case CommandType_SetRenderStates:
				break;
			case CommandType_BeginComputeShader:
				break;
			case CommandType_BindComputePushConstants:
				break;
			case CommandType_BindComputeBufferUniform:
			{
				BindComputeBufferUniformCommand* thisCommand =
					(BindComputeBufferUniformCommand*)command;
				if (thisCommand->buffer)
					CFRelease(thisCommand->buffer);
				break;
			}
			case CommandType_BindComputeTextureUniform:
			{
				BindComputeTextureUniformCommand* thisCommand =
					(BindComputeTextureUniformCommand*)command;
				if (thisCommand->texture)
					CFRelease(thisCommand->texture);
				if (thisCommand->sampler)
					CFRelease(thisCommand->sampler);
				break;
			}
			case CommandType_BeginRenderPass:
			{
				BeginRenderPassCommand* thisCommand = (BeginRenderPassCommand*)command;
				CFRelease(thisCommand->renderPass);
				break;
			}
			case CommandType_EndRenderPass:
				break;
			case CommandType_ClearColorSurface:
			{
				ClearColorSurfaceCommand* thisCommand = (ClearColorSurfaceCommand*)command;
				CFRelease(thisCommand->texture);
				if (thisCommand->resolveTexture)
					CFRelease(thisCommand->resolveTexture);
				break;
			}
			case CommandType_ClearDepthStencilSurface:
			{
				ClearDepthStencilSurfaceCommand* thisCommand =
					(ClearDepthStencilSurfaceCommand*)command;
				if (thisCommand->depthTexture)
					CFRelease(thisCommand->depthTexture);
				if (thisCommand->resolveDepthTexture)
					CFRelease(thisCommand->resolveDepthTexture);
				if (thisCommand->stencilTexture)
					CFRelease(thisCommand->stencilTexture);
				if (thisCommand->resolveStencilTexture)
					CFRelease(thisCommand->resolveStencilTexture);
				break;
			}
			case CommandType_Draw:
			{
				DrawCommand* thisCommand = (DrawCommand*)command;
				CFRelease(thisCommand->pipeline);
				break;
			}
			case CommandType_DrawIndexed:
			{
				DrawIndexedCommand* thisCommand = (DrawIndexedCommand*)command;
				CFRelease(thisCommand->pipeline);
				CFRelease(thisCommand->indexBuffer);
				break;
			}
			case CommandType_DrawIndirect:
			{
				DrawIndirectCommand* thisCommand = (DrawIndirectCommand*)command;
				CFRelease(thisCommand->pipeline);
				CFRelease(thisCommand->indirectBuffer);
				break;
			}
			case CommandType_DrawIndexedIndirect:
			{
				DrawIndexedIndirectCommand* thisCommand = (DrawIndexedIndirectCommand*)command;
				CFRelease(thisCommand->pipeline);
				CFRelease(thisCommand->indirectBuffer);
				CFRelease(thisCommand->indexBuffer);
				break;
			}
			case CommandType_DispatchCompute:
			{
				DispatchComputeCommand* thisCommand = (DispatchComputeCommand*)command;
				CFRelease(thisCommand->computePipeline);
				break;
			}
			case CommandType_DispatchComputeIndirect:
			{
				DispatchComputeIndirectCommand* thisCommand =
					(DispatchComputeIndirectCommand*)command;
				CFRelease(thisCommand->computePipeline);
				CFRelease(thisCommand->buffer);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	DS_VERIFY(dsBufferAllocator_reset(&mtlCommandBuffer->commands));
}

void dsMTLSoftwareCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
}

bool dsMTLSoftwareCommandBuffer_submit(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	dsMTLSoftwareCommandBuffer* mtlSubmitBuffer = (dsMTLSoftwareCommandBuffer*)submitBuffer;

	uint8_t* buffer = (uint8_t*)mtlSubmitBuffer->commands.buffer;
	size_t bufferSize = ((dsAllocator*)&mtlSubmitBuffer->commands)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		bool result = true;
		switch (command->type)
		{
			case CommandType_CopyBuffer:
			{
				CopyBufferCommand* thisCommand = (CopyBufferCommand*)command;
				result = dsMTLCommandBuffer_copyBuffer(commandBuffer,
					(__bridge id<MTLBuffer>)thisCommand->srcBuffer, thisCommand->srcOffset,
					(__bridge id<MTLBuffer>)thisCommand->dstBuffer, thisCommand->dstOffset,
					thisCommand->size);
				break;
			}
			case CommandType_CopyTexture:
			{
				CopyTextureCommand* thisCommand = (CopyTextureCommand*)command;
				result = dsMTLCommandBuffer_copyTexture(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->srcTexture,
					(__bridge id<MTLTexture>)thisCommand->dstTexture, thisCommand->regions,
					thisCommand->regionCount);
				break;
			}
			case CommandType_GenerateMipmaps:
			{
				GenerateMipmapsCommand* thisCommand = (GenerateMipmapsCommand*)command;
				result = dsMTLCommandBuffer_generateMipmaps(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture);
				break;
			}
			case CommandType_BindPushConstants:
			{
				BindPushConstantsCommand* thisCommand = (BindPushConstantsCommand*)command;
				result = dsMTLCommandBuffer_bindPushConstants(commandBuffer, thisCommand->data,
					thisCommand->size, thisCommand->vertex, thisCommand->fragment);
				break;
			}
			case CommandType_BindBufferUniform:
			{
				BindBufferUniformCommand* thisCommand = (BindBufferUniformCommand*)command;
				result = dsMTLCommandBuffer_bindBufferUniform(commandBuffer,
					(__bridge id<MTLBuffer>)thisCommand->buffer, thisCommand->offset,
					thisCommand->vertexIndex, thisCommand->fragmentIndex);
				break;
			}
			case CommandType_BindTextureUniform:
			{
				BindTextureUniformCommand* thisCommand = (BindTextureUniformCommand*)command;
				result = dsMTLCommandBuffer_bindTextureUniform(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture,
					(__bridge id<MTLSamplerState>)thisCommand->sampler, thisCommand->vertexIndex,
					thisCommand->fragmentIndex);
				break;
			}
			case CommandType_SetRenderStates:
			{
				SetRenderStatesCommand* thisCommand = (SetRenderStatesCommand*)command;
				result = dsMTLCommandBuffer_setRenderStates(commandBuffer,
					&thisCommand->renderStates,
					(__bridge id<MTLDepthStencilState>)thisCommand->depthStencilState,
					thisCommand->hasDynamicStates ? &thisCommand->dynamicStates : NULL,
					thisCommand->dynamicOnly);
				break;
			}
			case CommandType_BeginComputeShader:
			{
				result = dsMTLCommandBuffer_beginComputeShader(commandBuffer);
				break;
			}
			case CommandType_BindComputePushConstants:
			{
				BindComputePushConstantsCommand* thisCommand =
					(BindComputePushConstantsCommand*)command;
				result = dsMTLCommandBuffer_bindComputePushConstants(commandBuffer,
					thisCommand->data, thisCommand->size);
				break;
			}
			case CommandType_BindComputeBufferUniform:
			{
				BindComputeBufferUniformCommand* thisCommand =
					(BindComputeBufferUniformCommand*)command;
				result = dsMTLCommandBuffer_bindComputeBufferUniform(commandBuffer,
					(__bridge id<MTLBuffer>)thisCommand->buffer, thisCommand->offset,
					thisCommand->index);
				break;
			}
			case CommandType_BindComputeTextureUniform:
			{
				BindComputeTextureUniformCommand* thisCommand =
					(BindComputeTextureUniformCommand*)command;
				result = dsMTLCommandBuffer_bindComputeTextureUniform(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture,
					(__bridge id<MTLSamplerState>)thisCommand->sampler, thisCommand->index);
				break;
			}
			case CommandType_BeginRenderPass:
			{
				BeginRenderPassCommand* thisCommand = (BeginRenderPassCommand*)command;
				result = dsMTLCommandBuffer_beginRenderPass(commandBuffer,
					(__bridge MTLRenderPassDescriptor*)thisCommand->renderPass,
					&thisCommand->viewport);
				break;
			}
			case CommandType_EndRenderPass:
			{
				result = dsMTLCommandBuffer_endRenderPass(commandBuffer);
				break;
			}
			case CommandType_ClearColorSurface:
			{
				ClearColorSurfaceCommand* thisCommand = (ClearColorSurfaceCommand*)command;
				result = dsMTLCommandBuffer_clearColorSurface(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture,
					(__bridge id<MTLTexture>)thisCommand->resolveTexture, thisCommand->clearColor);
				break;
			}
			case CommandType_ClearDepthStencilSurface:
			{
				ClearDepthStencilSurfaceCommand* thisCommand =
					(ClearDepthStencilSurfaceCommand*)command;
				result = dsMTLCommandBuffer_clearDepthStencilSurface(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->depthTexture,
					(__bridge id<MTLTexture>)thisCommand->resolveDepthTexture,
					thisCommand->depthValue, (__bridge id<MTLTexture>)thisCommand->stencilTexture,
					(__bridge id<MTLTexture>)thisCommand->resolveStencilTexture,
					thisCommand->stencilValue);
				break;
			}
			case CommandType_Draw:
			{
				DrawCommand* thisCommand = (DrawCommand*)command;
				result = dsMTLCommandBuffer_draw(commandBuffer,
					(__bridge id<MTLRenderPipelineState>)thisCommand->pipeline,
					&thisCommand->drawRange, thisCommand->primitiveType);
				break;
			}
			case CommandType_DrawIndexed:
			{
				DrawIndexedCommand* thisCommand = (DrawIndexedCommand*)command;
				result = dsMTLCommandBuffer_drawIndexed(commandBuffer,
					(__bridge id<MTLRenderPipelineState>)thisCommand->pipeline,
					(__bridge id<MTLBuffer>)thisCommand->indexBuffer, thisCommand->indexOffset,
					thisCommand->indexSize, &thisCommand->drawRange, thisCommand->primitiveType);
				break;
			}
			case CommandType_DrawIndirect:
			{
				DrawIndirectCommand* thisCommand = (DrawIndirectCommand*)command;
				result = dsMTLCommandBuffer_drawIndirect(commandBuffer,
					(__bridge id<MTLRenderPipelineState>)thisCommand->pipeline,
					(__bridge id<MTLBuffer>)thisCommand->indirectBuffer, thisCommand->offset,
					thisCommand->count, thisCommand->stride, thisCommand->primitiveType);
				break;
			}
			case CommandType_DrawIndexedIndirect:
			{
				DrawIndexedIndirectCommand* thisCommand = (DrawIndexedIndirectCommand*)command;
				result = dsMTLCommandBuffer_drawIndexedIndirect(commandBuffer,
					(__bridge id<MTLRenderPipelineState>)thisCommand->pipeline,
					(__bridge id<MTLBuffer>)thisCommand->indexBuffer, thisCommand->indexOffset,
					thisCommand->indexSize, (__bridge id<MTLBuffer>)thisCommand->indirectBuffer,
					thisCommand->indirectOffset, thisCommand->count, thisCommand->stride,
					thisCommand->primitiveType);
				break;
			}
			case CommandType_DispatchCompute:
			{
				DispatchComputeCommand* thisCommand = (DispatchComputeCommand*)command;
				result = dsMTLCommandBuffer_dispatchCompute(commandBuffer,
					(__bridge id<MTLComputePipelineState>)thisCommand->computePipeline,
					thisCommand->x, thisCommand->y, thisCommand->z, thisCommand->groupX,
					thisCommand->groupY, thisCommand->groupZ);
				break;
			}
			case CommandType_DispatchComputeIndirect:
			{
				DispatchComputeIndirectCommand* thisCommand =
					(DispatchComputeIndirectCommand*)command;
				result = dsMTLCommandBuffer_dispatchComputeIndirect(commandBuffer,
					(__bridge id<MTLComputePipelineState>)thisCommand->computePipeline,
					(__bridge id<MTLBuffer>)thisCommand->buffer, thisCommand->offset,
					thisCommand->groupX, thisCommand->groupY, thisCommand->groupZ);
				break;
			}
			default:
				DS_ASSERT(false);
		}

		if (!result)
			return false;
	}

	return true;
}

bool dsMTLSoftwareCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, const void* data, size_t size)
{
	dsRenderer* renderer = commandBuffer->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	CopyBufferCommand* command = (CopyBufferCommand*)allocateCommand(
		commandBuffer, CommandType_CopyBuffer, sizeof(CopyBufferCommand));
	if (!command)
		return false;

	id<MTLBuffer> tempBuffer = [device newBufferWithBytes: data length:
		size options: MTLResourceCPUCacheModeDefaultCache];
	if (!tempBuffer)
	{
		errno = ENOMEM;
		return false;
	}

	command->srcBuffer = CFBridgingRetain(tempBuffer);
	command->dstBuffer = CFBridgingRetain(buffer);
	command->srcOffset = 0;
	command->dstOffset = offset;
	command->size = size;
	return true;
}

bool dsMTLSoftwareCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> srcBuffer, size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset,
	size_t size)
{
	CopyBufferCommand* command = (CopyBufferCommand*)allocateCommand(
		commandBuffer, CommandType_CopyBuffer, sizeof(CopyBufferCommand));
	if (!command)
		return false;

	command->srcBuffer = CFBridgingRetain(srcBuffer);
	command->dstBuffer = CFBridgingRetain(dstBuffer);
	command->srcOffset = srcOffset;
	command->dstOffset = dstOffset;
	command->size = size;
	return true;
}

bool dsMTLSoftwareCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, const dsTextureInfo* textureInfo, const dsTexturePosition* position,
	uint32_t width, uint32_t height, uint32_t layers, const void* data, size_t size)
{
	DS_UNUSED(size);
	dsRenderer* renderer = commandBuffer->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	MTLTextureDescriptor* descriptor =
		[MTLTextureDescriptor texture2DDescriptorWithPixelFormat: texture.pixelFormat width: width
			height: height mipmapped: false];
	if (!descriptor)
	{
		errno = ENOMEM;
		return false;
	}

	unsigned int formatSize = dsGfxFormat_size(textureInfo->format);
	unsigned int blocksX, blocksY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blocksX, &blocksY, textureInfo->format));

	uint32_t blocksWide = (width + blocksX - 1)/blocksX;
	uint32_t blocksHigh = (width + blocksY - 1)/blocksY;
	uint32_t sliceSize = blocksWide*blocksHigh*formatSize;

	uint32_t faceCount = textureInfo->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = textureInfo->dimension == dsTextureDim_3D;
	bool is1D = textureInfo->dimension == dsTextureDim_1D;
	uint32_t iterations = is3D ? 1 : layers;
	uint32_t baseSlice = is3D ? 0 : position->depth*faceCount + position->face;
	const uint8_t* bytes = (const uint8_t*)data;

	MTLRegion region =
	{
		{0, 0, 0},
		{width, height, is3D ? layers: 1}
	};
	for (uint32_t i = 0; i < iterations; ++i)
	{
		id<MTLTexture> tempImage = [device newTextureWithDescriptor: descriptor];
		if (!tempImage)
		{
			errno = ENOMEM;
			return false;
		}

		[tempImage replaceRegion: region mipmapLevel: 0 slice: 0 withBytes: bytes + i*sliceSize
			bytesPerRow: is1D ? 0 : formatSize*blocksWide bytesPerImage: is3D ? sliceSize : 0];

		CopyTextureCommand* command = (CopyTextureCommand*)allocateCommand(
			commandBuffer, CommandType_CopyTexture,
			sizeof(CopyTextureCommand) + sizeof(dsTextureCopyRegion));
		if (!command)
			return false;

		command->srcTexture = CFBridgingRetain(tempImage);
		command->dstTexture = CFBridgingRetain(texture);
		command->regionCount = 1;
		command->regions->srcPosition.face = dsCubeFace_None;
		command->regions->srcPosition.x = 0;
		command->regions->srcPosition.y = 0;
		command->regions->srcPosition.depth = 0;
		command->regions->srcPosition.mipLevel = 0;
		command->regions->dstPosition = *position;

		uint32_t slice = baseSlice + i;
		command->regions->dstPosition.face = (dsCubeFace)(slice % faceCount);
		command->regions->dstPosition.depth = slice/faceCount;
	}

	return true;
}

bool dsMTLSoftwareCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount)
{
	CopyTextureCommand* command = (CopyTextureCommand*)allocateCommand(
		commandBuffer, CommandType_CopyTexture,
		sizeof(CopyTextureCommand) + regionCount*sizeof(dsTextureCopyRegion));
	if (!command)
		return false;

	command->srcTexture = CFBridgingRetain(srcTexture);
	command->dstTexture = CFBridgingRetain(dstTexture);
	command->regionCount = regionCount;
	memcpy(command->regions, regions, regionCount*sizeof(dsTextureCopyRegion));
	return true;
}

bool dsMTLSoftwareCommandBuffer_generateMipmaps(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture)
{
	GenerateMipmapsCommand* command = (GenerateMipmapsCommand*)allocateCommand(
		commandBuffer, CommandType_GenerateMipmaps, sizeof(GenerateMipmapsCommand));
	if (!command)
		return false;

	command->texture = CFBridgingRetain(texture);
	return true;
}

bool dsMTLSoftwareCommandBuffer_bindPushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment)
{
	BindPushConstantsCommand* command = (BindPushConstantsCommand*)allocateCommand(
		commandBuffer, CommandType_BindPushConstants, sizeof(BindPushConstantsCommand) + size);
	if (!command)
		return false;

	command->vertex = vertex;
	command->fragment = fragment;
	command->size = size;
	memcpy(command->data, data, size);
	return true;
}

bool dsMTLSoftwareCommandBuffer_bindBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	BindBufferUniformCommand* command = (BindBufferUniformCommand*)allocateCommand(
		commandBuffer, CommandType_BindBufferUniform, sizeof(BindBufferUniformCommand));
	if (!command)
		return false;

	command->buffer = CFBridgingRetain(buffer);
	command->offset = offset;
	command->vertexIndex = vertexIndex;
	command->fragmentIndex = fragmentIndex;
	return true;
}

bool dsMTLSoftwareCommandBuffer_setRenderStates(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly)
{
	SetRenderStatesCommand* command = (SetRenderStatesCommand*)allocateCommand(
		commandBuffer, CommandType_SetRenderStates, sizeof(SetRenderStatesCommand));
	if (!command)
		return false;

	command->renderStates = *renderStates;
	command->depthStencilState = CFBridgingRetain(depthStencilState);
	if (dynamicStates)
	{
		command->dynamicStates = *dynamicStates;
		command->hasDynamicStates = true;
	}
	else
	{
		DS_ASSERT(!dynamicOnly);
		command->hasDynamicStates = false;
	}
	command->dynamicOnly = dynamicOnly;
	return true;
}

bool dsMTLSoftwareCommandBuffer_bindTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t vertexIndex,
	uint32_t fragmentIndex)
{
	BindTextureUniformCommand* command = (BindTextureUniformCommand*)allocateCommand(
		commandBuffer, CommandType_BindTextureUniform, sizeof(BindTextureUniformCommand));
	if (!command)
		return false;

	command->texture = CFBridgingRetain(texture);
	command->sampler = CFBridgingRetain(sampler);
	command->vertexIndex = vertexIndex;
	command->fragmentIndex = fragmentIndex;
	return true;
}

bool dsMTLSoftwareCommandBuffer_beginComputeShader(dsCommandBuffer* commandBuffer)
{
	return allocateCommand(commandBuffer, CommandType_BeginComputeShader, sizeof(Command)) != NULL;
}

bool dsMTLSoftwareCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer,
	const void* data, uint32_t size)
{
	BindComputePushConstantsCommand* command =
		(BindComputePushConstantsCommand*)allocateCommand(commandBuffer,
			CommandType_BindComputePushConstants,
			sizeof(BindComputePushConstantsCommand) + size);
	if (!command)
		return false;

	command->size = size;
	memcpy(command->data, data, size);
	return true;
}

bool dsMTLSoftwareCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index)
{
	BindComputeBufferUniformCommand* command =
		(BindComputeBufferUniformCommand*)allocateCommand(commandBuffer,
			CommandType_BindComputeBufferUniform, sizeof(BindComputeBufferUniformCommand));
	if (!command)
		return false;

	command->buffer = CFBridgingRetain(buffer);
	command->offset = offset;
	command->index = index;
	return true;
}

bool dsMTLSoftwareCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index)
{
	BindComputeTextureUniformCommand* command =
		(BindComputeTextureUniformCommand*)allocateCommand(commandBuffer,
			CommandType_BindComputeTextureUniform, sizeof(BindComputeTextureUniformCommand));
	if (!command)
		return false;

	command->texture = CFBridgingRetain(texture);
	command->sampler = CFBridgingRetain(sampler);
	command->index = index;
	return true;
}

bool dsMTLSoftwareCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	MTLRenderPassDescriptor* renderPass, const dsAlignedBox3f* viewport)
{
	BeginRenderPassCommand* command =
		(BeginRenderPassCommand*)allocateCommand(commandBuffer,
			CommandType_BeginRenderPass, sizeof(BeginRenderPassCommand));
	if (!command)
		return false;

	command->renderPass = CFBridgingRetain(renderPass);
	command->viewport = *viewport;
	return true;
}

bool dsMTLSoftwareCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer)
{
	return allocateCommand(commandBuffer, CommandType_EndRenderPass, sizeof(Command)) != NULL;
}

bool dsMTLSoftwareCommandBuffer_clearColorSurface(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLTexture> resolveTexture, MTLClearColor clearColor)
{
	ClearColorSurfaceCommand* command = (ClearColorSurfaceCommand*)allocateCommand(commandBuffer,
		CommandType_ClearColorSurface, sizeof(ClearColorSurfaceCommand));
	if (!command)
		return false;

	command->texture = CFBridgingRetain(texture);
	command->resolveTexture = CFBridgingRetain(resolveTexture);
	command->clearColor = clearColor;
	return true;
}

bool dsMTLSoftwareCommandBuffer_clearDepthStencilSurface(dsCommandBuffer* commandBuffer,
	id<MTLTexture> depthTexture, id<MTLTexture> resolveDepthTexture, float depthValue,
	id<MTLTexture> stencilTexture, id<MTLTexture> resolveStencilTexture, uint32_t stencilValue)
{
	ClearDepthStencilSurfaceCommand* command =
		(ClearDepthStencilSurfaceCommand*)allocateCommand(commandBuffer,
			CommandType_ClearDepthStencilSurface, sizeof(ClearDepthStencilSurfaceCommand));
	if (!command)
		return false;

	command->depthTexture = CFBridgingRetain(depthTexture);
	command->resolveDepthTexture = CFBridgingRetain(resolveDepthTexture);
	command->stencilTexture = CFBridgingRetain(stencilTexture);
	command->resolveStencilTexture = CFBridgingRetain(resolveStencilTexture);
	command->depthValue = depthValue;
	command->stencilValue = stencilValue;
	return true;
}

bool dsMTLSoftwareCommandBuffer_draw(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, const dsDrawRange* drawRange,
	dsPrimitiveType primitiveType)
{
	DrawCommand* command = (DrawCommand*)allocateCommand(commandBuffer, CommandType_Draw,
		sizeof(DrawCommand));
	if (!command)
		return false;

	command->pipeline = CFBridgingRetain(pipeline);
	command->drawRange = *drawRange;
	command->primitiveType = primitiveType;
	return true;
}

bool dsMTLSoftwareCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, const dsDrawIndexedRange* drawRange, dsPrimitiveType primitiveType)
{
	DrawIndexedCommand* command = (DrawIndexedCommand*)allocateCommand(commandBuffer,
		CommandType_DrawIndexed, sizeof(DrawIndexedCommand));
	if (!command)
		return false;

	command->pipeline = CFBridgingRetain(pipeline);
	command->indexBuffer = CFBridgingRetain(indexBuffer);
	command->indexOffset = indexOffset;
	command->indexSize = indexSize;
	command->drawRange = *drawRange;
	command->primitiveType = primitiveType;
	return true;
}

bool dsMTLSoftwareCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	DrawIndirectCommand* command = (DrawIndirectCommand*)allocateCommand(commandBuffer,
		CommandType_DrawIndirect, sizeof(DrawIndirectCommand));
	if (!command)
		return false;

	command->pipeline = CFBridgingRetain(pipeline);
	command->indirectBuffer = CFBridgingRetain(indirectBuffer);
	command->offset = offset;
	command->count = count;
	command->stride = stride;
	command->primitiveType = primitiveType;
	return true;
}

bool dsMTLSoftwareCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, id<MTLBuffer> indirectBuffer, size_t indirectOffset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	DrawIndexedIndirectCommand* command =
		(DrawIndexedIndirectCommand*)allocateCommand(commandBuffer, CommandType_DrawIndexedIndirect,
			sizeof(DrawIndexedIndirectCommand));
	if (!command)
		return false;

	command->pipeline = CFBridgingRetain(pipeline);
	command->indexBuffer = CFBridgingRetain(indexBuffer);
	command->indexOffset = indexOffset;
	command->indexSize = indexSize;
	command->indirectBuffer = CFBridgingRetain(indirectBuffer);
	command->indirectOffset = indirectOffset;
	command->count = count;
	command->stride = stride;
	command->primitiveType = primitiveType;
	return true;
}

bool dsMTLSoftwareCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, uint32_t x, uint32_t y, uint32_t z,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	DispatchComputeCommand* command = (DispatchComputeCommand*)allocateCommand(commandBuffer,
		CommandType_DispatchCompute, sizeof(DispatchComputeCommand));
	if (!command)
		return false;

	command->computePipeline = CFBridgingRetain(computePipeline);
	command->x = x;
	command->y = y;
	command->z = z;
	command->groupX = groupX;
	command->groupY = groupY;
	command->groupZ = groupZ;
	return true;
}

bool dsMTLSoftwareCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, id<MTLBuffer> buffer, size_t offset,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	DispatchComputeIndirectCommand* command =
		(DispatchComputeIndirectCommand*)allocateCommand(commandBuffer,
			CommandType_DispatchComputeIndirect, sizeof(DispatchComputeIndirectCommand));
	if (!command)
		return false;

	command->computePipeline = CFBridgingRetain(computePipeline);
	command->buffer = CFBridgingRetain(buffer);
	command->offset = offset;
	command->groupX = groupX;
	command->groupY = groupY;
	command->groupZ = groupZ;
	return true;
}

static dsMTLCommandBufferFunctionTable softwareCommandBufferFunctions =
{
	&dsMTLSoftwareCommandBuffer_clear,
	&dsMTLSoftwareCommandBuffer_end,
	&dsMTLSoftwareCommandBuffer_submit,
	&dsMTLSoftwareCommandBuffer_copyBufferData,
	&dsMTLSoftwareCommandBuffer_copyBuffer,
	&dsMTLSoftwareCommandBuffer_copyTextureData,
	&dsMTLSoftwareCommandBuffer_copyTexture,
	&dsMTLSoftwareCommandBuffer_generateMipmaps,
	&dsMTLSoftwareCommandBuffer_bindPushConstants,
	&dsMTLSoftwareCommandBuffer_bindBufferUniform,
	&dsMTLSoftwareCommandBuffer_bindTextureUniform,
	&dsMTLSoftwareCommandBuffer_setRenderStates,
	&dsMTLSoftwareCommandBuffer_beginComputeShader,
	&dsMTLSoftwareCommandBuffer_bindComputePushConstants,
	&dsMTLSoftwareCommandBuffer_bindComputeBufferUniform,
	&dsMTLSoftwareCommandBuffer_bindComputeTextureUniform,
	&dsMTLSoftwareCommandBuffer_beginRenderPass,
	&dsMTLSoftwareCommandBuffer_endRenderPass,
	&dsMTLSoftwareCommandBuffer_clearColorSurface,
	&dsMTLSoftwareCommandBuffer_clearDepthStencilSurface,
	&dsMTLSoftwareCommandBuffer_draw,
	&dsMTLSoftwareCommandBuffer_drawIndexed,
	&dsMTLSoftwareCommandBuffer_drawIndirect,
	&dsMTLSoftwareCommandBuffer_drawIndexedIndirect,
	&dsMTLSoftwareCommandBuffer_dispatchCompute,
	&dsMTLSoftwareCommandBuffer_dispatchComputeIndirect
};

void dsMTLSoftwareCommandBuffer_initialize(dsMTLSoftwareCommandBuffer* commandBuffer,
	dsRenderer* renderer, dsAllocator* allocator, dsCommandBufferUsage usage)
{
	memset(commandBuffer, 0, sizeof(dsMTLSoftwareCommandBuffer));
	dsMTLCommandBuffer_initialize((dsMTLCommandBuffer*)commandBuffer, renderer, allocator, usage,
		&softwareCommandBufferFunctions);
}

void dsMTLSoftwareCommandBuffer_shutdown(dsMTLSoftwareCommandBuffer* commandBuffer)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	dsAllocator* allocator = baseCommandBuffer->allocator;
	// Not initialized yet.
	if (!allocator)
		return;

	dsMTLSoftwareCommandBuffer_clear(baseCommandBuffer);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->commands.buffer));
	dsMTLCommandBuffer_shutdown((dsMTLCommandBuffer*)commandBuffer);
}
