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
 * @brief Functions for using render passes.
 *
 * Render passes are used to draw a group of geometry together to a framebuffer. Render passes may
 * either be drawn to their own framebuffer or framebuffers may be shared to control draw order.
 *
 * A render pass contains one or more subpasses. Image attachment outputs from one subpass may be
 * accessed as inputs to other subpasses. When this is done, you can only access the same pixel's
 * value corresponding to the pixel being drawn. This is more efficient on some implementations
 * since it doesn't require the	full offscreen to be resolved while rendering the different portions
 * of the screen. One example where this is useful is for the various passes for deferred lighting.
 *
 * Render passes may only be created and destroyed on the main thread, though they may be used on
 * other threads with different command buffers.
 *
 * @see dsRenderPass
 */

/**
 * @brief Creates a render pass.
 * @remark errno will be set on failure.
 * @param renderer The renderer to draw the render pass with.
 * @param allocator The allocator to create the render pass with. If NULL, it will use the same
 *     allocator as the renderer.
 * @param attachments The attachments that will be used with the framebuffer. This array will be
 *     copied.
 * @param attachmentCount The number of attachments.
 * @param subpasses The subpasses within the render pass. At least one subpass must be provided.
 *     This array will be copied.
 * @param subpassCount The number of subpasses.
 * @param dependencies The dependencies between subpasses. If the implementation keeps the
 *     dependencies, this array will be copied.
 * @param dependencyCount The number of dependencies. This may be set to
 *     DS_DEFAULT_SUBPASS_DEPENDENCIES, in which case each subpass' fragment stage depends on the
 *     previous subpass' fragment stage, with the first subpass having an external dependency.
 * @return The created render pass, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsRenderPass* dsRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount);

/**
 * @brief Begins drawing a render pass.
 * @remark errno will be set on failure.
 * @param renderPass The render pass to begin.
 * @param commandBuffer The command buffer to push the commands on.
 * @param framebuffer The framebuffer to draw the render pass to.
 * @param viewport The viewport to draw to. The x/y values are in pixel space, while the z value is
 *     in the range [0, 1]. If NULL, the full range is used.
 * @param clearValues The values to clear the framebuffer with. Only values corresponding to
 *     attachments with the clear bit set are considered. This may be NULL if no attachments will be
 *     cleared.
 * @param clearValueCount The number of clear values. This must either be 0 if clearValues is NULL
 *     or equal to the number of attachments.
 * @param indirectCommands True if the render commands for the first subpass will be provided with
 *     command buffers, false if the render commands will be inlined.
 * @return False if the render pass couldn't be begun.
 */
DS_RENDER_EXPORT bool dsRenderPass_begin(const dsRenderPass* renderPass,
	dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool indirectCommands);

/**
 * @brief Advances to the next subpass in a render pass.
 * @remark errno will be set on failure.
 * @param renderPass The render pass to continue.
 * @param commandBuffer The command buffer to push the commands on.
 * @param indirectCommands True if the render commands for the subpass will be provided with command
 *     buffers, false if the render commands will be inlined.
 * @return False if the render pass couldn't be advanced.
 */
DS_RENDER_EXPORT bool dsRenderPass_nextSubpass(const dsRenderPass* renderPass,
	dsCommandBuffer* commandBuffer, bool indirectCommands);

/**
 * @brief Ends drawing to a render pass.
 * @remark errno will be set on failure.
 * @param renderPass The render pass to end.
 * @param commandBuffer The command buffer to push the commands on.
 * @return False if the render pass couldn't be ended.
 */
DS_RENDER_EXPORT bool dsRenderPass_end(const dsRenderPass* renderPass,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Destroys .
 * @remark errno will be set on failure.
 * @param renderPass The render pass to destroy.
 * @return False if the render pass couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsRenderPass_destroy(dsRenderPass* renderPass);

#ifdef __cplusplus
}
#endif
