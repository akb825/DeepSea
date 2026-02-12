/*
 * Copyright 2016-2026 Aaron Barany
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
#include <DeepSea/Render/Shadows/Types.h>
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
#define DS_NO_ATTACHMENT ((uint32_t)-1)

/**
 * @brief Constant for an external subpass.
 */
#define DS_EXTERNAL_SUBPASS ((uint32_t)-1)

/**
 * @brief Constant to use default subpass dependencies.
 */
#define DS_DEFAULT_SUBPASS_DEPENDENCIES ((uint32_t)-1)

/**
 * @brief Constant for the number of anti-alias samples of render surfaces.
 */
#define DS_SURFACE_ANTIALIAS_SAMPLES ((uint32_t)-1)

/**
 * @brief Constant for the default number of anti-alias samples.
 */
#define DS_DEFAULT_ANTIALIAS_SAMPLES ((uint32_t)-2)

/**
 * @brief Constant for the maximum supported anti-alias samples.
 *
 * This may be further limited by what's supported by the target hardware.
 */
#define DS_MAX_ANTIALIAS_SAMPLES 32

/**
 * @brief Constant for the maximum number of color attachments.
 *
 * This may be further limited by what's supported by the target hardware.
 */
#define DS_MAX_ATTACHMENTS 16

/**
 * @brief The size of the UUID that uniquely identifies a device.
 */
#define DS_DEVICE_UUID_SIZE 16

/**
 * @brief Vendor ID for AMD GPUs.
 */
#define DS_VENDOR_ID_AMD 0x1002

/**
 * @brief Vendor ID for ImgTec GPUs.
 */
#define DS_VENDOR_ID_IMGTEC 0x1010

/**
 * @brief Vendor ID for NVidia GPUs.
 */
#define DS_VENDOR_ID_NVIDIA 0x10DE

/**
 * @brief Vendor ID for ARM GPUs.
 */
#define DS_VENDOR_ID_ARM 0x13B5

/**
 * @brief Vendor ID for Qualcomm GPUs.
 */
#define DS_VENDOR_ID_QUALCOMM 0x5143

 /**
 * @brief Vendor ID for Intel GPUs.
 */
#define DS_VENDOR_ID_INTEL 0x8086

/**
 * @brief Enum for the type of render device.
 *
 * This enum is ordered from most to least desireable.
 */
typedef enum dsRenderDeviceType
{
	dsRenderDeviceType_Discrete,   ///< Discrete GPU. (separate graphics card)
	dsRenderDeviceType_Virtual,    ///< GPU exposed in a virtualized environment.
	dsRenderDeviceType_Integrated, ///< Integrated GPU. (built into CPU or mobile device)
	dsRenderDeviceType_CPU,        ///< CPU without hardware graphics acceleration.
	dsRenderDeviceType_Unknown     ///< Unknown device type.
} dsRenderDeviceType;

/**
 * @brief Enum for adjusting vsync.
 *
 * Triple buffering may have better performance than double buffering, though may have increased
 * latency. Triple buffering may not be available for all renderer implementations, in which case
 * double buffering will be used instead.
 */
typedef enum dsVSync
{
	dsVSync_Disabled,     ///< Render as quickly as possible without waiting for vsync.
	dsVSync_DoubleBuffer, ///< Wait for vsync with two render buffers.
	dsVSync_TripleBuffer  ///< Wait for vsync with three render buffers.
} dsVSync;

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
	dsAttachmentUsage_UseLater = 0x8    ///< May render with the attachment later.
} dsAttachmentUsage;

/**
 * @brief Enum for the platform for the graphics system.
 *
 * In many cases only a single choice is available. This enum will only contain the platforms that
 * can be chosen.
 */
typedef enum dsGfxPlatform
{
	dsGfxPlatform_Default, ///< The default platform.
	dsGfxPlatform_X11,     ///< X11 windowing platform. This is the default on Linux.
	dsGfxPlatform_Wayland  ///< Wayland windowing platform.
} dsGfxPlatform;

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
 * @brief Enum for how a render surface will be used.
 *
 * These are bitmask values, allowing a renderbuffer to be used for multiple purposes.
 * @see RenderSurface.h
 */
typedef enum dsRenderSurfaceUsage
{
	dsRenderSurfaceUsage_Standard = 0x0, ///< Standard usage.
	/**
	 * Can blit from the color surface to another surface.
	 */
	dsRenderSurfaceUsage_BlitColorFrom = 0x1,
	/**
	 * Can blit from another surface to the color surface.
	 */
	dsRenderSurfaceUsage_BlitColorTo = 0x2,
	/**
	 * When set, rendering color may continue across multiple passes or subpasses. When not set, the
	 * multisampled color contents may be discarded or never stored in the first place.
	 */
	dsRenderSurfaceUsage_ContinueColor = 0x4,
	/**
	 * Can blit from the depth/stencil surface to another surface.
	 */
	dsRenderSurfaceUsage_BlitDepthStencilFrom = 0x8,
	/**
	 * Can blit from another surface to the depth/stencil surface.
	 */
	dsRenderSurfaceUsage_BlitDepthStencilTo = 0x10,
	/**
	 * When set, rendering depth/stencil may continue across multiple passes or subpasses. When not
	 * set, the depth/stencil contents may be discarded or never stored in the first place.
	 */
	dsRenderSurfaceUsage_ContinueDepthStencil = 0x20,
	/**
	 * Perform rotations client-side. This may increase performance in cases like phones and tablets
	 * that allow screen rotation. If false, the rotation of the render surface will always be 0
	 * degrees.
	 */
	dsRenderSurfaceUsage_ClientRotations = 0x40
} dsRenderSurfaceUsage;

