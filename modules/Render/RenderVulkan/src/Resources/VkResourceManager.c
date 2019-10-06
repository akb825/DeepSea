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

#include "Resources/VkResourceManager.h"

#include "Resources/VkDeviceMaterial.h"
#include "Resources/VkDrawGeometry.h"
#include "Resources/VkFramebuffer.h"
#include "Resources/VkGfxFence.h"
#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxQueryPool.h"
#include "Resources/VkMaterialDesc.h"
#include "Resources/VkRenderbuffer.h"
#include "Resources/VkShader.h"
#include "Resources/VkShaderModule.h"
#include "Resources/VkTexture.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/DefaultShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <string.h>

#define DS_BUFFER_SIZE 256
#define DS_PIPELINE_MAGIC_NUMBER DS_FOURCC('D', 'S', 'V', 'K')
#define DS_PIPELINE_VERSION 0
#define DS_PIPELINE_FILE_NAME "vulkan_pipeline.cache"

struct dsResourceContext
{
	int dummy;
};

static dsResourceContext dummyContext;

static size_t fullAllocSize(const char* shaderCacheDir)
{
	size_t pathLen = shaderCacheDir ? strlen(shaderCacheDir) + 1 : 0;
	return DS_ALIGNED_SIZE(sizeof(dsVkResourceManager)) + DS_ALIGNED_SIZE(pathLen);
}

static void initializeFormat(dsVkResourceManager* resourceManager, dsGfxFormat format,
	VkFormat vkFormat)
{
	dsVkDevice* device = resourceManager->device;
	dsVkInstance* instance = &device->instance;
	dsVkFormatInfo* formatInfo = (dsVkFormatInfo*)dsVkResourceManager_getFormat(
		(dsResourceManager*)resourceManager, format);
	DS_ASSERT(formatInfo);
	formatInfo->vkFormat = vkFormat;
	DS_VK_CALL(instance->vkGetPhysicalDeviceFormatProperties)(device->physicalDevice,
		formatInfo->vkFormat, &formatInfo->properties);
}

