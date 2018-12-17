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

#include "Resources/VkComputePipeline.h"
#include "Resources/VkResource.h"
#include "VkRendererInternal.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsVkComputePipeline* dsVkComputePipeline_create(dsAllocator* allocator, dsShader* shader)
{
	dsVkComputePipeline* pipeline = DS_ALLOCATE_OBJECT(allocator, dsVkComputePipeline);
	if (!pipeline)
		return NULL;

	dsResourceManager* resourceManager = shader->resourceManager;
	dsVkResourceManager* vkResourceManager = (dsVkResourceManager*)resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkShader* vkShader = (dsVkShader*)shader;

	pipeline->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&pipeline->resource);
	pipeline->device = device;
	pipeline->pipeline = 0;

	VkComputePipelineCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		NULL,
		0,
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			NULL,
			0,
			VK_SHADER_STAGE_COMPUTE_BIT,
			vkShader->shaders[mslStage_Compute],
			"main",
			NULL
		},
		vkShader->layout,
		0,
		-1
	};

	VkResult result = DS_VK_CALL(device->vkCreateComputePipelines)(device->device,
		vkResourceManager->pipelineCache, 1, &createInfo, instance->allocCallbacksPtr,
		&pipeline->pipeline);
	if (!dsHandleVkResult(result))
	{
		dsVkComputePipeline_destroy(pipeline);
		return NULL;
	}

	return pipeline;
}

void dsVkComputePipeline_destroy(dsVkComputePipeline* pipeline)
{
	if (!pipeline)
		return;

	dsVkDevice* device = pipeline->device;
	dsVkInstance* instance = &device->instance;

	if (pipeline->pipeline)
	{
		DS_VK_CALL(device->vkDestroyPipeline)(device->device, pipeline->pipeline,
			instance->allocCallbacksPtr);
	}

	dsVkResource_shutdown(&pipeline->resource);
	if (pipeline->allocator)
		DS_VERIFY(dsAllocator_free(pipeline->allocator, pipeline));
}
