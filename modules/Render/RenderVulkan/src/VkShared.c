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

#include "VkShared.h"
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

typedef struct LastCallsite
{
	const char* lastFile;
	const char* lastFunction;
	unsigned int lastLine;
} LastCallsite;

static DS_THREAD_LOCAL LastCallsite lastCallsite;

bool dsHandleVkResult(VkResult result)
{
	switch (result)
	{
		case VK_SUCCESS:
			return true;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			errno = ENOMEM;
			return false;
		default:
			errno = EPERM;
			return false;
	}
}

void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line)
{
	LastCallsite* curLastCallsite = &lastCallsite;
	curLastCallsite->lastFile = file;
	curLastCallsite->lastFunction = function;
	curLastCallsite->lastLine = line;
}

void dsGetLastVkCallsite(const char** file, const char** function, unsigned int* line)
{
	const LastCallsite* curLastCallsite = &lastCallsite;
	*file = curLastCallsite->lastFile;
	*function = curLastCallsite->lastFunction;
	*line = curLastCallsite->lastLine;
}

uint32_t dsVkMemoryIndex(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags)
{
	uint32_t requiredFlags = 0;
	uint32_t optimalFlags = 0;
	if (!(memoryFlags & dsGfxMemory_GPUOnly))
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	if (memoryFlags & dsGfxMemory_Coherent)
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (memoryFlags & (dsGfxMemory_Dynamic | dsGfxMemory_Stream))
		optimalFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	uint32_t memoryIndex = DS_INVALID_HEAP;
	VkDeviceSize memorySize = 0;

	const VkPhysicalDeviceMemoryProperties* memoryProperties = &device->memoryProperties;
	for (uint32_t curBitmask = requirements->memoryTypeBits; curBitmask;
		curBitmask = dsRemoveLastBit(curBitmask))
	{
		uint32_t i = dsBitmaskIndex(curBitmask);
		const VkMemoryType* memoryType = memoryProperties->memoryTypes + i;
		if ((memoryType->propertyFlags & requiredFlags) != requiredFlags)
			continue;

		if (memoryIndex == DS_INVALID_HEAP)
			memoryIndex = i;

		// Find the largest optimal heap.
		VkDeviceSize size = memoryProperties->memoryHeaps[memoryType->heapIndex].size;
		if ((memoryType->propertyFlags & optimalFlags) == optimalFlags && size > memorySize)
			memoryIndex = i;
	}

	if (memoryIndex == DS_INVALID_HEAP)
	{
		errno = ENOMEM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "No suitable GPU heap found.");
	}

	return memoryIndex;
}

VkDeviceMemory dsAllocateVkMemory(const dsVkDevice* device,
	const VkMemoryRequirements* requirements, uint32_t memoryIndex)
{
	if (memoryIndex == DS_INVALID_HEAP)
	{
		errno = ENOMEM;
		return 0;
	}

	VkMemoryAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
		NULL,
		requirements->size,
		memoryIndex
	};
	const dsVkInstance* instance = &device->instance;
	VkDeviceMemory memory = 0;
	VkResult result = DS_VK_CALL(device->vkAllocateMemory)(device->device, &allocInfo,
		instance->allocCallbacksPtr, &memory);
	if (!dsHandleVkResult(result))
		return 0;

	return memory;
}

VkSampleCountFlagBits dsVkSampleCount(uint32_t sampleCount)
{
	if (sampleCount <= 1)
		return VK_SAMPLE_COUNT_1_BIT;
	else if (sampleCount <= 2)
		return VK_SAMPLE_COUNT_2_BIT;
	else if (sampleCount <= 4)
		return VK_SAMPLE_COUNT_4_BIT;
	else if (sampleCount <= 8)
		return VK_SAMPLE_COUNT_8_BIT;
	else if (sampleCount <= 16)
		return VK_SAMPLE_COUNT_16_BIT;
	else if (sampleCount <= 32)
		return VK_SAMPLE_COUNT_32_BIT;
	return VK_SAMPLE_COUNT_64_BIT;
}

VkAccessFlags dsVkSrcBufferAccessFlags(dsGfxBufferUsage usage, bool canMap)
{
	VkAccessFlags flags = 0;
	if (canMap)
		flags |= VK_ACCESS_HOST_WRITE_BIT;
	if (usage & (dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_MutableImage))
		flags |= VK_ACCESS_SHADER_WRITE_BIT;
	if (usage & dsGfxBufferUsage_CopyTo)
		flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	return flags;
}

VkAccessFlags dsVkDstBufferAccessFlags(dsGfxBufferUsage usage)
{
	VkAccessFlags flags = 0;
	if (usage & dsGfxBufferUsage_Index)
		flags |= VK_ACCESS_INDEX_READ_BIT;
	if (usage & dsGfxBufferUsage_Vertex)
		flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (usage & (dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch))
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_MutableImage))
	{
		flags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyFrom)
		flags |= VK_ACCESS_TRANSFER_READ_BIT;
	return flags;
}

VkPipelineStageFlags dsVkSrcBufferStageFlags(dsGfxBufferUsage usage, bool canMap)
{
	VkPipelineStageFlags flags = 0;
	if (canMap)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;
	if (usage & (dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_MutableImage))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyTo)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkPipelineStageFlags dsVkDstBufferStageFlags(dsGfxBufferUsage usage)
{
	VkAccessFlags flags = 0;
	if (usage & (dsGfxBufferUsage_Index | dsGfxBufferUsage_Vertex))
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	if (usage & dsGfxBufferUsage_IndirectDraw)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	if (usage & dsGfxBufferUsage_IndirectDispatch)
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_MutableImage))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyFrom)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkAccessFlags dsVkSrcImageAccessFlags(dsTextureUsage usage, bool offscreen, bool depthStencil)
{
	VkAccessFlags flags = 0;
	if (offscreen)
	{
		if (depthStencil)
			flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		else
			flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (usage & dsTextureUsage_Image)
		flags |= VK_ACCESS_SHADER_WRITE_BIT;
	if (usage & dsTextureUsage_CopyTo)
		flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	return flags;
}

VkAccessFlags dsVkDstImageAccessFlags(dsTextureUsage usage)
{
	VkAccessFlags flags = 0;
	if (usage & (dsTextureUsage_Image | dsTextureUsage_Texture))
		flags |= VK_ACCESS_SHADER_READ_BIT;
	if (usage & dsTextureUsage_SubpassInput)
		flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	if (usage & dsTextureUsage_CopyFrom)
		flags |= VK_ACCESS_TRANSFER_READ_BIT;
	return flags;
}

VkPipelineStageFlags dsVkSrcImageStageFlags(dsTextureUsage usage, bool offscreen, bool depthStencil)
{
	VkPipelineStageFlags flags = 0;
	if (offscreen)
	{
		if (depthStencil)
			flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		else
			flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (usage & dsTextureUsage_Image)
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (usage & dsTextureUsage_CopyTo)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkPipelineStageFlags dsVkDstTextureStageFlags(dsTextureUsage usage, bool depthStencilAttachment)
{
	VkAccessFlags flags = 0;
	if (depthStencilAttachment)
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	if (usage & (dsTextureUsage_Image | dsTextureUsage_Texture))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (usage & dsTextureUsage_CopyFrom)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}
