/*
 * Copyright 2019 Aaron Barany
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
#include "MTLTypes.h"

dsGfxBuffer* dsMTLGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size);
void* dsMTLGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size);
bool dsMTLGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer);
bool dsMTLGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsMTLGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsMTLGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size);
bool dsMTLGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size);
bool dsMTLGfxBuffer_copyToTexture(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer, dsTexture* dstTexture,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount);
void dsMTLGfxBuffer_process(dsResourceManager* resourceManager, dsGfxBuffer* buffer);
bool dsMTLGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

// NOTE: Must release the lifetime on the returned buffer data when done with it.
dsMTLGfxBufferData* dsMTLGfxBuffer_getData(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer);

id<MTLBuffer> dsMTLGfxBuffer_getBuffer(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer);
