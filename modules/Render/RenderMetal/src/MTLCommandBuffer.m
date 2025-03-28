/*
 * Copyright 2019-2025 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

void dsMTLCommandBuffer_initialize(dsMTLCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage,
	const dsMTLCommandBufferFunctionTable* functions)
{
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	DS_ASSERT(allocator->freeFunc);

	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = usage;

	commandBuffer->functions = functions;
}

bool dsMTLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		dsMTLCommandBuffer_clear(commandBuffer);
		return true;
	}
}

bool dsMTLCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport, dsGfxOcclusionQueryState parentOcclusionQueryState)
{
	DS_UNUSED(renderer);
	DS_UNUSED(framebuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpass);
	DS_UNUSED(viewport);
	DS_UNUSED(parentOcclusionQueryState);

	@autoreleasepool
	{
		dsMTLCommandBuffer_clear(commandBuffer);
		return true;
	}
}

bool dsMTLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	return true;
}

bool dsMTLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	@autoreleasepool
	{
		DS_UNUSED(renderer);
		dsMTLCommandBuffer* mtlSubmitBuffer = (dsMTLCommandBuffer*)submitBuffer;
		if (!mtlSubmitBuffer->functions->submitFunc(commandBuffer, submitBuffer))
			return false;

		for (uint32_t i = 0; i < mtlSubmitBuffer->gfxBufferCount; ++i)
		{
			dsMTLGfxBufferData* buffer =
				(dsMTLGfxBufferData*)dsLifetime_acquire(mtlSubmitBuffer->gfxBuffers[i]);
			if (!buffer)
				continue;

			dsMTLCommandBuffer_addGfxBuffer(commandBuffer, buffer);
			dsLifetime_release(mtlSubmitBuffer->gfxBuffers[i]);
		}

		for (uint32_t i = 0; i < mtlSubmitBuffer->tempBufferCount; ++i)
		{
			dsMTLTempBuffer* buffer =
				(dsMTLTempBuffer*)dsLifetime_acquire(mtlSubmitBuffer->tempBuffers[i]);
			if (!buffer)
				continue;

			dsMTLCommandBuffer_addTempBuffer(commandBuffer, buffer);
			dsLifetime_release(mtlSubmitBuffer->tempBuffers[i]);
		}

		for (uint32_t i = 0; i < mtlSubmitBuffer->readbackOffscreenCount; ++i)
		{
			dsTexture* texture =
				(dsTexture*)dsLifetime_acquire(mtlSubmitBuffer->readbackOffscreens[i]);
			if (!texture)
				continue;

			dsMTLCommandBuffer_addReadbackOffscreen(commandBuffer, texture);
			dsLifetime_release(mtlSubmitBuffer->readbackOffscreens[i]);
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
}

bool dsMTLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, const void* data, size_t size)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferDataFunc(commandBuffer, buffer, offset, data, size);
}

bool dsMTLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset, size_t size)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferFunc(commandBuffer, srcBuffer, srcOffset, dstBuffer, dstOffset,
		size);
}

bool dsMTLCommandBuffer_copyBufferToTexture(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	id<MTLTexture> dstTexture, dsGfxFormat format, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferToTextureFunc(commandBuffer, srcBuffer, dstTexture, format, regions,
		regionCount);
}

bool dsMTLCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, const dsTextureInfo* textureInfo, const dsTexturePosition* position,
	uint32_t width, uint32_t height, uint32_t layers, const void* data, size_t size)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureDataFunc(commandBuffer, texture, textureInfo, position, width,
		height, layers, data, size);
}

bool dsMTLCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, id<MTLTexture> srcTexture,
	id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureFunc(commandBuffer, srcTexture, dstTexture, regions, regionCount);
}

bool dsMTLCommandBuffer_copyTextureToBuffer(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLBuffer> dstBuffer, dsGfxFormat format,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureToBufferFunc(commandBuffer, srcTexture, dstBuffer, format, regions,
		regionCount);
}

bool dsMTLCommandBuffer_generateMipmaps(dsCommandBuffer* commandBuffer, id<MTLTexture> texture)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->generateMipmapsFunc(commandBuffer, texture);
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

bool dsMTLCommandBuffer_copyClearValues(dsCommandBuffer* commandBuffer,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	mtlCommandBuffer->clearValueCount = 0;
	if (clearValueCount == 0)
		return true;

	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->clearValues,
			mtlCommandBuffer->clearValueCount, mtlCommandBuffer->maxClearValues, clearValueCount))
	{
		return false;
	}

	memcpy(mtlCommandBuffer->clearValues, clearValues, clearValueCount*sizeof(dsSurfaceClearValue));
	return true;
}

bool dsMTLCommandBuffer_bindPushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindPushConstantsFunc(commandBuffer, data, size, vertex, fragment);
}

bool dsMTLCommandBuffer_bindBufferUniform(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindBufferUniformFunc(commandBuffer, buffer, offset, vertexIndex,
		fragmentIndex);
}

bool dsMTLCommandBuffer_bindTextureUniform(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	id<MTLSamplerState> sampler, uint32_t vertexIndex, uint32_t fragmentIndex)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindTextureUniformFunc(commandBuffer, texture, sampler, vertexIndex,
		fragmentIndex);
}

bool dsMTLCommandBuffer_setRenderStates(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->setRenderStatesFunc(commandBuffer, renderStates, depthStencilState,
		dynamicStates, dynamicOnly);
}

bool dsMTLCommandBuffer_beginComputeShader(dsCommandBuffer* commandBuffer)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->beginComputeShaderFunc(commandBuffer);
}

bool dsMTLCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindComputePushConstantsFunc(commandBuffer, data, size);
}

bool dsMTLCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindComputeBufferUniformFunc(commandBuffer, buffer, offset, index);
}

bool dsMTLCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->bindComputeTextureUniformFunc(commandBuffer, texture, sampler, index);
}

bool dsMTLCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	MTLRenderPassDescriptor* renderPass, const dsAlignedBox3f* viewport)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->beginRenderPassFunc(commandBuffer, renderPass, viewport);
}

bool dsMTLCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	mtlCommandBuffer->boundGeometry = NULL;
	mtlCommandBuffer->firstVertexBuffer = 0;
	mtlCommandBuffer->vertexOffset = 0;

	const dsMTLCommandBufferFunctionTable* functions = mtlCommandBuffer->functions;
	return functions->endRenderPassFunc(commandBuffer);
}

bool dsMTLCommandBuffer_setViewport(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport)
{
	DS_UNUSED(renderer);
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->setViewportFunc(commandBuffer, viewport);
}

bool dsMTLCommandBuffer_clearAttachments(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount)
{
	DS_UNUSED(renderer);
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->clearAttachmentsFunc(commandBuffer, attachments, attachmentCount, regions,
		regionCount);
}

bool dsMTLCommandBuffer_draw(dsCommandBuffer* commandBuffer, id<MTLRenderPipelineState> pipeline,
	const dsDrawRange* drawRange, dsPrimitiveType primitiveType)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->drawFunc(commandBuffer, pipeline, drawRange, primitiveType);
}

bool dsMTLCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, const dsDrawIndexedRange* drawRange, dsPrimitiveType primitiveType)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndexedFunc(commandBuffer, pipeline, indexBuffer, indexOffset, indexSize,
		drawRange, primitiveType);
}

bool dsMTLCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndirectFunc(commandBuffer, pipeline, indirectBuffer, offset, count,
		stride, primitiveType);
}

bool dsMTLCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, id<MTLBuffer> indirectBuffer, size_t indirectOffset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndexedIndirectFunc(commandBuffer, pipeline, indexBuffer, indexOffset,
		indexSize, indirectBuffer, indirectOffset, count, stride, primitiveType);
}

bool dsMTLCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, uint32_t x, uint32_t y, uint32_t z,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeFunc(commandBuffer, computePipeline, x, y, z, groupX, groupY,
		groupZ);
}

bool dsMTLCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, id<MTLBuffer> buffer, size_t offset,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeIndirectFunc(commandBuffer, computePipeline, buffer, offset,
		groupX, groupY, groupZ);
}

bool dsMTLCommandBuffer_pushDebugGroup(dsCommandBuffer* commandBuffer, const char* name)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->pushDebugGroupFunc(commandBuffer, name);
}

bool dsMTLCommandBuffer_popDebugGroup(dsCommandBuffer* commandBuffer)
{
	const dsMTLCommandBufferFunctionTable* functions =
		((dsMTLCommandBuffer*)commandBuffer)->functions;
	return functions->popDebugGroupFunc(commandBuffer);
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

bool dsMTLCommandBuffer_addTempBuffer(dsCommandBuffer* commandBuffer, dsMTLTempBuffer* buffer)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	uint32_t index = mtlCommandBuffer->tempBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->tempBuffers,
			mtlCommandBuffer->tempBufferCount, mtlCommandBuffer->maxTempBuffers, 1))
	{
		return false;
	}

	mtlCommandBuffer->tempBuffers[index] = dsLifetime_addRef(buffer->lifetime);
	return true;
}

bool dsMTLCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsOffscreen* offscreen)
{
	dsMTLCommandBuffer* mtlCommandBuffer = (dsMTLCommandBuffer*)commandBuffer;
	dsMTLTexture* mtlTexture = (dsMTLTexture*)offscreen;
	if (!mtlTexture->mtlTexture)
		return true;

	for (uint32_t i = 0; i < mtlCommandBuffer->readbackOffscreenCount; ++i)
	{
		if (mtlCommandBuffer->readbackOffscreens[i] == mtlTexture->lifetime)
			return true;
	}

	uint32_t index = mtlCommandBuffer->readbackOffscreenCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, mtlCommandBuffer->readbackOffscreens,
			mtlCommandBuffer->readbackOffscreenCount, mtlCommandBuffer->maxReadbackOffscreens, 1))
	{
		return false;
	}

	mtlCommandBuffer->readbackOffscreens[index] = dsLifetime_addRef(mtlTexture->lifetime);
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

	for (uint32_t i = 0; i < mtlCommandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->gfxBuffers[i]);
	mtlCommandBuffer->gfxBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->tempBufferCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->tempBuffers[i]);
	mtlCommandBuffer->tempBufferCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->readbackOffscreenCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->readbackOffscreens[i]);
	mtlCommandBuffer->readbackOffscreenCount = 0;

	for (uint32_t i = 0; i < mtlCommandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(mtlCommandBuffer->fences[i]);
	mtlCommandBuffer->fenceCount = 0;

	mtlCommandBuffer->functions->clearFunc(commandBuffer);

	mtlCommandBuffer->boundGeometry = NULL;
	mtlCommandBuffer->firstVertexBuffer = 0;
	mtlCommandBuffer->vertexOffset = 0;
}

void dsMTLCommandBuffer_shutdown(dsMTLCommandBuffer* commandBuffer)
{
	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;
	DS_ASSERT(allocator);

	for (uint32_t i = 0; i < commandBuffer->gfxBufferCount; ++i)
		dsLifetime_freeRef(commandBuffer->gfxBuffers[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->gfxBuffers));

	for (uint32_t i = 0; i < commandBuffer->tempBufferCount; ++i)
		dsLifetime_freeRef(commandBuffer->tempBuffers[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->tempBuffers));

	for (uint32_t i = 0; i < commandBuffer->readbackOffscreenCount; ++i)
		dsLifetime_freeRef(commandBuffer->readbackOffscreens[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->readbackOffscreens));

	for (uint32_t i = 0; i < commandBuffer->fenceCount; ++i)
		dsLifetime_freeRef(commandBuffer->fences[i]);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fences));

	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->pushConstantData));
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->clearValues));
}
