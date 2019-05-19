/*
 * Copyright 2016-2019 Aaron Barany
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
#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/ShaderTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types for graphics resources used in the DeepSea/Render library.
 */

/**
 * @brief Constant for mapping the full buffer.
 */
#define DS_MAP_FULL_BUFFER (size_t)-1

/**
 * @brief Constant for using all mip levels.
 */
#define DS_ALL_MIP_LEVELS (uint32_t)-1

/**
 * @brief Constant for the maximum number of allowed vertex attributes.
 */
#define DS_MAX_ALLOWED_VERTEX_ATTRIBS 32

/**
 * @brief Constant for the maximum number of vertex buffers in a dsDrawGeometry instance.
 */
#define DS_MAX_GEOMETRY_VERTEX_BUFFERS 4

/**
 * @brief Flags used as hints for how graphics memory will be used.
 */
typedef enum dsGfxMemory
{
	dsGfxMemory_GPUOnly = 0x001,    ///< The memory will only ever be accessed by the GPU.
	dsGfxMemory_Static = 0x002,     ///< The memory will never be modified from the CPU.
	dsGfxMemory_Dynamic = 0x004,    ///< The memory will be modified on the CPU occasionally.
	dsGfxMemory_Stream = 0x008,     ///< The memory will be modified on the CPU constantly.
	dsGfxMemory_Draw = 0x010,       ///< The memory will be used by draw commands.
	dsGfxMemory_Read = 0x020,       ///< The memory will be read back from the GPU.
	dsGfxMemory_Persistent = 0x040, ///< The memory will be mapped mersistently across frames.
	dsGfxMemory_Coherent = 0x080,   ///< The memory should remain coherent to avoid manual flushing.
	dsGfxMemory_Synchronize = 0x100 ///< Wait for the memory to not be in use when mapping.
} dsGfxMemory;

/**
 * @brief Enum for how a graphics buffer will be used.
 *
 * These are bitmask values, allowing a buffer to be used for multiple purposes.
 * @see GfxBuffer.h
 */
typedef enum dsGfxBufferUsage
{
	dsGfxBufferUsage_Index = 0x001,            ///< Index buffer.
	dsGfxBufferUsage_Vertex  = 0x002,          ///< Vertex buffer.
	dsGfxBufferUsage_IndirectDraw = 0x004,     ///< Indirect draw information.
	dsGfxBufferUsage_IndirectDispatch = 0x008, ///< Indirect dispatch information.
	dsGfxBufferUsage_UniformBlock = 0x010,     ///< Shader uniform block.
	dsGfxBufferUsage_UniformBuffer = 0x020,    ///< Shader uniform buffer, modifiable by the shader.
	dsGfxBufferUsage_Texture = 0x040,          ///< Shader texture buffer.
	dsGfxBufferUsage_Image = 0x080,            ///< Shader image buffer that can be stored to.
	dsGfxBufferUsage_CopyFrom = 0x100,         ///< Source for GPU copy operations.
	dsGfxBufferUsage_CopyTo = 0x200            ///< Destination for GPU and CPU copy operations.
} dsGfxBufferUsage;

/**
 * @brief Enum for how a texture will be used.
 *
 * These are bitmask values, allowing a texture to be used for multiple purposes.
 * @see Texture.h
 */
typedef enum dsTextureUsage
{
	dsTextureUsage_Texture = 0x1,      ///< Use as a sampled texture.
	dsTextureUsage_Image = 0x2,        ///< Use as an image without a sampler.
	dsTextureUsage_SubpassInput = 0x4, ///< Use for passing image data from one subpass to another.
	dsTextureUsage_CopyFrom = 0x8,     ///< Source for GPU copy operations.
	dsTextureUsage_CopyTo = 0x10,      ///< Destination for GPU and CPU copy operations.
	/**
	 * When used as an offscreen, rendering may continue across multiple passes or subpasses. When
	 * not set, some offscreen contents may be discarded after rendering depending on the underlying
	 * hardware and other usage flags to save memory.
	 */
	dsTextureUsage_OffscreenContinue = 0x20
} dsTextureUsage;

/**
 * @brief Enum for how a renderbuffer will be used.
 *
 * These are bitmask values, allowing a renderbuffer to be used for multiple purposes.
 * @see Renderbuffer.h
 */
typedef enum dsRenderbufferUsage
{
	dsRenderbufferUsage_Standard = 0x0, ///< Standard usage.
	dsRenderbufferUsage_BlitFrom = 0x1, ///< Can blit from the renderbuffer to another surface.
	dsRenderbufferUsage_BlitTo = 0x2,   ///< Can blit from another surface to the renderbuffer.
	/**
	 * Can be cleared explicitly with dsRenderer_clearColorSurface() or
	 * dsRenderer_clearDepthStencilSurface(). Clearing as part of a render pass is still allowed.
	 */
	dsRenderbufferUsage_Clear = 0x4,
	/**
	 * When used as an offscreen, rendering may continue across multiple passes or subpasses. When
	 * not set, the contents may be discarded or never stored in the first place.
	 */
	dsRenderbufferUsage_Continue = 0x8
} dsRenderbufferUsage;

/**
 * @brief Flags for how to map a graphics buffer to memory.
 * @see GfxBuffer.h
 */
typedef enum dsGfxBufferMap
{
	dsGfxBufferMap_Read = 0x1,       ///< Read data from the buffer.
	dsGfxBufferMap_Write = 0x2,      ///< Write data to the buffer.
	dsGfxBufferMap_Orphan = 0x4,     ///< Orphan the contents of the buffer to replace the data.
	dsGfxBufferMap_Persistent = 0x8  ///< Allow the buffer to remain locked.
} dsGfxBufferMap;

/**
 * @brief Enum for what kind of mapping is supported on the system.
 *
 * Each level assumes that the features of the previous enum values are also supported.
 */
typedef enum dsGfxBufferMapSupport
{
	dsGfxBufferMapSupport_None,      ///< Mapping of buffers isn't supported.
	dsGfxBufferMapSupport_Full,      ///< May only map the full buffer. The library will offset
	                                 ///  into the buffer to simulate mapping ranges.
	dsGfxBufferMapSupport_Range,     ///< May arbitrary ranges of buffers.
	dsGfxBufferMapSupport_Persistent ///< Buffers may be persistently locked.
} dsGfxBufferMapSupport;

/**
 * @brief Enum for formats used for vertex buffers and images.
 *
 * There are multiple sections of this enum, which are mutually exclusive:
 * - Standard formats. These require a decorator.
 * - Special formats. These may not be used with any decorators.
 * - Compressed formats. These are compressed blocks of memory, and require a decorator.
 * - Decorators. These are OR'd with standard and compressed formats to determine the final format.
 *
 * @see GfxFormat.h
 */
