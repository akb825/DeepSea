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

#include "Resources/MTLResourceManager.h"

#include "Resources/MTLDrawGeometry.h"
#include "Resources/MTLGfxBuffer.h"
#include "Resources/MTLGfxFence.h"
#include "Resources/MTLRenderbuffer.h"
#include "Resources/MTLTexture.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>

#include <string.h>

struct dsResourceContext
{
	int dummy;
};

static dsResourceContext dummyContext;

static void initializePixelFormats(dsMTLResourceManager* resourceManager, id<MTLDevice> device,
	MTLFeatureSet featureSet)
{
	DS_UNUSED(featureSet);
	memset(resourceManager->standardPixelFormats, 0, sizeof(resourceManager->standardPixelFormats));
	memset(resourceManager->specialPixelFormats, 0, sizeof(resourceManager->specialPixelFormats));
	memset(resourceManager->compressedPixelFormats, 0,
		sizeof(resourceManager->compressedPixelFormats));

	uint32_t unormIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UNorm);
	uint32_t snormIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SNorm);
	uint32_t uintIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UInt);
	uint32_t sintIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SInt);
	uint32_t floatIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_Float);
	uint32_t srgbIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SRGB);

#if DS_IOS
	// packed formats only available on iOS
	resourceManager->standardPixelFormats[dsGfxFormat_R4G4B4A4][unormIndex] =
		MTLPixelFormatABGR4Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R5G6B5][unormIndex] =
		MTLPixelFormatB5G6R5Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R5G5B5A1][unormIndex] =
		MTLPixelFormatA1BGR5Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_A1R5G5B5][unormIndex] =
		MTLPixelFormatBGR5A1Unorm;
#endif

	resourceManager->standardPixelFormats[dsGfxFormat_R8][unormIndex] = MTLPixelFormatR8Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8][snormIndex] = MTLPixelFormatR8Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8][uintIndex] = MTLPixelFormatR8Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R8][sintIndex] = MTLPixelFormatR8Sint;
#if DS_IOS
	resourceManager->standardPixelFormats[dsGfxFormat_R8][srgbIndex] = MTLPixelFormatR8Unorm_sRGB;
#endif

	resourceManager->standardPixelFormats[dsGfxFormat_R8G8][unormIndex] = MTLPixelFormatRG8Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8][snormIndex] = MTLPixelFormatRG8Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8][uintIndex] = MTLPixelFormatRG8Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8][sintIndex] = MTLPixelFormatRG8Sint;
#if DS_IOS
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8][srgbIndex] =
		MTLPixelFormatRG8Unorm_sRGB;
#endif

	resourceManager->standardPixelFormats[dsGfxFormat_R8G8B8A8][unormIndex] =
		MTLPixelFormatRGBA8Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8B8A8][snormIndex] =
		MTLPixelFormatRGBA8Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8B8A8][uintIndex] =
		MTLPixelFormatRGBA8Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8B8A8][sintIndex] =
		MTLPixelFormatRGBA8Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R8G8B8A8][srgbIndex] =
		MTLPixelFormatRGBA8Unorm_sRGB;

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	resourceManager->standardPixelFormats[dsGfxFormat_A2R10G10B10][unormIndex] =
		MTLPixelFormatBGR10A2Unorm;
#endif
	resourceManager->standardPixelFormats[dsGfxFormat_A2B10G10R10][unormIndex] =
		MTLPixelFormatRGB10A2Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_A2B10G10R10][uintIndex] =
		MTLPixelFormatRGB10A2Uint;

	resourceManager->standardPixelFormats[dsGfxFormat_R16][unormIndex] = MTLPixelFormatR16Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16][snormIndex] = MTLPixelFormatR16Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16][uintIndex] = MTLPixelFormatR16Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16][sintIndex] = MTLPixelFormatR16Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16][floatIndex] = MTLPixelFormatR16Float;

	resourceManager->standardPixelFormats[dsGfxFormat_R16G16][unormIndex] = MTLPixelFormatRG16Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16][snormIndex] = MTLPixelFormatRG16Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16][uintIndex] = MTLPixelFormatRG16Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16][sintIndex] = MTLPixelFormatRG16Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16][floatIndex] = MTLPixelFormatRG16Float;

	resourceManager->standardPixelFormats[dsGfxFormat_R16G16B16A16][unormIndex] =
		MTLPixelFormatRGBA16Unorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16B16A16][snormIndex] =
		MTLPixelFormatRGBA16Snorm;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16B16A16][uintIndex] =
		MTLPixelFormatRGBA16Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16B16A16][sintIndex] =
		MTLPixelFormatRGBA16Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R16G16B16A16][floatIndex] =
		MTLPixelFormatRGBA16Float;

	resourceManager->standardPixelFormats[dsGfxFormat_R32][uintIndex] = MTLPixelFormatR32Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32][sintIndex] = MTLPixelFormatR32Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32][floatIndex] = MTLPixelFormatR32Float;

	resourceManager->standardPixelFormats[dsGfxFormat_R32G32][uintIndex] = MTLPixelFormatRG32Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32G32][sintIndex] = MTLPixelFormatRG32Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32G32][floatIndex] = MTLPixelFormatRG32Float;

	resourceManager->standardPixelFormats[dsGfxFormat_R32G32B32A32][uintIndex] =
		MTLPixelFormatRGBA32Uint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32G32B32A32][sintIndex] =
		MTLPixelFormatRGBA32Sint;
	resourceManager->standardPixelFormats[dsGfxFormat_R32G32B32A32][floatIndex] =
		MTLPixelFormatRGBA32Float;

	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_B10G11R11_UFloat)] =
		MTLPixelFormatRG11B10Float;
	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_E5B9G9R9_UFloat)] =
		MTLPixelFormatRGB9E5Float;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_D16)] =
		MTLPixelFormatDepth16Unorm;
