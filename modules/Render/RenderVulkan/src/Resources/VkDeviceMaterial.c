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

#include "Resources/VkDeviceMaterial.h"

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkMaterialDesc.h"
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

	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	DS_ASSERT(materialDesc);
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	const dsVkBindingCounts* bindingCounts =
		&vkMaterialDesc->bindings[dsMaterialBinding_Material].bindingCounts;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDeviceMaterial)) +
		DS_ALIGNED_SIZE(sizeof(VkWriteDescriptorSet)*bindingCounts->total) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorImageInfo)*bindingCounts->textures) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*bindingCounts->textures) +
		DS_ALIGNED_SIZE(sizeof(VkDescriptorBufferInfo)*bindingCounts->buffers) +
		DS_ALIGNED_SIZE(sizeof(dsVkGfxBufferBinding)*bindingCounts->buffers) +
		DS_ALIGNED_SIZE(sizeof(VkBufferView)*bindingCounts->texelBuffers) +
		DS_ALIGNED_SIZE(sizeof(dsVkTexelBufferBinding)*bindingCounts->texelBuffers);
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

	dsVkBindingMemory* bindingMemory = &deviceMaterial->bindingMemory;
	bindingMemory->counts = *bindingCounts;
	if (bindingCounts->total > 0)
	{
		bindingMemory->bindings = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkWriteDescriptorSet, bindingCounts->total);
		DS_ASSERT(bindingMemory->bindings);
	}
	else
		bindingMemory->bindings = NULL;

	if (bindingCounts->textures > 0)
	{
		bindingMemory->imageInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkDescriptorImageInfo, bindingCounts->textures);
		DS_ASSERT(bindingMemory->imageInfos);

		bindingMemory->textures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsTexture*, bindingCounts->textures);
		DS_ASSERT(bindingMemory->textures);
	}
	else
	{
		bindingMemory->imageInfos = NULL;
		bindingMemory->textures = NULL;
	}

	if (bindingCounts->buffers > 0)
	{
		bindingMemory->bufferInfos = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkDescriptorBufferInfo, bindingCounts->buffers);
		DS_ASSERT(bindingMemory->bufferInfos);

		bindingMemory->buffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkGfxBufferBinding, bindingCounts->buffers);
		DS_ASSERT(bindingMemory->buffers);
	}
	else
	{
		bindingMemory->bufferInfos = NULL;
		bindingMemory->buffers = NULL;
	}

	if (bindingCounts->texelBuffers > 0)
	{
		bindingMemory->bufferViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkBufferView, bindingCounts->texelBuffers);
		DS_ASSERT(bindingMemory->bufferViews);

		bindingMemory->texelBuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVkTexelBufferBinding, bindingCounts->texelBuffers);
		DS_ASSERT(bindingMemory->texelBuffers);
	}
	else
	{
		bindingMemory->bufferViews = NULL;
		bindingMemory->texelBuffers = NULL;
	}

	dsSpinlock_initialize(&deviceMaterial->lock);
	return deviceMaterial;
}

void dsVkDeviceMaterial_destroy(dsResourceManager* resourceManager, dsMaterial* material,
	dsDeviceMaterial* deviceMaterial)
{
	DS_UNUSED(resourceManager);

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&deviceMaterial->lock));
	dsVkMaterialDescriptorRef* descriptors = deviceMaterial->descriptors;
	uint32_t descriptorCount = deviceMaterial->descriptorCount;
	deviceMaterial->descriptors = NULL;
	deviceMaterial->descriptorCount = 0;
	deviceMaterial->maxDescriptors = 0;
	DS_VERIFY(dsSpinlock_unlock(&deviceMaterial->lock));

	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	for (uint32_t i = 0; i < descriptorCount; ++i)
	{
		dsShader* shader = (dsShader*)dsLifetime_acquire(descriptors[i].shader);
		if (shader)
		{
			dsVkShader_removeMaterial(shader, deviceMaterial);
			dsLifetime_release(descriptors[i].shader);
		}
		dsLifetime_freeRef(descriptors[i].shader);
		dsVkMaterialDesc_freeDescriptor(materialDesc, descriptors[i].descriptor);
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
	dsVkShader* vkShader = (dsVkShader*)shader;
	dsVkMaterialDescriptor* descriptor = NULL;
	DS_VERIFY(dsSpinlock_lock(&material->lock));
	for (uint32_t i = 0; i < material->descriptorCount; ++i)
	{
		if (material->descriptors[i].shader == vkShader->lifetime)
		{
			descriptor = material->descriptors[i].descriptor;
			dsLifetime_freeRef(material->descriptors[i].shader);
			material->descriptors[i] = material->descriptors[material->descriptorCount - 1];
			--material->descriptorCount;
			break;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&material->lock));

	dsVkMaterialDesc_freeDescriptor(shader->materialDesc, descriptor);
}

