/*
 * Copyright 2018 Aaron Barany
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

#include "Resources/VkMaterialDesc.h"
#include "Resources/VkResource.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

typedef struct DescriptorSetInfo
{
	VkDescriptorType type;
	bool isVolatile;
	uint32_t count;
} DescriptorSetInfo;

dsMaterialDesc* dsVkMaterialDesc_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsMaterialElement* elements, uint32_t elementCount)
{
	uint32_t bindingCounts[2] = {0, 0};
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		// Guarantee it's 0 or 1.
		bool isVolatile = elements[i].isVolatile != false;
		if (dsVkDescriptorType(elements[i].type, isVolatile) != VK_DESCRIPTOR_TYPE_MAX_ENUM)
			++bindingCounts[isVolatile];
	}

	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkMaterialDesc)) +
		DS_ALIGNED_SIZE(sizeof(dsMaterialElement)*elementCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*elementCount) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorSetLayoutBinding)*bindingCounts[0]) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorSetLayoutBinding)*bindingCounts[1]);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkMaterialDesc* materialDesc = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkMaterialDesc);
	DS_ASSERT(materialDesc);

	dsMaterialDesc* baseMaterialDesc = (dsMaterialDesc*)materialDesc;
	baseMaterialDesc->resourceManager = resourceManager;
	baseMaterialDesc->allocator = dsAllocator_keepPointer(allocator);
	baseMaterialDesc->elementCount = elementCount;

	if (elementCount > 0)
	{
		baseMaterialDesc->elements = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsMaterialElement, elementCount);
		DS_ASSERT(baseMaterialDesc->elements);
		memcpy(baseMaterialDesc->elements, elements, sizeof(dsMaterialElement)*elementCount);

		materialDesc->elementMappings = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			uint32_t, elementCount);
		DS_ASSERT(materialDesc->elementMappings);
		memset(materialDesc->elementMappings, 0xFF, sizeof(uint32_t)*elementCount);
	}
	else
	{
		baseMaterialDesc->elements = NULL;
		materialDesc->elementMappings = NULL;
	}

	for (uint32_t i = 0; i < 2; ++i)
	{
		if (bindingCounts[i] == 0)
		{
			materialDesc->bindings[i] = 0;
			materialDesc->descriptorSets[i] = 0;
			continue;
		}

		materialDesc->bindings[i] = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkDescriptorSetLayoutBinding, bindingCounts[i]);
		DS_ASSERT(materialDesc->bindings[i]);

		uint32_t index = 0;
		for (uint32_t j = 0; j < elementCount; ++j)
		{
			bool isVolatile = elements[j].isVolatile != false;
			if (isVolatile != i)
				continue;

			VkDescriptorType type = dsVkDescriptorType(elements[j].type, i);
			if (type == VK_DESCRIPTOR_TYPE_MAX_ENUM)
				continue;

			DS_ASSERT(index < bindingCounts[i]);
			materialDesc->elementMappings[j] = index;

			VkDescriptorSetLayoutBinding* binding = materialDesc->bindings[i] + index;
			binding->binding = index;
			binding->descriptorType = type;
			binding->descriptorCount = dsMax(1U, elements[j].count);
			if (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				binding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			else
				binding->stageFlags = VK_SHADER_STAGE_ALL;
			binding->pImmutableSamplers = NULL;

			++index;
		}
		DS_ASSERT(index == bindingCounts[i]);

		VkDescriptorSetLayoutCreateInfo createInfo =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			NULL,
			0,
			bindingCounts[i],
			materialDesc->bindings[i]
		};

		VkResult result = DS_VK_CALL(device->vkCreateDescriptorSetLayout)(device->device,
			&createInfo, instance->allocCallbacksPtr, materialDesc->descriptorSets + i);
		if (!dsHandleVkResult(result))
		{
			for (uint32_t j = 0; j < i; ++j)
			{
				DS_VK_CALL(device->vkDestroyDescriptorSetLayout)(device->device,
					materialDesc->descriptorSets[i], instance->allocCallbacksPtr);
			}

			if (baseMaterialDesc->allocator)
				DS_VERIFY(dsAllocator_free(baseMaterialDesc->allocator, baseMaterialDesc));
			return NULL;
		}
	}

	return baseMaterialDesc;
}

bool dsVkMaterialDesc_destroy(dsResourceManager* resourceManager, dsMaterialDesc* materialDesc)
{
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkMaterialDesc* vkMaterialDesc = (dsVkMaterialDesc*)materialDesc;

	for (uint32_t i = 0; i < 2; ++i)
	{
		if (vkMaterialDesc->descriptorSets[i])
		{
			DS_VK_CALL(device->vkDestroyDescriptorSetLayout)(device->device,
				vkMaterialDesc->descriptorSets[i], instance->allocCallbacksPtr);
		}
	}

	if (materialDesc->allocator)
		DS_VERIFY(dsAllocator_free(materialDesc->allocator, materialDesc));
	return true;
}