/**
 * @brief The rotation to apply to the render surface.
 *
 * The client must apply this rotation to any geometry drawn to the render surface to display in the
 * correct oreintation. All rotations are clockwise.
 *
 * The width and height for the surface will be the post-rotated dimensions. For example, if you
 * take a 200 x 100 surface with rotation 0 and rotate it to 90 degrees, the render surface will
 * still report a dimensions of 100 x 200. In other words, the dimensions always correspond to the
 * final logical dimensions of the surface, while dsRenderSurfaceRotation indicates the
 * transformation that needs to be applied to properly orient the image contents.
 *
 * The projection matrix may be adjusted with the matrix from dsRenderSurface_makeRotationMatrix().
 * However, any viewport locations and blit regions must also be adjusted accordingly for the
 * rotation.
 *
 * @see RenderSurface.h
 */
typedef enum dsRenderSurfaceRotation
{
	dsRenderSurfaceRotation_0,   ///< 0 degrees of rotation.
	dsRenderSurfaceRotation_90,  ///< 90 degrees of rotation.
	dsRenderSurfaceRotation_180, ///< 180 degrees of rotation.
	dsRenderSurfaceRotation_270  ///< 270 degrees of rotation.
} dsRenderSurfaceRotation;

/**
 * @brief Enum for how to use a command buffer.
 *
 * This enum is a bitmask to allow multiple combinations of the usage bits.
 * @see CommandBufferPool.h
 */
