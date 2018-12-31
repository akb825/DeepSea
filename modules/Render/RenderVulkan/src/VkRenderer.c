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
#include "VkRendererInternal.h"

#include "Platform/VkPlatform.h"
#include "Resources/VkComputePipeline.h"
#include "Resources/VkCopyImage.h"
#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkGfxFence.h"
#include "Resources/VkGfxQueryPool.h"
#include "Resources/VkMaterialDescriptor.h"
#include "Resources/VkPipeline.h"
#include "Resources/VkRealFramebuffer.h"
#include "Resources/VkRenderbuffer.h"
#include "Resources/VkResource.h"
#include "Resources/VkResourceManager.h"
#include "Resources/VkSamplerList.h"
#include "Resources/VkTexture.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkCommandBufferPool.h"
#include "VkCommandPoolData.h"
#include "VkInit.h"
#include "VkRenderSurface.h"
#include "VkRenderSurfaceData.h"
#include "VkResourceList.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

static size_t fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsVkRenderer)) + dsMutex_fullAllocSize() +
		dsConditionVariable_fullAllocSize();
}

static bool createCommandBuffers(dsVkRenderer* renderer)
{
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;
	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		device->queueFamilyIndex
	};
	VkResult result = DS_VK_CALL(device->vkCreateCommandPool)(device->device,
		&commandPoolCreateInfo, instance->allocCallbacksPtr, &renderer->commandPool);
	if (!dsHandleVkResult(result))
		return false;

	VkCommandBufferAllocateInfo commandAllocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		renderer->commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		2
	};

	VkFenceCreateInfo fenceCreateInfo =
	{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		0
	};

	VkSemaphoreCreateInfo semaphoreCreateInfo =
	{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		NULL,
		0
	};

	for (uint32_t i = 0; i < DS_MAX_SUBMITS; ++i)
	{
		dsVkSubmitInfo* submit = renderer->submits + i;
		submit->submitIndex = DS_NOT_SUBMITTED;
		result = DS_VK_CALL(device->vkAllocateCommandBuffers)(device->device, &commandAllocateInfo,
			&submit->resourceCommands);
		if (!dsHandleVkResult(result))
			return false;

		result = DS_VK_CALL(device->vkCreateFence)(device->device, &fenceCreateInfo,
			instance->allocCallbacksPtr, &submit->fence);
		if (!dsHandleVkResult(result))
			return false;

		result = DS_VK_CALL(device->vkCreateSemaphore)(device->device, &semaphoreCreateInfo,
			instance->allocCallbacksPtr, &submit->semaphore);
		if (!dsHandleVkResult(result))
			return false;
	}

	// Start at submit count 1 so it's ahead of the finished index.
	renderer->submitCount = 1;

	// Set up the main command buffer.
	dsVkSubmitInfo* firstSubmit = renderer->submits + renderer->curSubmit;
	dsVkCommandBuffer* mainCommandBuffer = &renderer->mainCommandBuffer;
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)mainCommandBuffer;
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	baseCommandBuffer->allocator = baseRenderer->allocator;
	mainCommandBuffer->vkCommandBuffer = firstSubmit->renderCommands;
	baseRenderer->mainCommandBuffer = baseCommandBuffer;

	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};
	DS_VK_CALL(device->vkBeginCommandBuffer)(firstSubmit->resourceCommands, &beginInfo);
	DS_VK_CALL(device->vkBeginCommandBuffer)(firstSubmit->renderCommands, &beginInfo);

	return true;
}

static void freeAllResources(dsVkResourceList* deleteList)
{
	for (uint32_t i = 0; i < deleteList->bufferCount; ++i)
		dsVkGfxBufferData_destroy(deleteList->buffers[i]);

	for (uint32_t i = 0; i < deleteList->textureCount; ++i)
		dsVkTexture_destroyImpl(deleteList->textures[i]);

	for (uint32_t i = 0; i < deleteList->copyImageCount; ++i)
		dsVkCopyImage_destroy(deleteList->copyImages[i]);

	for (uint32_t i = 0; i < deleteList->renderbufferCount; ++i)
		dsVkRenderbuffer_destroyImpl(deleteList->renderbuffers[i]);

	for (uint32_t i = 0; i < deleteList->framebufferCount; ++i)
		dsVkRealFramebuffer_destroy(deleteList->framebuffers[i]);

	for (uint32_t i = 0; i < deleteList->fenceCount; ++i)
		dsVkGfxFence_destroyImpl(deleteList->fences[i]);

	for (uint32_t i = 0; i < deleteList->queryCount; ++i)
		dsVkGfxQueryPool_destroyImpl(deleteList->queries[i]);

	for (uint32_t i = 0; i < deleteList->descriptorCount; ++i)
		dsVkMaterialDescriptor_destroy(deleteList->descriptors[i]);

	for (uint32_t i = 0; i < deleteList->samplerCount; ++i)
		dsVkSamplerList_destroy(deleteList->samplers[i]);

	for (uint32_t i = 0; i < deleteList->computePipelineCount; ++i)
		dsVkComputePipeline_destroy(deleteList->computePipelines[i]);

	for (uint32_t i = 0; i < deleteList->pipelineCount; ++i)
		dsVkPipeline_destroy(deleteList->pipelines[i]);

	for (uint32_t i = 0; i < deleteList->renderSurfaceCount; ++i)
		dsVkRenderSurfaceData_destroy(deleteList->renderSurfaces[i]);

	for (uint32_t i = 0; i < deleteList->commandPoolCount; ++i)
		dsVkCommandPoolData_destroy(deleteList->commandPools[i]);

	dsVkResourceList_clear(deleteList);
}

