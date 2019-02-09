/*
 * Copyright 2018-2019 Aaron Barany
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
#include "VkRenderPassData.h"
#include "VkRenderSurface.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static bool getImageViews(dsResourceManager* resourceManager, const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount, uint32_t layers, VkImageView* imageViews, bool* imageViewTemp,
	uint32_t imageCount, const dsRenderPass* renderPass)
{
	DS_UNUSED(imageCount);
	dsRenderer* renderer = resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;

	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = surfaces + i;
		uint32_t resolveIndex = vkRenderPass->resolveIndices[i];
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			{
				dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface->surface;
				dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
				VkImageView baseImage;
				if (surface->surfaceType == dsGfxSurfaceType_ColorRenderSurfaceRight)
				{
					DS_ASSERT(surfaceData->rightImageViews);
					baseImage = surfaceData->rightImageViews[0];
				}
				else
					baseImage = surfaceData->leftImageViews[0];

				if (resolveIndex == DS_NO_ATTACHMENT)
					imageViews[i] = baseImage;
				else
				{
					if (surfaceData->resolveImageView)
						imageViews[i] = surfaceData->resolveImageView;
					else
						imageViews[i] = baseImage;
					imageViews[resolveIndex] = baseImage;
				}
				break;
			}
			case dsGfxSurfaceType_DepthRenderSurface:
			case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			case dsGfxSurfaceType_DepthRenderSurfaceRight:
			{
				dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface->surface;
				dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
				imageViews[i] = surfaceData->depthImageView;
				DS_ASSERT(resolveIndex == DS_NO_ATTACHMENT);
				break;
			}
			case dsGfxSurfaceType_Texture:
			{
				dsOffscreen* offscreen = (dsOffscreen*)surface->surface;
				dsVkTexture* vkOffscreen = (dsVkTexture*)offscreen;
				const dsTextureInfo* info = &offscreen->info;
				uint32_t index = i;
				if (offscreen->resolve && !dsGfxFormat_isDepthStencil(info->format))
				{
					DS_ASSERT(resolveIndex != DS_NO_ATTACHMENT);
					DS_ASSERT(offscreen->info.samples > 1);
					DS_ASSERT(vkOffscreen->surfaceImageView);
					imageViews[i] = vkOffscreen->surfaceImageView;
					index = resolveIndex;
				}

				if (info->mipLevels == 1 && info->depth == 0 &&
					info->dimension != dsTextureDim_Cube)
				{
					imageViews[index] = vkOffscreen->deviceImageView;
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
						&createInfo, instance->allocCallbacksPtr, imageViews + index);
					if (!dsHandleVkResult(result))
						return false;

					imageViewTemp[index] = true;

					// Make sure both are set.
					if (index == i*2)
						imageViews[i*2 + 1] = imageViews[i*2];
				}
				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsVkRenderbuffer* renderbuffer = (dsVkRenderbuffer*)surface->surface;
				imageViews[i] = renderbuffer->imageView;
				DS_ASSERT(resolveIndex == DS_NO_ATTACHMENT);
				break;
			}
			default:
				DS_ASSERT(false);
				return false;
		}
	}

	return true;
}

void updateRenderSurfaceImages(dsVkRealFramebuffer* framebuffer,
	const dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t imageIndex)
{
	const dsVkRenderPassData* renderPassData =
		(const dsVkRenderPassData*)dsLifetime_acquire(framebuffer->renderPassData);
	if (!renderPassData)
		return;

	const dsVkRenderPass* renderPass = (const dsVkRenderPass*)renderPassData->renderPass;
	VkImageView* imageViews = framebuffer->imageViews;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		const dsFramebufferSurface* surface = surfaces + i;
		switch (surface->surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurface:
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
			{
				dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface->surface;
				dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
				VkImageView baseImage;
				if (surface->surfaceType == dsGfxSurfaceType_ColorRenderSurfaceRight)
				{
					DS_ASSERT(surfaceData->rightImageViews);
					baseImage = surfaceData->rightImageViews[imageIndex];
				}
				else
					baseImage = surfaceData->leftImageViews[imageIndex];

				uint32_t resolveIndex = renderPass->resolveIndices[i];
				if (resolveIndex == DS_NO_ATTACHMENT)
					imageViews[i] = baseImage;
				else
				{
					if (surfaceData->resolveImageView)
						imageViews[i] = surfaceData->resolveImageView;
					else
						imageViews[i] = baseImage;
					imageViews[resolveIndex] = baseImage;
				}
				break;
			}
			default:
				break;
		}
	}

	dsLifetime_release(framebuffer->renderPassData);
}

dsVkRealFramebuffer* dsVkRealFramebuffer_create(dsAllocator* allocator,
	dsFramebuffer* framebuffer, const dsVkRenderPassData* renderPassData,
	const dsVkRenderSurfaceData* surfaceData)
{
	dsRenderer* renderer = framebuffer->resourceManager->renderer;
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	const dsRenderPass* renderPass = renderPassData->renderPass;
	const dsVkRenderPass* vkRenderPass = (const dsVkRenderPass*)renderPass;
	uint32_t framebufferCount = surfaceData ? surfaceData->imageCount : 1;

	uint32_t imageCount = vkRenderPass->fullAttachmentCount;
	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsVkRealFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(VkFramebuffer)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(VkImageView)*imageCount) +
		DS_ALIGNED_SIZE(sizeof(bool)*imageCount);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));

	dsVkRealFramebuffer* realFramebuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVkRealFramebuffer);
	DS_ASSERT(realFramebuffer);

	realFramebuffer->allocator = dsAllocator_keepPointer(allocator);
	dsVkResource_initialize(&realFramebuffer->resource);
	realFramebuffer->device = device;
	realFramebuffer->renderPassData = dsLifetime_addRef(renderPassData->lifetime);
	realFramebuffer->surfaceData = surfaceData;

	realFramebuffer->framebuffers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		VkFramebuffer, framebufferCount);
	DS_ASSERT(realFramebuffer->framebuffers);
	memset(realFramebuffer->framebuffers, 0, sizeof(VkFramebuffer)*framebufferCount);

	if (framebuffer->surfaceCount > 0)
	{
		realFramebuffer->imageViews = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			VkImageView, imageCount);
		DS_ASSERT(realFramebuffer->imageViews);
		memset(realFramebuffer->imageViews, 0, sizeof(VkImageView)*imageCount);

		realFramebuffer->imageViewTemp = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, bool,
			imageCount);
		DS_ASSERT(realFramebuffer->imageViewTemp);
		memset(realFramebuffer->imageViewTemp, 0, sizeof(bool)*imageCount);

		if (!getImageViews(framebuffer->resourceManager, framebuffer->surfaces,
			framebuffer->surfaceCount, framebuffer->layers, realFramebuffer->imageViews,
			realFramebuffer->imageViewTemp, imageCount, renderPass))
		{
			dsVkRealFramebuffer_destroy(realFramebuffer);
			return NULL;
		}
	}
	else
	{
		realFramebuffer->imageViews = NULL;
		realFramebuffer->imageViewTemp = NULL;
	}
	realFramebuffer->imageCount = imageCount;
	realFramebuffer->framebufferCount = framebufferCount;

	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		updateRenderSurfaceImages(realFramebuffer, framebuffer->surfaces, framebuffer->surfaceCount,
			i);

		VkFramebufferCreateInfo createInfo =
		{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			NULL,
			0,
			renderPassData->vkRenderPass,
			imageCount, realFramebuffer->imageViews,
			framebuffer->width,
			framebuffer->height,
			framebuffer->layers
		};
		VkResult result = DS_VK_CALL(device->vkCreateFramebuffer)(device->device, &createInfo,
			instance->allocCallbacksPtr, realFramebuffer->framebuffers + i);
		if (!dsHandleVkResult(result))
		{
			dsVkRealFramebuffer_destroy(realFramebuffer);
			return NULL;
		}
	}

	return realFramebuffer;
}

void dsVkRealFramebuffer_destroy(dsVkRealFramebuffer* framebuffer)
{
	if (!framebuffer)
		return;

	dsVkDevice* device = framebuffer->device;
	dsVkInstance* instance = &device->instance;
	for (uint32_t i = 0; i < framebuffer->imageCount; ++i)
	{
		if (framebuffer->imageViewTemp[i])
		{
			DS_VK_CALL(device->vkDestroyImageView)(device->device, framebuffer->imageViews[i],
				instance->allocCallbacksPtr);
		}
	}

	for (uint32_t i = 0; i < framebuffer->framebufferCount; ++i)
	{
		if (framebuffer->framebuffers[i])
		{
			DS_VK_CALL(device->vkDestroyFramebuffer)(device->device, framebuffer->framebuffers[i],
				instance->allocCallbacksPtr);
		}
	}

	dsLifetime_freeRef(framebuffer->renderPassData);

	if (framebuffer->allocator)
		DS_VERIFY(dsAllocator_free(framebuffer->allocator, framebuffer));
}

VkFramebuffer dsVkRealFramebuffer_getFramebuffer(const dsVkRealFramebuffer* framebuffer)
{
	if (framebuffer->surfaceData)
		return framebuffer->framebuffers[framebuffer->surfaceData->imageIndex];
	return framebuffer->framebuffers[0];
}
