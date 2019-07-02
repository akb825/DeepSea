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

#include "Resources/VkMaterialDesc.h"

#include "Resources/VkMaterialDescriptor.h"
#include "Resources/VkResource.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/List.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

typedef struct DescriptorSetInfo
{
	VkDescriptorType type;
	bool isShared;
	uint32_t count;
} DescriptorSetInfo;

dsMaterialDesc* dsVkMaterialDesc_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsMaterialElement* elements, uint32_t elementCount)
{
	uint32_t bindingCounts[3] = {0, 0, 0};
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		dsMaterialBinding binding = elements[i].binding;
		if (dsVkDescriptorType(elements[i].type, binding) != VK_DESCRIPTOR_TYPE_MAX_ENUM)
			++bindingCounts[binding];
	}

	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkMaterialDesc)) +
		DS_ALIGNED_SIZE(sizeof(dsMaterialElement)*elementCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*elementCount) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorSetLayoutBinding)*bindingCounts[0]) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorSetLayoutBinding)*bindingCounts[1]) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorSetLayoutBinding)*bindingCounts[2]);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkMaterialDesc* materialDesc = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkMaterialDesc);
	DS_ASSERT(materialDesc);

	dsMaterialDesc* baseMaterialDesc = (dsMaterialDesc*)materialDesc;
	baseMaterialDesc->resourceManager = resourceManager;
	baseMaterialDesc->allocator = dsAllocator_keepPointer(allocator);
	baseMaterialDesc->elementCount = elementCount;

	if (elementCount > 0)
	{
		baseMaterialDesc->elements = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsMaterialElement,
			elementCount);
		DS_ASSERT(baseMaterialDesc->elements);
		memcpy(baseMaterialDesc->elements, elements, sizeof(dsMaterialElement)*elementCount);

		materialDesc->elementMappings = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t,
			elementCount);
		DS_ASSERT(materialDesc->elementMappings);
		memset(materialDesc->elementMappings, 0xFF, sizeof(uint32_t)*elementCount);
	}
	else
	{
		baseMaterialDesc->elements = NULL;
		materialDesc->elementMappings = NULL;
	}

	materialDesc->lifetime = NULL;

	uint32_t setIndex = 0;
	memset(materialDesc->bindings, 0, sizeof(materialDesc->bindings));
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(materialDesc->bindings); ++i)
	{
		dsVkMaterialDescBindings* bindings = materialDesc->bindings + i;
		if (bindingCounts[i] == 0)
		{
			bindings->setIndex = DS_MATERIAL_UNKNOWN;
			continue;
		}

		bindings->setIndex = setIndex++;
		bindings->bindings = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkDescriptorSetLayoutBinding,
			bindingCounts[i]);
		DS_ASSERT(bindings->bindings);

		uint32_t index = 0;
		for (uint32_t j = 0; j < elementCount; ++j)
		{
			if (elements[j].binding != i)
				continue;

			VkDescriptorType type = dsVkDescriptorType(elements[j].type, i);
			if (type == VK_DESCRIPTOR_TYPE_MAX_ENUM)
				continue;

			switch (elements[j].type)
			{
				case dsMaterialType_Texture:
				case dsMaterialType_Image:
				case dsMaterialType_SubpassInput:
					++bindings->bindingCounts.textures;
					break;
				case dsMaterialType_TextureBuffer:
				case dsMaterialType_ImageBuffer:
					++bindings->bindingCounts.texelBuffers;
					break;
				case dsMaterialType_VariableGroup:
				case dsMaterialType_UniformBlock:
				case dsMaterialType_UniformBuffer:
					++bindings->bindingCounts.buffers;
					break;
				default:
					DS_ASSERT(false);
					break;
			}

			DS_ASSERT(index < bindingCounts[i]);
			materialDesc->elementMappings[j] = index;

			VkDescriptorSetLayoutBinding* binding = bindings->bindings + index;
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
		bindings->bindingCounts.total = bindingCounts[i];

		VkDescriptorSetLayoutCreateInfo createInfo =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			NULL,
			0,
			bindingCounts[i],
			bindings->bindings
		};

		VkResult result = DS_VK_CALL(device->vkCreateDescriptorSetLayout)(device->device,
			&createInfo, instance->allocCallbacksPtr, &bindings->descriptorSets);
		if (!dsHandleVkResult(result))
		{
			dsVkMaterialDesc_destroy(resourceManager, baseMaterialDesc);
			return NULL;
		}

		DS_VERIFY(dsList_initialize(&bindings->descriptorFreeList));
		DS_VERIFY(dsSpinlock_initialize(&bindings->lock));
	}

	materialDesc->lifetime = dsLifetime_create(allocator, materialDesc);
	if (!materialDesc->lifetime)
		dsVkMaterialDesc_destroy(resourceManager, baseMaterialDesc);

	return baseMaterialDesc;
}

