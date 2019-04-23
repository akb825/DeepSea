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

#import <Metal/MTLRenderPipeline.h>

dsShader* dsMTLShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc);
bool dsMTLShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues, const dsDynamicRenderStates* renderStates);
bool dsMTLShader_updateSharedValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* sharedValues);
bool dsMTLShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader);
bool dsMTLShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues);
bool dsMTLShader_updateComputeSharedValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* sharedValues);
bool dsMTLShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader);
bool dsMTLShader_destroy(dsResourceManager* resourceManager, dsShader* shader);

id<MTLRenderPipelineState> dsMTLShader_getPipeline(dsShader* shader, dsCommandBuffer* commandBuffer,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry);
void dsMTLShader_removeRenderPass(dsShader* shader, dsRenderPass* renderPass);