static void initializeFormats(dsVkResourceManager* resourceManager)
{
	dsVkDevice* device = resourceManager->device;

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R4G4, dsGfxFormat_UNorm),
		VK_FORMAT_R4G4_UNORM_PACK8);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R4G4B4A4, dsGfxFormat_UNorm),
		VK_FORMAT_R4G4B4A4_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B4G4R4A4, dsGfxFormat_UNorm),
		VK_FORMAT_B4G4R4A4_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm),
		VK_FORMAT_R5G6B5_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B5G6R5, dsGfxFormat_UNorm),
		VK_FORMAT_B5G6R5_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R5G5B5A1, dsGfxFormat_UNorm),
		VK_FORMAT_R5G5B5A1_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B5G5R5A1, dsGfxFormat_UNorm),
		VK_FORMAT_B5G5R5A1_UNORM_PACK16);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A1R5G5B5, dsGfxFormat_UNorm),
		VK_FORMAT_A1R5G5B5_UNORM_PACK16);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm),
		VK_FORMAT_R8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SNorm),
		VK_FORMAT_R8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UScaled),
		VK_FORMAT_R8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SScaled),
		VK_FORMAT_R8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UInt),
		VK_FORMAT_R8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SInt),
		VK_FORMAT_R8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_SRGB),
		VK_FORMAT_R8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UNorm),
		VK_FORMAT_R8G8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SNorm),
		VK_FORMAT_R8G8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UScaled),
		VK_FORMAT_R8G8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SScaled),
		VK_FORMAT_R8G8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UInt),
		VK_FORMAT_R8G8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SInt),
		VK_FORMAT_R8G8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_SRGB),
		VK_FORMAT_R8G8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm),
		VK_FORMAT_R8G8B8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SNorm),
		VK_FORMAT_R8G8B8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UScaled),
		VK_FORMAT_R8G8B8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SScaled),
		VK_FORMAT_R8G8B8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UInt),
		VK_FORMAT_R8G8B8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SInt),
		VK_FORMAT_R8G8B8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_SRGB),
		VK_FORMAT_R8G8B8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_UNorm),
		VK_FORMAT_B8G8R8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_SNorm),
		VK_FORMAT_B8G8R8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_UScaled),
		VK_FORMAT_B8G8R8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_SScaled),
		VK_FORMAT_B8G8R8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_UInt),
		VK_FORMAT_B8G8R8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_SInt),
		VK_FORMAT_B8G8R8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8, dsGfxFormat_SRGB),
		VK_FORMAT_B8G8R8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		VK_FORMAT_R8G8B8A8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SNorm),
		VK_FORMAT_R8G8B8A8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
			dsGfxFormat_UScaled), VK_FORMAT_R8G8B8A8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
			dsGfxFormat_SScaled), VK_FORMAT_R8G8B8A8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UInt),
		VK_FORMAT_R8G8B8A8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SInt),
		VK_FORMAT_R8G8B8A8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB),
		VK_FORMAT_R8G8B8A8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm),
		VK_FORMAT_B8G8R8A8_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SNorm),
		VK_FORMAT_B8G8R8A8_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8,
			dsGfxFormat_UScaled), VK_FORMAT_B8G8R8A8_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8,
			dsGfxFormat_SScaled), VK_FORMAT_B8G8R8A8_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UInt),
		VK_FORMAT_B8G8R8A8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SInt),
		VK_FORMAT_B8G8R8A8_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_SRGB),
		VK_FORMAT_B8G8R8A8_SRGB);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_UNorm),
		VK_FORMAT_A8B8G8R8_UNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SNorm),
		VK_FORMAT_A8B8G8R8_SNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8,
			dsGfxFormat_UScaled), VK_FORMAT_A8B8G8R8_USCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8,
			dsGfxFormat_SScaled), VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_UInt),
		VK_FORMAT_A8B8G8R8_UINT_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SInt),
		VK_FORMAT_A8B8G8R8_SINT_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A8B8G8R8, dsGfxFormat_SRGB),
		VK_FORMAT_A8B8G8R8_SRGB_PACK32);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
		dsGfxFormat_UNorm), VK_FORMAT_A2R10G10B10_UNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
		dsGfxFormat_SNorm), VK_FORMAT_A2R10G10B10_SNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
			dsGfxFormat_UScaled), VK_FORMAT_A2R10G10B10_USCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
			dsGfxFormat_SScaled), VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
		dsGfxFormat_UInt), VK_FORMAT_A2R10G10B10_UINT_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2R10G10B10,
		dsGfxFormat_SInt), VK_FORMAT_A2R10G10B10_SINT_PACK32);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
		dsGfxFormat_UNorm), VK_FORMAT_A2B10G10R10_UNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
		dsGfxFormat_SNorm), VK_FORMAT_A2B10G10R10_SNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
			dsGfxFormat_UScaled), VK_FORMAT_A2B10G10R10_USCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
			dsGfxFormat_SScaled), VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
		dsGfxFormat_UInt), VK_FORMAT_A2B10G10R10_UINT_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_A2B10G10R10,
		dsGfxFormat_SInt), VK_FORMAT_A2B10G10R10_SINT_PACK32);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UNorm),
		VK_FORMAT_R16_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SNorm),
		VK_FORMAT_R16_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UScaled),
		VK_FORMAT_R16_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SScaled),
		VK_FORMAT_R16_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_UInt),
		VK_FORMAT_R16_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_SInt),
		VK_FORMAT_R16_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16, dsGfxFormat_Float),
		VK_FORMAT_R16_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UNorm),
		VK_FORMAT_R16G16_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SNorm),
		VK_FORMAT_R16G16_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UScaled),
		VK_FORMAT_R16G16_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SScaled),
		VK_FORMAT_R16G16_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_UInt),
		VK_FORMAT_R16G16_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_SInt),
		VK_FORMAT_R16G16_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16, dsGfxFormat_Float),
		VK_FORMAT_R16G16_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16,
		dsGfxFormat_UNorm), VK_FORMAT_R16G16B16_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16,
		dsGfxFormat_SNorm), VK_FORMAT_R16G16B16_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16,
			dsGfxFormat_UScaled), VK_FORMAT_R16G16B16_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16,
			dsGfxFormat_SScaled), VK_FORMAT_R16G16B16_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_UInt),
		VK_FORMAT_R16G16B16_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16, dsGfxFormat_SInt),
		VK_FORMAT_R16G16B16_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16,
		dsGfxFormat_Float), VK_FORMAT_R16G16B16_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
		dsGfxFormat_UNorm), VK_FORMAT_R16G16B16A16_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
		dsGfxFormat_SNorm), VK_FORMAT_R16G16B16A16_SNORM);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
			dsGfxFormat_UScaled), VK_FORMAT_R16G16B16A16_USCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
			dsGfxFormat_SScaled), VK_FORMAT_R16G16B16A16_SSCALED);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
		dsGfxFormat_UInt), VK_FORMAT_R16G16B16A16_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
		dsGfxFormat_SInt), VK_FORMAT_R16G16B16A16_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16,
		dsGfxFormat_Float), VK_FORMAT_R16G16B16A16_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_UInt),
		VK_FORMAT_R32_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_SInt),
		VK_FORMAT_R32_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32, dsGfxFormat_Float),
		VK_FORMAT_R32_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_UInt),
		VK_FORMAT_R32G32_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_SInt),
		VK_FORMAT_R32G32_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32, dsGfxFormat_Float),
		VK_FORMAT_R32G32_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_UInt),
		VK_FORMAT_R32G32B32_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32, dsGfxFormat_SInt),
		VK_FORMAT_R32G32B32_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32,
		dsGfxFormat_Float), VK_FORMAT_R32G32B32_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32,
		dsGfxFormat_UInt), VK_FORMAT_R32G32B32A32_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32,
		dsGfxFormat_SInt), VK_FORMAT_R32G32B32A32_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32,
		dsGfxFormat_Float), VK_FORMAT_R32G32B32A32_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64, dsGfxFormat_UInt),
		VK_FORMAT_R64_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64, dsGfxFormat_SInt),
		VK_FORMAT_R64_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64, dsGfxFormat_Float),
		VK_FORMAT_R64_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64, dsGfxFormat_UInt),
		VK_FORMAT_R64G64_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64, dsGfxFormat_SInt),
		VK_FORMAT_R64G64_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64, dsGfxFormat_Float),
		VK_FORMAT_R64G64_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64, dsGfxFormat_UInt),
		VK_FORMAT_R64G64B64_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64, dsGfxFormat_SInt),
		VK_FORMAT_R64G64B64_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64,
		dsGfxFormat_Float), VK_FORMAT_R64G64B64_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64A64,
		dsGfxFormat_UInt), VK_FORMAT_R64G64B64A64_UINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64A64,
		dsGfxFormat_SInt), VK_FORMAT_R64G64B64A64_SINT);
	initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_R64G64B64A64,
		dsGfxFormat_Float), VK_FORMAT_R64G64B64A64_SFLOAT);

	initializeFormat(resourceManager, dsGfxFormat_B10G11R11_UFloat,
		VK_FORMAT_B10G11R11_UFLOAT_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_E5B9G9R9_UFloat,
		VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);;
	initializeFormat(resourceManager, dsGfxFormat_D16, VK_FORMAT_D16_UNORM);
	initializeFormat(resourceManager, dsGfxFormat_X8D24, VK_FORMAT_X8_D24_UNORM_PACK32);
	initializeFormat(resourceManager, dsGfxFormat_D32_Float, VK_FORMAT_D32_SFLOAT);
	initializeFormat(resourceManager, dsGfxFormat_S8, VK_FORMAT_S8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_D16S8, VK_FORMAT_D16_UNORM_S8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_D24S8, VK_FORMAT_D24_UNORM_S8_UINT);
	initializeFormat(resourceManager, dsGfxFormat_D32S8_Float, VK_FORMAT_D32_SFLOAT_S8_UINT);

	if (device->features.textureCompressionBC)
	{
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC1_RGB,
			dsGfxFormat_UNorm), VK_FORMAT_BC1_RGB_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC1_RGB,
			dsGfxFormat_SRGB), VK_FORMAT_BC1_RGB_SRGB_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA,
			dsGfxFormat_UNorm), VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC1_RGBA,
			dsGfxFormat_SRGB), VK_FORMAT_BC1_RGBA_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_UNorm),
			VK_FORMAT_BC2_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC2, dsGfxFormat_SRGB),
			VK_FORMAT_BC2_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm),
			VK_FORMAT_BC3_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_SRGB),
			VK_FORMAT_BC3_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_UNorm),
			VK_FORMAT_BC4_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC4, dsGfxFormat_SNorm),
			VK_FORMAT_BC4_SNORM_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_UNorm),
			VK_FORMAT_BC5_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC5, dsGfxFormat_SNorm),
			VK_FORMAT_BC5_SNORM_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC6H,
			dsGfxFormat_UFloat), VK_FORMAT_BC6H_UFLOAT_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC6H, dsGfxFormat_Float),
			VK_FORMAT_BC6H_SFLOAT_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_UNorm),
			VK_FORMAT_BC7_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC7, dsGfxFormat_SRGB),
			VK_FORMAT_BC7_SRGB_BLOCK);
	}

	if (device->features.textureCompressionETC2)
	{
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC1,
			dsGfxFormat_UNorm), VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8,
			dsGfxFormat_UNorm), VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8,
			dsGfxFormat_SRGB), VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A1,
			dsGfxFormat_UNorm), VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A1,
			dsGfxFormat_SRGB), VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A8,
			dsGfxFormat_UNorm), VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8A8,
			dsGfxFormat_SRGB), VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_EAC_R11,
			dsGfxFormat_UNorm), VK_FORMAT_EAC_R11_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_EAC_R11,
			dsGfxFormat_SNorm), VK_FORMAT_EAC_R11_SNORM_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_EAC_R11G11,
			dsGfxFormat_UNorm), VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_EAC_R11G11,
			dsGfxFormat_SNorm), VK_FORMAT_EAC_R11G11_SNORM_BLOCK);
	}

	if (device->features.textureCompressionASTC_LDR)
	{
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_4x4,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_4x4_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_4x4,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_4x4_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_5x4,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_5x4_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_5x4,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_5x4_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_5x5,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_5x5_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_5x5,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_5x5_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_6x5,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_6x5_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_6x5,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_6x5_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_6x6,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_6x6_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_6x6,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_6x6_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_8x5_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_8x5_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x6,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_8x6_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x6,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_8x6_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x8,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_8x8_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x8,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_8x8_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x5,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_10x5_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x5,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_10x5_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x6,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_10x6_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x6,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_10x6_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x8,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_10x8_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x8,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_10x8_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x10,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_10x10_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_10x10,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_10x10_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_12x10,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_12x10_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_12x10,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_12x10_SRGB_BLOCK);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_12x12,
			dsGfxFormat_UNorm), VK_FORMAT_ASTC_12x12_UNORM_BLOCK);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_ASTC_12x12,
			dsGfxFormat_SRGB), VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
	}

	if (device->hasPVRTC)
	{
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_2BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_2BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_4BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_4BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_2BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_4BPP,
			dsGfxFormat_UNorm), VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_2BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_2BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGB_4BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC1_RGBA_4BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG);

		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_2BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG);
		initializeFormat(resourceManager, dsGfxFormat_decorate(dsGfxFormat_PVRTC2_RGBA_4BPP,
			dsGfxFormat_SRGB), VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
	}
}

