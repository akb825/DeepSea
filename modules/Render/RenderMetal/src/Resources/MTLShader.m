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

#include "Resources/MTLShader.h"

#include "Resources/MTLPipeline.h"
#include "Resources/MTLShaderModule.h"
#include "MTLCommandBuffer.h"
#include "MTLGfxBuffer.h"
#include "MTLGfxBufferData.h"
#include "MTLRenderPass.h"
#include "MTLShared.h"
#include "MTLTexture.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

static size_t fullAllocSize(const mslPipeline* pipeline, uint32_t uniformCount,
	uint32_t pushConstantCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsMTLShader));

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] != DS_MATERIAL_UNKNOWN)
			fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*pipeline->uniformCount);
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(CFTypeRef)*pipeline->samplerStateCount);
	fullSize += DS_ALIGNED_SIZE(sizeof(dsMTLUniformInfo)*uniformCount);
	fullSize += DS_ALIGNED_SIZE(sizeof(dsMTLPushConstantInfo)*pushConstantCount);
	return fullSize;
}

static MTLSamplerMinMagFilter getFilter(mslFilter filter)
{
	switch (filter)
	{
		case mslFilter_Linear:
			return MTLSamplerMinMagFilterLinear;
		case mslFilter_Nearest:
		default:
			return MTLSamplerMinMagFilterNearest;
	}
}

static MTLSamplerMipFilter getMipFilter(mslMipFilter filter)
{
	switch (filter)
	{
		case mslMipFilter_Nearest:
			return MTLSamplerMipFilterNearest;
		case mslMipFilter_Linear:
		case mslMipFilter_Anisotropic:
			return MTLSamplerMipFilterLinear;
		case mslMipFilter_None:
		default:
			return MTLSamplerMipFilterNotMipmapped;
	}
}

static MTLSamplerAddressMode getAddressMode(mslAddressMode mode)
{
	switch (mode)
	{
		case mslAddressMode_MirroredRepeat:
			return MTLSamplerAddressModeMirrorRepeat;
		case mslAddressMode_ClampToEdge:
			return MTLSamplerAddressModeClampToEdge;
		case mslAddressMode_ClampToBorder:
			return MTLSamplerAddressModeClampToBorderColor;
		case mslAddressMode_MirrorOnce:
			return MTLSamplerAddressModeMirrorClampToEdge;
		case mslAddressMode_Repeat:
		default:
			return MTLSamplerAddressModeRepeat;
	}
}

static MTLSamplerBorderColor getBorderColor(mslBorderColor color)
{
	switch (color)
	{
		case mslBorderColor_OpaqueBlack:
		case mslBorderColor_OpaqueIntZero:
			return MTLSamplerBorderColorOpaqueBlack;
		case mslBorderColor_OpaqueWhite:
		case mslBorderColor_OpaqueIntOne:
			return MTLSamplerBorderColorOpaqueWhite;
		case mslBorderColor_TransparentBlack:
		case mslBorderColor_TransparentIntZero:
		default:
			return MTLSamplerBorderColorTransparentBlack;
	}
}

static bool createDepthStencilState(dsMTLShader* shader)
{
	dsRenderer* renderer = ((dsShader*)shader)->resourceManager->renderer;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	MTLDepthStencilDescriptor* descriptor = [MTLDepthStencilDescriptor new];
	if (!descriptor)
	{
		errno = ENOMEM;
		return false;
	}

	const mslDepthStencilState* states = &shader->renderState.depthStencilState;
	descriptor.depthCompareFunction = states->depthTestEnable == mslBool_True ?
		dsGetMTLCompareFunction(states->depthCompareOp) : MTLCompareFunctionAlways;
	descriptor.depthWriteEnabled = states->depthWriteEnable != mslBool_False;
	if (states->stencilTestEnable == mslBool_True)
	{
		descriptor.frontFaceStencil = dsCreateMTLStencilDescriptor(&states->frontStencil,
			states->frontStencil.compareMask, states->frontStencil.writeMask);
		if (!descriptor.frontFaceStencil)
		{
			errno = ENOMEM;
			return false;
		}

		descriptor.backFaceStencil = dsCreateMTLStencilDescriptor(&states->backStencil,
			states->backStencil.compareMask, states->backStencil.writeMask);
		if (!descriptor.backFaceStencil)
		{
			errno = ENOMEM;
			return false;
		}
	}

	id<MTLDepthStencilState> depthStencilState =
		[device newDepthStencilStateWithDescriptor: descriptor];
	if (!depthStencilState)
	{
		errno = ENOMEM;
		return false;
	}

	shader->depthStencilState = CFBridgingRetain(depthStencilState);
	return true;
}

