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

#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>

#define DDS_MAGIC_NUMBER 0x20534444
#define DDS_FOURCC(a, b, c, d) \
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24 ))

typedef enum DdsFlags
{
	DdsFlags_Caps = 0x1,
	DdsFlags_Height = 0x2,
	DdsFlags_Width = 0x4,
	DdsFlags_Pitch = 0x8,
	DdsFlags_PixelFormat = 0x1000,
	DdsFlags_MipmapCount = 0x20000,
	DdsFlags_LinearSize = 0x80000,
	DdsFlags_Depth = 0x800000,
	DdsFlags_Requred = DdsFlags_Caps | DdsFlags_Height | DdsFlags_Width | DdsFlags_PixelFormat
} DdsFlags;

typedef enum DdsFormatFlags
{
	DdsFormatFlags_AlphaPixels = 0x1,
	DdsFormatFlags_Alpha = 0x2,
	DdsFormatFlags_FourCC = 0x4,
	DdsFormatFlags_Rgb = 0x40,
	DdsFormatFlags_Yuv = 0x200,
	DdsFormatFlags_Luminance = 0x20000
} DdsFormatFlags;

typedef enum DdsCaps2Flags
{
	DdsCaps2Flags_Cube = 0x200,
	DdsCaps2Flags_Volume = 0x200000
} DdsCaps2Flags;

