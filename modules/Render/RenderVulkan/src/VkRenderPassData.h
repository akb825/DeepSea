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
#include "VkTypes.h"

dsVkRenderPassData* dsVkRenderPassData_create(dsAllocator* allocator, dsVkDevice* device,
	const dsRenderPass* renderPass);
bool dsVkRenderPassData_begin(const dsVkRenderPassData* renderPass,
	dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount);
bool dsVkRenderPassData_nextSubpass(const dsVkRenderPassData* renderPass,
	dsCommandBuffer* commandBuffer, uint32_t index);
bool dsVkRenderPassData_end(const dsVkRenderPassData* renderPass, dsCommandBuffer* commandBuffer);

bool dsVkRenderPassData_addShader(dsVkRenderPassData* renderPass, dsShader* shader);
void dsVkRenderPassData_removeShader(dsVkRenderPassData* renderPass, dsShader* shader);

bool dsVkRenderPassData_addFramebuffer(dsVkRenderPassData* renderPass, dsFramebuffer* framebuffer);
void dsVkRenderPassData_removeFramebuffer(dsVkRenderPassData* renderPass,
	dsFramebuffer* framebuffer);

void dsVkRenderPassData_destroy(dsVkRenderPassData* renderPass);
