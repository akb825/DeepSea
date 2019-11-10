/*
 * Copyright 2017-2019 Aaron Barany
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
#include <DeepSea/Render/Export.h>
#include <DeepSea/Render/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and using command buffer pools.
 *
 * Command buffer pools are used to allocate groups of command buffers when building up draw
 * commands in parallel to be submitted to the GPU later.
 *
 * When the command buffer pool is reset, all command buffers are cleared. The pool may be
 * double-buffered, which is useful when submitting command buffers to the GPU in parallel to
 * building up the next set of commands.
 *
 * @see dsCommandBufferPool
 */

/**
 * @brief Creates a command buffer pool.
 * @remark errno will be set on failure.
 * @param renderer The renderer to use with the command buffers.
 * @param allocator The allocator to create the render surface with. If NULL, it will use the same
 *     allocator as the renderer.
 * @param usage The usage flags for how to use the command buffers. This should be a combination of
 *     dsCommandBufferUsage flags, or 0 if none of the options are required.
 * @return The created command buffer pool, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsCommandBufferPool* dsCommandBufferPool_create(dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage);

/**
 * @brief Creates multiple command buffers in the pool.
 *
 * This will add new command buffers to the pool, incrementing the count and potentially changing
 * the commandBuffers pointer.
 *
 * @remark errno will be set on failure.
 * @param pool The pool to create the command buffer with.
 * @param count The number of command buffers to allocate.
 * @return An array pointing to the first command buffer. This will be the same as offsetting
 *     pool->commandBuffers by the count that was previously available. NULL will be returned on
 *     failure.
 */
DS_RENDER_EXPORT dsCommandBuffer** dsCommandBufferPool_createCommandBuffers(
	dsCommandBufferPool* pool, uint32_t count);

/**
 * @brief Resets a command buffer pool.
 *
 * This clears out all currently active command buffers and returns them to the pool, resetting the
 * count to 0.
 *
 * @remark errno will be set on failure.
 * @param pool The command buffer pool to reset.
 * @return False if the command buffer pool couldn't be reset.
 */
DS_RENDER_EXPORT bool dsCommandBufferPool_reset(dsCommandBufferPool* pool);

/**
 * @brief Destroys a command buffer pool.
 * @remark errno will be set on failure.
 * @param pool The command buffer pool to destroy.
 * @return False if the command buffer pool couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsCommandBufferPool_destroy(dsCommandBufferPool* pool);

#ifdef __cplusplus
}
#endif
