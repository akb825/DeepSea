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

#include <DeepSea/RenderVulkan/VkRenderer.h>
#include "VkRendererInternal.h"

#include "Platform/VkPlatform.h"
#include "Resources/VkComputePipeline.h"
#include "Resources/VkDrawGeometry.h"
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
#include "Resources/VkShader.h"
#include "Resources/VkTempBuffer.h"
#include "Resources/VkTexture.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkCommandBufferPool.h"
#include "VkCommandPoolData.h"
#include "VkInit.h"
#include "VkProcessResourceList.h"
#include "VkRenderPass.h"
#include "VkRenderPassData.h"
#include "VkRenderSurface.h"
#include "VkResourceList.h"
#include "VkShared.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <limits.h>
#include <string.h>

static size_t fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsVkRenderer)) + dsMutex_fullAllocSize() +
		dsConditionVariable_fullAllocSize();
}

static bool useBGRASurface(const char* deviceName)
{
	DS_UNUSED(deviceName);

	// Devices that use RGBA surfaces.
	if (DS_ANDROID)
		return false;

	// Most devices use BGRA surfaces.
	return true;
}

static bool createCommandBuffers(dsVkRenderer* renderer)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

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
		if (!dsVkCommandBuffer_initialize(&submit->commandBuffer, baseRenderer,
			baseRenderer->allocator, dsCommandBufferUsage_Standard, 0))
		{
			return false;
		}

		VkResult result = DS_VK_CALL(device->vkCreateFence)(device->device, &fenceCreateInfo,
			instance->allocCallbacksPtr, &submit->fence);
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't create fence"))
			return false;

		result = DS_VK_CALL(device->vkCreateSemaphore)(device->device, &semaphoreCreateInfo,
			instance->allocCallbacksPtr, &submit->semaphore);
		if (!DS_HANDLE_VK_RESULT(result, "Couldn't create semaphore"))
			return false;
	}

	// Start at submit count 1 so it's ahead of the finished index.
	renderer->submitCount = 1;

	// Set up the main command buffer.
	dsVkSubmitInfo* firstSubmit = renderer->submits + renderer->curSubmit;
	dsVkCommandBufferWrapper* mainCommandBuffer = &renderer->mainCommandBuffer;
	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)mainCommandBuffer;
	baseCommandBuffer->renderer = baseRenderer;
	baseCommandBuffer->allocator = baseRenderer->allocator;
	baseCommandBuffer->usage = dsCommandBufferUsage_Standard;
	mainCommandBuffer->realCommandBuffer = (dsCommandBuffer*)&firstSubmit->commandBuffer;
	baseRenderer->mainCommandBuffer = baseCommandBuffer;

	firstSubmit->resourceCommands = dsVkCommandBuffer_getCommandBuffer(
		mainCommandBuffer->realCommandBuffer);
	dsVkCommandBuffer_forceNewCommandBuffer(mainCommandBuffer->realCommandBuffer);

	return true;
}

static VkSampler createDefaultSampler(dsVkDevice* device)
{
	dsVkInstance* instance = &device->instance;
	VkSamplerCreateInfo samplerCreateInfo =
	{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		NULL,
		0,
		VK_FILTER_NEAREST,
		VK_FILTER_NEAREST,
		VK_SAMPLER_MIPMAP_MODE_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0.0f,
		false,
		0.0,
		false,
		VK_COMPARE_OP_NEVER,
		0.0f,
		1000.0f,
		VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		false
	};

	VkSampler sampler;
	VkResult result = DS_VK_CALL(device->vkCreateSampler)(device->device, &samplerCreateInfo,
		instance->allocCallbacksPtr, &sampler);
	if (!DS_HANDLE_VK_RESULT(result, "Couldn't create sampler"))
		return 0;

	return sampler;
}

static void freeAllResources(dsVkResourceList* deleteList, bool ignoreCommandBufferRefs)
{
	uint32_t finalCount = 0;
	// Free command pools first since they may free other types of resources.
	for (uint32_t i = 0; i < deleteList->commandPoolCount; ++i)
	{
		dsVkCommandPoolData* commandPool = deleteList->commandPools[i];
		if (ignoreCommandBufferRefs || commandPool->resource.commandBufferCount == 0)
			dsVkCommandPoolData_destroy(commandPool);
		else
			deleteList->commandPools[finalCount++] = commandPool;
	}
	deleteList->commandPoolCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->bufferCount; ++i)
	{
		if (ignoreCommandBufferRefs || deleteList->buffers[i]->resource.commandBufferCount == 0)
			dsVkGfxBufferData_destroy(deleteList->buffers[i]);
		else
			deleteList->buffers[finalCount++] = deleteList->buffers[i];
	}
	deleteList->bufferCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->textureCount; ++i)
	{
		dsTexture* texture = deleteList->textures[i];
		dsVkTexture* vkTexture = (dsVkTexture*)texture;
		if (ignoreCommandBufferRefs || vkTexture->resource.commandBufferCount == 0)
			dsVkTexture_destroyImpl(texture);
		else
			deleteList->textures[finalCount++] = texture;
	}
	deleteList->textureCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->tempBufferCount; ++i)
	{
		if (ignoreCommandBufferRefs || deleteList->tempBuffers[i]->resource.commandBufferCount == 0)
			dsVkTempBuffer_destroy(deleteList->tempBuffers[i]);
		else
			deleteList->tempBuffers[finalCount++] = deleteList->tempBuffers[i];
	}
	deleteList->tempBufferCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->renderbufferCount; ++i)
	{
		dsRenderbuffer* renderbuffer = deleteList->renderbuffers[i];
		dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;
		if (ignoreCommandBufferRefs || vkRenderbuffer->resource.commandBufferCount == 0)
			dsVkRenderbuffer_destroyImpl(renderbuffer);
		else
			deleteList->renderbuffers[finalCount++] = renderbuffer;
	}
	deleteList->renderbufferCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->framebufferCount; ++i)
	{
		dsVkRealFramebuffer* framebuffer =deleteList->framebuffers[i];
		if (ignoreCommandBufferRefs || framebuffer->resource.commandBufferCount == 0)
			dsVkRealFramebuffer_destroy(framebuffer);
		else
			deleteList->framebuffers[finalCount++] = framebuffer;
	}
	deleteList->framebufferCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->fenceCount; ++i)
	{
		dsGfxFence* fence = deleteList->fences[i];
		dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;
		if (ignoreCommandBufferRefs || vkFence->resource.commandBufferCount == 0)
			dsVkGfxFence_destroyImpl(fence);
		else
			deleteList->fences[finalCount++] = fence;
	}
	deleteList->fenceCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->queryCount; ++i)
	{
		dsGfxQueryPool* queries = deleteList->queries[i];
		dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
		if (ignoreCommandBufferRefs || vkQueries->resource.commandBufferCount == 0)
			dsVkGfxQueryPool_destroyImpl(queries);
		else
			deleteList->queries[finalCount++] = queries;
	}
	deleteList->queryCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->descriptorCount; ++i)
	{
		if (ignoreCommandBufferRefs || deleteList->descriptors[i]->resource.commandBufferCount == 0)
			dsVkMaterialDescriptor_destroy(deleteList->descriptors[i]);
		else
			deleteList->descriptors[finalCount++] = deleteList->descriptors[i];
	}
	deleteList->descriptorCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->samplerCount; ++i)
	{
		if (ignoreCommandBufferRefs || deleteList->samplers[i]->resource.commandBufferCount == 0)
			dsVkSamplerList_destroy(deleteList->samplers[i]);
		else
			deleteList->samplers[finalCount++] = deleteList->samplers[i];
	}
	deleteList->samplerCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->computePipelineCount; ++i)
	{
		dsVkComputePipeline* computePipeline = deleteList->computePipelines[i];
		if (ignoreCommandBufferRefs || computePipeline->resource.commandBufferCount == 0)
			dsVkComputePipeline_destroy(computePipeline);
		else
			deleteList->computePipelines[finalCount++] = computePipeline;
	}
	deleteList->computePipelineCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->pipelineCount; ++i)
	{
		if (ignoreCommandBufferRefs || deleteList->pipelines[i]->resource.commandBufferCount == 0)
			dsVkPipeline_destroy(deleteList->pipelines[i]);
		else
			deleteList->pipelines[finalCount++] = deleteList->pipelines[i];
	}
	deleteList->pipelineCount = finalCount;

	finalCount = 0;
	for (uint32_t i = 0; i < deleteList->renderPassCount; ++i)
	{
		dsVkRenderPassData* renderPass = deleteList->renderPasses[i];
		if (ignoreCommandBufferRefs || renderPass->resource.commandBufferCount == 0)
			dsVkRenderPassData_destroy(renderPass);
		else
			deleteList->renderPasses[finalCount++] = renderPass;
	}
	deleteList->renderPassCount = finalCount;
}

