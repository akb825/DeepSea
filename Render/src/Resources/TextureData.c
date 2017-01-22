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
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>

#define PVR_GENERIC_FORMAT(channel0, bits0, channel1, bits1, channel2, bits2, channel3, bits3) \
	(((uint64_t)(channel0) | ((uint64_t)(channel1) << 8) | ((uint64_t)(channel2) << 16) | \
		((uint64_t)(channel3) << 24) | \
	((uint64_t)(bits0) << 32) | ((uint64_t)(bits1) << 40) | ((uint64_t)(bits2) << 48) | \
		((uint64_t)(bits3) << 56)))

typedef enum PvrFormat
{
	PVRTC_2bppRGB,
	PVRTC_2bppRGBA,
	PVRTC_4bppRGB,
	PVRTC_4bppRGBA,
	PVRTC2_2bpp,
	PVRTC2_4bpp,
	ETC1,
	DXT1,
	DXT2,
	DXT3,
	DXT4,
	DXT5,
	BC1 = DXT1,
	BC2 = DXT3,
	BC3 = DXT5,
	BC4,
	BC5,
	BC6,
	BC7,
	UYVY,
	YUY2,
	BW1bpp,
	R9G9B9E5_UFloat,
	R8G8B8G8,
	G8R8G8B8,
	ETC2_RGB,
	ETC2_RGBA,
	ETC2_RGB_A1,
	EAC_R11,
	EAC_RG11,
	ASTC_4x4,
	ASTC_5x4,
	ASTC_5x5,
	ASTC_6x5,
	ASTC_6x6,
	ASTC_8x5,
	ASTC_8x6,
	ASTC_8x8,
	ASTC_10x5,
	ASTC_10x6,
	ASTC_10x8,
	ASTC_10x10,
	ASTC_12x10,
	ASTC_12x12,
	ASTC_3x3x3,
	ASTC_4x3x3,
	ASTC_4x4x3,
	ASTC_4x4x4,
	ASTC_5x4x4,
	ASTC_5x5x4,
	ASTC_5x5x5,
	ASTC_6x5x5,
	ASTC_6x6x5,
	ASTC_6x6x6,
	PvrFormatCount
} PvrFormat;

DS_STATIC_ASSERT(PvrFormatCount == 51, invalid_pvr_type_enum);

