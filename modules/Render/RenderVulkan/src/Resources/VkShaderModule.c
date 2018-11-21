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

#include "Resources/VkShaderModule.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

#define DS_SHADER_ERROR ((VkShaderModule)0xFFFFFFFFFFFFFFFFULL)

dsShaderModule* dsVkShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name)
{
	uint32_t shaderCount = mslModule_shaderCount(module);
	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkShaderModule)) +
		DS_ALIGNED_SIZE(sizeof(VkShaderModule)*shaderCount);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkShaderModule* shaderModule = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkShaderModule);
	DS_ASSERT(shaderModule);

	dsShaderModule* baseShaderModule = (dsShaderModule*)shaderModule;
	baseShaderModule->resourceManager = resourceManager;
	baseShaderModule->allocator = allocator;
	baseShaderModule->module = module;
	baseShaderModule->name = name;

	if (shaderCount > 0)
	{
		shaderModule->shaders = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkShaderModule,
			shaderCount);
		DS_ASSERT(shaderModule->shaders);
		memset(shaderModule->shaders, 0, sizeof(VkShaderModule)*shaderCount);
	}
	else
		shaderModule->shaders = NULL;

	return baseShaderModule;
}

bool dsVkShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkShaderModule* vkModule = (dsVkShaderModule*)module;
	uint32_t shaderCount = mslModule_shaderCount(module->module);
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		if (vkModule->shaders[i])
		{
			DS_VK_CALL(device->vkDestroyShaderModule)(device->device, vkModule->shaders[i],
				instance->allocCallbacksPtr);
		}
	}

	if (module->allocator)
		DS_VERIFY(dsAllocator_free(module->allocator, module));
	return true;
}

bool dsVkShaderModule_getShader(VkShaderModule* outShader, dsShaderModule* module,
	uint32_t shaderIndex, const char* pipelineName)
{
	DS_STATIC_ASSERT(sizeof(VkShaderModule) == sizeof(uint64_t), unexpected_size_of_VkShaderModule);
	dsVkShaderModule* vkModule = (dsVkShaderModule*)module;

	// Check if the shader was already created.
	DS_ATOMIC_LOAD64(vkModule->shaders + shaderIndex, outShader);
	if (*outShader == DS_SHADER_ERROR)
	{
		*outShader = 0;
		return false;
	}
	else if (*outShader)
		return true;

	dsVkDevice* device = &((dsVkRenderer*)module->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	VkShaderModule shaderError = DS_SHADER_ERROR;

	const void* shaderData = mslModule_shaderData(module->module, shaderIndex);
	if (!shaderData)
	{
		errno = EFORMAT;
		DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "No shader data for shader %s.%s", module->name,
			pipelineName);
		DS_ATOMIC_STORE64(vkModule->shaders + shaderIndex, &shaderError);
		*outShader = 0;
		return false;
	}

	VkShaderModuleCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		mslModule_shaderSize(module->module, shaderIndex),
		(const uint32_t*)shaderData
	};
	VkResult result = device->vkCreateShaderModule(device->device, &createInfo,
		instance->allocCallbacksPtr, outShader);
	if (!dsHandleVkResult(result))
	{
		DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load shader %s.%s", module->name,
			pipelineName);
		DS_ATOMIC_STORE64(vkModule->shaders + shaderIndex, &shaderError);
		*outShader = 0;
		return false;
	}

	VkShaderModule expected = 0;
	if (!DS_ATOMIC_COMPARE_EXCHANGE64(vkModule->shaders + shaderIndex, &expected, outShader,
		false))
	{
		DS_VK_CALL(device->vkDestroyShaderModule)(device->device, *outShader,
			instance->allocCallbacksPtr);
		if (expected == DS_SHADER_ERROR)
		{
			*outShader = 0;
			return false;
		}

		*outShader = expected;
		return true;
	}

	return true;
}
