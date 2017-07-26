/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Core/Assert.h>

bool dsGfxFormat_isValid(dsGfxFormat format)
{
	if (format & dsGfxFormat_StandardMask)
	{
		if (!(format & dsGfxFormat_DecoratorMask))
			return false;

		if ((format & dsGfxFormat_SpecialMask) || (format & dsGfxFormat_CompressedMask))
			return false;

		return true;
	}
	else if ((format & dsGfxFormat_SpecialMask))
	{
		if (format & dsGfxFormat_DecoratorMask)
			return false;

		if (format & dsGfxFormat_CompressedMask)
			return false;

		return true;
	}
	else if (format & dsGfxFormat_CompressedMask)
	{
		if (!(format & dsGfxFormat_DecoratorMask))
			return false;

		return true;
	}
	else
		return false;
}

unsigned int dsGfxFormat_size(dsGfxFormat format)
{
	if (!dsGfxFormat_isValid(format))
		return 0;

	static unsigned int standardSizes[] =
	{
		0,
		1,  // dsGfxFormat_R4G4
		2,  // dsGfxFormat_R4G4B4A4
		2,  // dsGfxFormat_B4G4R4A4
		2,  // dsGfxFormat_R5G6B5
		2,  // dsGfxFormat_B5G6R5
		2,  // dsGfxFormat_R5G5B5A1
		2,  // dsGfxFormat_B5G5R5A1
		2,  // dsGfxFormat_A1R5G5B5
		1,  // dsGfxFormat_R8
		2,  // dsGfxFormat_R8G8
		3,  // dsGfxFormat_R8G8B8
		3,  // dsGfxFormat_B8G8R8
		4,  // dsGfxFormat_R8G8B8A8
		4,  // dsGfxFormat_B8G8R8A8
		4,  // dsGfxFormat_A8B8G8R8
		4,  // dsGfxFormat_A2R10G10B10
		4,  // dsGfxFormat_A2B10G10R10
		2,  // dsGfxFormat_R16
		4,  // dsGfxFormat_R16G16
		6,  // dsGfxFormat_R16G16B16
		8,  // dsGfxFormat_R16G16B16A16
		4,  // dsGfxFormat_R32
		8,  // dsGfxFormat_R32G32
		12, // dsGfxFormat_R32G32B32
		16, // dsGfxFormat_R32G32B32A32
		8,  // dsGfxFormat_R64
		16, // dsGfxFormat_R64G64
		24, // dsGfxFormat_R64G64B64
		32, // dsGfxFormat_R64G64B64A64
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(standardSizes) == dsGfxFormat_StandardCount,
		standard_format_array_mismatch);

	static unsigned int specialSizes[] =
	{
		0,
		4, // dsGfxFormat_B10G11R11_UFloat
		4, // dsGfxFormat_E5B9G9R9_UFloat
		2, // dsGfxFormat_D16
		4, // dsGfxFormat_X8D24
		4, // dsGfxFormat_D32_Float
		1, // dsGfxFormat_S8
		3, // dsGfxFormat_D16S8
		4, // dsGfxFormat_D24S8
		5, // dsGfxFormat_D32S8_Float
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(specialSizes) == dsGfxFormat_SpecialCount,
		special_format_array_mismatch);

	static unsigned int compressedSizes[] =
	{
		0,
		8,  // dsGfxFormat_BC1_RGB
		8,  // dsGfxFormat_BC1_RGBA
		16, // dsGfxFormat_BC2
		16, // dsGfxFormat_BC3
		16, // dsGfxFormat_BC4
		16, // dsGfxFormat_BC5
		16, // dsGfxFormat_BC6H
		16, // dsGfxFormat_BC7
		8,  // dsGfxFormat_ETC1
		8,  // dsGfxFormat_ETC2_R8G8B8
		8,  // dsGfxFormat_ETC2_R8G8B8A1
		16, // dsGfxFormat_ETC2_R8G8B8A8
		8,  // dsGfxFormat_EAC_R11
		16, // dsGfxFormat_EAC_R11G11
		16, // dsGfxFormat_ASTC_4x4
		16, // dsGfxFormat_ASTC_5x4
		16, // dsGfxFormat_ASTC_5x5
		16, // dsGfxFormat_ASTC_6x5
		16, // dsGfxFormat_ASTC_6x6
		16, // dsGfxFormat_ASTC_8x5
		16, // dsGfxFormat_ASTC_8x6
		16, // dsGfxFormat_ASTC_8x8
		16, // dsGfxFormat_ASTC_10x5
		16, // dsGfxFormat_ASTC_10x6
		16, // dsGfxFormat_ASTC_10x8
		16, // dsGfxFormat_ASTC_10x10
		16, // dsGfxFormat_ASTC_12x10
		16, // dsGfxFormat_ASTC_12x12
		8,  // dsGfxFormat_PVRTC1_RGB_2BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC1_RGB_4BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_4BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_4BPP
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(compressedSizes) == dsGfxFormat_CompressedCount,
		compressed_format_array_mismatch);

	int index = dsGfxFormat_standardIndex(format);
	if (index > 0)
	{
		DS_ASSERT(index < dsGfxFormat_StandardCount);
		return standardSizes[index];
	}

	index = dsGfxFormat_specialIndex(format);
	if (index > 0)
	{
		DS_ASSERT(index < dsGfxFormat_SpecialCount);
		return specialSizes[index];
	}

	index = dsGfxFormat_compressedIndex(format);
	if (index > 0)
	{
		DS_ASSERT(index < dsGfxFormat_CompressedCount);
		return compressedSizes[index];
	}

	return 0;
}

