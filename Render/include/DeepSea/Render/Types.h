/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Render library.
 */

/**
 * @brief Log tag used by the render library.
 */
#define DS_RENDER_LOG_TAG "render"

/**
 * @brief Constant for no attachment.
 */
#define DS_NO_ATTACHMENT (uint32_t)-1

/**
 * @brief Enum for how an image attachment will be used.
 *
 * This enum is a bitmask to allow multiple combinations of the usage bits.
 */
typedef enum dsAttachmentUsage
{
	dsAttachmentUsage_Clear = 0x1,      ///< Clear the contents of the attachment before rendering.
	dsAttachmentUsage_KeepBefore = 0x2, ///< Keep the existing value before rendering begins.
	dsAttachmentUsage_KeepAfter = 0x4,  ///< Keep the value after rendering ends.
	/**
	 * Resolved multisampled attachment. Writes will be done to the multisample buffer, reads from
	 * the resolved buffer.
	 */
	dsAttachmentUsage_Resolve = 0x8
} dsAttachmentUsage;

/**
 * @brief Enum for the stage for a pipeline dependency.
 */
typedef enum dsSubpassDependencyStage
{
	dsSubpassDependencyStage_Vertex,  ///< Vertex operations, including tessellation and geometry.
	dsSubpassDependencyStage_Fragment ///< Fragment operations, including the final resolve.
} dsSubpassDependencyStage;

/**
 * @brief Enum for the type of a render surface.
 * @see RenderSurface.h
 */
typedef enum dsRenderSurfaceType
{
	dsRenderSurfaceType_Unknown, ///< Unknown surface type.
	dsRenderSurfaceType_Window,  ///< Window surface.
	dsRenderSurfaceType_PBuffer, ///< Pixel buffer surface.
	dsRenderSurfaceType_Pixmap   ///< Pixmap surface.
} dsRenderSurfaceType;

/**
 * @brief Enum for how to use a command buffer.
 *
 * This enum is a bitmask to allow multiple combinations of the usage bits.
 * @see CommandBufferPool.h
 */
typedef enum dsCommandBufferUsage
{
	dsCommandBufferUsage_Subpass = 0x1,        ///< Will only be used within a render subpass.
	dsCommandBufferUsage_MultipleSubmit = 0x2, ///< Will be submitted multiple times in a frame.
	dsCommandBufferUsage_MultipleFrames = 0x4, ///< Will be submitted across frames.
	/**
	 * Double-buffer the command buffers within the pool, allowing for writing to one set of buffers
	 * in parallel to another set being submitted.
	 */
	dsCommandBufferUsage_DoubleBuffer = 0x8
} dsCommandBufferUsage;

/**
 * @brief Struct for a pool of command buffers.
 *
 * Multiple command buffers may be used to queue draw commands in parallel before submitting them to
 * the GPU. The pool is double-buffered, allowing for
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsCommandBuffer and the true internal type.
 *
 * @see CommandBufferPool.h
 */
typedef struct dsCommandBufferPool
{
	/**
	 * @brief The renderer this is used with.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The current command buffers to use.
	 * @remark Even of the dsCommandBufferUsage_DoubleBuffer flag isn't set, this may still change
	 * between resets.
	 */
	dsCommandBuffer** currentBuffers;

	/**
	 * @brief The other set of command buffers when double-buffering is enabled.
	 *
	 * When resetting the pool, the currentBuffers and otherBuffers arrays will be swapped.
	 */
	dsCommandBuffer** otherBuffers;

	/**
	 * @brief The number of command buffers in the pool.
	 */
	uint32_t count;

	/**
	 * @brief The usage flags for the command buffers.
	 */
	dsCommandBufferUsage usage;
} dsCommandBufferPool;

/**
 * @brief Struct for a command buffer.
 *
 * This is used to queue render commands. It is used as a part of dsRenderPass in order to either
 * send render commands to the GPU or hold onto the commands for later execution.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsCommandBuffer and the true internal type.
 *
 * @see CommandBuffer.h
 */
typedef struct dsCommandBuffer
{
	/**
	 * @brief The renderer this is used with.
	 */
	dsRenderer* renderer;
} dsCommandBuffer;

/**
 * @brief Base object for interfacing with the DeepSea Render library.
 *
 * To ensure a lack of contention for system resources, only one dsRenderer instance should be used
 * in any given application.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderer and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @remark The virtual functions on the renderer should not be called directly. The public interface
 * functions handle error checking and statistic management, which could cause invalid values to be
 * reported when skipped.
 *
 * @see Renderer.h
 */
