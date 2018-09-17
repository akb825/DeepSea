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
 * @brief Constant for an external subpass.
 */
#define DS_EXTERNAL_SUBPASS (uint32_t)-1

/**
 * @brief Constant to use default subpass dependencies.
 */
#define DS_DEFAULT_SUBPASS_DEPENDENCIES (uint32_t)-1

/**
 * @brief Constant for the default number of anti-alias samples.
 */
#define DS_DEFAULT_ANTIALIAS_SAMPLES (uint32_t)-1

/**
 * @brief The size of the UUID that uniquely identifies a device.
 */
#define DS_DEVICE_UUID_SIZE 16

/**
 * @brief Enum for the type of render device.
 *
 * This enum is ordered from most to least desireable.
 */
typedef enum dsRenderDeviceType
{
	dsRenderDeviceType_Discrete,   ///< Discrete GPU. (separate graphics card)
	dsRenderDeviceType_Virtual,    ///< Virtual GPU in a virtualized environment.
	dsRenderDeviceType_Integrated, ///< Integrated GPU. (built into CPU or mobile device)
	dsRenderDeviceType_CPU,        ///< CPU without hardware graphics acceleration.
	dsRenderDeviceType_Unknown     ///< Unknown device type.
} dsRenderDeviceType;

/**
 * @brief Enum for how an image attachment will be used.
 *
 * This enum is a bitmask to allow multiple combinations of the usage bits.
 * @see RenderSurface.h
 */
typedef enum dsAttachmentUsage
{
	dsAttachmentUsage_Standard = 0,     ///< No special usage.
	dsAttachmentUsage_Clear = 0x1,      ///< Clear the contents of the attachment before rendering.
	dsAttachmentUsage_KeepBefore = 0x2, ///< Keep the existing value before rendering begins.
	dsAttachmentUsage_KeepAfter = 0x4,  ///< Keep the value after rendering ends.
	/**
	 * Resolve multisample attachment once the render pass has completed.
	 */
	dsAttachmentUsage_Resolve = 0x8
} dsAttachmentUsage;

/**
 * @brief Enum for the stage for a pipeline dependency.
 * @see RenderPass.h
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
	dsRenderSurfaceType_Direct,  ///< Surface was already set up to be used directly for rendering.
	dsRenderSurfaceType_Window,  ///< Window surface.
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
	dsCommandBufferUsage_Standard = 0,      ///< Standard usage.
	dsCommandBufferUsage_Subpass = 0x1,     ///< Will only be used within a render subpass.
	dsCommandBufferUsage_MultiSubmit = 0x2, ///< Will be submitted multiple times in a frame.
	dsCommandBufferUsage_MultiFrame = 0x4,  ///< Will be submitted across frames.
	/**
	 * Double-buffer the command buffers within the pool, allowing for writing to one set of buffers
	 * in parallel to another set being submitted.
	 */
	dsCommandBufferUsage_DoubleBuffer = 0x8
} dsCommandBufferUsage;

/**
 * @brief Enum for whether to clear the depth or stencil value for a surface.
 * @see Renderer.h
 */
typedef enum dsClearDepthStencil
{
	dsClearDepthStencil_Depth,   ///< Clear only the depth value.
	dsClearDepthStencil_Stencil, ///< Clear only the stencil value.
	dsClearDepthStencil_Both     ///< Clear both depth and stencil values.
} dsClearDepthStencil;

/**
 * @brief Bitmask enum for determining graphics memory access.
 */