#endif
	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_D32_Float)] =
		MTLPixelFormatDepth32Float;
	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_S8)] =
		MTLPixelFormatStencil8;
#if !DS_IOS
	if (device.depth24Stencil8PixelFormatSupported)
	{
		resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_D24S8)] =
			MTLPixelFormatDepth24Unorm_Stencil8;
	}
#endif

#if !defined(IPHONE_OS_VERSION_MIN_REQUIRED) || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	resourceManager->specialPixelFormats[dsGfxFormat_specialIndex(dsGfxFormat_D32S8_Float)] =
		MTLPixelFormatDepth32Float_Stencil8;
#endif

#if DS_IOS
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC1)][unormIndex] = MTLPixelFormatETC2_RGB8;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8)][unormIndex] = MTLPixelFormatETC2_RGB8;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8)][srgbIndex] =
			MTLPixelFormatETC2_RGB8_sRGB;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8A1)][unormIndex] =
			MTLPixelFormatETC2_RGB8A1;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8)][srgbIndex] =
			MTLPixelFormatETC2_RGB8A1_sRGB;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8A8)][unormIndex] = MTLPixelFormatEAC_RGBA8;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_ETC2_R8G8B8A8)][srgbIndex] = MTLPixelFormatEAC_RGBA8;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_EAC_R11)][unormIndex] = MTLPixelFormatEAC_R11Unorm;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_EAC_R11)][snormIndex] = MTLPixelFormatEAC_R11Snorm;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_EAC_R11G11)][unormIndex] =
			MTLPixelFormatEAC_R11G11Unorm;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_EAC_R11G11)][snormIndex] =
			MTLPixelFormatEAC_R11G11Snorm;

	if (featureSet != MTLFeatureSet_iOS_GPUFamily1_v1 &&
		featureSet != MTLFeatureSet_iOS_GPUFamily1_v2 &&
		featureSet != MTLFeatureSet_iOS_GPUFamily1_v3 &&
		featureSet != MTLFeatureSet_iOS_GPUFamily1_v4 &&
		featureSet != MTLFeatureSet_iOS_GPUFamily1_v5)
	{
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_4x4)][unormIndex] =
				MTLPixelFormatASTC_4x4_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_4x4)][srgbIndex] =
				MTLPixelFormatASTC_4x4_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_5x4)][unormIndex] =
				MTLPixelFormatASTC_5x4_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_5x4)][srgbIndex] =
				MTLPixelFormatASTC_5x4_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_5x5)][unormIndex] =
				MTLPixelFormatASTC_5x5_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_5x5)][srgbIndex] =
				MTLPixelFormatASTC_5x5_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_6x5)][unormIndex] =
				MTLPixelFormatASTC_6x5_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_6x5)][srgbIndex] =
				MTLPixelFormatASTC_6x5_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_6x6)][unormIndex] =
				MTLPixelFormatASTC_6x6_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_6x6)][srgbIndex] =
				MTLPixelFormatASTC_6x6_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x5)][unormIndex] =
				MTLPixelFormatASTC_8x5_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x5)][srgbIndex] =
				MTLPixelFormatASTC_8x5_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x6)][unormIndex] =
				MTLPixelFormatASTC_8x6_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x6)][srgbIndex] =
				MTLPixelFormatASTC_8x6_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x8)][unormIndex] =
				MTLPixelFormatASTC_8x8_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_8x8)][srgbIndex] =
				MTLPixelFormatASTC_8x8_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x5)][unormIndex] =
				MTLPixelFormatASTC_10x5_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x5)][srgbIndex] =
				MTLPixelFormatASTC_10x5_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x6)][unormIndex] =
				MTLPixelFormatASTC_10x6_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x6)][srgbIndex] =
				MTLPixelFormatASTC_10x6_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x8)][unormIndex] =
				MTLPixelFormatASTC_10x8_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x8)][srgbIndex] =
				MTLPixelFormatASTC_10x8_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x10)][unormIndex] =
				MTLPixelFormatASTC_10x10_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_10x10)][srgbIndex] =
				MTLPixelFormatASTC_10x10_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_12x10)][unormIndex] =
				MTLPixelFormatASTC_12x10_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_12x10)][srgbIndex] =
				MTLPixelFormatASTC_12x10_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_12x12)][unormIndex] =
				MTLPixelFormatASTC_12x12_LDR;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_ASTC_12x12)][srgbIndex] =
				MTLPixelFormatASTC_12x12_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGB_2BPP)][unormIndex] =
				MTLPixelFormatPVRTC_RGB_2BPP;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGB_2BPP)][srgbIndex] =
				MTLPixelFormatPVRTC_RGB_2BPP_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGBA_2BPP)][unormIndex] =
				MTLPixelFormatPVRTC_RGBA_2BPP;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGBA_2BPP)][srgbIndex] =
				MTLPixelFormatPVRTC_RGBA_2BPP_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGB_4BPP)][unormIndex] =
				MTLPixelFormatPVRTC_RGB_4BPP;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGB_4BPP)][srgbIndex] =
				MTLPixelFormatPVRTC_RGB_4BPP_sRGB;

		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGBA_4BPP)][unormIndex] =
				MTLPixelFormatPVRTC_RGBA_4BPP;
		resourceManager->compressedPixelFormats
			[dsGfxFormat_specialIndex(dsGfxFormat_PVRTC1_RGBA_4BPP)][srgbIndex] =
				MTLPixelFormatPVRTC_RGBA_4BPP_sRGB;
	}