static void freeResources(dsVkRenderer* renderer)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;

	DS_VERIFY(dsSpinlock_lock(&renderer->deleteLock));
	dsVkResourceList* prevDeleteList = renderer->deleteResources + renderer->curDeleteResources;
	renderer->curDeleteResources = (renderer->curDeleteResources + 1) % DS_DELETE_RESOURCES_ARRAY;
	DS_VERIFY(dsSpinlock_unlock(&renderer->deleteLock));

	for (uint32_t i = 0; i < prevDeleteList->bufferCount; ++i)
	{
		dsVkGfxBufferData* buffer = prevDeleteList->buffers[i];
		DS_ASSERT(buffer);

		bool stillInUse = dsVkResource_isInUse(&buffer->resource, baseRenderer) ||
			(buffer->uploadedSubmit != DS_NOT_SUBMITTED &&
				buffer->uploadedSubmit > renderer->finishedSubmitCount);
		if (stillInUse)
		{
			dsVkRenderer_deleteGfxBuffer(baseRenderer, buffer);
			continue;
		}

		dsVkGfxBufferData_destroy(buffer);
	}

	for (uint32_t i = 0; i < prevDeleteList->textureCount; ++i)
	{
		dsTexture* texture = prevDeleteList->textures[i];
		DS_ASSERT(texture);
		dsVkTexture* vkTexture = (dsVkTexture*)texture;

		bool stillInUse = dsVkResource_isInUse(&vkTexture->resource, baseRenderer) ||
			(vkTexture->uploadedSubmit != DS_NOT_SUBMITTED &&
				vkTexture->uploadedSubmit > renderer->finishedSubmitCount) ||
			(vkTexture->lastDrawSubmit != DS_NOT_SUBMITTED &&
				vkTexture->lastDrawSubmit > renderer->finishedSubmitCount);
		if (stillInUse)
		{
			dsVkRenderer_deleteTexture(baseRenderer, texture);
			continue;
		}

		dsVkTexture_destroyImpl(texture);
	}

	for (uint32_t i = 0; i < prevDeleteList->copyImageCount; ++i)
	{
		dsVkCopyImage* copyImage = prevDeleteList->copyImages[i];
		DS_ASSERT(copyImage);

		if (dsVkResource_isInUse(&copyImage->resource, baseRenderer))
		{
			dsVkRenderer_deleteCopyImage(baseRenderer, copyImage);
			continue;
		}

		dsVkCopyImage_destroy(copyImage);
	}

	for (uint32_t i = 0; i < prevDeleteList->renderbufferCount; ++i)
	{
		dsRenderbuffer* renderbuffer = prevDeleteList->renderbuffers[i];
		DS_ASSERT(renderbuffer);
		dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;

		if (dsVkResource_isInUse(&vkRenderbuffer->resource, baseRenderer))
		{
			dsVkRenderer_deleteRenderbuffer(baseRenderer, renderbuffer);
			continue;
		}

		dsVkRenderbuffer_destroyImpl(renderbuffer);
	}

	for (uint32_t i = 0; i < prevDeleteList->framebufferCount; ++i)
	{
		dsVkRealFramebuffer* framebuffer = prevDeleteList->framebuffers[i];
		DS_ASSERT(framebuffer);

		if (dsVkResource_isInUse(&framebuffer->resource, baseRenderer))
		{
			dsVkRenderer_deleteFramebuffer(baseRenderer, framebuffer);
			continue;
		}

		dsVkRealFramebuffer_destroy(framebuffer);
	}

	for (uint32_t i = 0; i < prevDeleteList->fenceCount; ++i)
	{
		dsGfxFence* fence = prevDeleteList->fences[i];
		DS_ASSERT(fence);
		dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;

		if (dsVkResource_isInUse(&vkFence->resource, baseRenderer))
		{
			dsVkRenderer_deleteFence(baseRenderer, fence);
			continue;
		}

		dsVkGfxFence_destroyImpl(fence);
	}

	for (uint32_t i = 0; i < prevDeleteList->queryCount; ++i)
	{
		dsGfxQueryPool* queries = prevDeleteList->queries[i];
		DS_ASSERT(queries);
		dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;

		if (dsVkResource_isInUse(&vkQueries->resource, baseRenderer))
		{
			dsVkRenderer_deleteQueriePool(baseRenderer, queries);
			continue;
		}

		dsVkGfxQueryPool_destroyImpl(queries);
	}

	for (uint32_t i = 0; i < prevDeleteList->descriptorCount; ++i)
	{
		dsVkMaterialDescriptor* descriptor = prevDeleteList->descriptors[i];
		DS_ASSERT(descriptor);

		if (dsVkResource_isInUse(&descriptor->resource, baseRenderer))
		{
			dsVkRenderer_deleteMaterialDescriptor(baseRenderer, descriptor);
			continue;
		}

		dsVkMaterialDescriptor_destroy(descriptor);
	}

	for (uint32_t i = 0; i < prevDeleteList->samplerCount; ++i)
	{
		dsVkSamplerList* samplers = prevDeleteList->samplers[i];
		DS_ASSERT(samplers);

		if (dsVkResource_isInUse(&samplers->resource, baseRenderer))
		{
			dsVkRenderer_deleteSamplerList(baseRenderer, samplers);
			continue;
		}

		dsVkSamplerList_destroy(samplers);
	}

	for (uint32_t i = 0; i < prevDeleteList->computePipelineCount; ++i)
	{
		dsVkComputePipeline* pipeline = prevDeleteList->computePipelines[i];
		DS_ASSERT(pipeline);

		if (dsVkResource_isInUse(&pipeline->resource, baseRenderer))
		{
			dsVkRenderer_deleteComputePipeline(baseRenderer, pipeline);
			continue;
		}

		dsVkComputePipeline_destroy(pipeline);
	}

	for (uint32_t i = 0; i < prevDeleteList->pipelineCount; ++i)
	{
		dsVkPipeline* pipeline = prevDeleteList->pipelines[i];
		DS_ASSERT(pipeline);

		if (dsVkResource_isInUse(&pipeline->resource, baseRenderer))
		{
			dsVkRenderer_deletePipeline(baseRenderer, pipeline);
			continue;
		}

		dsVkPipeline_destroy(pipeline);
	}

	for (uint32_t i = 0; i < prevDeleteList->renderSurfaceCount; ++i)
	{
		dsVkRenderSurfaceData* surface = prevDeleteList->renderSurfaces[i];
		DS_ASSERT(surface);

		if (dsVkResource_isInUse(&surface->resource, baseRenderer))
		{
			dsVkRenderer_deleteRenderSurface(baseRenderer, surface);
			continue;
		}

		dsVkRenderSurfaceData_destroy(surface);
	}

	for (uint32_t i = 0; i < prevDeleteList->commandPoolCount; ++i)
	{
		dsVkCommandPoolData* pool = prevDeleteList->commandPools[i];
		DS_ASSERT(pool);

		if (dsVkResource_isInUse(&pool->resource, baseRenderer))
		{
			dsVkRenderer_deleteCommandPool(baseRenderer, pool);
			continue;
		}

		dsVkCommandPoolData_destroy(pool);
	}

	dsVkResourceList_clear(prevDeleteList);
}

