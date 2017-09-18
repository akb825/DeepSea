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
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>
#include <math.h>

dsTextureData* dsTextureData_loadDds(bool* isDds, dsAllocator* allocator, dsStream* stream,
	const char* filePath);
dsTextureData* dsTextureData_loadKtx(bool* isKtx, dsAllocator* allocator, dsStream* stream,
	const char* filePath);
dsTextureData* dsTextureData_loadPvr(bool* isPvr, dsAllocator* allocator, dsStream* stream,
	const char* filePath);

// Order from most to fewest supported formats to attempt to process the most common formats first.
static dsTextureData* (*loadTextureFuncs[])(bool*, dsAllocator*, dsStream*, const char*) =
{
	&dsTextureData_loadPvr,
	&dsTextureData_loadKtx,
	&dsTextureData_loadDds
};

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

dsTextureData* dsTextureData_loadFile(dsAllocator* allocator, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFileStream fileStream;
	if (!dsFileStream_openPath(&fileStream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open PVR file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	bool isFormat = true;
	dsTextureData* textureData = NULL;
	for (size_t i = 0; i < DS_ARRAY_SIZE(loadTextureFuncs); ++i)
	{
		textureData = loadTextureFuncs[i](&isFormat, allocator, (dsStream*)&fileStream, filePath);
		if (textureData || isFormat)
			break;

		if (i < DS_ARRAY_SIZE(loadTextureFuncs) - 1)
			dsFileStream_seek(&fileStream, 0, dsStreamSeekWay_Beginning);
	}

	// If check is false, we couldn't find the format.
	if (!isFormat)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Unknown texture file format when reading file '%s'.",
			filePath);
		errno = EFORMAT;
	}

	DS_VERIFY(dsFileStream_close(&fileStream));
	DS_PROFILE_FUNC_RETURN(textureData);
}

dsTextureData* dsTextureData_loadStream(dsAllocator* allocator, dsStream* stream)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!stream->seekFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Stream must be seekable to determine the texture file format.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint64_t streamPos = dsStream_tell(stream);
	DS_ASSERT(streamPos != DS_STREAM_INVALID_POS);

	bool isFormat = true;
	dsTextureData* textureData = NULL;
	for (size_t i = 0; i < DS_ARRAY_SIZE(loadTextureFuncs); ++i)
	{
		textureData = loadTextureFuncs[i](&isFormat, allocator, stream, NULL);
		if (textureData || isFormat)
			break;

		if (i < DS_ARRAY_SIZE(loadTextureFuncs) - 1)
			dsStream_seek(stream, streamPos, dsStreamSeekWay_Beginning);
	}

	// If check is false, we couldn't find the format.
	if (!isFormat)
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Unknown texture file format.");
		errno = EFORMAT;
	}

	DS_PROFILE_FUNC_RETURN(textureData);
}

dsTexture* dsTextureData_loadFileToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, const char* filePath,
	const dsTextureDataOptions* options, int usage, int memoryHints)
{
	if (!resourceManager || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!tempAllocator)
	{
		if (textureAllocator)
			tempAllocator = textureAllocator;
		else
			tempAllocator = resourceManager->allocator;
	}

	dsTextureData* textureData = dsTextureData_loadFile(tempAllocator, filePath);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		options, usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}

dsTexture* dsTextureData_loadStreamToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsStream* stream,
	const dsTextureDataOptions* options, int usage, int memoryHints)
{
	if (!resourceManager || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!tempAllocator)
	{
		if (textureAllocator)
			tempAllocator = textureAllocator;
		else
			tempAllocator = resourceManager->allocator;
	}

	dsTextureData* textureData = dsTextureData_loadStream(tempAllocator, stream);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		options, usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}

void dsTextureData_destroy(dsTextureData* textureData)
{
	if (!textureData || !textureData->allocator)
		return;

	DS_VERIFY(dsAllocator_free(textureData->allocator, textureData));
}