typedef enum dsCommandBufferUsage
{
	dsCommandBufferUsage_Standard = 0,      ///< Standard usage.
	dsCommandBufferUsage_Secondary = 0x1,   ///< Only used for draw calls within render subpasses.
	dsCommandBufferUsage_Resource = 0x2,    ///< Only used for resource operations.
	dsCommandBufferUsage_MultiSubmit = 0x4, ///< Will be submitted multiple times in a frame.
	dsCommandBufferUsage_MultiFrame = 0x8   ///< Will be submitted multiple times across frames.
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
 * @brief Enum for a stage of the render pipeline.
 *
 * This is typically used in conjunction with dsGfxAccess to determine how memory is accessed inside
 * the GPU stages.
 */
typedef enum dsGfxPipelineStage
{
	dsGfxPipelineStage_CommandBuffer = 0x1,   ///< Begin/end of the command buffer execution.
	dsGfxPipelineStage_DrawIndirect = 0x2,    ///< Consume indirect draw parameters.
	dsGfxPipelineStage_VertexInput = 0x4,     ///< Read vertex attributes and indices.
	dsGfxPipelineStage_VertexShader = 0x8,    ///< Execution of vertex shader.
	/// Execution of tessellation control shader.
	dsGfxPipelineStage_TessellationControlShader = 0x10,
	/// Execution of tessellation evaluation shader.
	dsGfxPipelineStage_TessellationEvaluationShader = 0x20,
	dsGfxPipelineStage_GeometryShader = 0x40, ///< Execution of geometry shader.
	dsGfxPipelineStage_FragmentShader = 0x80, ///< Execution of fragment shader.
	/// Tests before running the fragment shader. This includes reading depth values.
	dsGfxPipelineStage_PreFragmentShaderTests = 0x100,
	/// Tests after running the fragment shader. This includes writing depth values.
	dsGfxPipelineStage_PostFragmentShaderTests = 0x200,
	/// Color output after running the fragment shader. This also handles loads for blending and
	/// multisample resolve.
	dsGfxPipelineStage_ColorOutput = 0x400,
	dsGfxPipelineStage_ComputeShader = 0x800, ///< Execution of compute shader.
	dsGfxPipelineStage_Copy = 0x1000,         ///< Copy between buffers and textures.
	dsGfxPipelineStage_HostAccess = 0x2000,   ///< Access of mapped memory on the host.
	dsGfxPipelineStage_AllGraphics = 0x4000,  ///< All graphics stages.
	dsGfxPipelineStage_AllCommands = 0x8000,  ///< All graphics and compute stages.
} dsGfxPipelineStage;

/**
 * @brief Bitmask enum for determining graphics memory access.
 *
 * This is typically used with dsGfxPipelineStage to determine what rendering stages the access
 * occurs in.
 */
typedef enum dsGfxAccess
{
	dsGfxAccess_None = 0x0,                           ///< No access flags.
	dsGfxAccess_IndirectCommandRead = 0x1,            ///< Read data for indirect draws and compute.
	dsGfxAccess_IndexRead = 0x2,                      ///< Read the index buffer.
	dsGfxAccess_VertexAttributeRead = 0x4,            ///< Read vertex attributes.
	dsGfxAccess_UniformBlockRead = 0x8,               ///< Read a uniform block from a shader.
	dsGfxAccess_UniformBufferRead = 0x10,             ///< Read a uniform buffer from a shader.
	dsGfxAccess_UniformBufferWrite = 0x20,            ///< Write a uniform buffer from a shader.
	dsGfxAccess_TextureRead = 0x40,                   ///< Read a texture from a shader.
	dsGfxAccess_ImageRead = 0x80,                     ///< Read an image from a shader.
	dsGfxAccess_ImageWrite = 0x100,                   ///< Write an image from a shader.
	dsGfxAccess_InputAttachmentRead = 0x200,          ///< Read a subpass input attachment.
	dsGfxAccess_ColorAttachmentRead = 0x400,          ///< Read from a color attachment.
	dsGfxAccess_ColorAttachmentWrite = 0x800,         ///< Write to a color attachment.
	dsGfxAccess_DepthStencilAttachmentRead = 0x1000,  ///< Read from a depth/stencil attachment.
	dsGfxAccess_DepthStencilAttachmentWrite = 0x2000, ///< Write to a depth/stencil attachment.
	dsGfxAccess_CopyRead = 0x4000,                    ///< Read for a copy operation.
	dsGfxAccess_CopyWrite = 0x8000,                   ///< Write for a copy operation.
	dsGfxAccess_HostRead = 0x10000,                   ///< Read mapped memory from the host.
	dsGfxAccess_HostWrite = 0x20000,                  ///< Write mapped memory from the host.
	dsGfxAccess_MemoryRead = 0x40000,                 ///< General memory read access.
	dsGfxAccess_MemoryWrite = 0x80000,                ///< General memory write access.
} dsGfxAccess;

/**
 * @brief Enum for the type of primitive to draw with.
 * @see Shader.h
 * @see Renderer.h
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
 * @brief Enum for the type of a projection.
 * @see dsProjectionParams
 * @see ProjectionParams.h
 */
typedef enum dsProjectionType
{
	dsProjectionType_Ortho,      ///< Orthographic projection.
	dsProjectionType_Frustum,    ///< Frustum with all planes explicitly given.
	dsProjectionType_Perspective ///< Frustum with typical perspective parameters.
} dsProjectionType;

/// @cond
typedef struct dsRenderer dsRenderer;
typedef struct dsRenderPass dsRenderPass;
/// @endcond

/**
 * @brief Description for the physical device to render on.
 */
typedef struct dsRenderDeviceInfo
{
	/**
	 * @brief The name of the device.
	 *
	 * This may be NULL if not known. Either vendorName or vendorID should be set.
	 */
	const char* name;

	/**
	 * @brief ID for the vendor that provides the device.
	 *
	 * This may be 0 if not known. Either vendorName or vendorID should be set.
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
 * @brief Function to create a background surface.
 * @param userData User data to aid with managing background surfaces.
 * @param surfaceType The render surface type.
 */
typedef void* (*dsCreateBackgroundRenderSurfaceFunction)(
	void* userData, dsRenderSurfaceType surfaceType);

/**
 * @brief Function to destroy a background surface.
 * @param userData User data to aid with managing background surfaces.
 * @param surfaceType The render surface type.
 * @param surface The surface to destroy.
 */
typedef void (*dsDestroyBackgroundRenderSurfaceFunction)(
	void* userData, dsRenderSurfaceType surfaceType, void* surface);

/**
 * @brief Function to get the handle for a background surface.
 * @param userData User data to aid with managing background surfaces.
 * @param surfaceType The render surface type.
 * @param surface The surface to get the handle from.
 * @return The window handle to be used directly from the renderer.
 */
typedef void* (*dsGetBackgroundRenderSurfaceHandleFunction)(
	void* userData, dsRenderSurfaceType surfaceType, void* surface);

/**
 * @brief Struct containing the otpions for initializing a renderer.
 */
typedef struct dsRendererOptions
{
	/**
	 * @brief The graphics windowing platform.
	 */
	dsGfxPlatform platform;

	/**
	 * @brief The surface type when background surface creation/destruction functions are provided.
	 */
	dsRenderSurfaceType backgroundSurfaceType;

	/**
	 * @brief The OS display.
	 */
	void* osDisplay;

	/**
	 * @brief The already initialized graphics display for the renderer type.
	 */
	void* gfxDisplay;

	/**
	 * @brief User data for managing background windows.
	 */
	void* backgroundSurfaceUserData;

	/**
	 * @brief Function for creating a background window.
	 *
	 * Background windows are used when a graphics device must be bound with a window for
	 * non-rendering operations.
	 */
	dsCreateBackgroundRenderSurfaceFunction createBackgroundSurfaceFunc;

	/**
	 * @brief Function for destroying a background window.
	 */
	dsDestroyBackgroundRenderSurfaceFunction destroyBackgroundSurfaceFunc;

	/**
	 * @brief Function for getting the handle for a background surface.
	 *
	 * If NULL, the surface returned from createBackgroundSurfaceFunc will be used directly.
	 */
	dsGetBackgroundRenderSurfaceHandleFunction getBackgroundSurfaceHandleFunc;

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
	 * @brief The color format to force dsRenderer.surfaceColorFormat to be.
	 *
	 * If dsGfxFormat_Unknown, will derive a format from the bit counts.
	 */
	dsGfxFormat forcedColorFormat;

	/**
	 * @brief The color format to force dsRenderer.surfaceDepthStencilFormat to be.
	 *
	 * If dsGfxFormat_Unknown, will derive a format from the bit counts.
	 */
	dsGfxFormat forcedDepthStencilFormat;

	/**
	 * @brief The default number of anti-alias samples for render surfaces.
	 *
	 * This may be changed later, but render surfaces must re-created. It will be clamped to the
	 * maximum number of supported samples.
	 */
	uint8_t surfaceSamples;

	/**
	 * @brief The default number of anti-alias samples for offscreens and renderbuffers.
	 *
	 * This may be changed later, but offscreens and renderbuffers re-created. It will be clamped to
	 * the maximum number of supported samples.
	 */
	uint8_t defaultSamples;

	/**
	 * @brief True to avoid using a back buffer for rendering if possible.
	 */
	bool singleBuffer;

	/**
	 * @brief Use reverse Z rendering, inverting the depth range.
	 */
	bool reverseZ;

	/**
	 * @brief True to use sRGB, false to use linear color space.
	 */
	bool srgb;

	/**
	 * @brief True to prefer half depth clip range of [0, 1] rather than [-1, 1].
	 *
	 * Use this option if you intend to use reversed depth with a floating point depth buffer for
	 * increased depth precision.
	 */
	bool preferHalfDepthRange;

	/**
	 * @brief True to use stereoscopic rendering, false to use a single surface.
	 */
	bool stereoscopic;

	/**
	 * @brief True to enable debugging.
	 *
	 * When calling dsRenderer_defaultOptions(), this will default to true for debug builds and
	 * false for release builds. The default can be adjusted by setting the DEEPSEA_GRAPHICS_DEBUG
	 * environment variable to 0 or 1.
	 */
	bool debug;

	/**
	 * @brief The maximum number of resource threads.
	 */
	uint32_t maxResourceThreads;

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
	 * @brief The name of the device to use.
	 *
	 * This will do a case-insensitive substring search based on the device name. This will be
	 * performed after searching for the device UUID. If not found, the default device will be used.
	 */
	const char* deviceName;

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
	 * @brief Usage flags for the surface.
	 */
	dsRenderSurfaceUsage usage;

	/**
	 * @brief The width of the surface.
	 */
	uint32_t width;

	/**
	 * @brief The height of the render surface.
	 */
	uint32_t height;

	/**
	 * @brief The width of the surface before applying rotation.
	 *
	 * This will be different from the width if rotation is 90 or 270 degrees. This is the dimension
	 * that should be used for the framebuffer when used with this surface. This can be ignored if
	 * the dsRenderSurfaceUsage_ClientRotations usage bit isn't set.
	 */
	uint32_t preRotateWidth;

	/**
	 * @brief The height of the render surface before applying rotation.
	 *
	 * This will be different from the height if rotation is 90 or 270 degrees. This is the
	 * dimension that should be used for the framebuffer when used with this surface. This can be
	 * ignored if the dsRenderSurfaceUsage_ClientRotations usage bit isn't set.
	 */
	uint32_t preRotateHeight;

	/**
	 * @brief The rotation to apply to the image.
	 */
	dsRenderSurfaceRotation rotation;
} dsRenderSurface;

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
 * @brief Reference for an attachment.
 * @see RenderPass.h
 */
typedef struct dsAttachmentRef
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
	 * This may be more efficient than using the dsAttachmentUsage_Resolve flag to resolve an
	 * attachment after the subpasses finish. It also allows the resolved result to be available
	 * for other subpass inputs.
	 *
	 * If this is set, it's best not to set the dsAttachmentUsage_Resolve flag to avoid resolving
	 * twice.
	 *
	 * @remark When this is used, the framebuffer surface must be resolvable. Unresolvable surfaces
	 * are renderbuffers and offscreens with the resolve parameter set to false.
	 *
	 * @remark This is ignored when the surface doesn't have multisampling.
	 */
	bool resolve;
} dsAttachmentRef;

/**
 * @brief Structure defining what is used for a subpass.
 * @see RenderPass.h
 */
typedef struct dsRenderSubpassInfo
{
	/**
	 * @brief The name of this subpass, used for profiling info.
	 *
	 * The implementation will copy this string.
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
	const dsAttachmentRef* colorAttachments;

	/**
	 * @brief The depth stencil attachment as an index to the attachment list for the render pass.
	 *
	 * Set to DS_NO_ATTACHMENT to not have a depth attachment.
	 */
	dsAttachmentRef depthStencilAttachment;

	/**
	 * @brief The number of input attachments.
	 */
	uint32_t inputAttachmentCount;

	/**
	 * @brief The number of color attachments.
	 */
	uint32_t colorAttachmentCount;
} dsRenderSubpassInfo;

/**
 * @brief Structure defining an explicit subpass dependency.
 *
 * This ensures that the GPU is done with the specified stage from the source subpass before
 * processing the specified stage for the destination subpass.
 *
 * Dependencies on external render passes (with DS_EXTERNAL_SUBPASS) should only be required if the
 * same attachments are used across multiple render passes or cases where an offscreen written in
 * one render pass is read from another render pass. Other cases, writing to buffers or images, are
 * automatically managed.
 *
 * Subpass dependencies between subpasses within the same render pass are required in two
 * situations:
 * 1. The destination subpass reads from a resource written from the source subpass. This can either
 *    be a color or depth/stencil attachment, buffer write, or image write within a shader. This is
 *    the most obvious situation, where the results need to be available before being read from.
 * 2. The destination subpass writes to a resource read from the source subpass. As with the first
 *    point, attachments, buffers, and images used between both subpasses all require a dependency.
 *    While less obvious of a requirement as the first point, this is required to ensure the
 *    destination subpass doesn't overwrite the data before the source subpass is finished using it.
 *
 * Failure to have the correct dependencies may cause rendering artifacts, depending on if the
 * hardware and underlying render implementation support executing subpasses in parallel.
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
	 * @brief The stages to wait after in the source subpass.
	 */
	dsGfxPipelineStage srcStages;

	/**
	 * @brief The access types to wait after in the source subpass.
	 */
	dsGfxAccess srcAccess;

	/**
	 * @brief The index of the destination subpass.
	 *
	 * This may be DS_EXTERNAL_SUBPASS to be a subpass outside of the current render pass.
	 */
	uint32_t dstSubpass;

	/**
	 * @brief The stages to wait before executing in the destination subpass.
	 */
	dsGfxPipelineStage dstStages;

	/**
	 * @brief The access types wait before executing in the destination subpass.
	 */
	dsGfxAccess dstAccess;

	/**
	 * @brief True if the dependency is by region as opposed to the full surface.
	 */
	bool regionDependency;
} dsSubpassDependency;

