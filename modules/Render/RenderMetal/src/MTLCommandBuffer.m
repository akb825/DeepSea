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

#include "MTLCommandBuffer.h"

#include "MTLRendererInternal.h"
#include "MTLShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

#import <Metal/MTLCommandQueue.h>

typedef enum CommandType
{
	CommandType_BindPushConstants,
	CommandType_BindBufferUniform,
	CommandType_BindTextureUniform,
	CommandType_SetRenderStates,
	CommandType_BindComputePushConstants,
	CommandType_BindComputeBufferUniform,
	CommandType_BindComputeTextureUniform
} CommandType;

typedef struct Command
{
	CommandType type;
	uint32_t size;
} Command;

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

static Command* allocateCommand(dsCommandBuffer* commandBuffer, CommandType type, size_t size)
{
	DS_ASSERT(size >= sizeof(Command));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	int prevErrno = errno;
	Command* command = (Command*)dsAllocator_alloc(
		(dsAllocator*)&mtlCommandBuffer->secondaryCommands, size);
	if (!command)
	{
		// Allocate a new buffer.
		errno = prevErrno;
		size_t newBufferSize = dsMax(mtlCommandBuffer->secondaryCommands.bufferSize*2,
			mtlCommandBuffer->secondaryCommands.bufferSize + size);
		void* newBuffer = dsAllocator_reallocWithFallback(commandBuffer->allocator,
			mtlCommandBuffer->secondaryCommands.buffer,
			((dsAllocator*)&mtlCommandBuffer->secondaryCommands)->size, newBufferSize);
		if (!newBuffer)
			return NULL;

		DS_VERIFY(dsBufferAllocator_initialize(&mtlCommandBuffer->secondaryCommands, newBuffer,
			newBufferSize));
		command = (Command*)dsAllocator_alloc((dsAllocator*)&mtlCommandBuffer->secondaryCommands,
			size);
		DS_ASSERT(command);
	}

	command->type = type;
	command->size = (uint32_t)DS_ALIGNED_SIZE(size);
	return command;
}