static dsGfxFormat formatMap[] =
{
	dsGfxFormat_PVRTC1_RGB_2BPP,  // PVRTC_2bppRGB
	dsGfxFormat_PVRTC1_RGBA_2BPP, // PVRTC_2bppRGBA,
	dsGfxFormat_PVRTC1_RGB_4BPP,  // PVRTC_4bppRGB,
	dsGfxFormat_PVRTC1_RGBA_4BPP, // PVRTC_4bppRGBA
	dsGfxFormat_PVRTC2_RGBA_2BPP, // PVRTC2_2bpp
	dsGfxFormat_PVRTC2_RGBA_4BPP, // PVRTC2_4bpp
	dsGfxFormat_ETC1,             // ETC1
	dsGfxFormat_BC1_RGBA,         // DXT1, BC1
	dsGfxFormat_BC2,              // DXT2
	dsGfxFormat_BC2,              // DXT3, BC2
	dsGfxFormat_BC3,              // DXT4
	dsGfxFormat_BC3,              // DXT5, BC3
	dsGfxFormat_BC4,              // BC4
	dsGfxFormat_BC5,              // BC5
	dsGfxFormat_BC6H,             // BC6
	dsGfxFormat_BC7,              // BC7
	dsGfxFormat_Unknown,          // UYVY
	dsGfxFormat_Unknown,          // YUY2
	dsGfxFormat_Unknown,          // BW1bpp,
	dsGfxFormat_E5B9G9R9_UFloat,  // R9G9B9E5_UFloat
	dsGfxFormat_Unknown,          // R8G8B8G8
	dsGfxFormat_Unknown,          // G8R8G8B8
	dsGfxFormat_ETC2_R8G8B8,      // ETC2_RGB
	dsGfxFormat_ETC2_R8G8B8,      // ETC2_RGBA
	dsGfxFormat_ETC2_R8G8B8A1,    // ETC2_RGB_A1
	dsGfxFormat_EAC_R11,          // EAC_R11
	dsGfxFormat_EAC_R11G11,       // EAC_RG11
	dsGfxFormat_ASTC_4x4,         // ASTC_4x4
	dsGfxFormat_ASTC_5x4,         // ASTC_5x4
	dsGfxFormat_ASTC_5x5,         // ASTC_5x5
	dsGfxFormat_ASTC_6x5,         // ASTC_6x5
	dsGfxFormat_ASTC_6x6,         // ASTC_6x6
	dsGfxFormat_ASTC_8x5,         // ASTC_8x5
	dsGfxFormat_ASTC_8x6,         // ASTC_8x6
	dsGfxFormat_ASTC_8x8,         // ASTC_8x8
	dsGfxFormat_ASTC_10x5,        // ASTC_10x5
	dsGfxFormat_ASTC_10x6,        // ASTC_10x6
	dsGfxFormat_ASTC_10x8,        // ASTC_10x8
	dsGfxFormat_ASTC_10x10,       // ASTC_10x10
	dsGfxFormat_ASTC_12x10,       // ASTC_12x10
	dsGfxFormat_ASTC_12x12,       // ASTC_12x12
	dsGfxFormat_Unknown,          // ASTC_3x3x3
	dsGfxFormat_Unknown,          // ASTC_4x3x3
	dsGfxFormat_Unknown,          // ASTC_4x4x3
	dsGfxFormat_Unknown,          // ASTC_4x4x4
	dsGfxFormat_Unknown,          // ASTC_5x4x4
	dsGfxFormat_Unknown,          // ASTC_5x5x4
	dsGfxFormat_Unknown,          // ASTC_5x5x5
	dsGfxFormat_Unknown,          // ASTC_6x5x5
	dsGfxFormat_Unknown,          // ASTC_6x6x5
	dsGfxFormat_Unknown,          // ASTC_6x6x6
};

DS_STATIC_ASSERT(sizeof(formatMap)/sizeof(*formatMap) == PvrFormatCount, format_map_mismatch);

typedef struct GenericFormat
{
	uint64_t pvrFormat;
	dsGfxFormat format;
} GenericFormat;

