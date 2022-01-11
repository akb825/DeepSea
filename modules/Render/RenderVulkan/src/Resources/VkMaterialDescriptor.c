/*
* Copyright 2018-2022 Aaron Barany
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

// TODO: Add additional indices if extension types are used in the future. (e.g.
// VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR for ray tracing)
#define DS_MAX_DESCRIPTOR_SETS (uint32_t)(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1)

dsVkMaterialDescriptor* dsVkMaterialDescriptor_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsMaterialDesc* materialDesc, const dsVkBindingCounts* counts, dsMaterialBinding binding)
{
	DS_ASSERT(counts->total > 0);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkMaterialDescriptor)) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorImageInfo)*counts->textures) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorBufferInfo)*counts->buffers) +
		DS_ALIGNED_SIZE(sizeof(VkBufferView)*counts->texelBuffers);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkMaterialDescriptor* descriptor = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkMaterialDescriptor);
	DS_ASSERT(descriptor);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	VkDescriptorSetLayout layout = vkMaterialDesc->bindings[binding].descriptorSets;

	descriptor->renderer = renderer;
	descriptor->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&descriptor->resource);
	descriptor->materialDesc = materialDesc;

	descriptor->samplers = NULL;
	descriptor->refObject = NULL;
	descriptor->pointerVersion = 0;
	descriptor->offsetVersion = 0;

	if (counts->textures > 0)
	{
		descriptor->imageInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkDescriptorImageInfo,
			counts->textures);
		DS_ASSERT(descriptor->imageInfos);
		memset(descriptor->imageInfos, 0, sizeof(VkDescriptorImageInfo)*counts->textures);
	}
	else
		descriptor->imageInfos = NULL;

	if (counts->buffers > 0)
	{
		descriptor->bufferInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkDescriptorBufferInfo,
			counts->buffers);
		DS_ASSERT(descriptor->bufferInfos);
		memset(descriptor->bufferInfos, 0, sizeof(VkDescriptorBufferInfo)*counts->buffers);
	}
	else
		descriptor->bufferInfos = NULL;

	if (counts->texelBuffers > 0)
	{
		descriptor->bufferViews = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VkBufferView,
			counts->texelBuffers);
		DS_ASSERT(descriptor->bufferViews);
		memset(descriptor->bufferViews, 0, sizeof(VkBufferView)*counts->texelBuffers);
	}
	else
		descriptor->bufferViews = NULL;

	descriptor->counts = *counts;
	descriptor->binding = binding;

	descriptor->pool = 0;
	descriptor->set = 0;

	if (!layout)
		return descriptor;

	VkDescriptorPoolSize sizes[DS_MAX_DESCRIPTOR_SETS];
	uint32_t poolSizeCount = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->binding != binding ||
			vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
		{
			continue;
		}

		VkDescriptorType type = dsVkDescriptorType(element->type, binding);
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
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create descriptor pool"))
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
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't allocate descriptor sets"))
	{
		dsVkMaterialDescriptor_destroy(descriptor);
		return NULL;
	}

	return descriptor;
}

bool dsVkMaterialDescriptor_shouldCheckPointers(const dsVkMaterialDescriptor* descriptor,
	const dsVkSamplerList* samplers, const void* refObject, uint32_t pointerVersion)
{
	return descriptor->samplers != samplers || descriptor->refObject != refObject ||
		descriptor->pointerVersion != pointerVersion;
}

bool dsVkMaterialDescriptor_shouldCheckOffsets(const dsVkMaterialDescriptor* descriptor,
	uint32_t offsetVersion)
{
	return descriptor->offsetVersion != offsetVersion;
}

bool dsVkMaterialDescriptor_isUpToDate(const dsVkMaterialDescriptor* descriptor,
	const dsVkBindingMemory* bindingMemory)
{
	DS_ASSERT(memcmp(&descriptor->counts, &bindingMemory->counts, sizeof(dsVkBindingCounts)) == 0);
	return memcmp(descriptor->imageInfos, bindingMemory->imageInfos,
			sizeof(VkDescriptorImageInfo)*descriptor->counts.textures) == 0 &&
		memcmp(descriptor->bufferInfos, bindingMemory->bufferInfos,
			sizeof(VkDescriptorBufferInfo)*descriptor->counts.buffers) == 0 &&
		memcmp(descriptor->bufferViews, bindingMemory->bufferViews,
			sizeof(VkBufferView)*descriptor->counts.texelBuffers) == 0;
}

void dsVkMaterialDescriptor_updateEarlyChecks(dsVkMaterialDescriptor* descriptor,
	const dsVkSamplerList* samplers, const void* refObject, uint32_t pointerVersion,
	uint32_t offsetVersion)
{
	descriptor->samplers = samplers;
	descriptor->refObject = refObject;
	descriptor->pointerVersion = pointerVersion;
	descriptor->offsetVersion = offsetVersion;
}

void dsVkMaterialDescriptor_update(dsVkMaterialDescriptor* descriptor, const dsShader* shader,
	dsVkBindingMemory* bindingMemory, const dsVkSamplerList* samplers, const void* refObject,
	uint32_t pointerVersion, uint32_t offsetVersion)
{
	DS_UNUSED(shader);
	DS_ASSERT(shader->materialDesc == descriptor->materialDesc);
	DS_ASSERT(memcmp(&descriptor->counts, &bindingMemory->counts, sizeof(dsVkBindingCounts)) == 0);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)descriptor->renderer;
	dsVkDevice* device = &vkRenderer->device;

	memcpy(descriptor->imageInfos, bindingMemory->imageInfos,
		sizeof(VkDescriptorImageInfo)*descriptor->counts.textures);
	memcpy(descriptor->bufferInfos, bindingMemory->bufferInfos,
		sizeof(VkDescriptorBufferInfo)*descriptor->counts.buffers);
	memcpy(descriptor->bufferViews, bindingMemory->bufferViews,
		sizeof(VkBufferView)*descriptor->counts.texelBuffers);

	descriptor->samplers = samplers;
	descriptor->refObject = refObject;
	descriptor->pointerVersion = pointerVersion;
	descriptor->offsetVersion = offsetVersion;

	for (uint32_t i = 0; i < bindingMemory->counts.total; ++i)
		bindingMemory->bindings[i].dstSet = descriptor->set;

	if (bindingMemory->counts.total > 0)
	{
		DS_VK_CALL(device->vkUpdateDescriptorSets)(device->device, bindingMemory->counts.total,
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
