/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/Render/Resources/Shader.h>

#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Types.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

// Ensure no padding for dsDynamicRenderStates.
_Static_assert(sizeof(dsDynamicRenderStates) == sizeof(uint32_t)*16,
	"Unexpected sizeof(dsDynamicRenderStates).");

extern const char* dsResourceManager_noContextError;

static const dsShaderVariableElement* findShaderVariableElement(const dsMaterialDesc* materialDesc,
	const char* uniformName, const char* name)
{
	const dsShaderVariableElement* element = NULL;
	bool isDuplicate = false;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (materialDesc->elements[i].type != dsMaterialType_VariableGroup)
			continue;

		const dsShaderVariableGroupDesc* groupDesc =
			materialDesc->elements[i].shaderVariableGroupDesc;
		DS_ASSERT(groupDesc);
		uint32_t foundElement = dsShaderVariableGroupDesc_findElement(groupDesc, name);
		if (foundElement != DS_MATERIAL_UNKNOWN)
		{
			if (element)
				isDuplicate = true;
			else
				element = groupDesc->elements + foundElement;
		}
	}

	if (isDuplicate)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Shader variable group element '%s' found in multiple groups.", name);
		return NULL;
	}
	else if (!element)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Uniform '%s.%s' not found in material description or any shader variable group.",
			uniformName, name);
		return NULL;
	}

	return element;
}

static bool arePushConstantsCompatible(dsResourceManager* resourceManager, const mslModule* module,
	uint32_t pipelineIndex, uint32_t structIndex, const dsMaterialDesc* materialDesc,
	bool supportsBuffers, const char* uniformName, const char* moduleName, const char* shaderName)
{
	DS_ASSERT(materialDesc);
	mslStruct structInfo;
	DS_VERIFY(mslModule_struct(&structInfo, module, pipelineIndex, structIndex));

	bool success = true;
	for (uint32_t i = 0; i < structInfo.memberCount; ++i)
	{
		mslStructMember structMember;
		DS_VERIFY(mslModule_structMember(&structMember, module, pipelineIndex, structIndex, i));
		if (resourceManager->isShaderUniformInternalFunc &&
			resourceManager->isShaderUniformInternalFunc(resourceManager, structMember.name))
		{
			continue;
		}

		dsMaterialType type = dsMaterialType_Count;
		uint32_t arrayCount = 0;
		uint32_t elementIndex = dsMaterialDesc_findElement(materialDesc, structMember.name);
		if (elementIndex == DS_MATERIAL_UNKNOWN)
		{
			if (supportsBuffers)
			{
				DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
					"Uniform '%s.%s' not found in material description for shader '%s.%s'.",
					uniformName, structMember.name, moduleName, shaderName);
				success = false;
				continue;
			}
			else
			{
				const dsShaderVariableElement* element = findShaderVariableElement(materialDesc,
					uniformName, structMember.name);
				if (!element)
				{
					// Error printed from above function call.
					success = false;
					continue;
				}

				type = element->type;
				arrayCount = element->count;
			}
		}
		else
		{
			const dsMaterialElement* element = materialDesc->elements + elementIndex;
			type = element->type;
			arrayCount = element->count;
		}


		uint32_t structMemberArrayCount = 0;
		if (structMember.arrayElementCount > 1)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Multi-dimensional arrays aren't supported for uniform '%s.%s' for shader '%s.%s'.",
				uniformName, structMember.name, moduleName, shaderName);
			success = false;
			continue;
		}
		else if (structMember.arrayElementCount == 1)
		{
			mslArrayInfo arrayInfo;
			DS_VERIFY(mslModule_structMemberArrayInfo(&arrayInfo, module, pipelineIndex,
				structIndex, i, 0));
			structMemberArrayCount = arrayInfo.length;
		}

		if (dsMaterialDesc_convertMaterialType(structMember.type) != type ||
			structMemberArrayCount != arrayCount)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Types for uniform '%s.%s' differ between shader and material for shader '%s.%s'.",
				uniformName, structMember.name, moduleName, shaderName);
			success = false;
			continue;
		}
	}

	return success;
}