static GenericFormat genericFormats[] =
{
	{PVR_GENERIC_FORMAT('r', 4, 'g', 4, 0, 0, 0, 0), dsGfxFormat_R4G4},
	{PVR_GENERIC_FORMAT('r', 4, 'g', 4, 'b', 4, 'a', 4), dsGfxFormat_R4G4B4A4},
	{PVR_GENERIC_FORMAT('b', 4, 'g', 4, 'r', 4, 'a', 4), dsGfxFormat_B4G4R4A4},
	{PVR_GENERIC_FORMAT('r', 5, 'g', 6, 'b', 5, 0, 0), dsGfxFormat_R5G6B5},
	{PVR_GENERIC_FORMAT('b', 5, 'g', 6, 'r', 5, 0, 0), dsGfxFormat_B5G6R5},
	{PVR_GENERIC_FORMAT('r', 5, 'g', 5, 'b', 5, 'a', 1), dsGfxFormat_R5G5B5A1},
	{PVR_GENERIC_FORMAT('b', 5, 'g', 5, 'r', 5, 'a', 1), dsGfxFormat_B5G5R5A1},
	{PVR_GENERIC_FORMAT('a', 1, 'r', 5, 'g', 5, 'b', 5), dsGfxFormat_A1R5G5B5},
	{PVR_GENERIC_FORMAT('r', 8, 0, 0, 0, 0, 0, 0), dsGfxFormat_R8},
	{PVR_GENERIC_FORMAT('r', 8, 'g', 8, 0, 0, 0, 0), dsGfxFormat_R8G8},
	{PVR_GENERIC_FORMAT('r', 8, 'g', 8, 'b', 8, 0, 0), dsGfxFormat_R8G8B8},
	{PVR_GENERIC_FORMAT('b', 8, 'g', 8, 'r', 8, 0, 0), dsGfxFormat_B8G8R8},
	{PVR_GENERIC_FORMAT('r', 8, 'g', 8, 'b', 8, 'a', 8), dsGfxFormat_R8G8B8A8},
	{PVR_GENERIC_FORMAT('b', 8, 'g', 8, 'r', 8, 'a', 8), dsGfxFormat_B8G8R8A8},
	{PVR_GENERIC_FORMAT('a', 8, 'b', 8, 'g', 8, 'r', 8), dsGfxFormat_A8B8G8R8},
	{PVR_GENERIC_FORMAT('a', 2, 'r', 10, 'g', 10, 'b', 10), dsGfxFormat_A2R10G10B10},
	{PVR_GENERIC_FORMAT('a', 2, 'b', 10, 'g', 10, 'r', 10), dsGfxFormat_A2B10G10R10},
	{PVR_GENERIC_FORMAT('r', 16, 0, 0, 0, 0, 0, 0), dsGfxFormat_R16},
	{PVR_GENERIC_FORMAT('r', 16, 'g', 16, 0, 0, 0, 0), dsGfxFormat_R16G16},
	{PVR_GENERIC_FORMAT('r', 16, 'g', 16, 'b', 16, 0, 0), dsGfxFormat_R16G16B16},
	{PVR_GENERIC_FORMAT('r', 16, 'g', 16, 'b', 16, 'a', 16), dsGfxFormat_R16G16B16A16},
	{PVR_GENERIC_FORMAT('r', 32, 0, 0, 0, 0, 0, 0), dsGfxFormat_R32},
	{PVR_GENERIC_FORMAT('r', 32, 'g', 32, 0, 0, 0, 0), dsGfxFormat_R32G32},
	{PVR_GENERIC_FORMAT('r', 32, 'g', 32, 'b', 32, 0, 0), dsGfxFormat_R32G32B32},
	{PVR_GENERIC_FORMAT('r', 32, 'g', 32, 'b', 32, 'a', 32), dsGfxFormat_R32G32B32A32},
	{PVR_GENERIC_FORMAT('r', 64, 0, 0, 0, 0, 0, 0), dsGfxFormat_R64},
	{PVR_GENERIC_FORMAT('r', 64, 'g', 64, 0, 0, 0, 0), dsGfxFormat_R64G64},
	{PVR_GENERIC_FORMAT('r', 64, 'g', 64, 'b', 64, 0, 0), dsGfxFormat_R64G64B64},
	{PVR_GENERIC_FORMAT('r', 64, 'g', 64, 'b', 64, 'a', 64), dsGfxFormat_R64G64B64A64}
};

typedef enum PvrChannelType
{
	UByteN,
	SByteN,
	UByte,
	SByte,
	UShortN,
	SShortN,
	UShort,
	SShort,
	UIntN,
	SIntN,
	UInt,
	SInt,
	Float,
	PvrChannelTypeCount
} PvrChannelType;

DS_STATIC_ASSERT(PvrChannelTypeCount == 13, invalidpvr_channel_type_enum);

static void pvrError(const char* errorString, const char* filePath)
{
	if (filePath)
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%s when reading file %s", errorString, filePath);
	else
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, errorString);
}

static void pvrSizeError(const char* filePath)
{
	pvrError("Invalid PVR texture file size", filePath);
	errno = EIO;
}

static bool readUInt32(dsStream* stream, uint32_t* value, const char* filePath)
{
	if (dsStream_read(stream, value, sizeof(*value)) != sizeof(*value))
	{
		pvrSizeError(filePath);
		return false;
	}

	return true;
}