static id<MTLSamplerState> createSampler(dsRenderer* renderer, const mslSamplerState* samplerState)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
	MTLSamplerDescriptor* descriptor = [MTLSamplerDescriptor new];
	if (!descriptor)
	{
		errno = ENOMEM;
		return NULL;
	}

	descriptor.rAddressMode = getAddressMode(samplerState->addressModeU);
	descriptor.sAddressMode = getAddressMode(samplerState->addressModeV);
	descriptor.tAddressMode = getAddressMode(samplerState->addressModeW);
	descriptor.borderColor = getBorderColor(samplerState->borderColor);

	descriptor.minFilter = getFilter(samplerState->minFilter);
	descriptor.magFilter = getFilter(samplerState->magFilter);
	descriptor.mipFilter = getMipFilter(samplerState->mipFilter);
	if (samplerState->minLod != MSL_UNKNOWN_FLOAT)
		descriptor.lodMinClamp = samplerState->minLod;
	if (samplerState->maxLod != MSL_UNKNOWN_FLOAT)
		descriptor.lodMaxClamp = samplerState->maxLod;
	if (samplerState->mipFilter == mslMipFilter_Anisotropic)
	{
		if (samplerState->maxAnisotropy == MSL_UNKNOWN)
			descriptor.maxAnisotropy = (NSUInteger)roundf(samplerState->maxAnisotropy);
		else
			descriptor.maxAnisotropy = (NSUInteger)roundf(renderer->defaultAnisotropy);
	}

#if DS_MAC || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	descriptor.compareFunction = dsGetMTLCompareFunction(samplerState->compareOp);
#endif

	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor: descriptor];
	if (!sampler)
		errno = ENOMEM;
	return sampler;
}

static bool createSamplers(dsMTLShader* shader, mslModule* module, uint32_t shaderIndex)
{
	bool hasDefaultAnisotropy = false;
	dsRenderer* renderer = ((dsShader*)shader)->resourceManager->renderer;
	for (uint32_t i = 0; i < shader->pipeline.samplerStateCount; ++i)
	{
		mslSamplerState samplerState;
		DS_VERIFY(mslModule_samplerState(&samplerState, module, shaderIndex, i));

		if (samplerState.mipFilter == mslMipFilter_Anisotropic &&
			samplerState.maxAnisotropy == MSL_UNKNOWN_FLOAT)
		{
			hasDefaultAnisotropy = true;
		}

		id<MTLSamplerState> sampler = createSampler(renderer, &samplerState);
		if (!sampler)
			return false;

		shader->samplers[i] = CFBridgingRetain(sampler);
	}

	if (hasDefaultAnisotropy)
		shader->defaultAnisotropy = renderer->defaultAnisotropy;
	else
		shader->defaultAnisotropy = MSL_UNKNOWN_FLOAT;
	return true;
}

static void setupUniformIndices(dsMTLShader* shader, mslModule* module, uint32_t shaderIndex)
{
	shader->firstVertexBuffer = 0;
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (!shader->stages[i].uniformIndices)
			continue;

		mslStage stage = (mslStage)i;
		uint32_t maxIndex = 0;
		for (uint32_t j = 0; j < shader->pipeline.uniformCount; ++j)
		{
			uint32_t index = mslModule_shaderUniformId(module, shaderIndex, j, stage);
			shader->stages[i].uniformIndices[j] = index;
			if (index != MSL_UNKNOWN && index > maxIndex)
				maxIndex = index;
		}

		if (stage == mslStage_Vertex)
			shader->firstVertexBuffer = maxIndex;
	}
}

