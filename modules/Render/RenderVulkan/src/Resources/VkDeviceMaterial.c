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

#include "Resources/VkDeviceMaterial.h"
#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkMaterialDescriptor.h"
#include "Resources/VkShader.h"
#include "Resources/VkTexture.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <string.h>

dsDeviceMaterial* dsVkDeviceMaterial_create(dsResourceManager* resourceManager,
	dsMaterial* material, dsAllocator* allocator)
{
	dsAllocator* scratchAllocator = allocator;
	if (!scratchAllocator->freeFunc)
		scratchAllocator = resourceManager->allocator;

	uint32_t bindingCount = 0;
	uint32_t imageInfoCount = 0;
	uint32_t bufferInfoCount = 0;
	uint32_t bufferViewCount = 0;

	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	DS_ASSERT(materialDesc);
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isVolatile || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
				++imageInfoCount;
				break;
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_MutableTextureBuffer:
				++bufferViewCount;
				break;
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
				++bufferInfoCount;
				break;
			default:
				DS_ASSERT(false);
		}

		++bindingCount;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDeviceMaterial)) +
		DS_ALIGNED_SIZE(sizeof(VkWriteDescriptorSet)*bindingCount) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorImageInfo)*imageInfoCount) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorBufferInfo)*bufferInfoCount) +
		DS_ALIGNED_SIZE(sizeof(VkBufferView)*bufferViewCount) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*imageInfoCount) +
		DS_ALIGNED_SIZE(sizeof(dsVkGfxBufferBinding)*bufferInfoCount) +
		DS_ALIGNED_SIZE(sizeof(dsVkTexelBufferBinding)*bufferViewCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsDeviceMaterial* deviceMaterial = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsDeviceMaterial);
	DS_ASSERT(deviceMaterial);

	dsLifetime* lifetime = dsLifetime_create(scratchAllocator, deviceMaterial);
	if (!lifetime)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, deviceMaterial));
		return NULL;
	}

	deviceMaterial->resourceManager = resourceManager;
	deviceMaterial->allocator = dsAllocator_keepPointer(allocator);
	deviceMaterial->scratchAllocator = scratchAllocator;
	deviceMaterial->material = material;
	deviceMaterial->lifetime = lifetime;

	deviceMaterial->descriptors = NULL;
	deviceMaterial->descriptorCount = 0;
	deviceMaterial->maxDescriptors = 0;

	if (bindingCount > 0)
	{
		deviceMaterial->bindings = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkWriteDescriptorSet, bindingCount);
		DS_ASSERT(deviceMaterial->bindings);
	}
	else
		deviceMaterial->bindings = NULL;

	if (imageInfoCount > 0)
	{
		deviceMaterial->imageInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkDescriptorImageInfo, imageInfoCount);
		DS_ASSERT(deviceMaterial->imageInfos);

		deviceMaterial->textures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsTexture*, imageInfoCount);
		DS_ASSERT(deviceMaterial->textures);
	}
	else
	{
		deviceMaterial->imageInfos = NULL;
		deviceMaterial->textures = NULL;
	}

	if (bufferInfoCount > 0)
	{
		deviceMaterial->bufferInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkDescriptorBufferInfo, bufferViewCount);
		DS_ASSERT(deviceMaterial->bufferInfos);

		deviceMaterial->buffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkGfxBufferBinding, bufferInfoCount);
		DS_ASSERT(deviceMaterial->buffers);
	}
	else
	{
		deviceMaterial->bufferInfos = NULL;
		deviceMaterial->buffers = NULL;
	}

	if (bufferViewCount > 0)
	{
		deviceMaterial->bufferViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkBufferView, bufferViewCount);
		DS_ASSERT(deviceMaterial->bufferViews);

		deviceMaterial->texelBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkTexelBufferBinding, bufferViewCount);
		DS_ASSERT(deviceMaterial->texelBuffers);
	}
	else
	{
		deviceMaterial->bufferViews = NULL;
		deviceMaterial->texelBuffers = NULL;
	}

	deviceMaterial->bindingCount = bindingCount;
	deviceMaterial->imageInfoCount = imageInfoCount;
	deviceMaterial->bufferInfoCount = bufferInfoCount;
	deviceMaterial->bufferViewCount = bufferViewCount;

	dsSpinlock_initialize(&deviceMaterial->lock);

	uint32_t index = 0;
	uint32_t imageInfoIndex = 0;
	uint32_t bufferInfoIndex = 0;
	uint32_t bufferViewIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isVolatile || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		DS_ASSERT(index < bindingCount);
		VkWriteDescriptorSet* binding = deviceMaterial->bindings + index;
		binding->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		binding->pNext = NULL;
		binding->dstSet = 0;
		binding->dstBinding = vkMaterialDesc->elementMappings[i];
		binding->dstArrayElement = 0;
		binding->descriptorType = dsVkDescriptorType(element->type, false);
		binding->descriptorCount = 1;
		binding->pImageInfo = NULL;
		binding->pBufferInfo = NULL;
		binding->pTexelBufferView = NULL;

		++index;
		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
				DS_ASSERT(imageInfoIndex < imageInfoCount);
				binding->pImageInfo = deviceMaterial->imageInfos + imageInfoIndex;
				++imageInfoIndex;
				break;
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_MutableTextureBuffer:
				DS_ASSERT(bufferViewIndex < bufferViewCount);
				binding->pTexelBufferView = deviceMaterial->bufferViews + bufferViewIndex;
				++bufferViewIndex;
				break;
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
				DS_ASSERT(bufferInfoIndex < bufferInfoCount);
				binding->pBufferInfo = deviceMaterial->bufferInfos + bufferInfoIndex;
				++bufferInfoIndex;
				break;
			default:
				DS_ASSERT(false);
		}
	}

	DS_ASSERT(index == bindingCount);
	DS_ASSERT(imageInfoIndex == imageInfoCount);
	DS_ASSERT(bufferInfoIndex == bufferInfoCount);
	DS_ASSERT(bufferViewIndex == bufferViewCount);

	return deviceMaterial;
}