typedef enum dsGfxAccess
{
    dsGfxAccess_IndirectCommand = 0x1,         ///< Indirect draw/compute buffers.
    dsGfxAccess_Index = 0x2,                   ///< Index buffers.
    dsGfxAccess_VertexAttribute = 0x4,         ///< Vertex buffers.
    dsGfxAccess_UniformBlock = 0x8,            ///< Uniform blocks.
    dsGfxAccess_UniformBuffer = 0x10,          ///< Uniform buffers.
    dsGfxAccess_InputAttachment = 0x20,        ///< Render pass input attachments.
    dsGfxAccess_DepthStencilAttachment = 0x40, ///< Depth/stencil attachments.
    dsGfxAccess_ColorStencilAttachment = 0x80, ///< Color attachments.
    dsGfxAccess_Copy = 0x100,                  ///< Copy operation.
    dsGfxAccess_MappedBuffer = 0x200,          ///< Direct access to a mapped buffer.
    dsGfxAccess_Memory = 0x400                 ///< General memory access.
} dsGfxAccess;

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
 * @brief Description for the physical device to render on.
 */
typedef struct dsRenderDeviceInfo
{
	/**
	 * @brief The name of the device.
	 */
	const char* name;

	/**
	 * @brief ID for the vendor that provides the device.
	 */
	uint32_t vendorID;

	/**
	 * @brief The ID for the device provided by the vendor.
	 */
	uint32_t deviceID;

	/**
	 * @brief The type of the device.
	 */
	dsRenderDeviceType deviceType;

	/**
	 * @brief True if the device will be the default.
	 */
	bool isDefault;

	/**
	 * @brief UUID uniquely identifying the device.
	 */
	uint8_t deviceUUID[DS_DEVICE_UUID_SIZE];
} dsRenderDeviceInfo;

/**
 * @brief Struct containing the otpions for initializing a renderer.
 */
typedef struct dsRendererOptions
{
	/**
	 * @brief The platform display.
	 */
	void* display;

	/**
	 * @brief The name of the application.
	 */
	const char* applicationName;

	/**
	 * @brief The version of the application.
	 */
	uint32_t applicationVersion;

	/**
	 * @brief The number of bits for the red channel.
	 */
	uint8_t redBits;

	/**
	 * @brief The number of bits for the green channel.
	 */
	uint8_t greenBits;

	/**
	 * @brief The number of bits for the blue channel.
	 */
	uint8_t blueBits;

	/**
	 * @brief The number of bits for the alpha channel.
	 */
	uint8_t alphaBits;

	/**
	 * @brief The number of bits for the depth buffer.
	 */
	uint8_t depthBits;

	/**
	 * @brief The number of bits for the stencil buffer.
	 */
	uint8_t stencilBits;

	/**
	 * @brief The default number of anti-alias samples.
	 *
	 * This may be changed later, but all surfaces must be re-created. It will be clamped to the
	 * maximum number of supported samples.
	 */
	uint8_t samples;

	/**
	 * @brief True to double-buffer rendering, false to single-buffer.
	 */
	bool doubleBuffer;

	/**
	 * @brief True to use sRGB, false to use linear color space.
	 */
	bool srgb;

	/**
	 * @brief True to use stereoscopic rendering, false to use a single surface.
	 */
	bool stereoscopic;

	/**
	 * @brief True to enable debugging.
	 */
	bool debug;

	/**
	 * @brief The maximum number of resource threads.
	 */
	uint8_t maxResourceThreads;

	/**
	 * @brief Directory to cache shader binaries.
	 *
	 * This will be copied when the renderer is created so it need not be permanently allocated.
	 */
	const char* shaderCacheDir;

	/**
	 * @brief The UUID of the device to use.
	 *
	 * If not found, the default will be used. Use ds*Renderer_queryDevices() to get the list of
	 * devices and UUIDs associated with them.
	 */
	uint8_t deviceUUID[DS_DEVICE_UUID_SIZE];

	/**
	 * @brief The allocator to use for the graphics API when supported.
	 */
	dsAllocator* gfxAPIAllocator;
} dsRendererOptions;

/**
 * @brief Struct containing a shader version.
 *
 * Applications will typically compile for specific shader versions. This structure can be used to
 * provide what versions are available, which can then be used to choose which version to use at
 * runtime.
 */