static bool setupShaders(dsShader* shader, mslModule* module, uint32_t shaderIndex)
{
	dsMTLRenderer* renderer = (dsMTLRenderer*)shader->resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	mslPipeline* pipeline = &mtlShader->pipeline;
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] == MSL_UNKNOWN)
			continue;

		id<MTLFunction> function = dsMTLShaderModule_getShader(shader->module, pipeline->shaders[i],
			pipeline->name);
		if (!function)
			return false;

		mtlShader->stages[i].function = CFBridgingRetain(function);
	}
	setupUniformIndices(mtlShader, module, shaderIndex);

	if (pipeline->shaders[mslStage_Compute] != MSL_UNKNOWN)
	{
		id<MTLFunction> function = dsMTLShaderModule_getShader(shader->module,
			pipeline->shaders[mslStage_Compute], pipeline->name);
		if (!function)
			return false;

		NSError* error = NULL;
		id<MTLComputePipelineState> computePipeline =
			[device newComputePipelineStateWithFunction: function error: &error];
		if (!computePipeline)
		{
			if (error)
			{
				DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
					"Error creating compute pipeline for shader %s.%s: %s",
					shader->module->name, shader->name, [[error localizedDescription] UTF8String]);
			}
			else
			{
				DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
					"Error creating compute pipeline for shader %s.%s.",
					shader->module->name, shader->name);
			}
			return false;
		}

		mtlShader->computePipeline = CFBridgingRetain(computePipeline);
	}
	return true;
}

static void* setPushConstantData(const dsShader* shader, const dsMaterial* material,
	dsCommandBuffer* commandBuffer)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	uint8_t* data = (uint8_t*)dsMTLCommandBuffer_getPushConstantData(commandBuffer,
		mtlShader->pushConstantSize);
	if (!data)
		return NULL;

	for (uint32_t i = 0; i < mtlShader->pushConstantCount; ++i)
	{
		const dsMTLPushConstantInfo* info = mtlShader->pushConstantInfos + i;
		const dsMaterialElement* element = shader->materialDesc->elements + info->element;
		if (info->count == 1 ||
			info->stride == dsMaterialType_cpuSize(element->type))
		{
			// Can copy all elements at once.
			DS_VERIFY(dsMaterial_getElementData(data + info->offset, material, info->element,
				element->type, 0, info->count));
		}
		else
		{
			// Must copy each element individually if the stride is different from the storage.
			for (uint32_t j = 0; j < info->count; ++j)
			{
				DS_VERIFY(dsMaterial_getElementData(data + info->offset + info->stride*j,
					material, info->element, element->type, j, 1));
			}
		}
	}

	return data;
}

static bool setPushConstnants(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	bool vertexPushConstants = mtlShader->stages[mslStage_Vertex].hasPushConstants;
	bool fragmentPushConstants = mtlShader->stages[mslStage_Fragment].hasPushConstants;
	if (!vertexPushConstants && !fragmentPushConstants)
		return true;

	void* data = setPushConstantData(shader, material, commandBuffer);
	if (!data)
		return false;

	return dsMTLCommandBuffer_bindPushConstants(commandBuffer, data, mtlShader->pushConstantSize,
		vertexPushConstants, fragmentPushConstants);
}

static bool setComputePushConstnants(const dsShader* shader, dsCommandBuffer* commandBuffer,
	const dsMaterial* material)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	if (!mtlShader->stages[mslStage_Compute].hasPushConstants)
		return true;

	void* data = setPushConstantData(shader, material, commandBuffer);
	if (!data)
		return false;

	return dsMTLCommandBuffer_bindComputePushConstants(commandBuffer, data,
		mtlShader->pushConstantSize);
}

static void getTextureAndSampler(id<MTLTexture>* outTexture, id<MTLSamplerState>* outSampler,
	const dsShader* shader, const dsMaterial* material, const dsSharedMaterialValues* sharedValues,
	const dsMTLUniformInfo* uniform)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	const dsMaterialElement* element = shader->materialDesc->elements + uniform->element;
	dsTexture* texture;
	if (element->binding == dsMaterialBinding_Material)
	{
		DS_ASSERT(material);
		texture = dsMaterial_getTexture(material, uniform->element);
	}
	else
	{
		DS_ASSERT(sharedValues);
		texture = dsSharedMaterialValues_getTextureId(sharedValues, element->nameId);
	}

	if (texture)
	{
		dsMTLTexture_process(texture->resourceManager, texture);
		*outTexture = (__bridge id<MTLTexture>)((dsMTLTexture*)texture)->mtlTexture;
		if (element->type == dsMaterialType_Texture)
		{
			if (uniform->sampler == DS_MATERIAL_UNKNOWN)
			{
				dsMTLResourceManager* mtlResourceManager =
					(dsMTLResourceManager*)shader->resourceManager;
				*outSampler = (__bridge id<MTLSamplerState>)mtlResourceManager->defaultSampler;
			}
			else
			{
				*outSampler =
					(__bridge id<MTLSamplerState>)mtlShader->samplers[uniform->sampler];
			}
		}
		else
			*outSampler = nil;
	}
	else
	{
		*outTexture = nil;
		*outSampler = nil;
	}
}

