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

#include "Resources/VkSamplerList.h"
#include "Resources/VkResource.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

static VkFilter textureFilter(mslFilter filter)
{
	switch (filter)
	{
		case mslFilter_Linear:
			return VK_FILTER_LINEAR;
		case mslFilter_Nearest:
		default:
			return VK_FILTER_NEAREST;
	}
}

static VkSamplerMipmapMode mipFilter(mslMipFilter filter)
{
	switch (filter)
	{
		case mslMipFilter_Linear:
		case mslMipFilter_Anisotropic:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case mslMipFilter_Nearest:
		case mslMipFilter_None:
		default:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
}

static VkSamplerAddressMode addressMode(mslAddressMode addressMode)
{
	switch (addressMode)
	{
		case mslAddressMode_MirroredRepeat:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case mslAddressMode_ClampToEdge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case mslAddressMode_ClampToBorder:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case mslAddressMode_MirrorOnce:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		case mslAddressMode_Repeat:
		default:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static VkBorderColor borderColor(mslBorderColor color)
{
	switch (color)
	{
		case mslBorderColor_TransparentIntZero:
			return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case mslBorderColor_OpaqueBlack:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case mslBorderColor_OpaqueIntZero:
			return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case mslBorderColor_OpaqueWhite:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case mslBorderColor_OpaqueIntOne:
			return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		case mslBorderColor_TransparentBlack:
		default:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	}
}

dsVkSamplerList* dsVkSamplerList_create(dsAllocator* allocator, dsShader* shader)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	uint32_t samplerCount = vkShader->samplerCount;
	uint32_t elementCount = shader->materialDesc->elementCount;
	const dsVkSamplerMapping* samplerMapping = vkShader->samplerMapping;

	DS_ASSERT(samplerCount > 0);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkSamplerList)) +
		DS_ALIGNED_SIZE(sizeof(VkSampler)*samplerCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVkSamplerList* samplers = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkSamplerList);
	if (!samplers)
		return NULL;

	mslModule* module = shader->module->module;
	uint32_t pipelineIndex = shader->pipelineIndex;

	samplers->resourceManager = shader->resourceManager;
	samplers->allocator = allocator;
	if (samplerCount > 0)
	{
		samplers->samplers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkSampler, samplerCount);
		DS_ASSERT(samplers->samplers);
		memset(samplers->samplers, 0, sizeof(VkSampler)*samplerCount);
	}
	else
		samplers->samplers = NULL;
	samplers->samplerCount = samplerCount;
	samplers->defaultAnisotropy = shader->resourceManager->renderer->defaultAnisotropy;

	dsVkDevice* device = &((dsVkRenderer*)shader->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < elementCount; ++i)
	{
		if (samplerMapping[i].samplerIndex == DS_MATERIAL_UNKNOWN)
			continue;

		mslUniform uniform;
		DS_VERIFY(mslModule_uniform(&uniform, module, pipelineIndex,
			samplerMapping[i].uniformIndex));

		mslSamplerState samplerState;
		DS_ASSERT(uniform.uniformType == mslUniformType_SampledImage);
		if (uniform.samplerIndex == MSL_UNKNOWN)
		{
			samplerState.minFilter = mslFilter_Unset;
			samplerState.magFilter = mslFilter_Unset;
			samplerState.mipFilter = mslMipFilter_Unset;
			samplerState.addressModeU = mslAddressMode_Unset;
			samplerState.addressModeV = mslAddressMode_Unset;
			samplerState.addressModeW = mslAddressMode_Unset;
			samplerState.mipLodBias = MSL_UNKNOWN_FLOAT;
			samplerState.maxAnisotropy = MSL_UNKNOWN_FLOAT;
			samplerState.minLod = MSL_UNKNOWN_FLOAT;
			samplerState.maxLod = MSL_UNKNOWN_FLOAT;
			samplerState.borderColor = mslBorderColor_Unset;
			samplerState.compareOp = mslCompareOp_Unset;
		}
		else
		{
			DS_VERIFY(mslModule_samplerState(&samplerState, module, pipelineIndex,
				uniform.samplerIndex));
		}

		float maxAnisotropy = samplerState.maxAnisotropy == MSL_UNKNOWN_FLOAT ?
			samplers->defaultAnisotropy : samplerState.maxAnisotropy;
		VkSamplerCreateInfo createInfo =
		{
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			NULL,
			0,
			textureFilter(samplerState.minFilter),
			textureFilter(samplerState.magFilter),
			mipFilter(samplerState.mipFilter),
			addressMode(samplerState.addressModeU),
			addressMode(samplerState.addressModeV),
			addressMode(samplerState.addressModeW),
			samplerState.mipLodBias == MSL_UNKNOWN_FLOAT ? 0.0f : samplerState.mipLodBias,
			samplerState.mipFilter == mslMipFilter_Anisotropic &&
				device->features.samplerAnisotropy && maxAnisotropy > 1.0f,
			maxAnisotropy,
			samplerState.compareOp != mslCompareOp_Unset,
			dsVkCompareOp(samplerState.compareOp, VK_COMPARE_OP_LESS),
			samplerState.minLod == MSL_UNKNOWN_FLOAT ? 0.0f : samplerState.minLod,
			samplerState.mipFilter == mslMipFilter_None ? 0.25f :
				(samplerState.maxLod == MSL_UNKNOWN_FLOAT ? 1000.0f : samplerState.maxLod),
			borderColor(samplerState.borderColor),
			false
		};

		VkResult result = DS_VK_CALL(device->vkCreateSampler)(device->device, &createInfo,
			instance->allocCallbacksPtr, samplers->samplers + samplerMapping[i].samplerIndex);
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't create sampler"))
		{
			dsVkSamplerList_destroy(samplers);
			return NULL;
		}
	}

	dsVkResource_initialize(&samplers->resource);
	return samplers;
}

void dsVkSamplerList_destroy(dsVkSamplerList* samplers)
{
	if (!samplers)
		return;

	dsVkDevice* device = &((dsVkRenderer*)samplers->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < samplers->samplerCount; ++i)
	{
		if (samplers->samplers[i])
		{
			DS_VK_CALL(device->vkDestroySampler)(device->device, samplers->samplers[i],
				instance->allocCallbacksPtr);
		}
	}

	dsVkResource_shutdown(&samplers->resource);
	if (samplers->allocator)
		DS_VERIFY(dsAllocator_free(samplers->allocator, samplers));
}