void dsVkDeviceMaterial_destroy(dsResourceManager* resourceManager, dsMaterial* material,
	dsDeviceMaterial* deviceMaterial)
{
	dsRenderer* renderer = resourceManager->renderer;
	DS_UNUSED(material);

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&deviceMaterial->lock));
	dsVkMaterialDescriptor** descriptors = deviceMaterial->descriptors;
	uint32_t descriptorCount = deviceMaterial->descriptorCount;
	deviceMaterial->descriptors = NULL;
	deviceMaterial->descriptorCount = 0;
	deviceMaterial->maxDescriptors = 0;
	DS_VERIFY(dsSpinlock_unlock(&deviceMaterial->lock));

	for (uint32_t i = 0; i < descriptorCount; ++i)
	{
		dsShader* shader = (dsShader*)dsLifetime_acquire(descriptors[i]->shader);
		if (shader)
		{
			dsVkShader_removeMaterial(shader, deviceMaterial);
			dsLifetime_release(descriptors[i]->shader);
		}
		dsVkRenderer_deleteMaterialDescriptor(renderer, descriptors[i]);
	}
	DS_VERIFY(dsAllocator_free(deviceMaterial->scratchAllocator, descriptors));
	DS_ASSERT(!deviceMaterial->descriptors);

	dsSpinlock_shutdown(&deviceMaterial->lock);
	dsLifetime_destroy(deviceMaterial->lifetime);
	if (deviceMaterial->allocator)
		DS_VERIFY(dsAllocator_free(deviceMaterial->allocator, deviceMaterial));
}

void dsVkDeviceMaterial_removeShader(dsDeviceMaterial* material, dsShader* shader)
{
	dsVkMaterialDescriptor* descriptor = NULL;
	DS_VERIFY(dsSpinlock_lock(&material->lock));
	for (uint32_t i = 0; i < material->descriptorCount; ++i)
	{
		void* descriptorShader = dsLifetime_getObject(material->descriptors[i]->shader);
		DS_ASSERT(descriptorShader);
		if (descriptorShader == shader)
		{
			descriptor = material->descriptors[i];
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(material->descriptors, material->descriptorCount,
				i, 1));
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&material->lock));

	if (descriptor)
		dsVkRenderer_deleteMaterialDescriptor(material->resourceManager->renderer, descriptor);
}

