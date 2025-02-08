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

bool dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage, VkCommandPool commandPool);
dsCommandBuffer* dsVkCommandBuffer_get(dsCommandBuffer* commandBuffer);

bool dsVkCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsVkCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport, dsGfxOcclusionQueryState parentOcclusionQueryState);
bool dsVkCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer);
bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

void dsVkCommandBuffer_prepare(dsCommandBuffer* commandBuffer);
VkCommandBuffer dsVkCommandBuffer_getCommandBuffer(dsCommandBuffer* commandBuffer);
void dsVkCommandBuffer_forceNewCommandBuffer(dsCommandBuffer* commandBuffer);
void dsVkCommandBuffer_finishCommandBuffer(dsCommandBuffer* commandBuffer);
void dsVkCommandBuffer_submitFence(dsCommandBuffer* commandBuffer, bool readback);
bool dsVkCommandBuffer_endSubmitCommands(dsCommandBuffer* commandBuffer);

bool dsVkCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer, VkRenderPass renderPass,
	VkFramebuffer framebuffer, const VkViewport* viewport, const VkClearValue* clearValues,
	uint32_t clearValueCount, bool secondary);
bool dsVkCommandBuffer_nextSubpass(dsCommandBuffer* commandBuffer, bool secondary);
bool dsVkCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer);

void dsVkCommandBuffer_bindPipeline(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	VkPipeline pipeline);
void dsVkCommandBuffer_bindComputePipeline(dsCommandBuffer* commandBuffer,
	VkCommandBuffer submitBuffer, VkPipeline pipeline);
void dsVkCommandBuffer_bindDescriptorSet(dsCommandBuffer* commandBuffer,
	VkCommandBuffer submitBuffer, VkPipelineBindPoint bindPoint, uint32_t setIndex,
	VkPipelineLayout layout, VkDescriptorSet descriptorSet, const uint32_t* offsets,
	uint32_t offsetCount);

void* dsVkCommandBuffer_getTempData(size_t* outOffset, VkBuffer* outBuffer,
	dsCommandBuffer* commandBuffer, size_t size, uint32_t alignment);

VkBufferMemoryBarrier* dsVkCommandBuffer_addBufferBarrier(dsCommandBuffer* commandBuffer);
VkImageMemoryBarrier* dsVkCommandBuffer_addImageBarrier(dsCommandBuffer* commandBuffer);
bool dsVkCommandBuffer_submitMemoryBarriers(dsCommandBuffer* commandBuffer,
	VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);
void dsVkCommandBuffer_resetMemoryBarriers(dsCommandBuffer* commandBuffer);

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource);
bool dsVkCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsOffscreen* offscreen);
bool dsVkCommandBuffer_addRenderSurface(dsCommandBuffer* commandBuffer,
	dsVkRenderSurfaceData* surface);
void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer, bool gpuFinished);
void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount);
void dsVkCommandBuffer_submittedReadbackOffscreens(dsCommandBuffer* commandBuffer,
	uint64_t submitCount);
void dsVkCommandBuffer_submittedRenderSurfaces(dsCommandBuffer* commandBuffer,
	uint64_t submitCount);

dsVkSharedDescriptorSets* dsVkCommandBuffer_getGlobalDescriptorSets(
	dsCommandBuffer* commandBuffer);
dsVkSharedDescriptorSets* dsVkCommandBuffer_getInstanceDescriptorSets(
	dsCommandBuffer* commandBuffer);
uint8_t* dsVkCommandBuffer_allocatePushConstantData(dsCommandBuffer* commandBuffer, uint32_t size);

void dsVkCommandBuffer_shutdown(dsVkCommandBuffer* commandBuffer);
