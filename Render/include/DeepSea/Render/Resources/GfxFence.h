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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for using fences.
 *
 * Fences can be used to wait on the CPU until a task on the GPU is completed. A common usage for
 * fences is to protect against modifying ranges of a persistently mapped buffer before it is
 * finished drawing.
 *
 * Unless a command buffer is passed as an argument, all functions must either be called on the main
 * thread or on a thread with an active resource context. A resource shouldn't be accessed
 * simultaneously across multiple threads.
 *
 * @see dsGfxFence
 */

/**
 * @brief Creates a fence.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the fence from.
 * @param allocator The allocator to create the fence with. If NULL, it will use the same allocator
 *     as the resource manager.
 * @return The created fence, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsGfxFence* dsGfxFence_create(dsResourceManager* resourceManager,
	dsAllocator* allocator);

/**
 * @brief Sets a fence on the command buffer.
 *
 * Fences have a limited granularity for when they can be set on the GPU:
 * - A fence may be set at any point on the main command buffer when outside of a render pass.
 * - When set during a render pass on the main command buffer, the fence will be set at the end of
 *   the render pass.
 * - When set in a command buffer other than the main one, the fence will be set on the GPU when
 *   submitted to the main command buffer. It will be set once all commands during that submit have
 *   completed.
 *
 * A fence may not be set on a command buffer that can be submitted multiple times. Once set, a
 * fence must be reset before being set again.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to queue the fence on.
 * @param fence The fence.
 * @param bufferReadback True if persistently mapped buffers will be read back.
 * @return False if the fence couldn't be set.
 */
DS_RENDER_EXPORT bool dsGfxFence_set(dsCommandBuffer* commandBuffer, dsGfxFence* fence,
	bool bufferReadback);

/**
 * @brief Sets multiple fences on the command buffer.
 *
 * These fences will be set together based on the rules in dsGfxFence_set(). This will be more
 * efficient than setting each fence individually.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to queue the fence on.
 * @param fences The fences to set.
 * @param fenceCount The number of fences.
 * @param bufferReadback True if persistently mapped buffers will be read back.
 * @return False if the fence couldn't be set.
 */
DS_RENDER_EXPORT bool dsGfxFence_setMultiple(dsCommandBuffer* commandBuffer, dsGfxFence** fencees,
	uint32_t fenceCount, bool bufferReadback);

/**
 * @brief Waits for a fence to complete.
 * @remark errno will be set on failure.
 * @param fence The fence.
 * @param timeout The number of nanoseconds to wait for the fence.
 * @return The result of the wait.
 */
DS_RENDER_EXPORT dsGfxFenceResult dsGfxFence_wait(dsGfxFence* fence, uint64_t timeout);

/**
 * @brief Resets a fence so it may be set again.
 * @param fence The fence.
 * @return False if the fence couldn't be reset.
 */
DS_RENDER_EXPORT bool dsGfxFence_reset(dsGfxFence* fence);

/**
 * @brief Destroys a fence.
 * @remark errno will be set on failure.
 * @param fence The fence to destroy.
 * @return False if the fence couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsGfxFence_destroy(dsGfxFence* fence);

#ifdef __cplusplus
}
#endif