/**
 * @brief Struct for a render pass used by the renderer.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderPass and the true internal type.
 *
 * @see RenderPass.h
 */
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
 * Command buffers within the same pool should not be used concurrently across threads. To avoid
 * this, a separate command buffer pool should be used for each thread..
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
	 * @brief The command buffers to use.
	 *
	 * This pointer may change when calling dsCommandBufferPool_reset() if the implementation double
	 * or triple buffers the internal usage.
	 */
	dsCommandBuffer** commandBuffers;

	/**
	 * @brief The number of active command buffers in the pool.
	 */
	uint32_t count;

	/**
	 * @brief The usage flags for the command buffers.
	 */
	dsCommandBufferUsage usage;
} dsCommandBufferPool;

/// @cond Doxygen_Suppress
typedef struct dsCommandBufferProfileInfo
{
	uint32_t beginDeferredResourcesIndex;
	uint32_t beginDeferredResourcesSwapCount;
	uint32_t beginSurfaceIndex;
	uint32_t beginSurfaceSwapCount;
	uint32_t beginSubpassIndex;
	uint32_t beginSubpassSwapCount;
	uint32_t beginComputeIndex;
	uint32_t beginComputeSwapCount;
	bool subpassDebugGroups;
} dsCommandBufferProfileInfo;
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
	 * @brief True if secondary commands are used within a render pass, false if commands are
	 * directly written.
	 */
	bool secondaryRenderPassCommands;

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
	 * @brief The currently set viewport for the render pass.
	 */
	dsAlignedBox3f viewport;

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
 * @brief Struct identifying an attachment to clear and the value to clear it with.
 */
