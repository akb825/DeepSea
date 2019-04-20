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

#include "VkSharedDescriptorSets.h"

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkShader.h"
#include "Resources/VkTexture.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <string.h>

// Arbitrary max values for each pool.
#define DS_MAX_SETS 100
#define DS_MAX_TEXTURES 400
#define DS_MAX_IMAGES 100
#define DS_MAX_INPUT_ATTACHMENTS 100
#define DS_MAX_TEXEL_BUFFERS 50
#define DS_MAX_MUTABLE_TEXEL_BUFFERS 50
#define DS_MAX_UNIFORM_BUFFERS 300
#define DS_MAX_DYNAMIC_UNIFORM_BUFFERS 200
#define DS_MAX_DYNAMIC_STORAGE_BUFFERS 100

static bool setupElements(bool* outIsEqual, dsVkSharedDescriptorSets* descriptors,
	dsCommandBuffer* commandBuffer, dsShader* shader,
	const dsSharedMaterialValues* sharedValues, VkDescriptorSetLayout layout)
{
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	dsVkShader* vkShader = (dsVkShader*)shader;

	dsVkSamplerList* samplers = NULL;
	if (vkShader->samplerCount > false)
	{
		samplers = dsVkShader_getSamplerList(shader, commandBuffer);
		if (!samplers)
			return false;
	}

	if (!layout)
		return false;

	*outIsEqual = layout == descriptors->lastLayout;
	uint32_t prevImageCount = descriptors->imageCount;
	uint32_t prevBufferCount = descriptors->bufferCount;
	uint32_t prevTexelBufferCount = descriptors->texelBufferCount;
	uint32_t prevBindingInfoCount = descriptors->bindingInfoCount;

	descriptors->imageCount = 0;
	descriptors->bufferCount = 0;
	descriptors->texelBufferCount = 0;
	descriptors->offsetCount = 0;
	descriptors->bindingInfoCount = 0;

	// Clear out the last layout now so if an error occurs it won't try to use the last descriptor
	// set. It will be re-assigned on success.
	descriptors->lastLayout = 0;

	uint32_t bindingIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (!element->isShared || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		bool hasBinding = true;
		VkDescriptorType type = dsVkDescriptorType(element->type, true);
		uint32_t resourceIndex = 0;

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				dsTexture* texture = dsSharedMaterialValues_getTextureId(sharedValues,
					element->nameId);
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				VkDescriptorImageInfo imageInfo;
				if (texture)
				{
					if (!dsVkTexture_addMemoryBarrier(texture, commandBuffer))
						return false;

					imageInfo.imageView = vkTexture->deviceImageView;
					imageInfo.imageLayout = dsVkTexture_bindImageLayout(texture);
				}
				else
				{
					imageInfo.imageView = 0;
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					hasBinding = false;
				}

				uint32_t samplerIndex = vkShader->samplerMapping[i].samplerIndex;
				DS_ASSERT(samplers && samplerIndex < samplers->samplerCount);
				imageInfo.sampler = samplers->samplers[samplerIndex];

				uint32_t index = descriptors->imageCount;
				resourceIndex = index;
				if (index >= prevImageCount ||
					descriptors->images[index].sampler != imageInfo.sampler ||
					descriptors->images[index].imageView != imageInfo.imageView ||
					descriptors->images[index].imageLayout != imageInfo.imageLayout)
				{
					*outIsEqual = false;
				}

				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->images,
						descriptors->imageCount, descriptors->maxImages, 1))
				{
					return false;
				}

				descriptors->images[index] = imageInfo;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				dsGfxFormat format;
				size_t offset;
				size_t count;
				dsGfxBuffer* buffer = dsSharedMaterialValues_getTextureBufferId(&format, &offset,
					&count, sharedValues, element->nameId);

				VkBufferView bufferView = 0;
				if (buffer)
				{
					size_t size = count*dsGfxFormat_size(format);
					dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!bufferData || !dsVkGfxBufferData_addMemoryBarrier(bufferData, offset, size,
							commandBuffer))
					{
						return false;
					}

					bufferView = dsVkGfxBufferData_getBufferView(bufferData, format, offset, count);
				}
				else
				{
					hasBinding = false;
					--bindingIndex;
				}

				uint32_t index = descriptors->texelBufferCount;
				resourceIndex = index;
				if (index >= prevTexelBufferCount || descriptors->texelBuffers[index] != bufferView)
					*outIsEqual = false;

				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->texelBuffers,
						descriptors->texelBufferCount, descriptors->maxTexelBuffers, 1))
				{
					return false;
				}

				descriptors->texelBuffers[index] = bufferView;
				break;
			}
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				VkDescriptorBufferInfo bufferInfo = {0, 0, 0};
				dsGfxBuffer* buffer = NULL;
				if (element->type == dsMaterialType_VariableGroup)
				{
					dsShaderVariableGroup* group = dsSharedMaterialValues_getVariableGroupId(
						sharedValues, element->nameId);
					if (group)
					{
						buffer = dsShaderVariableGroup_getGfxBuffer(group);
						bufferInfo.range = buffer->size;
					}
				}
				else
				{
					uint32_t offsetIndex = descriptors->offsetCount;
					if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->offsets,
							descriptors->offsetCount, descriptors->maxOffsets, 1))
					{
						return false;
					}

					size_t offset = 0;
					size_t size = 0;
					buffer = dsSharedMaterialValues_getBufferId(&offset, &size, sharedValues,
						element->nameId);
					descriptors->offsets[offsetIndex] = (uint32_t)offset;
					bufferInfo.range = size;
				}

				if (buffer)
				{
					dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!bufferData || !dsVkGfxBufferData_addMemoryBarrier(bufferData,
							bufferInfo.offset, bufferInfo.range, commandBuffer))
					{
						return false;
					}

					bufferInfo.buffer = dsVkGfxBufferData_getBuffer(bufferData);
				}
				else
				{
					hasBinding = false;
					--bindingIndex;
				}

				// Offset is allowed to change.
				uint32_t index = descriptors->bufferCount;
				resourceIndex = index;
				if (index >= prevBufferCount ||
					descriptors->buffers[index].buffer != bufferInfo.buffer ||
					descriptors->buffers[index].range != bufferInfo.range)
				{
					*outIsEqual = false;
				}

				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->buffers,
						descriptors->bufferCount, descriptors->maxBuffers, 1))
				{
					return false;
				}

				descriptors->buffers[index] = bufferInfo;
				break;
			}
			default:
				DS_ASSERT(false);
		}

		if (hasBinding)
		{
			if (bindingIndex >= prevBindingInfoCount || descriptors->bindingInfos[i].type != type ||
				descriptors->bindingInfos[i].resourceIndex != resourceIndex)
			{
				*outIsEqual = false;
			}

			uint32_t index = descriptors->bindingInfoCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->bindingInfos,
					descriptors->bindingInfoCount, descriptors->maxBindingInfos, 1))
			{
				return false;
			}

			descriptors->bindingInfos[index].type = type;
			descriptors->bindingInfos[index].resourceIndex = resourceIndex;
		}
	}

	if (descriptors->imageCount != prevImageCount ||
		descriptors->bufferCount != prevBufferCount ||
		descriptors->texelBufferCount != prevTexelBufferCount ||
		descriptors->bindingInfoCount != prevBindingInfoCount)
	{
		*outIsEqual = false;
	}

	return true;
}

