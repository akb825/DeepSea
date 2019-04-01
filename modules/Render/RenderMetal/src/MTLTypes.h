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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Types.h>

#import <Foundation/NSObject.h>
#import <Metal/MTLArgument.h>
#import <Metal/MTLDevice.h>
#import <Metal/MTLPixelFormat.h>
#import <Metal/MTLVertexDescriptor.h>

typedef struct dsMTLResourceManager
{
	dsResourceManager resourceManager;

	MTLPixelFormat standardPixelFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	MTLPixelFormat specialPixelFormats[dsGfxFormat_SpecialCount];
	MTLPixelFormat compressedPixelFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	MTLVertexFormat vertexFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
} dsMTLResourceManager;

typedef struct dsMTLRenderer
{
	dsRenderer renderer;

	CFTypeRef mtlDevice;
	MTLFeatureSet featureSet;
} dsMTLRenderer;
