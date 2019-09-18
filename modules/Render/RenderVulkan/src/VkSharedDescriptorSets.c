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
#include "Resources/VkMaterialDesc.h"
#include "Resources/VkMaterialDescriptor.h"
#include "Resources/VkShader.h"
#include "Resources/VkTexture.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <string.h>

static bool setupElements(dsVkSharedDescriptorSets* descriptors, dsCommandBuffer* commandBuffer,
	dsShader* shader, const dsSharedMaterialValues* sharedValues)
{
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;

	dsVkBindingMemory* bindingMemory = &descriptors->bindingMemory;
	dsVkBindingCounts* bindingCounts = &bindingMemory->counts;

	bindingCounts->total = 0;
	bindingCounts->textures = 0;
	bindingCounts->buffers = 0;
	bindingCounts->texelBuffers = 0;
	descriptors->offsetCount = 0;

	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		const dsMaterialElement* element = materialDesc->elements + i;
		if (element->binding != descriptors->binding ||
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
				dsTexture* texture = dsSharedMaterialValues_getTextureID(sharedValues,
					element->nameID);
				if (texture && !dsVkTexture_processAndAddResource(texture, commandBuffer))
					return false;

				uint32_t index = bindingCounts->textures;
				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->textures,
						bindingCounts->textures, descriptors->maxTextures, 1))
				{
					return false;
				}

				bindingMemory->textures[index] = texture;
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				dsGfxFormat format;
				size_t offset;
				size_t count;
				dsGfxBuffer* buffer = dsSharedMaterialValues_getTextureBufferID(&format, &offset,
					&count, sharedValues, element->nameID);

				uint32_t index = bindingCounts->texelBuffers;
				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->texelBuffers,
						bindingCounts->texelBuffers, descriptors->maxTexelBuffers, 1))
				{
					return false;
				}

				if (buffer)
				{
					dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!bufferData)
						return false;

					bindingMemory->texelBuffers[index].buffer = bufferData;
					bindingMemory->texelBuffers[index].format = format;
					bindingMemory->texelBuffers[index].offset = offset;
					bindingMemory->texelBuffers[index].count = count;
					dsVkRenderer_processGfxBuffer(commandBuffer->renderer, bufferData);
				}
				else
				{
					bindingMemory->texelBuffers[index].buffer = NULL;
					bindingMemory->texelBuffers[index].format = 0;
					bindingMemory->texelBuffers[index].offset = 0;
					bindingMemory->texelBuffers[index].count = 0;
				}

				break;
			}
			case dsMaterialType_VariableGroup:
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				dsGfxBuffer* buffer = NULL;
				size_t offset = 0;
				size_t bindingOffset = 0;
				size_t size = 0;
				buffer = dsSharedMaterialValues_getBufferID(&offset, &size, sharedValues,
					element->nameID);
				// Dynamic offsets forinstance variables.
				if (buffer && descriptors->binding == dsMaterialBinding_Instance)
				{
					uint32_t offsetIndex = descriptors->offsetCount;
					if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, descriptors->offsets,
							descriptors->offsetCount, descriptors->maxOffsets, 1))
					{
						return false;
					}
					descriptors->offsets[offsetIndex] = (uint32_t)offset;
				}
				else
					bindingOffset = offset;

				uint32_t index = bindingCounts->buffers;
				if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->buffers,
						bindingCounts->buffers, descriptors->maxBuffers, 1))
				{
					return false;
				}

				if (buffer)
				{
					dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
					if (!bufferData)
					{
						return false;
					}

					bindingMemory->buffers[index].buffer = bufferData;
					bindingMemory->buffers[index].offset = bindingOffset;
					bindingMemory->buffers[index].size = size;
					dsVkRenderer_processGfxBuffer(commandBuffer->renderer, bufferData);
				}
				else
				{
					bindingMemory->buffers[index].buffer = NULL;
					bindingMemory->buffers[index].offset = 0;
					bindingMemory->buffers[index].size = 0;
				}
				break;
			}
			default:
				DS_ASSERT(false);
		}
	}

	// Allocate memory for the bindings and info arrays.
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->bindings,
			bindingCounts->total, descriptors->maxBindings,
			vkMaterialDesc->bindings[descriptors->binding].bindingCounts.total))
	{
		return false;
	}

	uint32_t dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->imageInfos, dummyCount,
			descriptors->maxImageInfos, bindingCounts->textures))
	{
		return false;
	}
	dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->bufferInfos, dummyCount,
			descriptors->maxBufferInfos, bindingCounts->buffers))
	{
		return false;
	}
	dummyCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(descriptors->allocator, bindingMemory->bufferViews, dummyCount,
			descriptors->maxBufferViews, bindingCounts->texelBuffers))
	{
		return false;
	}

	DS_ASSERT(bindingCounts->total ==
		vkMaterialDesc->bindings[descriptors->binding].bindingCounts.total);
	DS_ASSERT(bindingCounts->textures ==
		vkMaterialDesc->bindings[descriptors->binding].bindingCounts.textures);
	DS_ASSERT(bindingCounts->buffers ==
		vkMaterialDesc->bindings[descriptors->binding].bindingCounts.buffers);
	DS_ASSERT(bindingCounts->texelBuffers ==
		vkMaterialDesc->bindings[descriptors->binding].bindingCounts.texelBuffers);

	return true;
}