typedef struct dsRenderer dsRenderer;

/**
 * @brief Structure defining a render surface, such as a window.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderSurface and the true internal type.
 *
 * @see dsRenderSurface.h
 */
typedef struct dsRenderSurface
{
	/**
	 * The renderer this is used with.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the render surface.
	 */
	dsRenderSurfaceType surfaceType;

	/**
	 * @brief The width of the surface.
	 */
	uint32_t width;

	/**
	 * @brief The height of the render surface.
	 */
	uint32_t height;
} dsRenderSurface;

/**
 * @brief Struct for a render pass used by the renderer.
 *
 * This is used to draw a group of geometry together to a framebuffer. Render passes may either be
 * drawn to their own framebuffer or framebuffers may be shared to control draw order.
 *
 * A render pass contains one or more subpasses. Image attachment outputs from one subpass may be
 * accessed as inputs to other subpasses. When this is done, you can only access the same pixel's
 * value corresponding to the pixel being drawn. This is more efficient on some implementations
 * since it doesn't require the	full offscreen to be resolved while rendering the different portions
 * of the screen. One example where this is useful is for the various passes for deferred lighting.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderPass and the true internal type.
 */
typedef struct dsRenderPass dsRenderPass;

/**
 * @brief Structure describing the data for a draw call.
 */
typedef struct dsDrawData
{
	/**
	 * @brief The geometry to draw.
	 */
	dsDrawGeometry* geometry;

	/**
	 * @brief The material to apply to the shader.
	 */
	dsMaterial* material;

	/**
	 * @brief The index of the shader to draw with.
	 */
	uint32_t shaderIndex;

	/**
	 * @brief The world matrix to apply.
	 */
	dsMatrix44f worldMatrix;

	/**
	 * @brief The first index to draw.
	 */
	uint32_t startIndex;

	/**
	 * @brief The number of indices to draw.
	 */
	uint32_t indexCount;

	/**
	 * @brief The offset to apply to each index when looking up in the vertex buffer.
	 */
	int32_t vertexOffset;

	/**
	 * @brief The index of the first instance that's drawn.
	 */
	uint32_t firstInstance;

	/**
	 * @brief The number of instances to draw.
	 */
	uint32_t instanceCount;
} dsDrawData;

/**
 * @brief Structure defining a list of items to draw.
 */
typedef struct dsDrawList
{
	/**
	 * @brief The allocator to use for the underlying data.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The list of data to draw with.
	 */
	dsDrawData* drawData;

	/**
	 * @brief The number of active items in drawData.
	 */
	size_t size;

	/**
	 * @brief The maximum number of items that can be placed in drawData.
	 */
	size_t capacity;
} dsDrawList;

/**
 * @brief Structure describing how to draw with a shader.
 */
typedef struct dsShaderDrawInfo
{
	/**
	 * @brief The shader to draw with.
	 */
	dsShader* shader;

	/**
	 * @brief The type of primitives the shader will be drawn with.
	 */
	dsPrimitiveType primitiveType;

	/**
	 * @brief True to enable primitive restarts with strips and fans.
	 */
	bool primitiveRestart;
} dsShaderDrawInfo;

/**
 * @brief The info for an image attachment.
 *
 * This provides information ahead of time that can help improve performance during rendering.
 */
typedef struct dsAttachmentInfo
{
	/**
	 * @brief The usage of the attachment.
	 */
	dsAttachmentUsage usage;

	/**
	 * @brief The format of the attachment.
	 */
	dsGfxFormat format;

	/**
	 * @brief The number of samples for multisampling.
	 */
	uint16_t samples;
} dsAttachmentInfo;

/**
 * @brief Structure defining what is used for a subpass.
 */
typedef struct dsRenderSubpassInfo
{
	/**
	 * @brief The list of shaders to use with the subpass.
	 *
	 * This should not be changed after creation.
	 */
	const dsShaderDrawInfo* shaders;

	/**
	 * @brief List of image attachments to use as inputs as indices to the attachment list for the
	 * render pass.
	 *
	 * These can be read from the shader, though only the current pixel may be used.
	 */
	const uint32_t* inputAttachments;

	/**
	 * @brief List of image attachments to use as inputs as indices to the attachment list for the
	 * render pass.
	 */
	const uint32_t* colorAttachments;

	/**
	 * @brief The number of shaders.
	 */
	uint32_t shaderCount;

	/**
	 * @brief The number of input attachments.
	 */
	uint32_t inputAttachmentCount;

	/**
	 * @brief The number of color attachments.
	 */
	uint32_t colorAttachmentCount;

	/**
	 * @brief The depth stencil attachment as an index to the attachment list for the render pass.
	 *
	 * Set to DS_NO_ATTACHMENT to not have a depth attachment.
	 */
	uint32_t depthStencilAttachment;
} dsRenderSubpassInfo;