static void freeResources(dsVkRenderer* renderer, uint64_t finishedSubmitCount)
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

		bool stillInUse = dsVkResource_isInUse(&buffer->resource, finishedSubmitCount) ||
			(buffer->uploadedSubmit != DS_NOT_SUBMITTED &&
				buffer->uploadedSubmit > finishedSubmitCount);
		dsVkRenderer_deleteGfxBuffer(baseRenderer, buffer, !stillInUse);
	}

	for (uint32_t i = 0; i < prevDeleteList->textureCount; ++i)
	{
		dsTexture* texture = prevDeleteList->textures[i];
		DS_ASSERT(texture);
		dsVkTexture* vkTexture = (dsVkTexture*)texture;

		bool stillInUse = dsVkResource_isInUse(&vkTexture->resource, finishedSubmitCount) ||
			(vkTexture->uploadedSubmit != DS_NOT_SUBMITTED &&
				vkTexture->uploadedSubmit > finishedSubmitCount) ||
			(vkTexture->lastDrawSubmit != DS_NOT_SUBMITTED &&
				vkTexture->lastDrawSubmit > finishedSubmitCount);
		dsVkRenderer_deleteTexture(baseRenderer, texture, !stillInUse);
	}

	for (uint32_t i = 0; i < prevDeleteList->tempBufferCount; ++i)
	{
		dsVkTempBuffer* buffer = prevDeleteList->tempBuffers[i];
		DS_ASSERT(buffer);
		dsVkRenderer_deleteTempBuffer(baseRenderer, buffer,
			!dsVkResource_isInUse(&buffer->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->renderbufferCount; ++i)
	{
		dsRenderbuffer* renderbuffer = prevDeleteList->renderbuffers[i];
		DS_ASSERT(renderbuffer);
		dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;
		dsVkRenderer_deleteRenderbuffer(baseRenderer, renderbuffer,
			!dsVkResource_isInUse(&vkRenderbuffer->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->framebufferCount; ++i)
	{
		dsVkRealFramebuffer* framebuffer = prevDeleteList->framebuffers[i];
		DS_ASSERT(framebuffer);
		dsVkRenderer_deleteFramebuffer(baseRenderer, framebuffer,
			!dsVkResource_isInUse(&framebuffer->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->fenceCount; ++i)
	{
		dsGfxFence* fence = prevDeleteList->fences[i];
		DS_ASSERT(fence);
		dsVkGfxFence* vkFence = (dsVkGfxFence*)fence;
		dsVkRenderer_deleteFence(baseRenderer, fence,
			!dsVkResource_isInUse(&vkFence->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->queryCount; ++i)
	{
		dsGfxQueryPool* queries = prevDeleteList->queries[i];
		DS_ASSERT(queries);
		dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
		dsVkRenderer_deleteQueriePool(baseRenderer, queries,
			!dsVkResource_isInUse(&vkQueries->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->descriptorCount; ++i)
	{
		dsVkMaterialDescriptor* descriptor = prevDeleteList->descriptors[i];
		DS_ASSERT(descriptor);
		dsVkRenderer_deleteMaterialDescriptor(baseRenderer, descriptor,
			!dsVkResource_isInUse(&descriptor->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->samplerCount; ++i)
	{
		dsVkSamplerList* samplers = prevDeleteList->samplers[i];
		DS_ASSERT(samplers);
		dsVkRenderer_deleteSamplerList(baseRenderer, samplers,
			!dsVkResource_isInUse(&samplers->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->computePipelineCount; ++i)
	{
		dsVkComputePipeline* pipeline = prevDeleteList->computePipelines[i];
		DS_ASSERT(pipeline);
		dsVkRenderer_deleteComputePipeline(baseRenderer, pipeline,
			!dsVkResource_isInUse(&pipeline->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->pipelineCount; ++i)
	{
		dsVkPipeline* pipeline = prevDeleteList->pipelines[i];
		DS_ASSERT(pipeline);
		dsVkRenderer_deletePipeline(baseRenderer, pipeline,
			!dsVkResource_isInUse(&pipeline->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->commandPoolCount; ++i)
	{
		dsVkCommandPoolData* pool = prevDeleteList->commandPools[i];
		DS_ASSERT(pool);
		dsVkRenderer_deleteCommandPool(baseRenderer, pool,
			!dsVkResource_isInUse(&pool->resource, finishedSubmitCount));
	}

	for (uint32_t i = 0; i < prevDeleteList->renderPassCount; ++i)
	{
		dsVkRenderPassData* renderPass = prevDeleteList->renderPasses[i];
		DS_ASSERT(renderPass);
		dsVkRenderer_deleteRenderPass(baseRenderer, renderPass,
			!dsVkResource_isInUse(&renderPass->resource, finishedSubmitCount));
	}

	dsVkResourceList_clear(prevDeleteList);
}

static bool addBufferCopies(dsVkRenderer* renderer, dsVkGfxBufferData* buffer,
	const dsVkDirtyRange* dirtyRanges, uint32_t dirtyRangeCount, bool initial)
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

	for (uint32_t i = 0; i < dirtyRangeCount; ++i)
	{
		VkBufferCopy* copyInfo = renderer->bufferCopies + firstCopy + i;
		const dsVkDirtyRange* dirtyRange = dirtyRanges + i;
		copyInfo->srcOffset = copyInfo->dstOffset = dirtyRange->start;
		copyInfo->size = dirtyRange->size;

		// Need a barrier before.
		dsVkBarrierList_addBufferBarrier(preResourceBarriers, buffer->hostBuffer,
			dirtyRange->start, dirtyRange->size, 0, dsGfxBufferUsage_CopyFrom, true);
		if (!initial)
		{
			// Only need a barrier before the copy for the device buffer if it's not the initial
			// copy.
			dsVkBarrierList_addBufferBarrier(preResourceBarriers, buffer->deviceBuffer,
				dirtyRange->start, dirtyRange->size, buffer->usage | dsGfxBufferUsage_CopyTo,
				dsGfxBufferUsage_CopyTo, false);
		}
		// Also need a barrier after.
		dsVkBarrierList_addBufferBarrier(postResourceBarriers, buffer->deviceBuffer,
			dirtyRange->start, dirtyRange->size, dsGfxBufferUsage_CopyTo,
			buffer->usage | dsGfxBufferUsage_CopyTo, false);
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

static void prepareOffscreen(dsVkRenderer* renderer, dsVkTexture* texture)
{
	dsTexture* baseTexture = (dsTexture*)texture;
	bool isDepthStencil = dsGfxFormat_isDepthStencil(baseTexture->info.format);
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	VkImageSubresourceRange fullLayout = {texture->aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0,
		VK_REMAINING_ARRAY_LAYERS};
	dsVkBarrierList_addImageBarrier(postResourceBarriers, texture->deviceImage, &fullLayout,
		0, true, isDepthStencil, baseTexture->usage, VK_IMAGE_LAYOUT_UNDEFINED,
		dsVkTexture_imageLayout(baseTexture));
	if (texture->surfaceImage)
	{
		dsVkBarrierList_addImageBarrier(postResourceBarriers, texture->surfaceImage, &fullLayout,
			0, true, isDepthStencil, baseTexture->usage, VK_IMAGE_LAYOUT_UNDEFINED,
			isDepthStencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
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

	dsVkBarrierList_addImageBarrier(preResourceBarriers, texture->deviceImage, &fullLayout, 0,
		false, false, dsTextureUsage_CopyTo, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	dsVkBarrierList_addBufferBarrier(preResourceBarriers, texture->hostBuffer, 0,
		texture->hostMemorySize, 0, dsGfxBufferUsage_CopyFrom, true);

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
	copyInfo->srcBuffer= texture->hostBuffer;
	copyInfo->dstImage = texture->deviceImage;
	copyInfo->dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyInfo->firstRange = index;
	copyInfo->rangeCount = info->mipLevels;

	size_t offset = 0;
	dsTextureInfo surfaceInfo = baseTexture->info;
	surfaceInfo.mipLevels = 1;
	for (uint32_t i = 0; i < info->mipLevels; ++i)
	{
		uint32_t width = info->width >> i;
		uint32_t height = info->height >> i;
		uint32_t depth = is3D ? info->depth >> i : info->depth;
		width = dsMax(1U, width);
		height = dsMax(1U, height);
		depth = dsMax(1U, depth);

		uint32_t layerCount = faceCount*(is3D ? 1U : depth);
		VkBufferImageCopy* imageCopy = renderer->imageCopies + index + i;
		imageCopy->bufferOffset = offset;
		imageCopy->bufferRowLength = 0;
		imageCopy->bufferImageHeight = 0;
		imageCopy->imageSubresource.aspectMask = texture->aspectMask;
		imageCopy->imageSubresource.mipLevel = i;
		imageCopy->imageSubresource.baseArrayLayer = 0;
		imageCopy->imageSubresource.layerCount = layerCount;
		imageCopy->imageOffset.x = 0;
		imageCopy->imageOffset.y = 0;
		imageCopy->imageOffset.z = 0;
		imageCopy->imageExtent.width = width;
		imageCopy->imageExtent.height = height;
		imageCopy->imageExtent.depth = is3D ? depth : 1U;

		surfaceInfo.width = width;
		surfaceInfo.height = height;
		if (is3D)
			surfaceInfo.depth = depth;
		offset += dsTexture_size(&surfaceInfo);
	}
	DS_ASSERT(offset <= texture->hostMemorySize);

	// Even non-static images will have a barrier to process the layout conversion.
	dsVkBarrierList_addImageBarrier(postResourceBarriers, texture->deviceImage, &fullLayout,
		dsTextureUsage_CopyFrom, false, false, baseTexture->usage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dsVkTexture_imageLayout(baseTexture));

	return true;
}

static void prepareTexture(dsVkRenderer* renderer, dsVkTexture* texture)
{
	dsTexture* baseTexture = (dsTexture*)texture;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	VkImageSubresourceRange fullLayout = {texture->aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0,
		VK_REMAINING_ARRAY_LAYERS};

	dsVkBarrierList_addImageBarrier(postResourceBarriers, texture->deviceImage, &fullLayout,
		0, false, false, baseTexture->usage, VK_IMAGE_LAYOUT_UNDEFINED,
		dsVkTexture_imageLayout(baseTexture));
}

static void processBuffers(dsVkRenderer* renderer, dsVkProcessResourceList* resourceList,
	uint64_t finishedSubmitCount)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < resourceList->bufferCount; ++i)
	{
		dsLifetime* lifetime = resourceList->buffers[i];
		dsVkGfxBufferData* buffer = (dsVkGfxBufferData*)dsLifetime_acquire(lifetime);
		if (!buffer)
			continue;

		if (!buffer->hostBuffer)
		{
			dsLifetime_release(lifetime);
			continue;
		}

		DS_VERIFY(dsSpinlock_lock(&buffer->resource.lock));
		// Clear the submit queue now that we're processing it.
		buffer->submitQueue = NULL;
		if (buffer->mappedSize > 0)
		{
			// Still mapped, process later.
			DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));
			dsVkRenderer_processGfxBuffer(baseRenderer, buffer);
			dsLifetime_release(lifetime);
			continue;
		}

		// Record the ranges to copy.
		bool doUpload = false;
		if (buffer->needsInitialCopy)
		{
			DS_ASSERT(buffer->dirtyRangeCount == 0);
			if (buffer->deviceBuffer)
			{
				doUpload = true;
				dsVkDirtyRange dirtyRange = {0, buffer->size};
				addBufferCopies(renderer, buffer, &dirtyRange, 1, true);
			}
			else
			{
				// Just need to add a barrier if no device buffer.
				dsVkBarrierList_addBufferBarrier(&renderer->postResourceBarriers, buffer->hostBuffer,
					0, buffer->size, 0, buffer->usage, true);
			}
			buffer->needsInitialCopy = false;
		}
		else if (buffer->dirtyRangeCount > 0)
		{
			doUpload = true;
			addBufferCopies(renderer, buffer, buffer->dirtyRanges, buffer->dirtyRangeCount, false);
			buffer->dirtyRangeCount = 0;
		}

		// Record when the latest copy occurred. If no copy to process, then see if we can destroy
		// the host memory. (i.e. it was only used for the initial data)
		VkDeviceMemory hostMemory = 0;
		VkBuffer hostBuffer = 0;
		if (doUpload)
			buffer->uploadedSubmit = renderer->submitCount;
		else if (buffer->hostBuffer && !buffer->keepHost &&
			buffer->uploadedSubmit <= finishedSubmitCount)
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

		dsLifetime_release(lifetime);
	}
}

static void processTextures(dsVkRenderer* renderer, dsVkProcessResourceList* resourceList,
	uint64_t finishedSubmitCount)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkDevice* device = &renderer->device;
	dsVkInstance* instance = &device->instance;

	for (uint32_t i = 0; i < resourceList->textureCount; ++i)
	{
		dsLifetime* lifetime = resourceList->textures[i];
		dsTexture* texture = (dsTexture*)dsLifetime_acquire(lifetime);
		if (!texture)
			continue;

		dsVkTexture* vkTexture = (dsVkTexture*)texture;

		DS_VERIFY(dsSpinlock_lock(&vkTexture->resource.lock));
		// Clear the submit queue now that we're processing it.
		vkTexture->submitQueue = NULL;
		bool doUpload = false;
		if (vkTexture->needsInitialCopy)
		{
			doUpload = true;
			if (texture->offscreen)
				prepareOffscreen(renderer, vkTexture);
			else if (vkTexture->hostBuffer)
				addImageCopies(renderer, vkTexture);
			else
				prepareTexture(renderer, vkTexture);
			vkTexture->needsInitialCopy = false;
		}

		DS_VERIFY(dsSpinlock_unlock(&vkTexture->resource.lock));

		// Queue for re-processing if we still need to delete the host image.
		if (doUpload || vkTexture->uploadedSubmit > finishedSubmitCount)
			dsVkRenderer_processTexture(baseRenderer, texture);
		else if (!texture->offscreen && vkTexture->hostBuffer)
		{
			// Non-offscreens don't need host images to remain.
			DS_VK_CALL(device->vkDestroyBuffer)(device->device, vkTexture->hostBuffer,
				instance->allocCallbacksPtr);
			DS_VK_CALL(device->vkFreeMemory)(device->device, vkTexture->hostMemory,
				instance->allocCallbacksPtr);
			vkTexture->hostBuffer = 0;
			vkTexture->hostMemory = 0;
		}

		dsLifetime_release(lifetime);
	}
}

static void processRenderbuffers(dsVkRenderer* renderer, dsVkProcessResourceList* resourceList)
{
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	for (uint32_t i = 0; i < resourceList->renderbufferCount; ++i)
	{
		// Renderbuffers are always queued once, so no need to check if needs processing.
		dsRenderbuffer* renderbuffer = resourceList->renderbuffers[i];
		dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;

		VkImageAspectFlags aspectMask = dsVkImageAspectFlags(renderbuffer->format);
		VkImageSubresourceRange fullLayout = {aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0,
			VK_REMAINING_ARRAY_LAYERS};
		bool isDepthStencil = dsGfxFormat_isDepthStencil(renderbuffer->format);
		dsTextureUsage usage = 0;
		if (renderbuffer->usage & dsRenderbufferUsage_BlitFrom)
			usage |= dsTextureUsage_CopyFrom;
		if (renderbuffer->usage & dsRenderbufferUsage_BlitTo)
			usage |= dsTextureUsage_CopyTo;

		dsVkBarrierList_addImageBarrier(postResourceBarriers, vkRenderbuffer->image, &fullLayout,
			0, true, isDepthStencil, usage, VK_IMAGE_LAYOUT_UNDEFINED,
			isDepthStencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
}

static void processRenderSurfaces(dsVkRenderer* renderer, dsVkProcessResourceList* resourceList)
{
	dsRenderer* baseRenderer = (dsRenderer*)renderer;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	VkImageSubresourceRange fullColorLayout = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
		VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

	VkImageAspectFlags depthAspectMask =
		dsVkImageAspectFlags(baseRenderer->surfaceDepthStencilFormat);
	VkImageSubresourceRange fullDepthLayout = {depthAspectMask, 0, VK_REMAINING_MIP_LEVELS, 0,
		VK_REMAINING_ARRAY_LAYERS};

	for (uint32_t i = 0; i < resourceList->renderSurfaceCount; ++i)
	{
		// Render surfaces are always queued once, so no need to check if needs processing.
		dsVkRenderSurfaceData* surface = resourceList->renderSurfaces[i];

		dsTextureUsage usage = dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom;
		if (surface->resolveImage)
		{
			dsVkBarrierList_addImageBarrier(postResourceBarriers, surface->resolveImage,
				&fullColorLayout, 0, true, false, usage, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		if (surface->depthImage)
		{
			dsVkBarrierList_addImageBarrier(postResourceBarriers, surface->depthImage,
				&fullDepthLayout, 0, true, true, usage, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
	}
}

static void processResources(dsVkRenderer* renderer, VkCommandBuffer commandBuffer)
{
	DS_PROFILE_FUNC_START();
	dsVkDevice* device = &renderer->device;
	dsVkBarrierList* preResourceBarriers = &renderer->preResourceBarriers;
	dsVkBarrierList* postResourceBarriers = &renderer->postResourceBarriers;

	DS_VERIFY(dsSpinlock_lock(&renderer->resourceLock));
	dsVkProcessResourceList* prevResourceList = renderer->pendingResources +
		renderer->curPendingResources;
	renderer->curPendingResources =
		(renderer->curPendingResources + 1) % DS_PENDING_RESOURCES_ARRAY;
	DS_VERIFY(dsSpinlock_unlock(&renderer->resourceLock));

	uint64_t finishedSubmitCount = dsVkRenderer_getFinishedSubmitCount((dsRenderer*)renderer);

	// Clear everything out.
	renderer->bufferCopiesCount = 0;
	renderer->bufferCopyInfoCount = 0;
	renderer->imageCopyCount = 0;
	renderer->imageCopyInfoCount = 0;

	dsVkBarrierList_clear(preResourceBarriers);
	dsVkBarrierList_clear(postResourceBarriers);

	processBuffers(renderer, prevResourceList, finishedSubmitCount);
	processTextures(renderer, prevResourceList, finishedSubmitCount);
	processRenderbuffers(renderer, prevResourceList);
	processRenderSurfaces(renderer, prevResourceList);

	// Process the uploads.
	if (preResourceBarriers->bufferBarrierCount > 0 || preResourceBarriers->imageBarrierCount > 0)
	{
		DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, preResourceBarriers->bufferBarrierCount,
			preResourceBarriers->bufferBarriers, preResourceBarriers->imageBarrierCount,
			preResourceBarriers->imageBarriers);
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
		DS_VK_CALL(device->vkCmdCopyBufferToImage)(commandBuffer, copyInfo->srcBuffer,
			copyInfo->dstImage, copyInfo->dstLayout, copyInfo->rangeCount,
			renderer->imageCopies + copyInfo->firstRange);
	}

	// Ensure that all host access is synchronized.
	VkMemoryBarrier memoryBarrier =
	{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
		VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
			VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
			VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &memoryBarrier,
		postResourceBarriers->bufferBarrierCount, postResourceBarriers->bufferBarriers,
		postResourceBarriers->imageBarrierCount, postResourceBarriers->imageBarriers);

	dsVkProcessResourceList_clear(prevResourceList);
	DS_PROFILE_FUNC_RETURN_VOID();
}

static bool beginDraw(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)dsVkCommandBuffer_get(commandBuffer);

	// NOTE: If there are collisions with vertex hashes, then the full vertex formats need to be
	// stored in the CommandBuffer since there's no guarantee that the dsDrawGeometry object is
	// still active.
	if (!vkCommandBuffer->activePipeline ||
		vkCommandBuffer->activeShader != commandBuffer->boundShader ||
		(vkCommandBuffer->activeVertexGeometry != geometry &&
			!dsVkDrawGeometry_equivalentVertexFormats(geometry,
				vkCommandBuffer->activeVertexFormats)))
	{
		VkPipeline pipeline = dsVkShader_getPipeline((dsShader*)commandBuffer->boundShader,
			commandBuffer, primitiveType, geometry);
		if (!pipeline)
			return false;

		dsVkCommandBuffer_bindPipeline(commandBuffer, submitBuffer, pipeline);
		vkCommandBuffer->activeShader = commandBuffer->boundShader;
		vkCommandBuffer->activePrimitiveType = primitiveType;
		for (unsigned int i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
			vkCommandBuffer->activeVertexFormats[i] = geometry->vertexBuffers[i].format;
	}

	if (vkCommandBuffer->activeVertexGeometry == geometry)
		return true;

	VkBuffer buffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	VkDeviceSize offsets[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	uint32_t bindingCount = 0;
	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		const dsVertexBuffer* vertexBuffer = geometry->vertexBuffers + i;
		dsGfxBuffer* buffer = vertexBuffer->buffer;
		if (!buffer)
			continue;

		dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);
		if (!bufferData)
			return false;

		dsVkRenderer_processGfxBuffer(commandBuffer->renderer, bufferData);
		VkBuffer vkBuffer = dsVkGfxBufferData_getBuffer(bufferData);
		buffers[bindingCount] = vkBuffer;
		offsets[bindingCount] = vertexBuffer->offset;
		++bindingCount;
	}

	vkCommandBuffer->activeVertexGeometry = geometry;
	DS_VK_CALL(device->vkCmdBindVertexBuffers)(submitBuffer, 0, bindingCount, buffers, offsets);
	return true;
}

static bool beginIndexedDraw(dsCommandBuffer* commandBuffer, VkCommandBuffer submitBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)commandBuffer->renderer)->device;
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)dsVkCommandBuffer_get(commandBuffer);
	if (!beginDraw(commandBuffer, submitBuffer, geometry, NULL, primitiveType))
		return false;

	const dsIndexBuffer* indexBuffer = &geometry->indexBuffer;
	if (vkCommandBuffer->activeIndexBuffer == indexBuffer)
		return true;;

	dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(indexBuffer->buffer, commandBuffer);
	if (!bufferData)
		return false;

	dsVkRenderer_processGfxBuffer(commandBuffer->renderer, bufferData);
	vkCommandBuffer->activeIndexBuffer = indexBuffer;
	DS_VK_CALL(device->vkCmdBindIndexBuffer)(submitBuffer,
		dsVkGfxBufferData_getBuffer(bufferData), indexBuffer->offset,
		indexBuffer->indexSize == sizeof(uint16_t) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	return true;
}

static bool beginDispatch(dsRenderer* renderer, VkCommandBuffer submitBuffer,
	dsCommandBuffer* commandBuffer)
{
	dsVkCommandBuffer* vkCommandBuffer = (dsVkCommandBuffer*)dsVkCommandBuffer_get(commandBuffer);
	if (!vkCommandBuffer->activeComputeShader ||
		vkCommandBuffer->activeComputeShader != commandBuffer->boundComputeShader)
	{
		VkPipeline pipeline = dsVkShader_getComputePipeline(
			(dsShader*)commandBuffer->boundComputeShader, commandBuffer);
		if (!pipeline)
			return false;

		dsVkCommandBuffer_bindComputePipeline(commandBuffer, submitBuffer, pipeline);
		vkCommandBuffer->activeComputeShader = commandBuffer->boundComputeShader;
	}

	VkPipelineStageFlagBits srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if (renderer->hasTessellationShaders)
	{
		srcStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		srcStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	return dsVkCommandBuffer_submitMemoryBarriers(commandBuffer, srcStages, dstStages);
}

static bool setBeginBlitSurfaceBarrierInfo(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	VkImageMemoryBarrier* barrier, dsGfxSurfaceType surfaceType, void* surface,
	VkImageAspectFlags* aspectMask, VkPipelineStageFlags* stages)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		{
			dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface;
			dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
			if (!dsVkCommandBuffer_addResource(commandBuffer, &surfaceData->resource))
				return false;

			barrier->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier->image = surfaceData->images[surfaceData->imageIndex];
			barrier->subresourceRange.baseArrayLayer =
				surfaceType == dsGfxSurfaceType_ColorRenderSurfaceRight;
			*aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			*stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			return true;
		}
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
		{
			dsVkRenderSurface* renderSurface = (dsVkRenderSurface*)surface;
			dsVkRenderSurfaceData* surfaceData = renderSurface->surfaceData;
			if (!dsVkCommandBuffer_addResource(commandBuffer, &surfaceData->resource))
				return false;

			barrier->srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			barrier->image = surfaceData->depthImage;
			*aspectMask = dsVkImageAspectFlags(renderer->surfaceDepthStencilFormat);
			*stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			return true;
		}
		case dsGfxSurfaceType_Offscreen:
		{
			dsOffscreen* offscreen = (dsOffscreen*)surface;
			DS_ASSERT(offscreen->offscreen);
			dsVkTexture* vkTexture = (dsVkTexture*)offscreen;
			if (!dsVkCommandBuffer_addResource(commandBuffer, &vkTexture->resource))
				return false;

			dsVkRenderer_processTexture(renderer, offscreen);
			bool isDepthStencil = dsGfxFormat_isDepthStencil(offscreen->info.format);
			barrier->srcAccessMask = dsVkWriteImageAccessFlags(offscreen->usage, true,
				isDepthStencil);
			barrier->oldLayout = dsVkTexture_imageLayout(offscreen);
			barrier->image = vkTexture->deviceImage;
			*aspectMask = dsVkImageAspectFlags(offscreen->info.format);
			*stages |= dsVkWriteImageStageFlags(renderer, offscreen->usage, offscreen->offscreen,
				isDepthStencil);
			return true;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsRenderbuffer* renderbuffer = (dsRenderbuffer*)surface;
			dsVkRenderbuffer* vkRenderbuffer = (dsVkRenderbuffer*)renderbuffer;
			if (!dsVkCommandBuffer_addResource(commandBuffer, &vkRenderbuffer->resource))
				return false;

			barrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			if (dsGfxFormat_isDepthStencil(renderbuffer->format))
			{
				barrier->srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				barrier->oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				*stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else
			{
				barrier->srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier->oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				*stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			barrier->image = vkRenderbuffer->image;
			*aspectMask = dsVkImageAspectFlags(renderbuffer->format);
			return true;
		}
		default:
			DS_ASSERT(false);
			return false;
	}
}

static void setEndBlitSurfaceBarrierInfo(const dsRenderer* renderer, VkImageMemoryBarrier* barrier,
	dsGfxSurfaceType surfaceType, void* surface, VkPipelineStageFlags* stages)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			barrier->dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			*stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			barrier->dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier->newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			*stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case dsGfxSurfaceType_Offscreen:
		{
			dsOffscreen* offscreen = (dsOffscreen*)surface;
			DS_ASSERT(offscreen->offscreen);

			bool isDepthStencil = dsGfxFormat_isDepthStencil(offscreen->info.format);
			barrier->dstAccessMask = dsVkReadImageAccessFlags(offscreen->usage) |
				dsVkWriteImageAccessFlags(offscreen->usage, true, isDepthStencil);
			barrier->newLayout = dsVkTexture_imageLayout(offscreen);
			*stages |= dsVkReadImageStageFlags(renderer, offscreen->usage,
					offscreen->offscreen && isDepthStencil) |
				dsVkWriteImageStageFlags(renderer, offscreen->usage, true, isDepthStencil);
			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsRenderbuffer* renderbuffer = (dsRenderbuffer*)surface;
			barrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
				VK_ACCESS_TRANSFER_WRITE_BIT;
			if (dsGfxFormat_isDepthStencil(renderbuffer->format))
			{
				barrier->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				barrier->newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				*stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else
			{
				barrier->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier->newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				*stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			break;
		}
		default:
			DS_ASSERT(false);
	}
}

static VkSemaphore preFlush(dsRenderer* renderer, bool readback, bool useSemaphore)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	// Get the submit queue,
	dsVkSubmitInfo* submit = vkRenderer->submits + vkRenderer->curSubmit;
	dsCommandBuffer* submitBuffer = (dsCommandBuffer*)&submit->commandBuffer;
	DS_ASSERT(vkRenderer->mainCommandBuffer.realCommandBuffer == submitBuffer);
	dsVkCommandBuffer* vkSubmitBuffer = (dsVkCommandBuffer*)submitBuffer;

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
		dsVkCommandBuffer_endSubmitCommands(submitBuffer);
	dsVkCommandBuffer_finishCommandBuffer(submitBuffer);

	VkSemaphore* waitSemaphores = NULL;
	VkPipelineStageFlags* waitStages = NULL;
	if (vkSubmitBuffer->renderSurfaceCount > 0)
	{
		waitSemaphores = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSemaphore,
			vkSubmitBuffer->renderSurfaceCount);
		waitStages = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkPipelineStageFlags,
			vkSubmitBuffer->renderSurfaceCount);
		for (uint32_t i = 0; i < vkSubmitBuffer->renderSurfaceCount; ++i)
		{
			dsVkRenderSurfaceData* surface = vkSubmitBuffer->renderSurfaces[i];
			waitSemaphores[i] = surface->imageData[surface->imageDataIndex].semaphore;
			waitStages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
	}

	VkSemaphore submittedSemaphore = useSemaphore ? submit->semaphore : 0;

	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		vkSubmitBuffer->renderSurfaceCount, waitSemaphores, waitStages,
		vkSubmitBuffer->submitBufferCount, vkSubmitBuffer->submitBuffers,
		useSemaphore ? 1 : 0, &submittedSemaphore
	};

	DS_PROFILE_SCOPE_START("vkQueueSubmit");
	DS_VK_CALL(device->vkQueueSubmit)(device->queue, 1, &submitInfo, submit->fence);
	DS_PROFILE_SCOPE_END();

	// Clean up the previous command buffer.
	DS_PROFILE_SCOPE_START("Post submit cleanup");
	dsVkCommandBuffer_submittedResources(submitBuffer, submit->submitIndex);
	dsVkCommandBuffer_submittedRenderSurfaces(submitBuffer, submit->submitIndex);
	if (readback)
		dsVkCommandBuffer_submittedReadbackOffscreens(submitBuffer, submit->submitIndex);
	DS_PROFILE_SCOPE_END();

	return submittedSemaphore;
}

static void postFlush(dsRenderer* renderer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	// Prepare the next command buffer.
	dsVkSubmitInfo* submit = vkRenderer->submits + vkRenderer->curSubmit;
	dsCommandBuffer* submitBuffer = (dsCommandBuffer*)&submit->commandBuffer;

	// Wait until we can use the command buffer.
	uint64_t finishedSubmitCount;
	if (submit->submitIndex != DS_NOT_SUBMITTED)
	{
		DS_PROFILE_WAIT_START("vkWaitForFences");
		VkResult result = DS_VK_CALL(device->vkWaitForFences)(device->device, 1, &submit->fence,
			true, DS_DEFAULT_WAIT_TIMEOUT);
		DS_PROFILE_WAIT_END();
		if (result == VK_ERROR_DEVICE_LOST)
		{
			DS_LOG_FATAL_F(DS_RENDER_VULKAN_LOG_TAG, "Vulkan device was lost.");
			abort();
		}

		DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
		// NOTE: only assigned under mutex lock so no need to do atomic load.
		finishedSubmitCount = dsMax(vkRenderer->finishedSubmitCount, submit->submitIndex);
		DS_ATOMIC_STORE64(&vkRenderer->finishedSubmitCount, &finishedSubmitCount);
		DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));
	}
	else
		finishedSubmitCount = dsVkRenderer_getFinishedSubmitCount(renderer);

	// Free resources that are waiting to be in an unused state.
	freeResources(vkRenderer, finishedSubmitCount);

	vkRenderer->mainCommandBuffer.realCommandBuffer = submitBuffer;
	dsVkCommandBuffer_prepare(submitBuffer);

	submit->resourceCommands = dsVkCommandBuffer_getCommandBuffer(submitBuffer);
	dsVkCommandBuffer_forceNewCommandBuffer(submitBuffer);
}

bool dsVkRenderer_beginFrame(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	return true;
}

bool dsVkRenderer_endFrame(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
	return true;
}

bool dsVkRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	renderer->surfaceSamples = samples;
	return true;
}

bool dsVkRenderer_setDefaultSamples(dsRenderer* renderer, uint32_t samples)
{
	renderer->defaultSamples = samples;
	return true;
}

bool dsVkRenderer_setVSync(dsRenderer* renderer, dsVSync vsync)
{
	if (renderer->vsync == vsync)
		return true;

	renderer->vsync = vsync;

	// This will require re-creating render surfaces, so make sure to flush any previous resource
	// changes in order to avoid multiple simultaneous render surface changes.
	dsRenderer_waitUntilIdle(renderer);
	return true;
}

bool dsVkRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy)
{
	renderer->defaultAnisotropy = anisotropy;
	return true;
}

bool dsVKRenderer_setViewport(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	const dsFramebuffer* framebuffer = commandBuffer->boundFramebuffer;
	DS_ASSERT(framebuffer);
	VkViewport vkViewport;
	dsConvertVkViewport(&vkViewport, viewport, framebuffer->width, framebuffer->height);

	VkRect2D renderArea =
	{
		{(int32_t)floorf(vkViewport.x), (int32_t)vkViewport.y},
		{(uint32_t)ceilf(vkViewport.width), (uint32_t)ceilf(vkViewport.height)}
	};

	DS_VK_CALL(device->vkCmdSetViewport)(submitBuffer, 0, 1, &vkViewport);
	DS_VK_CALL(device->vkCmdSetScissor)(submitBuffer, 0, 1, &renderArea);
	return true;
}

bool dsVkRenderer_clearAttachments(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	VkClearAttachment* vkAttachments = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkClearAttachment,
		attachmentCount);
	const dsRenderPass* renderPass = commandBuffer->boundRenderPass;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + commandBuffer->activeRenderSubpass;
	VkImageAspectFlags depthStencilAspect = 0;
	if (subpass->depthStencilAttachment.attachmentIndex != DS_NO_ATTACHMENT)
	{
		depthStencilAspect = dsVkImageAspectFlags(
			renderPass->attachments[subpass->depthStencilAttachment.attachmentIndex].format);
	}

	uint32_t vkAttachmentCount = 0;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		const dsClearAttachment* attachment = attachments + i;
		VkClearAttachment* vkAttachment = vkAttachments + vkAttachmentCount;
		if (attachment->colorAttachment == DS_NO_ATTACHMENT)
		{
			vkAttachment->aspectMask = 0;
			switch (attachment->clearDepthStencil)
			{
				case dsClearDepthStencil_Depth:
					vkAttachment->aspectMask = depthStencilAspect & VK_IMAGE_ASPECT_DEPTH_BIT;
					break;
				case dsClearDepthStencil_Stencil:
					vkAttachment->aspectMask = depthStencilAspect & VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				case dsClearDepthStencil_Both:
					vkAttachment->aspectMask = depthStencilAspect;
					break;
			}
			if (!vkAttachment->aspectMask)
				continue;
		}
		else
			vkAttachment->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkAttachment->colorAttachment = attachment->colorAttachment;
		// Clear value is the same memory layout.
		vkAttachment->clearValue = *(VkClearValue*)&attachment->clearValue;
		++vkAttachmentCount;
	}

	if (vkAttachmentCount == 0)
		return true;

	VkClearRect* vkRegions = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkClearRect, regionCount);
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsAttachmentClearRegion* region = regions + i;
		VkClearRect* vkRegion = vkRegions + i;
		vkRegion->rect.offset.x = region->x;
		vkRegion->rect.offset.y = region->y;
		vkRegion->rect.extent.width = region->width;
		vkRegion->rect.extent.height = region->height;
		vkRegion->baseArrayLayer = region->layer;
		vkRegion->layerCount = region->layerCount;
	}

	DS_VK_CALL(device->vkCmdClearAttachments)(submitBuffer, vkAttachmentCount, vkAttachments,
		regionCount, vkRegions);
	return true;
}

bool dsVkRenderer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginDraw(commandBuffer, submitBuffer, geometry, drawRange,
			primitiveType))
	{
		return false;
	}

	DS_VK_CALL(device->vkCmdDraw)(submitBuffer, drawRange->vertexCount, drawRange->instanceCount,
		drawRange->firstVertex, drawRange->firstInstance);
	return true;
}

bool dsVkRenderer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginIndexedDraw(commandBuffer, submitBuffer, geometry, drawRange,
			primitiveType))
	{
		return false;
	}

	DS_VK_CALL(device->vkCmdDrawIndexed)(submitBuffer, drawRange->indexCount,
		drawRange->instanceCount, drawRange->firstIndex, drawRange->vertexOffset,
		drawRange->firstInstance);
	return true;
}

bool dsVkRenderer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginDraw(commandBuffer, submitBuffer, geometry, NULL, primitiveType))
		return false;

	dsVkGfxBufferData* indirectBufferData = dsVkGfxBuffer_getData((dsGfxBuffer*)indirectBuffer,
		commandBuffer);
	if (!indirectBufferData)
		return false;

	dsVkRenderer_processGfxBuffer(renderer, indirectBufferData);
	VkBuffer vkIndirectBuffer = dsVkGfxBufferData_getBuffer(indirectBufferData);
	if (device->features.multiDrawIndirect)
	{
		DS_VK_CALL(device->vkCmdDrawIndirect)(submitBuffer, vkIndirectBuffer, offset, count,
			stride);
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			DS_VK_CALL(device->vkCmdDrawIndirect)(submitBuffer, vkIndirectBuffer, offset + i*stride,
				1, stride);
		}
	}
	return true;
}

bool dsVkRenderer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginIndexedDraw(commandBuffer, submitBuffer, geometry, NULL,
			primitiveType))
	{
		return false;
	}

	dsVkGfxBufferData* indirectBufferData = dsVkGfxBuffer_getData((dsGfxBuffer*)indirectBuffer,
		commandBuffer);
	if (!indirectBufferData)
		return false;

	dsVkRenderer_processGfxBuffer(renderer, indirectBufferData);
	VkBuffer vkIndirectBuffer = dsVkGfxBufferData_getBuffer(indirectBufferData);
	if (device->features.multiDrawIndirect)
	{
		DS_VK_CALL(device->vkCmdDrawIndexedIndirect)(submitBuffer, vkIndirectBuffer, offset, count,
			stride);
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			DS_VK_CALL(device->vkCmdDrawIndexedIndirect)(submitBuffer, vkIndirectBuffer,
				offset + i*stride, count, stride);
		}
	}
	return true;
}