typedef enum dsGfxFormat
{
	dsGfxFormat_Unknown = 0, ///< No known format.

	// Standard formats
	dsGfxFormat_R4G4 = 1,            ///< RG 4 bits each.
	dsGfxFormat_R4G4B4A4,            ///< RGBA 4 bits each.
	dsGfxFormat_B4G4R4A4,            ///< BGRA 4 bits each.
	dsGfxFormat_A4R4G4B4,            ///< ARGB 4 bits each.
	dsGfxFormat_R5G6B5,              ///< RGB with 5, 6, 5 bits.
	dsGfxFormat_B5G6R5,              ///< BGR with 5, 6, 5 bits.
	dsGfxFormat_R5G5B5A1,            ///< RGBA with 5, 5, 5, 1 bits.
	dsGfxFormat_B5G5R5A1,            ///< BGRA with 5, 5, 5, 1 bits.
	dsGfxFormat_A1R5G5B5,            ///< ARGB with 1, 5, 5, 5 bits.
	dsGfxFormat_R8,                  ///< R with 8 bits.
	dsGfxFormat_R8G8,                ///< RG with 8 bits each.
	dsGfxFormat_R8G8B8,              ///< RGB with 8 bits each.
	dsGfxFormat_B8G8R8,              ///< BGR with 8 bits each.
	dsGfxFormat_R8G8B8A8,            ///< RGBA with 8 bits each.
	dsGfxFormat_B8G8R8A8,            ///< BGRA with 8 bits each.
	dsGfxFormat_A8B8G8R8,            ///< ABGR with 8 bits each.
	dsGfxFormat_A2R10G10B10,         ///< ARGB with 2, 10, 10, 10 bits.
	dsGfxFormat_A2B10G10R10,         ///< ABGR with 2, 10, 10, 10 bits.
	dsGfxFormat_R16,                 ///< R with 16 bits.
	dsGfxFormat_R16G16,              ///< RG wtih 16 bits each.
	dsGfxFormat_R16G16B16,           ///< RGB with 16 bits each.
	dsGfxFormat_R16G16B16A16,        ///< RGBA with 16 bits each.
	dsGfxFormat_R32,                 ///< R with 32 bits.
	dsGfxFormat_R32G32,              ///< RG with 32 bits each.
	dsGfxFormat_R32G32B32,           ///< RGB with 32 bits each.
	dsGfxFormat_R32G32B32A32,        ///< RGBA with 32 bits each.
	dsGfxFormat_R64,                 ///< R with 64 bits.
	dsGfxFormat_R64G64,              ///< RG with 64 bits each.
	dsGfxFormat_R64G64B64,           ///< RGB with 64 bits each.
	dsGfxFormat_R64G64B64A64,        ///< RGBA with 64 bits each.
	dsGfxFormat_StandardCount,       ///< The number of standard formats.
	dsGfxFormat_StandardMask = 0xFF, ///< Bitmask for standard formats.

	// Aliases for standard formats.
	dsGfxFormat_X8 = dsGfxFormat_R8,                     ///< X with 8 bits.
	dsGfxFormat_X8Y8 = dsGfxFormat_R8G8,                 ///< XY with 8 bits each.
	dsGfxFormat_X8Y8Z8 = dsGfxFormat_R8G8B8,             ///< XYZ with 8 bits each.
	dsGfxFormat_X8Y8Z8W8 = dsGfxFormat_R8G8B8A8,         ///< XYZW with 8 bits each.
	dsGfxFormat_W2X10Y10Z10 = dsGfxFormat_A2R10G10B10,   ///< WXYZ with 2, 10, 10, 10 bits.
	dsGfxFormat_W2Z10Y10X10 = dsGfxFormat_A2B10G10R10,   ///< WZYX with 2, 10, 10, 10 bits.
	dsGfxFormat_X16 = dsGfxFormat_R16,                   ///< X with 16 bits.
	dsGfxFormat_X16Y16 = dsGfxFormat_R16G16,             ///< XY with 16 bits each.
	dsGfxFormat_X16Y16Z16 = dsGfxFormat_R16G16B16,       ///< XYZ with 16 bits each.
	dsGfxFormat_X16Y16Z16W16 = dsGfxFormat_R16G16B16A16, ///< XYZW with 16 bits each.
	dsGfxFormat_X32 = dsGfxFormat_R32,                   ///< X with 32 bits.
	dsGfxFormat_X32Y32 = dsGfxFormat_R32G32,             ///< XY with 32 bits each.
	dsGfxFormat_X32Y32Z32 = dsGfxFormat_R32G32B32,       ///< XYZ with 32 bits each.
	dsGfxFormat_X32Y32Z32W32 = dsGfxFormat_R32G32B32A32, ///< XYZW with 32 bits each.
	dsGfxFormat_X64 = dsGfxFormat_R64,                   ///< X with 64 bits.
	dsGfxFormat_X64Y64 = dsGfxFormat_R64G64,             ///< XY with 64 bits each.
	dsGfxFormat_X64Y64Z64 = dsGfxFormat_R64G64B64,       ///< XYZ with 64 bits each.
	dsGfxFormat_X64Y64Z64W64 = dsGfxFormat_R64G64B64A64, ///< XYZW with 64 bits each.

	// Special formats.
	dsGfxFormat_B10G11R11_UFloat = 0x100, ///< BGR with 10, 11, 11 bits as unsigned floats.
	dsGfxFormat_E5B9G9R9_UFloat = 0x200,  ///< BGR with 9 bits each as unsigned floats with 5 bits
	                                      ///< shared exponent.
	dsGfxFormat_D16 = 0x300,              ///< Depth with 16 bits.
	dsGfxFormat_X8D24 = 0x400,            ///< Depth with 24 bits and padding.
	dsGfxFormat_D32_Float = 0x500,        ///< Depth with 32 bits as a float.
	dsGfxFormat_S8 = 0x600,               ///< Stencil with 8 bits.
	dsGfxFormat_D16S8 = 0x700,            ///< Depth stencil with 16, 8 bits.
	dsGfxFormat_D24S8 = 0x800,            ///< Depth stencil with 24, 8 bits.
	dsGfxFormat_D32S8_Float = 0x900,      ///< Depth stencil with 32, 8 bits. Depth is float.
	dsGfxFormat_SpecialCount = 0xA,       ///< The number of special formats.
	dsGfxFormat_SpecialMask = 0xF00,      ///< Bitmask for special formats.

	// Aliases for special formats.

	/** ZYX with 10, 11, 11 bits as unsigned floats. */
	dsGfxFormat_Z10Y11X11_UFloat = dsGfxFormat_B10G11R11_UFloat,
	/** ZYX with 9 bits each as unsigned floats with 5 bits shared exponent */
	dsGfxFormat_E5Z9Y9X9_UFloat = dsGfxFormat_E5B9G9R9_UFloat,

	// Compressed formats.
	dsGfxFormat_BC1_RGB = 0x01000,          ///< S3TC BC1 format (DXT1) with RGB.
	dsGfxFormat_BC1_RGBA = 0x02000,         ///< S3TC BC1 format (DXT1) with RGBA with 1 bit alpha.
	dsGfxFormat_BC2 = 0x03000,              ///< S3TC BC2 format (DXT2/3).
	dsGfxFormat_BC3 = 0x04000,              ///< S3TC BC3 format (DXT4/5).
	dsGfxFormat_BC4 = 0x05000,              ///< S3TC BC4 format.
	dsGfxFormat_BC5 = 0x06000,              ///< S3TC BC5 format.
	dsGfxFormat_BC6H = 0x07000,             ///< S3TC BC6H format.
	dsGfxFormat_BC7 = 0x08000,              ///< S3TC BC7 format.
	dsGfxFormat_ETC1 = 0x09000,             ///< ETC1 format.
	dsGfxFormat_ETC2_R8G8B8 = 0x0A000,      ///< ETC2 format with RGB with 8 bits each.
	dsGfxFormat_ETC2_R8G8B8A1 = 0x0B000,    ///< ETC2 format with RGBA with 8, 8, 8, 1 bits.
	dsGfxFormat_ETC2_R8G8B8A8 = 0x0C000,    ///< ETC2 format with RGBA with 8 bits each.
	dsGfxFormat_EAC_R11 = 0x0D000,          ///< EAC format with R with 11 bits.
	dsGfxFormat_EAC_R11G11 = 0x0E000,       ///< EAC format with RG with 11 bits each.
	dsGfxFormat_ASTC_4x4 = 0x0F000,         ///< ASTC with a 4x4 block.
	dsGfxFormat_ASTC_5x4 = 0x10000,         ///< ASTC with a 5x4 block.
	dsGfxFormat_ASTC_5x5 = 0x11000,         ///< ASTC with a 5x5 block.
	dsGfxFormat_ASTC_6x5 = 0x12000,         ///< ASTC with a 6x5 block.
	dsGfxFormat_ASTC_6x6 = 0x13000,         ///< ASTC with a 6x6 block.
	dsGfxFormat_ASTC_8x5 = 0x14000,         ///< ASTC with a 8x5 block.
	dsGfxFormat_ASTC_8x6 = 0x15000,         ///< ASTC with a 8x6 block.
	dsGfxFormat_ASTC_8x8 = 0x16000,         ///< ASTC with a 8x8 block.
	dsGfxFormat_ASTC_10x5 = 0x17000,        ///< ASTC with a 10x5 block.
	dsGfxFormat_ASTC_10x6 = 0x18000,        ///< ASTC with a 10x6 block.
	dsGfxFormat_ASTC_10x8 = 0x19000,        ///< ASTC with a 10x8 block.
	dsGfxFormat_ASTC_10x10 = 0x1A000,       ///< ASTC with a 10x10 block.
	dsGfxFormat_ASTC_12x10 = 0x1B000,       ///< ASTC with a 12x10 block.
	dsGfxFormat_ASTC_12x12 = 0x1C000,       ///< ASTC with a 12x12 block.
	dsGfxFormat_PVRTC1_RGB_2BPP = 0x1D000,  ///< PVRTC1 with RGB with 2 bits per pixel.
	dsGfxFormat_PVRTC1_RGBA_2BPP = 0x1E000, ///< PVRTC1 with RGBA with 2 bits per pixel.
	dsGfxFormat_PVRTC1_RGB_4BPP = 0x1F000,  ///< PVRTC1 with RGB with 4 bits per pixel.
	dsGfxFormat_PVRTC1_RGBA_4BPP = 0x20000, ///< PVRTC1 with RGBA with 4 bits per pixel.
	dsGfxFormat_PVRTC2_RGBA_2BPP = 0x21000, ///< PVRTC2 with RGBA with 2 bits per pixel.
	dsGfxFormat_PVRTC2_RGBA_4BPP = 0x22000, ///< PVRTC2 with RGBA with 4 bits per pixel.
	dsGfxFormat_CompressedCount = 0x23,     ///< The number of compressed formats.
	dsGfxFormat_CompressedMask = 0xFF000,   ///< Bitmask for compressed formats.

	// Decorators
	dsGfxFormat_UNorm = 0x100000,        ///< Integer converted to a float in the range [0, 1].
	dsGfxFormat_SNorm = 0x200000,        ///< Integer converted to a float in the range [-1, 1].
	dsGfxFormat_UScaled = 0x300000,      ///< Unsigned integer converted to a float.
	dsGfxFormat_SScaled = 0x400000,      ///< Signed integer converted to a float.
	dsGfxFormat_UInt = 0x500000,         ///< Unsigned integer.
	dsGfxFormat_SInt = 0x600000,         ///< Signed integer.
	dsGfxFormat_Float = 0x700000,        ///< Signed floating point.
	dsGfxFormat_UFloat = 0x800000,       ///< Unsigned floating point.
	dsGfxFormat_SRGB = 0x900000,         ///< RGB encoded in gamma space.
	dsGfxFormat_DecoratorCount = 0xA,    ///< The number of decorators.
	dsGfxFormat_DecoratorMask = 0xF00000 ///< Bitmask for decorators.
} dsGfxFormat;

/**
 * @brief Enum for the dimension of a texture.
 * @see Textrue.h
 */