static bool addBufferCopies(dsVkRenderer* renderer, dsVkGfxBufferData* buffer,
	const dsVkDirtyRange* dirtyRanges, uint32_t dirtyRangeCount)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkBarrierList* preResourceBarriers = &renderer->preResourceBarriers;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	uint32_t firstCopy = renderer->bufferCopiesCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->bufferCopies,
		renderer->bufferCopiesCount, renderer->maxBufferCopies, dirtyRangeCount))
	{
		return false;
	}

	bool isStatic = dsVkGfxBufferData_isStatic(buffer);
	for (uint32_t i = 0; i < dirtyRangeCount; ++i)
	{
		VkBufferCopy* copyInfo = renderer->bufferCopies + firstCopy + i;
		const dsVkDirtyRange* dirtyRange = dirtyRanges + i;
		copyInfo->srcOffset = copyInfo->dstOffset = dirtyRange->start;
		copyInfo->size = dirtyRange->size;

		// Need a barrier before. If the buffer is static, have a memory barrier after
		// the copy to avoid needing barriers for each usage.
		dsVkBarrierList_addBufferBarrier(preResourceBarriers, buffer->hostBuffer,
			dirtyRange->start, dirtyRange->size, 0, dsGfxBufferUsage_CopyFrom, true);
		if (isStatic)
		{
			dsVkBarrierList_addBufferBarrier(postResourceBarriers, buffer->deviceBuffer,
				dirtyRange->start, dirtyRange->size, dsGfxBufferUsage_CopyTo,
				buffer->usage, false);
		}
	}

	uint32_t curInfo = renderer->bufferCopyInfoCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->bufferCopyInfos,
		renderer->bufferCopyInfoCount, renderer->maxBufferCopyInfos, 1))
	{
		renderer->bufferCopiesCount = firstCopy;
		return false;
	}

	dsVkBufferCopyInfo* copyInfo = renderer->bufferCopyInfos + curInfo;
	copyInfo->srcBuffer = buffer->hostBuffer;
	copyInfo->dstBuffer = buffer->deviceBuffer;
	copyInfo->firstRange = firstCopy;
	copyInfo->rangeCount = dirtyRangeCount;
	return true;
}