static void* readPipelineCache(uint32_t* outSize, dsAllocator* allocator,
	const char* shaderCacheDir)
{
	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, DS_PATH_MAX, shaderCacheDir, DS_PIPELINE_FILE_NAME))
	{
		DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "Shader cache path is too long.");
		return NULL;
	}

	dsFileStream stream;
	uint8_t* data = NULL;
	if (!dsFileStream_openPath(&stream, path, "rb"))
		return false;

	uint32_t magicNumber;
	if (!dsFileStream_read(&stream, &magicNumber, sizeof(magicNumber)) ||
		magicNumber != DS_PIPELINE_MAGIC_NUMBER)
	{
		goto error;
	}

	uint32_t version;
	if (!dsFileStream_read(&stream, &version, sizeof(version)) || version != DS_PIPELINE_VERSION)
		goto error;

	if (!dsFileStream_read(&stream, outSize, sizeof(*outSize)))
		goto error;

	data = (uint8_t*)dsAllocator_alloc(allocator, *outSize);
	if (!data || !dsFileStream_read(&stream, data, *outSize))
		goto error;

	DS_VERIFY(dsFileStream_close(&stream));
	return data;

error:
	DS_VERIFY(dsFileStream_close(&stream));
	DS_VERIFY(dsAllocator_free(allocator, data));
	return NULL;
}