/**
 * @brief Structure defining an explicit subpass dependency.
 *
 * This ensures that the GPU is done with the specified stage from the source subpass before
 * processing the specified stage for the destination subpass.
 */
typedef struct dsSubpassDependency
{
	/**
	 * @brief The index of the source subpass.
	 */
	uint32_t srcSubpass;

	/**
	 * @brief The stage to wait after in the source subpass.
	 */
	dsSubpassDependencyStage srcStage;

	/**
	 * @brief The index of the destination subpass.
	 */
	uint32_t dstSubpass;

	/**
	 * @brief The stage to wait executing for in the destination subpass.
	 */
	dsSubpassDependencyStage dstStage;

	/**
	 * @brief True if the dependency is by region as opposed to the full surface.
	 */
	bool regionDependency;
} dsSubpassDependency;

/** @copydoc dsRenderPass */
struct dsRenderPass
{
	/**
	 * @brief The renderer this is used with.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The list of image attachments to use with the render pass.
	 */
	const dsAttachmentInfo* attachments;

	/**
	 * @brief The list of subpasses for this render pass.
	 */
	const dsRenderSubpassInfo* subpasses;

	/**
	 * @brief The list of subpass explicit subpass dependencies.
	 *
	 * If not specified, the default behavior is for each subpass' fragment stage to depend
	 * on the previous subpass' fragment stage.
	 */
	const dsSubpassDependency* subpassDependencies;

	/**
	 * @brief The number of attachments.
	 */
	uint32_t attachmentCount;

	/**
	 * @brief The number of subpasses.
	 */
	uint32_t subpassCount;

	/**
	 * @brief The number of subpass dependencies.
	 */
	uint32_t subpassDependeniesCount;
};

/**
 * @brief Struct containing a combined depth and stencil value.
 */
typedef struct dsDepthStencilValue
{
	/**
	 * @brief The depth value in the range [0, 1].
	 */
	float depth;

	/**
	 * @brief The stencil value.
	 */
	uint32_t stencil;
} dsDepthStencilValue;

/**
 * @brief Value used to clear a render surface when beginning a render pass.
 *
 * Which member of the union is used depends on the type of the surface.
 */
typedef union dsSurfaceClearValue
{
	/**
	 * @brief Color value for float and snorm surfaces.
	 */
	dsColor4f colorValue;

	/**
	 * @brief Color value for integer surfaces.
	 */
	int intValue[4];

	/**
	 * @brief Color value for unsigned integer surfaces.
	 */
	unsigned int uintValue[4];

	/**
	 * @brief Depth and stencil value for depth-stencil surfaces.
	 */
	dsDepthStencilValue depthStencil;
} dsSurfaceClearValue;

/**
 * @brief Function for creating a render surface.
 * @param renderer The renderer to use the render surface with.
 * @param allocator The allocator to create the render surface.
 * @param osHandle The OS handle, such as window handle.
 * @param type The type of the render surface.
 * @return The created render surface, or NULL if it couldn't be created.
 */
typedef dsRenderSurface* (*dsCreateRenderSurfaceFunction)(dsRenderer* renderer,
	dsAllocator* allocator, void* osHandle, dsRenderSurfaceType type);

/**
 * @brief Function for destroying a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param renderSurface The render surface to destroy
 * @return False if the render surface couldn't be destroyed.
 */
typedef bool (*dsDestroyRenderSurfaceFunction)(dsRenderer* renderer,
	dsRenderSurface* renderSurface);

/**
 * @brief Function for updating a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param renderSurface The render surface to update.
 * @return True if the render surface size changed.
 */
typedef bool (*dsUpdateRenderSurfaceFunction)(dsRenderer* renderer, dsRenderSurface* renderSurface);

/**
 * @brief Function to start drawing to a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderSurface The render surface to start drawing to.
 * @return False if this couldn't start drawing to the render surface.
 */
typedef bool (*dsBeginRenderSurfaceFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface);

/**
 * @brief Function to end drawing to a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderSurface The render surface to end drawing to.
 * @return False if this couldn't end drawing to the render surface.
 */
typedef bool (*dsEndRenderSurfaceFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface);

/**
 * @brief Function for swapping buffers for a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param renderSurface The render surface to swap the buffers on.
 * @return False if the buffers couldn't be swapped.
 */
typedef bool (*dsSwapRenderSurfaceBuffersFunction)(dsRenderer* renderer,
	dsRenderSurface* renderSurface);

/**
 * @brief Function for creating a command buffer pool.
 * @param renderer The renderer the command buffers will be used wtih.
 * @param allocator The allocator to create the command buffer pool.
 * @param usage The usage flags. Set to 0 if none of the usage options are needed.
 * @param count The number of command buffers to create in the pool.
 * @return The command buffer pool.
 */
typedef dsCommandBufferPool* (*dsCreateCommandBufferPoolFunction)(dsRenderer* renderer,
	dsAllocator* allocator, int usage, uint32_t count);

/**
 * @brief Function for destroying a command buffer pool.
 * @param renderer The renderer the command buffer pool was created with.
 * @param pool The command buffer pool to destroy.
 * @return False if the command buffer pool couldn't be destroyed.
 */
typedef bool (*dsDestroyCommandBufferPoolFunction)(dsRenderer* renderer, dsCommandBufferPool* pool);

/**
 * @brief Function for resetting a command buffer pool, preparing the command buffers to be built up
 *     with new render commands.
 *
 * It is the responsibility of the implementation to swap the command buffer arrays, since some
 * implementations have additional requirements and may need to tripple-buffer.
 *
 * @param renderer The renderer the command buffer pool was created with.
 * @param pool The command buffer pool to reset.
 * @return False if the command buffer pool couldn't be reset.
 */
typedef bool (*dsResetCommandBufferPoolFunction)(dsRenderer* renderer, dsCommandBufferPool* pool);

/**
 * @brief Function for beginning a frame.
 * @param renderer The renderer to draw with.
 * @return False if the frame couldn't be begun.
 */
typedef bool (*dsBeginFrameFunction)(dsRenderer* renderer);

/**
 * @brief Function for ending a frame.
 * @param renderer The renderer to draw with.
 * @return False if the frame couldn't be ended.
 */
typedef bool (*dsEndFrameFunction)(dsRenderer* renderer);

/**
 * @brief Function for starting to draw to a command buffer.
 * @param renderer The renderer that the command buffer will be drawn with.
 * @param commandBuffer The command buffer to begin.
 * @param renderPass The render pass the command buffer will be drawn with.
 * @param subpassIndex The subpass within the render pass that will be drawn with.
 * @param framebuffer The framebuffer that will be drawn to.
 * @return False if the command buffer couldn't be begun.
 */
typedef bool (*dsBeginCommandBufferFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex, const dsFramebuffer* framebuffer);

/**
 * @brief Function for ending drawing to a command buffer.
 * @param renderer The renderer that the command buffer will be drawn with.
 * @param commandBuffer The command buffer to end.
 * @return False if the command buffer couldn't be ended.
 */
typedef bool (*dsEndCommandBufferFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer);

/**
 * @brief Function for submitting a command buffer from one buffer to another.
 * @param renderer The renderer the command buffer will be drawn with.
 * @param commandBuffer The command buffer to submit the commands to.
 * @param submitBuffer The buffer to submit.
 * @return False if the command buffer couldn't be submitted.
 */
typedef bool (*dsSubmitCommandBufferFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsCommandBuffer* submitBuffer);

/**
 * @brief Function for creating a render pass.
 *
 * All arrays passed in and part of the structures should be copied by the implementation rather
 * than just copying the pointers.
 *
 * @param renderer The renderer to draw the render pass with.
 * @param allocator The allocator to create the render pass with.
 * @param attachments The attachments that will be used with the framebuffer.
 * @param attachmentCount The number of attachments.
 * @param subpasses The subpasses within the render pass.
 */
typedef dsRenderPass* (*dsCreateRenderPassFunction)(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount);

/**
 * @brief Function for destroying a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param renderPass The render pass to destroy.
 * @return False if the render pass couldn't be destroyed.
 */
typedef bool (*dsDestroyRenderPassFunction)(dsRenderer* renderer, dsRenderPass* renderPass);