static bool addImageCopies(dsVkRenderer* renderer, dsVkTexture* texture)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsTexture* baseTexture = (dsTexture*)texture;
	const dsTextureInfo* info = &baseTexture->info;
	uint32_t faceCount = info->dimension == dsTextureDim_Cube ? 6 : 1;
	bool is3D = info->dimension == dsTextureDim_3D;
	dsVkBarrierList* preResourceBarriers = &renderer->preResourceBarriers;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	VkImageSubresourceRange fullLayout = {texture->aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0,
		VK_REMAINING_ARRAY_LAYERS};
	if (texture->hostImage)
	{
		uint32_t index = renderer->imageCopyCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->imageCopies,
			renderer->imageCopyCount, renderer->maxImageCopies, info->mipLevels))
		{
			return false;
		}

		uint32_t infoIndex = renderer->imageCopyInfoCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->imageCopyInfos,
			renderer->imageCopyInfoCount, renderer->maxImageCopyInfos, 1))
		{
			renderer->imageCopyCount = index;
			return false;
		}

		dsVkImageCopyInfo* copyInfo = renderer->imageCopyInfos + infoIndex;
		copyInfo->srcImage = texture->hostImage;
		copyInfo->dstImage = texture->deviceImage;
		copyInfo->srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyInfo->dstLayout = VK_IMAGE_LAYOUT_GENERAL;
		copyInfo->firstRange = index;
		copyInfo->rangeCount = info->mipLevels;

		for (uint32_t i = 0; i < info->mipLevels; ++i)
		{
			uint32_t width = info->width >> i;
			uint32_t height = info->height >> i;
			uint32_t depth = is3D ? info->depth >> i : info->depth;
			width = dsMax(1U, width);
			height = dsMax(1U, height);
			depth = dsMax(1U, depth);

			uint32_t layerCount = faceCount*(is3D ? 1U : depth);
			VkImageCopy* imageCopy = renderer->imageCopies + index + i;
			imageCopy->srcSubresource.aspectMask = texture->aspectMask;
			imageCopy->srcSubresource.mipLevel = 0;
			imageCopy->srcSubresource.baseArrayLayer = 0;
			imageCopy->srcSubresource.layerCount = layerCount;
			imageCopy->srcOffset.x = 0;
			imageCopy->srcOffset.y = 0;
			imageCopy->srcOffset.z = 0;
			imageCopy->dstSubresource = imageCopy->srcSubresource;
			imageCopy->dstOffset = imageCopy->srcOffset;
			imageCopy->extent.width = width;
			imageCopy->extent.height = height;
			imageCopy->extent.depth = is3D ? depth : 1U;
		}

		dsVkBarrierList_addImageBarrier(preResourceBarriers, texture->hostImage, &fullLayout,
			0, true, false, false, dsTextureUsage_CopyTo, VK_IMAGE_LAYOUT_PREINITIALIZED,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	else
	{
		DS_ASSERT(texture->hostImageCount > 0);
		uint32_t index = renderer->imageCopyCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->imageCopies,
			renderer->imageCopyCount, renderer->maxImageCopies, texture->hostImageCount))
		{
			return false;
		}

		uint32_t infoIndex = renderer->imageCopyInfoCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(baseRenderer->allocator, renderer->imageCopyInfos,
			renderer->imageCopyInfoCount, renderer->maxImageCopyInfos, texture->hostImageCount))
		{
			renderer->imageCopyCount = index;
			return false;
		}

		for (uint32_t i = 0, imageIndex = 0; i < info->mipLevels; ++i)
		{
			uint32_t width = info->width >> i;
			uint32_t height = info->height >> i;
			uint32_t depth = is3D ? info->depth >> i : info->depth;
			width = dsMax(1U, width);
			height = dsMax(1U, height);
			depth = dsMax(1U, depth);
			for (uint32_t j = 0; j < depth; ++j)
			{
				for (uint32_t k = 0; k < faceCount; ++k, ++index, ++infoIndex, ++imageIndex)
				{
					DS_ASSERT(index < renderer->imageCopyCount);
					DS_ASSERT(infoIndex < renderer->imageCopyInfoCount);

					VkImageCopy* imageCopy = renderer->imageCopies + index;
					imageCopy->srcSubresource.aspectMask = texture->aspectMask;
					imageCopy->srcSubresource.mipLevel = 0;
					imageCopy->srcSubresource.baseArrayLayer = 0;
					imageCopy->srcSubresource.layerCount = 1;
					imageCopy->srcOffset.x = 0;
					imageCopy->srcOffset.y = 0;
					imageCopy->srcOffset.z = 0;
					imageCopy->dstSubresource.aspectMask = texture->aspectMask;
					imageCopy->dstSubresource.mipLevel = i;
					imageCopy->dstSubresource.baseArrayLayer = j*faceCount + k;
					imageCopy->dstSubresource.layerCount = 1;
					imageCopy->dstOffset.x = 0;
					imageCopy->dstOffset.y = 0;
					imageCopy->dstOffset.z = is3D ? j : 0;
					imageCopy->extent.width = width;
					imageCopy->extent.height = height;
					imageCopy->extent.depth = 1U;

					VkImage hostImage = texture->hostImages[imageIndex].image;
					dsVkImageCopyInfo* copyInfo = renderer->imageCopyInfos + infoIndex;
					copyInfo->srcImage = hostImage;
					copyInfo->dstImage = texture->deviceImage;
					copyInfo->srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					copyInfo->dstLayout = VK_IMAGE_LAYOUT_GENERAL;
					copyInfo->firstRange = index;
					copyInfo->rangeCount = info->mipLevels;

					dsVkBarrierList_addImageBarrier(preResourceBarriers, hostImage, &fullLayout, 0,
						true, false, false, dsTextureUsage_CopyTo, VK_IMAGE_LAYOUT_PREINITIALIZED,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				}
			}
		}

		DS_ASSERT(index == renderer->imageCopyCount);
		DS_ASSERT(infoIndex == renderer->imageCopyInfoCount);
	}

	// Even non-static images will have a barrier to process the layout conversion.
	dsVkBarrierList_addImageBarrier(postResourceBarriers, texture->deviceImage, &fullLayout,
		dsTextureUsage_CopyFrom, false, baseTexture->offscreen,
		dsGfxFormat_isDepthStencil(info->format), baseTexture->usage, VK_IMAGE_LAYOUT_GENERAL,
		dsVkTexture_imageLayout(baseTexture));

	return true;
}

