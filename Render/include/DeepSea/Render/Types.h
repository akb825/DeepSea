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
 * @brief Enum for the type of primitive to draw with.
 */
typedef enum dsPrimitiveType
{
	dsPrimitiveType_PointList,              ///< A list of points.
	dsPrimitiveType_LineList,               ///< A list of lines.
	dsPrimitiveType_LineStrip,              ///< A strip of connected lines.
	dsPrimitiveType_TriangleList,           ///< A list of triangles.
	dsPrimitiveType_TriangleStrip,          ///< A strip of connected triangles.
	dsPrimitiveType_TriangleFan,            ///< A fan of connected triangles.
	dsPrimitiveType_LineListAdjacency,      ///< A list of lines with adjacency info.
	dsPrimitiveType_TriangleListAdjacency,  ///< A list of triangles with adjacency info.
	dsPrimitiveType_TriangleStripAdjacency, ///< A strip of connected triangles with adjacency info.
	dsPrimitiveType_PatchList,              ///< A list of tessellation control patches.
} dsPrimitiveType;

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
 * @brief Struct for a command buffer.
 *
 * This is used to queue render commands. It is used as a part of dsRenderPass in order to either
 * send render commands to the GPU or hold onto the commands for later execution.
 *
 * This is an opaque type that is defined by the implementation.
 */
typedef struct dsCommandBuffer dsCommandBuffer;

/**
 * @brief Base object for interfacing with the DeepSea Render library.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderer and the true internal type.
 *
 * To ensure a lack of contention for system resources, only one dsRenderer instance should be used
 * in any given application.
 */
typedef struct dsRenderer dsRenderer;

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
 * casted between dsRenderer and the true internal type.
 */
typedef struct dsRenderPass dsRenderPass;

/**
 * @brief Structure describing a viewport.
 *
 * This includes the matrices used and the bounds of the framebuffer to draw to.
 */
typedef struct dsViewport
{
	/**
	 * @brief The view matrix.
	 */
	dsMatrix44f viewMatrix;

	/**
	 * @brief The projection matrix.
	 */
	dsMatrix44f projectionMatrix;

	/**
	 * @brief The view bounds.
	 *
	 * The origin is in the upper-left corner and the values should be in the range [0, 1].
	 */
	dsAlignedBox2f viewBounds;
} dsViewport;

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
	 * @brief The number of anti-alias samples.
	 */
	unsigned int samples;
} dsAttachmentInfo;

/**
 * @brief Structure defining what is used for a subpass.
 */
typedef struct dsRenderSubpassInfo
{
	/**
	 * @brief List of image attachments to use as inputs as indices to the attachment list for the
	 * render pass.
	 *
	 * These can be read from the shader, though only the current pixel may be used.
	 */
	const uint32_t* inputAttachments;

	/**
	 * @brief The number of input attachments.
	 */
	uint32_t inputAttachmentCount;

	/**
	 * @brief List of image attachments to use as inputs as indices to the attachment list for the
	 * render pass.
	 */
	const uint32_t* colorAttachments;

	/**
	 * @brief The number of color attachments.
	 */
	uint32_t colorAttachmentCount;

	/**
	 * @brief The depth stencil attachment as an index to the attachment list for the render pass.
	 *
	 * Set to DS_UNKNOWN to not have a depth attachment.
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
} dsSubpassDependency;

/**
 * @brief Function for drawing a render pass.
 * @param[out] outDrawData The draw data to use to draw with.
 * @param[out] outDrawCount The number of draw data elements.
 * @param renderPass The render pass that will be drawn.
 * @param subpassIndex The index of the current subpass.
 * @param commandBuffer The command buffer that will be drawn to. This can be used to for resource
 *     copy operations.
 * @param scratchDrawList A scratch list of items to draw. Usage of this is optional, but may be
 *     be used to avoid maintaining multiple allocations of draw lists across draws.
 */
typedef void (*dsRenderPassDrawFunction)(dsDrawData** outDrawData, size_t* outDrawCount,
	dsRenderPass* renderPass, uint32_t subpassIndex, dsCommandBuffer* commandBuffer,
	dsDrawList* scratchDrawList);

/**
 * @brief Function for operations after drawing the render pass.
 * @param renderPass The render pass that was drawn.
 * @param commandBuffer The command buffer that was drawn to. THis can be used for resource copy
 *     operations.
 */
typedef void (*dsRenderPassPostDrawFunction)(dsRenderPass* renderPass,
	dsCommandBuffer* commandBuffer);

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
	 * @brief The viewport to draw with.
	 */
	const dsViewport* viewport;

	/**
	 * @brief The list of shaders to use with the render pass.
	 *
	 * This should not be changed after creation.
	 */
	const dsShaderDrawInfo* shaders;

	/**
	 * @brief The number of shaders.
	 */
	uint32_t shaderCount;

	/**
	 * @brief The list of image attachments to use with the render pass.
	 */
	const dsAttachmentInfo* attachments;

	/**
	 * @brief The number of attachments.
	 */
	uint32_t attachmentCount;

	/**
	 * @brief The list of subpasses for this render pass.
	 */
	const dsRenderSubpassInfo* subpasses;

	/**
	 * @brief The number of subpasses.
	 */
	uint32_t subpassCount;

	/**
	 * @brief The list of subpass explicit subpass dependencies.
	 *
	 * If not specified, the default behavior is for each subpass' fragment stage to depend
	 * on the previous subpass' fragment stage.
	 */
	const dsSubpassDependency* subpassDependencies;

	/**
	 * @brief The number of subpass dependencies.
	 */
	uint32_t subpassDependeniesCount;

	/**
	 * @brief The render states to apply to the render pass.
	 *
	 * This should not be modified directly after creation.
	 */
	dsRenderState renderState;

	/**
	 * @brief User data to set on the render pass.
	 */
	void* userData;

	/**
	 * @brief The draw function.
	 *
	 * This will be used to provide the data to be drawn by the render pass.
	 */
	dsRenderPassDrawFunction drawFunc;

	/**
	 * @brief The post-draw function.
	 *
	 * If provided, this will can be used to perform operations after drawing has completed.
	 */
	dsRenderPassPostDrawFunction postDrawFunc;
};

/**
 * @brief Structure defining a group of render passes.
 *
 * The render passes will be processed based on thier dependencies between each other. The passes
 * may be
 */
typedef struct dsRenderPassGroup dsRenderPassGroup;

/**
 * @brief function called to update the renderer each fream.
 * @param renderer The renderer.
 * @param renderPasses The list of render pass groups that will be drawn.
 * @param renderPassesCount The number of elements in renderPasses.
 */
typedef void (*dsRenderUpdateFunction)(dsRenderer* renderer, dsRenderPassGroup** renderPasses,
	uint32_t renderPassesCount);

/**
 * @brief Function called to update the resources each frame.
 * @param The renderer.
 * @param commandBuffer The command buffer to use with resource copy oerations.
 */
typedef void (*dsRenderUpdateResourcesFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer);

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
	 * @brief User data set on the renderer.
	 */
	void* userData;

	/**
	 * @brief The frame update function.
	 */
	dsRenderUpdateFunction updateFunc;

	/**
	 * @brief The resource update function.
	 */
	dsRenderUpdateResourcesFunction updateResourcesFunc;
};

#ifdef __cplusplus
}
#endif
