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

#include "Resources/VkMaterialDescriptor.h"

#include "Resources/VkGfxBufferData.h"
#include "Resources/VkResource.h"
#include "Resources/VkTexture.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/Material.h>
#include <string.h>

#define DS_MAX_DESCRIPTOR_SETS (uint32_t)(VK_DESCRIPTOR_TYPE_END_RANGE + 1)

dsVkMaterialDescriptor* dsVkMaterialDescriptor_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsMaterialDesc* materialDesc, const dsVkBindingCounts* counts, bool isShared)
{
	DS_ASSERT(counts->total > 0);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkMaterialDescriptor)) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*counts->textures) +
		DS_ALIGNED_SIZE(sizeof(dsVkGfxBufferBinding)*counts->buffers) +
		DS_ALIGNED_SIZE(sizeof(dsVkTexelBufferBinding)*counts->texelBuffers);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkMaterialDescriptor* descriptor = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkMaterialDescriptor);
	DS_ASSERT(descriptor);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	VkDescriptorSetLayout layout = vkMaterialDesc->bindings[isShared != 0].descriptorSets;

	descriptor->renderer = renderer;
	descriptor->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&descriptor->resource);
	descriptor->materialDesc = materialDesc;
	descriptor->samplers = NULL;

	if (counts->textures > 0)
	{
		descriptor->textures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsTexture*,
			counts->textures);
		DS_ASSERT(descriptor->textures);
	}
	else
		descriptor->textures = NULL;

	if (counts->buffers > 0)
	{
		descriptor->buffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkGfxBufferBinding, counts->buffers);
		DS_ASSERT(descriptor->buffers);
	}
	else
		descriptor->buffers = NULL;

	if (counts->texelBuffers > 0)
	{
		descriptor->texelBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkTexelBufferBinding, counts->texelBuffers);
		DS_ASSERT(descriptor->texelBuffers);
	}
	else
		descriptor->texelBuffers = NULL;

	descriptor->counts = *counts;
	descriptor->isShared = isShared;

	descriptor->pool = 0;
	descriptor->set = 0;

	if (!layout)
		return descriptor;

	VkDescriptorPoolSize sizes[DS_MAX_DESCRIPTOR_SETS];
	uint32_t poolSizeCount = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isShared || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		VkDescriptorType type = dsVkDescriptorType(element->type, false);
		DS_ASSERT(type != VK_DESCRIPTOR_TYPE_MAX_ENUM);

		uint32_t index;
		for (index = 0; index < poolSizeCount; ++index)
		{
			if (sizes[index].type == type)
				break;
		}

		if (index == poolSizeCount)
		{
			DS_ASSERT(index < DS_MAX_DESCRIPTOR_SETS);
			++poolSizeCount;

			sizes[index].type = type;
			sizes[index].descriptorCount = 0;
		}

		++sizes[index].descriptorCount;
	}

	VkDescriptorPoolCreateInfo poolCreateInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		1,
		poolSizeCount, sizes
	};

	VkResult result = DS_VK_CALL(device->vkCreateDescriptorPool)(device->device, &poolCreateInfo,
		instance->allocCallbacksPtr, &descriptor->pool);
	if (!dsHandleVkResult(result))
	{
		dsVkMaterialDescriptor_destroy(descriptor);
		return NULL;
	}

	VkDescriptorSetAllocateInfo setAllocateInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptor->pool,
		1, &layout
	};
	result = DS_VK_CALL(device->vkAllocateDescriptorSets)(device->device, &setAllocateInfo,
		&descriptor->set);
	if (!dsHandleVkResult(result))
	{
		dsVkMaterialDescriptor_destroy(descriptor);
		return NULL;
	}

	return descriptor;
}

bool dsVkMaterialDescriptor_isUpToDate(const dsVkMaterialDescriptor* descriptor,
	const dsVkBindingMemory* bindingMemory, const dsVkSamplerList* samplers)
{
	DS_ASSERT(memcmp(&descriptor->counts, &bindingMemory->counts, sizeof(dsVkBindingCounts)) == 0);
	return samplers != descriptor->samplers &&
		memcmp(descriptor->textures, bindingMemory->textures,
			sizeof(dsTexture*)*descriptor->counts.textures) == 0 &&
		memcmp(descriptor->buffers, bindingMemory->buffers,
			sizeof(dsVkGfxBufferBinding)*descriptor->counts.buffers) == 0 &&
		memcmp(descriptor->texelBuffers, bindingMemory->texelBuffers,
			sizeof(dsVkTexelBufferBinding)*descriptor->counts.texelBuffers) == 0;
}