static void processBuffers(dsVkRenderer* renderer, dsVkResourceList* resourceList)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < resourceList->bufferCount; ++i)
	{
		dsVkGfxBufferData* buffer = resourceList->buffers[i];
		if (!buffer->deviceBuffer || !buffer->hostBuffer)
			continue;

		DS_VERIFY(dsSpinlock_lock(&buffer->resource.lock));
		// Clear the submit queue now that we're processing it.
		buffer->submitQueue = NULL;
		if (buffer->mappedSize > 0)
		{
			// Still mapped, process later.
			DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));
			dsVkRenderer_processGfxBuffer(baseRenderer, buffer);
			continue;
		}

		// Record the ranges to copy.
		bool doUpload = false;
		if (buffer->needsInitialCopy)
		{
			DS_ASSERT(buffer->dirtyRangeCount == 0);
			doUpload = true;
			dsVkDirtyRange dirtyRange = {0, buffer->size};
			addBufferCopies(renderer, buffer, &dirtyRange, 1);
			buffer->needsInitialCopy = false;
		}
		else if (buffer->dirtyRangeCount > 0)
		{
			doUpload = true;
			addBufferCopies(renderer, buffer, buffer->dirtyRanges, buffer->dirtyRangeCount);
			buffer->dirtyRangeCount = 0;
		}

		// Record when the latest copy occurred. If no copy to process, then see if we can destroy
		// the host memory. (i.e. it was only used for the initial data)
		VkDeviceMemory hostMemory = 0;
		VkBuffer hostBuffer = 0;
		if (doUpload)
			buffer->uploadedSubmit = renderer->submitCount;
		else if (buffer->hostBuffer && !buffer->keepHost &&
			buffer->uploadedSubmit <= renderer->finishedSubmitCount)
		{
			hostMemory = buffer->hostMemory;
			hostBuffer = buffer->hostBuffer;
			buffer->hostBuffer = 0;
			buffer->hostMemory = 0;
		}
		DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));

		// If we don't keep the host memory, either re-queue to do the deletion if we did the copy,
		// otherwise perform the deletion.
		if (!buffer->keepHost)
		{
			if (hostBuffer)
			{
				DS_ASSERT(!doUpload);
				DS_VK_CALL(device->vkDestroyBuffer)(device->device, hostBuffer,
					instance->allocCallbacksPtr);
				DS_VK_CALL(device->vkFreeMemory)(device->device, hostMemory,
					instance->allocCallbacksPtr);
			}
			else
				dsVkRenderer_processGfxBuffer(baseRenderer, buffer);
		}
	}
}

static void processTextures(dsVkRenderer* renderer, dsVkResourceList* resourceList)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < resourceList->textureCount; ++i)
	{
		dsTexture* texture = resourceList->textures[i];
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		if (!vkTexture->hostImage && vkTexture->hostImageCount == 0)
			continue;

		DS_VERIFY(dsSpinlock_lock(&vkTexture->resource.lock));
		// Clear the submit queue now that we're processing it.
		vkTexture->submitQueue = NULL;
		bool doUpload = false;
		if (vkTexture->needsInitialCopy)
		{
			doUpload = true;
			addImageCopies(renderer, vkTexture);
			vkTexture->needsInitialCopy = false;
		}

		DS_VERIFY(dsSpinlock_unlock(&vkTexture->resource.lock));

		// Queue for re-processing if we still need to delete the host image.
		if (doUpload || vkTexture->uploadedSubmit > renderer->finishedSubmitCount)
			dsVkRenderer_processTexture(baseRenderer, texture);
		else
		{
			if (vkTexture->hostImage)
			{
				DS_ASSERT(vkTexture->hostImageCount == 0);
				DS_VK_CALL(device->vkDestroyImage)(device->device, vkTexture->hostImage,
					instance->allocCallbacksPtr);
				vkTexture->hostImage = 0;
			}
			else
			{
				for (uint32_t j = 0; j < vkTexture->hostImageCount; ++j)
				{
					DS_VK_CALL(device->vkDestroyImage)(device->device,
						vkTexture->hostImages[j].image, instance->allocCallbacksPtr);
				}
				vkTexture->hostImageCount = 0;
			}
			DS_VK_CALL(device->vkFreeMemory)(device->device, vkTexture->hostMemory,
				instance->allocCallbacksPtr);
			vkTexture->hostMemory = 0;
		}
	}
}

static void processResources(dsVkRenderer* renderer, VkCommandBuffer commandBuffer)
{
	dsVkDevice* device = &renderer->device;
	dsVkBarrierList* preResourceBarriers = &renderer->preResourceBarriers;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	DS_VERIFY(dsSpinlock_lock(&renderer->resourceLock));
	dsVkResourceList* prevResourceList = renderer->pendingResources + renderer->curPendingResources;
	renderer->curPendingResources =
		(renderer->curPendingResources + 1) % DS_PENDING_RESOURCES_ARRAY;
	DS_VERIFY(dsSpinlock_unlock(&renderer->resourceLock));

	// Clear everything out.
	renderer->bufferCopiesCount = 0;
	renderer->bufferCopyInfoCount = 0;

	preResourceBarriers->bufferBarrierCount = 0;
	postResourceBarriers->bufferBarrierCount = 0;

	processBuffers(renderer, prevResourceList);
	processTextures(renderer, prevResourceList);

	// Process the uploads.
	if (preResourceBarriers->bufferBarrierCount > 0)
	{
		DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
			preResourceBarriers->bufferBarrierCount, preResourceBarriers->bufferBarriers,
			preResourceBarriers->imageBarrierCount, preResourceBarriers->imageBarriers);
	}

	for (uint32_t i = 0; i < renderer->bufferCopyInfoCount; ++i)
	{
		const dsVkBufferCopyInfo* copyInfo = renderer->bufferCopyInfos + i;
		DS_VK_CALL(device->vkCmdCopyBuffer)(commandBuffer, copyInfo->srcBuffer, copyInfo->dstBuffer,
			copyInfo->rangeCount, renderer->bufferCopies + copyInfo->firstRange);
	}

	for (uint32_t i = 0; i < renderer->imageCopyInfoCount; ++i)
	{
		const dsVkImageCopyInfo* copyInfo = renderer->imageCopyInfos + i;
		DS_VK_CALL(device->vkCmdCopyImage)(commandBuffer, copyInfo->srcImage, copyInfo->srcLayout,
			copyInfo->dstImage, copyInfo->dstLayout, copyInfo->rangeCount,
			renderer->imageCopies + copyInfo->firstRange);
	}

	if (postResourceBarriers->bufferBarrierCount > 0)
	{
		DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL,
			postResourceBarriers->bufferBarrierCount, postResourceBarriers->bufferBarriers,
			postResourceBarriers->imageBarrierCount, postResourceBarriers->imageBarriers);
	}

	dsVkResourceList_clear(prevResourceList);
}