static void clearSecondaryCommands(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;

	// Free any internal refs for resources.
	uint8_t* buffer = (uint8_t*)mtlCommandBuffer->secondaryCommands.buffer;
	size_t bufferSize = ((dsAllocator*)&mtlCommandBuffer->secondaryCommands)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		switch (command->type)
		{
			case CommandType_BindPushConstants:
				break;
			case CommandType_BindBufferUniform:
			{
				BindBufferUniformCommand* thisCommand = (BindBufferUniformCommand*)command;
				CFRelease(thisCommand->buffer);
				break;
			}
			case CommandType_BindTextureUniform:
			{
				BindTextureUniformCommand* thisCommand = (BindTextureUniformCommand*)command;
				CFRelease(thisCommand->texture);
				if (thisCommand->sampler)
					CFRelease(thisCommand->sampler);
				break;
			}
			case CommandType_SetRenderStates:
				break;
			case CommandType_BindComputePushConstants:
				break;
			case CommandType_BindComputeBufferUniform:
			{
				BindComputeBufferUniformCommand* thisCommand =
					(BindComputeBufferUniformCommand*)command;
				CFRelease(thisCommand->buffer);
				break;
			}
			case CommandType_BindComputeTextureUniform:
			{
				BindComputeTextureUniformCommand* thisCommand =
					(BindComputeTextureUniformCommand*)command;
				CFRelease(thisCommand->texture);
				if (thisCommand->sampler)
					CFRelease(thisCommand->sampler);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	DS_VERIFY(dsBufferAllocator_reset(&mtlCommandBuffer->secondaryCommands));
}

static void submitSecondaryCommands(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	dsMTLCommandBuffer* mtlSubmitBuffer = (dsMTLCommandBuffer*)submitBuffer;

	uint8_t* buffer = (uint8_t*)mtlSubmitBuffer->secondaryCommands.buffer;
	size_t bufferSize = ((dsAllocator*)&mtlSubmitBuffer->secondaryCommands)->size;
	size_t offset = 0;
	while (offset < bufferSize)
	{
		Command* command = (Command*)(buffer + offset);
		offset += command->size;
		switch (command->type)
		{
			case CommandType_BindPushConstants:
			{
				BindPushConstantsCommand* thisCommand = (BindPushConstantsCommand*)command;
				dsMTLCommandBuffer_bindPushConstants(commandBuffer, thisCommand->data,
					thisCommand->size, thisCommand->vertex, thisCommand->fragment);
				break;
			}
			case CommandType_BindBufferUniform:
			{
				BindBufferUniformCommand* thisCommand = (BindBufferUniformCommand*)command;
				dsMTLCommandBuffer_bindBufferUniform(commandBuffer,
					(__bridge id<MTLBuffer>)thisCommand->buffer, thisCommand->offset,
					thisCommand->vertexIndex, thisCommand->fragmentIndex);
				break;
			}
			case CommandType_BindTextureUniform:
			{
				BindTextureUniformCommand* thisCommand = (BindTextureUniformCommand*)command;
				dsMTLCommandBuffer_bindTextureUniform(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture,
					(__bridge id<MTLSamplerState>)thisCommand->sampler, thisCommand->vertexIndex,
					thisCommand->fragmentIndex);
				break;
			}
			case CommandType_SetRenderStates:
			{
				SetRenderStatesCommand* thisCommand = (SetRenderStatesCommand*)command;
				dsMTLCommandBuffer_setRenderStates(commandBuffer, &thisCommand->renderStates,
					(__bridge id<MTLDepthStencilState>)thisCommand->depthStencilState,
					thisCommand->hasDynamicStates ? &thisCommand->dynamicStates : NULL);
				break;
			}
			case CommandType_BindComputePushConstants:
			{
				BindComputePushConstantsCommand* thisCommand =
					(BindComputePushConstantsCommand*)command;
				dsMTLCommandBuffer_bindComputePushConstants(commandBuffer, thisCommand->data,
					thisCommand->size);
				break;
			}
			case CommandType_BindComputeBufferUniform:
			{
				BindComputeBufferUniformCommand* thisCommand =
					(BindComputeBufferUniformCommand*)command;
				dsMTLCommandBuffer_bindComputeBufferUniform(commandBuffer,
					(__bridge id<MTLBuffer>)thisCommand->buffer, thisCommand->offset,
					thisCommand->index);
				break;
			}
			case CommandType_BindComputeTextureUniform:
			{
				BindComputeTextureUniformCommand* thisCommand =
					(BindComputeTextureUniformCommand*)command;
				dsMTLCommandBuffer_bindComputeTextureUniform(commandBuffer,
					(__bridge id<MTLTexture>)thisCommand->texture,
					(__bridge id<MTLSamplerState>)thisCommand->sampler, thisCommand->index);
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}
}

static bool needsDynamicDepthStencil(const mslStencilOpState* state)
{
	return state->compareMask == MSL_UNKNOWN || state->writeMask == MSL_UNKNOWN;
}

static MTLStencilDescriptor* getStencilDescriptor(const mslStencilOpState* state,
	uint32_t compareMask, uint32_t writeMask)
{
	MTLStencilDescriptor* descriptor = [MTLStencilDescriptor new];
	if (!descriptor)
		return NULL;

	descriptor.stencilFailureOperation = dsGetMTLStencilOp(state->failOp);
	descriptor.depthFailureOperation = dsGetMTLStencilOp(state->depthFailOp);
	descriptor.depthStencilPassOperation = dsGetMTLStencilOp(state->passOp);
	descriptor.stencilCompareFunction = dsGetMTLCompareFunction(state->compareOp);
	descriptor.readMask = state->compareMask == MSL_UNKNOWN ? compareMask : state->compareMask;
	descriptor.writeMask = state->writeMask == MSL_UNKNOWN ? writeMask : state->writeMask;
	return descriptor;
}

static void setRasterizationState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, const dsDynamicRenderStates* dynamicStates)
{
	const mslRasterizationState* rasterState = &renderStates->rasterizationState;
	[encoder setTriangleFillMode: rasterState->polygonMode == mslPolygonMode_Line ?
		MTLTriangleFillModeLines : MTLTriangleFillModeFill];
	[encoder setFrontFacingWinding: rasterState->frontFace == mslFrontFace_Clockwise ?
		MTLWindingClockwise : MTLWindingCounterClockwise];

	MTLCullMode cullMode = MTLCullModeNone;
	switch (rasterState->cullMode)
	{
		case mslCullMode_Front:
			cullMode = MTLCullModeFront;
			break;
		case mslCullMode_Back:
			cullMode = MTLCullModeBack;
			break;
		case mslCullMode_None:
		default:
			cullMode = MTLCullModeNone;
			break;
	}
	[encoder setCullMode: cullMode];

	dsColor4f blendConstants;
	memcpy(&blendConstants, renderStates->blendState.blendConstants, sizeof(blendConstants));
	if (blendConstants.x == MSL_UNKNOWN_FLOAT)
	{
		if (dynamicStates)
			blendConstants = dynamicStates->blendConstants;
		else
		{
			blendConstants.r = 0.0f;
			blendConstants.g = 0.0f;
			blendConstants.b = 0.0f;
			blendConstants.a = 1.0f;
		}
	}
	[encoder setBlendColorRed: blendConstants.r green: blendConstants.g blue: blendConstants.b
		alpha: blendConstants.a];
}

static void setDepthStencilState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates)
{
	if (renderStates->depthStencilState.stencilTestEnable == mslBool_True &&
		dynamicStates &&
			(needsDynamicDepthStencil(&renderStates->depthStencilState.frontStencil) ||
			needsDynamicDepthStencil(&renderStates->depthStencilState.backStencil)))
	{
		MTLDepthStencilDescriptor* descriptor = [MTLDepthStencilDescriptor new];
		if (!descriptor)
			return;

		descriptor.depthCompareFunction =
			dsGetMTLCompareFunction(renderStates->depthStencilState.depthCompareOp);
		descriptor.depthWriteEnabled =
			renderStates->depthStencilState.depthWriteEnable != mslBool_False;
		descriptor.frontFaceStencil = getStencilDescriptor(
			&renderStates->depthStencilState.frontStencil, dynamicStates->frontStencilCompareMask,
			dynamicStates->backStencilCompareMask);
		descriptor.backFaceStencil = getStencilDescriptor(
			&renderStates->depthStencilState.backStencil, dynamicStates->backStencilCompareMask,
			dynamicStates->backStencilCompareMask);

		depthStencilState = [[encoder device] newDepthStencilStateWithDescriptor: descriptor];
		if (!depthStencilState)
			return;
	}

	[encoder setDepthStencilState: depthStencilState];

	if (renderStates->depthStencilState.stencilTestEnable == mslBool_True)
	{
		uint32_t frontReference = renderStates->depthStencilState.frontStencil.reference;
		if (frontReference == MSL_UNKNOWN && dynamicStates)
			frontReference = dynamicStates->frontStencilReference;
#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		uint32_t backReference = renderStates->depthStencilState.backStencil.reference;
		if (backReference == MSL_UNKNOWN && dynamicStates)
			backReference = dynamicStates->backStencilReference;
		[encoder setStencilFrontReferenceValue: frontReference backReferenceValue: backReference];
#else
		[encoder setStencilReferenceValue: frontReference];
#endif
	}
}

static void setDynamicDepthState(id<MTLRenderCommandEncoder> encoder,
	const mslRenderState* renderStates, const dsDynamicRenderStates* dynamicStates)
{
	if (renderStates->depthStencilState.depthWriteEnable == mslBool_False)
		return;

	float constBias = 0.0f, slopeBias = 0.0f, clamp = 0.0f;
	if (renderStates->rasterizationState.depthBiasEnable == mslBool_True)
	{
		if (renderStates->rasterizationState.depthBiasConstantFactor != MSL_UNKNOWN_FLOAT)
			constBias = renderStates->rasterizationState.depthBiasConstantFactor;
		else if (dynamicStates)
			constBias = dynamicStates->depthBiasConstantFactor;

		if (renderStates->rasterizationState.depthBiasSlopeFactor != MSL_UNKNOWN_FLOAT)
			slopeBias = renderStates->rasterizationState.depthBiasSlopeFactor;
		else if (dynamicStates)
			slopeBias = dynamicStates->depthBiasSlopeFactor;

		if (renderStates->rasterizationState.depthBiasClamp != MSL_UNKNOWN_FLOAT)
			clamp = renderStates->rasterizationState.depthBiasClamp;
		else if (dynamicStates)
			clamp = dynamicStates->depthBiasClamp;
	}

	[encoder setDepthBias: constBias slopeScale: slopeBias clamp: clamp];
	[encoder setDepthClipMode:
		renderStates->rasterizationState.depthClampEnable == mslBool_True ?
			MTLDepthClipModeClamp : MTLDepthClipModeClip];
}

void dsMTLCommandBuffer_initialize(dsMTLCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	DS_ASSERT(allocator->freeFunc);

	memset(commandBuffer, 0, sizeof(dsMTLCommandBuffer));
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;
}

bool dsMTLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(renderer);

	dsMTLCommandBuffer_clear(commandBuffer);
	return true;
}

bool dsMTLCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport)
{
	DS_UNUSED(renderer);
	DS_UNUSED(framebuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpass);
	DS_UNUSED(viewport);

	dsMTLCommandBuffer_clear(commandBuffer);
	return true;
}