VkDescriptorSet dsVkDeviceMaterial_getDescriptorSet(dsCommandBuffer* commandBuffer,
	dsDeviceMaterial* material, dsShader* shader)
{
	dsVkShader* vkShader = (dsVkShader*)shader;
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material->material);
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	dsVkBindingMemory* bindingMemory = &material->bindingMemory;
	if (bindingMemory->counts.total == 0)
		return 0;

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
		if (material->descriptors[i].shader == vkShader->lifetime)
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

		material->descriptors[index].descriptor = NULL;
		material->descriptors[index].shader = dsLifetime_addRef(vkShader->lifetime);
	}

	// Grab the list of resources needed to bind.
	uint32_t textureIndex = 0;
	uint32_t bufferIndex = 0;
	uint32_t texelBufferIndex = 0;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->binding != dsMaterialBinding_Material ||
			vkMaterialDesc->elementMappings[i] == DS_MATERIAL_UNKNOWN)
		{
			continue;
		}

		switch (element->type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				DS_ASSERT(textureIndex < bindingMemory->counts.textures);
				dsTexture* texture = dsMaterial_getTexture(material->material, i);
				if (texture && !dsVkTexture_addMemoryBarrier(texture, commandBuffer))
				{
					DS_VERIFY(dsSpinlock_unlock(&material->lock));
					return 0;
				}

				bindingMemory->textures[textureIndex] = texture;
				++textureIndex;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				DS_ASSERT(texelBufferIndex < bindingMemory->counts.texelBuffers);
				dsVkTexelBufferBinding* binding = bindingMemory->texelBuffers + texelBufferIndex;
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
				DS_ASSERT(bufferIndex < bindingMemory->counts.buffers);
				dsVkGfxBufferBinding* binding = bindingMemory->buffers + bufferIndex;
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
				DS_ASSERT(bufferIndex < bindingMemory->counts.buffers);
				dsVkGfxBufferBinding* binding = bindingMemory->buffers + bufferIndex;
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
				++bufferIndex;
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	DS_ASSERT(textureIndex == bindingMemory->counts.textures);
	DS_ASSERT(bufferIndex == bindingMemory->counts.buffers);
	DS_ASSERT(texelBufferIndex == bindingMemory->counts.texelBuffers);

	// Create the descriptor if new or if the resources have changed.
	dsVkMaterialDescriptor* descriptor = material->descriptors[index].descriptor;
	if (!descriptor || !dsVkMaterialDescriptor_isUpToDate(descriptor, bindingMemory, samplers))
	{
		dsVkMaterialDesc_freeDescriptor(materialDesc, descriptor);

		descriptor = dsVkMaterialDesc_createDescriptor(materialDesc, material->scratchAllocator,
			false);
		if (!descriptor)
		{
			material->descriptors[index].descriptor = NULL;
			DS_VERIFY(dsSpinlock_unlock(&material->lock));
			return 0;
		}

		dsVkMaterialDescriptor_update(descriptor, shader, bindingMemory, samplers);
		material->descriptors[index].descriptor = descriptor;
	}

	if (!dsVkCommandBuffer_addResource(commandBuffer, &descriptor->resource))
	{
		DS_VERIFY(dsSpinlock_unlock(&material->lock));
		return 0;
	}

	DS_VERIFY(dsSpinlock_unlock(&material->lock));

	return descriptor->set;
}