#else
	uint32_t ufloatIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UFloat);
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC1_RGB)][unormIndex] = MTLPixelFormatBC1_RGBA;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC1_RGBA)][unormIndex] = MTLPixelFormatBC1_RGBA;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC1_RGB)][srgbIndex] = MTLPixelFormatBC1_RGBA_sRGB;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC1_RGBA)][srgbIndex] = MTLPixelFormatBC1_RGBA_sRGB;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC2)][unormIndex] = MTLPixelFormatBC2_RGBA;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC2)][srgbIndex] = MTLPixelFormatBC2_RGBA_sRGB;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC3)][unormIndex] = MTLPixelFormatBC3_RGBA;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC3)][srgbIndex] = MTLPixelFormatBC3_RGBA_sRGB;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC4)][unormIndex] = MTLPixelFormatBC4_RUnorm;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC4)][snormIndex] = MTLPixelFormatBC4_RSnorm;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC5)][unormIndex] = MTLPixelFormatBC5_RGUnorm;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC5)][snormIndex] = MTLPixelFormatBC5_RGSnorm;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC6H)][floatIndex] = MTLPixelFormatBC6H_RGBFloat;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC6H)][ufloatIndex] = MTLPixelFormatBC6H_RGBUfloat;

	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC7)][unormIndex] = MTLPixelFormatBC7_RGBAUnorm;
	resourceManager->compressedPixelFormats
		[dsGfxFormat_specialIndex(dsGfxFormat_BC7)][srgbIndex] = MTLPixelFormatBC7_RGBAUnorm_sRGB;
#endif
}

static void initializeVertexFormats(dsMTLResourceManager* resourceManager)
{
	memset(resourceManager->vertexFormats, 0, sizeof(resourceManager->vertexFormats));

	uint32_t unormIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UNorm);
	uint32_t snormIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SNorm);
	uint32_t uintIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UInt);
	uint32_t sintIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SInt);
	uint32_t uscaledIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_UScaled);
	uint32_t sscaledIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_SScaled);
	uint32_t floatIndex = dsGfxFormat_decoratorIndex(dsGfxFormat_Float);

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	resourceManager->vertexFormats[dsGfxFormat_X8][unormIndex] = MTLVertexFormatUCharNormalized;
	resourceManager->vertexFormats[dsGfxFormat_X8][snormIndex] = MTLVertexFormatCharNormalized;
	resourceManager->vertexFormats[dsGfxFormat_X8][uintIndex] = MTLVertexFormatUChar;
	resourceManager->vertexFormats[dsGfxFormat_X8][sintIndex] = MTLVertexFormatChar;
	resourceManager->vertexFormats[dsGfxFormat_X8][uscaledIndex] = MTLVertexFormatUChar;
	resourceManager->vertexFormats[dsGfxFormat_X8][sscaledIndex] = MTLVertexFormatChar;