bool dsMTLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(renderer);
	dsMTLCommandBuffer_endEncoding(commandBuffer);

	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
	{
		CFRelease(mtlCommandBuffer->mtlCommandBuffer);
		mtlCommandBuffer = NULL;
	}

	return true;
}

bool dsMTLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(renderer);
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	dsMTLCommandBuffer* mtlSubmitBuffer = (dsMTLCommandBuffer*)submitBuffer;

	dsMTLCommandBuffer_endEncoding(commandBuffer);
	if (submitBuffer->usage & dsCommandBufferUsage_Secondary)
		submitSecondaryCommands(commandBuffer, submitBuffer);
	else
	{
		if (mtlCommandBuffer->mtlCommandBuffer)
		{
			CFRelease(mtlCommandBuffer->mtlCommandBuffer);
			mtlCommandBuffer = NULL;
		}

		if (mtlSubmitBuffer->mtlCommandBuffer)
		{
			uint32_t index = mtlCommandBuffer->submitBufferCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
					mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers, 1))
			{
				return false;
			}

			mtlCommandBuffer->submitBuffers[index] = CFRetain(mtlSubmitBuffer->mtlCommandBuffer);
		}
	}

	for (uint32_t i = 0; i < mtlSubmitBuffer->gfxBufferCount; ++i)
	{
		dsMTLGfxBufferData* buffer =
			(dsMTLGfxBufferData*)dsLifetime_acquire(mtlSubmitBuffer->gfxBuffers[i]);
		if (!buffer)
			continue;

		dsMTLCommandBuffer_addGfxBuffer(commandBuffer, buffer);
		dsLifetime_release(mtlSubmitBuffer->gfxBuffers[i]);
	}

	for (uint32_t i = 0; i < mtlSubmitBuffer->fenceCount; ++i)
	{
		dsGfxFence* fence = (dsGfxFence*)dsLifetime_acquire(mtlSubmitBuffer->fences[i]);
		if (!fence)
			continue;

		dsMTLCommandBuffer_addFence(commandBuffer, fence);
		dsLifetime_release(mtlSubmitBuffer->fences[i]);
	}

	if (!(submitBuffer->usage &
		(dsCommandBufferUsage_MultiFrame | dsCommandBufferUsage_MultiFrame)))
	{
		dsMTLCommandBuffer_clear(submitBuffer);
	}

	return true;
}