bool dsVkRenderer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer, uint32_t x,
	uint32_t y, uint32_t z)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginDispatch(renderer, submitBuffer, commandBuffer))
		return false;

	DS_VK_CALL(device->vkCmdDispatch)(submitBuffer, x, y, z);
	return true;
}

bool dsVkRenderer_dispatchComputeIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	dsVkGfxBufferData* indirectBufferData = dsVkGfxBuffer_getData((dsGfxBuffer*)indirectBuffer,
		commandBuffer);
	if (!indirectBufferData)
		return false;

	dsVkRenderer_processGfxBuffer(renderer, indirectBufferData);
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer || !beginDispatch(renderer, submitBuffer, commandBuffer))
		return false;

	DS_VK_CALL(device->vkCmdDispatchIndirect)(submitBuffer,
		dsVkGfxBufferData_getBuffer(indirectBufferData), offset);
	return true;
}

bool dsVkRenderer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, uint32_t regionCount, dsBlitFilter filter)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	uint32_t srcFaceCount = 1;
	bool srcIs3D = false;
	if (srcSurfaceType == dsGfxSurfaceType_Offscreen)
	{
		dsTexture* srcTexture = (dsTexture*)srcSurface;
		if (srcTexture->info.dimension == dsTextureDim_Cube)
			srcFaceCount = 6;
		srcIs3D = srcTexture->info.dimension == dsTextureDim_3D;
	}

	uint32_t dstFaceCount = 1;
	bool dstIs3D = false;
	if (dstSurfaceType == dsGfxSurfaceType_Offscreen)
	{
		dsTexture* dstTexture = (dsTexture*)dstSurface;
		if (dstTexture->info.dimension == dsTextureDim_Cube)
			dstFaceCount = 6;
		dstIs3D = dstTexture->info.dimension == dsTextureDim_3D;
	}

	uint32_t minSrcMipLevel = UINT_MAX;
	uint32_t maxSrcMipLevel = 0;
	uint32_t minSrcLayer = UINT_MAX;
	uint32_t maxSrcLayer = 0;
	uint32_t minDstMipLevel = UINT_MAX;
	uint32_t maxDstMipLevel = 0;
	uint32_t minDstLayer = UINT_MAX;
	uint32_t maxDstLayer = 0;
	if (srcIs3D)
		minSrcLayer = maxSrcLayer = 0;
	if (dstIs3D)
		minDstLayer = maxDstLayer = 0;
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsTexturePosition* srcPosition = &regions[i].srcPosition;
		minSrcMipLevel = dsMin(minSrcMipLevel, srcPosition->mipLevel);
		maxSrcMipLevel = dsMin(maxSrcMipLevel, srcPosition->mipLevel);
		if (!srcIs3D)
		{
			uint32_t srcLayer = srcPosition->depth*srcFaceCount + srcPosition->face;
			minSrcLayer = dsMin(minSrcLayer, srcLayer);
			maxSrcLayer = dsMax(maxSrcLayer, srcLayer + regions[i].layers - 1);
		}

		const dsTexturePosition* dstPosition = &regions[i].dstPosition;
		minDstMipLevel = dsMin(minDstMipLevel, dstPosition->mipLevel);
		maxDstMipLevel = dsMin(maxDstMipLevel, dstPosition->mipLevel);
		if (!dstIs3D)
		{
			uint32_t dstLayer = dstPosition->depth*dstFaceCount + dstPosition->face;
			minDstLayer = dsMin(minDstLayer, dstLayer);
			maxDstLayer = dsMax(maxDstLayer, dstLayer + regions[i].layers - 1);
		}
	}

	VkImageMemoryBarrier imageBarriers[2] =
	{
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			0,
			VK_ACCESS_TRANSFER_READ_BIT,
			0,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			0,
			{0, minSrcMipLevel, maxSrcMipLevel - minSrcMipLevel + 1, minSrcLayer,
				maxSrcLayer - minSrcLayer + 1}
		},
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			0,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			0,
			{0, minDstMipLevel, maxDstMipLevel - minDstMipLevel + 1, minDstLayer,
				maxDstLayer - minDstLayer + 1}
		}
	};

	// Image barriers to prepare for the blit.
	VkImageAspectFlags srcAspectMask = 0, dstAspectMask = 0;
	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	if (!setBeginBlitSurfaceBarrierInfo(renderer, commandBuffer, imageBarriers, srcSurfaceType,
			srcSurface, &srcAspectMask, &stageFlags) ||
		!setBeginBlitSurfaceBarrierInfo(renderer, commandBuffer, imageBarriers + 1, dstSurfaceType,
			dstSurface, &dstAspectMask, &stageFlags))
	{
		return false;
	}
	imageBarriers[0].subresourceRange.aspectMask = srcAspectMask;
	imageBarriers[1].subresourceRange.aspectMask = dstAspectMask;

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, stageFlags,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2, imageBarriers);

	// Perform the blit.
	// 512 regions is ~41 KB of stack space. After that use heap space.
	bool heapRegions = regionCount > 512;
	VkImageBlit* imageBlits;
	if (heapRegions)
	{
		imageBlits = DS_ALLOCATE_OBJECT_ARRAY(renderer->allocator, VkImageBlit, regionCount);
		if (!imageBlits)
			return false;
	}
	else
		imageBlits = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkImageBlit, regionCount);

	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsSurfaceBlitRegion* region = regions + i;
		const dsTexturePosition* srcPosition = &region->srcPosition;
		const dsTexturePosition* dstPosition = &region->dstPosition;
		VkImageBlit* imageBlit = imageBlits + i;

		imageBlit->srcSubresource.aspectMask = srcAspectMask;
		imageBlit->srcSubresource.mipLevel = srcPosition->mipLevel;
		imageBlit->srcSubresource.baseArrayLayer = srcPosition->depth*srcFaceCount +
			srcPosition->face;
		imageBlit->srcSubresource.layerCount = srcIs3D ? 1 : region->layers;

		imageBlit->srcOffsets[0].x = srcPosition->x;
		imageBlit->srcOffsets[0].y = srcPosition->y;
		if (srcIs3D)
			imageBlit->srcOffsets[0].z = srcPosition->depth;
		else
			imageBlit->srcOffsets[0].z = 0;

		imageBlit->srcOffsets[1].x = srcPosition->x + region->srcWidth;
		imageBlit->srcOffsets[1].y = srcPosition->y + region->srcHeight;
		if (srcIs3D)
			imageBlit->srcOffsets[1].z = srcPosition->depth + region->layers;
		else
			imageBlit->srcOffsets[1].z = 1;

		imageBlit->dstSubresource.aspectMask = dstAspectMask;
		imageBlit->dstSubresource.mipLevel = dstPosition->mipLevel;
		imageBlit->dstSubresource.baseArrayLayer = dstPosition->depth*srcFaceCount +
			dstPosition->face;
		imageBlit->dstSubresource.layerCount = dstIs3D ? 1 : region->layers;

		imageBlit->dstOffsets[0].x = dstPosition->x;
		imageBlit->dstOffsets[0].y = dstPosition->y;
		if (dstIs3D)
			imageBlit->dstOffsets[0].z = dstPosition->depth;
		else
			imageBlit->dstOffsets[0].z = 0;

		imageBlit->dstOffsets[1].x = dstPosition->x + region->dstWidth;
		imageBlit->dstOffsets[1].y = dstPosition->y + region->dstHeight;
		if (dstIs3D)
			imageBlit->dstOffsets[1].z = dstPosition->depth + region->layers;
		else
			imageBlit->dstOffsets[1].z = 1;
	}

	VkFilter vkFilter = filter == dsBlitFilter_Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	DS_VK_CALL(device->vkCmdBlitImage)(submitBuffer, imageBarriers[0].image,
		imageBarriers[0].newLayout, imageBarriers[1].image, imageBarriers[1].newLayout, regionCount,
		imageBlits, vkFilter);

	if (heapRegions)
		DS_VERIFY(dsAllocator_free(renderer->allocator, imageBlits));

	// Image barriers to clean up after the blit.
	stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	imageBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	setEndBlitSurfaceBarrierInfo(renderer, imageBarriers, srcSurfaceType, srcSurface, &stageFlags);

	imageBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	setEndBlitSurfaceBarrierInfo(renderer, imageBarriers + 1, dstSurfaceType, dstSurface,
		&stageFlags);

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		stageFlags, 0, 0, NULL, 0, NULL, 2, imageBarriers);

	return true;
}