typedef struct dsClearAttachment
{
	/**
	 * @brief The index of the color attachment.
	 *
	 * If DS_NO_ATTACHMENT, then the depth/stencil attachment will be cleared instead.
	 */
	uint32_t colorAttachment;

	/**
	 * @brief Which values to clear when clearing the depth/stencil surface.
	 */
	dsClearDepthStencil clearDepthStencil;

	/**
	 * @brief The value to clear the surface with.
	 */
	dsSurfaceClearValue clearValue;
} dsClearAttachment;

/**
 * @brief Struct defining the region to clear an attachment.
 */
typedef struct dsAttachmentClearRegion
{
	/**
	 * @brief The x coordinate.
	 */
	uint32_t x;

	/**
	 * @brief The y coordinate.
	 */
	uint32_t y;

	/**
	 * @brief The width.
	 */
	uint32_t width;

	/**
	 * @brief The height.
	 */
	uint32_t height;

	/**
	 * @brief The layer to start clearing from.
	 *
	 * This is either the depth, array layer, or face.
	 */
	uint32_t layer;

	/**
	 * @brief The number of layers to clear.
	 */
	uint32_t layerCount;
} dsAttachmentClearRegion;

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
	 * @brief The first index to draw.
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
	 * @brief Access bitmask for what to wait on before the barrier.
	 */
	dsGfxAccess beforeAccess;

	/**
	 * @brief Access bitmask for what to delay for after the barrier.
	 */
	dsGfxAccess afterAccess;
} dsGfxMemoryBarrier;

/**
 * @brief Struct containing the planes for an orthographic or frustum projection matrix.
 * @see dsProjectionParams
 * @see ProjectionParams.h
 */
typedef struct dsProjectionPlanes
{
	/**
	 * @brief The left plane.
	 */
	float left;

	/**
	 * @brief The right plane.
	 */
	float right;

	/**
	 * @brief The bottom plane.
	 */
	float bottom;

	/**
	 * @brief The top plane.
	 */
	float top;
} dsProjectionPlanes;

/**
 * @brief Struct containing the parameters of a perspective projection matrix.
 * @see dsProjectionParams
 * @see ProjectionParams.h
 */
typedef struct dsPerspectiveParams
{
	/**
	 * @brief he field of view in the Y direction in radians.
	 */
	float fovy;

	/**
	 * @brief The aspect ratio as X/Y.
	 */
	float aspect;
} dsPerspectiveParams;

/**
 * @brief Struct containing the parameters of a projection matrix.
 * @see ProjectionParams.h
 */