void dsVkRenderer_flush(dsRenderer* renderer)
{
	dsVkRenderer_flushImpl(renderer, true);
}

bool dsVkRenderer_destroy(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;

	DS_VK_CALL(device->vkQueueWaitIdle)(device->queue);

	for (uint32_t i = 0; i < DS_MAX_SUBMITS; ++i)
	{
		dsVkSubmitInfo* submit = vkRenderer->submits + i;
		if (submit->fence)
		{
			DS_VK_CALL(device->vkDestroyFence)(device->device, submit->fence,
				instance->allocCallbacksPtr);
		}

		if (submit->semaphore)
		{
			DS_VK_CALL(device->vkDestroySemaphore)(device->device, submit->semaphore,
				instance->allocCallbacksPtr);
		}
	}

	if (vkRenderer->commandPool)
	{
		DS_VK_CALL(device->vkDestroyCommandPool)(device->device, vkRenderer->commandPool,
			instance->allocCallbacksPtr);
	}

	dsVkCommandBuffer_shutdown(&vkRenderer->mainCommandBuffer);

	dsVkBarrierList_shutdown(&vkRenderer->preResourceBarriers);
	dsVkBarrierList_shutdown(&vkRenderer->postResourceBarriers);
	for (unsigned int i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkResourceList_shutdown(&vkRenderer->pendingResources[i]);
	for (unsigned int i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
	{
		dsVkResourceList* deleteResources = vkRenderer->deleteResources + i;
		freeAllResources(deleteResources);
		dsVkResourceList_shutdown(deleteResources);
	}

	dsVkResourceManager_destroy(renderer->resourceManager);
	dsVkPlatform_shutdown(&vkRenderer->platform);
	dsDestroyVkDevice(device);
	dsDestroyVkInstance(&device->instance);
	dsSpinlock_shutdown(&vkRenderer->resourceLock);
	dsSpinlock_shutdown(&vkRenderer->deleteLock);
	dsMutex_destroy(vkRenderer->submitLock);
	dsConditionVariable_destroy(vkRenderer->waitCondition);

	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->bufferCopies));
	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->bufferCopyInfos));
	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->imageCopies));
	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->imageCopyInfos));
	DS_VERIFY(dsAllocator_free(renderer->allocator, renderer));
	return true;
}