bool dsVkRenderer_memoryBarrier(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxPipelineStage beforeStages, dsGfxPipelineStage afterStages,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	VkMemoryBarrier* memoryBarriers = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkMemoryBarrier, barrierCount);
	for (uint32_t i = 0; i < barrierCount; ++i)
	{
		memoryBarriers[i].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarriers[i].pNext = NULL;
		memoryBarriers[i].srcAccessMask = dsVkAccessFlags(barriers[i].beforeAccess);
		memoryBarriers[i].dstAccessMask = dsVkAccessFlags(barriers[i].afterAccess);
	}

	VkPipelineStageFlags srcStages =  dsVkPipelineStageFlags(renderer, beforeStages, true);
	VkPipelineStageFlags dstStages =  dsVkPipelineStageFlags(renderer, afterStages, false);
	VkDependencyFlags dependencyFlags =
		commandBuffer->boundRenderPass ? VK_DEPENDENCY_BY_REGION_BIT : 0;
	DS_VK_CALL(device->vkCmdPipelineBarrier)(submitBuffer, srcStages, dstStages, dependencyFlags,
		barrierCount, memoryBarriers, 0, NULL, 0, NULL);
	return true;
}

bool dsVkRenderer_flush(dsRenderer* renderer)
{
	dsVkRenderer_flushImpl(renderer, true, false);
	return true;
}

