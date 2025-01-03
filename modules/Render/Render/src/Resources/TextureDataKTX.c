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
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_HALF_FLOAT                     0x140B
#define GL_RED                            0x1903
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGB16                          0x8054
#define GL_RGBA16                         0x805B
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT 0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT 0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT 0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT 0x8A57
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
#define GL_SRGB8                          0x8C41
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_RGB565                         0x8D62
#define GL_ETC1_RGB8_OES                  0x8D64
#define GL_HALF_FLOAT_OES                 0x8D61
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_BGR_INTEGER                    0x8D9A
#define GL_BGRA_INTEGER                   0x8D9B
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE
#define GL_COMPRESSED_RGBA_BPTC_UNORM     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_RGB10_A2UI                     0x906F
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0x9138
#define GL_COMPRESSED_R11_EAC             0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC      0x9271
#define GL_COMPRESSED_RG11_EAC            0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC     0x9273
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_SRGB8_ETC2          0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR   0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR   0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR   0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR   0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR   0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR   0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR   0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR   0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR  0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR  0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR  0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG 0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG 0x93F1

static char ktxHeader[12] =
{
	'\xAB', 'K', 'T', 'X', ' ', '1', '1', '\xBB', '\r', '\n', '\x1A', '\n'
};

static void ktxError(const char* errorString, const char* filePath)
{
	if (filePath)
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%s when reading file '%s'.", errorString, filePath);
	else
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%s.", errorString);
}

static void ktxSizeError(const char* filePath)
{
	ktxError("Invalid KTX texture file size", filePath);
	errno = EFORMAT;
}

static bool readUInt32(dsStream* stream, uint32_t* value, const char* filePath)
{
	if (dsStream_read(stream, value, sizeof(*value)) != sizeof(*value))
	{
		ktxSizeError(filePath);
		return false;
	}

	return true;
}

static bool skipBytes(dsStream* stream, uint64_t size, const char* filePath)
{
	if (dsStream_skip(stream, size) != size)
	{
		ktxSizeError(filePath);
		return false;
	}

	return true;
}