typedef struct dsShaderVersion
{
	/**
	 * @brief The renderer ID the shader version is used for.
	 *
	 * This not only differentiates between completely different renderer types (e.g. Vulkan vs.
	 * OpenGL), but also different variations of the same renderer. (e.g. OpenGL vs. OpenGL ES)
	 */
	uint32_t rendererID;

	/**
	 * @brief The version of the shader, encoded with DS_ENCODE_VERSION().
	 */
	uint32_t version;
} dsShaderVersion;

/**
 * @brief Structure defining a render surface, such as a window.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderSurface and the true internal type.
 *
 * @see RenderSurface.h
 */
typedef struct dsRenderSurface
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
	 * @brief The name of the surface.
	 */
	const char* name;

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
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderPass and the true internal type.
 *
 * @see RenderPass.h
 */
typedef struct dsRenderPass dsRenderPass;

/**
 * @brief The info for an image attachment.
 *
 * This provides information ahead of time that can help improve performance during rendering.
 * @see RenderPass.h
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
	 *
	 * This may be set to DS_DEFAULT_ANTIALIAS_SAMPLES to use the default set on the renderer.
	 */
	uint32_t samples;
} dsAttachmentInfo;

/**
 * @brief Reference for a color attachment.
 * @see RenderPass.h
 */
typedef struct dsColorAttachmentRef
{
	/**
	 * @brief The index to the attachment.
	 *
	 * If DS_NO_ATTACHMENT, nothing will be written to the attachment.
	 */
	uint32_t attachmentIndex;

	/**
	 * @brief True to resolve a multisampled attachment after the subpass.
	 *
	 * This should be set if a multisampled offscreen will be used in a later subpass. If this is
	 * set, it's best not to set the dsAttachmentUsage_Resolve flag to avoid resolving twice.
	 */
	bool resolve;
} dsColorAttachmentRef;

/**
 * @brief Structure defining what is used for a subpass.
 * @see RenderPass.h
 */