bool dsVkRenderer_waitUntilIdle(dsRenderer* renderer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	// NOTE: Don't lock for submitCuont since waitUntilIdle() can only be called on the main thread.
	uint64_t submitCount = vkRenderer->submitCount;
	preFlush(renderer, true, false);
	DS_VK_CALL(device->vkQueueWaitIdle)(device->queue);
	postFlush(renderer);

	DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));
	for (uint32_t i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
		freeAllResources(vkRenderer->deleteResources + i, false);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));

	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
	DS_ATOMIC_STORE64(&vkRenderer->finishedSubmitCount, &submitCount);
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));
	return true;
}

bool dsVkRenderer_destroy(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;
	dsVkInstance* instance = &device->instance;

	dsRenderer_shutdownResources(renderer);

	if (device && device->vkQueueWaitIdle)
		DS_VK_CALL(device->vkQueueWaitIdle)(device->queue);

	for (uint32_t i = 0; i < DS_MAX_SUBMITS; ++i)
	{
		dsVkSubmitInfo* submit = vkRenderer->submits + i;
		dsVkCommandBuffer_shutdown(&submit->commandBuffer);

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

	if (vkRenderer->defaultSampler)
	{
		DS_VK_CALL(device->vkDestroySampler)(device->device, vkRenderer->defaultSampler,
			instance->allocCallbacksPtr);
	}

	dsVkBarrierList_shutdown(&vkRenderer->preResourceBarriers);
	dsVkBarrierList_shutdown(&vkRenderer->postResourceBarriers);
	for (unsigned int i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkProcessResourceList_shutdown(&vkRenderer->pendingResources[i]);
	for (unsigned int i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
	{
		dsVkResourceList* deleteResources = vkRenderer->deleteResources + i;
		freeAllResources(deleteResources, true);
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

bool dsVkRenderer_pushDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const char* name)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	if ((!instance->vkCmdBeginDebugUtilsLabelEXT && !device->vkCmdDebugMarkerBeginEXT) ||
		device->buggyDebugLabels)
	{
		return true;
	}

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	if (instance->vkCmdBeginDebugUtilsLabelEXT)
	{
		VkDebugUtilsLabelEXT label =
		{
			VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			NULL,
			name,
			{0.0f, 0.0f, 0.0f, 0.0f}
		};
		DS_VK_CALL(instance->vkCmdBeginDebugUtilsLabelEXT)(submitBuffer, &label);
	}
	else
	{
		DS_ASSERT(device->vkCmdDebugMarkerBeginEXT);
		VkDebugMarkerMarkerInfoEXT label =
		{
			VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			NULL,
			name,
			{0.0f, 0.0f, 0.0f, 0.0f}
		};
		DS_VK_CALL(device->vkCmdDebugMarkerBeginEXT)(submitBuffer, &label);
	}

	return true;
}

bool dsVkRenderer_popDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;
	if ((!instance->vkCmdEndDebugUtilsLabelEXT && !device->vkCmdDebugMarkerEndEXT) ||
		device->buggyDebugLabels)
	{
		return true;
	}

	VkCommandBuffer submitBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!submitBuffer)
		return false;

	if (instance->vkCmdEndDebugUtilsLabelEXT)
		DS_VK_CALL(instance->vkCmdEndDebugUtilsLabelEXT)(submitBuffer);
	else
	{
		DS_ASSERT(device->vkCmdDebugMarkerEndEXT);
		DS_VK_CALL(device->vkCmdDebugMarkerEndEXT)(submitBuffer);
	}
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

bool dsVkRenderer_getDefaultDevice(dsRenderDeviceInfo* outDevice)
{
	return dsGetDefaultVkDevice(outDevice);
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

	size_t bufferSize = fullAllocSize();
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));
	dsVkRenderer* renderer = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVkRenderer);
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

	dsVkBarrierList_initialize(&renderer->preResourceBarriers, allocator, &renderer->device);
	dsVkBarrierList_initialize(&renderer->postResourceBarriers, allocator, &renderer->device);
	for (uint32_t i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkProcessResourceList_initialize(renderer->pendingResources + i, allocator);
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
	if (deviceProperties->apiVersion >= DS_ENCODE_VERSION(1, 3, 0))
		baseRenderer->shaderVersion = DS_ENCODE_VERSION(1, 6, 0);
	else if (deviceProperties->apiVersion >= DS_ENCODE_VERSION(1, 2, 0))
		baseRenderer->shaderVersion = DS_ENCODE_VERSION(1, 5, 0);
	else if (deviceProperties->apiVersion >= DS_ENCODE_VERSION(1, 1, 0))
		baseRenderer->shaderVersion = DS_ENCODE_VERSION(1, 3, 0);
	else
		baseRenderer->shaderVersion = DS_ENCODE_VERSION(1, 0, 0);

	if (baseRenderer->deviceName)
		DS_LOG_DEBUG_F(DS_RENDER_VULKAN_LOG_TAG, "Using device: %s", baseRenderer->deviceName);

	VkPhysicalDeviceFeatures deviceFeatures;
	DS_VK_CALL(instance->vkGetPhysicalDeviceFeatures)(device->physicalDevice, &deviceFeatures);

	const VkPhysicalDeviceLimits* limits = &deviceProperties->limits;
	baseRenderer->maxColorAttachments = dsMin(limits->maxColorAttachments, DS_MAX_ATTACHMENTS);
	// framebufferColorSampleCounts is a bitmask. Compute the maximum bit that's set.
	baseRenderer->maxSurfaceSamples = 1U << (31 - dsClz(limits->framebufferColorSampleCounts));
	baseRenderer->maxSurfaceSamples = dsMin(baseRenderer->maxSurfaceSamples,
		DS_MAX_ANTIALIAS_SAMPLES);
	baseRenderer->maxAnisotropy = limits->maxSamplerAnisotropy;

	baseRenderer->surfaceSamples = dsClamp(options->surfaceSamples, 1U,
		baseRenderer->maxSurfaceSamples);
	baseRenderer->defaultSamples = dsClamp(options->defaultSamples, 1U,
		baseRenderer->maxSurfaceSamples);
	baseRenderer->defaultAnisotropy = 1;
	baseRenderer->projectionOptions = dsProjectionMatrixOptions_HalfZRange |
		dsProjectionMatrixOptions_InvertY;
	if (options->reverseZ)
		baseRenderer->projectionOptions |= dsProjectionMatrixOptions_InvertZ;

	for (int i = 0; i < 3; ++i)
		baseRenderer->maxComputeWorkGroupSize[i] = limits->maxComputeWorkGroupCount[i];

	baseRenderer->singleBuffer = false;
	baseRenderer->stereoscopic = options->stereoscopic;
	baseRenderer->vsync = false;
	baseRenderer->hasGeometryShaders = deviceFeatures.geometryShader != 0;
	baseRenderer->hasTessellationShaders = deviceFeatures.tessellationShader != 0;
	baseRenderer->hasNativeMultidraw = true;
	baseRenderer->hasInstancedDrawing = true;
	baseRenderer->hasStartInstance = (bool)deviceFeatures.drawIndirectFirstInstance;
	baseRenderer->hasIndependentBlend = (bool)deviceFeatures.independentBlend;
	baseRenderer->hasDualSrcBlend = (bool)deviceFeatures.dualSrcBlend;
	baseRenderer->hasLogicOps = (bool)deviceFeatures.logicOp;
	baseRenderer->hasSampleShading = (bool)deviceFeatures.sampleRateShading;
	baseRenderer->hasDepthBounds = (bool)deviceFeatures.depthBounds;
	baseRenderer->hasDepthClamp = (bool)deviceFeatures.depthClamp;
	baseRenderer->hasDepthBiasClamp = (bool)deviceFeatures.depthBiasClamp;
	baseRenderer->hasDepthStencilMultisampleResolve = device->hasDepthStencilResolve;
	baseRenderer->hasFragmentInputs = false;
	baseRenderer->projectedTexCoordTInverted = false;

	baseRenderer->resourceManager = dsVkResourceManager_create(allocator, renderer,
		options->shaderCacheDir);
	if (!baseRenderer->resourceManager)
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	dsGfxFormat colorFormat = dsRenderer_optionsColorFormat(options,
		useBGRASurface(baseRenderer->deviceName), true);
	if (!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, colorFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Can't draw to surface color format.");
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	dsGfxFormat depthFormat = dsRenderer_optionsDepthFormat(options);
	// AMD doesn't support 24-bit dpeth.
	if (depthFormat == dsGfxFormat_D24S8 &&
		!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
	{
		depthFormat = dsGfxFormat_D32S8_Float;
	}
	else if (depthFormat == dsGfxFormat_X8D24 &&
		!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
	{
		depthFormat = dsGfxFormat_D32_Float;
	}

	if (depthFormat != dsGfxFormat_Unknown &&
		!dsGfxFormat_renderTargetSupported(baseRenderer->resourceManager, depthFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "Can't draw to surface depth format.");
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	baseRenderer->surfaceColorFormat = colorFormat;
	renderer->colorSurfaceAlpha = options->alphaBits > 0;
	baseRenderer->surfaceDepthStencilFormat = depthFormat;

	if (!createCommandBuffers(renderer))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	renderer->defaultSampler = createDefaultSampler(&renderer->device);
	if (!renderer->defaultSampler)
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
	baseRenderer->createCommandBuffersFunc = &dsVkCommandBufferPool_createCommandBuffers;
	baseRenderer->destroyCommandBufferPoolFunc = &dsVkCommandBufferPool_destroy;
	baseRenderer->resetCommandBufferPoolFunc = &dsVkCommandBufferPool_reset;

	// Command buffers
	baseRenderer->beginCommandBufferFunc = &dsVkCommandBuffer_begin;
	baseRenderer->beginSecondaryCommandBufferFunc = &dsVkCommandBuffer_beginSecondary;
	baseRenderer->endCommandBufferFunc = &dsVkCommandBuffer_end;
	baseRenderer->submitCommandBufferFunc = &dsVkCommandBuffer_submit;

	// Render passes
	baseRenderer->createRenderPassFunc = &dsVkRenderPass_create;
	baseRenderer->destroyRenderPassFunc = &dsVkRenderPass_destroy;
	baseRenderer->beginRenderPassFunc = &dsVkRenderPass_begin;
	baseRenderer->nextRenderSubpassFunc = &dsVkRenderPass_nextSubpass;
	baseRenderer->endRenderPassFunc = &dsVkRenderPass_end;

	// Renderer
	baseRenderer->beginFrameFunc = &dsVkRenderer_beginFrame;
	baseRenderer->endFrameFunc = &dsVkRenderer_endFrame;
	baseRenderer->setSurfaceSamplesFunc = &dsVkRenderer_setSurfaceSamples;
	baseRenderer->setDefaultSamplesFunc = &dsVkRenderer_setDefaultSamples;
	baseRenderer->setVSyncFunc = &dsVkRenderer_setVSync;
	baseRenderer->setDefaultAnisotropyFunc = &dsVkRenderer_setDefaultAnisotropy;
	baseRenderer->clearAttachmentsFunc = &dsVkRenderer_clearAttachments;
	baseRenderer->drawFunc = &dsVkRenderer_draw;
	baseRenderer->drawIndexedFunc = &dsVkRenderer_drawIndexed;
	baseRenderer->drawIndirectFunc = &dsVkRenderer_drawIndirect;
	baseRenderer->drawIndexedIndirectFunc = &dsVkRenderer_drawIndexedIndirect;
	baseRenderer->dispatchComputeFunc = &dsVkRenderer_dispatchCompute;
	baseRenderer->dispatchComputeIndirectFunc = &dsVkRenderer_dispatchComputeIndirect;
	baseRenderer->blitSurfaceFunc = &dsVkRenderer_blitSurface;
	baseRenderer->memoryBarrierFunc = &dsVkRenderer_memoryBarrier;
	baseRenderer->pushDebugGroupFunc = &dsVkRenderer_pushDebugGroup;
	baseRenderer->popDebugGroupFunc = &dsVkRenderer_popDebugGroup;;
	baseRenderer->flushFunc = &dsVkRenderer_flush;
	baseRenderer->waitUntilIdleFunc = &dsVkRenderer_waitUntilIdle;

	DS_VERIFY(dsRenderer_initializeResources(baseRenderer));

	return baseRenderer;
}

VkSemaphore dsVkRenderer_flushImpl(dsRenderer* renderer, bool readback, bool useSemaphore)
{
	DS_PROFILE_FUNC_START();

	VkSemaphore submittedSemaphore = preFlush(renderer, readback, useSemaphore);
	postFlush(renderer);
	DS_PROFILE_FUNC_RETURN(submittedSemaphore);
}

dsGfxFenceResult dsVkRenderer_waitForSubmit(dsRenderer* renderer, uint64_t submitCount,
	uint64_t timeout)
{
	VkFence fences[DS_MAX_SUBMITS];
	uint32_t fenceCount = 0;

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	if (dsVkRenderer_getFinishedSubmitCount(renderer) >= submitCount)
	{
		// Already synchronized to this submit.
		return dsGfxFenceResult_Success;
	}

	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
	if (vkRenderer->submitCount <= submitCount)
	{
		// Haven't submitted this yet to Vulkan.
		DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));
		return dsGfxFenceResult_WaitingToQueue;
	}

	++vkRenderer->waitCount;
	for (uint32_t i = 0; i < DS_MAX_SUBMITS; ++i)
	{
		dsVkSubmitInfo* submit = vkRenderer->submits + i;
		// NOTE: Only written inside of lock, so don't need atomic load.
		if (submit->submitIndex > vkRenderer->finishedSubmitCount &&
			submit->submitIndex <= submitCount)
		{
			fences[fenceCount++] = submit->fence;
		}
	}
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));

	dsVkDevice* device = &vkRenderer->device;
	VkResult result;
	if (fenceCount > 0)
	{
		DS_PROFILE_WAIT_START("vkWaitForFences");
		result = DS_VK_CALL(device->vkWaitForFences)(device->device, fenceCount, fences, true,
			timeout);
		DS_PROFILE_WAIT_END();
	}
	else
		result = VK_SUCCESS;

	DS_VERIFY(dsMutex_lock(vkRenderer->submitLock));
	if (--vkRenderer->waitCount == 0)
		DS_VERIFY(dsConditionVariable_notifyAll(vkRenderer->waitCondition));
	if (result == VK_SUCCESS && submitCount > vkRenderer->finishedSubmitCount)
		DS_ATOMIC_STORE64(&vkRenderer->finishedSubmitCount, &submitCount);
	DS_VERIFY(dsMutex_unlock(vkRenderer->submitLock));

	switch (result)
	{
		case VK_SUCCESS:
			return dsGfxFenceResult_Success;
		case VK_TIMEOUT:
			return dsGfxFenceResult_Timeout;
		case VK_ERROR_DEVICE_LOST:
			DS_LOG_FATAL_F(DS_RENDER_VULKAN_LOG_TAG, "Vulkan device was lost.");
			abort();
		default:
			DS_HANDLE_VK_RESULT(result, "Couldn't wait for fence");
			return dsGfxFenceResult_Error;
	}
}