bool dsVkRenderer_isSupported(void)
{
	static int supported = -1;
	if (supported >= 0)
		return (bool)supported;

	dsVkInstance instance;
	memset(&instance, 0, sizeof(dsVkInstance));
	supported = dsCreateVkInstance(&instance, NULL, false);
	if (supported)
		supported = dsGatherVkPhysicalDevices(&instance);
	dsDestroyVkInstance(&instance);
	return (bool)supported;
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

	size_t bufferSize = fullAllocSize();
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

	DS_VERIFY(dsSpinlock_initialize(&renderer->resourceLock));
	DS_VERIFY(dsSpinlock_initialize(&renderer->deleteLock));
	renderer->submitLock = dsMutex_create((dsAllocator*)&bufferAlloc, "Vulkan submit");
	DS_ASSERT(renderer->submitLock);
	renderer->waitCondition = dsConditionVariable_create((dsAllocator*)&bufferAlloc,
		"Fence wait");
	DS_ASSERT(renderer->waitCondition);

	if (!dsCreateVkInstance(&renderer->device.instance, options, true) ||
		!dsCreateVkDevice(&renderer->device, allocator, options))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (!dsVkPlatform_initialize(&renderer->platform, &renderer->device, options->platform,
		options->display))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	dsVkCommandBuffer_initialize(&renderer->mainCommandBuffer, baseRenderer, allocator,
		dsCommandBufferUsage_OcclusionQueries);

	dsVkBarrierList_initialize(&renderer->preResourceBarriers, allocator, &renderer->device);
	dsVkBarrierList_initialize(&renderer->postResourceBarriers, allocator, &renderer->device);
	for (uint32_t i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkResourceList_initialize(renderer->pendingResources + i, allocator);
	for (uint32_t i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
		dsVkResourceList_initialize(renderer->deleteResources + i, allocator);

	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	baseRenderer->platform = options->platform;
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
	baseRenderer->maxColorAttachments = dsMin(limits->maxColorAttachments, DS_MAX_ATTACHMENTS);
	// framebufferColorSampleCounts is a bitmask. Compute the maximum bit that's set.
	baseRenderer->maxSurfaceSamples = 1U << (31 - dsClz(limits->framebufferColorSampleCounts));
	baseRenderer->maxSurfaceSamples = dsMin(baseRenderer->maxSurfaceSamples,
		DS_MAX_ANTIALIAS_SAMPLES);
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
	baseRenderer->supportsStartInstance = (bool)deviceFeatures.drawIndirectFirstInstance;
	baseRenderer->defaultAnisotropy = 1;

	baseRenderer->resourceManager = dsVkResourceManager_create(allocator, renderer,
		options->shaderCacheDir);
	if (!baseRenderer->resourceManager)
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	if (!createCommandBuffers(renderer))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	baseRenderer->destroyFunc = &dsVkRenderer_destroy;

	// Render surfaces
	baseRenderer->createRenderSurfaceFunc = &dsVkRenderSurface_create;
	baseRenderer->destroyRenderSurfaceFunc = &dsVkRenderSurface_destroy;
	baseRenderer->updateRenderSurfaceFunc = &dsVkRenderSurface_update;
	baseRenderer->beginRenderSurfaceFunc = &dsVkRenderSurface_beginDraw;
	baseRenderer->endRenderSurfaceFunc = &dsVkRenderSurface_endDraw;
	baseRenderer->swapRenderSurfaceBuffersFunc = &dsVkRenderSurface_swapBuffers;

	// Command buffer pools
	baseRenderer->createCommandBufferPoolFunc = &dsVkCommandBufferPool_create;
	baseRenderer->destroyCommandBufferPoolFunc = &dsVkCommandBufferPool_destroy;
	baseRenderer->resetCommandBufferPoolFunc = &dsVkCommandBufferPool_reset;

	// Command buffers
	baseRenderer->beginCommandBufferFunc = &dsVkCommandBuffer_begin;
	baseRenderer->endCommandBufferFunc = &dsVkCommandBuffer_end;
	baseRenderer->submitCommandBufferFunc = &dsVkCommandBuffer_submit;

	return baseRenderer;
}

VkSemaphore dsVkRenderer_flushImpl(dsRenderer* renderer, bool readback)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	// Get the submit queue, waiting if it's not ready.
	dsVkSubmitInfo* submit = vkRenderer->submits + vkRenderer->curSubmit;
	if (submit->submitIndex != DS_NOT_SUBMITTED)
	{
		DS_VK_CALL(device->vkWaitForFences)(device->device, 1, &submit->fence, true,
			DS_DEFAULT_WAIT_TIMEOUT);
		vkRenderer->finishedSubmitCount = submit->submitIndex;
	}

	// Free resources that are waiting to be in an unused state.
	freeResources(vkRenderer);
	// Process currently pending resources.
	processResources(vkRenderer, submit->resourceCommands);

	// Advance the submits.
	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
	if (submit->submitIndex != DS_NOT_SUBMITTED)
	{
		// Wait until any remaining fence waits have finished to avoid resetting while another
		// thread uses it.
		while (vkRenderer->waitCount > 0)
			dsConditionVariable_wait(vkRenderer->waitCondition, vkRenderer->submitLock);
		DS_VK_CALL(device->vkResetFences)(device->device, 1, &submit->fence);
	}
	submit->submitIndex = vkRenderer->submitCount++;
	vkRenderer->curSubmit = (vkRenderer->curSubmit + 1) % DS_MAX_SUBMITS;
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));

	// Submit the queue.
	DS_VK_CALL(device->vkEndCommandBuffer)(submit->resourceCommands);

	if (readback)
		dsVkCommandBuffer_endSubmitCommands(renderer->mainCommandBuffer, submit->renderCommands);
	DS_VK_CALL(device->vkEndCommandBuffer)(submit->renderCommands);

	dsVkCommandBuffer* mainCommandBuffer = &vkRenderer->mainCommandBuffer;
	DS_ASSERT(mainCommandBuffer->vkCommandBuffer == submit->renderCommands);
	VkSemaphore* waitSemaphores = NULL;
	VkPipelineStageFlags* waitStages = NULL;
	if (mainCommandBuffer->renderSurfaceCount > 0)
	{
		waitSemaphores = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSemaphore,
			mainCommandBuffer->renderSurfaceCount);
		waitStages = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkPipelineStageFlags,
			mainCommandBuffer->renderSurfaceCount);
		for (uint32_t i = 0; i < mainCommandBuffer->renderSurfaceCount; ++i)
		{
			dsVkRenderSurfaceData* surface = mainCommandBuffer->renderSurfaces[i];
			waitSemaphores[i] = surface->imageData[surface->imageDataIndex].semaphore;
			waitStages[i] = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
	}

	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		mainCommandBuffer->renderSurfaceCount, waitSemaphores, waitStages,
		2, &submit->resourceCommands,
		1, &submit->semaphore
	};
	DS_VK_CALL(device->vkQueueSubmit)(device->queue, 1, &submitInfo, submit->fence);
	VkSemaphore submittedSemaphore = submit->semaphore;

	// Update the main command buffer.
	dsVkCommandBuffer_submittedResources(renderer->mainCommandBuffer, vkRenderer->submitCount);
	dsVkCommandBuffer_submittedRenderSurfaces(renderer->mainCommandBuffer, vkRenderer->submitCount);
	if (readback)
	{
		dsVkCommandBuffer_submittedReadbackOffscreens(renderer->mainCommandBuffer,
			vkRenderer->submitCount);
	}
	submit = vkRenderer->submits + vkRenderer->curSubmit;
	mainCommandBuffer->vkCommandBuffer = submit->renderCommands;
	dsVkCommandBuffer_prepare(renderer->mainCommandBuffer, true);
	DS_VK_CALL(device->vkResetCommandBuffer)(submit->resourceCommands, 0);

	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};
	DS_VK_CALL(device->vkBeginCommandBuffer)(submit->resourceCommands, &beginInfo);
	DS_VK_CALL(device->vkBeginCommandBuffer)(submit->renderCommands, &beginInfo);
	return submittedSemaphore;
}