#endif

	resourceManager->vertexFormats[dsGfxFormat_X8Y8][unormIndex] = MTLVertexFormatUChar2Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8][snormIndex] = MTLVertexFormatChar2Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8][uintIndex] = MTLVertexFormatUChar2;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8][sintIndex] = MTLVertexFormatChar2;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8][uscaledIndex] = MTLVertexFormatUChar2;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8][sscaledIndex] = MTLVertexFormatChar2;

	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][unormIndex] =
		MTLVertexFormatUChar3Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][snormIndex] = MTLVertexFormatChar3Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][uintIndex] = MTLVertexFormatUChar3;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][sintIndex] = MTLVertexFormatChar3;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][uscaledIndex] = MTLVertexFormatUChar3;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8][sscaledIndex] = MTLVertexFormatChar3;

	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][unormIndex] =
		MTLVertexFormatUChar4Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][snormIndex] =
		MTLVertexFormatChar4Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][uintIndex] = MTLVertexFormatUChar4;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][sintIndex] = MTLVertexFormatChar4;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][uscaledIndex] = MTLVertexFormatUChar4;
	resourceManager->vertexFormats[dsGfxFormat_X8Y8Z8W8][sscaledIndex] = MTLVertexFormatChar4;

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	resourceManager->vertexFormats[dsGfxFormat_B8G8R8A8][unormIndex] =
		MTLVertexFormatUChar4Normalized_BGRA;
#endif

	resourceManager->vertexFormats[dsGfxFormat_W2Z10Y10X10][unormIndex] =
		MTLVertexFormatUInt1010102Normalized;
	resourceManager->vertexFormats[dsGfxFormat_W2Z10Y10X10][snormIndex] =
		MTLVertexFormatInt1010102Normalized;

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	resourceManager->vertexFormats[dsGfxFormat_X16][unormIndex] = MTLVertexFormatUShortNormalized;
	resourceManager->vertexFormats[dsGfxFormat_X16][snormIndex] = MTLVertexFormatShortNormalized;
	resourceManager->vertexFormats[dsGfxFormat_X16][uintIndex] = MTLVertexFormatUShort;
	resourceManager->vertexFormats[dsGfxFormat_X16][sintIndex] = MTLVertexFormatShort;
	resourceManager->vertexFormats[dsGfxFormat_X16][uscaledIndex] = MTLVertexFormatUShort;
	resourceManager->vertexFormats[dsGfxFormat_X16][sscaledIndex] = MTLVertexFormatShort;
	resourceManager->vertexFormats[dsGfxFormat_X16][floatIndex] = MTLVertexFormatHalf;
#endif

	resourceManager->vertexFormats[dsGfxFormat_X16Y16][unormIndex] =
		MTLVertexFormatUShort2Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][snormIndex] =
		MTLVertexFormatShort2Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][uintIndex] = MTLVertexFormatUShort2;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][sintIndex] = MTLVertexFormatShort2;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][uscaledIndex] = MTLVertexFormatUShort2;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][sscaledIndex] = MTLVertexFormatShort2;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16][floatIndex] = MTLVertexFormatHalf2;

	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][unormIndex] =
		MTLVertexFormatUShort3Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][snormIndex] =
		MTLVertexFormatShort3Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][uintIndex] = MTLVertexFormatUShort3;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][sintIndex] = MTLVertexFormatShort3;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][uscaledIndex] = MTLVertexFormatUShort3;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][sscaledIndex] = MTLVertexFormatShort3;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16][floatIndex] = MTLVertexFormatHalf3;

	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][unormIndex] =
		MTLVertexFormatUShort4Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][snormIndex] =
		MTLVertexFormatShort4Normalized;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][uintIndex] = MTLVertexFormatUShort4;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][sintIndex] = MTLVertexFormatShort4;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][uscaledIndex] = MTLVertexFormatUShort4;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][sscaledIndex] = MTLVertexFormatShort4;
	resourceManager->vertexFormats[dsGfxFormat_X16Y16Z16W16][floatIndex] = MTLVertexFormatHalf4;

	resourceManager->vertexFormats[dsGfxFormat_X32][uintIndex] = MTLVertexFormatUInt;
	resourceManager->vertexFormats[dsGfxFormat_X32][sintIndex] = MTLVertexFormatInt;
	resourceManager->vertexFormats[dsGfxFormat_X32][uscaledIndex] = MTLVertexFormatUInt;
	resourceManager->vertexFormats[dsGfxFormat_X32][sscaledIndex] = MTLVertexFormatInt;
	resourceManager->vertexFormats[dsGfxFormat_X32][floatIndex] = MTLVertexFormatFloat;

	resourceManager->vertexFormats[dsGfxFormat_X32Y32][uintIndex] = MTLVertexFormatUInt2;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32][sintIndex] = MTLVertexFormatInt2;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32][uscaledIndex] = MTLVertexFormatUInt2;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32][sscaledIndex] = MTLVertexFormatInt2;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32][floatIndex] = MTLVertexFormatFloat2;

	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32][uintIndex] = MTLVertexFormatUInt3;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32][sintIndex] = MTLVertexFormatInt3;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32][uscaledIndex] = MTLVertexFormatUInt3;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32][sscaledIndex] = MTLVertexFormatInt3;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32][floatIndex] = MTLVertexFormatFloat3;

	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32W32][uintIndex] = MTLVertexFormatUInt4;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32W32][sintIndex] = MTLVertexFormatInt4;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32W32][uscaledIndex] = MTLVertexFormatUInt4;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32W32][sscaledIndex] = MTLVertexFormatInt4;
	resourceManager->vertexFormats[dsGfxFormat_X32Y32Z32W32][floatIndex] = MTLVertexFormatFloat4;
}