static bool createDescriptorPool(dsVkSharedDescriptorSets* descriptors)
{
	dsVkDevice* device = descriptors->device;
	dsVkInstance* instance = &device->instance;

	uint32_t index = descriptors->descriptorPoolCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator,descriptors->descriptorPools,
		descriptors->descriptorPoolCount, descriptors->maxDescriptorPools, 1))
	{
		return false;
	}

	VkDescriptorPoolSize poolSizes[] =
	{
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DS_MAX_TEXTURES},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DS_MAX_IMAGES},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DS_MAX_INPUT_ATTACHMENTS},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DS_MAX_TEXEL_BUFFERS},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DS_MAX_MUTABLE_TEXEL_BUFFERS},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DS_MAX_UNIFORM_BUFFERS},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DS_MAX_DYNAMIC_UNIFORM_BUFFERS},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DS_MAX_DYNAMIC_STORAGE_BUFFERS}
	};

	VkDescriptorPoolCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		DS_MAX_SETS,
		DS_ARRAY_SIZE(poolSizes), poolSizes
	};

	VkResult result = DS_VK_CALL(device->vkCreateDescriptorPool)(device->device, &createInfo,
		instance->allocCallbacksPtr, descriptors->descriptorPools + index);
	if (!dsHandleVkResult(result))
	{
		--descriptors->descriptorPoolCount;
		return false;
	}

	return true;
}

static VkResult createSetFromPool(VkDescriptorSet* outDescriptorSet,
	dsVkSharedDescriptorSets* descriptors, VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
	dsVkDevice* device = descriptors->device;

	VkDescriptorSetAllocateInfo allocateInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		pool,
		1, &layout
	};

	return DS_VK_CALL(device->vkAllocateDescriptorSets)(device->device, &allocateInfo,
		outDescriptorSet);
}

