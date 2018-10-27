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

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkResourceManager.h"
#include "VkBarrierList.h"
#include "VkCommandBuffer.h"
#include "VkInit.h"
#include "VkResourceList.h"
#include "VkShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <string.h>

static size_t dsVkRenderer_fullAllocSize(const dsRendererOptions* options)
{
	size_t pathLen = options->shaderCacheDir ? strlen(options->shaderCacheDir) + 1 : 0;
	return DS_ALIGNED_SIZE(sizeof(dsVkRenderer)) + DS_ALIGNED_SIZE(pathLen) +
		dsMutex_fullAllocSize() + dsConditionVariable_fullAllocSize();
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

static void freeResources(dsVkRenderer* renderer)
{
	dsVkDevice* device = &renderer->device;

	DS_VERIFY(dsSpinlock_lock(&renderer->deleteLock));
	dsVkResourceList* prevDeleteList = renderer->deleteResources + renderer->curDeleteResources;
	renderer->curDeleteResources = (renderer->curDeleteResources + 1) % DS_DELETE_RESOURCES_ARRAY;
	DS_VERIFY(dsSpinlock_unlock(&renderer->deleteLock));

	for (uint32_t i = 0; i < prevDeleteList->bufferCount; ++i)
	{
		dsVkGfxBufferData* buffer = prevDeleteList->buffers[i];
		DS_ASSERT(buffer);

		uint32_t commandBufferCount;
		DS_ATOMIC_LOAD32(&buffer->commandBufferCount, &commandBufferCount);
		bool stillInUse = commandBufferCount > 0 ||
			(buffer->lastUsedSubmit != DS_NOT_SUBMITTED &&
				buffer->lastUsedSubmit > renderer->finishedSubmitCount) ||
			(buffer->uploadedSubmit != DS_NOT_SUBMITTED &&
				buffer->uploadedSubmit > renderer->finishedSubmitCount);
		if (stillInUse)
		{
			dsVkRenderer_deleteGfxBuffer((dsRenderer*)renderer, buffer);
			continue;
		}

		dsVkGfxBufferData_destroy(buffer, device);
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

		DS_VERIFY(dsSpinlock_lock(&buffer->lock));
		// Clear the submit queue now that we're processing it.
		buffer->submitQueue = NULL;
		if (buffer->mappedSize > 0)
		{
			// Still mapped, process later.
			DS_VERIFY(dsSpinlock_unlock(&buffer->lock));
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
		DS_VERIFY(dsSpinlock_unlock(&buffer->lock));

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
			{
				DS_ASSERT(doUpload);
				dsVkRenderer_processGfxBuffer(baseRenderer, buffer);
			}
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

	// Process the uploads.
	if (preResourceBarriers->bufferBarrierCount > 0)
	{
		DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
			preResourceBarriers->bufferBarrierCount, preResourceBarriers->bufferBarriers, 0, NULL);
	}

	for (uint32_t i = 0; i < renderer->bufferCopyInfoCount; ++i)
	{
		const dsVkBufferCopyInfo* copyInfo = renderer->bufferCopyInfos + i;
		DS_VK_CALL(device->vkCmdCopyBuffer)(commandBuffer, copyInfo->srcBuffer, copyInfo->dstBuffer,
			copyInfo->rangeCount, renderer->bufferCopies + copyInfo->firstRange);
	}

	if (postResourceBarriers->bufferBarrierCount > 0)
	{
		DS_VK_CALL(device->vkCmdPipelineBarrier)(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL,
			postResourceBarriers->bufferBarrierCount, postResourceBarriers->bufferBarriers,
			0, NULL);
	}

	dsVkResourceList_clear(prevResourceList);
}

void dsVkRenderer_flush(dsRenderer* renderer)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	dsVkDevice* device = &vkRenderer->device;

	// Get the submit queue, waiting if it's not ready.
	dsVkSubmitInfo* submit = vkRenderer->submits + vkRenderer->curSubmit;
	if (submit->submitIndex != DS_NOT_SUBMITTED)
	{
		// 10 seconds in nanoseconds
		const uint64_t timeout = 10000000000;
		DS_VK_CALL(device->vkWaitForFences)(device->device, 1, &submit->fence, true, timeout);
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

	// Make sure any writes are visible for mapping buffers.
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
		VK_PIPELINE_STAGE_TRANSFER_BIT;
	if (renderer->hasTessellationShaders)
	{
		srcStage |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (renderer->hasGeometryShaders)
		srcStage |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_HOST_BIT;

	VkMemoryBarrier memoryBarrier =
	{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT
	};

	DS_VK_CALL(device->vkCmdPipelineBarrier)(submit->renderCommands, srcStage, dstStage, 0, 1,
		&memoryBarrier, 0, NULL, 0, NULL);

	// Submit the queue.
	DS_VK_CALL(device->vkEndCommandBuffer)(submit->resourceCommands);
	DS_VK_CALL(device->vkEndCommandBuffer)(submit->renderCommands);

	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		0, NULL, NULL,
		2, &submit->resourceCommands,
		0, NULL
	};
	DS_VK_CALL(device->vkQueueSubmit)(device->queue, 1, &submitInfo, submit->fence);

	// Update the main command buffer.
	dsVkCommandBuffer_submittedResources(renderer->mainCommandBuffer, vkRenderer->submitCount);
	submit = vkRenderer->submits + vkRenderer->curSubmit;
	vkRenderer->mainCommandBuffer.vkCommandBuffer = submit->renderCommands;

	VkCommandBufferBeginInfo beginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};
	DS_VK_CALL(device->vkBeginCommandBuffer)(submit->resourceCommands, &beginInfo);
	DS_VK_CALL(device->vkBeginCommandBuffer)(submit->renderCommands, &beginInfo);
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
		if (submit->resourceCommands)
		{
			DS_VK_CALL(device->vkFreeCommandBuffers)(device->device, vkRenderer->commandPool, 2,
				&submit->resourceCommands);
		}

		if (submit->fence)
		{
			DS_VK_CALL(device->vkDestroyFence)(device->device, submit->fence,
				instance->allocCallbacksPtr);
		}
	}

	if (vkRenderer->commandPool)
	{
		DS_VK_CALL(device->vkDestroyCommandPool)(device->device, vkRenderer->commandPool,
			instance->allocCallbacksPtr);
	}

	dsVkResourceList_shutdown(&vkRenderer->mainCommandBuffer.usedResources);
	dsVkBarrierList_shutdown(&vkRenderer->mainCommandBuffer.barriers);

	dsVkBarrierList_shutdown(&vkRenderer->preResourceBarriers);
	dsVkBarrierList_shutdown(&vkRenderer->postResourceBarriers);
	for (unsigned int i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkResourceList_shutdown(&vkRenderer->pendingResources[i]);
	for (unsigned int i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
	{
		dsVkResourceList* deleteResources = vkRenderer->deleteResources + i;
		for (uint32_t i = 0; i < deleteResources->bufferCount; ++i)
			dsVkGfxBufferData_destroy(deleteResources->buffers[i], device);

		dsVkResourceList_shutdown(deleteResources);
	}

	dsVkResourceManager_destroy((dsVkResourceManager*)renderer->resourceManager);
	dsDestroyVkDevice(device);
	dsDestroyVkInstance(&device->instance);
	dsSpinlock_shutdown(&vkRenderer->resourceLock);
	dsSpinlock_shutdown(&vkRenderer->deleteLock);
	dsMutex_destroy(vkRenderer->submitLock);
	dsConditionVariable_destroy(vkRenderer->waitCondition);

	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->bufferCopies));
	DS_VERIFY(dsAllocator_free(renderer->allocator, vkRenderer->bufferCopyInfos));
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

	dsVkResourceList_initialize(&renderer->mainCommandBuffer.usedResources, allocator);
	dsVkBarrierList_initialize(&renderer->mainCommandBuffer.barriers, allocator,
		&renderer->device);

	dsVkBarrierList_initialize(&renderer->preResourceBarriers, allocator, &renderer->device);
	dsVkBarrierList_initialize(&renderer->postResourceBarriers, allocator, &renderer->device);
	for (uint32_t i = 0; i < DS_PENDING_RESOURCES_ARRAY; ++i)
		dsVkResourceList_initialize(renderer->pendingResources + i, allocator);
	for (uint32_t i = 0; i < DS_DELETE_RESOURCES_ARRAY; ++i)
		dsVkResourceList_initialize(renderer->deleteResources + i, allocator);

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

	if (!createCommandBuffers(renderer))
	{
		dsVkRenderer_destroy(baseRenderer);
		return NULL;
	}

	return baseRenderer;
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
	DS_VERIFY(dsSpinlock_lock(&vkRenderer->resourceLock));

	// Once it's processed, it's now considered used.
	buffer->used = true;

	// Make sure this needs to be processed.
	if (!buffer->deviceBuffer || !buffer->hostBuffer ||
		(!buffer->needsInitialCopy && buffer->dirtyRangeCount == 0))
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
		return;
	}

	dsVkResourceList* resourceList = vkRenderer->pendingResources + vkRenderer->curPendingResources;

	DS_VERIFY(dsSpinlock_lock(&buffer->lock));
	// Keep track of the submit queue. If it's already on a queue, don't do anything.
	void* submitQueue;
	submitQueue = buffer->submitQueue;
	if (!submitQueue)
		buffer->submitQueue = resourceList;
	DS_VERIFY(dsSpinlock_unlock(&buffer->lock));

	if (submitQueue)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
		return;
	}

	dsVkResourceList_addBuffer(resourceList, buffer);
	DS_VERIFY(dsSpinlock_unlock(&vkRenderer->resourceLock));
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