static uint32_t getMinTextureBufferAlignment(dsMTLResourceManager* resourceManager,
	id<MTLDevice> device)
{
#if IPHONE_OS_VERSION_MIN_REQUIRED < 110000 && MAC_OS_X_VERSION_MIN_REQUIRED < 101300
	DS_UNUSED(resourceManager);
	DS_UNUSED(device);
#if DS_IOS
#if IPHONE_OS_VERSION_MIN_REQUIRED == 100000
	if ([device supportsFeatureSet: MTLFeatureSet_iOS_GPUFamily3_v2])
		return 16;
#endif
	return 64
#else
	return 0;
#endif
#else
	uint32_t alignment = 0;
	for (uint32_t i = 0; i < (uint32_t)dsGfxFormat_StandardCount; ++i)
	{
		for (uint32_t j = 0; j < (uint32_t)dsGfxFormat_DecoratorCount; ++j)
		{
			MTLPixelFormat format = resourceManager->standardPixelFormats[i][j];
			if (format == MTLPixelFormatInvalid)
				continue;

			alignment = dsMax(alignment,
				(uint32_t)[device minimumLinearTextureAlignmentForPixelFormat: format]);
		}
	}


	for (uint32_t i = 0; i < (uint32_t)dsGfxFormat_SpecialCount; ++i)
	{
		MTLPixelFormat format = resourceManager->specialPixelFormats[i];
		if (format == MTLPixelFormatInvalid)
			continue;

		alignment = dsMax(alignment,
			(uint32_t)[device minimumLinearTextureAlignmentForPixelFormat: format]);
	}

	for (uint32_t i = 0; i < (uint32_t)dsGfxFormat_CompressedCount; ++i)
	{
		for (uint32_t j = 0; j < (uint32_t)dsGfxFormat_DecoratorCount; ++j)
		{
			MTLPixelFormat format = resourceManager->compressedPixelFormats[i][j];
			if (format == MTLPixelFormatInvalid)
				continue;

			alignment = dsMax(alignment,
				(uint32_t)[device minimumLinearTextureAlignmentForPixelFormat: format]);
		}
	}
#endif

	return alignment;
}

static dsGfxBufferUsage getSupportedBuffers(void)
{
	dsGfxBufferUsage usage = dsGfxBufferUsage_Index | dsGfxBufferUsage_Vertex |
		dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch |
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer |
		dsGfxBufferUsage_CopyFrom | dsGfxBufferUsage_CopyTo;
#if DS_IOS || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	usage |= dsGfxBufferUsage_Image | dsGfxBufferUsage_MutableImage;
#endif
	return usage;
}

static size_t getMaxBufferLength(id<MTLDevice> device)
{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000 || MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
	return device.maxBufferLength;
#else
#if DS_IOS || MAC_OS_X_VERSION_MIN_REQUIRED == 101100
	return 256*1024*1024;
#else
	return 1024*1024*1024;
#endif
#endif
}