void* dsMTLCommandBuffer_getPushConstantData(dsCommandBuffer* commandBuffer, uint32_t size)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	uint32_t dummySize = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->pushConstantData,
			dummySize, mtlCommandBuffer->maxPushConstantDataSize, size))
	{
		return NULL;
	}

	return mtlCommandBuffer->pushConstantData;
}

bool dsMTLCommandBuffer_bindPushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindPushConstantsCommand* command = (BindPushConstantsCommand*)allocateCommand(
			commandBuffer, CommandType_BindPushConstants, sizeof(BindPushConstantsCommand) + size);
		if (!command)
			return false;

		command->vertex = vertex;
		command->fragment = fragment;
		command->size = size;
		memcpy(command->data, data, size);
	}
	else
	{
		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		DS_ASSERT(mtlCommandBuffer->renderCommandEncoder);
		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
		if (vertex)
			[encoder setVertexBytes: data length: size atIndex: 0];
		if (fragment)
			[encoder setFragmentBytes: data length: size atIndex: 0];
	}

	return true;
}

bool dsMTLCommandBuffer_bindBufferUniform(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindBufferUniformCommand* command = (BindBufferUniformCommand*)allocateCommand(
			commandBuffer, CommandType_BindBufferUniform, sizeof(BindBufferUniformCommand));
		if (!command)
			return false;

		command->buffer = CFBridgingRetain(buffer);
		command->offset = offset;
		command->vertexIndex = vertexIndex;
		command->fragmentIndex = fragmentIndex;
	}
	else
	{
		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		DS_ASSERT(mtlCommandBuffer->renderCommandEncoder);
		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
		if (vertexIndex != DS_MATERIAL_UNKNOWN)
			[encoder setVertexBuffer: buffer offset: offset atIndex: vertexIndex];
		if (fragmentIndex != DS_MATERIAL_UNKNOWN)
			[encoder setFragmentBuffer: buffer offset: offset atIndex: vertexIndex];
	}

	return true;
}