typedef enum DdsDxt10Format
{
	DdsDxt10Format_UNKNOWN                     = 0,
	DdsDxt10Format_R32G32B32A32_TYPELESS       = 1,
	DdsDxt10Format_R32G32B32A32_FLOAT          = 2,
	DdsDxt10Format_R32G32B32A32_UINT           = 3,
	DdsDxt10Format_R32G32B32A32_SINT           = 4,
	DdsDxt10Format_R32G32B32_TYPELESS          = 5,
	DdsDxt10Format_R32G32B32_FLOAT             = 6,
	DdsDxt10Format_R32G32B32_UINT              = 7,
	DdsDxt10Format_R32G32B32_SINT              = 8,
	DdsDxt10Format_R16G16B16A16_TYPELESS       = 9,
	DdsDxt10Format_R16G16B16A16_FLOAT          = 10,
	DdsDxt10Format_R16G16B16A16_UNORM          = 11,
	DdsDxt10Format_R16G16B16A16_UINT           = 12,
	DdsDxt10Format_R16G16B16A16_SNORM          = 13,
	DdsDxt10Format_R16G16B16A16_SINT           = 14,
	DdsDxt10Format_R32G32_TYPELESS             = 15,
	DdsDxt10Format_R32G32_FLOAT                = 16,
	DdsDxt10Format_R32G32_UINT                 = 17,
	DdsDxt10Format_R32G32_SINT                 = 18,
	DdsDxt10Format_R32G8X24_TYPELESS           = 19,
	DdsDxt10Format_D32_FLOAT_S8X24_UINT        = 20,
	DdsDxt10Format_R32_FLOAT_X8X24_TYPELESS    = 21,
	DdsDxt10Format_X32_TYPELESS_G8X24_UINT     = 22,
	DdsDxt10Format_R10G10B10A2_TYPELESS        = 23,
	DdsDxt10Format_R10G10B10A2_UNORM           = 24,
	DdsDxt10Format_R10G10B10A2_UINT            = 25,
	DdsDxt10Format_R11G11B10_FLOAT             = 26,
	DdsDxt10Format_R8G8B8A8_TYPELESS           = 27,
	DdsDxt10Format_R8G8B8A8_UNORM              = 28,
	DdsDxt10Format_R8G8B8A8_UNORM_SRGB         = 29,
	DdsDxt10Format_R8G8B8A8_UINT               = 30,
	DdsDxt10Format_R8G8B8A8_SNORM              = 31,
	DdsDxt10Format_R8G8B8A8_SINT               = 32,
	DdsDxt10Format_R16G16_TYPELESS             = 33,
	DdsDxt10Format_R16G16_FLOAT                = 34,
	DdsDxt10Format_R16G16_UNORM                = 35,
	DdsDxt10Format_R16G16_UINT                 = 36,
	DdsDxt10Format_R16G16_SNORM                = 37,
	DdsDxt10Format_R16G16_SINT                 = 38,
	DdsDxt10Format_R32_TYPELESS                = 39,
	DdsDxt10Format_D32_FLOAT                   = 40,
	DdsDxt10Format_R32_FLOAT                   = 41,
	DdsDxt10Format_R32_UINT                    = 42,
	DdsDxt10Format_R32_SINT                    = 43,
	DdsDxt10Format_R24G8_TYPELESS              = 44,
	DdsDxt10Format_D24_UNORM_S8_UINT           = 45,
	DdsDxt10Format_R24_UNORM_X8_TYPELESS       = 46,
	DdsDxt10Format_X24_TYPELESS_G8_UINT        = 47,
	DdsDxt10Format_R8G8_TYPELESS               = 48,
	DdsDxt10Format_R8G8_UNORM                  = 49,
	DdsDxt10Format_R8G8_UINT                   = 50,
	DdsDxt10Format_R8G8_SNORM                  = 51,
	DdsDxt10Format_R8G8_SINT                   = 52,
	DdsDxt10Format_R16_TYPELESS                = 53,
	DdsDxt10Format_R16_FLOAT                   = 54,
	DdsDxt10Format_D16_UNORM                   = 55,
	DdsDxt10Format_R16_UNORM                   = 56,
	DdsDxt10Format_R16_UINT                    = 57,
	DdsDxt10Format_R16_SNORM                   = 58,
	DdsDxt10Format_R16_SINT                    = 59,
	DdsDxt10Format_R8_TYPELESS                 = 60,
	DdsDxt10Format_R8_UNORM                    = 61,
	DdsDxt10Format_R8_UINT                     = 62,
	DdsDxt10Format_R8_SNORM                    = 63,
	DdsDxt10Format_R8_SINT                     = 64,
	DdsDxt10Format_A8_UNORM                    = 65,
	DdsDxt10Format_R1_UNORM                    = 66,
	DdsDxt10Format_R9G9B9E5_SHAREDEXP          = 67,
	DdsDxt10Format_R8G8_B8G8_UNORM             = 68,
	DdsDxt10Format_G8R8_G8B8_UNORM             = 69,
	DdsDxt10Format_BC1_TYPELESS                = 70,
	DdsDxt10Format_BC1_UNORM                   = 71,
	DdsDxt10Format_BC1_UNORM_SRGB              = 72,
	DdsDxt10Format_BC2_TYPELESS                = 73,
	DdsDxt10Format_BC2_UNORM                   = 74,
	DdsDxt10Format_BC2_UNORM_SRGB              = 75,
	DdsDxt10Format_BC3_TYPELESS                = 76,
	DdsDxt10Format_BC3_UNORM                   = 77,
	DdsDxt10Format_BC3_UNORM_SRGB              = 78,
	DdsDxt10Format_BC4_TYPELESS                = 79,
	DdsDxt10Format_BC4_UNORM                   = 80,
	DdsDxt10Format_BC4_SNORM                   = 81,
	DdsDxt10Format_BC5_TYPELESS                = 82,
	DdsDxt10Format_BC5_UNORM                   = 83,
	DdsDxt10Format_BC5_SNORM                   = 84,
	DdsDxt10Format_B5G6R5_UNORM                = 85,
	DdsDxt10Format_B5G5R5A1_UNORM              = 86,
	DdsDxt10Format_B8G8R8A8_UNORM              = 87,
	DdsDxt10Format_B8G8R8X8_UNORM              = 88,
	DdsDxt10Format_R10G10B10_XR_BIAS_A2_UNORM  = 89,
	DdsDxt10Format_B8G8R8A8_TYPELESS           = 90,
	DdsDxt10Format_B8G8R8A8_UNORM_SRGB         = 91,
	DdsDxt10Format_B8G8R8X8_TYPELESS           = 92,
	DdsDxt10Format_B8G8R8X8_UNORM_SRGB         = 93,
	DdsDxt10Format_BC6H_TYPELESS               = 94,
	DdsDxt10Format_BC6H_UF16                   = 95,
	DdsDxt10Format_BC6H_SF16                   = 96,
	DdsDxt10Format_BC7_TYPELESS                = 97,
	DdsDxt10Format_BC7_UNORM                   = 98,
	DdsDxt10Format_BC7_UNORM_SRGB              = 99,
	DdsDxt10Format_AYUV                        = 100,
	DdsDxt10Format_Y410                        = 101,
	DdsDxt10Format_Y416                        = 102,
	DdsDxt10Format_NV12                        = 103,
	DdsDxt10Format_P010                        = 104,
	DdsDxt10Format_P016                        = 105,
	DdsDxt10Format_420_OPAQUE                  = 106,
	DdsDxt10Format_YUY2                        = 107,
	DdsDxt10Format_Y210                        = 108,
	DdsDxt10Format_Y216                        = 109,
	DdsDxt10Format_NV11                        = 110,
	DdsDxt10Format_AI44                        = 111,
	DdsDxt10Format_IA44                        = 112,
	DdsDxt10Format_P8                          = 113,
	DdsDxt10Format_A8P8                        = 114,
	DdsDxt10Format_B4G4R4A4_UNORM              = 115,
	DdsDxt10Format_P208                        = 130,
	DdsDxt10Format_V208                        = 131,
	DdsDxt10Format_V408                        = 132,
} DdsDxt10Format;