static uint32_t getMaxTextureSize(MTLFeatureSet feature)
{
	switch (feature)
	{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 80000
		case MTLFeatureSet_iOS_GPUFamily1_v1:
		case MTLFeatureSet_iOS_GPUFamily2_v1:
			return 4096;
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
		case MTLFeatureSet_iOS_GPUFamily1_v2:
		case MTLFeatureSet_iOS_GPUFamily2_v2:
			return 8192;
		case MTLFeatureSet_iOS_GPUFamily3_v1:
			return 16384;
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
		case MTLFeatureSet_iOS_GPUFamily1_v3:
		case MTLFeatureSet_iOS_GPUFamily2_v3:
			return 8192;
		case MTLFeatureSet_iOS_GPUFamily3_v2:
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		case MTLFeatureSet_iOS_GPUFamily1_v4:
		case MTLFeatureSet_iOS_GPUFamily2_v4:
			return 8192;
		case MTLFeatureSet_iOS_GPUFamily3_v3:
		case MTLFeatureSet_iOS_GPUFamily4_v1:
			return 16384;
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
		case MTLFeatureSet_iOS_GPUFamily1_v5:
		case MTLFeatureSet_iOS_GPUFamily2_v5:
			return 8192;
		case MTLFeatureSet_iOS_GPUFamily3_v4:
		case MTLFeatureSet_iOS_GPUFamily4_v2:
		case MTLFeatureSet_iOS_GPUFamily5_v1:
			return 16384;
#endif

		default:
			return 16384;
	}
}

static bool hasLayeredRendering(MTLFeatureSet feature)
{
#if DS_IOS
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
	switch (feature)
	{
		case MTLFeatureSet_iOS_GPUFamily5_v1:
			return true;
		default:
			return false;
	}
#else
	DS_UNUSED(feature);
	return false;
#endif
#else
	DS_UNUSED(feature);
	return true;
#endif
}

static bool hasCubeArrays(MTLFeatureSet feature)
{
#if DS_IOS
	switch (feature)
	{
#if IPHONE_OS_VERSION_MIN_REQUIRED >= 110000
		case MTLFeatureSet_iOS_GPUFamily4_v1:
			return true;
#endif

#if IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
		case MTLFeatureSet_iOS_GPUFamily4_v2:
		case MTLFeatureSet_iOS_GPUFamily5_v1:
			return true;
#endif

		default:
			return false;
	}
#else
	DS_UNUSED(feature);
	return true;
#endif
}

bool dsMTLResourceManager_vertexFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
	return mtlResourceManager->vertexFormats[standardIndex][decoratorIndex] !=
		MTLVertexFormatInvalid;
}

bool dsMTLResourceManager_textureFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	if (standardIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->standardPixelFormats[standardIndex][decoratorIndex] !=
			MTLVertexFormatInvalid;
	}

	uint32_t specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
		return mtlResourceManager->specialPixelFormats[specialIndex] != MTLVertexFormatInvalid;

	uint32_t compressedIndex = dsGfxFormat_compressedIndex(format);
	if (compressedIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->compressedPixelFormats[compressedIndex][decoratorIndex] !=
			MTLVertexFormatInvalid;
	}

	return false;
}

bool dsMTLResourceManager_offscreenFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	if (standardIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->standardPixelFormats[standardIndex][decoratorIndex] !=
			MTLVertexFormatInvalid;
	}

	uint32_t specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
		return mtlResourceManager->specialPixelFormats[specialIndex] != MTLVertexFormatInvalid;

	return false;
}

bool dsMTLResourceManager_generateMipmapFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	if (standardIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->standardPixelFormats[standardIndex][decoratorIndex] !=
			MTLVertexFormatInvalid;
	}

	return false;
}

bool dsMTLResourceManager_textureCopyFormatsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat)
{
	if (!dsMTLResourceManager_textureFormatSupported(resourceManager, srcFormat) ||
		!dsMTLResourceManager_textureFormatSupported(resourceManager, dstFormat) ||
		dsGfxFormat_size(srcFormat) != dsGfxFormat_size(dstFormat))
	{
		return false;
	}

	uint32_t srcCompressedIndex = dsGfxFormat_compressedIndex(srcFormat);
	uint32_t dstCompressedIndex = dsGfxFormat_compressedIndex(dstFormat);
	if (srcCompressedIndex > 0 || dstCompressedIndex > 0)
	{
		if (srcCompressedIndex != dstCompressedIndex)
			return false;

		dsGfxFormat srcDecorator = srcFormat & dsGfxFormat_DecoratorMask;
		dsGfxFormat dstDecorator = srcFormat & dsGfxFormat_DecoratorMask;
		return srcDecorator == dstDecorator ||
			(srcDecorator == dsGfxFormat_SRGB && srcDecorator == dsGfxFormat_UNorm) ||
			(srcDecorator == dsGfxFormat_UNorm && srcDecorator == dsGfxFormat_SRGB);
	}

	return true;
}

dsResourceContext* dsMTLResourceManager_createResourceContext(dsResourceManager* resourceManager)
{
	DS_UNUSED(resourceManager);
	return &dummyContext;
}

