/*
 * Copyright 2018 Aaron Barany
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

void dsVkCommandBuffer_initialize(dsVkCommandBuffer* commandBuffer, dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage);

void dsVkCommandBuffer_prepare(dsCommandBuffer* commandBuffer);
void dsVkCommandBuffer_submitFence(dsCommandBuffer* commandBuffer, bool readback);
bool dsVkCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

bool dsVkCommandBuffer_endSubmitCommands(dsCommandBuffer* commandBuffer,
	VkCommandBuffer renderCommands);

bool dsVkCommandBuffer_addResource(dsCommandBuffer* commandBuffer, dsVkResource* resource);
bool dsVkCommandBuffer_addReadbackOffscreen(dsCommandBuffer* commandBuffer, dsTexture* offscreen);
void dsVkCommandBuffer_clearUsedResources(dsCommandBuffer* commandBuffer);
void dsVkCommandBuffer_submittedResources(dsCommandBuffer* commandBuffer, uint64_t submitCount);
void dsVkCommandBuffer_submittedReadbackOffscreens(dsCommandBuffer* commandBuffer,
	uint64_t submitCount);

uint8_t* dsVkCommandBuffer_allocatePushConstantData(dsCommandBuffer* commandBuffer, uint32_t size);

void dsVkCommandBuffer_shutdown(dsVkCommandBuffer* commandBuffer);