static bool isShaderVariableGroupCompatible(const mslModule* module, uint32_t pipelineIndex,
	uint32_t structIndex, const dsShaderVariableGroupDesc* groupDesc, const char* uniformName,
	const char* moduleName, const char* pipelineName)
{
	DS_ASSERT(groupDesc);
	mslStruct structInfo;
	DS_VERIFY(mslModule_struct(&structInfo, module, pipelineIndex, structIndex));

	if (structInfo.memberCount != groupDesc->elementCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Variable group '%s' doesn't match shader uniform block for shader '%s.%s'.",
			uniformName, moduleName, pipelineName);
		return false;
	}

	bool success = true;
	for (uint32_t i = 0; i < structInfo.memberCount; ++i)
	{
		const dsShaderVariableElement* element = groupDesc->elements + i;

		mslStructMember structMember;
		DS_VERIFY(mslModule_structMember(&structMember, module, pipelineIndex, structIndex, i));

		uint32_t structMemberArrayCount = 0;
		if (structMember.arrayElementCount > 1)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Multi-dimensional arrays aren't supported for "
				"variable group member '%s.%s' for shader '%s.%s'.", uniformName, structMember.name,
				moduleName, pipelineName);
			success = false;
			continue;
		}
		else if (structMember.arrayElementCount == 1)
		{
			mslArrayInfo arrayInfo;
			DS_VERIFY(mslModule_structMemberArrayInfo(&arrayInfo, module, pipelineIndex,
				structIndex, i, 0));
			structMemberArrayCount = arrayInfo.length;
		}

		if (dsMaterialDesc_convertMaterialType(structMember.type) != element->type ||
			structMemberArrayCount != element->count)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Types for element '%s.%s' differ between shader and "
				"shader variable group for shader '%s.%s'.", uniformName, structMember.name,
				moduleName, pipelineName);
			success = false;
		}

		if (((element->type >= dsMaterialType_Mat2x3 && element->type <= dsMaterialType_Mat4x3) ||
			(element->type >= dsMaterialType_DMat2x3 && element->type <= dsMaterialType_DMat4x3)) &&
			structMember.rowMajor)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Element '%s.%s' is row major for shader '%s.%s'. "
				"Non-square matrix elements within a shader variable group must be column-major.",
				uniformName, structMember.name, moduleName, pipelineName);
			success = false;
		}
	}

	return success;
}

static bool isMaterialDescCompatible(dsResourceManager* resourceManager, const mslModule* module,
	const mslPipeline* pipeline, uint32_t index, const dsMaterialDesc* materialDesc,
	bool supportsBuffers, const char* moduleName)
{
	bool nativeSubpassInput = mslModule_targetId(module) == DS_FOURCC('S', 'P', 'R', 'V');
	bool success = true;
	for (uint32_t i = 0; i < pipeline->uniformCount; ++i)
	{
		mslUniform uniform;
		DS_VERIFY(mslModule_uniform(&uniform, module, index, i));
		if (uniform.uniformType == mslUniformType_PushConstant)
		{
			if (!arePushConstantsCompatible(resourceManager, module, index, uniform.structIndex,
					materialDesc, supportsBuffers, uniform.name, moduleName, pipeline->name))
			{
				success = false;
			}
			continue;
		}

		uint32_t elementIndex = dsMaterialDesc_findElement(materialDesc, uniform.name);
		if (elementIndex == DS_MATERIAL_UNKNOWN)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Uniform '%s' not found in material description for shader '%s.%s'.",
				uniform.name, moduleName, pipeline->name);
			success = false;
			continue;
		}

		bool typesMatch = false;
		const dsMaterialElement* element = materialDesc->elements + elementIndex;
		switch (uniform.uniformType)
		{
			case mslUniformType_Block:
				typesMatch = element->type == dsMaterialType_VariableGroup ||
					element->type == dsMaterialType_UniformBlock;
				if (typesMatch && element->type == dsMaterialType_VariableGroup)
				{
					if (uniform.arrayElementCount != 0)
					{
						DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
							"Shader variable group '%s' may not be an array for shader '%s.%s'.",
							uniform.name, moduleName, pipeline->name);
						success = false;
					}

					if (!isShaderVariableGroupCompatible(module, index, uniform.structIndex,
							element->shaderVariableGroupDesc, uniform.name, moduleName,
							pipeline->name))
					{
						success = false;
					}
				}
				break;
			case mslUniformType_BlockBuffer:
				typesMatch = element->type == dsMaterialType_UniformBuffer;
				break;
			case mslUniformType_Image:
				typesMatch = element->type == dsMaterialType_Image ||
					element->type == dsMaterialType_ImageBuffer;
				if (!nativeSubpassInput && element->type == dsMaterialType_SubpassInput)
					typesMatch = true;
				break;
			case mslUniformType_SampledImage:
				typesMatch = element->type == dsMaterialType_Texture ||
					element->type == dsMaterialType_TextureBuffer;
				if (!nativeSubpassInput && element->type == dsMaterialType_SubpassInput)
					typesMatch = true;
				break;
			case mslUniformType_SubpassInput:
				typesMatch = element->type == dsMaterialType_SubpassInput;
				break;
			default:
				DS_ASSERT(false);
		}

		if (!typesMatch)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Types for uniform '%s' differ between shader and material for shader '%s.%s'.",
				uniform.name, moduleName, pipeline->name);
			success = false;
		}
	}

	return success;
}

