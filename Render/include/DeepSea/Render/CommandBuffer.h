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
 * @brief Functions for using command buffers.
 *
 * These functions are used for drawing command buffers created through dsCommandBufferPool.
 *
 * @see dsCommandBuffer
 */

/**
 * @brief Begins drawing to a command buffer.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to begin. This should not be the main command buffer.
 * @param renderPass The render pass the command buffer will be drawn with. This should be provided
 *     if the command buffer was created with the dsCommandBufferUsage_Subpass flag to improve
 *     performance on some implementations.
 * @param subpassIndex The subpass within the render pass that will be drawn with.
 * @param framebuffer The framebuffer that will be drawn to. This may be NULL if the command buffer
 *     will be re-played to draw to multiple framebuffers, but can improve performance if provided.
 * @return False if the command buffer couldn't be begun.
 */
DS_RENDER_EXPORT bool dsCommandBuffer_begin(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex, const dsFramebuffer* framebuffer);

/**
 * @brief Ends drawing to a command buffer.
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
DS_RENDER_EXPORT bool dsCommandBuffer_submit(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

#ifdef __cplusplus
}
#endif