bool dsMTLCommandBuffer_setRenderStates(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
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
			command->hasDynamicStates = false;
	}
	else
	{
		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		DS_ASSERT(mtlCommandBuffer->renderCommandEncoder);
		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
		setRasterizationState(encoder, renderStates, dynamicStates);
		setDepthStencilState(encoder, renderStates, depthStencilState, dynamicStates);
		setDynamicDepthState(encoder, renderStates, dynamicStates);
	}

	return true;
}

bool dsMTLCommandBuffer_bindTextureUniform(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	id<MTLSamplerState> sampler, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindTextureUniformCommand* command = (BindTextureUniformCommand*)allocateCommand(
			commandBuffer, CommandType_BindTextureUniform, sizeof(BindTextureUniformCommand));
		if (!command)
			return false;

		command->texture = CFBridgingRetain(texture);
		command->sampler = CFBridgingRetain(sampler);
		command->vertexIndex = vertexIndex;
		command->fragmentIndex = fragmentIndex;
	}
	else
	{
		dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
		DS_ASSERT(mtlCommandBuffer->renderCommandEncoder);
		id<MTLRenderCommandEncoder> encoder =
			(__bridge id<MTLRenderCommandEncoder>)mtlCommandBuffer->renderCommandEncoder;
		if (vertexIndex != DS_MATERIAL_UNKNOWN)
		{
			[encoder setVertexTexture: texture atIndex: vertexIndex];
			if (sampler)
				[encoder setVertexSamplerState: sampler atIndex: vertexIndex];
		}
		if (fragmentIndex != DS_MATERIAL_UNKNOWN)
		{
			[encoder setFragmentTexture: texture atIndex: vertexIndex];
			if (sampler)
				[encoder setFragmentSamplerState: sampler atIndex: vertexIndex];
		}
	}

	return true;
}

bool dsMTLCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindComputePushConstantsCommand* command =
			(BindComputePushConstantsCommand*)allocateCommand(commandBuffer,
				CommandType_BindComputePushConstants,
				sizeof(BindComputePushConstantsCommand) + size);
		if (!command)
			return false;

		command->size = size;
		memcpy(command->data, data, size);
	}
	else
	{
		id<MTLComputeCommandEncoder> encoder =
			dsMTLCommandBuffer_getComputeCommandEncoder(commandBuffer);
		if (!encoder)
			return false;

		[encoder setBytes: data length: size atIndex: 0];
	}

	return true;
}

bool dsMTLCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindComputeBufferUniformCommand* command =
			(BindComputeBufferUniformCommand*)allocateCommand(commandBuffer,
				CommandType_BindComputeBufferUniform, sizeof(BindComputeBufferUniformCommand));
		if (!command)
			return false;

		command->buffer = CFBridgingRetain(buffer);
		command->offset = offset;
		command->index = index;
	}
	else
	{
		id<MTLComputeCommandEncoder> encoder =
			dsMTLCommandBuffer_getComputeCommandEncoder(commandBuffer);
		if (!encoder)
			return false;

		[encoder setBuffer: buffer offset: offset atIndex: index];
	}

	return true;
}