static bool writePipelineCache(dsAllocator* allocator, const char* shaderCacheDir,
	dsVkDevice* device, VkPipelineCache pipelineCache)
{
	if (!dsShader_prepareCacheDirectory(shaderCacheDir))
		return false;

	size_t size = 0;
	VkResult result = DS_VK_CALL(device->vkGetPipelineCacheData)(device->device, pipelineCache,
		&size, NULL);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't get pipeline cache data"))
		return false;

	void* data = dsAllocator_alloc(allocator, size);
	if (!data)
		return false;

	result = DS_VK_CALL(device->vkGetPipelineCacheData)(device->device, pipelineCache,
		&size, NULL);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't get pipeline cache data"))
		goto bufferError;

	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, DS_PATH_MAX, shaderCacheDir, DS_PIPELINE_FILE_NAME))
	{
		DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "Shader cache path is too long.");
		goto bufferError;
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, path, "wb"))
	{
		DS_LOG_WARNING_F(DS_RENDER_VULKAN_LOG_TAG, "Couldn't write to directory '%s': %s",
			shaderCacheDir, dsErrorString(errno));
		goto bufferError;
	}

	uint32_t magicNumber = DS_PIPELINE_MAGIC_NUMBER;
	if (!dsFileStream_write(&stream, &magicNumber, sizeof(magicNumber)))
		goto error;

	uint32_t version = DS_PIPELINE_VERSION;
	if (!dsFileStream_write(&stream, &version, sizeof(version)))
		goto error;

	if (!dsFileStream_write(&stream, &size, sizeof(size)))
		goto error;

	if (!dsFileStream_write(&stream, data, size))
		goto error;

	DS_VERIFY(dsFileStream_close(&stream));
	DS_VERIFY(dsAllocator_free(allocator, data));

	return true;

