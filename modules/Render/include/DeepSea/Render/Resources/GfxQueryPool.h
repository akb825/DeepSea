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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Functions for creating and manipulating query objects.
 *
 * Query objects may be used to get information from the GPU to either use from the CPU or for other
 * GPU operations.
 */

/**
 * @brief Creates a query pool.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the query pool from.
 * @param allocator The allocator to create the query pool with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param type The type of queries used by the pool.
 * @return The created query pool, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsGfxQueryPool* dsGfxQueryPool_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count);

/**
 * @brief Resets queries to an unset state.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @return False if the queries couldn't be reset.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_reset(dsGfxQueryPool* queries, dsCommandBuffer* commandBuffer,
	uint32_t first, uint32_t count);

/**
 * @brief Begins a query.
 *
 * This is valid for all query types except timestamps.
 *
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param query The index of the query to begin.
 * @return False if the query couldn't begin.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_beginQuery(dsGfxQueryPool* queries,
	dsCommandBuffer* commandBuffer, uint32_t query);

/**
 * @brief Ends a query that was previously begun.
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param query The index of the query to end.
 * @return False if the query couldn't end.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_endQuery(dsGfxQueryPool* queries,
	dsCommandBuffer* commandBuffer, uint32_t query);

/**
 * @brief Querying the current GPU timestamp.
 *
 * This is only valid for timestamp queries.
 *
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param query The index of the query to set the timestamp for.
 * @return False if the timestamp couldn't be set.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_queryTimestamp(dsGfxQueryPool* queries,
	dsCommandBuffer* commandBuffer, uint32_t query);

/**
 * @brief Gets the current query values.
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @param data The data to write the results into.
 * @param dataSize The size of the data buffer.
 * @param stride The stride of the data.
 * @param elementSize The size of each element. This must be sizeof(uint32_t) or sizeof(uint64_t).
 * @param checkAvailability True to check availability and avoid waiting. When true, two values are
 *     set for each query: the value (if available) and a 0 or 1 value for the value being
 *     available.
 * @return False if the values couldn't be queried.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_getValues(dsGfxQueryPool* queries, uint32_t first,
	uint32_t count, void* data, size_t dataSize, size_t stride, size_t elementSize,
	bool checkAvailability);

/**
 * @brief Copies the current query values to a GPU buffer.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param queries The query pool.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @param buffer The graphic buffer to write the results into. This must have been created with the
 *     dsGfxBufferUsage_CopyTo flag.
 * @param offset The offset into the buffer for the first element.
 * @param stride The stride of the data.
 * @param elementSize The size of each element.
 * @param checkAvailability True to check availability and avoid waiting. When true, two values are
 *     set for each query: the value (if available) and a 0 or 1 value for the value being
 *     available.
 * @return False if the values couldn't be queried.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_copyValues(dsGfxQueryPool* queries,
	dsCommandBuffer* commandBuffer, uint32_t first, uint32_t count, dsGfxBuffer* buffer,
	size_t offset, size_t stride, size_t elementSize, bool checkAvailability);

/**
 * @brief Destroys a query pool.
 * @param queries The query pool.
 * @return False if the query pool couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsGfxQueryPool_destroy(dsGfxQueryPool* queries);

#ifdef __cplusplus
}
#endif
