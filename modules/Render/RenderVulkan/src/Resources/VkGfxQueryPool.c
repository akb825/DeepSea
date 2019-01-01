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

#include "Resources/VkGfxQueryPool.h"

#include "Resources/VkGfxBuffer.h"
#include "Resources/VkGfxBufferData.h"
#include "Resources/VkResource.h"
#include "VkCommandBuffer.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>

dsGfxQueryPool* dsVkGfxQueryPool_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count)
{
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	VkQueryType vkType;
	switch (type)
	{
		case dsGfxQueryType_SamplesPassed:
		case dsGfxQueryType_AnySamplesPassed:
			vkType = VK_QUERY_TYPE_TIMESTAMP;
			break;
		case dsGfxQueryType_Timestamp:
			vkType = VK_QUERY_TYPE_TIMESTAMP;
			break;
		default:
			DS_ASSERT(false);
			return NULL;
	}

	VkQueryPoolCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
		NULL,
		0,
		vkType,
		count,
		0
	};
	VkQueryPool vkQueries;
	VkResult result = DS_VK_CALL(device->vkCreateQueryPool)(device->device, &createInfo,
		instance->allocCallbacksPtr, &vkQueries);
	if (!dsHandleVkResult(result))
		return NULL;

	dsVkGfxQueryPool* queries = DS_ALLOCATE_OBJECT(allocator, dsVkGfxQueryPool);
	if (!queries)
	{
		DS_VK_CALL(device->vkDestroyQueryPool)(device->device, vkQueries,
			instance->allocCallbacksPtr);
		return NULL;
	}

	dsGfxQueryPool* baseQueries = (dsGfxQueryPool*)queries;
	baseQueries->resourceManager = resourceManager;
	baseQueries->allocator = dsAllocator_keepPointer(allocator);
	baseQueries->type = type;
	baseQueries->count = count;

	dsVkResource_initialize(&queries->resource);
	queries->vkQueries = vkQueries;

	return baseQueries;
}

bool dsVkGfxQueryPool_reset(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkQueries->resource))
		return false;

	DS_VK_CALL(device->vkCmdResetQueryPool)(vkCommandBuffer, vkQueries->vkQueries, first, count);
	return true;
}

bool dsVkGfxQueryPool_beginQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkQueries->resource))
		return false;

	VkQueryControlFlags flags = 0;
	if (queries->type == dsGfxQueryType_SamplesPassed)
		flags = VK_QUERY_CONTROL_PRECISE_BIT;
	DS_VK_CALL(device->vkCmdBeginQuery)(vkCommandBuffer, vkQueries->vkQueries, query, flags);
	return true;
}

bool dsVkGfxQueryPool_endQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkQueries->resource))
		return false;

	DS_VK_CALL(device->vkCmdEndQuery)(vkCommandBuffer, vkQueries->vkQueries, query);
	return true;
}

bool dsVkGfxQueryPool_queryTimestamp(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkQueries->resource))
		return false;

	DS_VK_CALL(device->vkCmdWriteTimestamp)(vkCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		vkQueries->vkQueries, query);
	return true;
}

bool dsVkGfxQueryPool_getValues(dsResourceManager* resourceManager, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, void* data, size_t dataSize, size_t stride, size_t elementSize,
	bool checkAvailability)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	DS_ASSERT(elementSize == sizeof(uint32_t) || elementSize == sizeof(uint64_t));
	VkQueryResultFlags flags = 0;
	if (elementSize == sizeof(uint64_t))
		flags |= VK_QUERY_RESULT_64_BIT;
	if (checkAvailability)
		flags |= VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
	else
		flags |= VK_QUERY_RESULT_WAIT_BIT;
	VkResult result = DS_VK_CALL(device->vkGetQueryPoolResults)(device->device,
		vkQueries->vkQueries, first, count, dataSize, data, stride, flags);
	return dsHandleVkResult(result);
}

bool dsVkGfxQueryPool_copyValues(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset,
	size_t stride, size_t elementSize, bool checkAvailability)
{
	DS_UNUSED(resourceManager);
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)resourceManager->renderer)->device;

	VkCommandBuffer vkCommandBuffer = dsVkCommandBuffer_getCommandBuffer(commandBuffer);
	if (!vkCommandBuffer)
		return false;

	if (!dsVkCommandBuffer_addResource(commandBuffer, &vkQueries->resource))
		return false;

	dsVkGfxBufferData* bufferData = dsVkGfxBuffer_getData(buffer, commandBuffer);

	VkBuffer dstBuffer = dsVkGfxBufferData_getBuffer(bufferData);

	VkQueryResultFlags flags = 0;
	if (elementSize == sizeof(uint64_t))
		flags |= VK_QUERY_RESULT_64_BIT;
	if (checkAvailability)
		flags |= VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
	else
		flags |= VK_QUERY_RESULT_WAIT_BIT;

	DS_VK_CALL(device->vkCmdCopyQueryPoolResults)(vkCommandBuffer, vkQueries->vkQueries, first,
		count, dstBuffer, offset, stride, flags);
	return true;
}

bool dsVkGfxQueryPool_destroy(dsResourceManager* resourceManager, dsGfxQueryPool* queries)
{
	dsVkRenderer_deleteQueriePool(resourceManager->renderer, queries);
	return true;
}

void dsVkGfxQueryPool_destroyImpl(dsGfxQueryPool* queries)
{
	dsVkGfxQueryPool* vkQueries = (dsVkGfxQueryPool*)queries;
	dsVkDevice* device = &((dsVkRenderer*)queries->resourceManager->renderer)->device;
	dsVkInstance* instance = &device->instance;

	DS_VK_CALL(device->vkDestroyQueryPool)(device->device, vkQueries->vkQueries,
		instance->allocCallbacksPtr);
	if (queries->allocator)
		DS_VERIFY(dsAllocator_free(queries->allocator, queries));
}