error:
	DS_VERIFY(dsFileStream_close(&stream));
bufferError:
	DS_VERIFY(dsAllocator_free(allocator, data));
	return false;
}

bool dsVkResourceManager_vertexFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
		return false;

	return (formatInfo->properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) != 0;
}

bool dsVkResourceManager_textureFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
		return false;

	return (formatInfo->properties.optimalTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
}

bool dsVkResourceManager_textureBufferFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
		return false;

	return (formatInfo->properties.bufferFeatures &
		VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) != 0;
}

bool dsVkResourceManager_imageFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
		return false;

	return (formatInfo->properties.optimalTilingFeatures &
		VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
}

bool dsVkResourceManager_renderTargetFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(resourceManager, format);
	if (!formatInfo)
		return false;

	return (formatInfo->properties.optimalTilingFeatures &
			VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) ||
		(formatInfo->properties.optimalTilingFeatures &
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool dsVkResourceManager_surfaceBlitFormatsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat, dsBlitFilter filter)
{
	const dsVkFormatInfo* srcFormatInfo = dsVkResourceManager_getFormat(resourceManager, srcFormat);
	if (!srcFormatInfo)
		return false;

	const dsVkFormatInfo* dstFormatInfo = dsVkResourceManager_getFormat(resourceManager, dstFormat);
	if (!dstFormatInfo)
		return false;

	if (!(srcFormatInfo->properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) ||
		!(dstFormatInfo->properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		return false;
	}

	return dsGfxFormat_standardSurfaceBlitSupported(srcFormat, dstFormat, filter);
}

bool dsVkResourceManager_generateMipmapFormatSupported(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	return dsVkResourceManager_surfaceBlitFormatsSupported(resourceManager, format, format,
		dsBlitFilter_Linear);
}

bool dsVkResourceManager_textureCopyFormatsSupported(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat)
{
	if (!dsVkResourceManager_textureFormatSupported(resourceManager, srcFormat) ||
		!dsVkResourceManager_textureFormatSupported(resourceManager, dstFormat))
	{
		return false;
	}

	return dsGfxFormat_size(srcFormat) == dsGfxFormat_size(srcFormat);
}

dsResourceContext* dsVkResourceManager_createResourceContext(dsResourceManager* resourceManager)
{
	DS_UNUSED(resourceManager);
	return &dummyContext;
}

bool dsVkResourceManager_destroyResourceContext(dsResourceManager* resourceManager,
	dsResourceContext* context)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(context);
	return true;
}

