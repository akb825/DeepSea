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

#pragma once

#include <DeepSea/Core/Config.h>
#include "GLTypes.h"

void dsGLCommandBuffer_initialize(dsCommandBuffer* commandBuffer);
void dsGLCommandBuffer_shutdown(dsCommandBuffer* commandBuffer);

void dsGLCommandBuffer_reset(dsCommandBuffer* commandBuffer);

bool dsGLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size);
bool dsGLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);

bool dsGLCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size);
bool dsGLCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount);
bool dsGLCommandBuffer_generateTextureMipmaps(dsCommandBuffer* commandBuffer, dsTexture* texture);

bool dsGLCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback);

bool dsGLCommandBuffer_beginQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query);
bool dsGLCommandBuffer_endQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query);
bool dsGLCommandBuffer_queryTimestamp(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query);
bool dsGLCommandBuffer_copyQueryValues(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability);

bool dsGLCommandBuffer_bindShaderAndMaterial(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsMaterial* material, const dsSharedMaterialValues* sharedValues,
	const dsDynamicRenderStates* renderStates);
bool dsGLCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates);
bool dsGLCommandBuffer_setTexture(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture);
bool dsGLCommandBuffer_setTextureBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count);
bool dsGLCommandBuffer_setShaderBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size);
bool dsGLCommandBuffer_setUniform(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data);
bool dsGLCommandBuffer_setInstanceMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsSharedMaterialValues* instanceValues);
bool dsGLCommandBuffer_updateDynamicRenderStates(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsDynamicRenderStates* renderStates);
bool dsGLCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader);

bool dsGLCommandBuffer_bindComputeShaderAndMaterial(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues);
bool dsGLCommandBuffer_bindComputeShader(dsCommandBuffer* commandBuffer, const dsShader* shader);
bool dsGLCommandBuffer_setComputeInstanceMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsSharedMaterialValues* instanceValues);
bool dsGLCommandBuffer_unbindComputeShader(dsCommandBuffer* commandBuffer, const dsShader* shader);

bool dsGLCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface);
bool dsGLCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface);

bool dsGLCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount);
bool dsGLCommandBuffer_nextRenderSubpass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index);
bool dsGLCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass);

bool dsGLCommandBuffer_clearColorSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	const dsSurfaceColorValue* colorValue);
bool dsGLCommandBuffer_clearDepthStencilSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue);

bool dsGLCommandBuffer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType);
bool dsGLCommandBuffer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType);
bool dsGLCommandBuffer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
bool dsGLCommandBuffer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
bool dsGLCommandBuffer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	uint32_t x, uint32_t y, uint32_t z);
bool dsGLCommandBuffer_dispatchComputeIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset);
bool dsGLCommandBuffer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, uint32_t regionCount,
	dsBlitFilter filter);

bool dsGLCommandBuffer_pushDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const char* name);
bool dsGLCommandBuffer_popDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer);

bool dsGLCommandBuffer_memoryBarrier(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount);

bool dsGLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsGLCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport);
bool dsGLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsGLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);
