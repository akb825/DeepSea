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

#include "Resources/VkShader.h"

#include "Resources/VkDeviceMaterial.h"
#include "Resources/VkSamplerList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <MSL/Client/ModuleC.h>
#include <string.h>

dsShader* dsVkShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc)
{
	mslPipeline pipeline;
	if (!mslModule_pipeline(&pipeline, module->module, shaderIndex))
		return NULL;

	uint32_t samplerCount = 0;
	bool samplersHaveDefaultAnisotropy = false;
	for (uint32_t i = 0; i < pipeline.uniformCount; ++i)
	{
		mslUniform uniform;
		if (!mslModule_uniform(&uniform, module->module, shaderIndex, i))
		{
			errno = EINDEX;
			return NULL;
		}
		if (uniform.uniformType != mslUniformType_SampledImage)
			continue;

		++samplerCount;
		if (uniform.samplerIndex == MSL_UNKNOWN)
			continue;

		mslSamplerState sampler;
		if (!mslModule_samplerState(&sampler, module->module, i, uniform.samplerIndex))
		{
			errno = EINDEX;
			return NULL;
		}

		if (sampler.mipFilter == mslMipFilter_Anisotropic &&
			sampler.maxAnisotropy != MSL_UNKNOWN_FLOAT)
		{
			samplersHaveDefaultAnisotropy = true;
		}
	}

	dsAllocator* scratchAllocator = allocator;
	if (!scratchAllocator->freeFunc)
		scratchAllocator = resourceManager->allocator;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkShader)) +
		(samplerCount > 0 ? DS_ALIGNED_SIZE(sizeof(uint32_t)*materialDesc->elementCount) : 0U);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkShader* shader = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkShader);
	DS_ASSERT(shader);

	dsLifetime* lifetime = dsLifetime_create(scratchAllocator, shader);
	if (!lifetime)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, shader));
		return NULL;
	}

	dsShader* baseShader = (dsShader*)shader;
	baseShader->resourceManager = resourceManager;
	baseShader->allocator = dsAllocator_keepPointer(allocator);
	baseShader->module = module;
	baseShader->name = pipeline.name;
	baseShader->pipelineIndex = shaderIndex;
	baseShader->pipeline = &shader->pipeline;
	baseShader->materialDesc = materialDesc;

	shader->scratchAllocator = scratchAllocator;
	shader->lifetime = lifetime;
	shader->pipeline = pipeline;

	shader->usedMaterials = NULL;
	shader->usedMaterialCount = 0;
	shader->maxUsedMaterials = 0;
	shader->pipelines = NULL;
	shader->pipelineCount = 0;
	shader->maxPipelines = 0;

	shader->samplers = NULL;
	if (samplerCount > 0)
	{
		shader->samplerMapping = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkSamplerMapping, materialDesc->elementCount);
		DS_ASSERT(shader->samplerMapping);

		uint32_t index = 0;
		for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
		{
			if (materialDesc->elements[i].type != dsMaterialType_Texture)
			{
				shader->samplerMapping[i].samplerIndex = DS_MATERIAL_UNKNOWN;
				shader->samplerMapping[i].uniformIndex = DS_MATERIAL_UNKNOWN;
				continue;
			}

			for (uint32_t j = 0; j < pipeline.uniformCount; ++j)
			{
				mslUniform uniform;
				DS_VERIFY(mslModule_uniform(&uniform, module->module, shaderIndex, j));
				if (strcmp(uniform.name, materialDesc->elements[i].name) != 0)
					continue;

				DS_ASSERT(uniform.uniformType == mslUniformType_SampledImage);
				shader->samplerMapping[i].samplerIndex = index++;
				shader->samplerMapping[i].uniformIndex = j;
				break;
			}
		}
		DS_ASSERT(index == samplerCount);
	}
	else
		shader->samplerMapping = NULL;
	shader->samplerCount = samplerCount;
	shader->samplersHaveDefaultAnisotropy = samplersHaveDefaultAnisotropy;

	DS_VERIFY(dsSpinlock_initialize(&shader->materialLock));
	DS_VERIFY(dsSpinlock_initialize(&shader->pipelineLock));
	DS_VERIFY(dsSpinlock_initialize(&shader->samplerLock));

	// If no dependency on default anisotropy, create immediately.
	if (samplerCount > 0 && !samplersHaveDefaultAnisotropy)
	{
		shader->samplers = dsVkSamplerList_create(scratchAllocator, baseShader);
		if (!shader->samplers)
		{
			dsVkShader_destroy(resourceManager, baseShader);
			return NULL;
		}
	}

	return baseShader;
}

