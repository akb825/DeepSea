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
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>
#include <math.h>

static uint32_t getSkipLevels(uint32_t dim, uint32_t targetDim)
{
	uint32_t curDiff = abs((int32_t)(dim - targetDim));
	uint32_t skip = 0;

	while (dim > 0)
	{
		dim /= 2;
		uint32_t diff = abs((int32_t)(dim - targetDim));
		if (diff > curDiff)
			break;

		curDiff = diff;
		++skip;
	}

	return skip;
}

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
	mipLevels = dsMax(mipLevels, 1U);
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
	dsAllocator* allocator, const dsTextureData* textureData, const dsTextureDataOptions* options,
	int usage, int memoryHints)
{
	if (!resourceManager || !textureData)
	{
		errno = EINVAL;
		return NULL;
	}

	dsGfxFormat format = textureData->format;
	uint32_t width = textureData->width;
	uint32_t height = textureData->height;
	uint32_t depth = textureData->depth;
	uint32_t mipLevels = textureData->mipLevels;
	const uint8_t* data = textureData->data;
	size_t dataSize = textureData->dataSize;
	if (options)
	{
		if (options->srgbFallback && (format & dsGfxFormat_DecoratorMask) == dsGfxFormat_SRGB &&
			!dsGfxFormat_textureSupported(resourceManager, format))
		{
			format = (dsGfxFormat)((format & ~dsGfxFormat_DecoratorMask) | dsGfxFormat_UNorm);
		}

		uint32_t skipLevels = 0;
		if (options->targetWidth)
			skipLevels = getSkipLevels(width, options->targetWidth);
		else if (options->targetHeight)
			skipLevels = getSkipLevels(height, options->targetHeight);
		else
			skipLevels = options->skipLevels;

		DS_ASSERT(textureData->mipLevels > 0);
		if (skipLevels >= textureData->mipLevels)
			skipLevels = textureData->mipLevels - 1;

		if (skipLevels)
		{
			size_t skipData = dsTexture_size(format, textureData->dimension, width, height, depth,
				skipLevels, 1);
			DS_ASSERT(skipData < dataSize);
			data += skipData;
			dataSize -= skipData;

			width = dsMax(width >> skipLevels, 1U);
			height = dsMax(height >> skipLevels, 1U);
			if (textureData->dimension == dsTextureDim_3D)
				depth = dsMax(depth >> skipLevels, 1U);
			mipLevels -= skipLevels;
		}
	}

	return dsTexture_create(resourceManager, allocator, usage, memoryHints, format,
		textureData->dimension, width, height, depth, mipLevels, data, dataSize);
}

void dsTextureData_destroy(dsTextureData* textureData)
{
	if (!textureData || !textureData->allocator)
		return;

	DS_VERIFY(dsAllocator_free(textureData->allocator, textureData));
}
