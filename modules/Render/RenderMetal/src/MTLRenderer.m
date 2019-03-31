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

#include <DeepSea/RenderMetal/MTLRenderer.h>
#include "MTLRendererInternal.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

#include <Metal/MTLDevice.h>
#include <Metal/MTLSampler.h>
#include <string.h>

static dsMTLFeature mtlFeatures[] =
{
#if DS_IOS
	dsMTLFeature_iOS_GPUFamily1_v1,
	dsMTLFeature_iOS_GPUFamily2_v1,

	dsMTLFeature_iOS_GPUFamily1_v2,
	dsMTLFeature_iOS_GPUFamily2_v2,
	dsMTLFeature_iOS_GPUFamily3_v1,

	dsMTLFeature_iOS_GPUFamily1_v3,
	dsMTLFeature_iOS_GPUFamily2_v3,
	dsMTLFeature_iOS_GPUFamily3_v2,

	dsMTLFeature_iOS_GPUFamily1_v4,
	dsMTLFeature_iOS_GPUFamily2_v4,
	dsMTLFeature_iOS_GPUFamily3_v3,
	dsMTLFeature_iOS_GPUFamily4_v1,

	dsMTLFeature_iOS_GPUFamily1_v5,
	dsMTLFeature_iOS_GPUFamily2_v5,
	dsMTLFeature_iOS_GPUFamily3_v4,
	dsMTLFeature_iOS_GPUFamily4_v2,
#else
	dsMTLFeature_macOS_GPUFamily1_v1,

	dsMTLFeature_macOS_GPUFamily1_v2,

	dsMTLFeature_macOS_GPUFamily1_v3,

	dsMTLFeature_macOS_GPUFamily1_v4,
	dsMTLFeature_macOS_GPUFamily2_v1,
#endif
};

static size_t dsMTLRenderer_fullAllocSize(size_t deviceNameLen)
{
	return DS_ALIGNED_SIZE(sizeof(dsMTLRenderer)) + DS_ALIGNED_SIZE(deviceNameLen + 1);
}

static uint32_t getShaderVersion(dsMTLFeature feature)
{
	switch (feature)
	{
		case dsMTLFeature_iOS_GPUFamily1_v1:
		case dsMTLFeature_iOS_GPUFamily2_v1:
			return DS_ENCODE_VERSION(1, 0, 0);
		case dsMTLFeature_iOS_GPUFamily1_v2:
		case dsMTLFeature_iOS_GPUFamily2_v2:
		case dsMTLFeature_iOS_GPUFamily3_v1:
			return DS_ENCODE_VERSION(1, 1, 0);
		case dsMTLFeature_iOS_GPUFamily1_v3:
		case dsMTLFeature_iOS_GPUFamily2_v3:
		case dsMTLFeature_iOS_GPUFamily3_v2:
			return DS_ENCODE_VERSION(1, 2, 0);
		case dsMTLFeature_iOS_GPUFamily1_v4:
		case dsMTLFeature_iOS_GPUFamily2_v4:
		case dsMTLFeature_iOS_GPUFamily3_v3:
		case dsMTLFeature_iOS_GPUFamily4_v1:
			return DS_ENCODE_VERSION(2, 0, 0);
		case dsMTLFeature_iOS_GPUFamily1_v5:
		case dsMTLFeature_iOS_GPUFamily2_v5:
		case dsMTLFeature_iOS_GPUFamily3_v4:
		case dsMTLFeature_iOS_GPUFamily4_v2:
			return DS_ENCODE_VERSION(2, 1, 0);
		case dsMTLFeature_macOS_GPUFamily1_v1:
			return DS_ENCODE_VERSION(1, 1, 0);
		case dsMTLFeature_macOS_GPUFamily1_v2:
			return DS_ENCODE_VERSION(1, 2, 0);
		case dsMTLFeature_macOS_GPUFamily1_v3:
			return DS_ENCODE_VERSION(2, 0, 0);
		case dsMTLFeature_macOS_GPUFamily1_v4:
		case dsMTLFeature_macOS_GPUFamily2_v1:
			return DS_ENCODE_VERSION(2, 1, 0);
	}
	DS_ASSERT(false);
	return 0;
}

static uint32_t getMaxColorAttachments(dsMTLFeature feature)
{
	switch (feature)
	{
		case dsMTLFeature_iOS_GPUFamily1_v1:
		case dsMTLFeature_iOS_GPUFamily1_v2:
		case dsMTLFeature_iOS_GPUFamily1_v3:
		case dsMTLFeature_iOS_GPUFamily1_v4:
		case dsMTLFeature_iOS_GPUFamily1_v5:
			return 4;
		default:
			return 8;
	}
}