uint64_t dsVkRenderer_getFinishedSubmitCount(const dsRenderer* renderer)
{
	const dsVkRenderer* vkRenderer = (const dsVkRenderer*)renderer;
	uint64_t finishedSubmitCount;
	DS_ATOMIC_LOAD64(&vkRenderer->finishedSubmitCount, &finishedSubmitCount);
	return finishedSubmitCount;
}

void dsVkRenderer_processGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer)
{
	DS_ASSERT(buffer);

	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;

	DS_VERIFY(dsSpinlock_lock(&buffer->resource.lock));

	// Once it's processed, it's now considered used.
	buffer->used = true;

	// Make sure this needs to be processed.
	if (!buffer->hostBuffer || (!buffer->needsInitialCopy && buffer->dirtyRangeCount == 0))
	{
		DS_VERIFY(dsSpinlock_unlock(&buffer->resource.lock));
		return;
	}

	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));
	dsVkProcessResourceList* resourceList = vkRenderer->pendingResources +
		vkRenderer->curPendingResources;

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

	dsVkProcessResourceList_addBuffer(resourceList, buffer);
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
	dsVkProcessResourceList* resourceList = vkRenderer->pendingResources +
		vkRenderer->curPendingResources;

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

	dsVkProcessResourceList_addTexture(resourceList, texture);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
}