typedef struct dsRenderSubpassInfo
{
	/**
	 * @brief The name of this subpass, used for profiling info.
	 *
	 * This should be allocated for the duration of the application, such as a string constant.
	 */
	const char* name;

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
	const dsColorAttachmentRef* colorAttachments;

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
 *
 * @see RenderPass.h
 */
typedef struct dsSubpassDependency
{
	/**
	 * @brief The index of the source subpass.
	 *
	 * This may be DS_EXTERNAL_SUBPASS to be a subpass outside of the current render pass.
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
	 *
	 * This may be DS_EXTERNAL_SUBPASS to be a subpass outside of the current render pass.
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
	uint32_t subpassDependencyCount;
};

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

/// @cond Doxygen_Suppress
/// \{
typedef struct dsCommandBufferProfileInfo
{
	uint32_t beginSurfaceIndex;
	uint32_t beginSurfaceSwapCount;
	uint32_t beginSubpassIndex;
	uint32_t beginSubpassSwapCount;
	uint32_t beginComputeIndex;
	uint32_t beginComputeSwapCount;
} dsCommandBufferProfileInfo;
/// \}
/// @endcond

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

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The usage of the command buffer.
	 *
	 * This should be inherited from the parent pool or dsCommandBufferUsage if not part of a pool.
	 */
	dsCommandBufferUsage usage;

	/**
	 * @brief True if inside of a beginFrame/endFrame block.
	 */
	bool frameActive;

	/**
	 * @brief The currently bound render surface.
	 * @remark This might be NULL even within a render pass for sub-command buffers.
	 */
	const dsRenderSurface* boundSurface;

	/**
	 * @brief The currently bound framebuffer.
	 * @remark This might be NULL even within a render pass for sub-command buffers.
	 */
	const dsFramebuffer* boundFramebuffer;

	/**
	 * @brief The currently bound render pass.
	 */
	const dsRenderPass* boundRenderPass;

	/**
	 * @brief The currently active render subpass.
	 */
	uint32_t activeRenderSubpass;

	/**
	 * @brief True if only indirect commands are supported.
	 */
	bool indirectCommands;

	/**
	 * @brief The currently bound shader.
	 */
	const dsShader* boundShader;

	/**
	 * @brief The currently bound compute shader.
	 */
	const dsShader* boundComputeShader;

	/**
	 * @brief Internal value used for GPU profiling.
	 */
	dsCommandBufferProfileInfo _profileInfo;
} dsCommandBuffer;

/**
 * @brief Union for a color value of a surface.
 *
 * Which member of the union is used depends on the type of the surface.
 * @see Renderer.h
 */
typedef union dsSurfaceColorValue
{
	/**
	 * @brief Color value for float and snorm surfaces.
	 */
	dsColor4f floatValue;

	/**
	 * @brief Color value for integer surfaces.
	 */
	int intValue[4];

	/**
	 * @brief Color value for unsigned integer surfaces.
	 */
	unsigned int uintValue[4];
} dsSurfaceColorValue;

/**
 * @brief Struct containing a combined depth and stencil value.
 * @see Renderer.h
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
 * @see Renderer.h
 */
typedef union dsSurfaceClearValue
{
	/**
	 * @brief Color value for color surfaces.
	 */
	dsSurfaceColorValue colorValue;

	/**
	 * @brief Depth and stencil value for depth-stencil surfaces.
	 */
	dsDepthStencilValue depthStencil;
} dsSurfaceClearValue;

/**
 * @brief Structure defining the range of data to draw without an index buffer.
 * @see Renderer.h
 */
typedef struct dsDrawRange
{
	/**
	 * @brief The number of vertices to draw.
	 */
	uint32_t vertexCount;

	/**
	 * @brief The number of instances to draw.
	 */
	uint32_t instanceCount;

	/**
	 * @brief The first vertex to draw.
	 */
	uint32_t firstVertex;

	/**
	 * @brief The first instance to draw.
	 */
	uint32_t firstInstance;
} dsDrawRange;

/**
 * @brief Structure defining the range of data to draw with an index buffer.
 * @see Renderer.h
 */
typedef struct dsDrawIndexedRange
{
	/**
	 * @brief The number of indices to draw.
	 */
	uint32_t indexCount;

	/**
	 * @brief The number of instances to draw.
	 */
	uint32_t instanceCount;

	/**
	 * @brief The first instance to draw.
	 */
	uint32_t firstIndex;

	/**
	 * @brief The offset to apply to each index value.
	 */
	int32_t vertexOffset;

	/**
	 * @brief The first instance to draw.
	 */
	uint32_t firstInstance;
} dsDrawIndexedRange;

/**
 * @brief Structure defining the region to blit for a surface.
 * @see Renderer.h
 */
typedef struct dsSurfaceBlitRegion
{
	/**
	 * @brief The position for the source surface.
	 */
	dsTexturePosition srcPosition;

	/**
	 * @brief The position for the destination surface.
	 */
	dsTexturePosition dstPosition;

	/**
	 * @brief The width of the source image to blit from.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t srcWidth;

	/**
	 * @brief The height of the source image to blit from.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t srcHeight;

	/**
	 * @brief The width of the destination image to blit to.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t dstWidth;

	/**
	 * @brief The height of the destination image to blit to.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t dstHeight;

	/**
	 * @brief The number of layers to blit.
	 *
	 * This is the depth multiplied by the number of faces.
	 */
	uint32_t layers;
} dsSurfaceBlitRegion;

/**
 * @brief Struct declaring a memory dependency for a barrier.
 * @see Renderer.h
 */
typedef struct dsGfxMemoryBarrier
{
	/**
	 * @brief Access bitmask for what was written.
	 */
	dsGfxAccess writeAccess;

	/**
	 * @brief Access bitmask for what will be read.
	 */
	dsGfxAccess readAccess;
} dsGfxMemoryBarrier;

/// \{
typedef struct dsGPUProfileContext dsGPUProfileContext;
/// \}

/**
 * @brief Function to query whether or not a renderer is supported.
 *
 * This render implementation will provide a ds*Renderer_isSupported() function to query if it's
 * supported.
 *
 * @return True if the renderer is supported.
 */
typedef bool (*dsIsRendererSupportedFunction)(void);

/**
 * @brief Function for querying the devices.
 *
 * The render implementation will provide a ds*Renderer_queryDevices() function. This may return an
 * empty set of devices if no information is provided by the renderer implementation.
 *
 * @param[out] outDevices The output devices. This can be NULL to simply query the number of
 *     devices.
 * @param[inout] outDeviceCount The number of devices that were set. If outDevices isn't NULL, the
 *     initial value is the capacity of outDevices.
 * @return False if an error occurred.
 */
typedef bool (*dsQueryRenderDevicesFunction)(dsRenderDeviceInfo* outDevices,
	uint32_t* outDeviceCount);

/**
 * @brief Function for creating a renderer.
 *
 * The renderer implementations will provide a ds*Renderer_create() function with this signature.
 *
 * @param allocator The allocator for the renderer.
 * @param options The options for the renderer.
 * @return The renderer, or NULL if an error occurred.
 */
typedef dsRenderer* (*dsCreateRendererFunction)(dsAllocator* allocator,
	const dsRendererOptions* options);

/**
 * @brief Function for destroying a renderer.
 * @param renderer The renderer to destroy.
 * @return False if the renderer couldn't be destroyed.
 */
typedef bool (*dsDestroyRendererFunction)(dsRenderer* renderer);

/**
 * @brief Function for enabling/disabling extra debugging for a renderer.
 * @param renderer The renderer to enable/disable extra debugging for.
 * @param enable True to enable, false to disable.
 */
typedef void (*dsSetExtraRendererDebuggingFunction)(dsRenderer* renderer, bool enable);

/**
 * @brief Function for creating a render surface.
 * @param renderer The renderer to use the render surface with.
 * @param allocator The allocator to create the render surface.
 * @param name The name of the render surface, used for profiling info
 * @param osHandle The OS handle, such as window handle.
 * @param type The type of the render surface.
 * @return The created render surface, or NULL if it couldn't be created.
 */
typedef dsRenderSurface* (*dsCreateRenderSurfaceFunction)(dsRenderer* renderer,
	dsAllocator* allocator, const char* name, void* osHandle, dsRenderSurfaceType type);

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
	dsRenderSurface** renderSurfaces, size_t count);