bool dsGfxFormat_blockDimensions(unsigned int* outX, unsigned int* outY,
	dsGfxFormat format)
{
	if (!outX || !outY || !dsGfxFormat_isValid(format))
		return false;

	static unsigned int compressedX[] =
	{
		0,
		4,  // dsGfxFormat_BC1_RGB
		4,  // dsGfxFormat_BC1_RGBA
		4,  // dsGfxFormat_BC2
		4,  // dsGfxFormat_BC3
		4,  // dsGfxFormat_BC4
		4,  // dsGfxFormat_BC5
		4,  // dsGfxFormat_BC6H
		4,  // dsGfxFormat_BC7
		4,  // dsGfxFormat_ETC1
		4,  // dsGfxFormat_ETC2_R8G8B8
		4,  // dsGfxFormat_ETC2_R8G8B8A1
		4,  // dsGfxFormat_ETC2_R8G8B8A8
		4,  // dsGfxFormat_EAC_R11
		4,  // dsGfxFormat_EAC_R11G11
		4,  // dsGfxFormat_ASTC_4x4
		5,  // dsGfxFormat_ASTC_5x4
		5,  // dsGfxFormat_ASTC_5x5
		6,  // dsGfxFormat_ASTC_6x5
		6,  // dsGfxFormat_ASTC_6x6
		8,  // dsGfxFormat_ASTC_8x5
		8,  // dsGfxFormat_ASTC_8x6
		8,  // dsGfxFormat_ASTC_8x8
		10, // dsGfxFormat_ASTC_10x5
		10, // dsGfxFormat_ASTC_10x6
		10, // dsGfxFormat_ASTC_10x8
		10, // dsGfxFormat_ASTC_10x10
		12, // dsGfxFormat_ASTC_12x10
		12, // dsGfxFormat_ASTC_12x12
		8,  // dsGfxFormat_PVRTC1_RGB_2BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_2BPP
		4,  // dsGfxFormat_PVRTC1_RGB_4BPP
		4,  // dsGfxFormat_PVRTC1_RGBA_4BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_2BPP
		4,  // dsGfxFormat_PVRTC2_RGBA_4BPP
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(compressedX) == dsGfxFormat_CompressedCount,
		compressed_format_x_array_mismatch);

	static unsigned int compressedY[] =
	{
		0,
		4,  // dsGfxFormat_BC1_RGB
		4,  // dsGfxFormat_BC1_RGBA
		4,  // dsGfxFormat_BC2
		4,  // dsGfxFormat_BC3
		4,  // dsGfxFormat_BC4
		4,  // dsGfxFormat_BC5
		4,  // dsGfxFormat_BC6H
		4,  // dsGfxFormat_BC7
		4,  // dsGfxFormat_ETC1
		4,  // dsGfxFormat_ETC2_R8G8B8
		4,  // dsGfxFormat_ETC2_R8G8B8A1
		4,  // dsGfxFormat_ETC2_R8G8B8A8
		4,  // dsGfxFormat_EAC_R11
		4,  // dsGfxFormat_EAC_R11G11
		4,  // dsGfxFormat_ASTC_4x4
		4,  // dsGfxFormat_ASTC_5x4
		5,  // dsGfxFormat_ASTC_5x5
		5,  // dsGfxFormat_ASTC_6x5
		6,  // dsGfxFormat_ASTC_6x6
		5,  // dsGfxFormat_ASTC_8x5
		6,  // dsGfxFormat_ASTC_8x6
		8,  // dsGfxFormat_ASTC_8x8
		5,  // dsGfxFormat_ASTC_10x5
		6,  // dsGfxFormat_ASTC_10x6
		8,  // dsGfxFormat_ASTC_10x8
		10, // dsGfxFormat_ASTC_10x10
		10, // dsGfxFormat_ASTC_12x10
		12, // dsGfxFormat_ASTC_12x12
		4,  // dsGfxFormat_PVRTC1_RGB_2BPP
		4,  // dsGfxFormat_PVRTC1_RGBA_2BPP
		4,  // dsGfxFormat_PVRTC1_RGB_4BPP
		4,  // dsGfxFormat_PVRTC1_RGBA_4BPP
		4,  // dsGfxFormat_PVRTC2_RGBA_2BPP
		4,  // dsGfxFormat_PVRTC2_RGBA_4BPP
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(compressedY) == dsGfxFormat_CompressedCount,
		compressed_format_y_array_mismatch);

	unsigned int index = dsGfxFormat_compressedIndex(format);
	if (index > 0)
	{
		DS_ASSERT(index < dsGfxFormat_CompressedCount);
		*outX = compressedX[index];
		*outY = compressedY[index];
	}
	else
	{
		*outX = 1;
		*outY = 1;
	}

	return true;
}