void dsVkRenderer_processRenderbuffer(dsRenderer* renderer, dsRenderbuffer* renderbuffer)
{
	// Only queued once during creation, so no need to check if it should be added.
	DS_ASSERT(renderbuffer);
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));
	dsVkProcessResourceList* resourceList = vkRenderer->pendingResources +
		vkRenderer->curPendingResources;
	dsVkProcessResourceList_addRenderbuffer(resourceList, renderbuffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
}

void dsVkRenderer_processRenderSurface(dsRenderer* renderer, dsVkRenderSurfaceData* surface)
{
	// Only queued once during creation, so no need to check if it should be added.
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));
	dsVkProcessResourceList* resourceList = vkRenderer->pendingResources +
		vkRenderer->curPendingResources;
	dsVkProcessResourceList_addRenderSurface(resourceList, surface);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
}

void dsVkRenderer_deleteGfxBuffer(dsRenderer* renderer, dsVkGfxBufferData* buffer, bool gpuFinished)
{
	if (!buffer)
		return;

	if (gpuFinished)
		dsVkGfxBufferData_destroy(buffer);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addBuffer(resourceList, buffer);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteTexture(dsRenderer* renderer, dsTexture* texture, bool gpuFinished)
{
	if (!texture)
		return;

	if (gpuFinished)
		dsVkTexture_destroyImpl(texture);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addTexture(resourceList, texture);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteTempBuffer(dsRenderer* renderer, dsVkTempBuffer* buffer, bool gpuFinished)
{
	if (!buffer)
		return;

	if (gpuFinished)
		dsVkTempBuffer_destroy(buffer);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addTempBuffer(resourceList, buffer);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteRenderbuffer(
	dsRenderer* renderer, dsRenderbuffer* renderbuffer, bool gpuFinished)
{
	if (!renderbuffer)
		return;

	if (gpuFinished)
		dsVkRenderbuffer_destroyImpl(renderbuffer);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addRenderbuffer(resourceList, renderbuffer);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteFramebuffer(
	dsRenderer* renderer, dsVkRealFramebuffer* framebuffer, bool gpuFinished)
{
	if (!framebuffer)
		return;

	if (gpuFinished)
		dsVkRealFramebuffer_destroy(framebuffer);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addFramebuffer(resourceList, framebuffer);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteFence(dsRenderer* renderer, dsGfxFence* fence, bool gpuFinished)
{
	if (!fence)
		return;

	if (gpuFinished)
		dsVkGfxFence_destroyImpl(fence);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addFence(resourceList, fence);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteQueriePool(dsRenderer* renderer, dsGfxQueryPool* queries, bool gpuFinished)
{
	if (!queries)
		return;

	if (gpuFinished)
		dsVkGfxQueryPool_destroyImpl(queries);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addQueries(resourceList, queries);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteMaterialDescriptor(
	dsRenderer* renderer, dsVkMaterialDescriptor* descriptor, bool gpuFinished)
{
	if (!descriptor)
		return;

	if (gpuFinished)
		dsVkMaterialDescriptor_destroy(descriptor);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addMaterialDescriptor(resourceList, descriptor);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteSamplerList(
	dsRenderer* renderer, dsVkSamplerList* samplers, bool gpuFinished)
{
	if (!samplers)
		return;

	if (gpuFinished)
		dsVkSamplerList_destroy(samplers);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addSamplerList(resourceList, samplers);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteComputePipeline(
	dsRenderer* renderer, dsVkComputePipeline* pipeline, bool gpuFinished)
{
	if (!pipeline)
		return;

	if (gpuFinished)
		dsVkComputePipeline_destroy(pipeline);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addComputePipeline(resourceList, pipeline);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deletePipeline(dsRenderer* renderer, dsVkPipeline* pipeline, bool gpuFinished)
{
	if (!pipeline)
		return;

	if (gpuFinished)
		dsVkPipeline_destroy(pipeline);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addPipeline(resourceList, pipeline);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteCommandPool(
	dsRenderer* renderer, dsVkCommandPoolData* pool, bool gpuFinished)
{
	if (!pool)
		return;

	if (gpuFinished)
		dsVkCommandPoolData_destroy(pool);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addCommandPool(resourceList, pool);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}

void dsVkRenderer_deleteRenderPass(
	dsRenderer* renderer, dsVkRenderPassData* renderPass, bool gpuFinished)
{
	if (!renderPass)
		return;

	if (gpuFinished)
		dsVkRenderPassData_destroy(renderPass);
	else
	{
		dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
		DS_VERIFY(dsSpinlock_lock(&vkRenderer->deleteLock));

		dsVkResourceList* resourceList =
			vkRenderer->deleteResources + vkRenderer->curDeleteResources;
		dsVkResourceList_addRenderPass(resourceList, renderPass);
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->deleteLock));
	}
}
