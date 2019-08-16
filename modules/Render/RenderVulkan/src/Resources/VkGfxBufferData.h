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

dsVkGfxBufferData* dsVkGfxBufferData_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* scratchAllocator, dsGfxBufferUsage usage,
	dsGfxMemory memoryHints, const void* data, size_t size);
inline VkBuffer dsVkGfxBufferData_getBuffer(const dsVkGfxBufferData* buffer);
VkBufferView dsVkGfxBufferData_getBufferView(dsVkGfxBufferData* buffer, dsGfxFormat format,
	size_t offset, size_t count);
bool dsVkGfxBufferData_canMapMainBuffer(const dsVkGfxBufferData* buffer);
bool dsVkGfxBufferData_isStatic(const dsVkGfxBufferData* buffer);
bool dsVkGfxBufferData_needsMemoryBarrier(const dsVkGfxBufferData* buffer, bool canMapMainBuffer);
bool dsVkGfxBufferData_addMemoryBarrier(dsVkGfxBufferData* buffer, VkDeviceSize offset,
	VkDeviceSize size, dsCommandBuffer* commandBuffer);
void dsVkGfxBufferData_destroy(dsVkGfxBufferData* buffer);

inline VkBuffer dsVkGfxBufferData_getBuffer(const dsVkGfxBufferData* buffer)
{
	return buffer->deviceBuffer ? buffer->deviceBuffer : buffer->hostBuffer;
}