bool dsVkMaterialDesc_destroy(dsResourceManager* resourceManager, dsMaterialDesc* materialDesc)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	dsVkMaterialDesc* vkMaterialDesc = (dsVkMaterialDesc*)materialDesc;

	dsLifetime_destroy(vkMaterialDesc->lifetime);

	for (uint32_t i = 0; i < DS_ARRAY_SIZE(vkMaterialDesc->bindings); ++i)
	{
		dsVkMaterialDescBindings* bindings = vkMaterialDesc->bindings + i;
		if (!bindings->descriptorSets)
			continue;

		DS_VK_CALL(device->vkDestroyDescriptorSetLayout)(device->device, bindings->descriptorSets,
			instance->allocCallbacksPtr);
		dsSpinlock_shutdown(&vkMaterialDesc->bindings->lock);

		for (dsListNode* node = bindings->descriptorFreeList.head; node; node = node->next)
			dsVkRenderer_deleteMaterialDescriptor(renderer, (dsVkMaterialDescriptor*)node);
	}

	if (materialDesc->allocator)
		DS_VERIFY(dsAllocator_free(materialDesc->allocator, materialDesc));
	return true;
}

dsVkMaterialDescriptor* dsVkMaterialDesc_createDescriptor(const dsMaterialDesc* materialDesc,
	dsAllocator* allocator, dsMaterialBinding binding)
{
	dsVkMaterialDesc* vkMaterialDesc = (dsVkMaterialDesc*)materialDesc;
	dsVkMaterialDescBindings* bindings = vkMaterialDesc->bindings + binding;
	if (!bindings->descriptorSets)
		return NULL;

	dsRenderer* renderer = materialDesc->resourceManager->renderer;
	uint64_t finishedSubmitCount = dsVkRenderer_getFinishedSubmitCount(renderer);
	DS_VERIFY(dsSpinlock_lock(&bindings->lock));
	dsVkMaterialDescriptor* descriptor = NULL;
	for (dsListNode* node = bindings->descriptorFreeList.head; node; node = node->next)
	{
		dsVkMaterialDescriptor* curDescriptor = (dsVkMaterialDescriptor*)node;
		if (!dsVkResource_isInUse(&curDescriptor->resource, finishedSubmitCount))
		{
			DS_VERIFY(dsList_remove(&bindings->descriptorFreeList, node));
			descriptor = curDescriptor;
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&bindings->lock));

	if (!descriptor)
	{
		descriptor = dsVkMaterialDescriptor_create(renderer, allocator, materialDesc,
			&bindings->bindingCounts, binding);
	}

	return descriptor;
}

void dsVkMaterialDesc_freeDescriptor(const dsMaterialDesc* materialDesc,
	dsVkMaterialDescriptor* descriptor)
{
	if (!descriptor)
		return;

	dsVkMaterialDesc* vkMaterialDesc = (dsVkMaterialDesc*)materialDesc;
	dsVkMaterialDescBindings* bindings = vkMaterialDesc->bindings + descriptor->binding;
	DS_VERIFY(dsSpinlock_lock(&bindings->lock));
	DS_VERIFY(dsList_append(&bindings->descriptorFreeList, (dsListNode*)descriptor));
	DS_VERIFY(dsSpinlock_unlock(&bindings->lock));
}