static id<MTLTexture> getTextureBuffer(const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues, const dsMTLUniformInfo* uniform,
	dsCommandBuffer* commandBuffer)
{
	const dsMaterialElement* element = shader->materialDesc->elements + uniform->element;
	dsGfxBuffer* buffer;
	dsGfxFormat format;
	size_t offset, count;
	if (element->binding == dsMaterialBinding_Material)
	{
		DS_ASSERT(material);
		buffer = dsMaterial_getTextureBuffer(&format, &offset, &count, material, uniform->element);
	}
	else
	{
		DS_ASSERT(sharedValues);
		buffer = dsSharedMaterialValues_getTextureBufferId(&format, &offset, &count, sharedValues,
			element->nameId);
	}

	if (!buffer)
		return nil;

	dsMTLGfxBufferData* data = dsMTLGfxBuffer_getData(buffer, commandBuffer);
	DS_ASSERT(data);
	id<MTLTexture> texture = dsMTLGfxBufferData_getBufferTexture(data, format, offset, count);
	dsLifetime_release(data->lifetime);
	return texture;
}

static id<MTLBuffer> getShaderVariableGroupBuffer(const dsShader* shader,
	const dsMaterial* material, const dsSharedMaterialValues* sharedValues,
	const dsMTLUniformInfo* uniform, dsCommandBuffer* commandBuffer)
{
	const dsMaterialElement* element = shader->materialDesc->elements + uniform->element;
	dsShaderVariableGroup* group;
	if (element->binding == dsMaterialBinding_Material)
	{
		DS_ASSERT(material);
		group = dsMaterial_getVariableGroup(material, uniform->element);
	}
	else
	{
		DS_ASSERT(sharedValues);
		group = dsSharedMaterialValues_getVariableGroupId(sharedValues, element->nameId);
	}

	if (!group)
		return nil;

	dsGfxBuffer* buffer = dsShaderVariableGroup_getGfxBuffer(group);
	DS_ASSERT(buffer);
	return dsMTLGfxBuffer_getBuffer(buffer, commandBuffer);
}

static id<MTLBuffer> getBuffer(size_t* outOffset, const dsShader* shader,
	const dsMaterial* material, const dsSharedMaterialValues* sharedValues,
	const dsMTLUniformInfo* uniform, dsCommandBuffer* commandBuffer)
{
	const dsMaterialElement* element = shader->materialDesc->elements + uniform->element;
	dsGfxBuffer* buffer;
	if (element->binding == dsMaterialBinding_Material)
	{
		DS_ASSERT(material);
		buffer = dsMaterial_getBuffer(outOffset, NULL, material, uniform->element);
	}
	else
	{
		DS_ASSERT(sharedValues);
		buffer = dsSharedMaterialValues_getBufferId(outOffset, NULL, sharedValues, element->nameId);
	}

	if (!buffer)
	{
		*outOffset = 0;
		return nil;
	}

	return dsMTLGfxBuffer_getBuffer(buffer, commandBuffer);
}

