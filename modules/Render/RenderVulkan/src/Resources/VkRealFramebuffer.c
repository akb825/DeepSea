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

#include "Resources/VkRealFramebuffer.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

bool getImageViews(dsResourceManager* resourceManager, const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount, uint32_t layers, VkImageView* imageViews, bool* imageViewsTemp)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = surfaces + i;
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			case dsGfxSurfaceType_DepthRenderSurface:
			case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			case dsGfxSurfaceType_DepthRenderSurfaceRight:
				// TODO
				return false;
			case dsGfxSurfaceType_Texture:
			{
				dsOffscreen* offscreen = (dsOffscreen*)surface->surfaceType;
				dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
				const dsTextureInfo* info = &offscreen->info;
				if (offscreen->resolve)
					imageViews[i] = vkOffscreen->surfaceImageView;
				else if (info->mipLevels == 1 && info->depth == 0 &&
					info->dimension != dsTextureDim_Cube)
				{
					imageViews[i] = vkOffscreen->deviceImageView;
				}
				else
				{
					uint32_t faceCount =1;
					VkImageViewType imageViewType;
					switch (info->dimension)
					{
						case dsTextureDim_1D:
							if (info->depth > 0)
								imageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
							else
								imageViewType = VK_IMAGE_VIEW_TYPE_1D;
							break;
						case dsTextureDim_2D:
							if (info->depth > 0)
								imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
							else
								imageViewType = VK_IMAGE_VIEW_TYPE_2D;
							break;
						case dsTextureDim_3D:
							imageViewType = VK_IMAGE_VIEW_TYPE_3D;
							break;
						case dsTextureDim_Cube:
							if (info->depth > 0)
								imageViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
							else
								imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
							faceCount = 6;
							break;
						default:
							DS_ASSERT(false);
							return false;
					}

					const dsVkFormatInfo* formatInfo = dsVkResourceManager_getFormat(
						resourceManager, info->format);
					DS_ASSERT(formatInfo);

					VkImageViewCreateInfo createInfo =
					{
						VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						NULL,
						0,
						vkOffscreen->deviceImage,
						imageViewType,
						formatInfo->vkFormat,
						{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
							VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
						{dsVkImageAspectFlags(info->format), surface->mipLevel, 1,
							surface->layer*faceCount + surface->cubeFace, layers}
					};
					VkResult result = DS_VK_CALL(device->vkCreateImageView)(device->device,
						&createInfo, instance->allocCallbacksPtr, imageViews + i);
					if (!dsHandleVkResult(result))
						return false;

					imageViewsTemp[i] = true;
				}
				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsVkRenderbuffer* renderbuffer = (dsVkRenderbuffer*)surface->surface;
				imageViews[i] = renderbuffer->imageView;
				break;
			}
			default:
				DS_ASSERT(false);
				return false;
		}
	}

	return true;
}

dsVkRealFramebuffer* dsVkRealFramebuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, VkRenderPass renderPass, const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount, uint32_t width, uint32_t height, uint32_t layers)
{
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkRealFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(VkImageView)*surfaceCount) +
		DS_ALIGNED_SIZE(sizeof(bool)*surfaceCount);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));

	dsVkRealFramebuffer* framebuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkRealFramebuffer);
	DS_ASSERT(framebuffer);

	framebuffer->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&framebuffer->resource);
	framebuffer->device = device;
	framebuffer->renderPass = renderPass;
	framebuffer->framebuffer = 0;

	if (surfaceCount > 0)
	{
		framebuffer->imageViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, VkImageView,
			surfaceCount);
		DS_ASSERT(framebuffer->imageViews);
		framebuffer->imageViewsTemp = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, bool,
			surfaceCount);
		DS_ASSERT(framebuffer->imageViewsTemp);
		memset(framebuffer->imageViewsTemp, 0, sizeof(bool)*surfaceCount);

		if (!getImageViews(resourceManager, surfaces, surfaceCount, layers, framebuffer->imageViews,
			framebuffer->imageViewsTemp))
		{
			dsVkRealFramebuffer_destroy(framebuffer);
			return NULL;
		}
	}
	else
	{
		framebuffer->imageViews = NULL;
		framebuffer->imageViewsTemp = NULL;
	}
	framebuffer->surfaceCount = surfaceCount;

	VkFramebufferCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		NULL,
		0,
		renderPass,
		surfaceCount, framebuffer->imageViews,
		width,
		height,
		layers
	};
	VkResult result = DS_VK_CALL(device->vkCreateFramebuffer)(device->device, &createInfo,
		instance->allocCallbacksPtr, &framebuffer->framebuffer);
	if (!dsHandleVkResult(result))
		dsVkRealFramebuffer_destroy(framebuffer);

	return framebuffer;
}

void dsVkRealFramebuffer_destroy(dsVkRealFramebuffer* framebuffer)
{
	if (!framebuffer)
		return;

	dsVkDevice* device = framebuffer->device;
	dsVkInstance* instance = &device->instance;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (framebuffer->imageViewsTemp[i])
		{
			DS_VK_CALL(device->vkDestroyImageView)(device->device, framebuffer->imageViews[i],
				instance->allocCallbacksPtr);
		}
	}

	DS_VK_CALL(device->vkDestroyFramebuffer)(device->device, framebuffer->framebuffer,
		instance->allocCallbacksPtr);

	if (framebuffer->allocator)
		DS_VERIFY(dsAllocator_free(framebuffer->allocator, framebuffer));
}
