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

static const dsMaterialType materialTypeMap[] =
{
	// Scalars and vectors
	dsMaterialType_Float,  // mslType_Float
	dsMaterialType_Vec2,   // mslType_Vec2
	dsMaterialType_Vec3,   // mslType_Vec3
	dsMaterialType_Vec4,   // mslType_Vec4
	dsMaterialType_Double, // mslType_Double
	dsMaterialType_DVec2,  // mslType_DVec2
	dsMaterialType_DVec3,  // mslType_DVec3
	dsMaterialType_DVec4,  // mslType_DVec4
	dsMaterialType_Int,    // mslType_Int
	dsMaterialType_IVec2,  // mslType_IVec2
	dsMaterialType_IVec3,  // mslType_IVec3
	dsMaterialType_IVec4,  // mslType_IVec4
	dsMaterialType_UInt,   // mslType_UInt
	dsMaterialType_UVec2,  // mslType_UVec2
	dsMaterialType_UVec3,  // mslType_UVec3
	dsMaterialType_UVec4,  // mslType_UVec4
	dsMaterialType_Bool,   // mslType_Bool
	dsMaterialType_BVec2,  // mslType_BVec2
	dsMaterialType_BVec3,  // mslType_BVec3
	dsMaterialType_BVec4,  // mslType_BVec4

	// Matrices
	dsMaterialType_Mat2,    // mslType_Mat2
	dsMaterialType_Mat3,    // mslType_Mat3
	dsMaterialType_Mat4,    // mslType_Mat4
	dsMaterialType_Mat2x3,  // mslType_Mat2x3
	dsMaterialType_Mat2x4,  // mslType_Mat2x4
	dsMaterialType_Mat3x2,  // mslType_Mat3x2
	dsMaterialType_Mat3x4,  // mslType_Mat3x4
	dsMaterialType_Mat4x2,  // mslType_Mat4x2
	dsMaterialType_Mat4x3,  // mslType_Mat4x3
	dsMaterialType_DMat2,   // mslType_DMat2
	dsMaterialType_DMat3,   // mslType_DMat3
	dsMaterialType_DMat4,   // mslType_DMat4
	dsMaterialType_DMat2x3, // mslType_DMat2x3
	dsMaterialType_DMat2x4, // mslType_DMat2x4
	dsMaterialType_DMat3x2, // mslType_DMat3x2
	dsMaterialType_DMat3x4, // mslType_DMat3x4
	dsMaterialType_DMat4x2, // mslType_DMat4x2
	dsMaterialType_DMat4x3, // mslType_DMat4x3

	// Samplers
	dsMaterialType_Texture, // mslType_Sampler1D
	dsMaterialType_Texture, // mslType_Sampler2D
	dsMaterialType_Texture, // mslType_Sampler3D
	dsMaterialType_Texture, // mslType_SamplerCube
	dsMaterialType_Texture, // mslType_Sampler1DShadow
	dsMaterialType_Texture, // mslType_Sampler2DShadow
	dsMaterialType_Texture, // mslType_Sampler1DArray
	dsMaterialType_Texture, // mslType_Sampler2DArray
	dsMaterialType_Texture, // mslType_Sampler1DArrayShadow
	dsMaterialType_Texture, // mslType_Sampler2DArrayShadow
	dsMaterialType_Texture, // mslType_Sampler2DMS
	dsMaterialType_Texture, // mslType_Sampler2DMSArray
	dsMaterialType_Texture, // mslType_SamplerCubeShadow
	dsMaterialType_Texture, // mslType_SamplerBuffer
	dsMaterialType_Texture, // mslType_Sampler2DRect
	dsMaterialType_Texture, // mslType_Sampler2DRectShadow
	dsMaterialType_Texture, // mslType_ISampler1D
	dsMaterialType_Texture, // mslType_ISampler2D
	dsMaterialType_Texture, // mslType_ISampler3D
	dsMaterialType_Texture, // mslType_ISamplerCube
	dsMaterialType_Texture, // mslType_ISampler1DArray
	dsMaterialType_Texture, // mslType_ISampler2DArray
	dsMaterialType_Texture, // mslType_ISampler2DMS
	dsMaterialType_Texture, // mslType_ISampler2DMSArray
	dsMaterialType_Texture, // mslType_ISampler2DRect
	dsMaterialType_Texture, // mslType_USampler1D
	dsMaterialType_Texture, // mslType_USampler2D
	dsMaterialType_Texture, // mslType_USampler3D
	dsMaterialType_Texture, // mslType_USamplerCube
	dsMaterialType_Texture, // mslType_USampler1DArray
	dsMaterialType_Texture, // mslType_USampler2DArray
	dsMaterialType_Texture, // mslType_USampler2DMS
	dsMaterialType_Texture, // mslType_USampler2DMSArray
	dsMaterialType_Texture, // mslType_USampler2DRect

	// Images
	dsMaterialType_Image, // mslType_Image1D
	dsMaterialType_Image, // mslType_Image2D
	dsMaterialType_Image, // mslType_Image3D
	dsMaterialType_Image, // mslType_ImageCube
	dsMaterialType_Image, // mslType_Image1DArray
	dsMaterialType_Image, // mslType_Image2DArray
	dsMaterialType_Image, // mslType_Image2DMS
	dsMaterialType_Image, // mslType_Image2DMSArray
	dsMaterialType_Image, // mslType_ImageBuffer
	dsMaterialType_Image, // mslType_Image2DRect
	dsMaterialType_Image, // mslType_IImage1D
	dsMaterialType_Image, // mslType_IImage2D
	dsMaterialType_Image, // mslType_IImage3D
	dsMaterialType_Image, // mslType_IImageCube
	dsMaterialType_Image, // mslType_IImage1DArray
	dsMaterialType_Image, // mslType_IImage2DArray
	dsMaterialType_Image, // mslType_IImage2DMS
	dsMaterialType_Image, // mslType_IImage2DMSArray
	dsMaterialType_Image, // mslType_IImage2DRect
	dsMaterialType_Image, // mslType_UImage1D
	dsMaterialType_Image, // mslType_UImage2D
	dsMaterialType_Image, // mslType_UImage3D
	dsMaterialType_Image, // mslType_UImageCube
	dsMaterialType_Image, // mslType_UImage1DArray
	dsMaterialType_Image, // mslType_UImage2DArray
	dsMaterialType_Image, // mslType_UImage2DMS
	dsMaterialType_Image, // mslType_UImage2DMSArray
	dsMaterialType_Image, // mslType_UImage2DRect

	// Subpass inputs.
	dsMaterialType_SubpassInput, // mslType_SubpassInput
	dsMaterialType_SubpassInput, // mslType_SubpassInputMS
	dsMaterialType_SubpassInput, // mslType_ISubpassInput
	dsMaterialType_SubpassInput, // mslType_ISubpassInputMS
	dsMaterialType_SubpassInput, // mslType_USubpassInput
	dsMaterialType_SubpassInput, // mslType_USubpassInputMS

	// Other.
	dsMaterialType_VariableGroup, // mslType_Struct (also UniformBlock or UniformBuffer)
};