typedef enum DdsTextureDim
{
  DdsTextureDim_UNKNOWN    = 0,
  DdsTextureDim_BUFFER     = 1,
  DdsTextureDim_TEXTURE1D  = 2,
  DdsTextureDim_TEXTURE2D  = 3,
  DdsTextureDim_TEXTURE3D  = 4
} DdsTextureDim;

typedef enum DdsDxt10MiscFlag
{
	DdsDxt10MiscFlag_CubeMap = 0x4
} DdsDxt10MiscFlag;

typedef enum DdsDxt10MiscFlags2
{
	DdsDxt10MiscFlags2_AlphaModeUnknown       = 0,
	DdsDxt10MiscFlags2_AlphaModeStraight      = 1,
	DdsDxt10MiscFlags2_AlphaModePreMultiplied = 2,
	DdsDxt10MiscFlags2_AlphaModeOpaque        = 3,
	DdsDxt10MiscFlags2_AlphaModeCustom        = 4,
} DdsDxt10MiscFlags2;

typedef struct DdsPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t rgbBitCount;
	uint32_t rBitMask;
	uint32_t gBitMask;
	uint32_t bBitMask;
	uint32_t aBitMask;
} DdsPixelFormat;

typedef struct DdsHeader
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	DdsPixelFormat ddspf;
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
} DdsHeader;

typedef struct DdsHeaderDxt10
{
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
} DdsHeaderDxt10;

static void ddsError(const char* errorString, const char* filePath)
{
	if (filePath)
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%s when reading file '%s'.", errorString, filePath);
	else
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%s.", errorString);
}

static void ddsFormatError(const char* filePath)
{
	ddsError("Invalid DDS texture file format", filePath);
	errno = EFORMAT;
}

static void ddsSizeError(const char* filePath)
{
	ddsError("Invalid DDS texture file size", filePath);
	errno = EFORMAT;
}