void dsVkMaterialDescriptor_update(dsVkMaterialDescriptor* descriptor, const dsShader* shader,
	dsVkBindingMemory* bindingMemory, const dsVkSamplerList* samplers)
{
	DS_ASSERT(shader->materialDesc == descriptor->materialDesc);
	DS_ASSERT(memcmp(&descriptor->counts, &bindingMemory->counts, sizeof(dsVkBindingCounts)) == 0);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)descriptor->renderer;
	dsVkDevice* device = &vkRenderer->device;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)descriptor->materialDesc;
	const dsVkShader* vkShader = (const dsVkShader*)shader;

	memcpy(descriptor->textures, bindingMemory->textures,
		sizeof(dsTexture*)*descriptor->counts.textures);
	memcpy(descriptor->buffers, bindingMemory->buffers,
		sizeof(dsVkGfxBufferBinding)*descriptor->counts.buffers);
	memcpy(descriptor->texelBuffers, bindingMemory->texelBuffers,
		sizeof(dsVkTexelBufferBinding)*descriptor->counts.texelBuffers);

	uint32_t index = 0;
	uint32_t imageInfoIndex = 0;
	uint32_t bufferInfoIndex = 0;
	uint32_t bufferViewIndex = 0;
	for (uint32_t i = 0; i < descriptor->materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = descriptor->materialDesc->elements + i;
		if (element->isShared || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		DS_ASSERT(index < descriptor->counts.total);
		VkWriteDescriptorSet* binding = bindingMemory->bindings + index;
		binding->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		binding->pNext = NULL;
		binding->dstSet = descriptor->set;
		binding->dstBinding = vkMaterialDesc->elementMappings[i];
		binding->dstArrayElement = 0;
		binding->descriptorCount = 1;
		binding->descriptorType = dsVkDescriptorType(element->type, element->isShared);
		binding->pImageInfo = NULL;
		binding->pBufferInfo = NULL;
		binding->pTexelBufferView = NULL;
		++index;

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				DS_ASSERT(imageInfoIndex < descriptor->counts.textures);

				dsTexture* texture = descriptor->textures[imageInfoIndex];
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				VkDescriptorImageInfo* imageInfo = bindingMemory->imageInfos + imageInfoIndex;

				if (element->type == dsMaterialType_Texture)
				{
					uint32_t samplerIndex = samplers ? vkShader->samplerMapping[i].samplerIndex :
						DS_MATERIAL_UNKNOWN;
					if (samplerIndex == DS_MATERIAL_UNKNOWN || !samplers)
						imageInfo->sampler = vkRenderer->defaultSampler;
					else
					{
						DS_ASSERT(samplers && samplerIndex < samplers->samplerCount);
						imageInfo->sampler = samplers->samplers[samplerIndex];
					}
				}
				else
					imageInfo->sampler = 0;

				if (texture)
				{
					imageInfo->imageView = vkTexture->deviceImageView;
					imageInfo->imageLayout = dsVkTexture_bindImageLayout(texture);
				}
				else
				{
					imageInfo->imageView = 0;
					imageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					--index;
				}
				binding->pImageInfo = imageInfo;
				++imageInfoIndex;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				DS_ASSERT(bufferViewIndex < descriptor->counts.texelBuffers);
				dsVkTexelBufferBinding* bufferBinding = descriptor->texelBuffers + bufferViewIndex;
				if (bufferBinding->buffer)
				{
					bindingMemory->bufferViews[bufferViewIndex] = dsVkGfxBufferData_getBufferView(
						bufferBinding->buffer, bufferBinding->format, bufferBinding->offset,
						bufferBinding->count);
				}
				else
				{
					bindingMemory->bufferViews[bufferViewIndex] = 0;
					--index;
				}
				binding->pTexelBufferView = bindingMemory->bufferViews + bufferViewIndex;
				++bufferViewIndex;
				break;
			}
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				DS_ASSERT(bufferInfoIndex < descriptor->counts.buffers);
				dsVkGfxBufferBinding* bufferBinding = descriptor->buffers + bufferInfoIndex;
				VkDescriptorBufferInfo* bufferInfo = bindingMemory->bufferInfos + bufferInfoIndex;
				if (bufferBinding->buffer)
				{
					bufferInfo->buffer = dsVkGfxBufferData_getBuffer(bufferBinding->buffer);
				}
				else
				{
					bufferInfo->buffer = 0;
					--index;
				}
				bufferInfo->offset = bufferBinding->offset;
				bufferInfo->range = bufferBinding->size;
				binding->pBufferInfo = bufferInfo;
				++bufferInfoIndex;
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	uint32_t bindingCount = index;
	DS_ASSERT(bindingCount <= descriptor->counts.total);
	DS_ASSERT(imageInfoIndex == descriptor->counts.textures);
	DS_ASSERT(bufferInfoIndex == descriptor->counts.buffers);
	DS_ASSERT(bufferViewIndex == descriptor->counts.texelBuffers);

	if (bindingCount > 0)
	{
		DS_VK_CALL(device->vkUpdateDescriptorSets)(device->device, bindingCount,
			bindingMemory->bindings, 0, NULL);
	}
}

void dsVkMaterialDescriptor_destroy(dsVkMaterialDescriptor* descriptor)
{
	dsVkDevice* device = &((dsVkRenderer*)descriptor->renderer)->device;
	dsVkInstance* instance = &device->instance;
	if (descriptor->pool)
	{
		DS_VK_CALL(device->vkDestroyDescriptorPool)(device->device, descriptor->pool,
			instance->allocCallbacksPtr);
	}

	if (descriptor->allocator)
		DS_VERIFY(dsAllocator_free(descriptor->allocator, descriptor));
}
