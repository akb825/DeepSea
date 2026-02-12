/*
 * Copyright 2017-2026 Aaron Barany
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
 * @brief Functions for using command buffers.
 *
 * These functions are used for drawing command buffers created through dsCommandBufferPool.
 *
 * @see dsCommandBuffer
 */

/**
 * @brief Begins processing commands on a command buffer.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to begin. This should not be the main command buffer.
 * @return False if the command buffer couldn't be begun.
 */
DS_RENDER_EXPORT bool dsCommandBuffer_begin(dsCommandBuffer* commandBuffer);

/**
 * @brief Begins drawing to a secondary command buffer.
 *
 * A secondary command buffer is for the draw calls (including shader binds) within a render pass.
 * As a result, the framebuffer and render subpass being drawn to is required when beginning the
 * command buffer, which must match the bound state for the command buffer being submitted to with
 * dsCommandBuffer_submit().
 *
 * @remark errno will be set on failure.
 * @remark either framebuffer or viewport must be non-NULL to know the full view range to render to.
 * @param commandBuffer The command buffer to begin. This must be from a dsCommandBufferPool created
 *     with the dsCommandBufferUsage_Secondary flag.
 * @param framebuffer The framebuffer being drawn to. This may be NULL, but can be faster if
 *     specified. If this is NULL, then viewport must be non-NULL.
 * @param renderPass The render pass being drawn to.
 * @param subpass The subpass within the render pass being drawn to.
 * @param viewport The viewport to draw to. The x/y values are in pixel space, while the z value is
 *     in the range [0, 1]. If NULL, the full range is used. This must match the viewport used to
 *     begin the render pass on the command buffer it will be submitted to. If this is NULL, then
 *     framebuffer must be non-NULL.
 * @param parentOcclusionQueryState The expected state of the occlusion query for the primary command
 *     buffer this will be submitted to. It will inherit the occlusion query state of the primary
 *     command buffer up to the precision level by this state. However, having a state with
 *     occlusion queries enabled when they will not be used may have a performance penalty on some
 *     platforms.
 * @return False if the command buffer couldn't be begun.
 */
DS_RENDER_EXPORT bool dsCommandBuffer_beginSecondary(dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport, dsGfxOcclusionQueryState parentOcclusionQueryState);

/**
 * @brief Ends submitting commands to a command buffer.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to end. This should not be the main command buffer.
 * @return False if the command buffer couldn't be ended.
 */
DS_RENDER_EXPORT bool dsCommandBuffer_end(dsCommandBuffer* commandBuffer);

/**
 * @brief Submits one command buffer to be executed on another command buffer.
 * @param commandBuffer The command buffer to execute the commands of submitBuffer on.
 * @param submitBuffer The command buffer to submit. This may not be the main command buffer.
 * @return False if the command buffer couldn't be submitted.
 */
DS_RENDER_EXPORT bool dsCommandBuffer_submit(
	dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer);

#ifdef __cplusplus
}
#endif