/**
 * @brief Function for creating a command buffer pool.
 * @param renderer The renderer the command buffers will be used wtih.
 * @param allocator The allocator to create the command buffer pool.
 * @param usage The usage flags. Set to 0 if none of the usage options are needed.
 * @param count The number of command buffers to create in the pool.
 * @return The command buffer pool.
 */
typedef dsCommandBufferPool* (*dsCreateCommandBufferPoolFunction)(dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage, uint32_t count);

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
	dsCommandBuffer* submitBuffer);

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
 * @param subpassCount The number of subpasses within the render pass.
 * @param dependencies The dependencies between subpasses.
 * @param dependencyCount The number of dependencies.
 * @return The created render pass, or NULL if it couldn't be created.
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
 * @param renderPass The render pass to begin.
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
typedef bool (*dsBeginRenderPassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool indirectCommands);

/**
 * @brief Function for advancing to the next subpass within a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to continue.
 * @param index The index of the subpass.
 * @param indirectCommands True if the render commands for the subpass will be provided with command
 *     buffers, false if the render commands will be inlined.
 * @return False if the render pass couldn't be advanced.
 */
typedef bool (*dsNextRenderSubpassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index, bool indirectCommands);

/**
 * @brief Function for ending a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to end.
 * @return False if the render pass couldn't be ended.
 */
