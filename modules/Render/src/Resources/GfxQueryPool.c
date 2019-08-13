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

#include <DeepSea/Render/Resources/GfxQueryPool.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

dsGfxQueryPool* dsGfxQueryPool_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxQueryType type, uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) || count == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceManager->hasQueries || !resourceManager->createQueryPoolFunc ||
		!resourceManager->destroyQueryPoolFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support queries.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	switch (type)
	{
		case dsGfxQueryType_SamplesPassed:
			if (!resourceManager->hasPreciseOcclusionQueries)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Current target doesn't support precise occlusion queries.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
			break;
		case dsGfxQueryType_AnySamplesPassed:
			break;
		case dsGfxQueryType_Timestamp:
			if (resourceManager->timestampPeriod <= 0.0f)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Current target doesn't support timestamp queries.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
			break;
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid query type.");
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxQueryPool* queries = resourceManager->createQueryPoolFunc(resourceManager, allocator, type,
		count);
	if (queries)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->queryPoolCount, 1);
	}
	DS_PROFILE_FUNC_RETURN(queries);
}

bool dsGfxQueryPool_reset(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t first, uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !queries || !queries->resourceManager ||
		!queries->resourceManager->resetQueryPoolFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(first, count, queries->count))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to reset queries out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Resetting query pools must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	bool success = resourceManager->resetQueryPoolFunc(resourceManager, commandBuffer, queries,
		first, count);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_beginQuery(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t query)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !queries || !queries->resourceManager ||
		!queries->resourceManager->beginQueryFunc || !queries->resourceManager->endQueryFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (queries->type == dsGfxQueryType_Timestamp)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot begin a timestamp query.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (query >= queries->count)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to begin a query out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Beginning a query must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	bool success = resourceManager->beginQueryFunc(resourceManager, commandBuffer, queries, query);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_endQuery(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t query)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !queries || !queries->resourceManager ||
		!queries->resourceManager->endQueryFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (queries->type == dsGfxQueryType_Timestamp)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot end a timestamp query.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (query >= queries->count)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to end a query out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Ending a query must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	bool success = resourceManager->endQueryFunc(resourceManager, commandBuffer, queries,
		query);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_queryTimestamp(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t query)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !queries || !queries->resourceManager ||
		!queries->resourceManager->queryTimestampFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (queries->type != dsGfxQueryType_Timestamp)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Must use a timestamp query when getting the timestamp.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (query >= queries->count)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to get the timestamp with a query out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Querying a timestamp must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	bool success = resourceManager->queryTimestampFunc(resourceManager, commandBuffer, queries,
		query);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_getValues(dsGfxQueryPool* queries, uint32_t first, uint32_t count, void* data,
	size_t dataSize, size_t stride, size_t elementSize, bool checkAvailability)
{
	DS_PROFILE_FUNC_START();

	if (!queries || !queries->resourceManager || !queries->resourceManager->getQueryValuesFunc ||
		!data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(first, count, queries->count))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to get query values out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	if (elementSize != sizeof(uint32_t) &&
		(elementSize != sizeof(uint64_t) || !resourceManager->has64BitQueries))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid query element size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((size_t)data % elementSize != 0 || stride % elementSize != 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Query data not properly aligned.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t minSize = elementSize*(checkAvailability ? 2 : 1);
	if (stride < minSize)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Stride is less than the minimum size for each query element.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t minDataSize = count == 0 ? 0 : stride*(count - 1) + minSize;
	if (dataSize < minDataSize)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Data buffer is too small for query values.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (queries->count == 0)
		DS_PROFILE_FUNC_RETURN(true);

	bool success = resourceManager->getQueryValuesFunc(resourceManager, queries, first, count, data,
		dataSize, stride, elementSize, checkAvailability);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_copyValues(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !queries || !queries->resourceManager ||
		!queries->resourceManager->copyQueryValuesFunc || !buffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(first, count, queries->count))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to get query values out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(buffer->usage & dsGfxBufferUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a buffer without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	if (elementSize != sizeof(uint32_t) &&
		(elementSize != sizeof(uint64_t) || !resourceManager->has64BitQueries))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid query element size.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (offset % elementSize != 0 || stride % elementSize != 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Query data not properly aligned.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t minSize = elementSize*(checkAvailability ? 2 : 1);
	if (stride < minSize)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Stride is less than the minimum size for each query element.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t minDataSize = count == 0 ? 0 : stride*(count - 1) + minSize;
	if (!DS_IS_BUFFER_RANGE_VALID(offset, minDataSize, buffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Query copy out of buffer range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!resourceManager->hasQueryBuffers)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Copying query values to buffers isn't supported.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Copying query values must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Copying query values must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (queries->count == 0)
		DS_PROFILE_FUNC_RETURN(true);

	bool success = resourceManager->copyQueryValuesFunc(resourceManager, commandBuffer, queries,
		first, count, buffer, offset, stride, elementSize, checkAvailability);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxQueryPool_destroy(dsGfxQueryPool* queries)
{
	if (!queries)
		return true;

	DS_PROFILE_FUNC_START();

	if (!queries->resourceManager || !queries->resourceManager->destroyQueryPoolFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = queries->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyQueryPoolFunc(resourceManager, queries);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->queryPoolCount, -1);
	}
	DS_PROFILE_FUNC_RETURN(success);
}