bool dsGfxFormat_minDimensions(unsigned int* outX, unsigned int* outY,
	dsGfxFormat format)
{
	if (!outX || !outY || !dsGfxFormat_isValid(format))
		return false;

	static unsigned int compressedX[] =
	{
		0,
		4,  // dsGfxFormat_BC1_RGB
		4,  // dsGfxFormat_BC1_RGBA
		4,  // dsGfxFormat_BC2
		4,  // dsGfxFormat_BC3
		4,  // dsGfxFormat_BC4
		4,  // dsGfxFormat_BC5
		4,  // dsGfxFormat_BC6H
		4,  // dsGfxFormat_BC7
		4,  // dsGfxFormat_ETC1
		4,  // dsGfxFormat_ETC2_R8G8B8
		4,  // dsGfxFormat_ETC2_R8G8B8A1
		4,  // dsGfxFormat_ETC2_R8G8B8A8
		4,  // dsGfxFormat_EAC_R11
		4,  // dsGfxFormat_EAC_R11G11
		4,  // dsGfxFormat_ASTC_4x4
		5,  // dsGfxFormat_ASTC_5x4
		5,  // dsGfxFormat_ASTC_5x5
		6,  // dsGfxFormat_ASTC_6x5
		6,  // dsGfxFormat_ASTC_6x6
		8,  // dsGfxFormat_ASTC_8x5
		8,  // dsGfxFormat_ASTC_8x6
		8,  // dsGfxFormat_ASTC_8x8
		10, // dsGfxFormat_ASTC_10x5
		10, // dsGfxFormat_ASTC_10x6
		10, // dsGfxFormat_ASTC_10x8
		10, // dsGfxFormat_ASTC_10x10
		12, // dsGfxFormat_ASTC_12x10
		12, // dsGfxFormat_ASTC_12x12
		16, // dsGfxFormat_PVRTC1_RGB_2BPP
		16, // dsGfxFormat_PVRTC1_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC1_RGB_4BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_4BPP
		16, // dsGfxFormat_PVRTC2_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_4BPP
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(compressedX) == dsGfxFormat_CompressedCount,
		compressed_format_x_array_mismatch);

	static unsigned int compressedY[] =
	{
		0,
		4,  // dsGfxFormat_BC1_RGB
		4,  // dsGfxFormat_BC1_RGBA
		4,  // dsGfxFormat_BC2
		4,  // dsGfxFormat_BC3
		4,  // dsGfxFormat_BC4
		4,  // dsGfxFormat_BC5
		4,  // dsGfxFormat_BC6H
		4,  // dsGfxFormat_BC7
		4,  // dsGfxFormat_ETC1
		4,  // dsGfxFormat_ETC2_R8G8B8
		4,  // dsGfxFormat_ETC2_R8G8B8A1
		4,  // dsGfxFormat_ETC2_R8G8B8A8
		4,  // dsGfxFormat_EAC_R11
		4,  // dsGfxFormat_EAC_R11G11
		4,  // dsGfxFormat_ASTC_4x4
		4,  // dsGfxFormat_ASTC_5x4
		5,  // dsGfxFormat_ASTC_5x5
		5,  // dsGfxFormat_ASTC_6x5
		6,  // dsGfxFormat_ASTC_6x6
		5,  // dsGfxFormat_ASTC_8x5
		6,  // dsGfxFormat_ASTC_8x6
		8,  // dsGfxFormat_ASTC_8x8
		5,  // dsGfxFormat_ASTC_10x5
		6,  // dsGfxFormat_ASTC_10x6
		8,  // dsGfxFormat_ASTC_10x8
		10, // dsGfxFormat_ASTC_10x10
		10, // dsGfxFormat_ASTC_12x10
		12, // dsGfxFormat_ASTC_12x12
		8,  // dsGfxFormat_PVRTC1_RGB_2BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC1_RGB_4BPP
		8,  // dsGfxFormat_PVRTC1_RGBA_4BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_2BPP
		8,  // dsGfxFormat_PVRTC2_RGBA_4BPP
	};
	DS_STATIC_ASSERT(DS_ARRAY_SIZE(compressedY) == dsGfxFormat_CompressedCount,
		compressed_format_y_array_mismatch);

	unsigned int index = dsGfxFormat_compressedIndex(format);
	if (index > 0)
	{
		DS_ASSERT(index < dsGfxFormat_CompressedCount);
		*outX = compressedX[index];
		*outY = compressedY[index];
	}
	else
	{
		*outX = 1;
		*outY = 1;
	}

	return true;
}