static bool readUInt64(dsStream* stream, uint64_t* value, const char* filePath)
{
	if (dsStream_read(stream, value, sizeof(*value)) != sizeof(*value))
	{
		pvrSizeError(filePath);
		return false;
	}

	return true;
}

static bool skipBytes(dsStream* stream, uint64_t size, const char* filePath)
{
	if (stream->seekFunc)
	{
		if (!dsStream_seek(stream, size, dsStreamSeekWay_Current))
		{
			pvrSizeError(filePath);
			return false;
		}
	}
	else
	{
		uint8_t buffer[1024];
		while (size > 0)
		{
			size_t readSize = (size_t)dsMin(size, sizeof(buffer));
			if (dsStream_read(stream, buffer, readSize) != readSize)
			{
				pvrSizeError(filePath);
				return false;
			}
			size -= readSize;
		}
	}

	return true;
}

static dsTextureData* loadPvr(dsAllocator* allocator, dsStream* stream, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t version;
	if (!readUInt32(stream, &version, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	const uint32_t expectedVersion = 0x03525650;
	if (version != expectedVersion)
	{
		pvrError("Invalid PVR file", filePath);
		errno = EFORMAT;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t flags;
	if (!readUInt32(stream, &flags, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	uint64_t pvrFormat;
	if (!readUInt64(stream, &pvrFormat, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	dsGfxFormat format = dsGfxFormat_Unknown;
	if (pvrFormat & 0xFFFFFFFF00000000ULL)
	{
		for (size_t i = 0; i < sizeof(genericFormats)/sizeof(*genericFormats); ++i)
		{
			if (genericFormats[i].pvrFormat == pvrFormat)
			{
				format = genericFormats[i].format;
				break;
			}
		}

		if (format == dsGfxFormat_Unknown)
		{
			pvrError("Unsupported PVR texture format", filePath);
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}
	else
	{
		if (pvrFormat >= (uint64_t)PvrFormatCount ||
			formatMap[(size_t)pvrFormat] == dsGfxFormat_Unknown)
		{
			pvrError("Unsupported PVR texture format", filePath);
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		format = formatMap[(size_t)pvrFormat];
	}

	uint32_t colorSpace;
	if (!readUInt32(stream, &colorSpace, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	if (colorSpace == 1)
		format = dsGfxFormat_decorate(format, dsGfxFormat_SRGB);
	else if (colorSpace != 0)
	{
		pvrError("Unknown color space for PVR texture", filePath);
		errno = EFORMAT;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t channelType;
	if (!readUInt32(stream, &channelType, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	if (dsGfxFormat_standardIndex(format) != 0)
	{
		switch (channelType)
		{
			case UByteN:
			case UShortN:
			case UIntN:
				format = dsGfxFormat_decorate(format, dsGfxFormat_UNorm);
				break;
			case SByteN:
			case SShortN:
			case SIntN:
				format = dsGfxFormat_decorate(format, dsGfxFormat_SNorm);
				break;
			case UByte:
			case UShort:
			case UInt:
				format = dsGfxFormat_decorate(format, dsGfxFormat_UInt);
				break;
			case SByte:
			case SShort:
			case SInt:
				format = dsGfxFormat_decorate(format, dsGfxFormat_SInt);
				break;
			case Float:
				format = dsGfxFormat_decorate(format, dsGfxFormat_Float);
				break;
			default:
				pvrError("Unknown channel type for PVR texture", filePath);
				errno = EFORMAT;
				DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	uint32_t width, height, depth, surfaces, faces, mipLevels;
	if (!readUInt32(stream, &width, filePath) || !readUInt32(stream, &height, filePath) ||
		!readUInt32(stream, &depth, filePath) || !readUInt32(stream, &surfaces, filePath) ||
		!readUInt32(stream, &faces, filePath) || !readUInt32(stream, &mipLevels, filePath))
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (width == 0 || height == 0 || depth == 0 || surfaces == 0 || faces == 0 || mipLevels == 0)
	{
		pvrError("Invalid dimensions for PVR texture", filePath);
		errno = EFORMAT;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (surfaces > 1 && depth > 1)
	{
		pvrError("Cannot have a 3D texture array for PVR texture", filePath);
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (faces != 1 && faces != 6)
	{
		pvrError("Must have a single face or full cube map for PVR texture", filePath);
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (depth > 1 && faces == 6)
	{
		pvrError("Cannot have a 3D cube map for PVR texture", filePath);
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureDim textureDim;
	if (faces == 6)
		textureDim = dsTextureDim_Cube;
	else if (depth > 1)
		textureDim = dsTextureDim_3D;
	else
	{
		depth = surfaces;
		if (depth == 1)
			depth = 0;

		if (height > 1)
			textureDim = dsTextureDim_2D;
		else
			textureDim = dsTextureDim_1D;
	}

	uint32_t metadataSize;
	if (!readUInt32(stream, &metadataSize, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);
	if (!skipBytes(stream, metadataSize, filePath))
		DS_PROFILE_FUNC_RETURN(NULL);

	dsTextureData* textureData = dsTextureData_create(allocator, format, textureDim, width, height,
		depth, mipLevels);
	if (!textureData)
		DS_PROFILE_FUNC_RETURN(NULL);

	if (dsStream_read(stream, textureData->data, textureData->dataSize) != textureData->dataSize)
	{
		pvrSizeError(filePath);
		DS_VERIFY(dsTextureData_destroy(textureData));
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	return textureData;
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
	size_t dataSize = dsTexture_size(format, dimension, width, height, depth, mipLevels, 1);
	if (dataSize == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	dsTextureData* textureData = dsAllocator_alloc(allocator, sizeof(dsTextureData) + dataSize);
	if (!textureData)
		return NULL;

	if (allocator->freeFunc)
		textureData->allocator = allocator;
	else
		textureData->allocator = NULL;
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

dsTextureData* dsTextureData_loadPvrFile(dsAllocator* allocator, const char* filePath)
{
	if (!allocator || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFileStream fileStream;
	if (!dsFileStream_openPath(&fileStream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open PVR file %s", filePath);
		return NULL;
	}

	dsTextureData* textureData = loadPvr(allocator, (dsStream*)&fileStream, filePath);
	if (textureData)
	{
		uint64_t pos = dsStream_tell((dsStream*)&fileStream);
		dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_End);
		if (pos != dsStream_tell((dsStream*)&fileStream))
		{
			pvrError("Unexpected file size", filePath);
			dsTextureData_destroy(textureData);
			textureData = NULL;
		}
	}
	DS_VERIFY(dsStream_close((dsStream*)&fileStream));
	return textureData;
}

dsTextureData* dsTextureData_loadPvrStream(dsAllocator* allocator, dsStream* stream)
{
	if (!allocator || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	return loadPvr(allocator, stream, NULL);
}

dsTexture* dsTextureData_loadPvrFileToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, const char* filePath, int usage,
	int memoryHints)
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

	dsTextureData* textureData = dsTextureData_loadPvrFile(tempAllocator, filePath);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		usage, memoryHints);
	DS_VERIFY(dsTextureData_destroy(textureData));
	return texture;
}

dsTexture* dsTextureData_loadPvrStreamToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsStream* stream, int usage,
	int memoryHints)
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

	dsTextureData* textureData = dsTextureData_loadPvrStream(tempAllocator, stream);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		usage, memoryHints);
	DS_VERIFY(dsTextureData_destroy(textureData));
	return texture;
}

bool dsTextureData_destroy(dsTextureData* textureData)
{
	if (!textureData)
	{
		errno = EINVAL;
		return false;
	}

	if (textureData->allocator)
		return dsAllocator_free(textureData->allocator, textureData);
	else
		return true;
}
