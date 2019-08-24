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

uint32_t dsVkPipeline_hash(uint32_t samples, float defaultAnisotropy,
	dsPrimitiveType primitiveType, uint32_t vertexFormatHash, const dsRenderPass* renderPass,
	uint32_t subpass);
dsVkPipeline* dsVkPipeline_create(dsAllocator* allocator, dsShader* shader,
	VkPipeline existingPipeline, uint32_t hash, uint32_t samples, float defaultAnisotropy,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry, const dsRenderPass* renderPass,
	uint32_t subpass);
bool dsVkPipeline_isEquivalent(const dsVkPipeline* pipeline, uint32_t hash,
	uint32_t samples, float defaultAnisotropy, dsPrimitiveType primitiveType,
	const dsDrawGeometry* geometry, const dsVkRenderPassData* renderPassData, uint32_t subpass);
void dsVkPipeline_destroy(dsVkPipeline* pipeline);