typedef bool (*dsEndRenderPassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass);

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
 * @brief Function for setting the number of anti-alias samples for the default render surfaces.
 *
 * This should set the default value on the renderer on success. The implementation is responsible
 * for making any necessary changse to the render passes when the attachment info is set to
 * DS_DEFAULT_ANTIALIAS_SAMPLES. The caller is responsible for re-creating any render surfaces,
 * offscreens, renderbuffers, and framebuffers.
 *
 * @param renderer The renderer.
 * @param samples The number of anti-alias samples.
 * @return False if the number of samples couldn't be set.
 */
typedef bool (*dsSetRenderSurfaceSamplesFunction)(dsRenderer* renderer, uint32_t samples);

/**
 * @brief Function for setting whether or not to wait for vsync.
 * @param renderer The renderer.
 * @param vsync True to wait for vsync.
 * @return False if the vsync couldn't be set.
 */
typedef bool (*dsSetRenderVsyncFunction)(dsRenderer* renderer, bool vsync);

/**
 * @brief Function for setting the default anisotropy for anisotropic filtering.
 * @param renderer The renderer.
 * @param anisotropy The default anisotropy.
 * @return False if the anisotropy couldn't be set.
 */
typedef bool (*dsSetRenderDefaultAnisotropyFunction)(dsRenderer* renderer, float anisotropy);

/**
 * @brief Function for clearing a color surface.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the clear command on.
 * @param surface The surface to clear.
 * @param colorValue The color value to clear with.
 * @return False if the surface couldn't be cleared.
 */
typedef bool (*dsRenderClearColorSurfaceFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	const dsSurfaceColorValue* colorValue);

/**
 * @brief Function for clearing a depth-stencil surface.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the clear command on.
 * @param surface The surface to clear.
 * @param surfaceParts The parts of the surface to clear.
 * @param depthStencilValue The depth-stencil value to clear with.
 * @return False if the surface couldn't be cleared.
 */
typedef bool (*dsRenderClearDepthStencilSurfaceFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue);

/**
 * @brief Function for drawing vertex geometry with the currently bound shader.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange);

/**
 * @brief Function for drawing indexed geometry with the currently bound shader.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawIndexedFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange);

/**
 * @brief Function for indirectly drawing vertex geometry with the currently bound shader.
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
typedef bool (*dsRenderDrawIndirectFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride);

/**
 * @brief Function for indirectly drawing indexed geometry with the currently bound shader.
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
typedef bool (*dsRenderDrawIndexedIndirectFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsGfxBuffer* indirectBuffer, size_t offset, uint32_t count, uint32_t stride);

/**
 * @brief Function for dispatching a compute job.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the dispatch command on.
 * @param x The number of working groups in the X direction.
 * @param y The number of working groups in the Y direction.
 * @param z The number of working groups in the Z direction.
 * @return False if the compute job couldn't be dispatched.
 */
typedef bool (*dsRenderDispatchComputeFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, uint32_t x, uint32_t y, uint32_t z);

/**
 * @brief Function for dispatching an indirect compute job.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the dispatch command on.
 * @param indirectBuffer The buffer that contains the number of working groups in the X, Y, and Z
 *     dimensions as 4-byte unsigned integers.
 * @param offset The offset into the indirect buffer.
 * @return False if the compute job couldn't be dispatched.
 */
typedef bool (*dsRenderDispatchComputeIndirectFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsGfxBuffer* indirectBuffer, size_t offset);

/**
 * @brief Function for blitting from one image to another, scaling when necessary.
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
typedef bool (*dsBlitSurfaceFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter);

/**
 * @brief Function for adding a memory barrier.
 * @param renderer The rendferer.
 * @param commandBuffer The command buffer to place the barrier on.
 * @param barriers List of write/read dependencies to place barriers for.
 * @param barrierCount The number of barriers.
 * @return False if the barrier couldn't be added.
 */
typedef bool (*dsGfxMemoryBarrierFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount);