bool dsGfxFormat_vertexSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	if (!resourceManager || !resourceManager->vertexFormatSupportedFunc ||
		!dsGfxFormat_isValid(format))
	{
		return false;
	}

	return resourceManager->vertexFormatSupportedFunc(resourceManager, format);
}

bool dsGfxFormat_textureSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	if (!resourceManager || !resourceManager->textureFormatSupportedFunc ||
		!dsGfxFormat_isValid(format))
	{
		return false;
	}

	return resourceManager->textureFormatSupportedFunc(resourceManager, format);
}

bool dsGfxFormat_offscreenSupported(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	if (!resourceManager || !resourceManager->offscreenFormatSupportedFunc ||
		!dsGfxFormat_isValid(format))
	{
		return false;
	}

	return resourceManager->offscreenFormatSupportedFunc(resourceManager, format);
}

bool dsGfxFormat_textureBufferSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	if (!resourceManager || !resourceManager->textureBufferFormatSupportedFunc ||
		!dsGfxFormat_isValid(format))
	{
		return false;
	}

	return resourceManager->textureBufferFormatSupportedFunc(resourceManager, format);
}

bool dsGfxFormat_generateMipmapsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	if (!resourceManager || !resourceManager->generateMipmapFormatSupportedFunc ||
		!dsGfxFormat_isValid(format))
	{
		return false;
	}

	return resourceManager->generateMipmapFormatSupportedFunc(resourceManager, format);
}

bool dsGfxFormat_textureCopySupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat)
{
	if (!resourceManager || !resourceManager->textureCopyFormatsSupportedFunc ||
		!dsGfxFormat_isValid(srcFormat) || !dsGfxFormat_isValid(dstFormat))
	{
		return false;
	}

	return resourceManager->textureCopyFormatsSupportedFunc(resourceManager, srcFormat, dstFormat);
}

bool dsGfxFormat_textureBlitSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat, dsBlitFilter filter)
{
	if (!resourceManager || !resourceManager->textureBlitFormatsSupportedFunc ||
		!dsGfxFormat_isValid(srcFormat) || !dsGfxFormat_isValid(dstFormat))
	{
		return false;
	}

	return resourceManager->textureBlitFormatsSupportedFunc(resourceManager, srcFormat, dstFormat,
		filter);
}

unsigned int dsGfxFormat_standardIndex(dsGfxFormat format);
dsGfxFormat dsGfxFormat_standardEnum(unsigned int index);
unsigned int dsGfxFormat_specialIndex(dsGfxFormat format);
dsGfxFormat dsGfxFormat_specialEnum(unsigned int index);
unsigned int dsGfxFormat_compressedIndex(dsGfxFormat format);
dsGfxFormat dsGfxFormat_compressedEnum(unsigned int index);
unsigned int dsGfxFormat_decoratorIndex(dsGfxFormat format);
dsGfxFormat dsGfxFormat_decoratorEnum(unsigned int index);
dsGfxFormat dsGfxFormat_decorate(dsGfxFormat format, dsGfxFormat decorator);