bool dsMTLResourceManager_destroyResourceContext(dsResourceManager* resourceManager,
	dsResourceContext* context)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(context);
	return true;
}

dsResourceManager* dsMTLResourceManager_create(dsAllocator* allocator, dsRenderer* renderer)
{
	dsMTLResourceManager* resourceManager = DS_ALLOCATE_OBJECT(allocator, dsMTLResourceManager);
	if (!resourceManager)
		return NULL;

	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)(mtlRenderer->device);

	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	DS_VERIFY(dsResourceManager_initialize(baseResourceManager));
	baseResourceManager->allocator = dsAllocator_keepPointer(allocator);
	baseResourceManager->renderer = renderer;
	baseResourceManager->maxResourceContexts = UINT_MAX;
	baseResourceManager->minNonCoherentMappingAlignment = 0;

	initializePixelFormats(resourceManager, device, mtlRenderer->featureSet);
	initializeVertexFormats(resourceManager);

	baseResourceManager->minTextureBufferAlignment = getMinTextureBufferAlignment(resourceManager,
		device);
#if DS_IOS
	baseResourceManager->minUniformBlockAlignment = 4;
	baseResourceManager->minUniformBufferAlignment = 4;
#else
	baseResourceManager->minUniformBlockAlignment = 256;
	baseResourceManager->minUniformBufferAlignment = 256;
#endif

	baseResourceManager->supportedBuffers = getSupportedBuffers();
	baseResourceManager->bufferMapSupport = dsGfxBufferMapSupport_Full;
	baseResourceManager->canCopyBuffers = true;
	baseResourceManager->hasTextureBufferSubrange = true;
	baseResourceManager->maxIndexSize = sizeof(uint32_t);
	baseResourceManager->maxUniformBlockSize = getMaxBufferLength(device);
	baseResourceManager->maxVertexAttribs = 31;
	baseResourceManager->maxSamplers = 16;
	baseResourceManager->maxVertexSamplers = 16;
	baseResourceManager->lineWidthRange.x = 1.0f;
	baseResourceManager->lineWidthRange.y = 1.0f;

	baseResourceManager->maxTextureSize = getMaxTextureSize(mtlRenderer->featureSet);
	baseResourceManager->maxTextureBufferElements =
		DS_IMAGE_BUFFER_WIDTH*baseResourceManager->maxTextureSize;
	baseResourceManager->maxTextureDepth = 2048;
	baseResourceManager->maxTextureArrayLevels = 2048;
	baseResourceManager->maxRenderbufferSize = baseResourceManager->maxTextureSize;
	baseResourceManager->maxFramebufferLayers =
		hasLayeredRendering(mtlRenderer->featureSet) ? 256 : 1;
	baseResourceManager->maxTextureSamples = renderer->maxSurfaceSamples;
	baseResourceManager->hasArbitraryMipmapping = true;
	baseResourceManager->hasCubeArrays = hasCubeArrays(mtlRenderer->featureSet);
	baseResourceManager->texturesReadable = true;
	baseResourceManager->requiresColorBuffer = false;
	baseResourceManager->requiresAnySurface = false;
	baseResourceManager->canMixWithRenderSurface = true;
	baseResourceManager->hasVertexPipelineWrites = true;
	baseResourceManager->hasFragmentWrites = true;
	baseResourceManager->hasFences = true;

	/*
	 * Disable Query support for Metal:
	 * Timer queries aren't natively supported. MoltenVK emulates timer queries by getting the CPU
	 * time in the submit complete handler for a command buffer. This is both a rough approximation
	 * and requires a flush of the command buffer. Some versions of iOS support getting the actual
	 * GPU time, but it's still in the submit complete handler, so it would still need a flush.
	 *
	 * Occlusion queries are very limited in how they can be used. A buffer must be set with the
	 * render pass, then only one element of that buffer can be used without stopping and
	 * re-starting the render pass. It would be very difficult to support the full range of uses of
	 * queries (i.e. both within and without of render passes and across multiple command buffers),
	 * and may make some optimizations like memoryless buffers impossible. (due to breaking if you
	 * stop/restart the render pass) Based on looking at the implementation, even MoltenVK appears
	 * to only support a subset of usage properly. (starting a query before a render pass and ending
	 * it after)
	 *
	 * Perhaps in the future some subset of queries will be supported, but until a compelling use
	 * case (that can also potentially be optimized for) comes up it's better to just not
	 */
	baseResourceManager->hasQueries = false;
	baseResourceManager->has64BitQueries = true;
	baseResourceManager->hasQueryBuffers = true;
	baseResourceManager->timestampPeriod = 0.0f;

	// Core functionality
	baseResourceManager->vertexFormatSupportedFunc = &dsMTLResourceManager_vertexFormatSupported;
	baseResourceManager->textureFormatSupportedFunc = &dsMTLResourceManager_textureFormatSupported;
	baseResourceManager->offscreenFormatSupportedFunc =
		&dsMTLResourceManager_offscreenFormatSupported;
