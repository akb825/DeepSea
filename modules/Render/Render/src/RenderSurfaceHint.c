/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Render/RenderSurfaceHint.h>

#include <DeepSea/Core/Error.h>

#include <DeepSea/Render/Resources/GfxFormat.h>

bool dsRenderSurfaceHint_default(dsRenderSurfaceHint* hint)
{
	if (!hint)
	{
		errno = EINVAL;
		return false;
	}

	hint->redBits = 8;
	hint->greenBits = 8;
	hint->blueBits = 8;
	hint->alphaBits = 0;
	hint->depthBits = 24;
	hint->stencilBits = 8;
	hint->colorSpace = dsRenderColorSpace_NonLinearSRGB;
	hint->forcedColorFormat = dsGfxFormat_Unknown;
	hint->forcedDepthStencilFormat = dsGfxFormat_Unknown;
	return true;
}

bool dsRenderSurfaceHint_fromFormats(dsRenderSurfaceHint* hint,
	dsGfxFormat colorFormat, dsGfxFormat depthStencilFormat, dsRenderColorSpace colorSpace,
	bool explicit)
{
	if (!hint || colorSpace < dsRenderColorSpace_NonLinearSRGB ||
		colorSpace > dsRenderColorSpace_Rec2100PQ)
	{
		errno = EINVAL;
		return false;
	}

	switch (colorFormat & dsGfxFormat_StandardMask)
	{
		case dsGfxFormat_R4G4:
			hint->redBits = 4;
			hint->greenBits = 4;
			hint->blueBits = 0;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R4G4B4A4:
		case dsGfxFormat_B4G4R4A4:
		case dsGfxFormat_A4R4G4B4:
			hint->redBits = 4;
			hint->greenBits = 4;
			hint->blueBits = 4;
			hint->alphaBits = 4;
			break;
		case dsGfxFormat_R5G6B5:
		case dsGfxFormat_B5G6R5:
			hint->redBits = 5;
			hint->greenBits = 6;
			hint->blueBits = 5;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R5G5B5A1:
		case dsGfxFormat_B5G5R5A1:
		case dsGfxFormat_A1R5G5B5:
			hint->redBits = 5;
			hint->greenBits = 5;
			hint->blueBits = 5;
			hint->alphaBits = 1;
			break;
		case dsGfxFormat_R8:
			hint->redBits = 8;
			hint->greenBits = 0;
			hint->blueBits = 0;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R8G8:
			hint->redBits = 8;
			hint->greenBits = 8;
			hint->blueBits = 0;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R8G8B8:
		case dsGfxFormat_B8G8R8:
			hint->redBits = 8;
			hint->greenBits = 8;
			hint->blueBits = 8;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R8G8B8A8:
		case dsGfxFormat_B8G8R8A8:
		case dsGfxFormat_A8B8G8R8:
			hint->redBits = 8;
			hint->greenBits = 8;
			hint->blueBits = 8;
			hint->alphaBits = 8;
			break;
		case dsGfxFormat_A2R10G10B10:
		case dsGfxFormat_A2B10G10R10:
			hint->redBits = 10;
			hint->greenBits = 10;
			hint->blueBits = 10;
			hint->alphaBits = 2;
			break;
		case dsGfxFormat_R16:
			hint->redBits = 16;
			hint->greenBits = 0;
			hint->blueBits = 0;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R16G16:
			hint->redBits = 16;
			hint->greenBits = 16;
			hint->blueBits = 0;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R16G16B16:
			hint->redBits = 16;
			hint->greenBits = 16;
			hint->blueBits = 16;
			hint->alphaBits = 0;
			break;
		case dsGfxFormat_R16G16B16A16:
			hint->redBits = 16;
			hint->greenBits = 16;
			hint->blueBits = 16;
			hint->alphaBits = 16;
			break;
		default:
			// 32 and 64 bit formats cannot be used for render surfaces.
			errno = EINVAL;
			return false;
	}

	switch (depthStencilFormat)
	{
		case dsGfxFormat_Unknown:
			hint->depthBits = 0;
			hint->stencilBits = 0;
			break;
		case dsGfxFormat_D16:
			hint->depthBits = 16;
			hint->stencilBits = 0;
			break;
		case dsGfxFormat_X8D24:
			hint->depthBits = 24;
			hint->stencilBits = 0;
			break;
		case dsGfxFormat_D32_Float:
			hint->depthBits = 32;
			hint->stencilBits = 0;
			break;
		case dsGfxFormat_S8:
			hint->depthBits = 0;
			hint->stencilBits = 8;
			break;
		case dsGfxFormat_D16S8:
			hint->depthBits = 16;
			hint->stencilBits = 8;
			break;
		case dsGfxFormat_D24S8:
			hint->depthBits = 24;
			hint->stencilBits = 8;
			break;
		case dsGfxFormat_D32S8_Float:
			hint->depthBits = 32;
			hint->stencilBits = 8;
			break;
		default:
			errno = EINVAL;
			return false;
	}

	hint->colorSpace = colorSpace;
	if (explicit)
	{
		hint->forcedColorFormat = colorFormat;
		hint->forcedDepthStencilFormat = depthStencilFormat;
	}
	else
	{
		hint->forcedColorFormat = dsGfxFormat_Unknown;
		hint->forcedDepthStencilFormat = dsGfxFormat_Unknown;
	}

	return true;
}