static uint32_t getMaxSurfaceSamples(id<MTLDevice> device)
{
	if ([device supportsTextureSampleCount: 8])
		return 8;
	return 4;
}

static uint32_t hasTessellationShaders(dsMTLFeature feature)
{
	switch (feature)
	{
		case dsMTLFeature_iOS_GPUFamily1_v1:
		case dsMTLFeature_iOS_GPUFamily2_v1:
		case dsMTLFeature_iOS_GPUFamily1_v2:
		case dsMTLFeature_iOS_GPUFamily2_v2:
		case dsMTLFeature_iOS_GPUFamily3_v1:
		case dsMTLFeature_iOS_GPUFamily1_v3:
		case dsMTLFeature_iOS_GPUFamily2_v3:
		case dsMTLFeature_iOS_GPUFamily1_v4:
		case dsMTLFeature_iOS_GPUFamily2_v4:
		case dsMTLFeature_iOS_GPUFamily1_v5:
		case dsMTLFeature_iOS_GPUFamily2_v5:
		case dsMTLFeature_macOS_GPUFamily1_v1:
			return false;
		default:
			return true;
	}
}

bool dsMTLRenderer_isSupported(void)
{
	return [MTLCopyAllDevices() count] > 0;
}

bool dsMTLRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	DS_UNUSED(outDevices);
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	*outDeviceCount = 0;
	return true;
}

dsRenderer* dsMTLRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Renderer allocator must support freeing memory.");
		return NULL;
	}

	dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options, false, false);
	if (!dsGfxFormat_isValid(colorFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Invalid color format.");
		return NULL;
	}

	dsGfxFormat depthFormat = dsRenderer_optionsDepthFormat(options);

	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	if (!device)
	{
		errno = EAGAIN;
		DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Couldn't create Metal device.");
		return NULL;
	}

	const char* deviceName = [device.name UTF8String];
	size_t deviceNameLen = strlen(deviceName);

	size_t bufferSize = dsMTLRenderer_fullAllocSize(deviceNameLen);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsMTLRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsMTLRenderer);
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	renderer->mtlDevice = CFBridgingRetain(device);
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(mtlFeatures); ++i)
	{
		if ([device supportsFeatureSet:(MTLFeatureSet)mtlFeatures[i]])
			renderer->featureSet = mtlFeatures[i];
	}

	baseRenderer->allocator = allocator;
	//baseRenderer->resourceManager = ...;
	//baseRenderer->mainCommandBuffer = ...;

	baseRenderer->platform = dsGfxPlatform_Default;
	baseRenderer->rendererID = DS_MTL_RENDERER_ID;
	baseRenderer->platformID = 0;
	baseRenderer->name = "Metal";
#if DS_IOS
	baseRenderer->shaderLanguage = "metal-ios";
#else
	baseRenderer->shaderLanguage = "metal-osx";
#endif

	char* deviceNameCopy = (char*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, deviceNameLen + 1);
	DS_ASSERT(deviceNameCopy);
	memcpy(deviceNameCopy, deviceName, deviceNameLen + 1);
	baseRenderer->deviceName = deviceNameCopy;
	baseRenderer->shaderVersion = getShaderVersion(renderer->featureSet);
	baseRenderer->vendorID = 0;
	baseRenderer->deviceID = 0;
	baseRenderer->driverVersion = 0;

	baseRenderer->maxColorAttachments = getMaxColorAttachments(renderer->featureSet);
	baseRenderer->maxSurfaceSamples = getMaxSurfaceSamples(device);
	baseRenderer->maxAnisotropy = 16.0f;
	baseRenderer->surfaceColorFormat = colorFormat;
	baseRenderer->surfaceDepthStencilFormat = depthFormat;
	baseRenderer->surfaceSamples = options->samples;
	baseRenderer->doubleBuffer = true;
	baseRenderer->stereoscopic = false;
	baseRenderer->vsync = false;
	baseRenderer->clipHalfDepth = true;
	baseRenderer->clipInvertY = false;
	baseRenderer->hasGeometryShaders = false;
	baseRenderer->hasTessellationShaders = hasTessellationShaders(renderer->featureSet);

	MTLSize maxComputeSize = device.maxThreadsPerThreadgroup;
	baseRenderer->maxComputeWorkGroupSize[0] = (uint32_t)maxComputeSize.width;
	baseRenderer->maxComputeWorkGroupSize[1] = (uint32_t)maxComputeSize.height;
	baseRenderer->maxComputeWorkGroupSize[2] = (uint32_t)maxComputeSize.depth;

	baseRenderer->hasNativeMultidraw = false;
	baseRenderer->supportsInstancedDrawing = true;
	baseRenderer->supportsStartInstance = true;
	baseRenderer->defaultAnisotropy = 1.0f;

	return baseRenderer;

}
