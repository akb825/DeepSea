/*
 * Copyright 2019-2021 Aaron Barany
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

#include "MTLShared.h"

#include "Resources/MTLResourceManager.h"

MTLCompareFunction dsGetMTLCompareFunction(mslCompareOp compare, MTLCompareFunction defaultVal)
{
	switch (compare)
	{
		case mslCompareOp_Never:
			return MTLCompareFunctionNever;
		case mslCompareOp_Less:
			return MTLCompareFunctionLess;
		case mslCompareOp_Equal:
			return MTLCompareFunctionEqual;
		case mslCompareOp_LessOrEqual:
			return MTLCompareFunctionLessEqual;
		case mslCompareOp_Greater:
			return MTLCompareFunctionGreater;
		case mslCompareOp_NotEqual:
			return MTLCompareFunctionNotEqual;
		case mslCompareOp_GreaterOrEqual:
			return MTLCompareFunctionGreaterEqual;
		case mslCompareOp_Always:
			return MTLCompareFunctionAlways;
		default:
			return defaultVal;
	}
}

MTLStencilOperation dsGetMTLStencilOp(mslStencilOp op)
{
	switch (op)
	{
		case mslStencilOp_Zero:
			return MTLStencilOperationZero;
		case mslStencilOp_Replace:
			return MTLStencilOperationReplace;
		case mslStencilOp_IncrementAndClamp:
			return MTLStencilOperationIncrementClamp;
		case mslStencilOp_DecrementAndClamp:
			return MTLStencilOperationDecrementClamp;
		case mslStencilOp_Invert:
			return MTLStencilOperationInvert;
		case mslStencilOp_IncrementAndWrap:
			return MTLStencilOperationIncrementWrap;
		case mslStencilOp_DecrementAndWrap:
			return MTLStencilOperationDecrementWrap;
		case mslStencilOp_Keep:
		default:
			return MTLStencilOperationKeep;
	}
}

MTLStencilDescriptor* dsCreateMTLStencilDescriptor(const mslStencilOpState* state,
	uint32_t compareMask, uint32_t writeMask)
{
	MTLStencilDescriptor* descriptor = [MTLStencilDescriptor new];
	if (!descriptor)
		return NULL;

	descriptor.stencilFailureOperation = dsGetMTLStencilOp(state->failOp);
	descriptor.depthFailureOperation = dsGetMTLStencilOp(state->depthFailOp);
	descriptor.depthStencilPassOperation = dsGetMTLStencilOp(state->passOp);
	descriptor.stencilCompareFunction = dsGetMTLCompareFunction(state->compareOp,
		MTLCompareFunctionAlways);
	descriptor.readMask = state->compareMask == MSL_UNKNOWN ? compareMask : state->compareMask;
	descriptor.writeMask = state->writeMask == MSL_UNKNOWN ? writeMask : state->writeMask;
	return descriptor;
}

MTLClearColor dsGetMTLClearColor(dsGfxFormat format, const dsSurfaceColorValue* value)
{
	if ((format & dsGfxFormat_DecoratorMask) == dsGfxFormat_UInt)
	{
		return MTLClearColorMake(value->uintValue[0], value->uintValue[1], value->uintValue[2],
			value->uintValue[3]);
	}
	else if ((format & dsGfxFormat_DecoratorMask) == dsGfxFormat_SInt)
	{
		return MTLClearColorMake(value->intValue[0], value->intValue[1], value->intValue[2],
			value->intValue[3]);
	}
	else
	{
		return MTLClearColorMake(value->floatValue.r, value->floatValue.g, value->floatValue.b,
			value->floatValue.a);
	}
}

bool dsIsMTLFormatPVR(dsGfxFormat format)
{
	dsGfxFormat compressedFormat = format & dsGfxFormat_CompressedMask;
	return compressedFormat >= dsGfxFormat_PVRTC1_RGB_2BPP &&
		compressedFormat <= dsGfxFormat_PVRTC2_RGBA_4BPP;
}

MTLPixelFormat dsGetMTLDepthFormat(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
		case dsGfxFormat_D32_Float:
		case dsGfxFormat_D24S8:
			return dsMTLResourceManager_getPixelFormat(resourceManager, format);
		case dsGfxFormat_D16S8:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, dsGfxFormat_D16);
			return pixelFormat;
		}
		case dsGfxFormat_D32S8_Float:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
			{
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
					dsGfxFormat_D32_Float);
			}
			return pixelFormat;
		}
		default:
			return MTLPixelFormatInvalid;
	}
}

MTLPixelFormat dsGetMTLStencilFormat(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_S8:
		case dsGfxFormat_D24S8:
			return dsMTLResourceManager_getPixelFormat(resourceManager, format);
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D32S8_Float:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, dsGfxFormat_S8);
			return pixelFormat;
		}
		default:
			return MTLPixelFormatInvalid;
	}
}
