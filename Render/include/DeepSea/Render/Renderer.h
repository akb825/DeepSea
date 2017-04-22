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
 * @brief Functions for working with a renderer.
 *
 * This contains functions for general renderer management as well as functions for drawing and
 * dispatching compute shaders.
 *
 * Unlike most types, this doesn't contain any functions for creating or destroying a renderer.
 * This is handled by the renderer implementation libraries.
 *
 * @see dsRenderer
 */

/**
 * @brief Begins a frame.
 *
 * This must be called on the main thread before any render commands get executed.
 *
 * @remark errno will be set on failure.
 * @param renderer The renderer to draw with.
 * @return False if the frame couldn't be begun.
 */
DS_RENDER_EXPORT bool dsRenderer_beginFrame(dsRenderer* renderer);

/**
 * @brief Ends the current frame.
 *
 * This must be called on the main thread before any render commands get executed.
 *
 * @remark errno will be set on failure.
 * @param renderer The renderer to draw with.
 * @return False if the frame couldn't be ended.
 */
DS_RENDER_EXPORT bool dsRenderer_endFrame(dsRenderer* renderer);

/**
 * @brief Sets the number of anti-alias samples for the default render surfaces.
 *
 * This value will be used when the DS_DEFAULT_ANTIALIAS_SAMPLES constant is used. It is the
 * responsibility of the caller to re-create any render surfaces, offscreens, renderbuffers, and
 * framebuffers to respect this change.
 *
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param samples The number of anti-alias samples.
 * @return False if the number of samples couldn't be set.
 */
DS_RENDER_EXPORT bool dsRenderer_setSurfaceSamples(dsRenderer* renderer, uint16_t samples);

/**
 * @brief Sets whether or not to wait for vsync.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param vsync True to wait for vsync.
 * @return False if the vsync couldn't be set.
 */
DS_RENDER_EXPORT bool dsRenderer_setVsync(dsRenderer* renderer, bool vsync);

/**
 * @brief Sets the default anisotropy for anisotropic filtering.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param anisotropy The default anisotropy.
 * @return False if the anisotropy couldn't be set.
 */
DS_RENDER_EXPORT bool dsRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy);

/**
 * @brief Clears a color surface.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the clear command on.
 * @param surface The surface to clear.
 * @param colorValue The color value to clear with.
 * @return False if the surface couldn't be cleared.
 */
DS_RENDER_EXPORT bool dsRenderer_clearColorSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	const dsSurfaceColorValue* colorValue);

/**
 * @brief Clears a depth-stencil surface.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the clear command on.
 * @param surface The surface to clear.
 * @param surfaceParts The parts of the surface to clear.
 * @param depthStencilValue The depth-stencil value to clear with.
 * @return False if the surface couldn't be cleared.
 */
DS_RENDER_EXPORT bool dsRenderer_clearDepthStencilSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue);

/**
 * @brief Draws vertex geometry with the currently bound shader.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @return False if the geometry couldn't be drawn.
 */
DS_RENDER_EXPORT bool dsRenderer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange);

/**
 * @brief Draws indexed geometry with the currently bound shader.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @return False if the geometry couldn't be drawn.
 */
DS_RENDER_EXPORT bool dsRenderer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange);

/**
 * @brief Indirectly draws vertex geometry with the currently bound shader.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param indirectBuffer The buffer containing the draw information. The contents should be the same
 *     layout as dsDrawRange.
 * @param offset The offset into the buffer.
 * @param count The number of draw calls.
 * @param stride The stride for each element in the indirect buffer.
 * @return False if the geometry couldn't be drawn.
 */
DS_RENDER_EXPORT bool dsRenderer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride);

/**
 * @brief Indirectly draws indexed geometry with the currently bound shader.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param indirectBuffer The buffer containing the draw information. The contents should be the same
 *     layout as dsDrawRange.
 * @param offset The offset into the buffer.
 * @param count The number of draw calls.
 * @param stride The stride for each element in the indirect buffer.
 * @return False if the geometry couldn't be drawn.
 */
DS_RENDER_EXPORT bool dsRenderer_drawIndexedIndirect(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsGfxBuffer* indirectBuffer, size_t offset, uint32_t count, uint32_t stride);

/**
 * @brief Dispatches a compute job.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the dispatch command on.
 * @param x The number of working groups in the X direction.
 * @param y The number of working groups in the Y direction.
 * @param z The number of working groups in the Z direction.
 * @return False if the compute job couldn't be dispatched.
 */
DS_RENDER_EXPORT bool dsRenderer_dispatchCompute(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, uint32_t x, uint32_t y, uint32_t z);

/**
 * @brief Dispatches an indirect compute job.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the dispatch command on.
 * @param indirectBuffer The buffer that contains the number of working groups in the X, Y, and Z
 *     dimensions as 4-byte unsigned integers.
 * @param offset The offset into the indirect buffer.
 * @return False if the compute job couldn't be dispatched.
 */
DS_RENDER_EXPORT bool dsRenderer_dispatchComputeIndirect(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsGfxBuffer* indirectBuffer, size_t offset);

/**
 * @brief Initializes the members of a renderer.
 *
 * This will initiialize all members to 0 and set up any internal structures. This is called by the
 * render implementation.
 *
 * @remark errno will be set on failure.
 * @return False if an error occurred.
 */
DS_RENDER_EXPORT bool dsRenderer_initialize(dsRenderer* renderer);

/**
 * @brief Destroys the private members of a renderer.
 *
 * This is called by the render implementation.
 */
DS_RENDER_EXPORT void dsRenderer_shutdown(dsRenderer* renderer);

#ifdef __cplusplus
}
#endif
