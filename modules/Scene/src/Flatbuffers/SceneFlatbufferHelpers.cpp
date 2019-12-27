/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

namespace DeepSeaScene
{

static const dsGfxFormat textureFormatMap[] =
{
	dsGfxFormat_R4G4,
	dsGfxFormat_R4G4B4A4,
	dsGfxFormat_B4G4R4A4,
	dsGfxFormat_A4R4G4B4,
	dsGfxFormat_R5G6B5,
	dsGfxFormat_B5G6R5,
	dsGfxFormat_R5G5B5A1,
	dsGfxFormat_B5G5R5A1,
	dsGfxFormat_A1R5G5B5,
	dsGfxFormat_R8,
	dsGfxFormat_R8G8,
	dsGfxFormat_R8G8B8,
	dsGfxFormat_B8G8R8,
	dsGfxFormat_R8G8B8A8,
	dsGfxFormat_B8G8R8A8,
	dsGfxFormat_A8B8G8R8,
	dsGfxFormat_A2R10G10B10,
	dsGfxFormat_A2B10G10R10,
	dsGfxFormat_R16,
	dsGfxFormat_R16G16,
	dsGfxFormat_R16G16B16,
	dsGfxFormat_R16G16B16A16,
	dsGfxFormat_R32,
	dsGfxFormat_R32G32,
	dsGfxFormat_R32G32B32,
	dsGfxFormat_R32G32B32A32,
	dsGfxFormat_R64,
	dsGfxFormat_R64G64,
	dsGfxFormat_R64G64B64,
	dsGfxFormat_R64G64B64A64,
	dsGfxFormat_B10G11R11_UFloat,
	dsGfxFormat_E5B9G9R9_UFloat,
	dsGfxFormat_D16,
	dsGfxFormat_X8D24,
	dsGfxFormat_D32_Float,
	dsGfxFormat_S8,
	dsGfxFormat_D16S8,
	dsGfxFormat_D24S8,
	dsGfxFormat_D32S8_Float,
	dsGfxFormat_BC1_RGB,
	dsGfxFormat_BC1_RGBA,
	dsGfxFormat_BC2,
	dsGfxFormat_BC3,
	dsGfxFormat_BC4,
	dsGfxFormat_BC5,
	dsGfxFormat_BC6H,
	dsGfxFormat_BC7,
	dsGfxFormat_ETC1,
	dsGfxFormat_ETC2_R8G8B8,
	dsGfxFormat_ETC2_R8G8B8A1,
	dsGfxFormat_ETC2_R8G8B8A8,
	dsGfxFormat_EAC_R11,
	dsGfxFormat_EAC_R11G11,
	dsGfxFormat_ASTC_4x4,
	dsGfxFormat_ASTC_5x4,
	dsGfxFormat_ASTC_5x5,
	dsGfxFormat_ASTC_6x5,
	dsGfxFormat_ASTC_6x6,
	dsGfxFormat_ASTC_8x5,
	dsGfxFormat_ASTC_8x6,
	dsGfxFormat_ASTC_8x8,
	dsGfxFormat_ASTC_10x5,
	dsGfxFormat_ASTC_10x6,
	dsGfxFormat_ASTC_10x8,
	dsGfxFormat_ASTC_10x10,
	dsGfxFormat_ASTC_12x10,
	dsGfxFormat_ASTC_12x12,
	dsGfxFormat_PVRTC1_RGB_2BPP,
	dsGfxFormat_PVRTC1_RGBA_2BPP,
	dsGfxFormat_PVRTC1_RGB_4BPP,
	dsGfxFormat_PVRTC1_RGBA_4BPP,
	dsGfxFormat_PVRTC2_RGBA_2BPP,
	dsGfxFormat_PVRTC2_RGBA_4BPP
};

static_assert(DS_ARRAY_SIZE(textureFormatMap) == static_cast<uint32_t>(TextureFormat::MAX) + 1,
	"Invalid texture format map size.");

static const dsGfxFormat vertexElementFormatMap[] =
{
	dsGfxFormat_Unknown,
	dsGfxFormat_X8,
	dsGfxFormat_X8Y8,
	dsGfxFormat_X8Y8Z8,
	dsGfxFormat_X8Y8Z8W8,
	dsGfxFormat_W2X10Y10Z10,
	dsGfxFormat_W2Z10Y10X10,
	dsGfxFormat_X16,
	dsGfxFormat_X16Y16,
	dsGfxFormat_X16Y16Z16,
	dsGfxFormat_X16Y16Z16W16,
	dsGfxFormat_X32,
	dsGfxFormat_X32Y32,
	dsGfxFormat_X32Y32Z32,
	dsGfxFormat_X32Y32Z32W32,
	dsGfxFormat_X64,
	dsGfxFormat_X64Y64,
	dsGfxFormat_X64Y64Z64,
	dsGfxFormat_X64Y64Z64W64
};

static_assert(
	DS_ARRAY_SIZE(vertexElementFormatMap) == static_cast<uint32_t>(VertexElementFormat::MAX) + 1,
	"Invalid verex element format map size.");

static const dsGfxFormat formatDecorationMap[] =
{
	dsGfxFormat_UNorm,
	dsGfxFormat_SNorm,
	dsGfxFormat_UScaled,
	dsGfxFormat_SScaled,
	dsGfxFormat_UInt,
	dsGfxFormat_SInt,
	dsGfxFormat_Float,
	dsGfxFormat_UFloat,
	dsGfxFormat_SRGB
};

static_assert(
	DS_ARRAY_SIZE(formatDecorationMap) == static_cast<uint32_t>(FormatDecoration::MAX) + 1,
	"Invalid format decoration map size.");

dsGfxFormat convert(TextureFormat format, FormatDecoration decoration)
{
	auto formatIndex = static_cast<uint32_t>(format);
	auto decorationIndex = static_cast<uint32_t>(decoration);
	if (formatIndex > static_cast<uint32_t>(TextureFormat::MAX) ||
		decorationIndex > static_cast<uint32_t>(FormatDecoration::MAX))
	{
		return dsGfxFormat_Unknown;
	}

	return dsGfxFormat_decorate(textureFormatMap[formatIndex],
		formatDecorationMap[decorationIndex]);
}

dsGfxFormat convert(VertexElementFormat format, FormatDecoration decoration)
{
	auto formatIndex = static_cast<uint32_t>(format);
	auto decorationIndex = static_cast<uint32_t>(decoration);
	if (format == VertexElementFormat::Unset ||
		formatIndex > static_cast<uint32_t>(VertexElementFormat::MAX) ||
		decorationIndex > static_cast<uint32_t>(FormatDecoration::MAX))
	{
		return dsGfxFormat_Unknown;
	}

	return dsGfxFormat_decorate(vertexElementFormatMap[formatIndex],
		formatDecorationMap[decorationIndex]);
}

} // namespace DeepSeaScene