typedef enum dsTextureDim
{
	dsTextureDim_1D,  ///< 1-dimensional
	dsTextureDim_2D,  ///< 2-dimensional
	dsTextureDim_3D,  ///< 3-dimensional
	dsTextureDim_Cube ///< Cube map
} dsTextureDim;

/**
 * @brief Enum for the face of a cubemap.
 * @see Texture.h
 */
typedef enum dsCubeFace
{
	dsCubeFace_PosX, ///< +X
	dsCubeFace_NegX, ///< -X
	dsCubeFace_PosY, ///< +Y
	dsCubeFace_NegY, ///< -Y
	dsCubeFace_PosZ, ///< +Z
	dsCubeFace_NegZ, ///< -Z

	dsCubeFace_None = 0 ///< No cube face.
} dsCubeFace;

/**
 * @brief Enum for the filter to use when blitting.
 * @see Texture.h
 */
typedef enum dsBlitFilter
{
	dsBlitFilter_Nearest, ///< Nearest-neighbor
	dsBlitFilter_Linear   ///< Linear scaling
} dsBlitFilter;

/**
 * @brief Enum for named vertex attributes.
 *
 * These are mainly suggestions rather than a requirement to make it easier to match vertex
 * attributes between code and shaders.
 *
 * @see VertexFormat.h
 */
typedef enum dsVertexAttrib
{
	dsVertexAttrib_Position,                            ///< Vertex position.
	dsVertexAttrib_Position0 = dsVertexAttrib_Position, ///< Primary vertex position.
	dsVertexAttrib_Position1,                           ///< Secondary vertex position.
	dsVertexAttrib_Normal,                              ///< Vertex normal.
	dsVertexAttrib_Color,                               ///< Vertex color.
	dsVertexAttrib_Color0 = dsVertexAttrib_Color,       ///< Primary vertex color.
	dsVertexAttrib_Color1,                              ///< Secondary vertex color.
	dsVertexAttrib_FogCoord,                            ///< Fog coordinate.
	dsVertexAttrib_Tangent,                             ///< Vertex tangent.
	dsVertexAttrib_Bitangent,                           ///< Verex bitangent. (also called binormal)
	dsVertexAttrib_TexCoord0,                           ///< First texture coordinate.
	dsVertexAttrib_TexCoord1,                           ///< Second texture coordinate.
	dsVertexAttrib_TexCoord2,                           ///< Third texture coordinate.
	dsVertexAttrib_TexCoord3,                           ///< Fourth texture coordinate.
	dsVertexAttrib_TexCoord4,                           ///< Fifth texture coordinate.
	dsVertexAttrib_TexCoord5,                           ///< Sixth texture coordinate.
	dsVertexAttrib_TexCoord6,                           ///< Seventh texture coordinate.
	dsVertexAttrib_TexCoord7                            ///< Eighth texture coordinate.
} dsVertexAttrib;

/**
 * @brief Enum for the type of a graphics surface used within a framebuffer or blitting.
 * @see Framebuffer.h
 */
typedef enum dsGfxSurfaceType
{
	dsGfxSurfaceType_ColorRenderSurface,      ///< The color portion of a render surface.
	/// The left color portion of a render surface.
	dsGfxSurfaceType_ColorRenderSurfaceLeft,
	/// The right color portion of a render surface.
	dsGfxSurfaceType_ColorRenderSurfaceRight,
	/// The depth/stencil portion of a render surface.
	dsGfxSurfaceType_DepthRenderSurface,
	/// The left depth/stencil portion of a render surface.
	dsGfxSurfaceType_DepthRenderSurfaceLeft,
	/// The right depth/stencil portion of a render surface.
	dsGfxSurfaceType_DepthRenderSurfaceRight,
	dsGfxSurfaceType_Offscreen,               ///< Offscreen texture.
	dsGfxSurfaceType_Renderbuffer             ///< Color or depth renderbuffer.
} dsGfxSurfaceType;

/**
 * @brief Enum for the result of waiting for a fence.
 */
typedef enum dsGfxFenceResult
{
	dsGfxFenceResult_Success,        ///< The fence has finishd on the GPU side.
	dsGfxFenceResult_Timeout,        ///< The timeout has been reached.
	dsGfxFenceResult_Unset,          ///< The fence hasn't been set yet.
	dsGfxFenceResult_WaitingToQueue, ///< Still waiting to queue the fence on the GPU.
	dsGfxFenceResult_Error           ///< An error occurred. errno will have more info.
} dsGfxFenceResult;

/**
 * @brief Enum for the type of query.
 */
typedef enum dsGfxQueryType
{
	dsGfxQueryType_SamplesPassed,    ///< The number of samples that pass the depth test.
	dsGfxQueryType_AnySamplesPassed, ///< Non-zero if any samples passed.
	dsGfxQueryType_Timestamp         ///< The current timestamp on the GPU in ns.
} dsGfxQueryType;

/// @cond
typedef struct dsCommandBuffer dsCommandBuffer;
typedef struct dsRenderer dsRenderer;
typedef struct dsRenderSurface dsRenderSurface;
typedef struct mslModule mslModule;
/// @endcond

/**
 * @brief Manager for graphics resources.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 *
 * @remark The virtual functions on the resource manager should not be called directly. The public
 * interface functions handle error checking and statistic management, which could cause invalid
 * values to be reported when skipped.
 *
 * @remark When implementing the virutal functions of the resource manager, if an error occurs errno
 * to an appropriate value. If the error is due to invalid usage, it is recommended an error is
 * printed to the console.
 *
 * @see ResourceManager.h
 */
typedef struct dsResourceManager dsResourceManager;

/**
 * @brief Struct holding information about a graphics buffer.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsGfxBuffer and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see GfxBuffer.h
 */
typedef struct dsGfxBuffer
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The usage of the buffer.
	 */
	dsGfxBufferUsage usage;

	/**
	 * @brief Memory hints for how the memory will be accessed.
	 */
	dsGfxMemory memoryHints;

	/**
	 * @brief The size of the buffer in bytes.
	 */
	size_t size;
} dsGfxBuffer;

/**
 * @brief Struct describing an element of a vertex buffer.
 * @see VertexFormat.h
 */
typedef struct dsVertexElement
{
	/**
	 * @brief The format of element.
	 */
	dsGfxFormat format;

	/**
	 * @brief The offset of the element within the vertex.
	 */
	uint16_t offset;

	/**
	 * @brief The size of the element.
	 */
	uint16_t size;
} dsVertexElement;

/**
 * @brief Struct describing the format of a vertex buffer.
 * @see VertexFormat.h
 */
typedef struct dsVertexFormat
{
	/**
	 * @brief The elements of the vertex.
	 */
	dsVertexElement elements[DS_MAX_ALLOWED_VERTEX_ATTRIBS];

	/**
	 * @brief Bitmask controlling which vertex attributes are enabled.
	 *
	 * The dsBitmaskIndex() and dsRemoveLastBit() functions in DeepSea/Core/Bits.h may be used to
	 * iterate over the enabled attributes.
	 */
	uint32_t enabledMask;

	/**
	 * @brief The divisor to use when drawing instanced data.
	 *
	 * Set to 0 to disable.
	 */
	uint32_t divisor;

	/**
	 * @brief The size fo the vertex in bytes.
	 */
	uint32_t size;
} dsVertexFormat;

/**
 * @brief Struct describing a vertex buffer.
 *
 * This combines a graphics buffer, range of the buffer to use, and the vertex format.
 *
 * @see DrawGeometry.h
 */
typedef struct dsVertexBuffer
{
	/**
	 * @brief The graphcis buffer containing the data.
	 */
	dsGfxBuffer* buffer;

	/**
	 * @brief The offset into the buffer for the vertex data.
	 */
	size_t offset;

	/**
	 * @brief The number of vertices to use in the buffer.
	 */
	uint32_t count;

	/**
	 * @brief The vertex format.
	 *
	 * dsVertexFormat_computeOffsetsAndSize() must have been called earlier.
	 */
	dsVertexFormat format;
} dsVertexBuffer;

/**
 * @brief Struct describing an index buffer.
 *
 * This combines a graphics buffer, range of the buffer to use, and the size of the index.
 *
 * @see DrawGeometry.h
 */
typedef struct dsIndexBuffer
{
	/**
	 * @brief The graphcis buffer containing the data.
	 */
	dsGfxBuffer* buffer;

	/**
	 * @brief The offset into the buffer for the index data.
	 */
	size_t offset;

	/**
	 * @brief The number of indices to use in the buffer.
	 */
	uint32_t count;

	/**
	 * @brief The size of each index in bytes.
	 */
	uint32_t indexSize;
} dsIndexBuffer;

/**
 * @brief Struct holding information about drawable geometry.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsDrawGeoemtry and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see DrawGeometry.h
 */
typedef struct dsDrawGeometry
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Array of vertex buffers used to draw the geometry.
	 *
	 * Unused vertex buffers will have a NULL graphics buffer.
	 */
	dsVertexBuffer vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS];

	/**
	 * @brief The index buffer used to draw the geometry.
	 *
	 * If the graphics buffer is NULL, this cannot be used with indexed drawing.
	 */
	dsIndexBuffer indexBuffer;
} dsDrawGeometry;

/**
 * @brief Struct containing info describing the attributes of a texture.
 *
 * This is used when creating a texture as well as for some helper functions.
 *
 * @see Texture.h
 */
