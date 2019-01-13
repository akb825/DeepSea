/*
 * Copyright 2017 Aaron Barany
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

typedef struct dsMockTexture
{
	dsTexture texture;
	size_t dataSize;
	uint8_t data[];
} dsMockTexture;

dsTexture* dsMockTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsTextureUsage usage, dsGfxMemory memoryHints, const dsTextureInfo* info, const void* data,
	size_t size);
dsOffscreen* dsMockTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsTextureUsage usage, dsGfxMemory memoryHints,
	const dsTextureInfo* info, bool resolve);
bool dsMockTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size);
bool dsMockTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	uint32_t regionCount);
bool dsMockTexture_generateMipmaps(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture);
bool dsMockTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height);
bool dsMockTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture);