VkDescriptorSet dsVkDeviceMaterial_getDescriptorSet(dsCommandBuffer* commandBuffer,
	dsDeviceMaterial* material, dsShader* shader)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material->material);
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	dsRenderer* renderer = commandBuffer->renderer;

	if (!dsVkShader_addMaterial(shader, material))
		return 0;

	dsVkSamplerList* samplers = NULL;
	if (vkShader->samplerCount > 0)
	{
		samplers = dsVkShader_getSamplerList(shader, commandBuffer);
		if (!samplers)
			return 0;
	}

	// NOTE: This is a somewhat lengthy block, but contention is expected to be low, so a spinlock
	// is used.
	DS_VERIFY(dsSpinlock_lock(&material->lock));

	uint32_t index = 0;
	for (uint32_t i = 0; i < material->descriptorCount; ++i, ++index)
	{
		void* descriptorShader = dsLifetime_getObject(material->descriptors[i]->shader);
		DS_ASSERT(descriptorShader);
		if (descriptorShader == shader)
			break;
	}

	if (index == material->descriptorCount)
	{
		if (!DS_RESIZEABLE_ARRAY_ADD(material->allocator, material->descriptors,
			material->descriptorCount, material->maxDescriptors, 1))
		{
			DS_VERIFY(dsSpinlock_unlock(&material->lock));
			return 0;
		}

		material->descriptors[index] = NULL;
	}

	// Grab the list of resources needed to bind.
	uint32_t textureIndex = 0;
	uint32_t bufferIndex = 0;
	uint32_t texelBufferIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->isVolatile || vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
			continue;

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				DS_ASSERT(textureIndex < material->imageInfoCount);
				DS_ASSERT(samplers);
				dsTexture* texture = dsMaterial_getTexture(material->material, i);
				if (texture && !dsVkTexture_addMemoryBarrier(texture, commandBuffer))
				{
					DS_VERIFY(dsSpinlock_unlock(&material->lock));
					return 0;
				}

				material->textures[textureIndex] = texture;
				++textureIndex;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_MutableTextureBuffer:
			{
				DS_ASSERT(texelBufferIndex < material->bufferViewCount);
				dsVkTexelBufferBinding* binding = material->texelBuffers + texelBufferIndex;
				dsGfxBuffer* buffer = dsMaterial_getTextureBuffer(&binding->format,
					&binding->offset, &binding->count, material->material, i);

				if (buffer)
				{
					size_t size = binding->count*dsGfxFormat_size(binding->format);
					binding->buffer = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!binding->buffer || !dsVkGfxBufferData_addMemoryBarrier(binding->buffer,
							binding->offset, size, commandBuffer))
					{
						DS_VERIFY(dsSpinlock_unlock(&material->lock));
						return 0;
					}
				}
				else
				{
					binding->buffer = NULL;
					binding->format = 0;
					binding->offset = 0;
					binding->count = 0;
				}
				++texelBufferIndex;
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				DS_ASSERT(bufferIndex < material->bufferInfoCount);
				dsVkGfxBufferBinding* binding = material->buffers + bufferIndex;
				dsShaderVariableGroup* group = dsMaterial_getVariableGroup(material->material, i);
				dsGfxBuffer* buffer;
				if (group)
					buffer = dsShaderVariableGroup_getGfxBuffer(group);
				else
					buffer = NULL;

				if (buffer)
				{
					binding->buffer = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!binding->buffer || !dsVkGfxBufferData_addMemoryBarrier(binding->buffer, 0,
							buffer->size, commandBuffer))
					{
						DS_VERIFY(dsSpinlock_unlock(&material->lock));
						return 0;
					}

					binding->offset = 0;
					binding->size = buffer->size;
				}
				else
				{
					binding->buffer = NULL;
					binding->offset = 0;
					binding->size = 0;
				}
				++bufferIndex;
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				DS_ASSERT(bufferIndex < material->bufferInfoCount);
				dsVkGfxBufferBinding* binding = material->buffers + bufferIndex;
				dsGfxBuffer* buffer = dsMaterial_getBuffer(&binding->offset, &binding->size,
					material->material, i);

				if (buffer)
				{
					binding->buffer = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!binding->buffer || !dsVkGfxBufferData_addMemoryBarrier(binding->buffer,
							binding->offset, binding->size, commandBuffer))
					{
						DS_VERIFY(dsSpinlock_unlock(&material->lock));
						return 0;
					}
				}
				else
				{
					buffer = NULL;
					binding->offset = 0;
					binding->size = 0;
				}
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	DS_ASSERT(textureIndex == material->imageInfoCount);
	DS_ASSERT(bufferIndex == material->bufferInfoCount);
	DS_ASSERT(texelBufferIndex == material->bufferViewCount);

	// Create the descriptor if new or if the resources have changed.
	dsVkMaterialDescriptor* descriptor = material->descriptors[index];
	if (!descriptor || descriptor->samplers != samplers ||
		memcmp(descriptor->textures, material->textures,
			sizeof(dsTexture*)*material->imageInfoCount) != 0 ||
		memcmp(descriptor->buffers, material->buffers,
			sizeof(dsVkGfxBufferBinding)*material->bufferInfoCount) != 0 ||
		memcmp(descriptor->texelBuffers, material->texelBuffers,
			sizeof(dsVkTexelBufferBinding)*material->bufferViewCount) != 0)
	{
		if (descriptor)
			dsVkRenderer_deleteMaterialDescriptor(renderer, descriptor);

		descriptor = dsVkMaterialDescriptor_create(
			dsMaterial_getResourceManager(material->material)->renderer, material->scratchAllocator,
			material, shader, samplers);
		if (!descriptor)
		{
			material->descriptors[index] = NULL;
			DS_VERIFY(dsSpinlock_unlock(&material->lock));
			return 0;
		}

		material->descriptors[index] = descriptor;
		if (!dsVkCommandBuffer_addResource(commandBuffer, &descriptor->resource))
		{
			DS_VERIFY(dsSpinlock_unlock(&material->lock));
			return 0;
		}
	}

	DS_VERIFY(dsSpinlock_unlock(&material->lock));

	return descriptor->set;
}