static bool verifySharedMaterialBufferValue(const dsMaterialElement* element, dsGfxBuffer* buffer,
	const char* moduleName, const char* pipelineName)
{
	if (element->type == dsMaterialType_UniformBlock &&
		!(buffer->usage & dsGfxBufferUsage_UniformBlock))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Buffer '%s' doesn't support being used as a uniform block for shader '%s.%s'.",
			element->name, moduleName, pipelineName);
		return false;
	}

	if (element->type == dsMaterialType_UniformBuffer &&
		!(buffer->usage & dsGfxBufferUsage_UniformBuffer))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Buffer '%s' doesn't support being used as a uniform buffer for shader '%s.%s'.",
			element->name, moduleName, pipelineName);
		return false;
	}

	return true;
}

static bool verifySharedMaterialValues(const dsMaterialDesc* materialDesc,
	const dsSharedMaterialValues* sharedValues, dsMaterialBinding binding, const char* moduleName,
	const char* pipelineName)
{
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->binding != binding)
			continue;

		if (!sharedValues)
		{
			DS_ASSERT(binding == dsMaterialBinding_Global);
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Material uses global values, but no global values "
				"provided during shader bind for shader '%s.%s'.", moduleName, pipelineName);
			return false;
		}

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				dsTexture* texture = dsSharedMaterialValues_getTextureID(sharedValues,
					element->nameID);
				if (!texture)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Shared texture '%s' not found for shader '%s.%s'.", element->name,
						moduleName, pipelineName);
					return false;
				}

				if (element->type == dsMaterialType_Texture &&
					!(texture->usage & dsTextureUsage_Texture))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Texture '%s' doesn't support being used as "
						"a texture sampler for shader '%s.%s'.", element->name, moduleName,
						pipelineName);
					return false;
				}

				if (element->type == dsMaterialType_Image &&
					!(texture->usage & dsTextureUsage_Image))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Texture '%s' doesn't support being used as "
						"an image sampler for shader '%s.%s'.", element->name, moduleName,
						pipelineName);
					return false;
				}

				if (element->type == dsMaterialType_SubpassInput &&
					!(texture->usage & dsTextureUsage_SubpassInput))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Texture '%s' doesn't support being used as "
						"a subpass input for shader '%s.%s'.", element->name, moduleName,
						pipelineName);
					return false;
				}
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				dsShaderVariableGroup* variableGroup = dsSharedMaterialValues_getVariableGroupID(
					sharedValues, element->nameID);
				if (!variableGroup)
				{
					// Check if there's an explicitly set buffer.
					dsGfxBuffer* buffer = dsSharedMaterialValues_getBufferID(NULL, NULL,
						sharedValues, element->nameID);
					if (buffer)
					{
						if (!verifySharedMaterialBufferValue(element, buffer, moduleName,
								pipelineName))
						{
							return false;
						}
						break;
					}
					else
					{
						DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
							"Shared variable group '%s' not found for shader '%s.%s'.",
							element->name, moduleName, pipelineName);
						return false;
					}
				}

				if (dsShaderVariableGroup_getDescription(variableGroup) !=
					element->shaderVariableGroupDesc)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Shared variable group description for "
						"'%s' doesn't match description set on material element for shader "
						"'%s.%s'.", element->name, moduleName, pipelineName);
					return false;
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				dsGfxBuffer* buffer = dsSharedMaterialValues_getBufferID(NULL, NULL,
					sharedValues, element->nameID);
				if (!buffer)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Buffer '%s' not found for shader '%s.%s'.",
						element->name, moduleName, pipelineName);
					return false;
				}

				if (!verifySharedMaterialBufferValue(element, buffer, moduleName, pipelineName))
					return false;
				break;
			}
			default:
				DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid shared material type for shader '%s.%s'.",
					moduleName, pipelineName);
				return false;
		}
	}

	return true;
}