typedef struct dsTextureInfo
{
	/**
	 * @brief The format of the texture data.
	 */
	dsGfxFormat format;

	/**
	 * @brief The dimension of the texture.
	 */
	dsTextureDim dimension;

	/**
	 * @brief The width of the texture.
	 */
	uint32_t width;

	/**
	 * @brief The height of the texture.
	 */
	uint32_t height;

	/**
	 * @brief The depth of the texture.
	 *
	 * If not a 3D texture, this will denote the number of array levels. If 0, the texture is not
	 * an array.
	 */
	uint32_t depth;

	/**
	 * @brief The number of mip-map levels.
	 *
	 * Use DS_ALL_MIP_LEVELS to use the maximum number of mip levels.
	 */
	uint32_t mipLevels;

	/**
	 * @brief The number of samples used for multisampling.
	 *
	 * This will only be used for offscreens. This may be set to DS_DEFAULT_ANTIALIAS_SAMPLES to use
	 * the default set on the renderer. The renderbuffer will need to be re-created by the caller if
	 * the default changes. When multisampling isn't supported, this will silently fallback to no
	 * multisampling for resolved surfaces or fail for non-resolved surfaces.
	 */
	uint32_t samples;
} dsTextureInfo;

/**
 * @brief Struct holding information about a texture.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsTexture and the true internal type.
 *
 * Textures have their origin in the upper left corner.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see Texture.h
 */
typedef struct dsTexture
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	* @brief The usage of the texture.
	*/
	dsTextureUsage usage;

	/**
	* @brief Memory hints for how the memory will be accessed.
	*/
	dsGfxMemory memoryHints;

	/**
	 * @brief The info describing the texture.
	 */
	dsTextureInfo info;

	/**
	 * @brief True if this is an offscreen texture.
	 */
	bool offscreen;

	/**
	 * @brief True to resolve multisampled offscreens.
	 */
	bool resolve;
} dsTexture;

/**
 * @brief Typedef for an offscreen.
 *
 * In most cases, an offscreen may be used as a texture.
 *
 * @see Texture.h
 */
typedef dsTexture dsOffscreen;

/**
 * @brief Structure defining the position of a texture.
 * @see Texture.h
 */
typedef struct dsTexturePosition
{
	/**
	 * @brief The cube map face.
	 */
	dsCubeFace face;

	/**
	 * @brief The x coordinate.
	 *
	 * This must always be a multiple of the format's block size.
	 */
	uint32_t x;

	/**
	 * @brief The y coordinate.
	 *
	 * This must always be a multiple of the format's block size.
	 */
	uint32_t y;

	/**
	 * @brief The depth or array level for the texture.
	 */
	uint32_t depth;

	/**
	 * @brief The mipmap level.
	 */
	uint32_t mipLevel;
} dsTexturePosition;

/**
 * @brief Structure defining the region to copy for a texture.
 * @see Texture.h
 */
typedef struct dsTextureCopyRegion
{
	/**
	 * @brief The position for the source texture.
	 */
	dsTexturePosition srcPosition;

	/**
	 * @brief The position for the destination texture.
	 */
	dsTexturePosition dstPosition;

	/**
	 * @brief The width of the region to copy.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t width;

	/**
	 * @brief The height of the region to copy.
	 *
	 * This must always be a multiple of the format's block size or reach the edge of the image.
	 */
	uint32_t height;

	/**
	 * @brief The number of layers to copy.
	 *
	 * This is the number of depth layers multiplied by the number of faces.
	 */
	uint32_t layers;
} dsTextureCopyRegion;

/**
 * @brief Struct for holding data for a texture.
 *
 * This is generally loaded from a file in order to create a final renderable texture.
 *
 * @see TextureData.h
 */
typedef struct dsTextureData
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The info for the texture data.
	 */
	dsTextureInfo info;

	/**
	 * @brief The size of the data.
	 */
	size_t dataSize;

	/**
	 * @brief The data of the texture.
	 *
	 * This struct will be allocated such that it will be large enough for this array to be of size

	 */
	uint8_t data[];
} dsTextureData;

/**
 * @brief Options for converting a dsTextureData structure to a dsTexture.
 */
typedef struct dsTextureDataOptions
{
	/**
	 * @brief Number of mip-map levels to skip.
	 *
	 * @remark This will be ignored if targetWidth or targetHeight is not 0.
	 */
	uint32_t skipLevels;

	/**
	 * @brief The target height for mip level 0.
	 *
	 * This will skip mip levels until it reaches the level height a dimension closest to
	 * targetHeight. This check will not be performed if set to 0.
	 *
	 * @remark This will be ignored if targetWidth is not 0. If not 0, this will disable skipLevels.
	 */
	uint32_t targetHeight;

	/**
	 * @brief The target width for mip level 0.
	 *
	 * This will skip mip levels until it reaches the level with a dimension closest to
	 * targetWidth. This check will not be performed if set to 0.
	 *
	 * @remark If not 0, this will disable skipLevels and targetHeight.
	 */
	uint32_t targetWidth;

	/**
	 * @brief Fall back to linear texture formats for sRGB textures when sRGB isn't supported.
	 */
	bool srgbFallback;
} dsTextureDataOptions;

/**
 * @brief Structure defining a buffer to render to without having to access the contents directly.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRenderbuffer and the true internal type.
 *
 * @see Renderbuffer.h
 */
typedef struct dsRenderbuffer
{
	/**
	 * The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Flags for how the renderbuffer will be used.
	 */
	dsRenderbufferUsage usage;

	/**
	 * @brief The format of the buffer.
	 */
	dsGfxFormat format;

	/**
	 * @brief The width of the buffer.
	 */
	uint32_t width;

	/**
	 * @brief The height of the buffer.
	 */
	uint32_t height;

	/**
	 * @brief The number of samples used for multisampling.
	 */
	uint32_t samples;
} dsRenderbuffer;

/**
 * @brief Structure defining a surface to render to within a framebuffer.
 * @see Framebuffer.h
 */
typedef struct dsFramebufferSurface
{
	/**
	 * @brief The type of the surface.
	 */
	dsGfxSurfaceType surfaceType;

	/**
	 * @brief The cube face to use for cubemap offscreens.
	 *
	 * This is used when a single layer is used with the framebuffer. If multiple layers are used,
	 * all faces will be used.
	 */
	dsCubeFace cubeFace;

	/**
	 * @brief The texture array level or 3D texture level to use for offscreens.
	 *
	 * This is ignored when binding all levels.
	 */
	uint32_t layer;

	/**
	 * @brief The mipmap level to use for offscreens.
	 */
	uint32_t mipLevel;

	/**
	 * @brief The surface.
	 *
	 * The type of this pointer should be:
	 * - dsRenderSurface* if surfaceType is dsFramebufferSurfaceType_ColorRenderSurface* or
	 *   dsFramebufferSurfaceType_DepthRenderSurface*.
	 * - dsOffscreen* if surfaceType is dsFramebufferSurfaceType_Offscreen.
	 * - dsRenderbuffer* if surfaceType is dsFramebufferSurfaceType_Renderbuffer.
	 */
	void* surface;
} dsFramebufferSurface;

/**
 * @brief Structure defining a framebuffer, which is a set of surfaces to render to.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsFramebuffer and the true internal type.
 *
 * @see Framebuffer.h
 */
typedef struct dsFramebuffer
{
	/**
	 * The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The name of the framebuffer.
	 */
	const char* name;

	/**
	 * @brief The surfaces for the framebuffer.
	 */
	dsFramebufferSurface* surfaces;

	/**
	 * @brief The number of surfaces.
	 */
	uint32_t surfaceCount;

	/**
	 * @brief The width of the framebuffer.
	 */
	uint32_t width;

	/**
	 * @brief The height of the framebuffer.
	 */
	uint32_t height;

	/**
	 * @brief The number of image layers.
	 *
	 * This can be array layers, cube map images, or a combination of both.
	 */
	uint32_t layers;
} dsFramebuffer;

/**
 * @brief Structure defining a fence, allowing the CPU to synchronize to the GPU.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsGfxFence and the true internal type.
 *
 * @see GfxFence.h
 */