static bool bindUniforms(const dsShader* shader, const dsMaterial* material,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* sharedValues,
	dsMaterialBinding binding)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	for (uint32_t i = 0; i < mtlShader->uniformCount; ++i)
	{
		uint32_t vertexIndex = mtlShader->stages[mslStage_Vertex].uniformIndices[i];
		uint32_t fragmentIndex = mtlShader->stages[mslStage_Fragment].uniformIndices[i];
		if (vertexIndex == DS_MATERIAL_UNKNOWN && fragmentIndex == DS_MATERIAL_UNKNOWN)
			continue;

		const dsMTLUniformInfo* info = mtlShader->uniformInfos + i;
		if (info->element == DS_MATERIAL_UNKNOWN)
			continue;

		const dsMaterialElement* element = materialDesc->elements + info->element;
		bool invalidBinding = element->binding != dsMaterialBinding_Material &&
			element->binding != binding;
		bool isMaterialBinding = element->binding == dsMaterialBinding_Material;
		if (invalidBinding ||  (isMaterialBinding && !material) ||
			(!isMaterialBinding && !sharedValues))
		{
			continue;
		}

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				id<MTLTexture> texture;
				id<MTLSamplerState> sampler;
				getTextureAndSampler(&texture, &sampler, shader, material, sharedValues, info);
				if (!dsMTLCommandBuffer_bindTextureUniform(commandBuffer, texture, sampler,
						vertexIndex, fragmentIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				id<MTLTexture> texture = getTextureBuffer(shader, material, sharedValues, info,
					commandBuffer);
				if (!dsMTLCommandBuffer_bindTextureUniform(commandBuffer, texture, nil, vertexIndex,
						fragmentIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				id<MTLBuffer> buffer = getShaderVariableGroupBuffer(shader, material, sharedValues,
					info, commandBuffer);
				if (!dsMTLCommandBuffer_bindBufferUniform(commandBuffer, buffer, 0, vertexIndex,
						fragmentIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				size_t offset;
				id<MTLBuffer> buffer = getBuffer(&offset, shader, material, sharedValues, info,
					commandBuffer);
				if (!dsMTLCommandBuffer_bindBufferUniform(commandBuffer, buffer, offset,
						vertexIndex, fragmentIndex))
				{
					return false;
				}
				break;
			}
			default:
				DS_ASSERT(false);
				break;
		}
	}

	return true;
}

static bool bindComputeUniforms(const dsShader* shader, const dsMaterial* material,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* sharedValues,
	dsMaterialBinding binding)
{
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	for (uint32_t i = 0; i < mtlShader->uniformCount; ++i)
	{
		uint32_t computeIndex = mtlShader->stages[mslStage_Compute].uniformIndices[i];
		if (computeIndex == DS_MATERIAL_UNKNOWN)
			continue;

		const dsMTLUniformInfo* info = mtlShader->uniformInfos + i;
		const dsMaterialElement* element = materialDesc->elements + info->element;
		bool invalidBinding = element->binding != dsMaterialBinding_Material &&
			element->binding != binding;
		bool isMaterialBinding = element->binding == dsMaterialBinding_Material;
		if (invalidBinding ||  (isMaterialBinding && !material) ||
			(!isMaterialBinding && !sharedValues))
		{
			continue;
		}

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				id<MTLTexture> texture;
				id<MTLSamplerState> sampler;
				getTextureAndSampler(&texture, &sampler, shader, material, sharedValues, info);
				if (!dsMTLCommandBuffer_bindComputeTextureUniform(commandBuffer, texture, sampler,
						computeIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				id<MTLTexture> texture = getTextureBuffer(shader, material, sharedValues, info,
					commandBuffer);
				if (!dsMTLCommandBuffer_bindComputeTextureUniform(commandBuffer, texture, nil,
						computeIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				id<MTLBuffer> buffer = getShaderVariableGroupBuffer(shader, material, sharedValues,
					info, commandBuffer);
				if (!dsMTLCommandBuffer_bindComputeBufferUniform(commandBuffer, buffer, 0,
						computeIndex))
				{
					return false;
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				size_t offset;
				id<MTLBuffer> buffer = getBuffer(&offset, shader, material, sharedValues, info,
					commandBuffer);
				if (!dsMTLCommandBuffer_bindComputeBufferUniform(commandBuffer, buffer, offset,
						computeIndex))
				{
					return false;
				}
				break;
			}
			default:
				DS_ASSERT(false);
				break;
		}
	}

	return true;
}

dsShader* dsMTLShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(module);
	DS_ASSERT(materialDesc);

	mslPipeline pipeline;
	DS_VERIFY(mslModule_pipeline(&pipeline, module->module, shaderIndex));
	mslStruct pushConstantStruct = {NULL, 0, 0};
	if (pipeline.pushConstantStruct != MSL_UNKNOWN)
	{
		DS_VERIFY(mslModule_struct(&pushConstantStruct, module->module, shaderIndex,
			pipeline.pushConstantStruct));
	}

	size_t fullSize = fullAllocSize(&pipeline, pipeline.uniformCount,
		pushConstantStruct.memberCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsMTLShader* shader = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsMTLShader);
	DS_ASSERT(shader);

	dsShader* baseShader = (dsShader*)shader;
	baseShader->resourceManager = resourceManager;
	baseShader->allocator = dsAllocator_keepPointer(allocator);
	baseShader->module = module;
	baseShader->name = pipeline.name;
	baseShader->pipelineIndex = shaderIndex;
	baseShader->pipeline = &shader->pipeline;
	baseShader->materialDesc = materialDesc;

	shader->scratchAllocator = resourceManager->allocator;
	shader->lifetime = NULL;
	shader->pipeline = pipeline;
	shader->depthStencilState = NULL;

	for (int i = 0; i < mslStage_Count; ++i)
	{
		shader->stages[i].function = NULL;
		if (pipeline.shaders[i] == MSL_UNKNOWN)
		{
			shader->stages[i].uniformIndices = NULL;
			shader->stages[i].hasPushConstants = false;
		}
		else
		{
			shader->stages[i].uniformIndices = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				uint32_t, pipeline.uniformCount);
			DS_ASSERT(shader->stages[i].uniformIndices);
			memset(shader->stages[i].uniformIndices, 0xFF, sizeof(uint32_t)*pipeline.uniformCount);
			shader->stages[i].hasPushConstants = mslModule_shaderUsesPushConstants(module->module,
				pipeline.shaders[i]);
		}
	}

	if (pipeline.samplerStateCount > 0)
	{
		shader->samplers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, CFTypeRef,
			pipeline.samplerStateCount);
		DS_ASSERT(shader->samplers);
		memset(shader->samplers, 0, sizeof(CFTypeRef)*pipeline.samplerStateCount);
	}
	DS_VERIFY(dsSpinlock_initialize(&shader->samplerLock));

	if (pipeline.uniformCount > 0)
	{
		shader->uniformInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsMTLUniformInfo, materialDesc->elementCount);
		DS_ASSERT(shader->uniformInfos);
		for (uint32_t i = 0; i < pipeline.uniformCount; ++i)
		{
			mslUniform uniform;
			DS_VERIFY(mslModule_uniform(&uniform, module->module, shaderIndex, i));

			dsMTLUniformInfo* info = shader->uniformInfos + i;
			info->element = DS_MATERIAL_UNKNOWN;
			info->sampler = uniform.samplerIndex;
			if (uniform.uniformType == mslUniformType_PushConstant)
				continue;

			for (uint32_t j = 0; j < materialDesc->elementCount; ++j)
			{
				const dsMaterialElement* element = materialDesc->elements + j;
				if (strcmp(uniform.name, element->name) == 0)
				{
					info->element = j;
					break;
				}
			}
			// Should have been validated before creation.
			DS_ASSERT(info->element != DS_MATERIAL_UNKNOWN);
		}
	}
	else
		shader->uniformInfos = NULL;
	shader->uniformCount = pipeline.uniformCount;

	if (pushConstantStruct.memberCount > 0)
	{
		shader->pushConstantInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsMTLPushConstantInfo, pushConstantStruct.memberCount);
		DS_ASSERT(shader->pushConstantInfos);
		for (uint32_t i = 0; i < pushConstantStruct.memberCount; ++i)
		{
			mslStructMember member;
			DS_VERIFY(mslModule_structMember(&member, module->module, shaderIndex,
				pipeline.pushConstantStruct, i));

			dsMTLPushConstantInfo* info = shader->pushConstantInfos + i;
			info->element = DS_MATERIAL_UNKNOWN;
			info->offset = member.offset;
			if (member.arrayElementCount > 0)
			{
				DS_ASSERT(member.arrayElementCount == 1);
				mslArrayInfo arrayInfo;
				DS_VERIFY(mslModule_structMemberArrayInfo(&arrayInfo, module->module, shaderIndex,
					pipeline.pushConstantStruct, i, 0));
				info->count = arrayInfo.length;
			}
			else
			{
				info->count = 1;
				info->stride = 0;
			}

			for (uint32_t j = 0; j < materialDesc->elementCount; ++j)
			{
				const dsMaterialElement* element = materialDesc->elements + i;
				if (element->type >= dsMaterialType_Texture)
					continue;

				if (strcmp(member.name, element->name) == 0)
				{
					info->element = j;
					DS_ASSERT(element->type == dsMaterialDesc_convertMaterialType(member.type));
					DS_ASSERT(info->count == dsMax(element->count, 1));
					break;
				}
			}
			// Should have been validated before creation.
			DS_ASSERT(info->element != DS_MATERIAL_UNKNOWN);
		}
	}
	else
		shader->pushConstantInfos = NULL;
	shader->pushConstantCount = pushConstantStruct.memberCount;
	shader->pushConstantSize = pushConstantStruct.size;

	shader->usedRenderPasses = NULL;
	shader->usedRenderPassCount = 0;
	shader->maxUsedRenderPasses = 0;
	shader->pipelines = NULL;
	shader->pipelineCount = 0;
	shader->maxPipelines = 0;
	DS_VERIFY(dsSpinlock_initialize(&shader->pipelineLock));

	shader->computePipeline = NULL;

	shader->lifetime = dsLifetime_create(allocator, shader);
	if (!shader->lifetime)
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	if (!setupShaders(baseShader, module->module, shaderIndex))
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	DS_VERIFY(mslModule_renderState(&shader->renderState, module->module, shaderIndex));
	if (!createDepthStencilState(shader) || !createSamplers(shader, module->module, shaderIndex))
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	return baseShader;
}

bool dsMTLShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(resourceManager);
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	if (!dsMTLCommandBuffer_setRenderStates(commandBuffer, &mtlShader->renderState,
			(__bridge id<MTLDepthStencilState>)mtlShader->depthStencilState, renderStates, false))
	{
		return false;
	}

	if (!setPushConstnants(shader, commandBuffer, material))
		return false;

	return bindUniforms(shader, material, commandBuffer, globalValues, dsMaterialBinding_Global);
}

