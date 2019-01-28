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

dsTexture* dsVkTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size);
dsOffscreen* dsVkTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, bool resolve);
bool dsVkTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size);
bool dsVkTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount);
bool dsVkTexture_generateMipmaps(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture);
bool dsVkTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height);
void dsVkTexture_process(dsResourceManager* resourceManager, dsTexture* texture);
bool dsVkTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture);

bool dsVkTexture_supportsHostImage(dsVkDevice* device, const dsVkFormatInfo* formatInfo,
	VkImageType imageType, const dsTextureInfo* info);
bool dsVkTexture_isStatic(const dsTexture* texture);
VkImageLayout dsVkTexture_imageLayout(const dsTexture* texture);
void dsVkTexture_resetSubresourceLayouts(dsTexture* texture);
bool dsVkTexture_canReadBack(const dsTexture* texture);
bool dsVkTexture_addMemoryBarrier(dsTexture* texture, dsCommandBuffer* commandBuffer);
bool dsVkTexture_clearColor(dsOffscreen* offscreen, dsCommandBuffer* commandBuffer,
	const dsSurfaceColorValue* colorValue);
bool dsVkTexture_clearDepthStencil(dsOffscreen* offscreen, dsCommandBuffer* commandBuffer,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue);
void dsVkTexture_destroyImpl(dsTexture* texture);
