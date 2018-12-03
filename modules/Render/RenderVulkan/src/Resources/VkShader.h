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

dsShader* dsVkShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc);
bool dsVkShader_isUniformInternal(dsResourceManager* resourceManager, const char* name);
bool dsVkShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues, const dsDynamicRenderStates* renderStates);
bool dsVkShader_updateVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues);
bool dsVkShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader);
bool dsVkShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues);
bool dsVkShader_updateComputeVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues);
bool dsVkShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader);
bool dsVkShader_destroy(dsResourceManager* resourceManager, dsShader* shader);

bool dsVkShader_addMaterial(dsShader* shader, dsDeviceMaterial* material);
void dsVkShader_removeMaterial(dsShader* shader, dsDeviceMaterial* material);
dsVkSamplerList* dsVkShader_getSamplerList(dsShader* shader, dsCommandBuffer* commandBuffer);
