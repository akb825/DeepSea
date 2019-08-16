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

#include "VkShared.h"
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Assert.h>
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

const char* dsGetVkResultString(VkResult result)
{
	switch (result)
	{
		case VK_SUCCESS:
			return "success";
		case VK_NOT_READY:
			return "not ready";
		case VK_TIMEOUT:
			return "timeout";
		case VK_EVENT_SET:
			return "event set";
		case VK_EVENT_RESET:
			return "event reset";
		case VK_INCOMPLETE:
			return "incomplete";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "out of host memory";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "out of device memory";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "initialization failed";
		case VK_ERROR_DEVICE_LOST:
			return "device lost";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "memory map failed";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "layer not present";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "extension not present";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "feature not present";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "incompatible driver";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "too many objects";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "format not supported";
		case VK_ERROR_FRAGMENTED_POOL:
			return "fragmented pool";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "out of pool memory";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "invalid external handle";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "surface lost";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "native window in use";
		case VK_SUBOPTIMAL_KHR:
			return "suboptimal";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "out of date";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "incompatible display";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "validation failed";
		case VK_ERROR_INVALID_SHADER_NV:
			return "invalid shader";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "invalid drm format modifier plane layout";
		case VK_ERROR_FRAGMENTATION_EXT:
			return "fragmentation";
		case VK_ERROR_NOT_PERMITTED_EXT:
			return "not permitted";
		case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
			return "invalid device address";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "full screen exclusive mode lost";
		default:
			return "unknown";
	}
}

bool dsHandleVkResult(VkResult result, const char* failMessage, const char* file, unsigned int line,
	const char* function)
{
	if (failMessage && result != VK_SUCCESS)
	{
		dsLog_messagef(dsLogLevel_Error, DS_RENDER_VULKAN_LOG_TAG, file, line, function, "%s: %s",
			failMessage, dsGetVkResultString(result));
	}

	switch (result)
	{
		case VK_SUCCESS:
			return true;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_TOO_MANY_OBJECTS:
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

uint32_t dsVkMemoryIndexImpl(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags optimalFlags)
{
	uint32_t memoryIndex = DS_INVALID_HEAP;
	bool isOptimal = false;
	VkDeviceSize memorySize = 0;

	const VkPhysicalDeviceMemoryProperties* memoryProperties = &device->memoryProperties;
	for (uint32_t curBitmask = requirements->memoryTypeBits; curBitmask;
		curBitmask = dsRemoveLastBit(curBitmask))
	{
		uint32_t i = dsBitmaskIndex(curBitmask);
		const VkMemoryType* memoryType = memoryProperties->memoryTypes + i;
		if ((memoryType->propertyFlags & requiredFlags) != requiredFlags)
			continue;

		VkDeviceSize size = memoryProperties->memoryHeaps[memoryType->heapIndex].size;
		if (memoryIndex == DS_INVALID_HEAP)
			memoryIndex = i;

		// Find the largest optimal heap.
		if (size > memorySize)
		{
			if ((memoryType->propertyFlags & optimalFlags) == optimalFlags)
			{
				isOptimal = true;
				memoryIndex = i;
			}
			else if (!isOptimal)
				memoryIndex = i;
		}

		if (memoryIndex == i)
			memorySize = size;
	}

	if (memoryIndex == DS_INVALID_HEAP)
	{
		errno = ENOMEM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "No suitable GPU heap found.");
	}

	return memoryIndex;
}

uint32_t dsVkMemoryIndex(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags)
{
	VkMemoryPropertyFlags requiredFlags = 0;
	VkMemoryPropertyFlags optimalFlags = 0;
	if (memoryFlags & dsGfxMemory_GPUOnly)
		optimalFlags |= VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
	else
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	if (memoryFlags & dsGfxMemory_Coherent)
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (memoryFlags & (dsGfxMemory_Dynamic | dsGfxMemory_Stream))
		optimalFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	return dsVkMemoryIndexImpl(device, requirements, requiredFlags, optimalFlags);
}

bool dsVkMemoryIndexCompatible(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags, uint32_t memoryIndex)
{
	if (memoryIndex == DS_INVALID_HEAP || !(requirements->memoryTypeBits & (1 << memoryIndex)))
		return false;

	VkMemoryPropertyFlags requiredFlags = 0;
	if (!(memoryFlags & dsGfxMemory_GPUOnly))
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	if (memoryFlags & dsGfxMemory_Coherent)
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	const VkPhysicalDeviceMemoryProperties* memoryProperties = &device->memoryProperties;
	const VkMemoryType* memoryType = memoryProperties->memoryTypes + memoryIndex;
	return (memoryType->propertyFlags & requiredFlags) == requiredFlags;
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
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		requirements->size,
		memoryIndex
	};
	const dsVkInstance* instance = &device->instance;
	VkDeviceMemory memory = 0;
	VkResult result = DS_VK_CALL(device->vkAllocateMemory)(device->device, &allocInfo,
		instance->allocCallbacksPtr, &memory);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't allocate memory"))
		return 0;

	return memory;
}

bool dsVkHeapIsCoherent(const dsVkDevice* device, uint32_t memoryIndex)
{
	const VkPhysicalDeviceMemoryProperties* memoryProperties = &device->memoryProperties;
	const VkMemoryType* memoryType = memoryProperties->memoryTypes + memoryIndex;
	return (memoryProperties->memoryHeaps[memoryType->heapIndex].flags &
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
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

VkAccessFlags dsVkReadBufferAccessFlags(dsGfxBufferUsage usage)
{
	VkAccessFlags flags = 0;
	if (usage & dsGfxBufferUsage_Index)
		flags |= VK_ACCESS_INDEX_READ_BIT;
	if (usage & dsGfxBufferUsage_Vertex)
		flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (usage & (dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch))
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Texture | dsGfxBufferUsage_Image))
	{
		flags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyFrom)
		flags |= VK_ACCESS_TRANSFER_READ_BIT;
	return flags;
}

