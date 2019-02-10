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
	dsDeviceMaterial* deviceMaterial, dsShader* shader, dsVkSamplerList* samplers)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkMaterialDescriptor)) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*deviceMaterial->imageInfoCount) +
		DS_ALIGNED_SIZE(sizeof(dsVkGfxBufferBinding)*deviceMaterial->bufferInfoCount) +
		DS_ALIGNED_SIZE(sizeof(dsVkTexelBufferBinding)*deviceMaterial->bufferViewCount);
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
	const dsMaterial* material = deviceMaterial->material;
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	dsVkShader* vkShader = (dsVkShader*)shader;

	VkDescriptorSetLayout layout = vkMaterialDesc->descriptorSets[0];

	descriptor->renderer = renderer;
	descriptor->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&descriptor->resource);
	descriptor->shader = dsLifetime_addRef(vkShader->lifetime);
	descriptor->samplers = samplers;

	if (deviceMaterial->imageInfoCount > 0)
	{
		descriptor->textures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsTexture*,
			deviceMaterial->imageInfoCount);
		DS_ASSERT(descriptor->textures);
		memcpy(descriptor->textures, deviceMaterial->textures,
			sizeof(dsTexture*)*deviceMaterial->imageInfoCount);
	}
	else
		descriptor->textures = NULL;

	if (deviceMaterial->bufferInfoCount > 0)
	{
		descriptor->buffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkGfxBufferBinding, deviceMaterial->bufferInfoCount);
		DS_ASSERT(descriptor->buffers);
		memcpy(descriptor->buffers, deviceMaterial->buffers,
			sizeof(dsVkGfxBufferBinding)*deviceMaterial->bufferInfoCount);
	}
	else
		descriptor->buffers = NULL;

	if (deviceMaterial->bufferViewCount > 0)
	{
		descriptor->texelBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkTexelBufferBinding, deviceMaterial->bufferViewCount);
		DS_ASSERT(descriptor->texelBuffers);
		memcpy(descriptor->texelBuffers, deviceMaterial->texelBuffers,
			sizeof(dsVkTexelBufferBinding)*deviceMaterial->bufferViewCount);
	}
	else
		descriptor->texelBuffers = NULL;

	descriptor->pool = 0;
	descriptor->set = 0;

	if (!layout)
		return descriptor;

	VkDescriptorPoolSize sizes[DS_MAX_DESCRIPTOR_SETS];
	uint32_t poolSizeCount = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isVolatile || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
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

	DS_ASSERT(deviceMaterial->bindingCount > 0);

	uint32_t index = 0;
	uint32_t imageInfoIndex = 0;
	uint32_t bufferInfoIndex = 0;
	uint32_t bufferViewIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isVolatile || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		DS_ASSERT(index < deviceMaterial->bindingCount);
		VkWriteDescriptorSet* binding = deviceMaterial->bindings + index;
		binding->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		binding->pNext = NULL;
		binding->dstSet = descriptor->set;
		binding->dstBinding = vkMaterialDesc->elementMappings[i];
		binding->dstArrayElement = 0;
		binding->descriptorCount = 1;
		binding->descriptorType = dsVkDescriptorType(element->type, element->isVolatile);
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
				DS_ASSERT(imageInfoIndex < deviceMaterial->imageInfoCount);

				dsTexture* texture = descriptor->textures[imageInfoIndex];
				dsVkTexture* vkTexture = (dsVkTexture*)texture;
				VkDescriptorImageInfo* imageInfo = deviceMaterial->imageInfos + imageInfoIndex;

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
				}
				binding->pImageInfo = imageInfo;
				++imageInfoIndex;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_MutableTextureBuffer:
			{
				DS_ASSERT(bufferViewIndex < deviceMaterial->bufferViewCount);
				dsVkTexelBufferBinding* bufferBinding = descriptor->texelBuffers + bufferViewIndex;
				if (bufferBinding->buffer)
				{
					deviceMaterial->bufferViews[bufferViewIndex] = dsVkGfxBufferData_getBufferView(
						bufferBinding->buffer, bufferBinding->format, bufferBinding->offset,
						bufferBinding->count);
				}
				else
					deviceMaterial->bufferViews[bufferViewIndex] = 0;
				binding->pTexelBufferView = deviceMaterial->bufferViews + bufferViewIndex;
				++bufferViewIndex;
				break;
			}
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				DS_ASSERT(bufferInfoIndex < deviceMaterial->bufferInfoCount);
				dsVkGfxBufferBinding* bufferBinding = descriptor->buffers + bufferInfoIndex;
				VkDescriptorBufferInfo* bufferInfo = deviceMaterial->bufferInfos + bufferInfoIndex;
				if (bufferBinding->buffer)
				{
					bufferInfo->buffer = dsVkGfxBufferData_getBuffer(bufferBinding->buffer);
				}
				else
					bufferInfo->buffer = 0;
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

	DS_ASSERT(index == deviceMaterial->bindingCount);
	DS_ASSERT(imageInfoIndex == deviceMaterial->imageInfoCount);
	DS_ASSERT(bufferInfoIndex == deviceMaterial->bufferInfoCount);
	DS_ASSERT(bufferViewIndex == deviceMaterial->bufferViewCount);

	DS_VK_CALL(device->vkUpdateDescriptorSets)(device->device, deviceMaterial->bindingCount,
		deviceMaterial->bindings, 0, NULL);

	return descriptor;
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

	dsLifetime_freeRef(descriptor->shader);

	if (descriptor->allocator)
		DS_VERIFY(dsAllocator_free(descriptor->allocator, descriptor));
}