dsShader* dsShader_createName(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* shaderModule, const char* name, const dsMaterialDesc* materialDesc)
{
	if (!resourceManager || !resourceManager->createShaderFunc ||
		!resourceManager->destroyShaderFunc || !shaderModule || !name || !materialDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t index = 0, shaderCount = dsShaderModule_shaderCount(shaderModule);
	for (; index < shaderCount; ++index)
	{
		if (strcmp(name, dsShaderModule_shaderName(shaderModule, index)) == 0)
			break;
	}

	if (index == shaderCount)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Shader '%s' not found in shader module.", name);
		return NULL;
	}

	return dsShader_createIndex(resourceManager, allocator, shaderModule, index, materialDesc);
}

dsShader* dsShader_createIndex(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* shaderModule, uint32_t index, const dsMaterialDesc* materialDesc)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->createShaderFunc ||
		!resourceManager->destroyShaderFunc || !shaderModule || !materialDesc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (index >= dsShaderModule_shaderCount(shaderModule))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	mslPipeline pipeline;
	DS_VERIFY(mslModule_pipeline(&pipeline, shaderModule->module, index));
	if (!isMaterialDescCompatible(resourceManager, shaderModule->module, &pipeline,
			index, materialDesc,
			(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock) != 0,
			shaderModule->name))
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Material description isn't compatible with shader '%s'.",
			pipeline.name);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderer* renderer = resourceManager->renderer;
	if ((pipeline.shaders[mslStage_TessellationControl] != MSL_UNKNOWN ||
		pipeline.shaders[mslStage_TessellationEvaluation] != MSL_UNKNOWN) &&
		!renderer->hasTessellationShaders)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support tessellation shaders.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (pipeline.shaders[mslStage_Geometry] != MSL_UNKNOWN && !renderer->hasGeometryShaders)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support geometry shaders.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (pipeline.shaders[mslStage_Compute] != MSL_UNKNOWN)
	{
		if (resourceManager->maxComputeLocalWorkGroupInvocations == 0)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		unsigned long long invocations = 1;
		const char* dimensions[] = {"x", "y", "z"};
		for (unsigned int i = 0; i < 3; ++i)
		{
			invocations *= pipeline.computeLocalSize[i];
			if (pipeline.computeLocalSize[i] > resourceManager->maxComputeLocalWorkGroupSize[i])
			{
				errno = EPERM;
				DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Compute local working group size of %u along %s "
					"exceeds maximum of %u for shader '%s'.", pipeline.computeLocalSize[i],
					dimensions[i], resourceManager->maxComputeLocalWorkGroupSize[i], pipeline.name);
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (invocations > resourceManager->maxComputeLocalWorkGroupInvocations)
		{
			errno = EPERM;
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Total compute local working group invocations of "
				"%llu exceeds maximum of %u for shader '%s'.", invocations,
				resourceManager->maxComputeLocalWorkGroupInvocations, pipeline.name);
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	mslRenderState renderState;
	DS_VERIFY(mslModule_renderState(&renderState, shaderModule->module, index));
	if (renderState.clipDistanceCount > resourceManager->maxClipDistances)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Declared clip distances of %u exceeds maximum of %u for "
			"shader '%s'.", renderState.clipDistanceCount, resourceManager->maxClipDistances,
			pipeline.name);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (renderState.cullDistanceCount > resourceManager->maxCullDistances)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Declared cull distances of %u exceeds maximum of %u for "
			"shader '%s'.", renderState.cullDistanceCount, resourceManager->maxCullDistances,
			pipeline.name);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t combinedClipAndCull = renderState.clipDistanceCount + renderState.cullDistanceCount;
	if (combinedClipAndCull > resourceManager->maxCombinedClipAndCullDistances)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Sum of declared clip and cull distances of %u exceeds "
			"maximum of %u for shader '%s'.", combinedClipAndCull,
			resourceManager->maxCombinedClipAndCullDistances, pipeline.name);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShader* shader = resourceManager->createShaderFunc(resourceManager, allocator, shaderModule,
		index, materialDesc);
	if (shader)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderCount, 1);
	DS_PROFILE_FUNC_RETURN(shader);
}

