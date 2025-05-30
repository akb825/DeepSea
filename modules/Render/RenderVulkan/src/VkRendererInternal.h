/*
 * Copyright 2018-2025 Aaron Barany
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
#include "VkTypes.h"

VkSemaphore dsVkRenderer_flushImpl(dsRenderer* renderer, bool readback, bool useSemaphore);
dsGfxFenceResult dsVkRenderer_waitForSubmit(dsRenderer* renderer, uint64_t submitCount,
	uint64_t timeout);
uint64_t dsVkRenderer_getFinishedSubmitCount(const dsRenderer* renderer);

void dsVkRenderer_processGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer);
void dsVkRenderer_processTexture(dsRenderer* renderer, dsTexture* texture);
void dsVkRenderer_processRenderbuffer(dsRenderer* renderer, dsRenderbuffer* renderbuffer);
void dsVkRenderer_processRenderSurface(dsRenderer* renderer, dsVkRenderSurfaceData* surface);

void dsVkRenderer_deleteGfxBuffer(
	dsRenderer* renderer, dsVkGfxBufferData* buffer, bool gpuFinished);
void dsVkRenderer_deleteTexture(dsRenderer* renderer, dsTexture* texture, bool gpuFinished);
void dsVkRenderer_deleteTempBuffer(dsRenderer* renderer, dsVkTempBuffer* buffer, bool gpuFinished);
void dsVkRenderer_deleteRenderbuffer(
	dsRenderer* renderer, dsRenderbuffer* renderbuffer, bool gpuFinished);
void dsVkRenderer_deleteFramebuffer(
	dsRenderer* renderer, dsVkRealFramebuffer* framebuffer, bool gpuFinished);
void dsVkRenderer_deleteFence(dsRenderer* renderer, dsGfxFence* fence, bool gpuFinished);
void dsVkRenderer_deleteQueriePool(dsRenderer* renderer, dsGfxQueryPool* queries, bool gpuFinished);
void dsVkRenderer_deleteMaterialDescriptor(
	dsRenderer* renderer, dsVkMaterialDescriptor* descriptor, bool gpuFinished);
void dsVkRenderer_deleteSamplerList(
	dsRenderer* renderer, dsVkSamplerList* samplers, bool gpuFinished);
void dsVkRenderer_deleteComputePipeline(
	dsRenderer* renderer, dsVkComputePipeline* pipeline, bool gpuFinished);
void dsVkRenderer_deletePipeline(dsRenderer* renderer, dsVkPipeline* pipeline, bool gpuFinished);
void dsVkRenderer_deleteCommandPool(
	dsRenderer* renderer, dsVkCommandPoolData* pool, bool gpuFinished);
void dsVkRenderer_deleteRenderPass(
	dsRenderer* renderer, dsVkRenderPassData* renderPass, bool gpuFinished);