static dsGfxFormat getDdsFormat(const DdsPixelFormat* format)
{
	if (format->flags & DdsFormatFlags_FourCC)
	{
		switch (format->fourCC)
		{
			case DDS_FOURCC('D', 'X', 'T', '1'):
			{
				if (format->flags & DdsFormatFlags_AlphaPixels)
					return dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA, dsGfxFormat_UNorm);
				else
					return dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm);
			}
			case DDS_FOURCC('D', 'X', 'T', '2'):
			case DDS_FOURCC('D', 'X', 'T', '3'):
				return dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_UNorm);
			case DDS_FOURCC('D', 'X', 'T', '4'):
			case DDS_FOURCC('D', 'X', 'T', '5'):
				return dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm);
			case DDS_FOURCC('B', 'C', '4', 'U'):
				return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_UNorm);
			case DDS_FOURCC('B', 'C', '4', 'S'):
				return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_SNorm);
			case DDS_FOURCC('B', 'C', '5', 'U'):
				return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_UNorm);
			case DDS_FOURCC('B', 'C', '5', 'S'):
				return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_SNorm);
			case 36:
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UNorm);
			case 110:
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SNorm);
			case 111:
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_Float);
			case 112:
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_Float);
			case 113:
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
			case 114:
				return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_Float);
			case 115:
				return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_Float);
			case 116:
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
			default:
				return dsGfxFormat_Unknown;
		}
	}

#define DDS_CHECK_BITS(r, g, b, a) (format->rBitMask == (r) && format->gBitMask == (g) && \
	format->bBitMask == (b) && format->aBitMask == (a))
	if (DDS_CHECK_BITS(0xF0, 0x0F, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R4G4, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xF000, 0x0F00, 0x00F0, 0x000F))
		return dsGfxFormat_decorate(dsGfxFormat_R4G4B4A4, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x00F0, 0x0F00, 0xF000, 0x000F))
		return dsGfxFormat_decorate(dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xF800, 0x7E0, 0x1F, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x1F, 0x7E0, 0xF800, 0))
		return dsGfxFormat_decorate(dsGfxFormat_B5G6R5, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xF800, 0x7C0, 0x3E, 0x1))
		return dsGfxFormat_decorate(dsGfxFormat_R5G5B5A1, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x3E, 0x7C0, 0xF800, 0x1))
		return dsGfxFormat_decorate(dsGfxFormat_B5G5R5A1, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x7C00, 0x3E0, 0x1F, 0x8000))
		return dsGfxFormat_decorate(dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF, 0, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF00, 0xFF, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF0000, 0xFF00, 0xFF, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF, 0xFF00, 0xFF0000, 0))
		return dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF000000, 0xFF0000, 0xFF00, 0xFF))
		return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF00, 0xFF0000, 0xFF000000, 0xFF))
		return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFF, 0xFF00, 0xFF0000, 0xFF000000))
		return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x3FF00000, 0xFFC00, 0x3FF, 0xC0000000))
		return dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0x3FF, 0xFFC00, 0x3FF00000, 0xC0000000))
		return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFFFF, 0, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFFFF0000, 0xFFFF, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UNorm);
	else if (DDS_CHECK_BITS(0xFFFFFFFF, 0, 0, 0))
		return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_UNorm);
#undef DDS_CHECK_BITS

	return dsGfxFormat_Unknown;
}