typedef struct dsGfxFence
{
	/**
	 * The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;
} dsGfxFence;

/**
 * @brief Structure defining a pool of query objects, allowing GPU information to be queried for GPU
 * or CPU operations.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsGfxQueryPool and the true internal type.
 *
 * @see GfxQueryPool.h
 */
typedef struct dsGfxQeuryPool
{
	/**
	 * The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of query used by the pool.
	 */
	dsGfxQueryType type;

	/**
	 * @brief The number of queries in the pool.
	 */
	uint32_t count;
} dsGfxQueryPool;

/**
 * @brief Struct for a resource context.
 *
 * A resource context must be created for each thread that manages resources. The context will be
 * globally bound for that thread when created, then un-bound when finally destroyed. (generally
 * when a thread will exit)
 *
 * This is declared here for internal use, and implementations will provide the final definition.
 *
 * @see ResourceManager.h
 */
typedef struct dsResourceContext dsResourceContext;

/**
 * @brief Function for determining whether or not a format is supported.
 *
 * A separate function pointer is used to determine if a format is supported for vertex buffers,
 * textures, and offscreens.
 *
 * @param resourceManager The resource manager.
 * @param format The graphics format.
 * @return True if the format may be used.
 */
typedef bool (*dsFormatSupportedFunction)(const dsResourceManager* resourceManager,
	dsGfxFormat format);

/**
 * @brief Function for determining whether or not copying between two formats is supported.
 * @param resourceManager The resource manager.
 * @param srcFormat The format for the source texture.
 * @param dstFormat The format for the destination texture.
 */
typedef bool (*dsCopySupportedFunction)(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat);

/**
 * @brief Function for determining whether or not blitting between two formats is supported.
 * @param resourceManager The resource manager.
 * @param srcFormat The format for the source texture.
 * @param dstFormat The format for the destination texture.
 * @param filter The blit filter.
 */
typedef bool (*dsBlitSupportedFunction)(const dsResourceManager* resourceManager,
	dsGfxFormat srcFormat, dsGfxFormat dstFormat, dsBlitFilter filter);

/**
 * @brief Function for creating a resource context for the current thread.
 * @param resourceManager The resource manager to create the resource context with.
 * @return The created resource context, or NULL if it could not be created.
 */
typedef dsResourceContext* (*dsCreateResourceContextFunction)(dsResourceManager* resourceManager);

/**
 * @brief Function for destroying a resource context.
 * @param resourceManager The resource manager that the context was created with.
 * @param context The resource context to destroy.
 * @return False if the destruction is invalid. errno should be set if the destruction failed.
 */
typedef bool (*dsDestroyResourceContextFunction)(dsResourceManager* resourceManager,
	dsResourceContext* context);

/**
 * @brief Function for creating a graphics buffer.
 * @param resourceManager The resource manager to create the buffer from.
 * @param allocator The allocator to create the graphics buffer with.
 * @param usage How the buffer will be used. This should be a combination of dsGfxBufferUsage flags.
 * @param memoryHints Hints for how the memory for the buffer will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param data The initial data for the buffer, or NULL to leave uninitialized.
 * @param size The size of the buffer.
 * @return The created buffer, or NULL if it couldn't be created.
 */
typedef dsGfxBuffer* (*dsCreateGfxBufferFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data,
	size_t size);

/**
 * @brief Function for destroying a graphics buffer.
 * @param resourceManager The resource manager the buffer was created with.
 * @param buffer The buffer to destroy.
 * @return False if the buffer couldn't be destroyed.
 */
typedef bool (*dsDestroyGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

/**
 * @brief Function for mapping a range of a buffer to memory.
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to map.
 * @param flags The flags describing how to map the memory. This should be a combination of
 *     dsGfxBufferMap flags
 * @param offset The offset into the buffer to map. This must be aligned with minMappingAlignment
 *     from dsResourceManager.
 * @param size The number of bytes to map. This may be set to DS_MAP_FULL_BUFFER to map from the
 *     offset to the end of the buffer.
 * @return A pointer to the mapped memory or NULL if the memory couldn't be mapped.
 */
typedef void* (*dsMapGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size);

/**
 * @brief Function for unmapping previously mapped memory from a buffer.
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to unmap.
 * @return False if the memory couldn't be unmapped.
 */
typedef bool (*dsUnmapGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

/**
 * @brief Function for flushing writes to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the CPU will be visible from the GPU.
 *
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to flush.
 * @param offset The offset of the range to flush.
 * @param size The size of the memory to flush.
 * @return False if the memory couldn't be flushed.
 */
typedef bool (*dsFlushGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);

/**
 * @brief Function for invalidating reads to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the GPU will be visible from the CPU.
 *
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to invalidate.
 * @param offset The offset of the range to invalidate.
 * @param size The size of the memory to invalidate.
 * @return False if the memory couldn't be invalidated.
 */
typedef bool (*dsInvalidateGfxBufferFunction)(dsResourceManager* resourceManager,
	dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Function for copying data to a buffer.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @param resourceManager The resource manager that the buffer was created with.
 * @param commandBuffer The command buffer to process the copy on.
 * @param buffer The buffer to copy the data to.
 * @param offset The offset into the buffer.
 * @param data The data to copy to the buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyGfxBufferDataFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer, size_t offset, const void* data,
	size_t size);

/**
 * @brief Function for copying data to a buffer.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @param resourceManager The resource manager that the buffers were created with.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcBuffer The buffer to copy the data from.
 * @param srcOffset The offset into the source buffer.
 * @param dstBuffer The buffer to copy to.
 * @param dstOffset The offset into the destination buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyGfxBufferFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer, size_t srcOffset,
	dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);

/**
 * @brief Function for processing a buffer.
 * @param resourceManager The resource manager the buffer was created with.
 * @param buffer The buffer to process.
 */
typedef void (*dsProcessGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

/**
 * @brief Function for creating drawable geometry.
 * @param resourceManager The resource manager to create the geomtry from.
 * @param allocator The allocator to create the geometry with.
 * @param vertexBuffers The vertex buffers to be used. NULL vertex buffers are ignored.
 * @param indexBuffer The index buffer to be used. This may be NULL if no index buffer is needed.
 * @return The created draw geometry, or NULL if it couldn't be created.
 */
typedef dsDrawGeometry* (*dsCreateDrawGeometryFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS],
	dsIndexBuffer* indexBuffer);

/**
 * @brief Function for destroying drawable geometry.
 * @param resourceManager The resource manager the geometry was created with.
 * @param geometry The geometry to destroy.
 * @return False if the geometry couldn't be destroyed.
 */
typedef bool (*dsDestroyDrawGeometryFunction)(dsResourceManager* resourceManager,
	dsDrawGeometry* geometry);

/**
 * @brief Function for creating a texture.
 * @param resourceManager The resource manager to create the texture from.
 * @param allocator The allocator to create the texture with.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param info The info for the texture.
 * @param data The initial data for the texture, or NULL to leave uninitialized. The order of the
 *     data is:
 *     - Mip levels.
 *     - Array elements/depth slices.
 *     - Cube faces in the order of dsCubeFace.
 *     - Texture rows.
 *     Data is tightly packed.
 * @param size The size of the data. This is used to help catch mismatched data.
 * @return The created texture, or NULL if it couldn't be created.
 */
typedef dsTexture* (*dsCreateTextureFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsTextureUsage usage, dsGfxMemory memoryHints,
	const dsTextureInfo* info, const void* data, size_t size);

/**
 * @brief Function for creating an offscreen texture.
 * @param resourceManager The resource manager to create the offscreen from.
 * @param allocator The allocator to create the offscreen with.
 * @param usage How the offscreen will be used. This should be a combination of dsTextureUsage
 *     flags.
 * @param memoryHints Hints for how the memory for the offscreen will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param info The info for the texture.
 * @param resolve True to resolve multisampled offscreens, false to leave unresolved to sample in
 *     the shader.
 * @return The created offscreen, or NULL if it couldn't be created.
 */
typedef dsOffscreen* (*dsCreateOffscreenFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsTextureUsage usage, dsGfxMemory memoryHints,
	const dsTextureInfo* info, bool resolve);

/**
 * @brief Function for destroying a texture or offscreen.
 * @param resourceManager The resource manager the texture was created with.
 * @param texture The texture to destroy.
 * @return False if the texture couldn't be destroyed.
 */
typedef bool (*dsDestroyTextureFunction)(dsResourceManager* resourceManager, dsTexture* texture);

/**
 * @brief Function for copying data to a texture.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @param resourceManager The resource manager the texture was created with.
 * @param commandBuffer The command buffer to process the copy on.
 * @param texture The texture to copy the data to.
 * @param position The position of the texture to copy to.
 * @param width The width of the texture data.
 * @param height The height of the texture data.
 * @param layers The number of layers in the texture data.
 * @param data The texture data to copy. This must be tightly packed.
 * @param size The size of the data. This is used to help catch mismatched data.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyTextureDataFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture, const dsTexturePosition* position,
	uint32_t width, uint32_t height, uint32_t layers, const void* data, size_t size);

/**
 * @brief Function for copying from one texture to another.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @param resourceManager The resource manager the textures were created with.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcTexture The texture to copy from.
 * @param dstTexture The texture to copy to.
 * @param regions The regions to copy.
 * @param regionCount The number of regions to copy.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyTextureFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsTexture* dstTexture,
	const dsTextureCopyRegion* regions, uint32_t regionCount);

/**
 * @brief Function for generating mipmaps for textures.
 * @param resourceManager The resource manager the texture was created with.
 * @param commandBuffer The command buffer to process the generate mipmaps on.
 * @param texture The texture to generate mipmaps for.
 * @return False if the mipmaps couldn't be generated.
 */
typedef bool (*dsGenerateTextureMipmapsFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture);

/**
 * @brief Function for getting texture data.
 * @param[out] result The output buffer for the result.
 * @param size The size of the result buffer.
 * @param resourceManager The resource manager the texture was created with.
 * @param texture The texture to read from.
 * @param position The position to read from.
 * @param width The width of the texture region.
 * @param height The height of the texture region.
 * @return False if the data couldn't be read.
 */
typedef bool (*dsGetTextureDataFunction)(void* result, size_t size,
	dsResourceManager* resourceManager, dsTexture* texture, const dsTexturePosition* position,
	uint32_t width, uint32_t height);

/**
 * @brief Function for processing a texture.
 * @param resourceManager The resource manager the texture was created with.
 * @param texture The texture to process.
 */
typedef void (*dsProcessTextureFunction)(dsResourceManager* resourceManager, dsTexture* texture);

/**
 * @brief Function for creeating a framebuffer.
 * @param resourceManager The resource manager to create the framebuffer from.
 * @param allocator The allocator to create the framebuffer with.
 * @param name The name of the framebuffer.
 * @param surfaces The surfaces that make up the framebuffer.
 * @param surfaceCount The number of surfaces.
 * @param width The width of the framebuffer.
 * @param height The height of the framebuffer.
 * @param layers The number of array layers in the framebuffer.
 * @return The created framebuffer, or NULL if it couldn't be created.
 */
typedef dsFramebuffer* (*dsCreateFramebufferFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* name, const dsFramebufferSurface* surfaces,
	uint32_t surfaceCount, uint32_t width, uint32_t height, uint32_t layers);

/**
 * @brief Function for destroying a renderbuffer.
 * @param resourceManager The resource manager the framebuffer was created with.
 * @param renderbuffer The renderbuffer.
 * @return False if the renderbuffer couldn't be destroyed.
 */
typedef bool (*dsDestroyRenderbufferFunction)(dsResourceManager* resourceManager,
	dsRenderbuffer* renderbuffer);

/**
 * @brief Function for creating a renderbuffer.
 * @param resourceManager The resource manager to create the renderbuffer from.
 * @param allocator The allocator to create the renderbuffer with.
 * @param usage How the renderbuffer will be used.
 * @param format The format of the renderbuffer.
 * @param width The width of the renderbuffer.
 * @param height The height of the renderbuffer.
 * @param samples The number of samples to use for multisampling.
 * @return The created renderbuffer, or NULL if it couldn't be created.
 */
typedef dsRenderbuffer* (*dsCreateRenderbufferFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsRenderbufferUsage usage, dsGfxFormat format, uint32_t width,
	uint32_t height, uint32_t samples);

/**
 * @brief Function for destroying a framebuffer.
 * @param resourceManager The resource manager the framebuffer was created with.
 * @param framebuffer The framebuffer.
 * @return False if the framebuffer couldn't be destroyed.
 */
typedef bool (*dsDestroyFramebufferFunction)(dsResourceManager* resourceManager,
	dsFramebuffer* framebuffer);

/**
 * @brief Function for creating a fence.
 * @param resourceManager The resource manager to create the fence from.
 * @param allocator The allocator to create the fence with.
 * @return The created fence, or NULL if it couldn't be created.
 */
typedef dsGfxFence* (*dsCreateFenceFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator);

/**
 * @brief Function for destroying a fence.
 * @param resourceManager The resource manager the fence was created with.
 * @param fence The fence.
 * @return False if the fence couldn't be destroyed.
 */
typedef bool (*dsDestroyFenceFunction)(dsResourceManager* resourceManager,
	dsGfxFence* fence);

/**
 * @brief Function for setting multiple fences.
 * @param resourceManager The resource manager the fence was created with.
 * @param commandBuffer The command buffer to queue the fence on.
 * @param fences The fences.
 * @param fenceCount The number of fences.
 * @param bufferReadback True if persistently mapped buffers will be read back.
 * @return False if the fence couldn't be set.
 */
typedef bool (*dsSetFencesFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback);

/**
 * @brief Function for waiting for a fence to complete.
 * @param resourceManager The resource manager the fence was created with.
 * @param fence The fence.
 * @param timeout The number of nanoseconds to wait for the fence.
 * @return The result of the wait.
 */
typedef dsGfxFenceResult (*dsWaitFenceFunction)(dsResourceManager* resourceManager,
	dsGfxFence* fence, uint64_t timeout);

/**
 * @brief Function for resetting a fence.
 * @param resourceManager The resource manager the fence was created with.
 * @param fence The fence.
 * @return False if the fence couldn't be reset.
 */
typedef bool (*dsResetFenceFunction)(dsResourceManager* resourceManager,
	dsGfxFence* fence);

/**
 * @brief Function for creating a query pool.
 * @param resourceManager The resource manager to create the query pool from.
 * @param allocator The allocator to create the query pool with.
 * @param type The type of queries used by the pool.
 * @return The created query pool, or NULL if it couldn't be created.
 */
typedef dsGfxQueryPool* (*dsCreateQueryPoolFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count);

/**
 * @brief Function for destroying a query pool.
 * @param resourceManager The resource manager the query pool was created with.
 * @param queries The query pool.
 * @return False if the query pool couldn't be destroyed.
 */
typedef bool (*dsDestroyQueryPoolFunction)(dsResourceManager* resourceManager,
	dsGfxQueryPool* queries);

/**
 * @brief Function for resetting queries to an unset state.
 * @param resourceManager The resource manager the query pool was created with.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param queries The query pool.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @return False if the queries couldn't be reset.
 */
typedef bool (*dsResetQueryPoolFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count);

/**
 * @brief Function for beginning or ending a query.
 * @param resourceManager The resource manager the query pool was created with.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param queries The query pool.
 * @param query The index of the query to begin.
 * @return False if the query couldn't begin.
 */
typedef bool (*dsBeginEndQueryFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query);

/**
 * @brief Function for querying the current GPU timestamp.
 * @param resourceManager The resource manager the query pool was created with.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param queries The query pool.
 * @param query The index of the query to set the timestamp for.
 * @return False if the timestamp couldn't be set.
 */
typedef bool (*dsQueryTimestampFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query);

/**
 * @brief Function for getting the query values.
 * @param resourceManager The resource manager the query pool was created with.
 * @param queries The query pool.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @param data The data to write the results into.
 * @param dataSize The size of the data buffer.
 * @param stride The stride of the data.
 * @param elementSize The size of each element.
 * @param checkAvailability True to check availability and avoid waiting. When true, two values are
 *     set for each query: the value (if available) and a 0 or 1 value for the value being
 *     available.
 * @return False if the values couldn't be queried.
 */
typedef bool (*dsGetQueryValuesFunction)(dsResourceManager* resourceManager,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count, void* data, size_t dataSize,
	size_t stride, size_t elementSize, bool checkAvailability);

/**
 * @brief Function for copying the query values to a GPU buffer.
 * @param resourceManager The resource manager the query pool was created with.
 * @param commandBuffer The command buffer to queue the command onto.
 * @param queries The query pool.
 * @param first The first query to reset.
 * @param count The number of queries to reset.
 * @param buffer The graphic buffer to write the results into.
 * @param offset The offset into the buffer for the first element.
 * @param stride The stride of the data.
 * @param elementSize The size of each element.
 * @param checkAvailability True to check availability and avoid waiting. When true, two values are
 *     set for each query: the value (if available) and a 0 or 1 value for the value being
 *     available.
 * @return False if the values couldn't be queried.
 */
typedef bool (*dsCopyQueryValuesFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability);

/**
 * @brief Function for creating a shader module.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with.
 * @param module The MSL shader module.
 * @param name The name of the module.
 * @return The created shader module, or NULL if it couldn't be created.
 */
typedef dsShaderModule* (*dsCreateShaderModuleFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, mslModule* module, const char* name);

/**
 * @brief Function for destroying a shader module.
 * @param resourceManager The resource manager the shader module was created with.
 * @param module The shader module.
 * @return False if the shader module couldn't be destroyed.
 */
typedef bool (*dsDestroyShaderModuleFunction)(dsResourceManager* resourceManager,
	dsShaderModule* module);

/**
 * @brief Function for creating a material description.
 * @param resourceManager The resource manager the material description was created with.
 * @param allocator The allocator to create the material description with.
 * @param elements An array of elements for the description. This array should be allocated
 *     with the material description and copied.
 * @param elementCount The number of elements.
 * @return The created material description, or NULL if it couldn't be created.
 */
typedef dsMaterialDesc* (*dsCreateMaterialDescFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount);

/**
 * @brief Function for destroying a material description.
 * @param resourceManager The resource manager the material description was created with.
 * @param materialDesc The material description.
 * @return False if the material description couldn't be destroyed.
 */
typedef bool (*dsDestroyMaterialDescFunction)(dsResourceManager* resourceManager,
	dsMaterialDesc* materialDesc);

/**
 * @brief Function for creating a shader variable group description.
 * @param resourceManager The resource manager the shader variable group description was created
 *     with.
 * @param allocator The allocator to create the shader variable group description with.
 * @param elements An array of elements for the description.
 * @param elementCount The number of elements.
 * @return The created shader variable group description, or NULL if it couldn't be created.
 */
typedef dsShaderVariableGroupDesc* (*dsCreateShaderVariableGroupDescFunction)(
	dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsShaderVariableElement* elements, uint32_t elementCount);

/**
 * @brief Function for destroying a shader variable group description.
 * @param resourceManager The resource manager the shader variable group description was created
 *     with.
 * @param shaderVarGroupDesc The shader variable group description.
 * @return False if the shader variable group description couldn't be destroyed.
 */
typedef bool (*dsDestroyShaderVariableGroupDescFunction)(dsResourceManager* resourceManager,
	dsShaderVariableGroupDesc* shaderVarGroupDesc);

/**
 * @brief Function for creating data for device-specific material management.
 * @remark Setting the device material functions is optional, but if set it's expected that
 *     returning NULL is an error.
 * @param resourceManager The resource manager the material was created with.
 * @param material The material to create the device material for.
 * @param allocator The allocator to create the device material with.
 * @return The device material, or NULL if it couldn't be created.
 */
typedef dsDeviceMaterial* (*dsCreateDeviceMaterialFunction)(dsResourceManager* resourceManager,
	dsMaterial* material, dsAllocator* allocator);

/**
 * @brief Function for destroy data for device-specific material management.
 * @param resourceManager The resource manager the material was created with.
 * @param material The material the device material was created with.
 * @param deviceMaterial The device material to destroy.
 */
typedef void (*dsDestroyDeviceMaterialFunction)(dsResourceManager* resourceManager,
	dsMaterial* material, dsDeviceMaterial* deviceMaterial);

/**
 * @brief Function for reacting to a material change.
 * @param resourceManager The resource manager the material was created with.
 * @param material The material the device material was created with.
 * @param deviceMaterial The device material.
 * @param The material element that was changed.
 */
typedef void (*dsDeviceMaterialChangedFunction)(dsResourceManager* resourceManager,
	dsMaterial* material, dsDeviceMaterial* deviceMaterial, uint32_t element);

/**
 * @brief Function to check if a uniform is internal, removing the requirement for the user
 *     to set it in the material description.
 * @param resourceManager The resource manager.
 * @param name The name of the uniform.
 * @return True if the uniform is internal.
 */
typedef bool (*dsIsShaderUniformInternalFunction)(dsResourceManager* resourceManager,
	const char* name);

/**
 * @brief Function for creating a shader.
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with.
 * @param module The shader module that contains the shader.
 * @param shaderIndex The index of the shader.
 * @param materialDesc The description of the material type used by the shader.
 * @param primitiveType The type of primitives the shader will be drawn with.
 * @return The created shader, or NULL if it couldn't be created.
 */
typedef dsShader* (*dsCreateShaderFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* module, uint32_t shaderIndex,
	const dsMaterialDesc* materialDesc);

/**
 * @brief Function for destroying a shader.
 * @param resourceManager The resource manager the shader was created with.
 * @param shader The shader.
 * @return False if the shader couldn't be destroyed.
 */
typedef bool (*dsDestroyShaderFunction)(dsResourceManager* resourceManager, dsShader* shader);

/**
 * @brief Function for binding a shader for drawing.
 * @param resourceManager The resource manager the shader was created with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param shader The shader to draw with.
 * @param material The material values to apply to the shader.
 * @param globalValues The global values to apply to the shader.
 * @param renderStates The dynamic render states to apply. This may be NULL to use the default
 *     values.
 * @return False if the values couldn't be bound.
 */
typedef bool (*dsBindShaderFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates);

/**
 * @brief Function for updating the instance material values used for the currently bound shader.
 *
 * The implementation should attempt to only update the values that have changed.
 *
 * @param resourceManager The resource manager the shader was created with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param shader The shader to update the values on.
 * @param instanceValues The instance values to update.
 * @return False if the values couldn't be updated.
 */
typedef bool (*dsUpdateShaderInstanceValuesFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* instanceValues);

/**
 * @brief Function for updating the dynamic render states for the currently bound shader.
 *
 * The implementation should attempt to only update the values that have changed.
 *
 * @param resourceManager The resource manager the shader was created with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param shader The shader to update the render states on.
 * @param renderStates The dynamic render states to apply.
 * @return False if the values couldn't be updated.
 */
typedef bool (*dsUpdateShaderDynamicRenderStatesFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Function for un-binding the currently bound shader.
 * @param resourceManager The resource manager the shader was created with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if the values couldn't be unbound.
 */
typedef bool (*dsUnbindShaderFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader);

/**
 * @brief Function for binding a shader for compute.
 * @param resourceManager The resource manager the shader was created with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param shader The shader to draw with.
 * @param material The material values to apply to the shader.
 * @param globalValues The global values to apply to the shader.
 * @return False if the values couldn't be bound.
 */
typedef bool (*dsBindComputeShaderFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* globalValues);

/** @copydoc dsResourceManager */
struct dsResourceManager
{
	/**
	 * The renderer this belongs to.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The allocator this was created with.
	 *
	 * This will also be the default allocator for allocating resources.
	 */
	dsAllocator* allocator;

	// --------------------------------- Resource capabilities -------------------------------------

	/**
	 * @brief The number of resource contexts that may be created for other threads.
	 */
	uint32_t maxResourceContexts;

	/**
	 * @brief The minimum alignment when mapping the range of a non-coherent buffer.
	 */
	uint32_t minNonCoherentMappingAlignment;

	/**
	 * @brief The minimum alignment for an texture buffer offset.
	 */
	uint32_t minTextureBufferAlignment;

	/**
	 * @brief The minimum alignment for a uniform block offset.
	 */
	uint32_t minUniformBlockAlignment;

	/**
	* @brief The minimum alignment for a uniform buffer offset.
	*/
	uint32_t minUniformBufferAlignment;

	/**
	 * @brief Bitmask for the supported buffer types.
	 */
	dsGfxBufferUsage supportedBuffers;

	/**
	 * @brief Enum describing how buffers may be mapped.
	 */
	dsGfxBufferMapSupport bufferMapSupport;

	/**
	 * @brief Whether or not buffers can be copied between each other.
	 */
	bool canCopyBuffers;

	/**
	 * @brief Whether or not texture buffers can be a subrange of the buffer.
	 */
	bool hasTextureBufferSubrange;

	/**
	 * @brief The maximum size for each index in bytes.
	 */
	uint32_t maxIndexSize;

	/**
	 * @brief The maximum size of a uniform block.
	 */
	size_t maxUniformBlockSize;

	/**
	 * @brief The maximum number of elements in a texture buffer.
	 */
	size_t maxTextureBufferElements;

	/**
	 * @brief The maximum number of vertex attributes.
	 *
	 * This must not be greater than DS_MAX_ALLOWED_VERTEX_ATTRIBS.
	 */
	uint32_t maxVertexAttribs;

	/**
	 * @brief The minimum and maximum values for line widths.
	 */
	dsVector2f lineWidthRange;

	/**
	 * @brief The maximum number of samplers that can be used at once in a shader.
	 */
	uint32_t maxSamplers;

	/**
	 * @brief The maximum number of samplers that can be used at once in a vertex shader.
	 *
	 * This may be 0 if vertex texture lookup isn't supported.
	 */
	uint32_t maxVertexSamplers;

	/**
	 * @brief The maximum size of textures along the width and height.
	 */
	uint32_t maxTextureSize;

	/**
	 * @brief The maximum depth of 3D textures, or 0 if 3D textures aren't supported.
	 */
	uint32_t maxTextureDepth;

	/**
	 * @brief The maximum number of texture array levels, or 0 if texture arrays aren't supported.
	 */
	uint32_t maxTextureArrayLevels;

	/**
	 * @brief The maximum size of renderbuffers along the width and height.
	 */
	uint32_t maxRenderbufferSize;

	/**
	 * @brief The maximum number of layers for a framebuffer.
	 */
	uint32_t maxFramebufferLayers;

	/**
	 * @brief Maximum number of samples that a texture supports.
	 *
	 * This is set to 1 if multisampled textures aren't supported.
	 */
	uint32_t maxTextureSamples;

	/**
	 * @brief Boolean for whether or not textures can be arbitrarily mipmapped.
	 *
	 * When false, textures may only be created with 1 or the maximum mip levels.
	 */
	bool hasArbitraryMipmapping;

	/**
	 * @brief Boolean for whether or not cubemap arrays are supported.
	 */
	bool hasCubeArrays;

	/**
	 * @brief Boolean for whether or not textures are readable.
	 *
	 * Offscreens will always be readable.
	 */
	bool texturesReadable;

	/**
	 * @brief True if a color buffer must be provided with a framebuffer.
	 */
	bool requiresColorBuffer;

	/**
	 * @brief True if a surface must be provided with a framebuffer.
	 */
	bool requiresAnySurface;

	/**
	 * @brief True if offscreens or renderbuffers may be mixed with render surfaces in a
	 * framebuffer.
	 */
	bool canMixWithRenderSurface;

	/**
	 * @brief True if vertex, geometry, and tessellation shaders can write to uniform buffers and
	 * images.
	 */
	bool hasVertexPipelineWrites;

	/**
	 * @brief True if fragment shaders can write to uniform buffers and images.
	 */
	bool hasFragmentWrites;

	/**
	 * @brief True if fences are supported.
	 */
	bool hasFences;

	/**
	 * @brief True if queries are supported.
	 */
	bool hasQueries;

	/**
	 * @brief True if query values may be 64 bits.
	 *
	 * This will always be true if timestamp queries are supported.
	 */
	bool has64BitQueries;

	/**
	 * @brief True if query values may be copied to a buffer.
	 */
	bool hasQueryBuffers;

	/**
	 * @brief The number of nanoseconds for each timestamp tick.
	 *
	 * This will be 0 if timestamp queries aren't supported.
	 */
	float timestampPeriod;

	// ---------------------------------- Resource statistics --------------------------------------

	/**
	 * @brief The current number of resource contexts.
	 */
	uint32_t resourceContextCount;

	/**
	 * @brief The number of buffers currently allocated by the resource manager.
	 */
	uint32_t bufferCount;

	/**
	 * @brief The number of draw geometries currently allocated by the resource manager.
	 */
	uint32_t geometryCount;

	/**
	 * @brief The number of textures currently allocated by the resource manager.
	 */
	uint32_t textureCount;

	/**
	 * @brief The number of renderbuffers currently allocated by the resource manager.
	 */
	uint32_t renderbufferCount;

	/**
	 * @brief The number of framebuffers currently allocated by the resource manager.
	 */
	uint32_t framebufferCount;

	/**
	 * @brief The number of fences currently allocated by the resource manager.
	 */
	uint32_t fenceCount;

	/**
	 * @brief The number of query pools currently allocated by the resource manager.
	 */
	uint32_t queryPoolCount;

	/**
	 * @brief The number of shader modules currently allocated by the resource manager.
	 */
	uint32_t shaderModuleCount;

	/**
	 * @brief The number of material descriptions currently allocated by the resource manager.
	 */
	uint32_t materialDescCount;

	/**
	 * @brief The number of shader variable group descriptions currently allocated by the resource
	 * manager.
	 */
	uint32_t shaderVariableGroupDescCount;

	/**
	 * @brief The number of shader variable groups currently allocated by the resource manager.
	 */
	uint32_t shaderVariableGroupCount;

	/**
	 * @brief The number of shaders currently allocated by the resource manager.
	 */
	uint32_t shaderCount;

	/**
	 * @brief The number of bytes allocated for graphics buffers.
	 */
	size_t bufferMemorySize;

	/**
	 * @brief The number of bytes allocated for textures.
	 */
	size_t textureMemorySize;

	/**
	 * @brief The number of bytes allocated for renderbuffers.
	 */
	size_t renderbufferMemorySize;

	// ----------------------------- Internals and function table ----------------------------------

	/**
	 * Current thread's resource context.
	 */
	dsThreadStorage _resourceContext;

	// Virtual function table

	/**
	 * @brief Vertex format supported function.
	 */
	dsFormatSupportedFunction vertexFormatSupportedFunc;

	/**
	 * @brief Texture format supported function.
	 */
	dsFormatSupportedFunction textureFormatSupportedFunc;

	/**
	 * @brief Offscreen format supported function.
	 */
	dsFormatSupportedFunction offscreenFormatSupportedFunc;

	/**
	 * @brief Texture buffer format supported function.
	 */
	dsFormatSupportedFunction textureBufferFormatSupportedFunc;

	/**
	 * @brief Texture mipmap generation validity check function.
	 */
	dsFormatSupportedFunction generateMipmapFormatSupportedFunc;

	/**
	 * @brief Texture copy validity check function.
	 */
	dsCopySupportedFunction textureCopyFormatsSupportedFunc;

	/**
	 * @brief Texture blit validity check function.
	 */
	dsBlitSupportedFunction surfaceBlitFormatsSupportedFunc;

	/**
	 * @brief Resource context creation function.
	 */
	dsCreateResourceContextFunction createResourceContextFunc;

	/**
	 * @brief Resource context destruction function.
	 */
	dsDestroyResourceContextFunction destroyResourceContextFunc;

	/**
	 * @brief Buffer creation function.
	 */
	dsCreateGfxBufferFunction createBufferFunc;

	/**
	 * @brief Buffer destruction function.
	 */
	dsDestroyGfxBufferFunction destroyBufferFunc;

	/**
	 * @brief Buffer mapping function.
	 */
	dsMapGfxBufferFunction mapBufferFunc;

	/**
	 * @brief Buffer unmapping function.
	 */
	dsUnmapGfxBufferFunction unmapBufferFunc;

	/**
	 * @brief Buffer flushing function.
	 */
	dsFlushGfxBufferFunction flushBufferFunc;

	/**
	 * @brief Buffer invalidating function.
	 */
	dsInvalidateGfxBufferFunction invalidateBufferFunc;

	/**
	 * @brief Buffer data copying function.
	 */
	dsCopyGfxBufferDataFunction copyBufferDataFunc;

	/**
	 * @brief Buffer to buffer copying function.
	 */
	dsCopyGfxBufferFunction copyBufferFunc;

	/**
	 * @brief Buffer process function.
	 */
	dsProcessGfxBufferFunction processBufferFunc;

	/**
	 * @brief Geometry creation function.
	 */
	dsCreateDrawGeometryFunction createGeometryFunc;

	/**
	 * @brief Geometry destruction function.
	 */
	dsDestroyDrawGeometryFunction destroyGeometryFunc;

	/**
	 * @brief Texture creation function.
	 */
	dsCreateTextureFunction createTextureFunc;

	/**
	 * @brief Offscreen creation function.
	 */
	dsCreateOffscreenFunction createOffscreenFunc;

	/**
	 * @brief Texture destruction function.
	 */
	dsDestroyTextureFunction destroyTextureFunc;

	/**
	 * @brief Texture data copying function.
	 */
	dsCopyTextureDataFunction copyTextureDataFunc;

	/**
	 * @brief Texture to texture copying function.
	 */
	dsCopyTextureFunction copyTextureFunc;

	/**
	 * @brief Texture process function.
	 */
	dsProcessTextureFunction processTextureFunc;

	/**
	 * @brief Texture mipmap generation function.
	 */
	dsGenerateTextureMipmapsFunction generateTextureMipmapsFunc;

	/**
	 * @brief Texture data getting function.
	 */
	dsGetTextureDataFunction getTextureDataFunc;

	/**
	 * @brief Function for creating a renderbuffer.
	 */
	dsCreateRenderbufferFunction createRenderbufferFunc;

	/**
	 * @brief Function for destroying a renderbuffer.
	 */
	dsDestroyRenderbufferFunction destroyRenderbufferFunc;

	/**
	 * @brief Function for creating a framebuffer.
	 */
	dsCreateFramebufferFunction createFramebufferFunc;

	/**
	 * @brief Function for destroying a framebuffer.
	 */
	dsDestroyFramebufferFunction destroyFramebufferFunc;

	/**
	 * @brief Function for creating a fence.
	 */
	dsCreateFenceFunction createFenceFunc;

	/**
	 * @brief Function for destroying a fence.
	 */
	dsDestroyFenceFunction destroyFenceFunc;

	/**
	 * @brief Function for setting multiple fences.
	 */
	dsSetFencesFunction setFencesFunc;

	/**
	 * @brief Function for waiting on a fence.
	 */
	dsWaitFenceFunction waitFenceFunc;

	/**
	 * @brief Function for resetting a fence.
	 */
	dsResetFenceFunction resetFenceFunc;

	/**
	 * @brief Function for creating a qury pool
	 */
	dsCreateQueryPoolFunction createQueryPoolFunc;

	/**
	 * @brief Function for destroying a qury pool
	 */
	dsDestroyQueryPoolFunction destroyQueryPoolFunc;

	/**
	 * @brief Function for resetting a query pool.
	 */
	dsResetQueryPoolFunction resetQueryPoolFunc;

	/**
	 * @brief Function for beginning a query.
	 */
	dsBeginEndQueryFunction beginQueryFunc;

	/**
	 * @brief Function for ending a query.
	 */
	dsBeginEndQueryFunction endQueryFunc;

	/**
	 * @brief Function for querying the GPU timestamp.
	 */
	dsQueryTimestampFunction queryTimestampFunc;

	/**
	 * @brief Function for getting the query values.
	 */
	dsGetQueryValuesFunction getQueryValuesFunc;

	/**
	 * @brief Function for copying the query values on the GPU.
	 */
	dsCopyQueryValuesFunction copyQueryValuesFunc;

	/**
	 * @brief Shader module creation function.
	 */
	dsCreateShaderModuleFunction createShaderModuleFunc;

	/**
	 * @brief Shader module destruction function.
	 */
	dsDestroyShaderModuleFunction destroyShaderModuleFunc;

	/**
	 * @brief Material description creation function.
	 */
	dsCreateMaterialDescFunction createMaterialDescFunc;

	/**
	 * @brief Material description destruction function.
	 */
	dsDestroyMaterialDescFunction destroyMaterialDescFunc;

	/**
	 * @brief Shader variable group description creation function.
	 */
	dsCreateShaderVariableGroupDescFunction createShaderVariableGroupDescFunc;

	/**
	 * @brief Shader variable group description destruction function.
	 */
	dsDestroyShaderVariableGroupDescFunction destroyShaderVariableGroupDescFunc;

	/**
	 * @brief Device material create function.
	 */
	dsCreateDeviceMaterialFunction createDeviceMaterialFunc;

	/**
	 * @brief Destroy material create function.
	 */
	dsDestroyDeviceMaterialFunction destroyDeviceMaterialFunc;

	/**
	 * @brief Material element changed function.
	 */
	dsDeviceMaterialChangedFunction materialElementChangedFunc;

	/**
	 * @brief Shader creation function.
	 */
	dsCreateShaderFunction createShaderFunc;

	/**
	 * @brief Shader destruction function.
	 */
	dsDestroyShaderFunction destroyShaderFunc;

	/**
	 * @brief Internal shader name function.
	 */
	dsIsShaderUniformInternalFunction isShaderUniformInternalFunc;

	/**
	 * @brief Shader binding function.
	 */
	dsBindShaderFunction bindShaderFunc;

	/**
	 * @brief Shader instance value update function.
	 */
	dsUpdateShaderInstanceValuesFunction updateShaderInstanceValuesFunc;

	/**
	 * @brief Shader dynamic render state update function.
	 */
	dsUpdateShaderDynamicRenderStatesFunction updateShaderDynamicRenderStatesFunc;

	/**
	 * @brief Shader unbinding function.
	 */
	dsUnbindShaderFunction unbindShaderFunc;

	/**
	 * @brief Compute shader binding function.
	 */
	dsBindComputeShaderFunction bindComputeShaderFunc;

	/**
	 * @brief Compute shader instance value update function.
	 */
	dsUpdateShaderInstanceValuesFunction updateComputeShaderInstanceValuesFunc;

	/**
	 * @brief Compute shader unbinding function.
	 */
	dsUnbindShaderFunction unbindComputeShaderFunc;
};

#ifdef __cplusplus
}
#endif