VkAccessFlags dsVkWriteBufferAccessFlags(dsGfxBufferUsage usage, bool canMapMainBuffer)
{
	VkAccessFlags flags = 0;
	if (canMapMainBuffer)
		flags |= VK_ACCESS_HOST_WRITE_BIT;
	if (usage & (dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Image))
		flags |= VK_ACCESS_SHADER_WRITE_BIT;
	if (usage & dsGfxBufferUsage_CopyTo)
		flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	return flags;
}

bool dsVkImageUsageSupportsTransient(VkImageUsageFlags usage)
{
	return (usage & ~(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0;
}

VkPipelineStageFlags dsVkReadBufferStageFlags(const dsRenderer* renderer, dsGfxBufferUsage usage)
{
	VkAccessFlags flags = 0;
	if (usage & (dsGfxBufferUsage_Index | dsGfxBufferUsage_Vertex))
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	if (usage & dsGfxBufferUsage_IndirectDraw)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	if (usage & dsGfxBufferUsage_IndirectDispatch)
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_Texture | dsGfxBufferUsage_Image))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (renderer->hasTessellationShaders)
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
				VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
		if (renderer->hasGeometryShaders)
			flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyFrom)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkPipelineStageFlags dsVkWriteBufferStageFlags(const dsRenderer* renderer, dsGfxBufferUsage usage,
	bool canMapMainBuffer)
{
	VkPipelineStageFlags flags = 0;
	if (canMapMainBuffer)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;
	if (usage & (dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Image))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (renderer->hasTessellationShaders)
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
				VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
		if (renderer->hasGeometryShaders)
			flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (usage & dsGfxBufferUsage_CopyTo)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkAccessFlags dsVkReadImageAccessFlags(dsTextureUsage usage)
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

VkAccessFlags dsVkWriteImageAccessFlags(dsTextureUsage usage, bool offscreen, bool depthStencil)
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

VkPipelineStageFlags dsVkReadImageStageFlags(const dsRenderer* renderer, dsTextureUsage usage, bool depthStencilAttachment)
{
	VkAccessFlags flags = 0;
	if (depthStencilAttachment)
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	if (usage & (dsTextureUsage_Image | dsTextureUsage_Texture))
	{
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (renderer->hasTessellationShaders)
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
				VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
		if (renderer->hasGeometryShaders)
			flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (usage & dsTextureUsage_SubpassInput)
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if (usage & dsTextureUsage_CopyFrom)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkPipelineStageFlags dsVkWriteImageStageFlags(const dsRenderer* renderer, dsTextureUsage usage,
	bool offscreen, bool depthStencil)
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
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (renderer->hasTessellationShaders)
		{
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
				VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		}
		if (renderer->hasGeometryShaders)
			flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (usage & dsTextureUsage_CopyTo)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	return flags;
}

VkImageAspectFlags dsVkImageAspectFlags(dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
		case dsGfxFormat_D32_Float:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case dsGfxFormat_S8:
			return VK_IMAGE_ASPECT_STENCIL_BIT;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

VkImageAspectFlags dsVkClearDepthStencilImageAspectFlags(dsGfxFormat format,
	dsClearDepthStencil surfaceParts)
{
	VkImageAspectFlags aspectFlags = 0;
	if (surfaceParts & dsClearDepthStencil_Depth)
		aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (surfaceParts & dsClearDepthStencil_Stencil)
		aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	return aspectFlags & dsVkImageAspectFlags(format);
}

VkDescriptorType dsVkDescriptorType(dsMaterialType type, dsMaterialBinding binding)
{
	switch (type)
	{
		case dsMaterialType_Texture:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case dsMaterialType_Image:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case dsMaterialType_SubpassInput:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case dsMaterialType_TextureBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case dsMaterialType_ImageBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case dsMaterialType_VariableGroup:
		case dsMaterialType_UniformBlock:
			if (binding == dsMaterialBinding_Instance)
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case dsMaterialType_UniformBuffer:
			if (binding == dsMaterialBinding_Instance)
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		default:
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

VkCompareOp dsVkCompareOp(mslCompareOp compareOp, VkCompareOp defaultOp)
{
	switch (compareOp)
	{
		case mslCompareOp_Less:
			return VK_COMPARE_OP_LESS;
		case mslCompareOp_Equal:
			return VK_COMPARE_OP_EQUAL;
		case mslCompareOp_LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case mslCompareOp_Greater:
			return VK_COMPARE_OP_GREATER;
		case mslCompareOp_NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case mslCompareOp_GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case mslCompareOp_Always:
			return VK_COMPARE_OP_ALWAYS;
		case mslCompareOp_Never:
		default:
			return defaultOp;
	}
}

VkShaderStageFlagBits dsVkShaderStage(mslStage stage)
{
	switch (stage)
	{
		case mslStage_Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case mslStage_TessellationControl:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case mslStage_TessellationEvaluation:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case mslStage_Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case mslStage_Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case mslStage_Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			DS_ASSERT(false);
			return 0;
	}
}

VkPrimitiveTopology dsVkPrimitiveType(dsPrimitiveType type)
{
	switch (type)
	{
		case dsPrimitiveType_PointList:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case dsPrimitiveType_LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case dsPrimitiveType_LineStrip:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case dsPrimitiveType_TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case dsPrimitiveType_TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case dsPrimitiveType_TriangleFan:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case dsPrimitiveType_LineListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case dsPrimitiveType_TriangleListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case dsPrimitiveType_TriangleStripAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case dsPrimitiveType_PatchList:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default:
			DS_ASSERT(false);
			return 0;
	}
}
