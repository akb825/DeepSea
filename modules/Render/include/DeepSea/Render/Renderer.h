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
 * @remark All functions that don't take a command buffer must be called on the main thread.
 * @see dsRenderer
 */

/**
 * @brief Makes an orthographic projection matrix.
 *
 * The matrix is generated assuming looking down the -Z axis. As a result, the near and far plane
 * distances are negated compared to world space.
 *
 * The coordinate space of the matrix will depend on the clipHalfDepth and clipInvertY parameters
 * stored in renderer.
 *
 * @remark errno will be set on failure.
 * @param[out] result The matrix for the result.
 * @param renderer The renderer to create the matrix for.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsRenderer_makeOrtho(dsMatrix44f* result, const dsRenderer* renderer,
	float left, float right, float bottom, float top, float near, float far);

/**
 * @brief Makes a projection matrix for a frustum.
 *
 * The coordinate space of the matrix will depend on the clipHalfDepth and clipInvertY parameters
 * stored in renderer.
 *
 * @remark errno will be set on failure.
 * @param[out] result The matrix for the result.
 * @param renderer The renderer to create the matrix for.
 * @param left The left plane.
 * @param right The right plane.
 * @param bottom The bottom plane.
 * @param top The top plane.
 * @param near The near plane.
 * @param far The far plane.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsRenderer_makeFrustum(dsMatrix44f* result, const dsRenderer* renderer,
	float left, float right, float bottom, float top, float near, float far);

/**
 * @brief Makes a perspective projection matrix.
 *
 * The coordinate space of the matrix will depend on the clipHalfDepth and clipInvertY parameters
 * stored in renderer.
 *
 * @remark errno will be set on failure.
 * @param[out] result The matrix for the result.
 * @param renderer The renderer to create the matrix for.
 * @param fovy The field of view in the Y direction in radians.
 * @param aspect The aspect ratio as X/Y.
 * @param near The near plane.
 * @param far The far plane.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsRenderer_makePerspective(dsMatrix44f* result, const dsRenderer* renderer,
	float fovy, float aspect, float near, float far);

/**
 * @brief Makes a frustum from a projection matrix.
 *
 * This will take the coordinate space of projection matricies for the renderer into account.
 *
 * @remark errno will be set on failure.
 * @remark The planes aren't guaranteed to be normalized.
 * @param renderer The renderer to create the frustum for.
 * @param[out] result The resulting frustum.
 * @param matrix The projection matrix to extract the frustum planes from.
 * @return False if the parameters are invalid.
 */
DS_RENDER_EXPORT bool dsRenderer_frustumFromMatrix(dsFrustum3f* result, const dsRenderer* renderer,
	const dsMatrix44f* matrix);

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
DS_RENDER_EXPORT bool dsRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples);

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
 * @remark This must be called outside of a render pass.
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
 * @remark This must be called outside of a render pass.
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
 * @remark This must be called inside of a render pass.
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
 * @remark This must be called inside of a render pass.
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
 * @remark This must be called inside of a render pass.
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
 * @remark This must be called inside of a render pass.
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
 * @remark This must be called outside of a render pass.
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
 * @remark This must be called outside of a render pass.
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
 * @brief Blits from one surface to another, scaling when necessary.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to process the blit on.
 * @param srcSurfaceType The type of the source surface.
 * @param srcSurface The surface to blit from.
 * @param dstSurfaceType The type of the source surface.
 * @param dstSurface The surface to blit from.
 * @param regions The regions to blit.
 * @param regionCount The number of regions to blit.
 * @param filter The filter to use when scaling is required.
 * @return False if the data couldn't be blitted.
 */
DS_RENDER_EXPORT bool dsRenderer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter);

/**
 * @brief Waits until the GPU is idle.
 *
 * Waiting until idle is useful when destroying large numbers of graphics resources (e.g. unloading
 * a scene) since it allows the implementation to destroy those resources immediately rather than
 * waiting for the current frame to complete. This can reduce spikes of memory usage when a new set
 * of resources is loaded while waiting to destroy the old set.
 *
 * @param renderer The renderer.
 * @return False if the renderer couldn't be waited on.
 */
DS_RENDER_EXPORT bool dsRenderer_waitUntilIdle(dsRenderer* renderer);

/**
 * @brief Restores the expected global state of the renderer.
 *
 * Call this if an external library may modify the external state from what was expected. For
 * example, under OpenGL this will bind the expected context.
 *
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @return False if the state couldn't be restored.
 */
DS_RENDER_EXPORT bool dsRenderer_restoreGlobalState(dsRenderer* renderer);

/**
 * @brief Initializes the members of a renderer.
 *
 * This will initiialize all members to 0 and set up any internal structures. This is called by the
 * render implementation.
 *
 * @remark errno will be set on failure.
 * @param renderer The renderer.
 * @return False if an error occurred.
 */
DS_RENDER_EXPORT bool dsRenderer_initialize(dsRenderer* renderer);

/**
 * @brief Destroys the private members of a renderer.
 *
 * This is called by the render implementation.
 *
 * @param renderer The renderer.
 */
DS_RENDER_EXPORT void dsRenderer_shutdown(dsRenderer* renderer);

#ifdef __cplusplus
}
#endif