static dsGfxFormat getTextureFormat(uint32_t glType, uint32_t glFormat, uint32_t glInternalFormat)
{
	switch (glInternalFormat)
	{
		case GL_RGBA4:
			if (glType != GL_UNSIGNED_SHORT_4_4_4_4)
				return dsGfxFormat_Unknown;

			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_R4G4B4A4, dsGfxFormat_UNorm);
			else if (glFormat == GL_BGRA)
				return dsGfxFormat_decorate(dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB565:
			if (glFormat != GL_RGB)
				return dsGfxFormat_Unknown;

			if (glType == GL_UNSIGNED_SHORT_5_6_5)
				return dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm);
			else if (glType == GL_UNSIGNED_SHORT_5_6_5_REV)
				return dsGfxFormat_decorate(dsGfxFormat_B5G6R5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB5_A1:
			if (glType == GL_UNSIGNED_SHORT_5_5_5_1)
			{
				if (glFormat == GL_RGBA)
					return dsGfxFormat_decorate(dsGfxFormat_R5G5B5A1, dsGfxFormat_UNorm);
				else if (glFormat == GL_BGRA)
					return dsGfxFormat_decorate(dsGfxFormat_B5G5R5A1, dsGfxFormat_UNorm);
			}
			else if (glType == GL_UNSIGNED_SHORT_1_5_5_5_REV && glFormat == GL_BGRA)
				return dsGfxFormat_decorate(dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_R8:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_R8_SNORM:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_R8UI:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R8I:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG8:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_RG8_SNORM:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_RG8UI:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG8I:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RGB8:
			if (glFormat == GL_RGB && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_SRGB8:
			if (glFormat == GL_RGB && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_RGB8_SNORM:
			if (glFormat == GL_RGB && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
				return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB8UI:
			if (glFormat == GL_RGB && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UInt);
			return dsGfxFormat_Unknown;
		case GL_RGB8I:
			if (glFormat == GL_RGB && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE))
				return dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SInt);
			return dsGfxFormat_Unknown;
		case GL_RGBA8:
			if (glFormat == GL_RGBA)
			{
				if (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
					glType == GL_UNSIGNED_INT_8_8_8_8)
				{
					return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
				}
				else if (glType == GL_UNSIGNED_INT_8_8_8_8_REV)
					return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_UNorm);
			}
			else if (glFormat == GL_BGRA && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
				glType == GL_UNSIGNED_INT_8_8_8_8))
			{
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_SRGB8_ALPHA8:
			if (glFormat == GL_RGBA)
			{
				if (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
					glType == GL_UNSIGNED_INT_8_8_8_8)
				{
					return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB);
				}
				else if (glType == GL_UNSIGNED_INT_8_8_8_8_REV)
					return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SRGB);
			}
			else if (glFormat == GL_BGRA && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
				glType == GL_UNSIGNED_INT_8_8_8_8))
			{
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB);
			}
			return dsGfxFormat_Unknown;
		case GL_RGBA8_SNORM:
			if (glFormat == GL_RGBA)
			{
				if (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
					glType == GL_UNSIGNED_INT_8_8_8_8)
				{
					return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SNorm);
				}
				else if (glType == GL_UNSIGNED_INT_8_8_8_8_REV)
					return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SNorm);
			}
			else if (glFormat == GL_BGRA && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
				glType == GL_UNSIGNED_INT_8_8_8_8))
			{
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_RGBA8UI:
			if (glFormat == GL_RGBA)
			{
				if (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
					glType == GL_UNSIGNED_INT_8_8_8_8)
				{
					return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UInt);
				}
				else if (glType == GL_UNSIGNED_INT_8_8_8_8_REV)
					return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_UInt);
			}
			else if (glFormat == GL_BGRA && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
				glType == GL_UNSIGNED_INT_8_8_8_8))
			{
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RGBA8I:
			if (glFormat == GL_RGBA)
			{
				if (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
					glType == GL_UNSIGNED_INT_8_8_8_8)
				{
					return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SInt);
				}
				else if (glType == GL_UNSIGNED_INT_8_8_8_8_REV)
					return dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SInt);
			}
			else if (glFormat == GL_BGRA && (glType == GL_BYTE || glType == GL_UNSIGNED_BYTE ||
				glType == GL_UNSIGNED_INT_8_8_8_8))
			{
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RGB10_A2:
		case GL_RGB10:
			if (glFormat == GL_BGRA && glType == GL_UNSIGNED_INT_2_10_10_10_REV)
				return dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10, dsGfxFormat_UNorm);
			else if (glFormat == GL_RGBA && glType == GL_UNSIGNED_INT_2_10_10_10_REV)
				return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB10_A2UI:
			if ((glFormat == GL_BGRA || glFormat == GL_BGRA_INTEGER) &&
				glType == GL_UNSIGNED_INT_2_10_10_10_REV)
			{
				return dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10, dsGfxFormat_UInt);
			}
			else if ((glFormat == GL_RGBA || glFormat == GL_RGBA_INTEGER) &&
				glType == GL_UNSIGNED_INT_2_10_10_10_REV)
			{
				return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R16:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_R16_SNORM:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_R16UI:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R16I:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R16F:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_HALF_FLOAT || glType == GL_HALF_FLOAT_OES))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_Float);
			}
			return dsGfxFormat_Unknown;
		case GL_RG16:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_RG16_SNORM:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SNorm);
			}
			return dsGfxFormat_Unknown;
		case GL_RG16UI:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG16I:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG16F:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_HALF_FLOAT || glType == GL_HALF_FLOAT_OES))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_Float);
			}
			return dsGfxFormat_Unknown;
		case GL_RGB16:
			if (glFormat == GL_RGB && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB16_SNORM:
			if (glFormat == GL_RGB && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_RGB16UI:
			if (glFormat == GL_RGB && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_UInt);
			return dsGfxFormat_Unknown;
		case GL_RGB16I:
			if (glFormat == GL_RGB && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_SInt);
			return dsGfxFormat_Unknown;
		case GL_RGB16F:
			if (glFormat == GL_RGB && (glType == GL_HALF_FLOAT || glType == GL_HALF_FLOAT_OES))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_RGBA16:
			if (glFormat == GL_RGBA && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_RGBA16_SNORM:
			if (glFormat == GL_RGBA && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_RGBA16UI:
			if (glFormat == GL_RGBA && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_UInt);
			return dsGfxFormat_Unknown;
		case GL_RGBA16I:
			if (glFormat == GL_RGBA && (glType == GL_SHORT || glType == GL_UNSIGNED_SHORT))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_SInt);
			return dsGfxFormat_Unknown;
		case GL_RGBA16F:
			if (glFormat == GL_RGBA && (glType == GL_HALF_FLOAT || glType == GL_HALF_FLOAT_OES))
				return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_R32UI:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_INT || glType == GL_UNSIGNED_INT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R32I:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) &&
				(glType == GL_INT || glType == GL_UNSIGNED_INT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_R32F:
			if ((glFormat == GL_RED || glFormat == GL_LUMINANCE) && glType == GL_FLOAT)
				return dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_RG32UI:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_INT || glType == GL_UNSIGNED_INT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_UInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG32I:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) &&
				(glType == GL_INT || glType == GL_UNSIGNED_INT))
			{
				return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_SInt);
			}
			return dsGfxFormat_Unknown;
		case GL_RG32F:
			if ((glFormat == GL_RG || glFormat == GL_LUMINANCE_ALPHA) && glType == GL_FLOAT)
				return dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_RGB32UI:
			if (glFormat == GL_RGB && (glType == GL_INT || glType == GL_UNSIGNED_INT))
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_UInt);
			return dsGfxFormat_Unknown;
		case GL_RGB32I:
			if (glFormat == GL_RGB && (glType == GL_INT || glType == GL_UNSIGNED_INT))
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_SInt);
			return dsGfxFormat_Unknown;
		case GL_RGB32F:
			if (glFormat == GL_RGB && glType == GL_FLOAT)
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_RGBA32UI:
			if (glFormat == GL_RGBA && (glType == GL_INT || glType == GL_UNSIGNED_INT))
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_UInt);
			return dsGfxFormat_Unknown;
		case GL_RGBA32I:
			if (glFormat == GL_RGBA && (glType == GL_INT || glType == GL_UNSIGNED_INT))
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_SInt);
			return dsGfxFormat_Unknown;
		case GL_RGBA32F:
			if (glFormat == GL_RGBA && glType == GL_FLOAT)
				return dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_R11F_G11F_B10F:
			if (glFormat == GL_RGB && glFormat == GL_UNSIGNED_INT_10F_11F_11F_REV)
				return dsGfxFormat_B10G11R11_UFloat;
			return dsGfxFormat_Unknown;
		case GL_RGB9_E5:
			if (glFormat == GL_RGB && glFormat == GL_UNSIGNED_INT_5_9_9_9_REV)
				return dsGfxFormat_E5B9G9R9_UFloat;
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RED_RGTC1:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RG_RGTC2:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_UFloat);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_Float);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_BPTC_UNORM:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_ETC1_RGB8_OES:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_ETC1, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB8_ETC2:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ETC2:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A1, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A1, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A8, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A8, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_R11_EAC:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_EAC_R11, dsGfxFormat_UNorm);
		case GL_COMPRESSED_SIGNED_R11_EAC:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_EAC_R11, dsGfxFormat_SNorm);
		case GL_COMPRESSED_RG11_EAC:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_EAC_R11G11, dsGfxFormat_UNorm);
		case GL_COMPRESSED_SIGNED_RG11_EAC:
			if (glFormat == GL_RED)
				return dsGfxFormat_decorate(dsGfxFormat_EAC_R11G11, dsGfxFormat_SNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_4x4, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_4x4, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_5x4, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_5x4, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_5x5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_5x5, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_6x5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_6x5, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_6x6, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_6x6, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x6, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x6, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x8, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_8x8, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x5, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x5, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x6, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x6, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x8, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x8, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x10, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_10x10, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_12x10, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_12x10, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_12x12, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_ASTC_12x12, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_2BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_2BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_2BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_2BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_4BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:
			if (glFormat == GL_RGB)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_4BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_4BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_4BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_2BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_2BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_4BPP, dsGfxFormat_UNorm);
			return dsGfxFormat_Unknown;
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:
			if (glFormat == GL_RGBA)
				return dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_4BPP, dsGfxFormat_SRGB);
			return dsGfxFormat_Unknown;
	}

	return dsGfxFormat_Unknown;
}