static bool setDescriptorBindings(dsVkSharedDescriptorSets* descriptors,
	const dsMaterialDesc* materialDesc, VkDescriptorSet descriptorSet)
{
	dsVkDevice* device = descriptors->device;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	uint32_t bindingCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->bindings, bindingCount,
			descriptors->maxTexelBuffers, descriptors->bindingInfoCount))
	{
		return false;
	}

	uint32_t index = 0;
	uint32_t imageIndex = 0;
	uint32_t bufferIndex = 0;
	uint32_t texelBufferIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (!element->isShared || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		VkDescriptorImageInfo* pImageInfo = NULL;
		VkBufferView* pTexelBufferView = NULL;
		VkDescriptorBufferInfo* pBufferInfo = NULL;
		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
				DS_ASSERT(imageIndex < descriptors->imageCount);
				if (descriptors->images[imageIndex].imageView)
					pImageInfo = descriptors->images + imageIndex;
				++imageIndex;
				if (!pImageInfo)
					continue;
				break;
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
				DS_ASSERT(texelBufferIndex < descriptors->texelBufferCount);
				if (descriptors->texelBuffers[texelBufferIndex])
					pTexelBufferView = descriptors->texelBuffers + texelBufferIndex;
				++texelBufferIndex;
				if (!pTexelBufferView)
					continue;
				break;
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
				DS_ASSERT(bufferIndex < descriptors->bufferCount);
				if (descriptors->buffers[bufferIndex].buffer)
					pBufferInfo = descriptors->buffers + bufferIndex;
				++bufferIndex;
				if (!pBufferInfo)
					continue;
				break;
			default:
				DS_ASSERT(false);
		}

		DS_ASSERT(index < bindingCount);
		VkWriteDescriptorSet* binding = descriptors->bindings + index;
		binding->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		binding->pNext = NULL;
		binding->dstSet = descriptorSet;
		binding->dstBinding = vkMaterialDesc->elementMappings[i];
		binding->dstArrayElement = 0;
		binding->descriptorCount = 1;
		binding->descriptorType = descriptors->bindingInfos[index].type;
		binding->pImageInfo = pImageInfo;
		binding->pBufferInfo = pBufferInfo;
		binding->pTexelBufferView = pTexelBufferView;
		++index;
	}

	DS_ASSERT(index == bindingCount);
	DS_ASSERT(imageIndex == descriptors->imageCount);
	DS_ASSERT(bufferIndex == descriptors->bufferCount);
	DS_ASSERT(texelBufferIndex == descriptors->texelBufferCount);

	if (bindingCount > 0)
	{
		DS_VK_CALL(device->vkUpdateDescriptorSets)(device->device, bindingCount,
			descriptors->bindings, 0, NULL);
	}
	return true;
}

void dsVkSharedDescriptorSets_initialize(dsVkSharedDescriptorSets* descriptors,
	dsAllocator* allocator, dsVkDevice* device)
{
	memset(descriptors, 0, sizeof(*descriptors));
	descriptors->allocator = dsAllocator_keepPointer(allocator);
	descriptors->device = device;
}

VkDescriptorSet dsVkSharedDescriptorSets_createSet(dsVkSharedDescriptorSets* descriptors,
	dsCommandBuffer* commandBuffer, dsShader* shader,
	const dsSharedMaterialValues* sharedValues)
{
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	VkDescriptorSetLayout layout = vkMaterialDesc->bindings[1].descriptorSets;
	bool isEqual = false;
	if (!setupElements(&isEqual, descriptors, commandBuffer, shader, sharedValues, layout))
		return 0;

	if (isEqual)
	{
		DS_ASSERT(descriptors->lastDescriptorSet);
		// Need to re-assign the last layout.
		descriptors->lastLayout = layout;
		return descriptors->lastDescriptorSet;
	}

	if (descriptors->descriptorPoolCount == 0)
	{
		if (!createDescriptorPool(descriptors))
			return 0;
	}

	DS_ASSERT(descriptors->descriptorPoolCount > 0);
	VkDescriptorSet descriptorSet;
	VkResult result = createSetFromPool(&descriptorSet, descriptors,
		descriptors->descriptorPools[descriptors->descriptorPoolCount - 1], layout);
	if (result != VK_SUCCESS)
	{
		if (!createDescriptorPool(descriptors))
			return 0;

		result = createSetFromPool(&descriptorSet, descriptors,
			descriptors->descriptorPools[descriptors->descriptorPoolCount - 1], layout);
		if (!dsHandleVkResult(result))
			return 0;
	}

	if (!setDescriptorBindings(descriptors, materialDesc, descriptorSet))
		return 0;

	descriptors->lastDescriptorSet = descriptorSet;
	descriptors->lastLayout = layout;

	return descriptorSet;
}

void dsVkSharedDescriptorSets_clear(dsVkSharedDescriptorSets* descriptors)
{
	dsVkDevice* device = descriptors->device;
	for (uint32_t i = 0; i < descriptors->descriptorPoolCount; ++i)
	{
		DS_VK_CALL(device->vkResetDescriptorPool)(device->device, descriptors->descriptorPools[i],
			0);
	}
	descriptors->lastLayout = 0;
	descriptors->lastDescriptorSet = 0;
}

void dsVkSharedDescriptorSets_shutdown(dsVkSharedDescriptorSets* descriptors)
{
	dsVkDevice* device = descriptors->device;
	dsVkInstance* instance = &device->instance;
	for (uint32_t i = 0; i < descriptors->descriptorPoolCount; ++i)
	{
		DS_VK_CALL(device->vkDestroyDescriptorPool)(device->device, descriptors->descriptorPools[i],
			instance->allocCallbacksPtr);
	}
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->descriptorPools));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->images));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->buffers));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->texelBuffers));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->bindingInfos));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->bindings));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->offsets));
}