bool dsMTLCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index)
{
	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		BindComputeTextureUniformCommand* command =
			(BindComputeTextureUniformCommand*)allocateCommand(commandBuffer,
				CommandType_BindComputeTextureUniform, sizeof(BindComputeTextureUniformCommand));
		if (!command)
			return false;

		command->texture = CFBridgingRetain(texture);
		command->sampler = CFBridgingRetain(sampler);
		command->index = index;
	}
	else
	{
		id<MTLComputeCommandEncoder> encoder =
			dsMTLCommandBuffer_getComputeCommandEncoder(commandBuffer);
		if (!encoder)
			return false;

		[encoder setTexture: texture atIndex: index];
		if (sampler)
			[encoder setSamplerState: sampler atIndex: index];
	}

	return true;
}

id<MTLCommandBuffer> dsMTLCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->mtlCommandBuffer)
		return (__bridge id<MTLCommandBuffer>)(mtlCommandBuffer);

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)commandBuffer->renderer;
	id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)mtlRenderer->commandQueue;
	id<MTLCommandBuffer> newCommandBuffer = [commandQueue commandBuffer];
	if (!newCommandBuffer)
	{
		errno = ENOMEM;
		return nil;
	}

	uint32_t index = mtlCommandBuffer->submitBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->submitBuffers,
			mtlCommandBuffer->submitBufferCount, mtlCommandBuffer->maxSubmitBuffers, 1))
	{
		return nil;
	}

	mtlCommandBuffer->mtlCommandBuffer = CFBridgingRetain(newCommandBuffer);
	mtlCommandBuffer->submitBuffers[index] = CFBridgingRetain(newCommandBuffer);
	return newCommandBuffer;
}

id<MTLBlitCommandEncoder> dsMTLCommandBuffer_getBlitCommandEncoder(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->blitCommandEncoder)
		return (__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = dsMTLCommandBuffer_getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLCommandBuffer_endEncoding(commandBuffer);

	id<MTLBlitCommandEncoder> encoder = [realCommandBuffer blitCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->blitCommandEncoder = CFBridgingRetain(encoder);
	return encoder;
}

id<MTLComputeCommandEncoder> dsMTLCommandBuffer_getComputeCommandEncoder(
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(!(commandBuffer->usage & dsCommandBufferUsage_Secondary));
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	if (mtlCommandBuffer->computeCommandEncoder)
		return (__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);

	id<MTLCommandBuffer> realCommandBuffer = dsMTLCommandBuffer_getCommandBuffer(commandBuffer);
	if (!realCommandBuffer)
		return nil;

	dsMTLCommandBuffer_endEncoding(commandBuffer);

	id<MTLComputeCommandEncoder> encoder = [realCommandBuffer computeCommandEncoder];
	if (!encoder)
	{
		errno = ENOMEM;
		return nil;
	}

	mtlCommandBuffer->computeCommandEncoder = CFBridgingRetain(encoder);
	return encoder;
}

void dsMTLCommandBuffer_endEncoding(dsCommandBuffer* commandBuffer)
{
	// Render encoder is fully managed by render passes.
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	DS_ASSERT(!mtlCommandBuffer->renderCommandEncoder);

	if (mtlCommandBuffer->blitCommandEncoder)
	{
		id<MTLBlitCommandEncoder> encoder =
			(__bridge id<MTLBlitCommandEncoder>)(mtlCommandBuffer->blitCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->blitCommandEncoder);
		mtlCommandBuffer->blitCommandEncoder = NULL;
	}

	if (mtlCommandBuffer->computeCommandEncoder)
	{
		id<MTLComputeCommandEncoder> encoder =
			(__bridge id<MTLComputeCommandEncoder>)(mtlCommandBuffer->computeCommandEncoder);
		[encoder endEncoding];
		CFRelease(mtlCommandBuffer->computeCommandEncoder);
		mtlCommandBuffer->computeCommandEncoder = NULL;
	}
}

bool dsMTLCommandBuffer_addGfxBuffer(dsCommandBuffer* commandBuffer, dsMTLGfxBufferData* buffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, mtlCommandBuffer->gfxBufferCount);
	for (uint32_t i = mtlCommandBuffer->gfxBufferCount - checkCount;
		i < mtlCommandBuffer->gfxBufferCount; ++i)
	{
		if (mtlCommandBuffer->gfxBuffers[i] == buffer->lifetime)
			return true;
	}

	uint32_t index = mtlCommandBuffer->gfxBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->gfxBuffers,
			mtlCommandBuffer->gfxBufferCount, mtlCommandBuffer->maxGfxBuffers, 1))
	{
		return false;
	}

	mtlCommandBuffer->gfxBuffers[index] = dsLifetime_addRef(buffer->lifetime);
	return true;
}