dsGfxFenceResult dsVkRenderer_waitForSubmit(dsRenderer* renderer, uint64_t submitCount,
	uint64_t timeout)
{
	VkFence fences[DS_MAX_SUBMITS];
	uint32_t fenceCount = 0;

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));

	if (vkRenderer->finishedSubmitCount >= submitCount)
	{
		// Already synchronized to this submit.
		DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));
		return dsGfxFenceResult_Success;
	}
	else if (vkRenderer->submitCount <= submitCount)
	{
		// Haven't submitted this yet to Vulkan.
		DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));
		return dsGfxFenceResult_WaitingToQueue;
	}

	++vkRenderer->waitCount;
	for (uint32_t i = 0; i < DS_MAX_SUBMITS; ++i)
	{
		dsVkSubmitInfo* submit = vkRenderer->submits + i;
		if (submit->submitIndex < submitCount)
			fences[fenceCount++] = submit->fence;
	}
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));

	dsVkDevice* device = &vkRenderer->device;
	VkResult result = DS_VK_CALL(device->vkWaitForFences)(device->device, fenceCount, fences, true,
		timeout);

	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
	if (--vkRenderer->waitCount == 0)
		DS_VERIFY(dsConditionVariable_notifyAll(vkRenderer->waitCondition));
	if (result == VK_SUCCESS && submitCount > vkRenderer->finishedSubmitCount)
		vkRenderer->finishedSubmitCount = submitCount;
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));

	switch (result)
	{
		case VK_SUCCESS:
			return dsGfxFenceResult_Success;
		case VK_TIMEOUT:
			return dsGfxFenceResult_Timeout;
		default:
			return dsGfxFenceResult_Error;
	}
}

void dsVkRenderer_processGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer)
{
	DS_ASSERT(buffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;

	DS_VERIFY(dsSpinlock_lock(&buffer->resource.lock));

	// Once it's processed, it's now considered used.
	buffer->used = true;

	// Make sure this needs to be processed.
	if (!buffer->deviceBuffer || !buffer->hostBuffer ||
		(!buffer->needsInitialCopy && buffer->dirtyRangeCount == 0))
	{
		DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));
		return;
	}

	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));
	dsVkResourceList* resourceList = vkRenderer->pendingResources + vkRenderer->curPendingResources;

	// Keep track of the submit queue. If it's already on a queue, don't do anything.
	void* submitQueue;
	submitQueue = buffer->submitQueue;
	if (!submitQueue)
		buffer->submitQueue = resourceList;
	DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));

	if (submitQueue)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
		return;
	}

	dsVkResourceList_addBuffer(resourceList, buffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
}

void dsVkRenderer_processTexture(dsRenderer* renderer, dsTexture* texture)
{
	DS_ASSERT(texture);
	dsVkTexture* vkTexture = (dsVkTexture*)texture;
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkTexture->resource.lock));

	// Make sure this needs to be processed.
	if (!vkTexture->needsInitialCopy)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkTexture->resource.lock));
		return;
	}

	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));
	dsVkResourceList* resourceList = vkRenderer->pendingResources + vkRenderer->curPendingResources;

	// Keep track of the submit queue. If it's already on a queue, don't do anything.
	void* submitQueue;
	submitQueue = vkTexture->submitQueue;
	if (!submitQueue)
		vkTexture->submitQueue = resourceList;
	DS_VERIFY(dsSpinlock_unlock(&vkTexture->resource.lock));

	if (submitQueue)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
		return;
	}

	dsVkResourceList_addTexture(resourceList, texture);
}

void dsVkRenderer_deleteGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer)
{
	DS_ASSERT(buffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addBuffer(resourceList, buffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteTexture(dsRenderer* renderer, dsTexture* texture)
{
	DS_ASSERT(texture);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addTexture(resourceList, texture);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteCopyImage(dsRenderer* renderer, dsVkCopyImage* copyImage)
{
	DS_ASSERT(copyImage);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addCopyImage(resourceList, copyImage);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteRenderbuffer(dsRenderer* renderer, dsRenderbuffer* renderbuffer)
{
	DS_ASSERT(renderbuffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addRenderbuffer(resourceList, renderbuffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteFramebuffer(dsRenderer* renderer, dsVkRealFramebuffer* framebuffer)
{
	DS_ASSERT(framebuffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addFramebuffer(resourceList, framebuffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteFence(dsRenderer* renderer, dsGfxFence* fence)
{
	DS_ASSERT(fence);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addFence(resourceList, fence);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteQueriePool(dsRenderer* renderer, dsGfxQueryPool* queries)
{
	DS_ASSERT(queries);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addQueries(resourceList, queries);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteMaterialDescriptor(dsRenderer* renderer, dsVkMaterialDescriptor* descriptor)
{
	DS_ASSERT(descriptor);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addMaterialDescriptor(resourceList, descriptor);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteSamplerList(dsRenderer* renderer, dsVkSamplerList* samplers)
{
	DS_ASSERT(samplers);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addSamplerList(resourceList, samplers);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteComputePipeline(dsRenderer* renderer, dsVkComputePipeline* pipeline)
{
	DS_ASSERT(pipeline);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addComputePipeline(resourceList, pipeline);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deletePipeline(dsRenderer* renderer, dsVkPipeline* pipeline)
{
	DS_ASSERT(pipeline);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addPipeline(resourceList, pipeline);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteRenderSurface(dsRenderer* renderer, dsVkRenderSurfaceData* surface)
{
	DS_ASSERT(surface);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addRenderSurface(resourceList, surface);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}

void dsVkRenderer_deleteCommandPool(dsRenderer* renderer, dsVkCommandPoolData* pool)
{
	DS_ASSERT(pool);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

	dsVkResourceList* resourceList = vkRenderer->deleteResources + vkRenderer->curDeleteResources;
	dsVkResourceList_addCommandPool(resourceList, pool);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
}
