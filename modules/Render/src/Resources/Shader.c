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

#include <DeepSea/Render/Resources/Shader.h>

#include <DeepSea/Core/Streams/FileUtils.h>
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
#include <DeepSea/Render/Resources/VolatileMaterialValues.h>
#include <DeepSea/Render/Types.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

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
	bool supportsBuffers, const char* uniformName)
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
					"Uniform '%s.%s' not found in material description.", uniformName,
					structMember.name);
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
				"Multi-dimentional arrays aren't supported for uniform '%s.%s'.",
				uniformName, structMember.name);
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
				"Types for uniform '%s.%s' differ between shader and material.", uniformName,
				structMember.name);
			success = false;
			continue;
		}
	}

	return success;
}

static bool isShaderVariableGroupCompatible(const mslModule* module, uint32_t pipelineIndex,
	uint32_t structIndex, const dsShaderVariableGroupDesc* groupDesc, const char* uniformName)
{
	DS_ASSERT(groupDesc);
	mslStruct structInfo;
	DS_VERIFY(mslModule_struct(&structInfo, module, pipelineIndex, structIndex));

	if (structInfo.memberCount != groupDesc->elementCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Variable group '%s' doesn't match shader uniform block.",
			uniformName);
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
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Multi-dimentional arrays aren't supported for variable group member '%s.%s'.",
				uniformName, structMember.name);
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
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Types for element '%s.%s' differ between shader and shader variable group.",
				uniformName, structMember.name);
			success = false;
		}

		if (((element->type >= dsMaterialType_Mat2x3 && element->type <= dsMaterialType_Mat4x3) ||
			(element->type >= dsMaterialType_DMat2x3 && element->type <= dsMaterialType_DMat4x3)) &&
			structMember.rowMajor)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Element '%s.%s' is row major. Non-square matrix "
				"elements within a shader variable group must be column-major.", uniformName,
				structMember.name);
			success = false;
		}
	}

	return success;
}

static bool isMaterialDescCompatible(dsResourceManager* resourceManager, const mslModule* module,
	const mslPipeline* pipeline, uint32_t index, const dsMaterialDesc* materialDesc,
	bool supportsBuffers)
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
				materialDesc, supportsBuffers, uniform.name))
			{
				success = false;
			}
			continue;
		}

		uint32_t elementIndex = dsMaterialDesc_findElement(materialDesc, uniform.name);
		if (elementIndex == DS_MATERIAL_UNKNOWN)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Uniform '%s' not found in material description.",
				uniform.name);
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
							"Shader variable group '%s' may not be an array.", uniform.name);
						success = false;
					}

					if (!isShaderVariableGroupCompatible(module, index, uniform.structIndex,
						element->shaderVariableGroupDesc, uniform.name))
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
					element->type == dsMaterialType_MutableImageBuffer;
				if (!nativeSubpassInput && element->type == dsMaterialType_SubpassInput)
					typesMatch = true;
				break;
			case mslUniformType_SampledImage:
				typesMatch = element->type == dsMaterialType_Texture ||
					element->type == dsMaterialType_ImageBuffer;
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
				"Types for uniform '%s' differ between shader and material.", uniform.name);
			success = false;
		}
	}

	return success;
}

