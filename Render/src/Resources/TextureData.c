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

#include <DeepSea/Render/Resources/TextureData.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>

dsTextureData* dsTextureData_create(dsAllocator* allocator, dsGfxFormat format,
	dsTextureDim dimension, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t maxLevels = dsTexture_maxMipmapLevels(width, height, DS_MIP_DEPTH(dimension, depth));
	mipLevels = dsMin(maxLevels, mipLevels);
	size_t dataSize = dsTexture_size(format, dimension, width, height, depth, mipLevels, 1);
	if (dataSize == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	dsTextureData* textureData = (dsTextureData*)dsAllocator_alloc(allocator,
		sizeof(dsTextureData) + dataSize);
	if (!textureData)
		return NULL;

	textureData->allocator = dsAllocator_keepPointer(allocator);
	textureData->format = format;
	textureData->dimension = dimension;
	textureData->width = width;
	textureData->height = height;
	textureData->depth = depth;
	textureData->mipLevels = mipLevels;
	textureData->dataSize = dataSize;

	return textureData;
}

dsTexture* dsTextureData_createTexture(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsTextureData* textureData, int usage, int memoryHints)
{
	if (!resourceManager || !textureData)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsTexture_create(resourceManager, allocator, usage, memoryHints, textureData->format,
		textureData->dimension, textureData->width, textureData->height, textureData->depth,
		textureData->mipLevels, textureData->data, textureData->dataSize);
}

void dsTextureData_destroy(dsTextureData* textureData)
{
	if (!textureData || !textureData->allocator)
		return;

	DS_VERIFY(dsAllocator_free(textureData->allocator, textureData));
}
