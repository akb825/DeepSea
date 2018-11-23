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

#pragma once

#include <DeepSea/Core/Config.h>

#include "VkTypes.h"
#include <DeepSea/Core/Debug.h>

#if DS_DEBUG
#define DS_VK_CALL(func) (dsSetLastVkCallsite(__FILE__, __FUNCTION__,__LINE__), (func))
#else
#define DS_VK_CALL(func) (func)
#endif

#define DS_INVALID_HEAP (uint32_t)-1

bool dsHandleVkResult(VkResult result);
void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line);
void dsGetLastVkCallsite(const char** file, const char** function,
	unsigned int* line);

uint32_t dsVkMemoryIndexImpl(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags optimalFlags);
uint32_t dsVkMemoryIndex(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags);
VkDeviceMemory dsAllocateVkMemory(const dsVkDevice* device,
	const VkMemoryRequirements* requirements, uint32_t memoryIndex);

VkSampleCountFlagBits dsVkSampleCount(uint32_t sampleCount);

VkAccessFlags dsVkSrcBufferAccessFlags(dsGfxBufferUsage usage, bool canMap);
VkAccessFlags dsVkDstBufferAccessFlags(dsGfxBufferUsage usage);

VkPipelineStageFlags dsVkSrcBufferStageFlags(dsGfxBufferUsage usage, bool canMap);
VkPipelineStageFlags dsVkDstBufferStageFlags(dsGfxBufferUsage usage);

VkAccessFlags dsVkSrcImageAccessFlags(dsTextureUsage usage, bool offscreen, bool depthStencil);
VkAccessFlags dsVkDstImageAccessFlags(dsTextureUsage usage);

VkPipelineStageFlags dsVkSrcImageStageFlags(dsTextureUsage usage, bool offscreen,
	bool depthStencil);
VkPipelineStageFlags dsVkDstImageStageFlags(dsTextureUsage usage, bool depthStencilAttachment);

VkImageAspectFlags dsVkImageAspectFlags(dsGfxFormat format);
VkDescriptorType dsVkDescriptorType(dsMaterialType type);