typedef struct dsProjectionParams
{
	/**
	 * @brief The type of the projection.
	 */
	dsProjectionType type;

	union
	{
		/**
		 * @brief The planes used for orthographic and frustum projection types.
		 */
		dsProjectionPlanes projectionPlanes;

		/**
		 * @brief Parameters for the perspective projection type.
		 */
		dsPerspectiveParams perspectiveParams;
	};

	/**
	 * @brief The near plane.
	 */
	float near;

	/**
	 * @brief The far plane.
	 *
	 * This may be INFINITY.
	 */
	float far;
} dsProjectionParams;

/// @cond Doxygen_Suppress
typedef struct dsGPUProfileContext dsGPUProfileContext;
typedef struct dsResourceCommandBuffers dsResourceCommandBuffers;
/// @endcond

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
 * @param name The name of the render surface, used for profiling info. Implementations should
 *     copy the string and store it with the render surface.
 * @param osHandle The OS handle, such as window handle.
 * @param type The type of the render surface.
 * @param usage Flags to determine how the render surface will be used.
 * @param widthHint Hint for the width of the surface.
 * @param heightHint Hint for the height of the surface.
 * @return The created render surface, or NULL if it couldn't be created.
 */
typedef dsRenderSurface* (*dsCreateRenderSurfaceFunction)(dsRenderer* renderer,
	dsAllocator* allocator, const char* name, void* displayHandle, void* osHandle,
	dsRenderSurfaceType type, dsRenderSurfaceUsage usage, unsigned int widthHint,
	unsigned int heightHint);

/**
 * @brief Function for destroying a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param renderSurface The render surface to destroy
 * @return False if the render surface couldn't be destroyed.
 */
typedef bool (*dsDestroyRenderSurfaceFunction)(
	dsRenderer* renderer, dsRenderSurface* renderSurface);

/**
 * @brief Function for updating a render surface.
 * @param renderer The renderer the render surface is used with.
 * @param renderSurface The render surface to update.
 * @param widthHint Hint for the width of the surface.
 * @param heightHint Hint for the height of the surface.
 * @return True if the render surface size changed.
 */
typedef bool (*dsUpdateRenderSurfaceFunction)(dsRenderer* renderer, dsRenderSurface* renderSurface,
	unsigned int widthHint, unsigned int heightHint);

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
 * @param renderSurfaces The render surfaces to swap the buffers on.
 * @param count The number of render surfaces to swap.
 * @return False if the buffers couldn't be swapped.
 */
typedef bool (*dsSwapRenderSurfaceBuffersFunction)(dsRenderer* renderer,
	dsRenderSurface** renderSurfaces, uint32_t count);

/**
 * @brief Function for creating a command buffer pool.
 * @param renderer The renderer the command buffers will be used wtih.
 * @param allocator The allocator to create the command buffer pool.
 * @param usage The usage flags. Set to 0 if none of the usage options are needed.
 * @return The command buffer pool.
 */
typedef dsCommandBufferPool* (*dsCreateCommandBufferPoolFunction)(dsRenderer* renderer,
	dsAllocator* allocator, dsCommandBufferUsage usage);

/**
 * @brief Function for destroying a command buffer pool.
 * @param renderer The renderer the command buffer pool was created with.
 * @param pool The command buffer pool to destroy.
 * @return False if the command buffer pool couldn't be destroyed.
 */
typedef bool (*dsDestroyCommandBufferPoolFunction)(dsRenderer* renderer, dsCommandBufferPool* pool);

/**
 * @brief Function for creating multiple command buffers from a command buffer pool.
 * @param renderer The renderer the command buffer pool was created with.
 * @param pool The command buffer pool create the command buffers with.
 * @param count The number of command buffers to create
 * @return False if the command buffers couldn't be created..
 */
typedef bool (*dsCommandBufferPoolCreateCommandBuffersFunction)(dsRenderer* renderer,
	dsCommandBufferPool* pool, uint32_t count);

/**
 * @brief Function for resetting a command buffer pool, preparing the command buffers to be built up
 *     with new render commands.
 * @param renderer The renderer the command buffer pool was created with.
 * @param pool The command buffer pool to reset.
 * @return False if the command buffer pool couldn't be reset.
 */
typedef bool (*dsResetCommandBufferPoolFunction)(dsRenderer* renderer, dsCommandBufferPool* pool);

/**
 * @brief Function for starting to draw to a command buffer.
 * @param renderer The renderer that the command buffer will be drawn with.
 * @param commandBuffer The command buffer to begin.
 * @return False if the command buffer couldn't be begun.
 */
typedef bool (*dsBeginCommandBufferFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer);

/**
 * @brief Function for starting to draw to a secondary command buffer.
 * @param renderer The renderer that the command buffer will be drawn with.
 * @param commandBuffer The command buffer to begin.
 * @param framebuffer The framebuffer being drawn to.
 * @param renderPass The render pass being drawn to.
 * @param subpass The subpass within the render pass being drawn to.
 * @param viewport The viewport to render to.
 * @param parentOcclusionQueryState The expected state of the occlusion query for the primary
 *     command buffer this will be submitted to.
 * @return False if the command buffer couldn't be begun.
 */