#if DS_IOS || MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	baseResourceManager->textureBufferFormatSupportedFunc =
		&dsMTLResourceManager_textureFormatSupported;
#endif
	baseResourceManager->generateMipmapFormatSupportedFunc =
		&dsMTLResourceManager_generateMipmapFormatSupported;
	baseResourceManager->textureCopyFormatsSupportedFunc =
		&dsMTLResourceManager_textureCopyFormatsSupported;
	// NOTE: Blitting between textures isn't supported. Can potentially use an offscreen in the
	// future if *really* needed, though ther emay be some restrictions.
	baseResourceManager->createResourceContextFunc = &dsMTLResourceManager_createResourceContext;
	baseResourceManager->destroyResourceContextFunc = &dsMTLResourceManager_destroyResourceContext;

	// Buffers
	baseResourceManager->createBufferFunc = &dsMTLGfxBuffer_create;
	baseResourceManager->destroyBufferFunc = &dsMTLGfxBuffer_destroy;
	baseResourceManager->mapBufferFunc = &dsMTLGfxBuffer_map;
	baseResourceManager->unmapBufferFunc = &dsMTLGfxBuffer_unmap;
	baseResourceManager->flushBufferFunc = &dsMTLGfxBuffer_flush;
	baseResourceManager->invalidateBufferFunc = &dsMTLGfxBuffer_invalidate;
	baseResourceManager->copyBufferDataFunc = &dsMTLGfxBuffer_copyData;
	baseResourceManager->copyBufferFunc = &dsMTLGfxBuffer_copy;

	// Draw geometry.
	baseResourceManager->createGeometryFunc = &dsMTLDrawGeometry_create;
	baseResourceManager->destroyGeometryFunc = &dsMTLDrawGeometry_destroy;

	// Textures
	baseResourceManager->createTextureFunc = &dsMTLTexture_create;
	baseResourceManager->createOffscreenFunc = &dsMTLTexture_createOffscreen;
	baseResourceManager->destroyTextureFunc = &dsMTLTexture_destroy;
	baseResourceManager->copyTextureDataFunc = &dsMTLTexture_copyData;
	baseResourceManager->copyTextureFunc = &dsMTLTexture_copy;
	baseResourceManager->generateTextureMipmapsFunc = &dsMTLTexture_generateMipmaps;
	baseResourceManager->getTextureDataFunc = &dsMTLTexture_getData;
	baseResourceManager->processTextureFunc = &dsMTLTexture_process;

	// Renderbuffers
	baseResourceManager->createRenderbufferFunc = &dsMTLRenderbuffer_create;
	baseResourceManager->destroyRenderbufferFunc = &dsMTLRenderbuffer_destroy;

	// Fences
	baseResourceManager->createFenceFunc = &dsMTLGfxFence_create;
	baseResourceManager->destroyFenceFunc = &dsMTLGfxFence_destroy;
	baseResourceManager->setFencesFunc = &dsMTLGfxFence_set;
	baseResourceManager->waitFenceFunc = &dsMTLGfxFence_wait;
	baseResourceManager->resetFenceFunc = &dsMTLGfxFence_reset;

	return baseResourceManager;
}

void dsMTLResourceManager_destroy(dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsResourceManager_shutdown(resourceManager);
	DS_VERIFY(dsAllocator_free(resourceManager->allocator, resourceManager));
}

MTLPixelFormat dsMTLResourceManager_getPixelFormat(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	if (standardIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->standardPixelFormats[standardIndex][decoratorIndex];
	}

	uint32_t specialIndex = dsGfxFormat_specialIndex(format);
	if (specialIndex > 0)
		return mtlResourceManager->specialPixelFormats[specialIndex];

	uint32_t compressedIndex = dsGfxFormat_compressedIndex(format);
	if (compressedIndex > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		return mtlResourceManager->compressedPixelFormats[compressedIndex][decoratorIndex];
	}

	return MTLPixelFormatInvalid;
}

MTLVertexFormat dsMTLResourceManager_getVertexFormat(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsMTLResourceManager* mtlResourceManager = (const dsMTLResourceManager*)resourceManager;
	uint32_t standardIndex = dsGfxFormat_standardIndex(format);
	uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
	return mtlResourceManager->vertexFormats[standardIndex][decoratorIndex];
}