bool dsRenderSurfaceHint_isValid(const dsRenderSurfaceHint* hint)
{
	return dsRenderSurfaceHint_colorFormat(hint, false, false) != dsGfxFormat_Unknown;
}

dsGfxFormat dsRenderSurfaceHint_colorFormat(
	const dsRenderSurfaceHint* hint, bool bgr, bool aligned)
{
	if (!hint)
		return dsGfxFormat_Unknown;

	if (hint->forcedColorFormat != dsGfxFormat_Unknown)
		return hint->forcedColorFormat;

	if (hint->redBits <= 5 && hint->greenBits <= 6 && hint->blueBits <= 5 && hint->alphaBits == 0)
	{
		if (hint->colorSpace == dsRenderColorSpace_NonLinearSRGBConverting)
			return dsGfxFormat_Unknown;
		if (bgr)
			return dsGfxFormat_decorate(dsGfxFormat_B5G6R5, dsGfxFormat_UNorm);
		return dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm);
	}

	if (hint->redBits <= 8 && hint->greenBits <= 8 && hint->blueBits <= 8 && hint->alphaBits <= 8)
	{
		dsGfxFormat decorator = hint->colorSpace == dsRenderColorSpace_NonLinearSRGBConverting ?
			dsGfxFormat_SRGB : dsGfxFormat_UNorm;
		if (hint->alphaBits != 0 || aligned)
		{
			if (bgr)
				return dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, decorator);
			return dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, decorator);
		}

		if (bgr)
			return dsGfxFormat_decorate(dsGfxFormat_B8G8R8, decorator);
		return dsGfxFormat_decorate(dsGfxFormat_R8G8B8, decorator);
	}

	if (hint->redBits <= 10 && hint->greenBits <= 10 && hint->blueBits <= 10 &&
		hint->alphaBits <= 2)
	{
		if (hint->colorSpace == dsRenderColorSpace_NonLinearSRGBConverting)
			return dsGfxFormat_Unknown;
		if (bgr)
			return dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10, dsGfxFormat_UNorm);
		return dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10, dsGfxFormat_UNorm);
	}

	if (hint->redBits <= 16 && hint->greenBits <= 16 && hint->blueBits <= 16 &&
		hint->alphaBits <= 16)
	{
		if (hint->colorSpace == dsRenderColorSpace_NonLinearSRGBConverting)
			return dsGfxFormat_Unknown;
		return dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float);
	}

	return dsGfxFormat_Unknown;
}

dsGfxFormat dsRenderSurfaceHint_depthStencilFormat(const dsRenderSurfaceHint* hint)
{
	if (!hint || (hint->depthBits == 0 && hint->stencilBits == 0))
		return dsGfxFormat_Unknown;

	if (hint->depthBits == 0 && hint->stencilBits <= 8)
		return dsGfxFormat_S8;
	if (hint->depthBits <= 16 && hint->stencilBits == 0)
		return dsGfxFormat_D16;
	if (hint->depthBits <= 16 && hint->stencilBits <= 8)
		return dsGfxFormat_D16S8;
	if (hint->depthBits <= 24 && hint->stencilBits == 0)
		return dsGfxFormat_X8D24;
	if (hint->depthBits <= 24 && hint->stencilBits <= 8)
		return dsGfxFormat_D24S8;
	if (hint->depthBits <= 32 && hint->stencilBits == 0)
		return dsGfxFormat_D32_Float;
	if (hint->depthBits <= 32 && hint->stencilBits <= 8)
		return dsGfxFormat_D32S8_Float;

	return dsGfxFormat_Unknown;
}
