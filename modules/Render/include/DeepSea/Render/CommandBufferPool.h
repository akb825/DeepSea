/*
 * Copyright 2017 Aaron Barany
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
 * @remark All command buffer pool operations must be performed on the main thread, though the
 * command buffers themselves may be used on separate threads.
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
 * @param count The number of command buffers to use.
 * @return The created command buffer pool, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsCommandBufferPool* dsCommandBufferPool_create(dsRenderer* renderer,
	dsAllocator* allocator, unsigned int usage, uint32_t count);

/**
 * @brief Resets a command buffer pool.
 *
 * This will prepare the command buffers to be rendered to again, clearing the contents and
 * potentially swapping the array of command buffers.
 *
 * @remark The command buffer array may be swapped in some implementations even if double buffering
 * isn't requested. This is done on implementations that asynchronously execute the contents within
 * the driver.
 *
 * @remark errno will be set on failure.
 * @param pool The command buffer pool to reset.
 * @return False if the command buffer pool couldn't be reset.
 *
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
