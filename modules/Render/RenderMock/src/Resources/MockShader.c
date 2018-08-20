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

#include "Resources/MockShader.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <MSL/Client/ModuleC.h>
#include <string.h>

dsShader* dsMockShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc,
	dsPrimitiveType primitiveType, uint32_t samples)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(module);
	DS_ASSERT(shaderIndex < dsShaderModule_shaderCount(module));
	DS_ASSERT(materialDesc);

	size_t size = DS_ALIGNED_SIZE(sizeof(dsShader)) + DS_ALIGNED_SIZE(sizeof(mslPipeline));
	void* buffer = dsAllocator_alloc(allocator, size);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, size));

	dsShader* shader = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator, dsShader);
	DS_ASSERT(shader);

	shader->resourceManager = resourceManager;
	shader->allocator = dsAllocator_keepPointer(allocator);
	shader->module = module;
	shader->pipelineIndex = shaderIndex;
	shader->pipeline = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator, mslPipeline);
	DS_ASSERT(shader->pipeline);
	DS_VERIFY(mslModule_pipeline(shader->pipeline, module->module, shaderIndex));
	shader->name = shader->pipeline->name;
	shader->materialDesc = materialDesc;
	shader->primitiveType = primitiveType;
	shader->samples = samples;

	return shader;
}

bool dsMockShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material, const dsVolatileMaterialValues* volatileValues,
	const dsDynamicRenderStates* renderStates)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(shader);
	DS_UNUSED(shader);
	DS_ASSERT(material);
	DS_UNUSED(material);
	DS_UNUSED(volatileValues);
	DS_UNUSED(renderStates);

	return true;
}

bool dsMockShader_updateVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(shader);
	DS_UNUSED(shader);
	DS_UNUSED(volatileValues);

	return true;
}

bool dsMockShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(shader);
	DS_UNUSED(shader);

	return true;
}

bool dsMockShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(shader);
	DS_UNUSED(shader);
	DS_ASSERT(material);
	DS_UNUSED(material);
	DS_UNUSED(volatileValues);

	return true;
}

bool dsMockShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	DS_UNUSED(resourceManager);
	if (shader->allocator)
		return dsAllocator_free(shader->allocator, shader);
	return true;
}