static dsGfxFormat getDdsDxt10Format(const DdsHeaderDxt10* format)
{
	switch (format->dxgiFormat)
	{
		case DdsDxt10Format_R32G32B32A32_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_UNorm);
		case DdsDxt10Format_R32G32B32A32_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
		case DdsDxt10Format_R32G32B32A32_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_UInt);
		case DdsDxt10Format_R32G32B32A32_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_SInt);
		case DdsDxt10Format_R32G32B32_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_UNorm);
		case DdsDxt10Format_R32G32B32_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_Float);
		case DdsDxt10Format_R32G32B32_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_UInt);
		case DdsDxt10Format_R32G32B32_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_SInt);
		case DdsDxt10Format_R16G16B16A16_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16G16B16A16_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
		case DdsDxt10Format_R16G16B16A16_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16G16B16A16_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UInt);
		case DdsDxt10Format_R16G16B16A16_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SNorm);
		case DdsDxt10Format_R16G16B16A16_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SInt);
		case DdsDxt10Format_R32G32_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_UNorm);
		case DdsDxt10Format_R32G32_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_Float);
		case DdsDxt10Format_R32G32_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_UInt);
		case DdsDxt10Format_R32G32_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_SInt);
		case DdsDxt10Format_R32G8X24_TYPELESS:
		case DdsDxt10Format_D32_FLOAT_S8X24_UINT:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_R32_FLOAT_X8X24_TYPELESS:
			return dsGfxFormat_D32S8_Float;
		case DdsDxt10Format_X32_TYPELESS_G8X24_UINT:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_R10G10B10A2_TYPELESS:
		case DdsDxt10Format_R10G10B10A2_UNORM:
				return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
		case DdsDxt10Format_R10G10B10A2_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UInt);
		case DdsDxt10Format_R11G11B10_FLOAT:
			return dsGfxFormat_B10G11R11_UFloat;
		case DdsDxt10Format_R8G8B8A8_TYPELESS:
		case DdsDxt10Format_R8G8B8A8_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		case DdsDxt10Format_R8G8B8A8_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB);
		case DdsDxt10Format_R8G8B8A8_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UInt);
		case DdsDxt10Format_R8G8B8A8_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SNorm);
		case DdsDxt10Format_R8G8B8A8_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SInt);
		case DdsDxt10Format_R16G16_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16G16_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_Float);
		case DdsDxt10Format_R16G16_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16G16_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UInt);
		case DdsDxt10Format_R16G16_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SNorm);
		case DdsDxt10Format_R16G16_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SInt);
		case DdsDxt10Format_R32_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_UNorm);
		case DdsDxt10Format_D32_FLOAT:
			return dsGfxFormat_D32_Float;
		case DdsDxt10Format_R32_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_Float);
		case DdsDxt10Format_R32_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_UInt);
		case DdsDxt10Format_R32_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_SInt);
		case DdsDxt10Format_R24G8_TYPELESS:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_D24_UNORM_S8_UINT:
			return dsGfxFormat_D24S8;
		case DdsDxt10Format_R24_UNORM_X8_TYPELESS:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_X24_TYPELESS_G8_UINT:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_R8G8_TYPELESS:
		case DdsDxt10Format_R8G8_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UNorm);
		case DdsDxt10Format_R8G8_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UInt);
		case DdsDxt10Format_R8G8_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SNorm);
		case DdsDxt10Format_R8G8_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SInt);
		case DdsDxt10Format_R16_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16_FLOAT:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_Float);
		case DdsDxt10Format_D16_UNORM:
			return dsGfxFormat_D16;
		case DdsDxt10Format_R16_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UNorm);
		case DdsDxt10Format_R16_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UInt);
		case DdsDxt10Format_R16_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SNorm);
		case DdsDxt10Format_R16_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SInt);
		case DdsDxt10Format_R8_TYPELESS:
		case DdsDxt10Format_R8_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm);
		case DdsDxt10Format_R8_UINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UInt);
		case DdsDxt10Format_R8_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SNorm);
		case DdsDxt10Format_R8_SINT:
			return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SInt);
		case DdsDxt10Format_A8_UNORM:
		case DdsDxt10Format_R1_UNORM:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_R9G9B9E5_SHAREDEXP:
			return dsGfxFormat_E5B9G9R9_UFloat;
		case DdsDxt10Format_R8G8_B8G8_UNORM:
		case DdsDxt10Format_G8R8_G8B8_UNORM:
			return dsGfxFormat_Unknown;
		case DdsDxt10Format_BC1_TYPELESS:
		case DdsDxt10Format_BC1_UNORM:
			if (format->miscFlags2 == DdsDxt10MiscFlags2_AlphaModeOpaque)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm);
			else
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC1_UNORM_SRGB:
			if (format->miscFlags2 == DdsDxt10MiscFlags2_AlphaModeOpaque)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB);
			else
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA, dsGfxFormat_SRGB);
		case DdsDxt10Format_BC2_TYPELESS:
		case DdsDxt10Format_BC2_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC2_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_SRGB);
		case DdsDxt10Format_BC3_TYPELESS:
		case DdsDxt10Format_BC3_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC3_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_SRGB);
		case DdsDxt10Format_BC4_TYPELESS:
		case DdsDxt10Format_BC4_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC4_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_SNorm);
		case DdsDxt10Format_BC5_TYPELESS:
		case DdsDxt10Format_BC5_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC5_SNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_SNorm);
		case DdsDxt10Format_B5G6R5_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm);
		case DdsDxt10Format_B5G5R5A1_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm);
		case DdsDxt10Format_B8G8R8A8_UNORM:
		case DdsDxt10Format_B8G8R8X8_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm);
		case DdsDxt10Format_R10G10B10_XR_BIAS_A2_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
		case DdsDxt10Format_B8G8R8A8_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm);
		case DdsDxt10Format_B8G8R8A8_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB);
		case DdsDxt10Format_B8G8R8X8_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm);
		case DdsDxt10Format_B8G8R8X8_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB);
		case DdsDxt10Format_BC6H_TYPELESS:
			return dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_Float);
		case DdsDxt10Format_BC6H_UF16:
			return dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_UFloat);
		case DdsDxt10Format_BC6H_SF16:
			return dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_Float);
		case DdsDxt10Format_BC7_TYPELESS:
		case DdsDxt10Format_BC7_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_UNorm);
		case DdsDxt10Format_BC7_UNORM_SRGB:
			return dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_SRGB);
		case DdsDxt10Format_B4G4R4A4_UNORM:
			return dsGfxFormat_decorate(dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm);
		case DdsDxt10Format_IA44:
			return dsGfxFormat_decorate(dsGfxFormat_R4G4, dsGfxFormat_UNorm);
		case DdsDxt10Format_AYUV:
		case DdsDxt10Format_Y410:
		case DdsDxt10Format_Y416:
		case DdsDxt10Format_NV12:
		case DdsDxt10Format_P010:
		case DdsDxt10Format_P016:
		case DdsDxt10Format_420_OPAQUE:
		case DdsDxt10Format_YUY2:
		case DdsDxt10Format_Y210:
		case DdsDxt10Format_Y216:
		case DdsDxt10Format_NV11:
		case DdsDxt10Format_AI44:
		case DdsDxt10Format_P8:
		case DdsDxt10Format_A8P8:
		case DdsDxt10Format_P208:
		case DdsDxt10Format_V208:
		case DdsDxt10Format_V408:
		default:
			return dsGfxFormat_Unknown;
	}
}