dsResourceManager* dsVkResourceManager_create(dsAllocator* allocator, dsVkRenderer* renderer,
	const char* shaderCacheDir)
{
	DS_ASSERT(allocator);
	DS_ASSERT(renderer);

	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	size_t fullSize = fullAllocSize(shaderCacheDir);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVkResourceManager* resourceManager = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkResourceManager);
	DS_ASSERT(resourceManager);

	dsVkDevice* device = &renderer->device;
	memset(resourceManager, 0, sizeof(dsVkResourceManager));
	resourceManager->device = device;

	dsResourceManager* baseResourceManager = (dsResourceManager*)resourceManager;
	DS_VERIFY(dsResourceManager_initialize(baseResourceManager));

	baseResourceManager->renderer = baseRenderer;
	baseResourceManager->allocator = dsAllocator_keepPointer(allocator);
	baseResourceManager->maxResourceContexts = UINT_MAX;

	const VkPhysicalDeviceLimits* limits = &renderer->device.properties.limits;
	const VkPhysicalDeviceFeatures* features = &renderer->device.features;
	baseResourceManager->minNonCoherentMappingAlignment = (uint32_t)limits->nonCoherentAtomSize;
	baseResourceManager->minTextureBufferAlignment =
		(uint32_t)limits->minTexelBufferOffsetAlignment;
	baseResourceManager->minUniformBlockAlignment =
		(uint32_t)limits->minUniformBufferOffsetAlignment;
	baseResourceManager->minUniformBufferAlignment =
		(uint32_t)limits->minStorageBufferOffsetAlignment;
	baseResourceManager->supportedBuffers = dsGfxBufferUsage_Index | dsGfxBufferUsage_Vertex |
		dsGfxBufferUsage_IndirectDraw | dsGfxBufferUsage_IndirectDispatch |
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_Texture |
		dsGfxBufferUsage_Image | dsGfxBufferUsage_CopyFrom | dsGfxBufferUsage_CopyTo;
	baseResourceManager->bufferMapSupport = dsGfxBufferMapSupport_Persistent;
	baseResourceManager->canCopyBuffers = true;
	baseResourceManager->hasTextureBufferSubrange = true;
	baseResourceManager->maxIndexSize = features->fullDrawIndexUint32 ? sizeof(uint32_t) :
		sizeof(uint16_t);
	baseResourceManager->maxUniformBlockSize = limits->maxUniformBufferRange;
	baseResourceManager->maxTextureBufferElements = limits->maxTexelBufferElements;
	baseResourceManager->maxVertexAttribs = limits->maxVertexInputAttributes;
	baseResourceManager->lineWidthRange.x = limits->lineWidthRange[0];
	baseResourceManager->lineWidthRange.y = limits->lineWidthRange[1];
	baseResourceManager->maxSamplers = limits->maxDescriptorSetSamplers;
	baseResourceManager->maxVertexSamplers = limits->maxDescriptorSetSamplers;
	baseResourceManager->maxTextureSize = limits->maxImageDimension2D;
	baseResourceManager->maxTextureDepth = limits->maxImageDimension3D;
	baseResourceManager->maxTextureArrayLevels = limits->maxImageArrayLayers;
	baseResourceManager->maxTextureArrayLevels = limits->maxImageArrayLayers;
	baseResourceManager->maxRenderbufferSize = limits->maxFramebufferWidth;
	baseResourceManager->maxFramebufferLayers = limits->maxFramebufferLayers;
	baseResourceManager->maxTextureSamples = limits->sampledImageColorSampleCounts;
	baseResourceManager->hasArbitraryMipmapping = true;
	baseResourceManager->hasCubeArrays = features->imageCubeArray != 0;
	baseResourceManager->texturesReadable = false;
	baseResourceManager->requiresColorBuffer = false;
	baseResourceManager->canMixWithRenderSurface = true;
	baseResourceManager->hasVertexPipelineWrites = features->vertexPipelineStoresAndAtomics != 0;
	baseResourceManager->hasFragmentWrites = features->fragmentStoresAndAtomics != 0;
	for (int i = 0; i < 3; ++i)
		baseResourceManager->maxComputeLocalWorkGroupSize[i] = limits->maxComputeWorkGroupSize[i];
	baseResourceManager->maxComputeLocalWorkGroupInvocations =
		limits->maxComputeWorkGroupInvocations;
	baseResourceManager->maxClipDistances = limits->maxClipDistances;
	baseResourceManager->maxCullDistances = limits->maxCullDistances;
	baseResourceManager->maxCombinedClipAndCullDistances = limits->maxCombinedClipAndCullDistances;
	baseResourceManager->hasFences = true;
	baseResourceManager->hasQueries = true;
	baseResourceManager->hasPreciseOcclusionQueries = features->occlusionQueryPrecise;
	baseResourceManager->has64BitQueries = true;
	baseResourceManager->hasQueryBuffers = true;
	baseResourceManager->timestampPeriod = limits->timestampPeriod;

	resourceManager->maxPushConstantSize = limits->maxPushConstantsSize;

	// Core functionality
	initializeFormats(resourceManager);
	baseResourceManager->vertexFormatSupportedFunc = &dsVkResourceManager_vertexFormatSupported;
	baseResourceManager->textureFormatSupportedFunc = &dsVkResourceManager_textureFormatSupported;
	baseResourceManager->textureBufferFormatSupportedFunc =
		&dsVkResourceManager_textureBufferFormatSupported;
	baseResourceManager->imageFormatSupportedFunc = &dsVkResourceManager_imageFormatSupported;
	baseResourceManager->renderTargetFormatSupportedFunc =
		&dsVkResourceManager_renderTargetFormatSupported;
	baseResourceManager->generateMipmapFormatSupportedFunc =
		&dsVkResourceManager_generateMipmapFormatSupported;
	baseResourceManager->textureCopyFormatsSupportedFunc =
		&dsVkResourceManager_textureCopyFormatsSupported;
	baseResourceManager->surfaceBlitFormatsSupportedFunc =
		&dsVkResourceManager_surfaceBlitFormatsSupported;
	baseResourceManager->copyBufferToTextureSupportedFunc =
		&dsVkResourceManager_textureFormatSupported;
	baseResourceManager->copyTextureToBufferSupportedFunc =
		&dsVkResourceManager_textureFormatSupported;
	baseResourceManager->createResourceContextFunc = &dsVkResourceManager_createResourceContext;
	baseResourceManager->destroyResourceContextFunc = &dsVkResourceManager_destroyResourceContext;

	// Buffers
	baseResourceManager->createBufferFunc = &dsVkGfxBuffer_create;
	baseResourceManager->destroyBufferFunc = &dsVkGfxBuffer_destroy;
	baseResourceManager->mapBufferFunc = &dsVkGfxBuffer_map;
	baseResourceManager->unmapBufferFunc = &dsVkGfxBuffer_unmap;
	baseResourceManager->flushBufferFunc = &dsVkGfxBuffer_flush;
	baseResourceManager->invalidateBufferFunc = &dsVkGfxBuffer_invalidate;
	baseResourceManager->copyBufferDataFunc = &dsVkGfxBuffer_copyData;
	baseResourceManager->copyBufferFunc = &dsVkGfxBuffer_copy;
	baseResourceManager->copyBufferToTextureFunc = &dsVkGfxBuffer_copyToTexture;

	// Draw geometry
	baseResourceManager->createGeometryFunc = &dsVkDrawGeometry_create;
	baseResourceManager->destroyGeometryFunc = &dsVkDrawGeometry_destroy;

	// Textures
	baseResourceManager->createTextureFunc = &dsVkTexture_create;
	baseResourceManager->createOffscreenFunc = &dsVkTexture_createOffscreen;
	baseResourceManager->destroyTextureFunc = &dsVkTexture_destroy;
	baseResourceManager->copyTextureDataFunc = &dsVkTexture_copyData;
	baseResourceManager->copyTextureFunc = &dsVkTexture_copy;
	baseResourceManager->copyTextureToBufferFunc = &dsVkTexture_copyToBuffer;
	baseResourceManager->generateTextureMipmapsFunc = &dsVkTexture_generateMipmaps;
	baseResourceManager->getTextureDataFunc = &dsVkTexture_getData;
	baseResourceManager->processTextureFunc = &dsVkTexture_process;

	// Renderbuffers
	baseResourceManager->createRenderbufferFunc = &dsVkRenderbuffer_create;
	baseResourceManager->destroyRenderbufferFunc = &dsVkRenderbuffer_destroy;

	// Framebuffers
	baseResourceManager->createFramebufferFunc = &dsVkFramebuffer_create;
	baseResourceManager->destroyFramebufferFunc = &dsVkFramebuffer_destroy;

	// Fences
	baseResourceManager->createFenceFunc = &dsVkGfxFence_create;
	baseResourceManager->destroyFenceFunc = &dsVkGfxFence_destroy;
	baseResourceManager->setFencesFunc = &dsVkGfxFence_set;
	baseResourceManager->waitFenceFunc = &dsVkGfxFence_wait;
	baseResourceManager->resetFenceFunc = &dsVkGfxFence_reset;

	// Queries
	baseResourceManager->createQueryPoolFunc = &dsVkGfxQueryPool_create;
	baseResourceManager->destroyQueryPoolFunc = &dsVkGfxQueryPool_destroy;
	baseResourceManager->resetQueryPoolFunc = &dsVkGfxQueryPool_reset;
	baseResourceManager->beginQueryFunc = &dsVkGfxQueryPool_beginQuery;
	baseResourceManager->endQueryFunc = &dsVkGfxQueryPool_endQuery;
	baseResourceManager->queryTimestampFunc = &dsVkGfxQueryPool_queryTimestamp;
	baseResourceManager->getQueryValuesFunc = &dsVkGfxQueryPool_getValues;
	baseResourceManager->copyQueryValuesFunc = &dsVkGfxQueryPool_copyValues;

	// Shader modules
	baseResourceManager->createShaderModuleFunc = &dsVkShaderModule_create;
	baseResourceManager->destroyShaderModuleFunc = &dsVkShaderModule_destroy;

	// Material descriptions
	baseResourceManager->createMaterialDescFunc = &dsVkMaterialDesc_create;
	baseResourceManager->destroyMaterialDescFunc = &dsVkMaterialDesc_destroy;

	// Shader variable group descriptions
	baseResourceManager->createShaderVariableGroupDescFunc =
		&dsDefaultShaderVariableGroupDesc_create;
	baseResourceManager->destroyShaderVariableGroupDescFunc =
		&dsDefaultShaderVariableGroupDesc_destroy;

	// Device materials
	baseResourceManager->createDeviceMaterialFunc = &dsVkDeviceMaterial_create;
	baseResourceManager->materialElementChangedFunc = &dsVkDeviceMaterial_valueChanged;
	baseResourceManager->destroyDeviceMaterialFunc = &dsVkDeviceMaterial_destroy;

	// Shaders
	baseResourceManager->createShaderFunc = &dsVkShader_create;
	baseResourceManager->destroyShaderFunc = &dsVkShader_destroy;
	baseResourceManager->bindShaderFunc = &dsVkShader_bind;
	baseResourceManager->updateShaderInstanceValuesFunc = &dsVkShader_updateInstanceValues;
	baseResourceManager->updateShaderDynamicRenderStatesFunc =
		&dsVkShader_updateDynamicRenderStates;
	baseResourceManager->unbindShaderFunc = &dsVkShader_unbind;
	baseResourceManager->bindComputeShaderFunc = &dsVkShader_bindCompute;
	baseResourceManager->updateComputeShaderInstanceValuesFunc =
		&dsVkShader_updateComputeInstanceValues;
	baseResourceManager->unbindComputeShaderFunc = &dsVkShader_unbindCompute;

	void* pipelineCacheData = NULL;
	uint32_t pipelineCacheDataSize = 0;
	if (shaderCacheDir)
	{
		size_t length = strlen(shaderCacheDir) + 1;
		char* stringCopy = (char*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, length);
		memcpy(stringCopy, shaderCacheDir, length);
		resourceManager->shaderCacheDir = stringCopy;

		pipelineCacheData = readPipelineCache(&pipelineCacheDataSize, allocator, shaderCacheDir);
	}

	dsVkInstance* instance = &device->instance;
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		NULL,
		0,
		pipelineCacheDataSize,
		pipelineCacheData
	};
	VkResult result = DS_VK_CALL(device->vkCreatePipelineCache)(device->device,
		&pipelineCacheCreateInfo, instance->allocCallbacksPtr, &resourceManager->pipelineCache);
	DS_VERIFY(dsAllocator_free(allocator, pipelineCacheData));
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create pipeline cache"))
	{
		dsVkResourceManager_destroy(baseResourceManager);
		return NULL;
	}

	return baseResourceManager;
}