bool dsMTLShader_updateInstanceValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues)
{
	DS_UNUSED(resourceManager);
	return bindUniforms(shader, NULL, commandBuffer, instanceValues, dsMaterialBinding_Instance);
}

bool dsMTLShader_updateDynamicRenderStates(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(resourceManager);
	const dsMTLShader* mtlShader = (const dsMTLShader*)shader;
	return dsMTLCommandBuffer_setRenderStates(commandBuffer, &mtlShader->renderState,
		(__bridge id<MTLDepthStencilState>)mtlShader->depthStencilState, renderStates, true);
}

bool dsMTLShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsMTLShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues)
{
	DS_UNUSED(resourceManager);
	if (!dsMTLCommandBuffer_beginComputeShader(commandBuffer))
		return false;

	if (!setComputePushConstnants(shader, commandBuffer, material))
		return false;

	return bindComputeUniforms(shader, material, commandBuffer, globalValues,
		dsMaterialBinding_Global);
}

bool dsMTLShader_updateComputeInstanceValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues)
{
	DS_UNUSED(resourceManager);
	return bindComputeUniforms(shader, NULL, commandBuffer, instanceValues,
		dsMaterialBinding_Instance);
}

bool dsMTLShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsMTLShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	DS_UNUSED(resourceManager);

	dsMTLShader* mtlShader = (dsMTLShader*)shader;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));
	dsLifetime** usedRenderPasses = mtlShader->usedRenderPasses;
	uint32_t usedRenderPassCount = mtlShader->usedRenderPassCount;
	mtlShader->usedRenderPasses = NULL;
	mtlShader->usedRenderPassCount = 0;
	mtlShader->maxUsedRenderPasses = 0;

	dsMTLPipeline** pipelines = mtlShader->pipelines;
	uint32_t pipelineCount = mtlShader->pipelineCount;
	mtlShader->pipelines = NULL;
	mtlShader->pipelineCount = 0;
	mtlShader->maxPipelines = 0;
	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));

	for (uint32_t i = 0; i < usedRenderPassCount; ++i)
	{
		dsRenderPass* renderPass = (dsRenderPass*)dsLifetime_acquire(usedRenderPasses[i]);
		if (renderPass)
		{
			dsMTLRenderPass_removeShader(renderPass, shader);
			dsLifetime_release(usedRenderPasses[i]);
		}
		dsLifetime_freeRef(usedRenderPasses[i]);
	}
	DS_VERIFY(dsAllocator_free(mtlShader->scratchAllocator, usedRenderPasses));
	DS_ASSERT(!mtlShader->usedRenderPasses);

	dsLifetime_destroy(mtlShader->lifetime);

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (mtlShader->stages[i].function)
			CFRelease(mtlShader->stages[i].function);
	}

	if (mtlShader->depthStencilState)
		CFRelease(mtlShader->depthStencilState);
	for (uint32_t i = 0; i < shader->pipeline->samplerStateCount; ++i)
	{
		if (mtlShader->samplers[i])
			CFRelease(mtlShader->samplers[i]);
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
		dsMTLPipeline_destroy(pipelines[i]);
	DS_VERIFY(dsAllocator_free(mtlShader->scratchAllocator, pipelines));

	if (mtlShader->computePipeline)
		CFRelease(mtlShader->computePipeline);

	if (shader->allocator)
		DS_VERIFY(dsAllocator_free(shader->allocator, shader));
	return true;
}