/**
 * @brief Function for beginning a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to start.
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
 */
typedef bool (*dsBeginRenderPassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3d* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool indirectCommands);

/**
 * @brief Function for continuing to the next subpass within a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to continue.
 * @param indirectCommands True if the render commands for the subpass will be provided with command
 *     buffers, false if the render commands will be inlined.
 */
typedef bool (*dsNextRenderSubpassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, bool indirectCommands);

/**
 * @brief Function for ending a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to end.
 */
typedef bool (*dsEndRenderPassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass);

/** @copydoc dsRenderer */
struct dsRenderer
{
	/**
	 * @brief The main allocator for the Renderer library.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Manager for resources used with the renderer.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief Thread ID for the main thread.
	 *
	 * Some operations may only be done from the main thread.
	 */
	dsThreadId mainThread;

	/**
	 * @brief The main command buffer.
	 *
	 * This should only be used from the main thread. The pointer may change after calling
	 * dsRenderer_beginFrame() depending on the implementation.
	 */
	dsCommandBuffer* mainCommandBuffer;

	/**
	 * @brief The maximum anisitropy level for anisotropic texture filtering.
	 */
	float maxAnisotropy;

	/**
	 * @brief The format for color render surfaces.
	 */
	dsGfxFormat surfaceColorFormat;

	/**
	 * @brief The format for depth/stencil render surfaces.
	 *
	 * This can be set to dsGfxFormat_Unknown if a depth buffer isn't used.
	 */
	dsGfxFormat surfaceDepthStencilFormat;

	/**
	 * @brief The number of samples for multisampling in render surfaces.
	 */
	uint16_t surfaceSamples;

	/**
	 * @brief True if render surfaces are double-buffered.
	 */
	bool doubleBuffer;

	/**
	 * @brief True to wait for vsync when drawing to a render surface.
	 */
	bool vsync;

	/**
	 * @brief True if geometry shaders are supported.
	 */
	bool hasGeometryShaders;

	/**
	 * @brief True if tessellation shaders are supported.
	 */
	bool hasTessellationShaders;

	/**
	 * @brief True if compute shaders are supported.
	 */
	bool hasComputeShaders;

	/**
	 * @brief The default level of anisotropy for anisotropic filtering.
	 */
	float defaultAnisotropy;

	/**
	 * @brief The current frame number.
	 *
	 * This is incremented when calling dsRenderer_beginFrame().
	 */
	uint32_t frameNumber;

	/**
	 * @brief Render surface creation function.
	 */
	dsCreateRenderSurfaceFunction createRenderSurfaceFunc;

	/**
	 * @brief Render surface destruction function.
	 */
	dsDestroyRenderSurfaceFunction destroyRenderSurfaceFunc;

	/**
	 * @brief Render surface update function.
	 */
	dsUpdateRenderSurfaceFunction updateRenderSurfaceFunc;

	/**
	 * @brief Render surface begin function.
	 */
	dsBeginRenderSurfaceFunction beginRenderSurfaceFunc;

	/**
	 * @brief Render surface end function.
	 */
	dsBeginRenderSurfaceFunction endRenderSurfaceFunc;

	/**
	 * @brief Render surface buffer swap function.
	 */
	dsSwapRenderSurfaceBuffersFunction swapRenderSurfaceBuffersFunc;

	/**
	 * @brief Command buffer pool creation function.
	 */
	dsCreateCommandBufferPoolFunction createCommandBufferPoolFunc;

	/**
	 * @brief Command buffer pool destruction function.
	 */
	dsDestroyCommandBufferPoolFunction destroyCommandBufferPoolFunc;

	/**
	 * @brief Command buffer pool reset function.
	 */
	dsResetCommandBufferPoolFunction resetCommandBufferPoolFunc;

	/**
	 * @brief Frame begin function.
	 */
	dsBeginFrameFunction beginFrameFunc;

	/**
	 * @brief Frame end function.
	 */
	dsEndFrameFunction endFrameFunc;

	/**
	 * @brief Command buffer begin function.
	 */
	dsBeginCommandBufferFunction beginCommandBufferFunc;

	/**
	 * @brief Command buffer end function.
	 */
	dsEndCommandBufferFunction endCommandBufferFunc;

	/**
	 * @brief Command buffer submit function.
	 */
	dsSubmitCommandBufferFunction submitCommandBufferFunc;
};

#ifdef __cplusplus
}
#endif