typedef bool (*dsBeginSecondaryCommandBufferFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer,
	const dsRenderPass* renderPass, uint32_t subpass, const dsAlignedBox3f* viewport,
	dsGfxOcclusionQueryState parentOcclusionQueryState);

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
 * than just copying the pointers. This also includes the strings stored inside of the subpasses.
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
 * @param secondary True if secondary command buffers will be used for render commands, false if
 *     render commands will be directly sent on the same command buffer as the render pass.
 * @return False if the render pass couldn't be begun.
 */
typedef bool (*dsBeginRenderPassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount, bool secondary);

/**
 * @brief Function for advancing to the next subpass within a render pass.
 * @param renderer The renderer the render pass was created with.
 * @param commandBuffer The command buffer to push the commands on.
 * @param renderPass The render pass to continue.
 * @param index The index of the subpass.
 * @param secondary True if secondary command buffers will be used for render commands, false if
 *     render commands will be directly sent on the same command buffer as the render pass.
 * @return False if the render pass couldn't be advanced.
 */
typedef bool (*dsNextRenderSubpassFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index, bool secondary);

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
 * @brief Function for setting the number of anti-alias samples.
 *
 * This should set the default value on the renderer on success. The implementation is responsible
 * for making any necessary changse to the render passes when the attachment info is set to
 * DS_SURFACE_ANTIALIAS_SAMPLES or DS_DEFAULT_ANTIALIAS_SAMPLES. The caller is responsible for
 * re-creating any render surfaces, offscreens, renderbuffers, and framebuffers.
 *
 * @param renderer The renderer.
 * @param samples The number of anti-alias samples.
 * @return False if the number of samples couldn't be set.
 */
typedef bool (*dsSetRenderSamplesFunction)(dsRenderer* renderer, uint32_t samples);

/**
 * @brief Function for setting how to wait for vsync.
 * @param renderer The renderer.
 * @param vsync How to wait for vsync.
 * @return False if the vsync couldn't be set.
 */
typedef bool (*dsSetRenderVSyncFunction)(dsRenderer* renderer, dsVSync vsync);

/**
 * @brief Function for setting the default anisotropy for anisotropic filtering.
 * @param renderer The renderer.
 * @param anisotropy The default anisotropy.
 * @return False if the anisotropy couldn't be set.
 */
typedef bool (*dsSetRenderDefaultAnisotropyFunction)(dsRenderer* renderer, float anisotropy);

/**
 * @brief Function for setting the viewport.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the command on.
 * @param viewport The viewport to draw to.
 * @return False if the viewport couldn't be set.
 */
typedef bool (*dsRenderSetViewportFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport);

/**
 * @brief Function for clearing attachments.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the clear command on.
 * @param attachments The attachments to clear with their values.
 * @param attachmentCount The number of attachments to clear.
 * @param regions The regions to clear.
 * @param regionCount The number of regions to clear.
 * @return False if the attachments couldn't be cleared.
 */
typedef bool (*dsRenderClearAttachmentsFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount);

/**
 * @brief Function for drawing vertex geometry with the currently bound shader.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @param primitiveType The type of primitive to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType);

/**
 * @brief Function for drawing indexed geometry with the currently bound shader.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the draw command on.
 * @param geometry The geometry to draw.
 * @param drawRange The range of vertices to draw.
 * @param primitiveType The type of primitive to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawIndexedFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType);

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
 * @param primitiveType The type of primitive to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawIndirectFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);

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
 * @param primitiveType The type of primitive to draw.
 * @return False if the geometry couldn't be drawn.
 */
typedef bool (*dsRenderDrawIndexedIndirectFunction)(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsGfxBuffer* indirectBuffer, size_t offset, uint32_t count, uint32_t stride,
	dsPrimitiveType primitiveType);

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
	void* dstSurface, const dsSurfaceBlitRegion* regions, uint32_t regionCount,
	dsBlitFilter filter);

/**
 * @brief Function for adding a memory barrier.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to place the barrier on.
 * @param beforeStages The stages to wait on before the barrier.
 * @param afterStages The stages to wait for after the barrier.
 * @param barriers List of write/read dependencies to place barriers for.
 * @param barrierCount The number of barriers.
 * @return False if the barrier couldn't be added.
 */
typedef bool (*dsGfxMemoryBarrierFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxPipelineStage beforeStages, dsGfxPipelineStage afterStages,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount);

/**
 * @brief Function for pushing a debug group.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to push the debug group on.
 * @param name The name of the debug group.
 * @return False if the debug group couldn't be pushed.
 */
typedef bool (*dsPushRenderDebugGroupFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const char* name);

/**
 * @brief Function for popping a debug group.
 * @param renderer The renderer.
 * @param commandBuffer The command buffer to pop the debug group on.
 * @return False if the debug group couldn't be pushed.
 */
typedef bool (*dsPopRenderDebugGroupFunction)(dsRenderer* renderer, dsCommandBuffer* commandBuffer);

/**
 * @brief Function for flushing queued commands to the GPU.
 * @param renderer The renderer.
 * @return False if the renderer is in an invalid state.
 */