id<MTLRenderPipelineState> dsMTLShader_getPipeline(dsShader* shader, dsCommandBuffer* commandBuffer,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry)
{
	dsRenderPass* renderPass = (dsRenderPass*)commandBuffer->boundRenderPass;
	if (!renderPass)
		return NULL;

	const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	if (!mtlShader->stages[mslStage_Vertex].function)
		return NULL;

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
		referenceAttachment = subpass->depthStencilAttachment;

	uint32_t samples = DS_DEFAULT_ANTIALIAS_SAMPLES;
	if (referenceAttachment != DS_NO_ATTACHMENT)
		samples = attachments[referenceAttachment].samples;

	if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = renderer->surfaceSamples;

	const dsMTLDrawGeometry* mtlDrawGeometry = (dsMTLDrawGeometry*)geometry;
	uint32_t hash = dsMTLPipeline_hash(samples, primitiveType, mtlDrawGeometry->vertexHash,
		renderPass, subpassIndex);

	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));

	// Search for an existing pipeline
	for (uint32_t i = 0; i < mtlShader->pipelineCount; ++i)
	{
		dsMTLPipeline* pipeline = mtlShader->pipelines[i];
		if (dsMTLPipeline_isEquivalent(pipeline, hash, samples, primitiveType, geometry, renderPass,
				subpassIndex))
		{
			id<MTLRenderPipelineState> mtlPipeline =
				(__bridge id<MTLRenderPipelineState>)pipeline->pipeline;
			DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
			return mtlPipeline;
		}
	}

	// Add a new pipeline if not present.
	uint32_t index = mtlShader->pipelineCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(mtlShader->scratchAllocator, mtlShader->pipelines,
		mtlShader->pipelineCount, mtlShader->maxPipelines, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return 0;
	}

	mtlShader->pipelines[index] = dsMTLPipeline_create(mtlShader->scratchAllocator, shader,
		hash, samples, primitiveType, geometry, renderPass, subpassIndex);
	if (!mtlShader->pipelines[index])
	{
		--mtlShader->pipelineCount;
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return 0;
	}

	// Register the render pass.
	bool hasRenderPass = false;
	for (uint32_t i = 0; i < mtlShader->usedRenderPassCount; ++i)
	{
		if (mtlShader->usedRenderPasses[i] == mtlRenderPass->lifetime)
		{
			hasRenderPass = true;
			break;
		}
	}

	if (!hasRenderPass)
	{
		uint32_t passIndex = mtlShader->usedRenderPassCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(mtlShader->scratchAllocator, mtlShader->usedRenderPasses,
			mtlShader->usedRenderPassCount, mtlShader->maxUsedRenderPasses, 1))
		{
			dsMTLPipeline_destroy(mtlShader->pipelines[index]);
			--mtlShader->pipelineCount;
			DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
			return 0;
		}

		mtlShader->usedRenderPasses[passIndex] = dsLifetime_addRef(mtlRenderPass->lifetime);
	}

	id<MTLRenderPipelineState> mtlPipeline =
		(__bridge id<MTLRenderPipelineState>)mtlShader->pipelines[index]->pipeline;
	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));

	if (!hasRenderPass)
		dsMTLRenderPass_addShader(renderPass, shader);

	return mtlPipeline;
}

void dsMTLShader_removeRenderPass(dsShader* shader, dsRenderPass* renderPass)
{
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));

	// Unregister the render pass.
	bool wasRegistered = false;
	for (uint32_t i = 0; i < mtlShader->usedRenderPassCount; ++i)
	{
		dsLifetime* renderPassLifetime = mtlShader->usedRenderPasses[i];
		if (renderPassLifetime == mtlRenderPass->lifetime)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlShader->usedRenderPasses,
				mtlShader->usedRenderPassCount, i, 1));
			dsLifetime_freeRef(renderPassLifetime);
			wasRegistered = true;
			break;
		}
	}

	if (!wasRegistered)
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return;
	}

	// Remove all pipelines for the render pass.
	for (uint32_t i = 0; i < mtlShader->pipelineCount;)
	{
		if (mtlShader->pipelines[i]->renderPass == mtlRenderPass->lifetime)
		{
			dsMTLPipeline_destroy(mtlShader->pipelines[i]);
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlShader->pipelines, mtlShader->pipelineCount, i,
				1));
		}
		else
			++i;
	}

	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
}
