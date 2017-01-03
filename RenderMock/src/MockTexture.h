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

typedef struct dsMockTexture dsMockTexture;

dsTexture* dsMockTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, const void* data, size_t size);
dsOffscreen* dsMockTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint16_t samples,
	bool resolve);
bool dsMockTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsMockTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	const void* data, size_t size);
bool dsMockTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsMockTexture* srcTexture, dsMockTexture* dstTexture, const dsTextureCopyRegion* regions,
	size_t regionCount);
bool dsMockTexture_blit(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsMockTexture* srcTexture, dsMockTexture* dstTexture, const dsTextureBlitRegion* regions,
	size_t regionCount, dsFilter filter);
bool dsMockTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsMockTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height);
bool dsMockTexture_destroy(dsResourceManager* resourceManager, dsMockTexture* texture);