bool dsVkShader_isUniformInternal(dsResourceManager* resourceManager, const char* name)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(name);
	return false;
}

bool dsVkShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkShader* vkShader = (dsVkShader*)shader;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&vkShader->materialLock));
	dsLifetime** usedMaterials = vkShader->usedMaterials;
	uint32_t usedMaterialCount = vkShader->usedMaterialCount;
	vkShader->usedMaterials = NULL;
	vkShader->usedMaterialCount = 0;
	vkShader->maxUsedMaterials = 0;
	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));

	for (uint32_t i = 0; i < usedMaterialCount; ++i)
	{
		dsDeviceMaterial* deviceMaterial = (dsDeviceMaterial*)dsLifetime_acquire(usedMaterials[i]);
		if (deviceMaterial)
		{
			dsVkDeviceMaterial_removeShader(deviceMaterial, shader);
			dsLifetime_release(usedMaterials[i]);
		}
		dsLifetime_freeRef(usedMaterials[i]);
	}
	DS_VERIFY(dsAllocator_free(vkShader->scratchAllocator, usedMaterials));
	DS_ASSERT(!vkShader->usedMaterials);

	dsLifetime_destroy(vkShader->lifetime);

	if (vkShader->samplers)
		dsVkRenderer_deleteSamplerList(renderer, vkShader->samplers);

	dsSpinlock_shutdown(&vkShader->materialLock);
	dsSpinlock_shutdown(&vkShader->pipelineLock);
	dsSpinlock_shutdown(&vkShader->samplerLock);
	if (shader->allocator)
		DS_VERIFY(dsAllocator_free(shader->allocator, shader));
	return true;
}

bool dsVkShader_addMaterial(dsShader* shader, dsDeviceMaterial* material)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&vkShader->materialLock));

	for (uint32_t i = 0; i < vkShader->usedMaterialCount; ++i)
	{
		void* usedMaterial = dsLifetime_getObject(vkShader->usedMaterials[i]);
		DS_ASSERT(usedMaterial);
		if (usedMaterial == material)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
			return true;
		}
	}

	uint32_t index = vkShader->usedMaterialCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(vkShader->scratchAllocator, vkShader->usedMaterials,
		vkShader->usedMaterialCount, vkShader->maxUsedMaterials, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
		return false;
	}

	vkShader->usedMaterials[index] = dsLifetime_addRef(material->lifetime);

	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));

	return true;
}

void dsVkShader_removeMaterial(dsShader* shader, dsDeviceMaterial* material)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	DS_VERIFY(dsSpinlock_lock(&vkShader->materialLock));
	for (uint32_t i = 0; i < vkShader->usedMaterialCount; ++i)
	{
		void* usedMaterial = dsLifetime_getObject(vkShader->usedMaterials[i]);
		DS_ASSERT(usedMaterial);
		if (usedMaterial == material)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(vkShader->usedMaterials,
				vkShader->usedMaterialCount, i, 1));
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
}

dsVkSamplerList* dsVkShader_getSamplerList(dsShader* shader, dsCommandBuffer* commandBuffer)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	dsRenderer* renderer = shader->resourceManager->renderer;

	if (vkShader->samplerCount == 0)
		return NULL;
	else if (vkShader->samplersHaveDefaultAnisotropy)
	{
		DS_VERIFY(dsSpinlock_lock(&vkShader->samplerLock));
		dsVkSamplerList* samplers = vkShader->samplers;
		if (!samplers || samplers->defaultAnisotropy != renderer->defaultAnisotropy)
		{
			if (samplers)
				dsVkRenderer_deleteSamplerList(renderer, samplers);
			samplers = vkShader->samplers = dsVkSamplerList_create(vkShader->scratchAllocator,
				shader);
			if (!samplers)
			{
				DS_VERIFY(dsSpinlock_unlock(&vkShader->samplerLock));
				return NULL;
			}
		}

		if (!dsVkCommandBuffer_addResource(commandBuffer, &samplers->resource))
			samplers = NULL;

		DS_VERIFY(dsSpinlock_unlock(&vkShader->samplerLock));
		return samplers;
	}

	DS_ASSERT(vkShader->samplers);
	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkShader->samplers->resource))
		return NULL;

	return vkShader->samplers;
}