/**
 * @brief Function for waiting until the GPU is idle.
 * @param renderer The renderer.
 * @return False if the renderer is in an invalid state.
 */
typedef bool (*dsRenderWaitUntilIdleFunction)(dsRenderer* renderer);

/**
 * @brief Function for restoring the global state.
 * @param renderer The renderer.
 * @return False if the state couldn't be restored.
 */
typedef bool (*dsRenderRestoreGlobalState)(dsRenderer* renderer);

/** @copydoc dsRenderer */
struct dsRenderer
{

	// ----------------------------------- Internal objects ----------------------------------------

	/**
	 * @brief The main allocator for the Renderer library.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Manager for resources used with the renderer.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The main command buffer.
	 *
	 * This should only be used from the main thread. The pointer may change after calling
	 * dsRenderer_beginFrame() depending on the implementation.
	 */
	dsCommandBuffer* mainCommandBuffer;

	// --------------------------------- Renderer information --------------------------------------

	/**
	 * @brief The ID of the renderer.
	 *
	 * This is typically created with DS_FOURCC() to describe the ID.
	 */
	uint32_t rendererID;

	/**
	 * @brief The ID of the renderer platform.
	 *
	 * This is typically created with DS_FOURCC() to describe the ID.
	 */
	uint32_t platformID;

	/**
	 * @brief The name of the renderer.
	 */
	const char* name;

	/**
	 * @brief The name for the shader language.
	 *
	 * This should be set by the renderer so "shaderLanguage-shaderVersion" is the same as the
	 * standard configs provided by the standard renderer implementations.
	 */
	const char* shaderLanguage;

	/**
	 * @brief The name of the GPU vendor.
	 *
	 * This may be NULL if not known. Either vendorName or vendorID should be set.
	 */
	const char* vendorName;

	/**
	 * @brief The name of the GPU driver.
	 *
	 * This should never be NULL.
	 */
	const char* driverName;

	/**
	 * @brief The shader version number, encoded with DS_ENCODE_VERSION().
	 */
	uint32_t shaderVersion;

	/**
	 * @brief The ID of the GPU vendor.
	 *
	 * This may be 0 if not known. Either vendorName or vendorID should be set.
	 */
	uint32_t vendorID;

	/**
	 * @brief The ID of the GPU driver.
	 *
	 * This may be 0 if not known.
	 */
	uint32_t driverID;

	/**
	 * @brief The version of the GPU driver.
	 *
	 * This will either be encoded with DS_ENCODE_VERSION() or 0 if not known.
	 */
	uint32_t driverVersion;

	/**
	 * @brief Thread ID for the main thread.
	 *
	 * Some operations may only be done from the main thread.
	 */
	dsThreadID mainThread;

	/**
	 * @brief The current frame number.
	 *
	 * This is incremented when calling dsRenderer_beginFrame().
	 */
	uint64_t frameNumber;

	// --------------------------------- Renderer capabilities -------------------------------------

	/**
	 * @brief The maximum number of color attachments when drawing a render subpass.
	 */
	uint32_t maxColorAttachments;

	/**
	 * @brief The maximum number of samples for render surfaces.
	 */
	uint32_t maxSurfaceSamples;

	/**
	 * @brief The configuration for the surface.
	 *
	 * This will be the visual ID for X11. It is currently undefined for other platforms.
	 */
	void* surfaceConfig;

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
	uint32_t surfaceSamples;

	/**
	 * @brief True if render surfaces are double-buffered.
	 */
	bool doubleBuffer;

	/**
	 * @brief True if render surfaces are stereoscopic.
	 */
	bool stereoscopic;

	/**
	 * @brief True to wait for vsync when drawing to a render surface.
	 */
	bool vsync;

	/**
	 * @brief True if the depth range is [0, 1] instead of [-1, 1] in clip space.
	 */
	bool clipHalfDepth;

