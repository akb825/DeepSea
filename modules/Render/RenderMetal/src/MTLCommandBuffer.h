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

#pragma once

#include <DeepSea/Core/Config.h>
#include "MTLTypes.h"

void dsMTLCommandBuffer_initialize(dsMTLCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage,
	const dsMTLCommandBufferFunctionTable* functions);

bool dsMTLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsMTLCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport);
bool dsMTLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsMTLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

bool dsMTLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, const void* data, size_t size);
bool dsMTLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset, size_t size);
bool dsMTLCommandBuffer_copyBufferToTexture(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	id<MTLTexture> dstTexture, dsGfxFormat format, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount);

bool dsMTLCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, const dsTextureInfo* textureInfo, const dsTexturePosition* position,
	uint32_t width, uint32_t height, uint32_t layers, const void* data, size_t size);
bool dsMTLCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, id<MTLTexture> srcTexture,
	id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount);
bool dsMTLCommandBuffer_copyTextureToBuffer(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLBuffer> dstBuffer, dsGfxFormat format,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount);
bool dsMTLCommandBuffer_generateMipmaps(dsCommandBuffer* commandBuffer, id<MTLTexture> texture);

void* dsMTLCommandBuffer_getPushConstantData(dsCommandBuffer* commandBuffer, uint32_t size);
bool dsMTLCommandBuffer_copyClearValues(dsCommandBuffer* commandBuffer,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount);

bool dsMTLCommandBuffer_bindPushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment);
bool dsMTLCommandBuffer_bindBufferUniform(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex);
bool dsMTLCommandBuffer_bindTextureUniform(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	id<MTLSamplerState> sampler, uint32_t vertexIndex, uint32_t fragmentIndex);
bool dsMTLCommandBuffer_setRenderStates(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly);

bool dsMTLCommandBuffer_beginComputeShader(dsCommandBuffer* commandBuffer);
bool dsMTLCommandBuffer_bindComputePushConstants(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size);
bool dsMTLCommandBuffer_bindComputeBufferUniform(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index);
bool dsMTLCommandBuffer_bindComputeTextureUniform(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index);

bool dsMTLCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	MTLRenderPassDescriptor* renderPass, const dsAlignedBox3f* viewport);
bool dsMTLCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer);

bool dsMTLCommandBuffer_clearAttachments(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount);
bool dsMTLCommandBuffer_draw(dsCommandBuffer* commandBuffer, id<MTLRenderPipelineState> pipeline,
	const dsDrawRange* drawRange, dsPrimitiveType primitiveType);
bool dsMTLCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, const dsDrawIndexedRange* drawRange, dsPrimitiveType primitiveType);
bool dsMTLCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
bool dsMTLCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, id<MTLBuffer> indirectBuffer, size_t indirectOffset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);

bool dsMTLCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, uint32_t x, uint32_t y, uint32_t z,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ);
bool dsMTLCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, id<MTLBuffer> buffer, size_t offset,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ);

bool dsMTLCommandBuffer_pushDebugGroup(dsCommandBuffer* commandBuffer, const char* name);
bool dsMTLCommandBuffer_popDebugGroup(dsCommandBuffer* commandBuffer);

bool dsMTLCommandBuffer_addGfxBuffer(dsCommandBuffer* commandBuffer, dsMTLGfxBufferData* buffer);
bool dsMTLCommandBuffer_addTempBuffer(dsCommandBuffer* commandBuffer, dsMTLTempBuffer* buffer);
bool dsMTLCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer,
	dsOffscreen* offscreen);
bool dsMTLCommandBuffer_addFence(dsCommandBuffer* commandBuffer, dsGfxFence* fence);

void dsMTLCommandBuffer_submitFence(dsCommandBuffer* commandBuffer);
void dsMTLCommandBuffer_clear(dsCommandBuffer* commandBuffer);

void dsMTLCommandBuffer_shutdown(dsMTLCommandBuffer* commandBuffer);
