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

#pragma once

#include <DeepSea/Core/Config.h>

#include "VkTypes.h"
#include <DeepSea/Core/Debug.h>

#if DS_DEBUG
#define DS_VK_CALL(func) (dsSetLastVkCallsite(__FILE__, __FUNCTION__, __LINE__), (func))
#else
#define DS_VK_CALL(func) (func)
#endif

#define DS_INVALID_HEAP (uint32_t)-1

#define DS_HANDLE_VK_RESULT(result, failMessage) \
	dsHandleVkResult(result, failMessage, __FILE__, __LINE__, __FUNCTION__)

const char* dsGetVkResultString(VkResult result);
bool dsHandleVkResult(VkResult result, const char* failMessage, const char* file, unsigned int line,
	const char* function);
void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line);
void dsGetLastVkCallsite(const char** file, const char** function,
	unsigned int* line);

uint32_t dsVkMemoryIndexImpl(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags optimalFlags);
uint32_t dsVkMemoryIndex(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags);
bool dsVkMemoryIndexCompatible(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags, uint32_t memoryIndex);
VkDeviceMemory dsAllocateVkMemory(const dsVkDevice* device,
	const VkMemoryRequirements* requirements, uint32_t memoryIndex);
bool dsVkHeapIsCoherent(const dsVkDevice* device, uint32_t memoryIndex);

VkSampleCountFlagBits dsVkSampleCount(uint32_t sampleCount);

VkAccessFlags dsVkReadBufferAccessFlags(dsGfxBufferUsage usage);
VkAccessFlags dsVkWriteBufferAccessFlags(dsGfxBufferUsage usage, bool canMapMainBuffer);

bool dsVkImageUsageSupportsTransient(VkImageUsageFlags usage);

VkPipelineStageFlags dsVkPipelineStageFlags(const dsRenderer* renderer, dsGfxPipelineStage stages,
	bool isSrc);
VkAccessFlags dsVkAccessFlags(dsGfxAccess access);

VkPipelineStageFlags dsVkReadBufferStageFlags(const dsRenderer* renderer, dsGfxBufferUsage usage);
VkPipelineStageFlags dsVkWriteBufferStageFlags(const dsRenderer* renderer, dsGfxBufferUsage usage,
	bool canMapMainBuffer);

VkAccessFlags dsVkReadImageAccessFlags(dsTextureUsage usage);
VkAccessFlags dsVkWriteImageAccessFlags(dsTextureUsage usage, bool offscreen, bool depthStencil);

VkPipelineStageFlags dsVkReadImageStageFlags(const dsRenderer* renderer, dsTextureUsage usage,
	bool depthStencilAttachment);
VkPipelineStageFlags dsVkWriteImageStageFlags(const dsRenderer* renderer, dsTextureUsage usage,
	bool offscreen, bool depthStencil);

VkImageAspectFlags dsVkImageAspectFlags(dsGfxFormat format);
VkImageAspectFlags dsVkClearDepthStencilImageAspectFlags(dsGfxFormat format,
	dsClearDepthStencil surfaceParts);
VkDescriptorType dsVkDescriptorType(dsMaterialType type, dsMaterialBinding binding);

VkCompareOp dsVkCompareOp(mslCompareOp compareOp, VkCompareOp defaultOp);
VkShaderStageFlagBits dsVkShaderStage(mslStage stage);
VkPrimitiveTopology dsVkPrimitiveType(dsPrimitiveType type);

void dsConvertVkViewport(VkViewport* outViewport, const dsAlignedBox3f* viewport, uint32_t width,
	uint32_t height);
