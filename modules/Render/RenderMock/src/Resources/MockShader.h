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

#include <DeepSea/Render/Resources/Types.h>

dsShader* dsMockShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc);
bool dsMockShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues, const dsDynamicRenderStates* renderStates);
bool dsMockShader_updateVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues);
bool dsMockShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader);
bool dsMockShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues);
bool dsMockShader_destroy(dsResourceManager* resourceManager, dsShader* shader);