static bool verifyVolatileMaterialValues(const dsMaterialDesc* materialDesc,
	const dsVolatileMaterialValues* volatileValues)
{
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (!materialDesc->elements[i].isVolatile)
			continue;

		if (!volatileValues)
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Material uses volatile values, but no volatile values "
				"provided during shader bind.");
			return false;
		}

		switch (materialDesc->elements[i].type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				dsTexture* texture = dsVolatileMaterialValues_getTextureId(volatileValues,
					materialDesc->elements[i].nameId);
				if (!texture)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Volatile texture '%s' not found.",
						materialDesc->elements[i].name);
					return false;
				}

				if (materialDesc->elements[i].type == dsMaterialType_Texture &&
					!(texture->usage & dsTextureUsage_Texture))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Texture '%s' doesn't support being used as a texture sampler.",
						materialDesc->elements[i].name);
					return false;
				}

				if (materialDesc->elements[i].type == dsMaterialType_Image &&
					!(texture->usage & dsTextureUsage_Image))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Texture '%s' doesn't support being used as an image sampler.",
						materialDesc->elements[i].name);
					return false;
				}

				if (materialDesc->elements[i].type == dsMaterialType_SubpassInput &&
					!(texture->usage & dsTextureUsage_SubpassInput))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Texture '%s' doesn't support being used as a subpass input.",
						materialDesc->elements[i].name);
					return false;
				}
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				dsShaderVariableGroup* variableGroup = dsVolatileMaterialValues_getVariableGroupId(
					volatileValues, materialDesc->elements[i].nameId);
				if (!variableGroup)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Volatile variable group '%s' not found.",
						materialDesc->elements[i].name);
					return false;
				}

				if (dsShaderVariableGroup_getDescription(variableGroup) !=
					materialDesc->elements[i].shaderVariableGroupDesc)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Volatile variable group description for "
						"'%s' doesn't match description set on material element.",
						materialDesc->elements[i].name);
					return false;
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				dsGfxBuffer* buffer = dsVolatileMaterialValues_getBufferId(NULL, NULL,
					volatileValues, materialDesc->elements[i].nameId);
				if (!buffer)
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Buffer '%s' not found.",
						materialDesc->elements[i].name);
					return false;
				}

				if (materialDesc->elements[i].type == dsMaterialType_UniformBlock &&
					!(buffer->usage & dsGfxBufferUsage_UniformBlock))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Buffer '%s' doesn't support being used as a uniform block.",
						materialDesc->elements[i].name);
					return false;
				}

				if (materialDesc->elements[i].type == dsMaterialType_UniformBuffer &&
					!(buffer->usage & dsGfxBufferUsage_UniformBuffer))
				{
					DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
						"Buffer '%s' doesn't support being used as a uniform buffer.",
						materialDesc->elements[i].name);
					return false;
				}
				break;
			}
			default:
				DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid volatile material type.");
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
		errno = EINVAL;
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
		index, materialDesc, (resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock)
		!= 0))
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

	if (pipeline.shaders[mslStage_Compute] != MSL_UNKNOWN && renderer->maxComputeInvocations == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
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
	const dsMaterial* material, const dsVolatileMaterialValues* volatileValues,
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

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
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

	if (commandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot bind a shader when another shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->bindShaderFunc(shader->resourceManager, commandBuffer, shader,
		material, volatileValues, renderStates);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundShader = shader;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateVolatileValues(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager ||
		!shader->resourceManager->updateShaderVolatileValuesFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only update volatile values for the currently bound shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->updateShaderVolatileValuesFunc(shader->resourceManager,
		commandBuffer, shader, volatileValues);
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
	const dsMaterial* material, const dsVolatileMaterialValues* volatileValues)
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
		resourceManager->renderer->maxComputeInvocations == 0)
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

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
	{
		errno = EINVAL;
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
		shader, material, volatileValues);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	commandBuffer->boundComputeShader = shader;
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateComputeVolatileValues(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	if (!resourceManager->updateComputeShaderVolatileValuesFunc ||
		resourceManager->renderer->maxComputeInvocations == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support compute shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only update compute volatile values for the currently bound compute shader.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->updateComputeShaderVolatileValuesFunc(shader->resourceManager,
		commandBuffer, shader, volatileValues);
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
		resourceManager->renderer->maxComputeInvocations == 0)
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
	switch (dsGetFileStatus(cacheDir))
	{
		case dsFileStatus_Error:
			if (printError)
			{
				DS_LOG_WARNING_F(DS_RENDER_LOG_TAG,
					"Couldn't access shader cache directory '%s': %s", cacheDir,
					dsErrorString(errno));
			}
			return false;
		case dsFileStatus_DoesntExist:
			if (!dsCreateDirectory(cacheDir) && errno != EEXIST)
			{
				if (printError)
				{
					DS_LOG_WARNING_F(DS_RENDER_LOG_TAG, "Couldn't create directory '%s': %s",
						cacheDir, dsErrorString(errno));
				}
				return false;
			}
			return true;
		case dsFileStatus_ExistsFile:
			if (printError)
			{
				DS_LOG_WARNING_F(DS_RENDER_LOG_TAG,
					"Shader cache directory '%s' isn't a directory.", cacheDir);
			}
			return false;
		case dsFileStatus_ExistsDirectory:
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