dsTextureData* dsTextureData_loadKTX(bool* isKTX, dsAllocator* allocator, dsStream* stream,
	const char* filePath)
{
	if (isKTX)
		*isKTX = false;
	if (!allocator || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	char header[sizeof(ktxHeader)];
	if (dsStream_read(stream, header, sizeof(header)) != sizeof(header))
	{
		if (isKTX)
			*isKTX = false;
		else
			ktxSizeError(filePath);
		return NULL;
	}

	if (memcmp(header, ktxHeader, sizeof(ktxHeader)) != 0)
	{
		if (isKTX)
			*isKTX = false;
		else
		{
			ktxError("Invalid KTX file", filePath);
			errno = EFORMAT;
		}
		return NULL;
	}

	uint32_t endianness;
	if (!readUInt32(stream, &endianness, filePath))
		return NULL;

	if (endianness != 0x04030201)
	{
		ktxError("Invalid KTX endianness", filePath);
		errno = EFORMAT;
		return NULL;
	}

	uint32_t glType, glTypeSize, glFormat, glInternalFormat, glBaseInternalFormat;
	if (!readUInt32(stream, &glType, filePath) || !readUInt32(stream, &glTypeSize, filePath) ||
		!readUInt32(stream, &glFormat, filePath) ||
		!readUInt32(stream, &glInternalFormat, filePath) ||
		!readUInt32(stream, &glBaseInternalFormat, filePath))
	{
		return NULL;
	}

	dsGfxFormat format = getTextureFormat(glType, glFormat, glInternalFormat);
	if (format == dsGfxFormat_Unknown)
	{
		ktxError("Unknown KTX pixel format", filePath);
		errno = EFORMAT;
		return NULL;
	}

	uint32_t width, height, depth;
	if (!readUInt32(stream, &width, filePath) || !readUInt32(stream, &height, filePath) ||
		!readUInt32(stream, &depth, filePath))
	{
		return NULL;
	}

	uint32_t arrayElements, faces, mipLevels;
	if (!readUInt32(stream, &arrayElements, filePath) || !readUInt32(stream, &faces, filePath) ||
		!readUInt32(stream, &mipLevels, filePath))
	{
		return NULL;
	}

	uint32_t metadataSize;
	if (!readUInt32(stream, &metadataSize, filePath))
		return NULL;

	if (!skipBytes(stream, metadataSize, filePath))
		return NULL;

	dsTextureDim textureDim;
	if (depth > 0)
		textureDim = dsTextureDim_3D;
	else if (faces == 6)
		textureDim = dsTextureDim_Cube;
	else if (height == 0)
	{
		textureDim = dsTextureDim_1D;
		height = 1;
	}
	else
		textureDim = dsTextureDim_2D;

	if (textureDim != dsTextureDim_3D)
		depth = arrayElements;

	dsTextureInfo info = {format, textureDim, width, height, depth, mipLevels, 1};
	dsTextureData* textureData = dsTextureData_create(allocator, &info);
	if (!textureData)
		return NULL;

	if (depth == 0)
		depth = 1;
	bool compressed = dsGfxFormat_compressedIndex(format) > 0;

	size_t curOffset = 0;
	for (uint32_t mip = 0; mip < mipLevels; ++mip)
	{
		uint32_t imageSize;
		if (!readUInt32(stream, &imageSize, filePath))
		{
			dsTextureData_destroy(textureData);
			return NULL;
		}
		DS_ASSERT(imageSize % 4 == 0);

		uint32_t curWidth = dsMax(width >> mip, 1U);
		uint32_t curHeight = dsMax(height >> mip, 1U);
		uint32_t curDepth = depth;
		if (textureDim == dsTextureDim_3D)
			curDepth = dsMax(depth >> mip, 1U);

		if (compressed)
		{
			dsTextureInfo surfaceInfo = {format, textureDim, curWidth, curHeight, curDepth, 1, 1};
			size_t size = dsTexture_size(&surfaceInfo);
			if (dsStream_read(stream, textureData->data + curOffset, size) != size)
			{
				ktxSizeError(filePath);
				dsTextureData_destroy(textureData);
				return NULL;
			}
			curOffset += size;
		}
		else
		{
			unsigned int formatSize = dsGfxFormat_size(format);
			for (uint32_t d = 0; d < depth; ++d)
			{
				for (uint32_t f = 0; f < faces; ++f)
				{
					uint32_t rowSize = curWidth*formatSize;
					uint32_t padding = rowSize % 4;
					if (padding != 0)
						padding = 4 - padding;

					for (uint32_t h = 0; h < curHeight; ++h)
					{
						if (dsStream_read(stream, textureData->data + curOffset, rowSize) !=
							rowSize)
						{
							ktxSizeError(filePath);
							dsTextureData_destroy(textureData);
							return NULL;
						}

						if (!skipBytes(stream, padding, filePath))
						{
							ktxSizeError(filePath);
							dsTextureData_destroy(textureData);
							return NULL;
						}

						curOffset += rowSize;
					}
				}
			}
		}
	}

	DS_ASSERT(curOffset == textureData->dataSize);

	return textureData;
}

dsTextureData* dsTextureData_loadKTXFile(dsAllocator* allocator, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open KTX file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureData* textureData = dsTextureData_loadKTX(NULL, allocator, (dsStream*)&stream,
		filePath);
	if (textureData)
	{
		if (dsFileStream_remainingBytes(&stream) != 0)
		{
			ktxError("Unexpected file size", filePath);
			dsTextureData_destroy(textureData);
			textureData = NULL;
		}
	}
	DS_VERIFY(dsStream_close((dsStream*)&stream));
	DS_PROFILE_FUNC_RETURN(textureData);
}

dsTextureData* dsTextureData_loadKTXResource(dsAllocator* allocator, dsFileResourceType type,
	const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open KTX file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureData* textureData = dsTextureData_loadKTX(NULL, allocator, (dsStream*)&stream,
		filePath);
	if (textureData)
	{
		if (dsResourceStream_remainingBytes(&stream) != 0)
		{
			ktxError("Unexpected file size", filePath);
			dsTextureData_destroy(textureData);
			textureData = NULL;
		}
	}
	DS_VERIFY(dsStream_close((dsStream*)&stream));
	DS_PROFILE_FUNC_RETURN(textureData);
}

dsTextureData* dsTextureData_loadKTXStream(dsAllocator* allocator, dsStream* stream)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsTextureData* result = dsTextureData_loadKTX(NULL, allocator, stream, NULL);
	DS_PROFILE_FUNC_RETURN(result);
}

dsTexture* dsTextureData_loadKTXFileToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, const char* filePath,
	const dsTextureDataOptions* options, dsTextureUsage usage, dsGfxMemory memoryHints)
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

	dsTextureData* textureData = dsTextureData_loadKTXFile(tempAllocator, filePath);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		options, usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}

dsTexture* dsTextureData_loadKTXResourceToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsFileResourceType type,
	const char* filePath, const dsTextureDataOptions* options, dsTextureUsage usage,
	dsGfxMemory memoryHints)
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

	dsTextureData* textureData = dsTextureData_loadKTXResource(tempAllocator, type, filePath);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		options, usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}

dsTexture* dsTextureData_loadKTXStreamToTexture(dsResourceManager* resourceManager,
	dsAllocator* textureAllocator, dsAllocator* tempAllocator, dsStream* stream,
	const dsTextureDataOptions* options, dsTextureUsage usage, dsGfxMemory memoryHints)
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

	dsTextureData* textureData = dsTextureData_loadKTXStream(tempAllocator, stream);
	if (!textureData)
		return NULL;

	dsTexture* texture = dsTextureData_createTexture(resourceManager, textureAllocator, textureData,
		options, usage, memoryHints);
	dsTextureData_destroy(textureData);
	return texture;
}
