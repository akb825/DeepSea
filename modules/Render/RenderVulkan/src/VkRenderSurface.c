/*
 * Copyright 2018-2025 Aaron Barany
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

#include "VkRenderSurface.h"

#include "Platform/VkPlatform.h"
#include "VkCommandBuffer.h"
#include "VkRenderSurfaceData.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Renderer.h>
#include <string.h>

static bool transitionToRenderable(dsCommandBuffer* commandBuffer, dsVkRenderSurfaceData* surface)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkImageMemoryBarrier imageBarrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		surface->images[surface->imageIndex],
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL,
		0, NULL, 1, &imageBarrier);

	return true;
}

static bool transitionToPresentable(dsCommandBuffer* commandBuffer, dsVkRenderSurfaceData* surface)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	VkImageMemoryBarrier imageBarrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		surface->images[surface->imageIndex],
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(vkCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);

	return true;
}

dsRenderSurface* dsVkRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* displayHandle, void* osHandle, dsRenderSurfaceType type,
	dsRenderSurfaceUsage usage, unsigned int widthHint, unsigned int heightHint)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;
	VkSurfaceKHR surface;
	switch (type)
	{
		case dsRenderSurfaceType_Direct:
			// VkSurfaceKHR is a dispatch handle, which typically means a 64-bit integer, even on
			// 32-bit systems. However, it's generally defined as a pointer to a struct as provided
			// in vk_icd.h. This is the case for all currently supported platforms.
			surface = (VkSurfaceKHR)(uintptr_t)osHandle;
			break;
		default:
			surface = dsVkPlatform_createSurface(&vkRenderer->platform, displayHandle, osHandle);
			if (!surface)
				return NULL;
			break;
	}

	VkBool32 supported = false;
	VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceSupportKHR)(
		device->physicalDevice, device->queueFamilyIndex, surface, &supported);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't get surface support"))
	{
		if (type != dsRenderSurfaceType_Direct)
			dsVkPlatform_destroySurface(&vkRenderer->platform, surface);
		return NULL;
	}

	if (!supported)
	{
		if (type != dsRenderSurfaceType_Direct)
			dsVkPlatform_destroySurface(&vkRenderer->platform, surface);
		errno = EPERM;
		DS_LOG_INFO(DS_RENDER_VULKAN_LOG_TAG, "Window surface can't be rendered to.");
		return NULL;
	}

	VkSurfaceCapabilitiesKHR surfaceInfo;
	result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
		device->physicalDevice, surface, &surfaceInfo);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't get surface capabilities"))
	{
		if (type != dsRenderSurfaceType_Direct)
			dsVkPlatform_destroySurface(&vkRenderer->platform, surface);
		return NULL;
	}

	dsAdjustVkSurfaceCapabilities(&surfaceInfo, widthHint, heightHint);

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVkRenderSurface)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (type != dsRenderSurfaceType_Direct)
			dsVkPlatform_destroySurface(&vkRenderer->platform, surface);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVkRenderSurface* renderSurface = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkRenderSurface);
	DS_ASSERT(renderSurface);

	dsRenderSurface* baseRenderSurface = (dsRenderSurface*)renderSurface;
	baseRenderSurface->renderer = renderer;
	baseRenderSurface->allocator = dsAllocator_keepPointer(allocator);
	baseRenderSurface->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(baseRenderSurface->name);
	memcpy((void*)baseRenderSurface->name, name, nameLen);
	baseRenderSurface->surfaceType = type;
	baseRenderSurface->usage = usage;

	renderSurface->scratchAllocator = renderer->allocator;
	renderSurface->lifetime = NULL;
	renderSurface->surface = surface;
	renderSurface->surfaceData = NULL;
	renderSurface->surfaceError = false;
	renderSurface->updatedFrame = renderer->frameNumber - 1;
	DS_VERIFY(dsSpinlock_initialize(&renderSurface->lock));

	renderSurface->lifetime = dsLifetime_create(allocator, renderSurface);
	if (!renderSurface->lifetime)
	{
		dsVkRenderSurface_destroy(renderer, baseRenderSurface);
		return NULL;
	}

	renderSurface->surfaceData = dsVkRenderSurfaceData_create(renderSurface->scratchAllocator,
		renderer, surface, renderer->vsync, 0, usage, &surfaceInfo);
	if (!renderSurface->surfaceData)
	{
		dsVkRenderSurface_destroy(renderer, baseRenderSurface);
		return NULL;
	}

	baseRenderSurface->width = renderSurface->surfaceData->width;
	baseRenderSurface->height = renderSurface->surfaceData->height;
	baseRenderSurface->preRotateWidth = renderSurface->surfaceData->preRotateWidth;
	baseRenderSurface->preRotateHeight = renderSurface->surfaceData->preRotateHeight;
	baseRenderSurface->rotation = renderSurface->surfaceData->rotation;

	return baseRenderSurface;
}

bool dsVkRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface,
	unsigned int widthHint, unsigned int heightHint)
{
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));

	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	VkSurfaceCapabilitiesKHR surfaceInfo;
	VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
		device->physicalDevice, vkSurface->surface, &surfaceInfo);
	if (result == VK_SUCCESS)
		dsAdjustVkSurfaceCapabilities(&surfaceInfo, widthHint, heightHint);
	else
	{
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
		return DS_HANDLE_VK_RESULT(result, "Couldn't get surface capabilities");
	}

	if (vkSurface->surfaceData && !vkSurface->surfaceError &&
		vkSurface->surfaceData->vsync == renderer->vsync)
	{
		dsAdjustVkSurfaceCapabilities(&surfaceInfo, widthHint, heightHint);
		uint32_t width = surfaceInfo.currentExtent.width;
		uint32_t height = surfaceInfo.currentExtent.height;
		dsRenderSurfaceRotation rotation = dsRenderSurfaceRotation_0;
		if (renderSurface->usage & dsRenderSurfaceUsage_ClientRotations)
			rotation = dsVkRenderSurfaceData_getRotation(surfaceInfo.currentTransform);

		if (width == vkSurface->surfaceData->width &&
			height == vkSurface->surfaceData->height &&
			rotation == vkSurface->surfaceData->rotation)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return true;
		}
		else if (width == 0 || height == 0)
		{
			// Ignore if the size is 0. (e.g. minimized)
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return true;
		}
	}
	else
	{
		// If we didn't take the above code path, need to check for size of 0. (e.g. minimized)
		dsAdjustVkSurfaceCapabilities(&surfaceInfo, widthHint, heightHint);
		if (surfaceInfo.currentExtent.width == 0 || surfaceInfo.currentExtent.height == 0)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return true;
		}
	}

	// NOTE: Some systems need to wait until all of the render commands have come through before
	// re-creating the render surface.
	dsRenderer_waitUntilIdle(renderer);

	VkSwapchainKHR prevSwapchain = 0;
	if (vkSurface->surfaceData)
	{
		// Delete the previous surface data here regardless. If it fails to be created, the
		// swapchain will become invalid, so it can't be re-used.
		prevSwapchain = vkSurface->surfaceData->swapchain;
		vkSurface->surfaceData->swapchain = 0;
		dsVkRenderSurfaceData_destroy(vkSurface->surfaceData);
		vkSurface->surfaceData = NULL;
	}

	dsVkRenderSurfaceData* surfaceData = dsVkRenderSurfaceData_create(vkSurface->scratchAllocator,
		renderer, vkSurface->surface, renderer->vsync, prevSwapchain, renderSurface->usage,
		&surfaceInfo);
	if (prevSwapchain)
	{
		DS_VK_CALL(device->vkDestroySwapchainKHR)(device->device, prevSwapchain,
			instance->allocCallbacksPtr);
	}

	if (surfaceData)
	{
		vkSurface->surfaceData = surfaceData;

		renderSurface->width = vkSurface->surfaceData->width;
		renderSurface->height = vkSurface->surfaceData->height;
		renderSurface->preRotateWidth = vkSurface->surfaceData->preRotateWidth;
		renderSurface->preRotateHeight = vkSurface->surfaceData->preRotateHeight;
		renderSurface->rotation = vkSurface->surfaceData->rotation;
		vkSurface->surfaceError = false;
	}
	else
		vkSurface->surfaceError = true;
	DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));

	return surfaceData != NULL;
}

bool dsVkRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));

	// Only one udpate per frame.
	if (vkSurface->updatedFrame == renderer->frameNumber)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
		return transitionToRenderable(commandBuffer, vkSurface->surfaceData);
	}

	if (!vkSurface->surfaceData || vkSurface->surfaceError ||
		vkSurface->surfaceData->vsync != renderer->vsync)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
		errno = EPERM;
		return false;
	}

	dsVkSurfaceResult result = dsVkRenderSurfaceData_acquireImage(vkSurface->surfaceData);
	if (result != dsVkSurfaceResult_Success)
	{
		// Wait until next update to re-create surface.
		vkSurface->surfaceError = true;
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
		errno = EPERM;
		return false;
	}

	bool success = dsVkCommandBuffer_addRenderSurface(commandBuffer,
		vkSurface->surfaceData);
	if (success)
		vkSurface->updatedFrame = renderer->frameNumber;
	DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
	if (success)
		return transitionToRenderable(commandBuffer, vkSurface->surfaceData);
	return false;
}

bool dsVkRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	const dsVkRenderSurface* vkSurface = (const dsVkRenderSurface*)renderSurface;
	return transitionToPresentable(commandBuffer, vkSurface->surfaceData);
}

bool dsVkRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	uint32_t count)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	uint64_t submitCount = vkRenderer->submitCount;
	VkSemaphore semaphore = dsVkRenderer_flushImpl(renderer, true, true);
	VkSwapchainKHR* swapchains = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSwapchainKHR, count);
	uint32_t* imageIndices = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, count);
	for (uint32_t i = 0; i < count; ++i)
	{
		dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurfaces[i];
		DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));
		dsVkRenderSurfaceData* surfaceData = vkSurface->surfaceData;
		if (!surfaceData)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			errno = EAGAIN;
			return false;
		}

		// Update the submit count basedon the current submit.
		DS_VERIFY(dsSpinlock_lock(&surfaceData->resource.lock));
		surfaceData->resource.lastUsedSubmit = submitCount;
		DS_VERIFY(dsSpinlock_unlock(&surfaceData->resource.lock));

		swapchains[i] = surfaceData->swapchain;
		imageIndices[i] = surfaceData->imageIndex;
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
	}

	DS_PROFILE_SCOPE_START("vkQueuePresentKHR");
	VkPresentInfoKHR presentInfo =
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		NULL,
		1, &semaphore,
		count, swapchains, imageIndices,
		NULL
	};

	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkResult result = DS_VK_CALL(device->vkQueuePresentKHR)(device->queue, &presentInfo);
	DS_PROFILE_SCOPE_END();
	return DS_HANDLE_VK_RESULT(result, "Couldn't queue present");
}

bool dsVkRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	dsLifetime_destroy(vkSurface->lifetime);

	// Make sure all draw calls to the surface are finished.
	dsRenderer_waitUntilIdle(renderer);

	dsVkRenderSurfaceData_destroy(vkSurface->surfaceData);
	if (vkSurface->surface && renderSurface->surfaceType != dsRenderSurfaceType_Direct)
		dsVkPlatform_destroySurface(&vkRenderer->platform, vkSurface->surface);
	dsSpinlock_shutdown(&vkSurface->lock);
	if (renderSurface->allocator)
		DS_VERIFY(dsAllocator_free(renderSurface->allocator, renderSurface));

	return true;
}