bool dsMTLCommandBuffer_addFence(dsCommandBuffer* commandBuffer, dsGfxFence* fence)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	dsMTLGfxFence* mtlFence = (dsMTLGfxFence*)fence;

	uint32_t checkCount = dsMin(DS_RECENTLY_ADDED_SIZE, mtlCommandBuffer->fenceCount);
	for (uint32_t i = mtlCommandBuffer->fenceCount - checkCount;
		i < mtlCommandBuffer->fenceCount; ++i)
	{
		if (mtlCommandBuffer->fences[i] == mtlFence->lifetime)
			return true;
	}

	uint32_t index = mtlCommandBuffer->fenceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->fences,
			mtlCommandBuffer->fenceCount, mtlCommandBuffer->maxFences, 1))
	{
		return false;
	}

	mtlCommandBuffer->fences[index] = dsLifetime_addRef(mtlFence->lifetime);
	return true;
}

void dsMTLCommandBuffer_submitFence(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;

	// Process immediately for the main command buffer if not in a render pass.
	if (commandBuffer == commandBuffer->renderer->mainCommandBuffer &&
		!commandBuffer->boundRenderPass)
	{
		dsMTLRenderer_flushImpl(commandBuffer->renderer, nil);
		mtlCommandBuffer->fenceSet = false;
		return;
	}

	mtlCommandBuffer->fenceSet = true;
}

void dsMTLCommandBuffer_clear(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlCommandBuffer->submitBuffers[i])
			CFRelease(mtlCommandBuffer->submitBuffers[i]);
	}
	mtlCommandBuffer->submitBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->gfxBuffers[i]);
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->fences[i]);
	mtlCommandBuffer->fenceCount = 0;

	clearSecondaryCommands(commandBuffer);
}

void dsMTLCommandBuffer_submitted(dsCommandBuffer* commandBuffer, uint64_t submitCount)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	for (uint32_t i = 0; i < mtlCommandBuffer->submitBufferCount; ++i)
	{
		if (mtlCommandBuffer->submitBuffers[i])
			CFRelease(mtlCommandBuffer->submitBuffers[i]);
	}
	mtlCommandBuffer->submitBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->gfxBuffers[i];
		dsMTLGfxBufferData* bufferData = (dsMTLGfxBufferData*)dsLifetime_acquire(lifetime);
		if (bufferData)
		{
			DS_ATOMIC_STORE64(&bufferData->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
	{
		dsLifetime* lifetime = mtlCommandBuffer->fences[i];
		dsMTLGfxFence* fence = (dsMTLGfxFence*)dsLifetime_acquire(lifetime);
		if (fence)
		{
			DS_ATOMIC_STORE64(&fence->lastUsedSubmit, &submitCount);
			dsLifetime_release(lifetime);
		}
		dsLifetime_freeRef(lifetime);
	}
	mtlCommandBuffer->fenceCount = 0;
}

void dsMTLCommandBuffer_shutdown(dsMTLCommandBuffer* commandBuffer)
{
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	// Not initialized yet.
	if (!allocator)
		return;

	if (commandBuffer->mtlCommandBuffer)
		CFRelease(commandBuffer->mtlCommandBuffer);
	if (commandBuffer->renderCommandEncoder)
		CFRelease(commandBuffer->renderCommandEncoder);
	if (commandBuffer->blitCommandEncoder)
		CFRelease(commandBuffer->blitCommandEncoder);
	if (commandBuffer->computeCommandEncoder)
		CFRelease(commandBuffer->computeCommandEncoder);

	for (uint32_t i = 0; i < commandBuffer->submitBufferCount; ++i)
	{
		if (commandBuffer->submitBuffers[i])
			CFRelease(commandBuffer->submitBuffers[i]);
	}
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->submitBuffers));

	for (uint32_t i = 0; i < commandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(commandBuffer->gfxBuffers[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->gfxBuffers));

	for (uint32_t i = 0; i < commandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(commandBuffer->fences[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fences));

	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->secondaryCommands.buffer));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->pushConstantData));
}
