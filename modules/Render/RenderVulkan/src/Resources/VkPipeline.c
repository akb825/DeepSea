/*
 * Copyright 2018-2019 Aaron Barany
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

#include "Resources/VkPipeline.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkRenderPass.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

uint32_t dsVkPipeline_hash(uint32_t samples, float defaultAnisotropy,
	dsPrimitiveType primitiveType, uint32_t vertexFormatHash, const dsRenderPass* renderPass,
	uint32_t subpass)
{
	uint32_t hash = dsHash32(&samples);
	hash = dsHashCombine(hash, dsHash32(&defaultAnisotropy));
	uint32_t primitiveType32 = primitiveType;
	hash = dsHashCombine(hash, dsHash32(&primitiveType32));
	hash = dsHashCombine(hash, vertexFormatHash);
	hash = dsHashCombine(hash, dsHashPointer(renderPass));
	return dsHashCombine(hash, dsHash32(&subpass));
}

dsVkPipeline* dsVkPipeline_create(dsAllocator* allocator, dsShader* shader,
	VkPipeline existingPipeline, uint32_t hash, uint32_t samples, float defaultAnisotropy,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry, const dsRenderPass* renderPass,
	uint32_t subpass)
{
	dsVkPipeline* pipeline = DS_ALLOCATE_OBJECT(allocator, dsVkPipeline);
	if (!pipeline)
		return NULL;

	dsResourceManager* resourceManager = shader->resourceManager;
	dsVkResourceManager* vkResourceManager = (dsVkResourceManager*)resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkShader* vkShader = (dsVkShader*)shader;
	dsVkRenderPassData* renderPassData = dsVkRenderPass_getData(renderPass);

	pipeline->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&pipeline->resource);
	pipeline->device = device;
	pipeline->pipeline = 0;
	pipeline->hash = hash;
	pipeline->samples = samples;
	pipeline->defaultAnisotropy = defaultAnisotropy;
	pipeline->primitiveType = primitiveType;
	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		memcpy(pipeline->formats + i, &geometry->vertexBuffers[i].format,
			sizeof(dsVertexFormat));
	}
	pipeline->renderPass = dsLifetime_addRef(renderPassData->lifetime);
	pipeline->subpass = subpass;

	uint32_t stageCount = 0;
	VkPipelineShaderStageCreateInfo stages[mslStage_Count];
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (i == mslStage_Compute || !vkShader->shaders[i])
			continue;

		stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[stageCount].pNext = NULL;
		stages[stageCount].flags = 0;
		stages[stageCount].stage = dsVkShaderStage((mslStage)i);
		stages[stageCount].module = vkShader->shaders[i];
		stages[stageCount].pName = "main";
		stages[stageCount].pSpecializationInfo = NULL;
		++stageCount;
	}

	DS_ASSERT(stageCount > 0);

	VkVertexInputBindingDescription vertexBindings[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	VkVertexInputAttributeDescription attributes[DS_MAX_ALLOWED_VERTEX_ATTRIBS];
	uint32_t bindingCount = 0;
	uint32_t attributeCount = 0;
	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		const dsVertexFormat* format = pipeline->formats + i;
		if (format->enabledMask == 0)
			continue;

		DS_ASSERT(format->size > 0);
		vertexBindings[bindingCount].binding = bindingCount;
		vertexBindings[bindingCount].stride = format->size;
		vertexBindings[bindingCount].inputRate = format->divisor > 0;
		for (uint32_t mask = format->enabledMask; mask; mask = dsRemoveLastBit(mask))
		{
			uint32_t attribute = dsBitmaskIndex(mask);

			const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager,
				format->elements[attribute].format);
			if (!formatInfo)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Unknown format.");
				dsVkPipeline_destroy(pipeline);
				return NULL;
			}

			attributes[attributeCount].location = attribute;
			attributes[attributeCount].binding = bindingCount;
			attributes[attributeCount].format = formatInfo->vkFormat;
			attributes[attributeCount].offset = format->elements[attribute].offset;
			++attributeCount;
			DS_ASSERT(attributeCount <= DS_MAX_ALLOWED_VERTEX_ATTRIBS);
		}

		++bindingCount;
	}

	VkPipelineVertexInputStateCreateInfo vertexInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		NULL,
		0,
		bindingCount, vertexBindings,
		attributeCount, attributes
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0,
		dsVkPrimitiveType(primitiveType),
		primitiveType == dsPrimitiveType_LineStrip ||
			primitiveType == dsPrimitiveType_TriangleStrip ||
			primitiveType == dsPrimitiveType_TriangleFan
	};

	VkPipelineMultisampleStateCreateInfo multisampleInfo = vkShader->multisampleInfo;
	multisampleInfo.rasterizationSamples = dsVkSampleCount(samples);

	VkPipelineCreateFlagBits flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	if (existingPipeline)
		flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	VkGraphicsPipelineCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		flags,
		stageCount, stages,
		&vertexInfo,
		&inputAssemblyInfo,
		&vkShader->tessellationInfo,
		&vkShader->viewportInfo,
		&vkShader->rasterizationInfo,
		&multisampleInfo,
		&vkShader->depthStencilInfo,
		&vkShader->blendInfo,
		&vkShader->dynamicInfo,
		vkShader->layout,
		renderPassData->vkRenderPass,
		subpass,
		existingPipeline,
		-1
	};

	VkResult result = DS_VK_CALL(device->vkCreateGraphicsPipelines)(device->device,
		vkResourceManager->pipelineCache, 1, &createInfo, instance->allocCallbacksPtr,
		&pipeline->pipeline);
	if (!dsHandleVkResult(result))
	{
		dsVkPipeline_destroy(pipeline);
		return NULL;
	}

	return pipeline;
}

bool dsVkPipeline_isEquivalent(const dsVkPipeline* pipeline, uint32_t hash, uint32_t samples,
	float defaultAnisotropy, dsPrimitiveType primitiveType, const dsDrawGeometry* geometry,
	const dsVkRenderPassData* renderPassData, uint32_t subpass)
{
	if (pipeline->hash != hash)
		return false;

	if (pipeline->samples != samples || pipeline->defaultAnisotropy != defaultAnisotropy ||
		pipeline->primitiveType != primitiveType || pipeline->subpass != subpass ||
		pipeline->renderPass != renderPassData->lifetime)
	{
		return false;
	}

	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (memcmp(pipeline->formats + i, &geometry->vertexBuffers[i].format,
			sizeof(dsVertexFormat)) != 0)
		{
			return false;
		}
	}

	return true;
}

void dsVkPipeline_destroy(dsVkPipeline* pipeline)
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

	dsLifetime_freeRef(pipeline->renderPass);

	dsVkResource_shutdown(&pipeline->resource);
	if (pipeline->allocator)
		DS_VERIFY(dsAllocator_free(pipeline->allocator, pipeline));
}
