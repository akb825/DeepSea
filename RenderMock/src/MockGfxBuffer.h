/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Render/Resources/Types.h>

typedef struct dsMockGfxBuffer dsMockGfxBuffer;

dsGfxBuffer* dsMockGfxBuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, size_t size, const void* data);
void* dsMockGfxBuffer_map(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer, int flags,
	size_t offset, size_t size);
bool dsMockGfxBuffer_unmap(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer);
bool dsMockGfxBuffer_flush(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsMockGfxBuffer_invalidate(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsMockGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsMockGfxBuffer* buffer, size_t offset, size_t size, const void* data);
bool dsMockGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsMockGfxBuffer* srcBuffer, size_t srcOffset, dsMockGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size);
bool dsMockGfxBuffer_destroy(dsResourceManager* resourceManager, dsMockGfxBuffer* buffer);