typedef bool (*dsRenderFlushFunction)(dsRenderer* renderer);

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
	 * @brief The graphics windowing platform.
	 */
	dsGfxPlatform platform;

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
	 * @brief The name of the GPU and driver.
	 *
	 * This should never be NULL.
	 */
	const char* deviceName;

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
	 * @brief The ID of the GPU and driver.
	 *
	 * This may be 0 if not known.
	 */
	uint32_t deviceID;

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
	 * @brief The number of samples for multisampling in offscreens and renderbuffers.
	 */
	uint32_t defaultSamples;

	/**
	 * @brief The default level of anisotropy for anisotropic filtering.
	 */
	float defaultAnisotropy;

	/**
	 * @brief The maximum size of the workgroup in each dimension.
	 *
	 * If 0, compute shaders aren't supported.
	 */
	uint32_t maxComputeWorkGroupSize[3];

	/**
	 * @brief The options used when creating a projection matrix.
	 */
	dsProjectionMatrixOptions projectionOptions;

	/**
	 * @brief How to wait for vsync when
	 */
	dsVSync vsync;

	/**
	 * @brief True to avoid using a back buffer for rendering if possible.
	 */
	bool singleBuffer;

	/**
	 * @brief True if render surfaces are stereoscopic.
	 */
	bool stereoscopic;

	/**
	 * @brief True if geometry shaders are supported.
	 */
	bool hasGeometryShaders;

	/**
	 * @brief True if tessellation shaders are supported.
	 */
	bool hasTessellationShaders;

	/**
	 * @brief True if indirect draws with a count > 1 will use a single graphics API call.
	 */
	bool hasNativeMultidraw;

	/**
	 * @brief Whether or not instanced drawing is supported.
	 */
	bool hasInstancedDrawing;

	/**
	 * @brief Whether or not the first instance can be set.
	 */
	bool hasStartInstance;

	/**
	 * @brief Whether or not each attachment can have the blend set independently.
	 */
	bool hasIndependentBlend;

	/**
	 * @brief Whether or not dual source blend is supported.
	 */
	bool hasDualSrcBlend;

	/**
	 * @brief Whether or not logical framebuffer operations are supported.
	 */
	bool hasLogicOps;

	/**
	 * @brief Whether or not shading per multisample is supported.
	 */
	bool hasSampleShading;

	/**
	 * @brief Whether or not custom depth bounds are supported.
	 */
	bool hasDepthBounds;

	/**
	 * @brief Whether or not depth clamping is supported.
	 */
	bool hasDepthClamp;

	/**
	 * @brief Whether or not depth bias clamping is supported.
	 */
	bool hasDepthBiasClamp;

	/**
	 * @brief Whether or not depth/stencil surfaces can be resolved.
	 */
	bool hasDepthStencilMultisampleResolve;

	/**
	 * @brief Whether or not fragment inputs are supported to read directly from another fragment
	 *     shader invocation.
	 */
	bool hasFragmentInputs;

	/**
	 * @brief Whether or not projected texture coordinates have an inverted T coordinate.
	 */
	bool projectedTexCoordTInverted;

	/**
	 * @brief Whether or not render passes require strictly all commands in separate command buffers
	 *     when using secondary command buffers.
	 *
	 * Example commands include debug groups and GPU timers.
	 */
	bool strictRenderPassSecondaryCommands;

	// ----------------------------- Internals and function table ----------------------------------

	/**
	 * @brief Context used internally for GPU profiling.
	 */
	dsGPUProfileContext* _profileContext;

	/**
	 * @brief Command buffers used for resource management..
	 */
	dsResourceCommandBuffers* _resourceCommandBuffers;

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
	 * @brief Command buffer creation function.
	 */
	dsCommandBufferPoolCreateCommandBuffersFunction createCommandBuffersFunc;

	/**
	 * @brief Command buffer pool reset function.
	 */
	dsResetCommandBufferPoolFunction resetCommandBufferPoolFunc;

	/**
	 * @brief Command buffer begin function.
	 */
	dsBeginCommandBufferFunction beginCommandBufferFunc;

	/**
	 * @brief Secondary command buffer begin function.
	 */
	dsBeginSecondaryCommandBufferFunction beginSecondaryCommandBufferFunc;

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
	dsSetRenderSamplesFunction setSurfaceSamplesFunc;

	/**
	 * @brief Default anti-alias sample set function.
	 */
	dsSetRenderSamplesFunction setDefaultSamplesFunc;

	/**
	 * @brief Vsync set function.
	 */
	dsSetRenderVSyncFunction setVSyncFunc;

	/**
	 * @brief Default anisotropy set function.
	 */
	dsSetRenderDefaultAnisotropyFunction setDefaultAnisotropyFunc;

	/**
	 * @brief Viewport set function.
	 */
	dsRenderSetViewportFunction setViewportFunc;

	/**
	 * @brief Attachment clear function.
	 */
	dsRenderClearAttachmentsFunction clearAttachmentsFunc;

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
	 * @brief Debug group push function.
	 */
	dsPushRenderDebugGroupFunction pushDebugGroupFunc;

	/**
	 * @brief Debug group pop function.
	 */
	dsPopRenderDebugGroupFunction popDebugGroupFunc;

	/**
	 * @brief Flush function.
	 */
	dsRenderFlushFunction flushFunc;

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

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsAttachmentUsage);
DS_ENUM_BITMASK_OPERATORS(dsRenderSurfaceUsage);
DS_ENUM_BITMASK_OPERATORS(dsCommandBufferUsage);
DS_ENUM_BITMASK_OPERATORS(dsGfxPipelineStage);
DS_ENUM_BITMASK_OPERATORS(dsGfxAccess);
/// @endcond