	/**
	 * @brief True if the Y coordinate of clip space is inverted.
	 */
	bool clipInvertY;

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
	 * @brief True if indirect draws with a count > 1 will use a single graphics API call.
	 */
	bool hasNativeMultidraw;

	/**
	 * @brief Whether or not instanced drawing is supported.
	 */
	bool supportsInstancedDrawing;

	/**
	 * @brief Whether or not the first instance can be set.
	 */
	bool supportsStartInstance;

	/**
	 * @brief The default level of anisotropy for anisotropic filtering.
	 */
	float defaultAnisotropy;

	// ----------------------------- Internals and function table ----------------------------------

	/**
	 * @brief Context used internally for GPU profiling.
	 */
	dsGPUProfileContext* _profileContext;

	/**
	 * @brief Render destroy function.
	 */
	dsDestroyRendererFunction destroyFunc;

	/**
	 * @brief Extra debugging set function.
	 */
	dsSetExtraRendererDebuggingFunction setExtraDebuggingFunc;

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

	/**
	 * @brief Render pass creation function.
	 */
	dsCreateRenderPassFunction createRenderPassFunc;

	/**
	 * @brief Render pass destruction function.
	 */
	dsDestroyRenderPassFunction destroyRenderPassFunc;

	/**
	 * @brief Render pass begin function.
	 */
	dsBeginRenderPassFunction beginRenderPassFunc;

	/**
	 * @brief Render subpass advancement function.
	 */
	dsNextRenderSubpassFunction nextRenderSubpassFunc;

	/**
	 * @brief Render pass end function.
	 */
	dsEndRenderPassFunction endRenderPassFunc;

	/**
	 * @brief Frame begin function.
	 */
	dsBeginFrameFunction beginFrameFunc;

	/**
	 * @brief Frame end function.
	 */
	dsEndFrameFunction endFrameFunc;

	/**
	 * @brief Surface anti-alias sample set function.
	 */
	dsSetRenderSurfaceSamplesFunction setSurfaceSamplesFunc;

	/**
	 * @brief Vsync set function.
	 */
	dsSetRenderVsyncFunction setVsyncFunc;

	/**
	 * @brief Default anisotropy set function.
	 */
	dsSetRenderDefaultAnisotropyFunction setDefaultAnisotropyFunc;

	/**
	 * @brief Color surface clear function.
	 */
	dsRenderClearColorSurfaceFunction clearColorSurfaceFunc;

	/**
	 * @brief Depth-stencil surface clear function.
	 */
	dsRenderClearDepthStencilSurfaceFunction clearDepthStencilSurfaceFunc;

	/**
	 * @brief Draw function.
	 */
	dsRenderDrawFunction drawFunc;

	/**
	 * @brief Indexed draw function.
	 */
	dsRenderDrawIndexedFunction drawIndexedFunc;

	/**
	 * @brief Indirect draw function.
	 */
	dsRenderDrawIndirectFunction drawIndirectFunc;

	/**
	 * @brief Indirect indexed draw function.
	 */
	dsRenderDrawIndexedIndirectFunction drawIndexedIndirectFunc;

	/**
	 * @brief Compute shader dispatch function.
	 */
	dsRenderDispatchComputeFunction dispatchComputeFunc;

	/**
	 * @brief Compute shader indirect dispatch function.
	 */
	dsRenderDispatchComputeIndirectFunction dispatchComputeIndirectFunc;

	/**
	 * @brief Surface blitting function.
	 */
	dsBlitSurfaceFunction blitSurfaceFunc;

	/**
	 * @brief Memory barrier function.
	 */
	dsGfxMemoryBarrierFunction memoryBarrierFunc;

	/**
	 * @brief Idle waiting function.
	 */
	dsRenderWaitUntilIdleFunction waitUntilIdleFunc;

	/**
	 * @brief Global state restore function.
	 */
	dsRenderRestoreGlobalState restoreGlobalStateFunc;
};

#ifdef __cplusplus
}
#endif