bool dsShader_hasStage(const dsShader* shader, dsShaderStage stage)
{
	if (!shader || (unsigned int)stage >= (unsigned int)mslStage_Count)
		return false;

	return shader->pipeline->shaders[stage] != MSL_UNKNOWN;
}

bool dsShader_bind(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues,
	const dsDynamicRenderStates* renderStates)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !material || !shader->resourceManager ||
		!shader->resourceManager->bindShaderFunc || !shader->resourceManager->unbindShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (shader->pipeline->shaders[mslStage_Vertex] == MSL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to bind a shader without graphics stages.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsMaterial_getDescription(material) != shader->materialDesc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Material descriptions for shader and material don't match.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifySharedMaterialValues(shader->materialDesc, globalValues, dsMaterialBinding_Global,
			shader->module->name, shader->name))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Shader binding must be performed inside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->secondaryRenderPassCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render commands cannot be submitted directly when inside "
			"of a render subpass begun with the secondary flag set to true.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot bind a shader when another shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->bindShaderFunc(shader->resourceManager, commandBuffer, shader,
		material, globalValues, renderStates);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundShader = shader;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateInstanceValues(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsSharedMaterialValues* instanceValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !instanceValues || !shader->resourceManager ||
		!shader->resourceManager->updateShaderInstanceValuesFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifySharedMaterialValues(shader->materialDesc, instanceValues,
			dsMaterialBinding_Instance, shader->module->name, shader->name))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->secondaryRenderPassCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render commands cannot be submitted directly when inside "
			"of a render subpass begun with the secondary flag set to true.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only update instance values for the currently bound shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->updateShaderInstanceValuesFunc(shader->resourceManager,
		commandBuffer, shader, instanceValues);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateDynamicRenderStates(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsDynamicRenderStates* renderStates)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !renderStates || !shader->resourceManager ||
		!shader->resourceManager->updateShaderDynamicRenderStatesFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only update shared values for the currently bound shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->secondaryRenderPassCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render commands cannot be submitted directly when inside "
			"of a render subpass begun with the secondary flag set to true.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->updateShaderDynamicRenderStatesFunc(shader->resourceManager,
		commandBuffer, shader, renderStates);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_unbind(const dsShader* shader, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager ||
		!shader->resourceManager->unbindShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Shader unbinding must be performed inside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->secondaryRenderPassCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render commands cannot be submitted directly when inside "
			"of a render subpass begun with the secondary flag set to true.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can only unbind the currently bound shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->unbindShaderFunc(resourceManager, commandBuffer, shader);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundShader = NULL;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_bindCompute(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !material || !shader->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (shader->pipeline->shaders[mslStage_Compute] == MSL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to bind a compute shader without a compute stage.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!resourceManager->bindComputeShaderFunc || !resourceManager->unbindComputeShaderFunc ||
		resourceManager->renderer->maxComputeWorkGroupSize[0] == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsMaterial_getDescription(material) != shader->materialDesc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Material descriptions for shader and material don't match.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifySharedMaterialValues(shader->materialDesc, globalValues, dsMaterialBinding_Global,
			shader->module->name, shader->name))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Resource)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Compute shaders cannot be bounds to a resource command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Compute shader binding must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Compute shader binding must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot bind a compute shader when another compute shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->bindComputeShaderFunc(shader->resourceManager, commandBuffer,
		shader, material, globalValues);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundComputeShader = shader;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateComputeInstanceValues(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsSharedMaterialValues* instanceValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !instanceValues || !shader->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!resourceManager->updateComputeShaderInstanceValuesFunc ||
		resourceManager->renderer->maxComputeWorkGroupSize[0] == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifySharedMaterialValues(shader->materialDesc, instanceValues,
			dsMaterialBinding_Instance, shader->module->name, shader->name))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only update compute instance values for the currently bound compute shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->updateComputeShaderInstanceValuesFunc(shader->resourceManager,
		commandBuffer, shader, instanceValues);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_unbindCompute(const dsShader* shader, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!resourceManager->unbindComputeShaderFunc ||
		resourceManager->renderer->maxComputeWorkGroupSize[0] == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Compute shader unbinding must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can only unbind the currently bound compute shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->unbindComputeShaderFunc(resourceManager, commandBuffer, shader);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundComputeShader = NULL;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_destroy(dsShader* shader)
{
	if (!shader)
		return true;

	DS_PROFILE_FUNC_START();

	if (!shader->resourceManager || !shader->resourceManager->destroyShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyShaderFunc(resourceManager, shader);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_prepareCacheDirectory(const char* cacheDir)
{
	static const char* lastCacheDir;
	const char* lastCacheDirLocal;
	DS_ATOMIC_EXCHANGE_PTR(&lastCacheDir, &cacheDir, &lastCacheDirLocal);
	bool printError = lastCacheDirLocal != cacheDir;
	switch (dsFileStream_pathStatus(cacheDir))
	{
		case dsPathStatus_Error:
			if (printError)
			{
				DS_LOG_WARNING_F(DS_RENDER_LOG_TAG,
					"Couldn't access shader cache directory '%s': %s", cacheDir,
					dsErrorString(errno));
			}
			return false;
		case dsPathStatus_Missing:
			if (!dsFileStream_createDirectory(cacheDir))
			{
				if (printError)
				{
					DS_LOG_WARNING_F(DS_RENDER_LOG_TAG, "Couldn't create directory '%s': %s",
						cacheDir, dsErrorString(errno));
				}
				return false;
			}
			return true;
		case dsPathStatus_ExistsFile:
			if (printError)
			{
				DS_LOG_WARNING_F(DS_RENDER_LOG_TAG,
					"Shader cache directory '%s' isn't a directory.", cacheDir);
			}
			return false;
		case dsPathStatus_ExistsDirectory:
			return true;
		default:
			DS_ASSERT(false);
			return false;
	}
}

bool dsShader_cacheFileName(char* result, size_t resultSize, const dsShader* shader,
	const char* cacheDir, const char* extension)
{
	if (!dsPath_combine(result, resultSize, cacheDir, shader->module->name))
		return false;

	size_t pathLength = strlen(result);
	size_t pipelineLength = strlen(shader->name);
	size_t extensionLength = strlen(extension);
	if (pathLength + 1 + pipelineLength + extensionLength + 1 > resultSize)
		return false;

	result[pathLength++] = '.';
	memcpy(result + pathLength, shader->name, pipelineLength);
	pathLength += pipelineLength;
	memcpy(result + pathLength, extension, extensionLength + 1);
	return true;
}