const dsVkFormatInfo* dsVkResourceManager_getFormat(const dsResourceManager* resourceManager,
	dsGfxFormat format)
{
	const dsVkResourceManager* vkResourceManager = (const dsVkResourceManager*)resourceManager;
	uint32_t index = dsGfxFormat_standardIndex(format);
	if (index > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		if (decoratorIndex == 0)
			return NULL;
		return &vkResourceManager->standardFormats[index][decoratorIndex];
	}

	index = dsGfxFormat_specialIndex(format);
	if (index > 0)
		return &vkResourceManager->specialFormats[index];

	index = dsGfxFormat_compressedIndex(format);
	if (index > 0)
	{
		uint32_t decoratorIndex = dsGfxFormat_decoratorIndex(format);
		if (decoratorIndex == 0)
			return NULL;
		return &vkResourceManager->compressedFormats[index][decoratorIndex];
	}

	return NULL;
}

void dsVkResourceManager_destroy(dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsVkResourceManager* vkResourceManager = (dsVkResourceManager*)resourceManager;

	dsVkDevice* device = vkResourceManager->device;
	dsVkInstance* instance = &device->instance;
	if (vkResourceManager->pipelineCache)
	{
		if (vkResourceManager->shaderCacheDir)
		{
			writePipelineCache(resourceManager->allocator, vkResourceManager->shaderCacheDir,
				device, vkResourceManager->pipelineCache);
		}

		DS_VK_CALL(device->vkDestroyPipelineCache)(device->device, vkResourceManager->pipelineCache,
			instance->allocCallbacksPtr);
	}

	DS_VERIFY(dsAllocator_free(resourceManager->allocator, resourceManager));
}