static dsTextureData* loadDds(dsAllocator* allocator, dsStream* stream, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t magicNumber;
	if (!dsStream_read(stream, &magicNumber, sizeof(magicNumber)) ||
		magicNumber != DDS_MAGIC_NUMBER)
	{
		ddsFormatError(filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	DdsHeader header;
	if (!dsStream_read(stream, &header, sizeof(header)) || header.size != sizeof(DdsHeader) ||
		header.ddspf.size != sizeof(DdsPixelFormat))
	{
		ddsFormatError(filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t width = header.width;
	uint32_t height = header.height;
	uint32_t mipLevels = header.mipMapCount;

	uint32_t depth = 0;
	dsTextureDim dimension = dsTextureDim_2D;
	if (header.flags & DdsCaps2Flags_Volume)
	{
		depth = header.depth;
		dimension = dsTextureDim_3D;
	}
	else if (header.flags & DdsCaps2Flags_Cube)
	{
		depth = header.depth;
		dimension = dsTextureDim_Cube;
	}

	dsGfxFormat format;
	if ((header.ddspf.flags & DdsFormatFlags_FourCC) &&
		header.ddspf.fourCC == DDS_FOURCC('D', 'X', '1', '0'))
	{
		DdsHeaderDxt10 headerDxt10;
		if (!dsStream_read(stream, &headerDxt10, sizeof(headerDxt10)))
		{
			ddsFormatError(filePath);
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		format = getDdsDxt10Format(&headerDxt10);
		switch (headerDxt10.resourceDimension)
		{
			case DdsTextureDim_BUFFER:
			case DdsTextureDim_TEXTURE1D:
				dimension = dsTextureDim_1D;
				break;
			case DdsTextureDim_TEXTURE2D:
				dimension = dsTextureDim_2D;
				break;
			case DdsTextureDim_TEXTURE3D:
				dimension = dsTextureDim_3D;
				break;
		}

		if (headerDxt10.miscFlag & DdsDxt10MiscFlag_CubeMap)
			dimension = dsTextureDim_Cube;

		if (dimension != dsTextureDim_3D && headerDxt10.arraySize > 1)
			depth = headerDxt10.arraySize;
	}
	else
		format = getDdsFormat(&header.ddspf);

	if (format == dsGfxFormat_Unknown)
	{
		ddsError("Unsupported DDS texture format", filePath);
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureData* textureData = dsTextureData_create(allocator, format, dimension, width, height,
		depth, mipLevels);
	if (!textureData)
		DS_PROFILE_FUNC_RETURN(NULL);

	uint32_t elements = 1;
	uint32_t volumes = 1;
	if (dimension == dsTextureDim_3D)
		volumes = depth;
	else
		elements = dsMax(1U, depth);
	unsigned int faces = dimension == dsTextureDim_Cube ? 6 : 1;
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, format));
	unsigned int formatSize = dsGfxFormat_size(format);
	for (uint32_t element = 0; element < elements; ++element)
	{
		for (uint32_t face = 0; face < faces; ++face)
		{
			for (uint32_t level = 0; level < mipLevels; ++level)
			{
				uint32_t curWidth = dsMax(1U, width >> level);
				uint32_t curHeight = dsMax(1U, height >> level);
				uint32_t curVolumes = dsMax(1U, volumes >> level);
				unsigned int curBlocksX = (curWidth + blockX - 1)/blockX;
				unsigned int curBlocksY = (curHeight + blockY - 1)/blockY;

				for (uint32_t volume = 0; volume < curVolumes; ++volume)
				{
					uint8_t* curData = textureData->data + dsTexture_surfaceOffset(format,
						dimension, width, height, depth, mipLevels, (dsCubeFace)face,
						dimension == dsTextureDim_3D ? volume : element, level);
					if (!dsStream_read(stream, curData, curBlocksX*curBlocksY*formatSize))
					{
						ddsSizeError(filePath);
						DS_PROFILE_FUNC_RETURN(NULL);
					}
				}
			}
		}
	}

	DS_PROFILE_FUNC_RETURN(textureData);
}

dsTextureData* dsTextureData_loadDdsFile(dsAllocator* allocator, const char* filePath)
{
	if (!allocator || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFileStream fileStream;
	if (!dsFileStream_openPath(&fileStream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open DDS file '%s'.", filePath);
		return NULL;
	}

	dsTextureData* textureData = loadDds(allocator, (dsStream*)&fileStream, filePath);
	if (textureData)
	{
		uint64_t pos = dsStream_tell((dsStream*)&fileStream);
		dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_End);
		if (pos != dsStream_tell((dsStream*)&fileStream))
		{
			ddsError("Unexpected file size", filePath);
			dsTextureData_destroy(textureData);
			textureData = NULL;
		}
	}
	DS_VERIFY(dsStream_close((dsStream*)&fileStream));
	return textureData;
}

dsTextureData* dsTextureData_loadDdsStream(dsAllocator* allocator, dsStream* stream)
{
	if (!allocator || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	return loadDds(allocator, stream, NULL);
}

dsTexture* dsTextureData_loadDdsFileToTexture(dsResourceManager* resourceManager,
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

	dsTextureData* textureData = dsTextureData_loadDdsFile(tempAllocator, filePath);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}

dsTexture* dsTextureData_loadDdsStreamToTexture(dsResourceManager* resourceManager,
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

	dsTextureData* textureData = dsTextureData_loadDdsStream(tempAllocator, stream);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}