DS_STATIC_ASSERT(DS_ARRAY_SIZE(materialTypeMap) == mslType_Count, material_type_map_mismatch);

static dsMaterialType convertMslType(mslType type)
{
	if ((unsigned int)type >= mslType_Count)
		return dsMaterialType_Count;

	return materialTypeMap[type];
}

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

		if (convertMslType(structMember.type) != type || structMemberArrayCount != arrayCount)
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

		if (convertMslType(structMember.type) != element->type ||
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
				typesMatch = element->type == dsMaterialType_Image;
				if (!nativeSubpassInput && element->type == dsMaterialType_SubpassInput)
					typesMatch = true;
				break;
			case mslUniformType_SampledImage:
				typesMatch = element->type == dsMaterialType_Texture;
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
	dsShaderModule* shaderModule, const char* name, const dsMaterialDesc* materialDesc,
	dsPrimitiveType primitiveType, uint32_t samples)
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

	return dsShader_createIndex(resourceManager, allocator, shaderModule, index, materialDesc,
		primitiveType, samples);
}

dsShader* dsShader_createIndex(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* shaderModule, uint32_t index, const dsMaterialDesc* materialDesc,
	dsPrimitiveType primitiveType, uint32_t samples)
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

	if (pipeline.shaders[mslStage_Compute] != MSL_UNKNOWN && !renderer->hasComputeShaders)
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

	samples = dsMax(1U, samples);
	dsShader* shader = resourceManager->createShaderFunc(resourceManager, allocator, shaderModule,
		index, materialDesc, primitiveType, samples);
	if (shader)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderCount, 1);
	DS_PROFILE_FUNC_RETURN(shader);
}

bool dsShader_bind(dsCommandBuffer* commandBuffer, const dsShader* shader,
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

	if (dsMaterial_getDescription(material) != shader->materialDesc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Material descriptions for shader and material don't match.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
	{
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->bindShaderFunc(shader->resourceManager, commandBuffer, shader,
		material, volatileValues, renderStates);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_updateVolatileValues(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager ||
		!shader->resourceManager->bindShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!verifyVolatileMaterialValues(shader->materialDesc, volatileValues))
	{
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->updateShaderVolatileValuesFunc(shader->resourceManager,
		commandBuffer, shader, volatileValues);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_unbind(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !shader || !shader->resourceManager ||
		!shader->resourceManager->unbindShaderFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shader->resourceManager;
	bool success = resourceManager->unbindShaderFunc(resourceManager, commandBuffer, shader);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsShader_destroy(dsShader* shader)
{
	DS_PROFILE_FUNC_START();

	if (!shader || !shader->resourceManager || !shader->resourceManager->destroyShaderFunc)
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
