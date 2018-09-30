/*
 * Copyright 2018 Aaron Barany
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

#include <DeepSea/RenderVulkan/VkRenderer.h>

#include "Resources/VkResourceManager.h"
#include "Types.h"
#include "VkInit.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static size_t dsVkRenderer_fullAllocSize(const dsRendererOptions* options)
{
	size_t pathLen = options->shaderCacheDir ? strlen(options->shaderCacheDir) + 1 : 0;
	return DS_ALIGNED_SIZE(sizeof(dsVkRenderer)) + DS_ALIGNED_SIZE(pathLen);
}

bool dsVkRenderer_destroy(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	dsVkResourceManager_destroy((dsVkResourceManager*)renderer->resourceManager);
	dsDestroyVkDevice(device);
	dsDestroyVkInstance(&device->instance);
	DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));
	return true;
}

bool dsVkRenderer_isSupported(void)
{
	static int supported = -1;
	if (supported >= 0)
		return supported;

	dsVkInstance instance;
	memset(&instance, 0, sizeof(dsVkInstance));
	supported = dsCreateVkInstance(&instance, NULL, false);
	if (supported)
		supported = dsGatherVkPhysicalDevices(&instance);
	dsDestroyVkInstance(&instance);
	return supported;
}

bool dsVkRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	return dsQueryVkDevices(outDevices, outDeviceCount);
}

dsRenderer* dsVkRenderer_create(dsAllocator* allocator, const dsRendererOptions* options)
{
	if (!allocator || !options)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Renderer allocator must support freeing memory.");
		return NULL;
	}

	dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options);
	if (!dsGfxFormat_isValid(colorFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Invalid color format.");
		return NULL;
	}

	dsGfxFormat depthFormat = dsRenderer_optionsDepthFormat(options);

	size_t bufferSize = dsVkRenderer_fullAllocSize(options);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVkRenderer);
	DS_ASSERT(renderer);
	memset(renderer, 0, sizeof(*renderer));
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsRenderer_initialize(baseRenderer));
	baseRenderer->allocator = allocator;

	if (!dsCreateVkInstance(&renderer->device.instance, options, true) ||
		!dsCreateVkDevice(&renderer->device, allocator, options))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	baseRenderer->rendererID = DS_VK_RENDERER_ID;
	baseRenderer->platformID = 0;
	baseRenderer->name = "Vulkan";
	baseRenderer->shaderLanguage = "spirv";

	const VkPhysicalDeviceProperties* deviceProperties = &device->properties;
	baseRenderer->deviceName = device->properties.deviceName;
	baseRenderer->vendorID = deviceProperties->vendorID;
	baseRenderer->deviceID = deviceProperties->deviceID;
	baseRenderer->driverVersion = deviceProperties->driverVersion;
	// NOTE: Vulkan version encoding happens to be the same as DeepSea. (unintentional, but
	// convenient)
	baseRenderer->shaderVersion = deviceProperties->apiVersion;

	VkPhysicalDeviceFeatures deviceFeatures;
	DS_VK_CALL(instance->vkGetPhysicalDeviceFeatures)(device->physicalDevice, &deviceFeatures);

	const VkPhysicalDeviceLimits* limits = &deviceProperties->limits;
	baseRenderer->maxColorAttachments = limits->maxColorAttachments;
	// framebufferColorSampleCounts is a bitmask. Compute the maximum bit that's set.
	baseRenderer->maxSurfaceSamples = 1 << (31 - dsClz(limits->framebufferColorSampleCounts));
	baseRenderer->maxAnisotropy = limits->maxSamplerAnisotropy;
	baseRenderer->surfaceColorFormat = colorFormat;
	baseRenderer->surfaceDepthStencilFormat = depthFormat;

	baseRenderer->surfaceSamples = dsNextPowerOf2(dsMax(options->samples, 1U));
	baseRenderer->surfaceSamples = dsMin(baseRenderer->surfaceSamples,
		baseRenderer->maxSurfaceSamples);

	baseRenderer->doubleBuffer = options->doubleBuffer;
	baseRenderer->stereoscopic = options->stereoscopic;
	baseRenderer->vsync = false;
	baseRenderer->clipHalfDepth = true;
	baseRenderer->clipInvertY = true;
	baseRenderer->hasGeometryShaders = deviceFeatures.geometryShader != 0;
	baseRenderer->hasTessellationShaders = deviceFeatures.tessellationShader != 0;
	baseRenderer->maxComputeInvocations = limits->maxComputeWorkGroupInvocations;
	baseRenderer->hasNativeMultidraw = true;
	baseRenderer->supportsInstancedDrawing = true;
	baseRenderer->supportsStartInstance = deviceFeatures.drawIndirectFirstInstance;
	baseRenderer->defaultAnisotropy = 1;

	baseRenderer->resourceManager = (dsResourceManager*)dsVkResourceManager_create(allocator,
		renderer);
	if (!baseRenderer->resourceManager)
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	return baseRenderer;
}
