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

#include "Resources/MockGfxQueryPool.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsGfxQueryPool* dsMockGfxQueryPool_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGfxQueryPool* queries = DS_ALLOCATE_OBJECT(allocator, dsGfxQueryPool);
	if (!queries)
		return NULL;

	queries->resourceManager = resourceManager;
	queries->allocator = dsAllocator_keepPointer(allocator);
	queries->type = type;
	queries->count = count;
	return queries;
}

bool dsMockGfxQueryPool_reset(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(queries);
	DS_ASSERT(DS_IS_BUFFER_RANGE_VALID(first, count, queries->count));

	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(first);
	DS_UNUSED(count);
	return true;
}

bool dsMockGfxQueryPool_beginQuery(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(queries);
	DS_ASSERT(query < queries->count);

	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(query);
	return true;
}

bool dsMockGfxQueryPool_endQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(queries);
	DS_ASSERT(query < queries->count);

	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(query);
	return true;
}

bool dsMockGfxQueryPool_queryTimestamp(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(queries);
	DS_ASSERT(query < queries->count);

	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(query);

	return true;
}

bool dsMockGfxQueryPool_getValues(dsResourceManager* resourceManager,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count, void* data, size_t dataSize,
	size_t stride, size_t elementSize, bool checkAvailability)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(queries);
	DS_ASSERT(DS_IS_BUFFER_RANGE_VALID(first, count, queries->count));
	DS_ASSERT(data);
	DS_ASSERT(dataSize >= stride*count);

	DS_UNUSED(resourceManager);
	DS_UNUSED(queries);
	DS_UNUSED(first);
	DS_UNUSED(count);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	DS_UNUSED(stride);
	DS_UNUSED(elementSize);
	DS_UNUSED(checkAvailability);

	return true;
}

bool dsMockGfxQueryPool_copyValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t first, uint32_t count,
	dsGfxBuffer* buffer, size_t offset, size_t stride, size_t elementSize, bool checkAvailability)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(queries);
	DS_ASSERT(DS_IS_BUFFER_RANGE_VALID(first, count, queries->count));
	DS_ASSERT(buffer);
	DS_ASSERT(DS_IS_BUFFER_RANGE_VALID(offset, stride*count, buffer->size));

	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(queries);
	DS_UNUSED(first);
	DS_UNUSED(count);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(stride);
	DS_UNUSED(elementSize);
	DS_UNUSED(checkAvailability);

	return true;
}

bool dsMockGfxQueryPool_destroy(dsResourceManager* resourceManager,
	dsGfxQueryPool* queries)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(queries);

	DS_UNUSED(resourceManager);
	if (queries->allocator)
		return dsAllocator_free(queries->allocator, queries);
	return true;
}
