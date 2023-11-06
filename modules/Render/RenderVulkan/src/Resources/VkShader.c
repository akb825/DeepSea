/*
 * Copyright 2018-2023 Aaron Barany
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

#include "Resources/VkComputePipeline.h"
#include "Resources/VkDeviceMaterial.h"
#include "Resources/VkPipeline.h"
#include "Resources/VkSamplerList.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkRenderPass.h"
#include "VkRenderPassData.h"
#include "VkShared.h"
#include "VkSharedDescriptorSets.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/MaterialType.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

static VkPolygonMode polygonMode(mslPolygonMode mode)
{
	switch (mode)
	{
		case mslPolygonMode_Line:
			return VK_POLYGON_MODE_LINE;
		case mslPolygonMode_Point:
			return VK_POLYGON_MODE_POINT;
		case mslPolygonMode_Fill:
		default:
			return VK_POLYGON_MODE_FILL;
	}
}

static VkCullModeFlags cullMode(mslCullMode mode)
{
	switch (mode)
	{
		case mslCullMode_Front:
			return VK_CULL_MODE_FRONT_BIT;
		case mslCullMode_Back:
			return VK_CULL_MODE_BACK_BIT;
		case mslCullMode_FrontAndBack:
			return VK_CULL_MODE_FRONT_BIT | VK_CULL_MODE_BACK_BIT;
		case mslCullMode_None:
		default:
			return VK_CULL_MODE_NONE;
	}
}

static VkFrontFace frontFace(mslFrontFace face)
{
	switch (face)
	{
		case mslFrontFace_Clockwise:
			return VK_FRONT_FACE_CLOCKWISE;
		case mslFrontFace_CounterClockwise:
		default:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

VkStencilOp stencilOp(mslStencilOp op)
{
	switch (op)
	{
		case mslStencilOp_Zero:
			return VK_STENCIL_OP_ZERO;
		case mslStencilOp_Replace:
			return VK_STENCIL_OP_REPLACE;
		case mslStencilOp_IncrementAndClamp:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case mslStencilOp_DecrementAndClamp:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case mslStencilOp_Invert:
			return VK_STENCIL_OP_INVERT;
		case mslStencilOp_IncrementAndWrap:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case mslStencilOp_DecrementAndWrap:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		case mslStencilOp_Keep:
		default:
			return VK_STENCIL_OP_KEEP;
	}
}

VkLogicOp logicOp(mslLogicOp op)
{
	switch (op)
	{
		case mslLogicOp_Clear:
			return VK_LOGIC_OP_CLEAR;
		case mslLogicOp_And:
			return VK_LOGIC_OP_AND;
		case mslLogicOp_AndReverse:
			return VK_LOGIC_OP_AND_REVERSE;
		case mslLogicOp_AndInverted:
			return VK_LOGIC_OP_AND_INVERTED;
		case mslLogicOp_NoOp:
			return VK_LOGIC_OP_NO_OP;
		case mslLogicOp_Xor:
			return VK_LOGIC_OP_XOR;
		case mslLogicOp_Equivalent:
			return VK_LOGIC_OP_EQUIVALENT;
		case mslLogicOp_Invert:
			return VK_LOGIC_OP_INVERT;
		case mslLogicOp_OrReverse:
			return VK_LOGIC_OP_OR_REVERSE;
		case mslLogicOp_CopyInverted:
			return VK_LOGIC_OP_COPY_INVERTED;
		case mslLogicOp_OrInverted:
			return VK_LOGIC_OP_OR_INVERTED;
		case mslLogicOp_Nand:
			return VK_LOGIC_OP_NAND;
		case mslLogicOp_Set:
			return VK_LOGIC_OP_SET;
		case mslLogicOp_Copy:
		default:
			return VK_LOGIC_OP_COPY;
	}
}

VkBlendOp blendOp(mslBlendOp op)
{
	switch (op)
	{
		case mslBlendOp_Subtract:
			return VK_BLEND_OP_SUBTRACT;
		case mslBlendOp_ReverseSubtract:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case mslBlendOp_Min:
			return VK_BLEND_OP_MIN;
		case mslBlendOp_Max:
			return VK_BLEND_OP_MAX;
		case mslBlendOp_Add:
		default:
			return VK_BLEND_OP_ADD;
	}
}

static VkBlendFactor blendFactor(mslBlendFactor blendFactor, VkBlendFactor defaultValue)
{
	switch (blendFactor)
	{
		case mslBlendFactor_Zero:
			return VK_BLEND_FACTOR_ZERO;
		case mslBlendFactor_One:
			return VK_BLEND_FACTOR_ONE;
		case mslBlendFactor_SrcColor:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case mslBlendFactor_OneMinusSrcColor:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case mslBlendFactor_DstColor:
			return VK_BLEND_FACTOR_DST_COLOR;
		case mslBlendFactor_OneMinusDstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case mslBlendFactor_SrcAlpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case mslBlendFactor_OneMinusSrcAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case mslBlendFactor_DstAlpha:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case mslBlendFactor_OneMinusDstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case mslBlendFactor_ConstColor:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case mslBlendFactor_OneMinusConstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case mslBlendFactor_ConstAlpha:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case mslBlendFactor_OneMinusConstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		case mslBlendFactor_Src1Color:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case mslBlendFactor_OneMinusSrc1Color:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case mslBlendFactor_Src1Alpha:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case mslBlendFactor_OneMinusSrc1Alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return defaultValue;
	}
}

static bool hasConstantFactor(VkBlendFactor factor)
{
	switch (factor)
	{
		case VK_BLEND_FACTOR_CONSTANT_ALPHA:
		case VK_BLEND_FACTOR_CONSTANT_COLOR:
		case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
			return true;
		default:
			return false;
	}
}

static void copyStencilState(VkStencilOpState* vkStencilState,
	const mslStencilOpState* stencilState)
{
	vkStencilState->failOp = stencilOp(stencilState->failOp);
	vkStencilState->passOp = stencilOp(stencilState->passOp);
	vkStencilState->depthFailOp = stencilOp(stencilState->depthFailOp);
	vkStencilState->compareOp = dsVkCompareOp(stencilState->compareOp, VK_COMPARE_OP_NEVER);
	vkStencilState->compareMask = stencilState->compareMask;
	vkStencilState->writeMask = stencilState->writeMask;
	vkStencilState->reference = stencilState->reference;
}

static void copyBlendAttachmentState(VkPipelineColorBlendAttachmentState* vkBlendAttachment,
	const mslBlendAttachmentState* blendAttachment)
{
	vkBlendAttachment->blendEnable = blendAttachment->blendEnable == mslBool_True;
	vkBlendAttachment->srcColorBlendFactor = blendFactor(blendAttachment->srcColorBlendFactor,
		VK_BLEND_FACTOR_ZERO);
	vkBlendAttachment->dstColorBlendFactor = blendFactor(blendAttachment->dstColorBlendFactor,
		VK_BLEND_FACTOR_ONE);
	vkBlendAttachment->colorBlendOp = blendOp(blendAttachment->colorBlendOp);
	vkBlendAttachment->srcAlphaBlendFactor = blendFactor(blendAttachment->srcAlphaBlendFactor,
		VK_BLEND_FACTOR_ZERO);
	vkBlendAttachment->dstAlphaBlendFactor = blendFactor(blendAttachment->dstAlphaBlendFactor,
		VK_BLEND_FACTOR_ONE);
	vkBlendAttachment->alphaBlendOp = blendOp(blendAttachment->alphaBlendOp);
	if (blendAttachment->colorWriteMask == mslColorMask_Unset)
	{
		vkBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}
	else
		vkBlendAttachment->colorWriteMask = blendAttachment->colorWriteMask;
}

static size_t fullAllocSize(const mslModule* module, const mslPipeline* pipeline,
	const dsMaterialDesc* materialDesc, uint32_t pushConstantCount, uint32_t samplerCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkShader)) +
		DS_ALIGNED_SIZE(sizeof(dsVkPushConstantMapping)*pushConstantCount) +
		(samplerCount > 0 ?
			DS_ALIGNED_SIZE(sizeof(dsVkSamplerMapping)*materialDesc->elementCount) : 0U);
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] != MSL_UNKNOWN)
			fullSize += DS_ALIGNED_SIZE(mslModule_shaderSize(module, pipeline->shaders[i]));
	}
	return fullSize;
}

static void setupCommonStates(dsShader* shader)
{
	const mslModule* module = shader->module->module;
	uint32_t pipelineIndex = shader->pipelineIndex;
	const dsResourceManager* resourceManager = shader->resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	const VkPhysicalDeviceFeatures* features = &device->features;
	dsVkShader* vkShader = (dsVkShader*)shader;

	mslRenderState renderState;
	mslModule_renderState(&renderState, module, pipelineIndex);

	VkPipelineTessellationStateCreateInfo* tessellationInfo = &vkShader->tessellationInfo;
	tessellationInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellationInfo->pNext = NULL;
	tessellationInfo->flags = 0;
	tessellationInfo->patchControlPoints =
		renderState.patchControlPoints == MSL_UNKNOWN ? 1 : renderState.patchControlPoints;

	VkPipelineViewportStateCreateInfo* viewportInfo = &vkShader->viewportInfo;
	viewportInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo->pNext = NULL;
	viewportInfo->flags = 0;
	viewportInfo->viewportCount = 1;
	viewportInfo->pViewports = NULL;
	viewportInfo->scissorCount = 1;
	viewportInfo->pScissors = NULL;

	const mslRasterizationState* rasterizationState = &renderState.rasterizationState;
	VkPipelineRasterizationStateCreateInfo* rasterizationInfo = &vkShader->rasterizationInfo;
	rasterizationInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo->pNext = NULL;
	rasterizationInfo->flags = 0;
	rasterizationInfo->depthClampEnable = features->depthClamp &&
		rasterizationState->depthClampEnable == mslBool_True;
	rasterizationInfo->rasterizerDiscardEnable =
		rasterizationState->rasterizerDiscardEnable == mslBool_True;
	if (features->fillModeNonSolid)
		rasterizationInfo->polygonMode = polygonMode(rasterizationState->polygonMode);
	else
		rasterizationInfo->polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo->cullMode = cullMode(rasterizationState->cullMode);
	rasterizationInfo->frontFace = frontFace(rasterizationState->frontFace);
	rasterizationInfo->depthBiasEnable = rasterizationState->depthBiasEnable == mslBool_True;
	rasterizationInfo->depthBiasConstantFactor =
		rasterizationState->depthBiasConstantFactor == MSL_UNKNOWN_FLOAT ? 0.0f :
			rasterizationState->depthBiasConstantFactor;
	rasterizationInfo->depthBiasClamp =
		rasterizationState->depthBiasClamp == MSL_UNKNOWN_FLOAT || !features->depthBiasClamp ?
			0.0f : rasterizationState->depthBiasClamp;
	rasterizationInfo->depthBiasSlopeFactor =
		rasterizationState->depthBiasSlopeFactor == MSL_UNKNOWN_FLOAT ? 0.0f :
			rasterizationState->depthBiasSlopeFactor;
	rasterizationInfo->lineWidth =
		!features->wideLines || rasterizationState->lineWidth == MSL_UNKNOWN_FLOAT ? 1.0f :
			dsClamp(rasterizationState->lineWidth, resourceManager->lineWidthRange.x,
				resourceManager->lineWidthRange.y);

	const mslMultisampleState* multisampleState = &renderState.multisampleState;
	VkPipelineMultisampleStateCreateInfo* multisampleInfo = &vkShader->multisampleInfo;
	multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo->pNext = NULL;
	multisampleInfo->flags = 0;
	multisampleInfo->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo->sampleShadingEnable = features->sampleRateShading &&
		multisampleState->sampleShadingEnable == mslBool_True;
	multisampleInfo->minSampleShading = multisampleState->minSampleShading == MSL_UNKNOWN_FLOAT ?
		1.0f : dsClamp(multisampleState->minSampleShading, 0.0f, 1.0f);
	vkShader->sampleMask = multisampleState->sampleMask;
	multisampleInfo->pSampleMask = &vkShader->sampleMask;
	multisampleInfo->alphaToCoverageEnable =
		multisampleState->alphaToCoverageEnable == mslBool_True;
	multisampleInfo->alphaToOneEnable = multisampleState->alphaToOneEnable == mslBool_True;

	const mslDepthStencilState* depthStencilState = &renderState.depthStencilState;
	bool depthEnabled = depthStencilState->depthTestEnable == mslBool_True;
	VkPipelineDepthStencilStateCreateInfo* depthStencilInfo = &vkShader->depthStencilInfo;
	depthStencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo->pNext = NULL;
	depthStencilInfo->flags = 0;
	depthStencilInfo->depthTestEnable = depthEnabled;
	depthStencilInfo->depthWriteEnable = depthEnabled &&
		depthStencilState->depthWriteEnable != mslBool_False;
	depthStencilInfo->depthCompareOp = dsVkCompareOp(depthStencilState->depthCompareOp,
		VK_COMPARE_OP_LESS);
	depthStencilInfo->depthBoundsTestEnable = features->depthBounds && depthEnabled &&
		depthStencilState->depthBoundsTestEnable == mslBool_True;
	depthStencilInfo->stencilTestEnable = depthStencilState->stencilTestEnable == mslBool_True;
	copyStencilState(&depthStencilInfo->front, &depthStencilState->frontStencil);
	copyStencilState(&depthStencilInfo->back, &depthStencilState->backStencil);
	depthStencilInfo->minDepthBounds = depthStencilState->minDepthBounds == MSL_UNKNOWN_FLOAT ?
		0.0f : depthStencilState->minDepthBounds;
	depthStencilInfo->maxDepthBounds = depthStencilState->maxDepthBounds == MSL_UNKNOWN_FLOAT ?
		1.0f : depthStencilState->minDepthBounds;

	const mslBlendState* blendState = &renderState.blendState;
	VkPipelineColorBlendStateCreateInfo* blendInfo = &vkShader->blendInfo;
	blendInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo->pNext = NULL;
	blendInfo->flags = 0;
	blendInfo->logicOpEnable = features->logicOp && blendState->logicalOpEnable == mslBool_True;
	blendInfo->logicOp = logicOp(blendState->logicalOp);
	blendInfo->attachmentCount = shader->pipeline->fragmentOutputCount;
	copyBlendAttachmentState(vkShader->attachments, blendState->blendAttachments);
	if (features->independentBlend)
	{
		for (uint32_t i = 1; i < DS_MAX_ATTACHMENTS; ++i)
			copyBlendAttachmentState(vkShader->attachments + i, blendState->blendAttachments + i);
	}
	else
	{
		for (uint32_t i = 1; i < DS_MAX_ATTACHMENTS; ++i)
			vkShader->attachments[i] = vkShader->attachments[0];
	}
	blendInfo->pAttachments = vkShader->attachments;
	if (blendState->blendConstants[0] == MSL_UNKNOWN_FLOAT)
	{
		blendInfo->blendConstants[0] = 0.0f;
		blendInfo->blendConstants[1] = 0.0f;
		blendInfo->blendConstants[2] = 0.0f;
		blendInfo->blendConstants[3] = 1.0f;
	}
	else
	{
		for (uint32_t i = 0; i < 4; ++i)
			blendInfo->blendConstants[i] = blendState->blendConstants[i];
	}

	uint32_t dynamicStateCount = 0;
	vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	vkShader->dynamicLineWidth = !features->wideLines &&
		rasterizationState->lineWidth == MSL_UNKNOWN_FLOAT;
	if (vkShader->dynamicLineWidth)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_LINE_WIDTH;

	vkShader->dynamicDepthBias = rasterizationInfo->depthBiasEnable &&
		(rasterizationState->depthBiasConstantFactor == MSL_UNKNOWN_FLOAT ||
		rasterizationState->depthBiasClamp == MSL_UNKNOWN_FLOAT ||
		rasterizationState->depthBiasSlopeFactor == MSL_UNKNOWN_FLOAT);
	vkShader->depthBiasConstantFactor = rasterizationState->depthBiasConstantFactor;
	vkShader->depthBiasClamp = rasterizationState->depthBiasClamp;
	vkShader->depthBiasSlopeFactor = rasterizationState->depthBiasSlopeFactor;
	if (vkShader->dynamicDepthBias)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;

	vkShader->dynamicBlendConstants = false;
	if (blendState->blendConstants[0] == MSL_UNKNOWN_FLOAT)
	{
		for (uint32_t i = 0; i < blendInfo->attachmentCount; ++i)
		{
			if (hasConstantFactor(blendInfo->pAttachments[i].srcColorBlendFactor) ||
				hasConstantFactor(blendInfo->pAttachments[i].dstColorBlendFactor) ||
				hasConstantFactor(blendInfo->pAttachments[i].srcAlphaBlendFactor) ||
				hasConstantFactor(blendInfo->pAttachments[i].dstAlphaBlendFactor))
			{
				vkShader->dynamicBlendConstants = true;
				break;
			}
		}
	}
	if (vkShader->dynamicBlendConstants)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;

	vkShader->dynamicDepthBounds = depthStencilInfo->depthBoundsTestEnable &&
		(depthStencilState->minDepthBounds == MSL_UNKNOWN_FLOAT ||
		depthStencilState->maxDepthBounds == MSL_UNKNOWN_FLOAT);
	if (vkShader->dynamicDepthBounds)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
	vkShader->dynamicStencilCompareMask = depthStencilInfo->stencilTestEnable &&
		(depthStencilState->frontStencil.compareMask == MSL_UNKNOWN ||
		depthStencilState->backStencil.compareMask == MSL_UNKNOWN);
	if (vkShader->dynamicStencilCompareMask)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
	vkShader->dynamicStencilWriteMask = depthStencilInfo->stencilTestEnable &&
		(depthStencilState->frontStencil.writeMask == MSL_UNKNOWN ||
		depthStencilState->backStencil.writeMask == MSL_UNKNOWN);
	if (vkShader->dynamicStencilWriteMask)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
	vkShader->dynamicStencilReference = depthStencilInfo->stencilTestEnable &&
		(depthStencilState->frontStencil.reference == MSL_UNKNOWN ||
		depthStencilState->backStencil.reference == MSL_UNKNOWN);
	if (vkShader->dynamicStencilReference)
		vkShader->dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
	DS_ASSERT(dynamicStateCount <= DS_MAX_DYNAMIC_STATES);

	VkPipelineDynamicStateCreateInfo* dynamicStateInfo = &vkShader->dynamicInfo;
	dynamicStateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo->pNext = NULL;
	dynamicStateInfo->flags = 0;
	dynamicStateInfo->dynamicStateCount = dynamicStateCount;
	dynamicStateInfo->pDynamicStates = vkShader->dynamicStates;
}

static void setupSpirv(dsShader* shader, dsAllocator* allocator)
{
	const mslModule* module = shader->module->module;
	const mslPipeline* pipeline = shader->pipeline;
	uint32_t pipelineIndex = shader->pipelineIndex;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	dsVkShader* vkShader = (dsVkShader*)shader;

	// Copy the SPIRV to patch with the bindings in the next pass.
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] == MSL_UNKNOWN)
		{
			vkShader->spirv[i].data = NULL;
			vkShader->spirv[i].size = 0;
			continue;
		}

		const void* shaderSpirv = mslModule_shaderData(module, pipeline->shaders[i]);
		uint32_t shaderSize = mslModule_shaderSize(module, pipeline->shaders[i]);
		vkShader->spirv[i].data = dsAllocator_alloc(allocator, shaderSize);
		DS_ASSERT(vkShader->spirv[i].data);
		vkShader->spirv[i].size = shaderSize;
		memcpy(vkShader->spirv[i].data, shaderSpirv, shaderSize);
	}

	// Set up the descriptor set bindings.
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		uint32_t binding = vkMaterialDesc->elementMappings[i];
		if (binding == DS_MATERIAL_UNKNOWN)
			continue;

		dsMaterialBinding materialBinding = materialDesc->elements[i].binding;
		uint32_t descriptorSet = vkMaterialDesc->bindings[materialBinding].setIndex;
		for (uint32_t j = 0; j < pipeline->uniformCount; ++j)
		{
			mslUniform uniform;
			DS_VERIFY(mslModule_uniform(&uniform, module, pipelineIndex, j));
			if (strcmp(uniform.name, materialDesc->elements[i].name) != 0)
				continue;

			DS_VERIFY(mslModule_setUniformBindingCopy(module, pipelineIndex, j, descriptorSet,
				binding, vkShader->spirv));
		}
	}
}

static bool setupShaders(dsShader* shader)
{
	dsResourceManager* resourceManager = shader->resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkShader* vkShader = (dsVkShader*)shader;
	mslModule* module = shader->module->module;

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (!vkShader->spirv[i].data)
			continue;

		VkShaderModuleCreateInfo moduleCreateInfo =
		{
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			NULL,
			0,
			vkShader->spirv[i].size,
			(const uint32_t*)vkShader->spirv[i].data
		};
		VkResult result = device->vkCreateShaderModule(device->device, &moduleCreateInfo,
			instance->allocCallbacksPtr, vkShader->shaders + i);
		if (!DS_HANDLE_VK_RESULT(result, NULL))
		{
			DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't load shader %s.%s: %s",
				shader->module->name, shader->name, dsGetVkResultString(result));
			return false;
		}
	}

	vkShader->stages = 0;
	vkShader->pushConstantStages = 0;
	vkShader->computeUsesPushConstants = false;
	if (vkShader->spirv[mslStage_Vertex].data)
	{
		vkShader->stages |= VK_SHADER_STAGE_VERTEX_BIT;
		if (mslModule_shaderUsesPushConstants(module, vkShader->pipeline.shaders[mslStage_Vertex]))
			vkShader->pushConstantStages |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (vkShader->spirv[mslStage_TessellationControl].data)
	{
		vkShader->stages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if (mslModule_shaderUsesPushConstants(module,
				vkShader->pipeline.shaders[mslStage_TessellationControl]))
		{
			vkShader->pushConstantStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}
	}
	if (vkShader->spirv[mslStage_TessellationEvaluation].data)
	{
		vkShader->stages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if (mslModule_shaderUsesPushConstants(module,
				vkShader->pipeline.shaders[mslStage_TessellationEvaluation]))
		{
			vkShader->pushConstantStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}
	}
	if (vkShader->spirv[mslStage_Geometry].data)
	{
		vkShader->stages |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if (mslModule_shaderUsesPushConstants(module,
				vkShader->pipeline.shaders[mslStage_Geometry]))
		{
			vkShader->pushConstantStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
		}
	}
	if (vkShader->spirv[mslStage_Fragment].data)
	{
		vkShader->stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (mslModule_shaderUsesPushConstants(module,
				vkShader->pipeline.shaders[mslStage_Fragment]))
		{
			vkShader->pushConstantStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}
	if (vkShader->spirv[mslStage_Compute].data)
	{
		vkShader->computeUsesPushConstants = mslModule_shaderUsesPushConstants(module,
				vkShader->pipeline.shaders[mslStage_Compute]);
	}

	return true;
}

static bool createLayout(VkPipelineLayout* layout, dsShader* shader,
	VkPipelineStageFlags pushConstantStages)
{
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	const mslModule* module = shader->module->module;
	const mslPipeline* pipeline = shader->pipeline;
	const dsVkResourceManager* vkResourceManager =
		(const dsVkResourceManager*)shader->resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)shader->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	VkDescriptorSetLayout layouts[3];
	uint32_t descriptorCount = 0;
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(vkMaterialDesc->bindings); ++i)
	{
		if (vkMaterialDesc->bindings[i].descriptorSets)
			layouts[descriptorCount++] = vkMaterialDesc->bindings[i].descriptorSets;
	}

	uint32_t pushConstantSize = 0;
	if (pipeline->pushConstantStruct != MSL_UNKNOWN && pushConstantStages != 0)
	{
		mslStruct pushConstantStruct;
		DS_VERIFY(mslModule_struct(&pushConstantStruct, module, shader->pipelineIndex,
			pipeline->pushConstantStruct));
		pushConstantSize = pushConstantStruct.size;
	}

	if (pushConstantSize > vkResourceManager->maxPushConstantSize)
	{
		DS_LOG_ERROR_F(DS_RENDER_VULKAN_LOG_TAG,
			"Push constant size (%u bytes) exceeds maximum (%u bytes) for shader %s.%s",
			pushConstantSize, vkResourceManager->maxPushConstantSize, shader->module->name,
			shader->name);
		errno = ENOMEM;
		return false;
	}

	VkPushConstantRange pushConstantRange =
	{
		pushConstantStages,
		0,
		pushConstantSize
	};

	VkPipelineLayoutCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		descriptorCount,
		layouts,
		pushConstantSize == 0 ? 0U : 1U, &pushConstantRange
	};

	VkResult result = DS_VK_CALL(device->vkCreatePipelineLayout)(device->device, &createInfo,
		instance->allocCallbacksPtr, layout);
	return DS_HANDLE_VK_RESULT(result, "Couldn't create pipeline layout");
}

static bool bindPushConstants(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	const dsShader* shader, const dsMaterial* material, VkPipelineLayout layout,
	VkShaderStageFlags stages)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	const dsVkShader* vkShader = (const dsVkShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;

	if (vkShader->pushConstantSize == 0)
		return true;

	uint8_t* pushConstantData = dsVkCommandBuffer_allocatePushConstantData(commandBuffer,
		vkShader->pushConstantSize);
	if (!pushConstantData)
		return false;

	for (uint32_t i = 0; i < vkShader->pushConstantCount; ++i)
	{
		const dsVkPushConstantMapping* pushConstant = vkShader->pushConstants + i;
		const dsMaterialElement* element = materialDesc->elements + pushConstant->materialElement;
		if (pushConstant->count == 1 ||
			pushConstant->stride == dsMaterialType_cpuSize(element->type))
		{
			// Can copy all elements at once.
			DS_VERIFY(dsMaterial_getElementData(pushConstantData + pushConstant->offset, material,
				pushConstant->materialElement, element->type, 0, pushConstant->count));
		}
		else
		{
			// Must copy each element individually if the stride is different from the storage.
			for (uint32_t j = 0; j < pushConstant->count; ++j)
			{
				DS_VERIFY(dsMaterial_getElementData(pushConstantData + pushConstant->offset +
					pushConstant->stride*j, material, pushConstant->materialElement, element->type,
					j, 1));
			}
		}
	}

	DS_VK_CALL(device->vkCmdPushConstants)(submitBuffer, layout, stages, 0,
		vkShader->pushConstantSize, pushConstantData);
	return true;
}

static bool bindShaderStates(VkCommandBuffer submitBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	const dsResourceManager* resourceManager = shader->resourceManager;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	const dsVkShader* vkShader = (const dsVkShader*)shader;

	if (vkShader->dynamicLineWidth)
	{
		float lineWidth = renderStates ? dsClamp(renderStates->lineWidth,
				resourceManager->lineWidthRange.x, resourceManager->lineWidthRange.y) : 1.0f;
		DS_VK_CALL(device->vkCmdSetLineWidth)(submitBuffer, lineWidth);
	}

	if (vkShader->dynamicDepthBias)
	{
		float constantFactor;
		if (vkShader->depthBiasConstantFactor == MSL_UNKNOWN_FLOAT)
			constantFactor = renderStates ? renderStates->depthBiasConstantFactor : 0.0f;
		else
			constantFactor = vkShader->depthBiasConstantFactor;

		float clamp;
		if (vkShader->depthBiasClamp == MSL_UNKNOWN_FLOAT)
		{
			clamp = device->features.depthClamp && renderStates ?
				renderStates->depthBiasClamp : 0.0f;
		}
		else
			clamp = vkShader->depthBiasClamp;

		float slopeFactor;
		if (vkShader->depthBiasSlopeFactor == MSL_UNKNOWN_FLOAT)
			slopeFactor = renderStates ? renderStates->depthBiasSlopeFactor : 0.0f;
		else
			slopeFactor = vkShader->depthBiasSlopeFactor;

		DS_VK_CALL(device->vkCmdSetDepthBias)(submitBuffer, constantFactor, clamp, slopeFactor);
	}

	if (vkShader->dynamicBlendConstants)
	{
		dsVector4f constants;
		if (renderStates)
			constants = renderStates->blendConstants;
		else
		{
			constants.x = 0.0f;
			constants.y = 0.0f;
			constants.z = 0.0f;
			constants.w = 1.0f;
		}

		DS_VK_CALL(device->vkCmdSetBlendConstants)(submitBuffer, constants.values);
	}

	if (vkShader->dynamicDepthBounds)
	{
		dsVector2f bounds;
		if (renderStates)
			bounds = renderStates->depthBounds;
		else
		{
			bounds.x = 0.0f;
			bounds.y = 1.0f;
		}

		DS_VK_CALL(device->vkCmdSetDepthBounds)(submitBuffer, bounds.x, bounds.y);
	}

	if (vkShader->dynamicStencilCompareMask)
	{
		uint32_t mask = 0xFFFFFFFF;
		if (renderStates)
			mask = renderStates->frontStencilCompareMask;
		DS_VK_CALL(device->vkCmdSetStencilCompareMask)(submitBuffer, VK_STENCIL_FACE_FRONT_BIT,
			mask);

		if (renderStates)
			mask = renderStates->backStencilCompareMask;
		DS_VK_CALL(device->vkCmdSetStencilCompareMask)(submitBuffer, VK_STENCIL_FACE_BACK_BIT,
			mask);
	}

	if (vkShader->dynamicStencilWriteMask)
	{
		uint32_t mask = 0;
		if (renderStates)
			mask = renderStates->frontStencilWriteMask;
		DS_VK_CALL(device->vkCmdSetStencilWriteMask)(submitBuffer, VK_STENCIL_FACE_FRONT_BIT,
			mask);

		if (renderStates)
			mask = renderStates->backStencilWriteMask;
		DS_VK_CALL(device->vkCmdSetStencilWriteMask)(submitBuffer, VK_STENCIL_FACE_BACK_BIT,
			mask);
	}

	if (vkShader->dynamicStencilReference)
	{
		uint32_t mask = 0;
		if (renderStates)
			mask = renderStates->frontStencilReference;
		DS_VK_CALL(device->vkCmdSetStencilReference)(submitBuffer, VK_STENCIL_FACE_FRONT_BIT,
			mask);

		if (renderStates)
			mask = renderStates->backStencilReference;
		DS_VK_CALL(device->vkCmdSetStencilReference)(submitBuffer, VK_STENCIL_FACE_BACK_BIT,
			mask);
	}

	return true;
}

static bool updateGlobalValues(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	const dsShader* shader, const dsSharedMaterialValues* globalValues,
	VkPipelineBindPoint bindPoint)
{
	const dsVkShader* vkShader = (const dsVkShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	if (!vkMaterialDesc->bindings[dsMaterialBinding_Global].descriptorSets)
		return true;

	dsVkSharedDescriptorSets* descriptors = dsVkCommandBuffer_getGlobalDescriptorSets(
		commandBuffer);
	VkDescriptorSet descriptorSet = dsVkSharedDescriptorSets_createSet(descriptors, commandBuffer,
		(dsShader*)shader, globalValues);
	if (!descriptorSet)
		return false;

	uint32_t setIndex = vkMaterialDesc->bindings[dsMaterialBinding_Global].setIndex;
	VkPipelineLayout layout = bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? vkShader->layout :
		vkShader->computeLayout;
	dsVkCommandBuffer_bindDescriptorSet(commandBuffer, submitBuffer, bindPoint, setIndex, layout,
		descriptorSet, descriptors->offsets, descriptors->offsetCount);
	return true;
}

static bool updateInstanceValues(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues, VkPipelineBindPoint bindPoint)
{
	const dsVkShader* vkShader = (const dsVkShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	if (!vkMaterialDesc->bindings[dsMaterialBinding_Instance].descriptorSets)
		return true;

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	dsVkSharedDescriptorSets* descriptors = dsVkCommandBuffer_getInstanceDescriptorSets(
		commandBuffer);
	VkDescriptorSet descriptorSet = dsVkSharedDescriptorSets_createSet(descriptors, commandBuffer,
		(dsShader*)shader, instanceValues);
	if (!descriptorSet)
		return false;

	uint32_t setIndex = vkMaterialDesc->bindings[dsMaterialBinding_Instance].setIndex;
	VkPipelineLayout layout = bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? vkShader->layout :
		vkShader->computeLayout;
	dsVkCommandBuffer_bindDescriptorSet(commandBuffer, submitBuffer, bindPoint, setIndex, layout,
		descriptorSet, descriptors->offsets, descriptors->offsetCount);
	return true;
}

dsShader* dsVkShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc)
{
	mslPipeline pipeline;
	if (!mslModule_pipeline(&pipeline, module->module, shaderIndex))
		return NULL;

	uint32_t samplerCount = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		dsMaterialElement* element = materialDesc->elements + i;
		if (element->type != dsMaterialType_Texture)
			continue;

		// Make sure the uniform is in use.
		bool found = false;
		for (uint32_t j = 0; j < pipeline.uniformCount; ++j)
		{
			mslUniform uniform;
			DS_VERIFY(mslModule_uniform(&uniform, module->module, shaderIndex, j));
			if (strcmp(uniform.name, element->name) == 0)
			{
				found = true;
				break;
			}
		}
		samplerCount += found;
	}

	bool samplersHaveDefaultAnisotropy = false;
	for (uint32_t i = 0; i < pipeline.uniformCount; ++i)
	{
		mslUniform uniform;
		DS_VERIFY(mslModule_uniform(&uniform, module->module, shaderIndex, i));
		if (uniform.samplerIndex == MSL_UNKNOWN)
			continue;

		mslSamplerState sampler;
		if (!mslModule_samplerState(&sampler, module->module, shaderIndex, uniform.samplerIndex))
		{
			errno = EINDEX;
			return NULL;
		}

		if (sampler.mipFilter == mslMipFilter_Anisotropic &&
			sampler.maxAnisotropy == MSL_UNKNOWN_FLOAT)
		{
			samplersHaveDefaultAnisotropy = true;
		}
	}

	uint32_t pushConstantCount = 0;
	uint32_t pushConstantSize = 0;
	if (pipeline.pushConstantStruct != MSL_UNKNOWN)
	{
		mslStruct pushConstant;
		DS_VERIFY(mslModule_struct(&pushConstant, module->module, shaderIndex,
			pipeline.pushConstantStruct));
		pushConstantCount = pushConstant.memberCount;
		pushConstantSize = pushConstant.size;
	}

	dsAllocator* scratchAllocator = allocator;
	if (!scratchAllocator->freeFunc)
		scratchAllocator = resourceManager->allocator;

	size_t fullSize = fullAllocSize(module->module, &pipeline, materialDesc, pushConstantCount,
		samplerCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkShader* shader = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkShader);
	DS_ASSERT(shader);
	memset(shader, 0, sizeof(dsVkShader));

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

	if (pushConstantCount > 0)
	{
		shader->pushConstants = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsVkPushConstantMapping,
			pushConstantCount);
		DS_ASSERT(shader->pushConstants);
		shader->pushConstantCount = pushConstantCount;
		for (uint32_t i = 0; i < pushConstantCount; ++i)
		{
			dsVkPushConstantMapping* pushConstant = shader->pushConstants + i;
			mslStructMember member;
			DS_VERIFY(mslModule_structMember(&member, module->module, shaderIndex,
				pipeline.pushConstantStruct, i));
			pushConstant->materialElement = dsMaterialDesc_findElement(materialDesc, member.name);
			DS_ASSERT(pushConstant->materialElement != DS_MATERIAL_UNKNOWN);
			DS_ASSERT(dsMaterialDesc_convertMaterialType(member.type) ==
				materialDesc->elements[pushConstant->materialElement].type);
			pushConstant->offset = member.offset;

			// Should have been varified to have at most 1 array element in initial shader load.
			DS_ASSERT(member.arrayElementCount <= 1);
			if (member.arrayElementCount == 1)
			{
				mslArrayInfo arrayInfo;
				DS_VERIFY(mslModule_structMemberArrayInfo(&arrayInfo, module->module, shaderIndex,
					pipeline.pushConstantStruct, i, 0));
				pushConstant->count = arrayInfo.length;
				pushConstant->stride = arrayInfo.stride;
			}
			else
			{
				pushConstant->count = 1;
				pushConstant->stride = 0;
			}
		}
	}
	else
		shader->pushConstants = NULL;
	shader->pushConstantCount = pushConstantCount;
	shader->pushConstantSize = pushConstantSize;

	bool samplerError = false;
	if (samplerCount > 0)
	{
		shader->samplerMapping = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsVkSamplerMapping,
			materialDesc->elementCount);
		DS_ASSERT(shader->samplerMapping);

		uint32_t index = 0;
		for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
		{
			dsVkSamplerMapping* samplerMapping = shader->samplerMapping + i;
			samplerMapping->samplerIndex = DS_MATERIAL_UNKNOWN;
			samplerMapping->uniformIndex = DS_MATERIAL_UNKNOWN;
			const dsMaterialElement* element = materialDesc->elements + i;
			if (element->type != dsMaterialType_Texture)
				continue;

			for (uint32_t j = 0; j < pipeline.uniformCount; ++j)
			{
				mslUniform uniform;
				DS_VERIFY(mslModule_uniform(&uniform, module->module, shaderIndex, j));
				if (strcmp(uniform.name, element->name) != 0)
					continue;

				DS_ASSERT(uniform.uniformType == mslUniformType_SampledImage);
				samplerMapping->samplerIndex = index++;
				samplerMapping->uniformIndex = j;
				break;
			}
		}
		if (index != samplerCount)
			samplerError = true;
	}
	else
		shader->samplerMapping = NULL;
	shader->samplerCount = samplerCount;
	shader->samplersHaveDefaultAnisotropy = samplersHaveDefaultAnisotropy;

	memset(shader->shaders, 0, sizeof(shader->shaders));
	shader->layout = 0;
	shader->computeLayout = 0;
	shader->computePipeline = NULL;

	DS_VERIFY(dsSpinlock_initialize(&shader->materialLock));
	DS_VERIFY(dsSpinlock_initialize(&shader->pipelineLock));
	DS_VERIFY(dsSpinlock_initialize(&shader->samplerLock));

	// At this point it's safe to destroy the shader if there was a sampler error.
	if (samplerError)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Mismatch for shader samplers. Likely a subpass "
			"input was declared as a sampler instead.");
		dsVkShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	setupCommonStates(baseShader);
	setupSpirv(baseShader, (dsAllocator*)&bufferAlloc);
	if (!setupShaders(baseShader))
	{
		dsVkShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	if (shader->stages != 0 && !createLayout(&shader->layout, baseShader, shader->stages))
	{
		dsVkShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	if (shader->spirv[mslStage_Compute].data && !createLayout(&shader->computeLayout, baseShader,
			shader->computeUsesPushConstants ? VK_SHADER_STAGE_COMPUTE_BIT : 0))
	{
		dsVkShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	if (shader->shaders[mslStage_Compute])
	{
		shader->computePipeline = dsVkComputePipeline_create(allocator, baseShader);
		if (!shader->computePipeline)
		{
			dsVkShader_destroy(resourceManager, baseShader);
			return NULL;
		}
	}

	// If no dependency on default anisotropy, create immediately.
	if (samplerCount && !samplersHaveDefaultAnisotropy)
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

bool dsVkShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(resourceManager);
	const dsVkShader* vkShader = (const dsVkShader*)shader;
	dsDeviceMaterial* deviceMaterial = dsMaterial_getDeviceMaterial(material);
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	if (!bindPushConstants(commandBuffer, submitBuffer, shader, material, vkShader->layout,
			vkShader->stages))
	{
		return false;
	}

	if (!bindShaderStates(submitBuffer, shader, renderStates))
		return false;

	if (vkMaterialDesc->bindings[dsMaterialBinding_Material].descriptorSets)
	{
		VkDescriptorSet descriptorSet = dsVkDeviceMaterial_getDescriptorSet(commandBuffer,
			deviceMaterial, (dsShader*)shader);
		if (!descriptorSet)
			return false;

		uint32_t setIndex = vkMaterialDesc->bindings[dsMaterialBinding_Material].setIndex;
		dsVkCommandBuffer_bindDescriptorSet(commandBuffer, submitBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, setIndex, vkShader->layout, descriptorSet, NULL, 0);
	}

	if (globalValues && !updateGlobalValues(commandBuffer, submitBuffer, shader, globalValues,
			VK_PIPELINE_BIND_POINT_GRAPHICS))
	{
		return false;
	}

	return true;
}

bool dsVkShader_updateInstanceValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues)
{
	DS_UNUSED(resourceManager);
	return updateInstanceValues(commandBuffer, shader, instanceValues,
		VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool dsVkShader_updateDynamicRenderStates(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(resourceManager);
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	return bindShaderStates(submitBuffer, shader, renderStates);
}

bool dsVkShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsVkShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues)
{
	DS_UNUSED(resourceManager);
	const dsVkShader* vkShader = (const dsVkShader*)shader;
	dsDeviceMaterial* deviceMaterial = dsMaterial_getDeviceMaterial(material);
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	if (vkShader->computeUsesPushConstants && !bindPushConstants(commandBuffer, submitBuffer,
			shader, material, vkShader->computeLayout, VK_SHADER_STAGE_COMPUTE_BIT))
	{
		return false;
	}

	if (vkMaterialDesc->bindings[dsMaterialBinding_Material].descriptorSets)
	{
		VkDescriptorSet descriptorSet = dsVkDeviceMaterial_getDescriptorSet(commandBuffer,
			deviceMaterial, (dsShader*)shader);
		if (!descriptorSet)
			return false;

		uint32_t setIndex = vkMaterialDesc->bindings[dsMaterialBinding_Material].setIndex;
		dsVkCommandBuffer_bindDescriptorSet(commandBuffer, submitBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE, setIndex, vkShader->computeLayout, descriptorSet, NULL,
			0);
	}

	if (globalValues && !updateGlobalValues(commandBuffer, submitBuffer, shader, globalValues,
			VK_PIPELINE_BIND_POINT_COMPUTE))
	{
		return false;
	}

	return true;
}

bool dsVkShader_updateComputeInstanceValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues)
{
	DS_UNUSED(resourceManager);
	return updateInstanceValues(commandBuffer, shader, instanceValues,
		VK_PIPELINE_BIND_POINT_COMPUTE);
}

bool dsVkShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsVkShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
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

	DS_VERIFY(dsSpinlock_lock(&vkShader->pipelineLock));
	dsLifetime** usedRenderPasses = vkShader->usedRenderPasses;
	uint32_t usedRenderPassCount = vkShader->usedRenderPassCount;
	vkShader->usedRenderPasses = NULL;
	vkShader->usedRenderPassCount = 0;
	vkShader->maxUsedRenderPasses = 0;

	dsVkPipeline** pipelines = vkShader->pipelines;
	uint32_t pipelineCount = vkShader->pipelineCount;
	vkShader->pipelines = NULL;
	vkShader->pipelineCount = 0;
	vkShader->maxPipelines = 0;
	DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));

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

	for (uint32_t i = 0; i < usedRenderPassCount; ++i)
	{
		dsVkRenderPassData* renderPass =
			(dsVkRenderPassData*)dsLifetime_acquire(usedRenderPasses[i]);
		if (renderPass)
		{
			dsVkRenderPassData_removeShader(renderPass, shader);
			dsLifetime_release(usedRenderPasses[i]);
		}
		dsLifetime_freeRef(usedRenderPasses[i]);
	}
	DS_VERIFY(dsAllocator_free(vkShader->scratchAllocator, usedRenderPasses));
	DS_ASSERT(!vkShader->usedRenderPasses);

	dsLifetime_destroy(vkShader->lifetime);

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (vkShader->shaders[i])
		{
			DS_VK_CALL(device->vkDestroyShaderModule)(device->device, vkShader->shaders[i],
				instance->allocCallbacksPtr);
		}
	}

	if (vkShader->layout)
	{
		DS_VK_CALL(device->vkDestroyPipelineLayout)(device->device, vkShader->layout,
			instance->allocCallbacksPtr);
	}

	if (vkShader->computeLayout)
	{
		DS_VK_CALL(device->vkDestroyPipelineLayout)(device->device, vkShader->computeLayout,
			instance->allocCallbacksPtr);
	}

	dsVkRenderer_deleteSamplerList(renderer, vkShader->samplers);
	dsVkRenderer_deleteComputePipeline(renderer, vkShader->computePipeline);

	for (uint32_t i = 0; i < pipelineCount; ++i)
		dsVkRenderer_deletePipeline(renderer, pipelines[i]);
	DS_VERIFY(dsAllocator_free(vkShader->scratchAllocator, pipelines));

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
		if (vkShader->usedMaterials[i] == material->lifetime)
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
		dsLifetime* materialLifetime = vkShader->usedMaterials[i];
		if (materialLifetime == material->lifetime)
		{
			vkShader->usedMaterials[i] = vkShader->usedMaterials[vkShader->usedMaterialCount - 1];
			--vkShader->usedMaterialCount;
			dsLifetime_freeRef(materialLifetime);
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkShader->materialLock));
}

void dsVkShader_removeRenderPass(dsShader* shader, dsVkRenderPassData* renderPass)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	dsRenderer* renderer = shader->resourceManager->renderer;
	DS_VERIFY(dsSpinlock_lock(&vkShader->pipelineLock));

	// Unregister the render pass.
	bool wasRegistered = false;
	for (uint32_t i = 0; i < vkShader->usedRenderPassCount; ++i)
	{
		dsLifetime* renderPassLifetime = vkShader->usedRenderPasses[i];
		if (renderPassLifetime == renderPass->lifetime)
		{
			vkShader->usedRenderPasses[i] =
				vkShader->usedRenderPasses[vkShader->usedRenderPassCount - 1];
			--vkShader->usedRenderPassCount;
			dsLifetime_freeRef(renderPassLifetime);
			wasRegistered = true;
			break;
		}
	}

	if (!wasRegistered)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
		return;
	}

	// Remove all pipelines for the render pass.
	for (uint32_t i = 0; i < vkShader->pipelineCount;)
	{
		if (vkShader->pipelines[i]->renderPass == renderPass->lifetime)
		{
			dsVkRenderer_deletePipeline(renderer, vkShader->pipelines[i]);
			vkShader->pipelines[i] = vkShader->pipelines[vkShader->pipelineCount - 1];
			--vkShader->pipelineCount;
		}
		else
			++i;
	}

	DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
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

VkPipeline dsVkShader_getComputePipeline(dsShader* shader, dsCommandBuffer* commandBuffer)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	if (!vkShader->computePipeline)
		return 0;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkShader->computePipeline->resource))
		return 0;

	return vkShader->computePipeline->pipeline;
}

VkPipeline dsVkShader_getPipeline(dsShader* shader, dsCommandBuffer* commandBuffer,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry)
{
	const dsRenderPass* renderPass = commandBuffer->boundRenderPass;
	if (!renderPass)
		return 0;

	dsVkRenderPassData* renderPassData = dsVkRenderPass_getData(renderPass);
	if (!renderPassData)
		return 0;

	dsVkShader* vkShader = (dsVkShader*)shader;
	if (!vkShader->shaders[mslStage_Vertex])
		return 0;

	dsRenderer* renderer = commandBuffer->renderer;
	uint32_t subpassIndex = commandBuffer->activeRenderSubpass;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;

	// Get the number of samples based on the attachments.
	const dsAttachmentInfo* attachments = renderPass->attachments;
	uint32_t referenceAttachment = DS_NO_ATTACHMENT;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t colorAttachment = subpass->colorAttachments[i].attachmentIndex;
		if (colorAttachment != DS_NO_ATTACHMENT)
		{
			referenceAttachment = colorAttachment;
			break;
		}
	}
	if (referenceAttachment == DS_NO_ATTACHMENT)
		referenceAttachment = subpass->depthStencilAttachment.attachmentIndex;

	uint32_t samples = DS_DEFAULT_ANTIALIAS_SAMPLES;
	if (referenceAttachment != DS_NO_ATTACHMENT)
		samples = attachments[referenceAttachment].samples;

	if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
		samples = renderer->surfaceSamples;
	else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = renderer->defaultSamples;

	// Don't use default anisotropy if default isn't used within the shaders.
	float anisotropy = renderer->defaultAnisotropy;
	if (!vkShader->samplersHaveDefaultAnisotropy)
		anisotropy = 1.0f;

	dsVkPipelineKey pipelineKey;
	dsVkPipeline_initializeKey(&pipelineKey, samples, anisotropy, primitiveType, geometry,
		renderPassData, subpassIndex);
	uint32_t hash = dsVkPipeline_hash(&pipelineKey);

	DS_VERIFY(dsSpinlock_lock(&vkShader->pipelineLock));

	// Search for an existing pipeline
	for (uint32_t i = 0; i < vkShader->pipelineCount; ++i)
	{
		dsVkPipeline* pipeline = vkShader->pipelines[i];
		if (dsVkPipeline_isEquivalent(pipeline, hash, &pipelineKey, geometry))
		{
			VkPipeline vkPipeline = pipeline->pipeline;
			if (!dsVkCommandBuffer_addResource(commandBuffer, &pipeline->resource))
				vkPipeline = 0;
			DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
			return vkPipeline;
		}
	}

	// Add a new pipeline if not present.
	uint32_t index = vkShader->pipelineCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(vkShader->scratchAllocator, vkShader->pipelines,
		vkShader->pipelineCount, vkShader->maxPipelines, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
		return 0;
	}

	vkShader->pipelines[index] = dsVkPipeline_create(vkShader->scratchAllocator, shader,
		index > 0 ? vkShader->pipelines[0]->pipeline : 0, hash, samples, anisotropy, primitiveType,
		geometry, renderPassData, subpassIndex);
	if (!vkShader->pipelines[index])
	{
		--vkShader->pipelineCount;
		DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
		return 0;
	}

	// Register the render pass.
	bool hasRenderPass = false;
	for (uint32_t i = 0; i < vkShader->usedRenderPassCount; ++i)
	{
		if (vkShader->usedRenderPasses[i] == renderPassData->lifetime)
		{
			hasRenderPass = true;
			break;
		}
	}

	if (!hasRenderPass)
	{
		uint32_t passIndex = vkShader->usedRenderPassCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(vkShader->scratchAllocator, vkShader->usedRenderPasses,
			vkShader->usedRenderPassCount, vkShader->maxUsedRenderPasses, 1))
		{
			dsVkPipeline_destroy(vkShader->pipelines[index]);
			--vkShader->pipelineCount;
			DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));
			return 0;
		}

		vkShader->usedRenderPasses[passIndex] = dsLifetime_addRef(renderPassData->lifetime);
	}

	VkPipeline vkPipeline = vkShader->pipelines[index]->pipeline;
	DS_VERIFY(dsSpinlock_unlock(&vkShader->pipelineLock));

	if (!hasRenderPass)
		dsVkRenderPassData_addShader(renderPassData, shader);

	return vkPipeline;
}