void dsVkSharedDescriptorSets_initialize(dsVkSharedDescriptorSets* descriptors,
	dsRenderer* renderer, dsAllocator* allocator, dsMaterialBinding binding)
{
	memset(descriptors, 0, sizeof(*descriptors));
	descriptors->renderer = renderer;
	descriptors->allocator = dsAllocator_keepPointer(allocator);
	descriptors->binding = binding;
}

VkDescriptorSet dsVkSharedDescriptorSets_createSet(dsVkSharedDescriptorSets* descriptors,
	dsCommandBuffer* commandBuffer, dsShader* shader, const dsSharedMaterialValues* sharedValues)
{
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	const dsVkMaterialDesc* vkMaterialDesc = (const dsVkMaterialDesc*)materialDesc;
	dsVkShader* vkShader = (dsVkShader*)shader;
	if (!setupElements(descriptors, commandBuffer, shader, sharedValues))
		return 0;

	dsVkSamplerList* samplers = NULL;
	if (vkShader->samplerCount > false)
	{
		samplers = dsVkShader_getSamplerList(shader, commandBuffer);
		if (!samplers)
			return 0;
	}

	if (descriptors->lastDescriptor && descriptors->lastDescriptor->materialDesc == materialDesc &&
		dsVkMaterialDescriptor_isUpToDate(descriptors->lastDescriptor, &descriptors->bindingMemory,
			samplers))
	{
		dsVkCommandBuffer_addResource(commandBuffer, &descriptors->lastDescriptor->resource);
		return descriptors->lastDescriptor->set;
	}

	dsVkSharedDescriptorSets_clearLastSet(descriptors);
	descriptors->lastDescriptor = dsVkMaterialDesc_createDescriptor(materialDesc,
		descriptors->allocator, descriptors->binding);
	if (!descriptors->lastDescriptor)
		return 0;

	descriptors->lastMaterialDesc = dsLifetime_addRef(vkMaterialDesc->lifetime);
	dsVkMaterialDescriptor_update(descriptors->lastDescriptor, shader, &descriptors->bindingMemory,
		samplers);
	dsVkCommandBuffer_addResource(commandBuffer, &descriptors->lastDescriptor->resource);
	return descriptors->lastDescriptor->set;
}

void dsVkSharedDescriptorSets_clearLastSet(dsVkSharedDescriptorSets* descriptors)
{
	if (!descriptors->lastDescriptor)
		return;

	dsMaterialDesc* materialDesc =
		(dsMaterialDesc*)dsLifetime_acquire(descriptors->lastMaterialDesc);
	if (materialDesc)
	{
		dsVkMaterialDesc_freeDescriptor(materialDesc, descriptors->lastDescriptor);
		dsLifetime_release(descriptors->lastMaterialDesc);
	}
	else
		dsVkRenderer_deleteMaterialDescriptor(descriptors->renderer, descriptors->lastDescriptor);

	dsLifetime_freeRef(descriptors->lastMaterialDesc);
	descriptors->lastMaterialDesc = NULL;
	descriptors->lastDescriptor = NULL;
}

void dsVkSharedDescriptorSets_shutdown(dsVkSharedDescriptorSets* descriptors)
{
	dsVkSharedDescriptorSets_clearLastSet(descriptors);
	dsVkBindingMemory* bindings = &descriptors->bindingMemory;
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->textures));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->buffers));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->texelBuffers));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->bindings));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->imageInfos));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->bufferInfos));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, bindings->bufferViews));
	DS_VERIFY(dsAllocator_free(descriptors->allocator, descriptors->offsets));
}
