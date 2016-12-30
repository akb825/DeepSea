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
#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/ShaderTypes.h>
#include <DeepSea/Render/RenderStates.h>
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
	dsGfxMemory_GpuOnly = 0x01,    ///< The memory will only ever be accessed by the GPU.
	dsGfxMemory_Static = 0x02,     ///< The memory will be modified on the CPU very rarely.
	dsGfxMemory_Dynamic = 0x04,    ///< The memory will be modified on the CPU somewhat often.
	dsGfxMemory_Stream = 0x08,     ///< The memory will be modified on the CPU every time it's used.
	dsGfxMemory_Draw = 0x10,       ///< The memory will be used by draw commands.
	dsGfxMemory_Read = 0x20,       ///< The memory will be read back from the GPU.
	dsGfxMemory_Coherent = 0x40,   ///< The memory should remain coherent to avoid manual flushing.
	dsGfxMemory_Synchronize = 0x80 ///< Wait for the memory to not be in use when mapping.
} dsGfxMemory;

/**
 * @brief Enum for how a graphics buffer will be used.
 *
 * These are bitmask values, allowing a buffer to be used for multiple purposes.
 */
typedef enum dsGfxBufferUsage
{
	dsGfxBufferUsage_Index = 0x001,         ///< Index buffer.
	dsGfxBufferUsage_Vertex  = 0x002,       ///< Vertex buffer.
	dsGfxBufferUsage_Indirect = 0x004,      ///< Indirect draw information.
	dsGfxBufferUsage_UniformBlock = 0x008,  ///< Shader uniform block.
	dsGfxBufferUsage_UniformBuffer = 0x010, ///< Shader uniform buffer, modifiable by the shader.
	dsGfxBufferUsage_Image = 0x020,         ///< Shader image buffer.
	dsGfxBufferUsage_Sampler = 0x040,       ///< Shader sampler buffer.
	dsGfxBufferUsage_CopyFrom = 0x080,      ///< Source for GPU copy operations.
	dsGfxBufferUsage_CopyTo = 0x100         ///< Destination for GPU and CPU copy operations.
} dsGfxBufferUsage;

/**
 * @brief Enum for how a texture will be used.
 *
 * These are bitmask values, allowing a texture to be used for multiple purposes.
 */
typedef enum dsTextureUsage
{
	dsTextureUsage_Texture = 0x1,  ///< Use as a sampled texture.
	dsTextureUsage_Image = 0x2,    ///< Use as an image without a sampler.
	dsTextureUsage_CopyFrom = 0x4, ///< Source for GPU copy operations.
	dsTextureUsage_CopyTo = 0x8    ///< Destination for GPU and CPU copy operations.
} dsTextureUsage;

/**
 * @brief Flags for how to map a graphics buffer to memory.
 */
typedef enum dsGfxBufferMap
{
	dsGfxBufferMap_Read = 0x1,      ///< Read data from the buffer.
	dsGfxBufferMap_Write = 0x2,     ///< Write data to the buffer.
	dsGfxBufferMap_Persistent = 0x4 ///< Allow the buffer to remain locked.
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
 */
typedef enum dsGfxFormat
{
	dsGfxFormat_Unknown = 0, ///< No known format.

	// Standard formats
	dsGfxFormat_R4G4 = 1,            ///< RG 4 bits each.
	dsGfxFormat_R4G4B4A4,            ///< RGBA 4 bits each.
	dsGfxFormat_B4G4R4A4,            ///< BGRA 4 bits each.
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
 */
typedef enum dsCubeFace
{
	dsCubeFace_PosX, ///< +X
	dsCubeFace_NegX, ///< -X
	dsCubeFace_PosY, ///< +Y
	dsCubeFace_NegY, ///< -Y
	dsCubeFace_PosZ, ///< +Z
	dsCubeFace_NegZ  ///< -Z
} dsCubeFace;

/**
 * @brief Enum for named vertex attributes.
 *
 * These are mainly suggestions rather than a requirement to make it easier to match vertex
 * attributes between code and shaders.
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
 * @brief Enum for the type of a surface used within a framebuffer.
 */
typedef enum dsFramebufferSurfaceType
{
	dsFramebufferSurfaceType_RenderSurface, ///< Render surface, such as a window.
	dsFramebufferSurfaceType_Offscreen      ///< Offscreen texture.
} dsFramebufferSurfaceType;

/// \{
typedef struct dsCommandBuffer dsCommandBuffer;
typedef struct dsRenderer dsRenderer;
typedef struct dsRenderSurface dsRenderSurface;
typedef struct mslModule mslModule;
/// \}

/**
 * @brief Manager for graphics resources.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * All manipulation of graphics resources requires a resource context to be created. There will
 * always be a resource context available on the main thread, while other threads require a resource
 * context to be created. Up to maxResourceContexts contexts may be created, which may be 0 for
 * platforms that don't allow multiple threads to access graphics resources.
 *
 * @remark None of the members should be modified outside of the implementation. If any of the
 * virtual functions fail, the implementation should set errno to an appropriate value. If the error
 * is due to invalid usage, it is recommended an error is printed to the console.
 *
 * @remark The virtual functions on the resource manager should not be called directly. The public
 * interface functions handle error checking and statistic management, which could cause invalid
 * values to be reported when skipped.
 */
typedef struct dsResourceManager dsResourceManager;

/**
 * @brief Struct holding information about a graphics buffer.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
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
	uint16_t size;
} dsVertexFormat;

/**
 * @brief Struct describing a vertex buffer.
 *
 * This combines a graphics buffer, range of the buffer to use, and the vertex format.
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
	uint32_t offset;

	/**
	 * @brief The number of vertices to use in the buffer.
	 */
	uint32_t count;

	/**
	 * @brief The vertex format.
	 */
	dsVertexFormat format;
} dsVertexBuffer;

/**
 * @brief Struct describing an index buffer.
 *
 * This combines a graphics buffer, range of the buffer to use, and the size of the index.
 */
typedef struct dsIndexBuffer
{
	/**
	 * @brief The graphcis buffer containing the data.
	 */
	dsGfxBuffer* buffer;

	/**
	 * @brief The offset into the buffer for the vertex data.
	 */
	uint32_t offset;

	/**
	 * @brief The number of indices to use in the buffer.
	 */
	uint32_t count;

	/**
	 * @brief The number of bits for each index.
	 */
	uint32_t indexBits;
} dsIndexBuffer;

/**
 * @brief Struct holding information about drawable geometry.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
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
 * @brief Struct holding information about a texture.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * Textures have their origin in the upper left corner.
 *
 * @remark None of the members should be modified outside of the implementation.
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
	 * @brief The format of the texture data.
	 */
	dsGfxFormat format;

	/**
	 * @brief THe dimension of the texture.
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
	 */
	uint32_t mipLevels;

	/**
	 * @brief True if this is an offscreen texture.
	 */
	bool offscreen;

	/**
	 * @brief True to resolve multisampled offscreens, false to leave unresolved to sample in the
	 *     shader.
	 */
	bool resolve;

	/**
	 * @brief The number of samples used for multisampling.
	 *
	 * This is generally only used for offscreens.
	 */
	uint16_t samples;
} dsTexture;

/**
 * @brief Typedef for an offscreen.
 *
 * In most cases, an offscreen may be used as a texture.
 */
typedef dsTexture dsOffscreen;

/**
 * @brief Structure defining the position of a texture.
 */
typedef struct dsTexturePosition
{
	/**
	 * @brief The cube map face.
	 */
	dsCubeFace face;

	/**
	 * @brief The x coordinate.
	 */
	uint32_t x;

	/**
	 * @brief The y coordinate.
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
	 */
	uint32_t width;

	/**
	 * @brief The height of the region to copy.
	 */
	uint32_t height;

	/**
	 * @brief The number of array levels to copy.
	 *
	 * This cannot be used for multiple depth levels of a 3D texture.
	 */
	uint32_t arrayLevelCount;
} dsTextureCopyRegion;

/**
 * @brief Structure defining the region to blit for a texture.
 */
typedef struct dsTextureBlitRegion
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
	 * @brief The width of the source image to blit from.
	 */
	uint32_t srcWidth;

	/**
	 * @brief The height of the source image to blit from.
	 */
	uint32_t srcHeight;

	/**
	 * @brief The number of depth levels or array levels to blit from.
	 *
	 * When using texture arrays, this must match dstDepthRange.
	 */
	uint32_t srcDepthRange;

	/**
	 * @brief The width of the destination image to blit to.
	 */
	uint32_t dstWidth;

	/**
	 * @brief The height of the destination image to blit to.
	 */
	uint32_t dstHeight;

	/**
	 * @brief The number of depth levels or array levels to blit to.
	 *
	 * When using texture arrays, this must match srcDepthRange.
	 */
	uint32_t dstDepthRange;
} dsTextureBlitRegion;

/**
 * @brief Structure defining a surface to render to within a framebuffer.
 */
typedef struct dsFramebufferSurface
{
	/**
	 * @brief The type of the surface.
	 */
	dsFramebufferSurfaceType surfaceType;

	/**
	 * @brief The surface.
	 *
	 * This will be dsRenderSurface* if surfaceType is dsFramebufferSurfaceType_RenderSUrface or
	 * dsOffscreen* if surfaceType is dsFramebufferSurfaceType_Offscreen.
	 */
	void* surface;
} dsFramebufferSurface;

/**
 * @brief Structure defining a framebuffer, which is a set of surfaces to render to.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
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
	 * @brief The number of array layers.
	 */
	uint32_t layers;
} dsFramebuffer;

/**
 * @brief Struct for a resource context.
 *
 * A resource context must be created for each thread that manages resources. The context will be
 * globally bound for that thread when created, then un-bound when finally destroyed. (generally
 * when a thread will exit)
 *
 * This is declared here for internal use, and implementations will provide the final definition.
 */
typedef struct dsResourceContext dsResourceContext;

/**
 * @brief Returns whether or not a format is supported.
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
 * @param size The size of the buffer.
 * @param data The initial data for the buffer, or NULL to leave uninitialized.
 * @return The created buffer, or NULL if it couldn't be created.
 */
typedef dsGfxBuffer* (*dsCreateGfxBufferFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, size_t size, const void* data);

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
	int flags, size_t offset, size_t size);

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
 * @param size The size of the data to copy.
 * @param data The data to copy to the buffer.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyGfxBufferDataFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer, size_t offset, size_t size,
	const void* data);

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
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param size The size of the data. This is used to help catch mismatched data.
 * @param data The initial data for the texture, or NULL to leave uninitialized. The order of the
 *     data is:
 *     - Mip levels.
 *     - Array elements/depth slices.
 *     - Cube faces in the order of dsCubeFace.
 *     - Texture rows.
 *     Data is tightly packed.
 * @return The created texture, or NULL if it couldn't be created.
 */
typedef dsTexture* (*dsCreateTextureFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, size_t size,
	const void* data);

/**
 * @brief Function for creating an offscreen texture.
 * @param resourceManager The resource manager to create the offscreen from.
 * @param allocator The allocator to create the offscreen with.
 * @param usage How the offscreen will be used. This should be a combination of dsTextureUsage
 *     flags.
 * @param memoryHints Hints for how the memory for the offscreen will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param format The format of the offscreen.
 * @param dimension The dimension of the offscreen.
 * @param width The width of the offscreen.
 * @param height The height of the offscreen.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param samples The number of samples to use for multisampling.
 * @param resolve True to resolve multisampled offscreens, false to leave unresolved to sample in
 *     the shader.
 * @return The created offscreen, or NULL if it couldn't be created.
 */
typedef dsOffscreen* (*dsCreateOffscreenFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, int usage, int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint16_t samples,
	bool resolve);

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
 * @param size The size of the data. This is used to help catch mismatched data.
 * @param data The texture data to copy. This must be tightly packed.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyTextureDataFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* texture, const dsTexturePosition* position,
	uint32_t width, unsigned height, size_t size, const void* data);

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
 * @param regionCount The number of regions to copy.
 * @param regions The regions to copy.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyTextureFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsCubeFace srcFace,
	dsTexture* dstTexture, size_t regionCount, const dsTextureCopyRegion* regions);

/**
 * @brief Function for blitting from one texture to another, scaling when necessary.
 * @param resourceManager The resource manager the textures were created with.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcTexture The texture to blit from.
 * @param dstTexture The texture to blit to.
 * @param regionCount The number of regions to blit.
 * @param regions The regions to blit.
 * @param filter The filter to use when scaling is required.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsBlitTextureFunction)(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsTexture* srcTexture, dsCubeFace srcFace,
	dsTexture* dstTexture, size_t regionCount, const dsTextureBlitRegion* regions,
	dsFilter filter);

/**
 * @brief Function for getting texture data.
 * @param[out] result The output buffer for the result.
 * @param resourceManager The resource manager the texture was created with.
 * @param texture The texture to read from.
 * @param position The position to read from.
 * @param width The width of the texture region.
 * @param height The height of the texture region.
 * @param size The size of the buffer.
 * @return False if the data couldn't be read.
 */
typedef bool (*dsGetTextureDataFunction)(void* result, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	size_t size);

/**
 * @brief Function for creating a shader module.
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with.
 * @param module The MSL shader module.
 * @return The created shader module, or NULL if it couldn't be created.
 */
typedef dsShaderModule* (*dsCreateShaderModuleFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, mslModule* module);

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
 * @param elementCount The number of elements.
 * @param elements An array of elements for the description.
 * @return The created material description, or NULL if it couldn't be created.
 */
typedef dsMaterialDesc* (*dsCreateMaterialDescFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, uint32_t elementCount, const dsMaterialElement* elements);

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
 * @param elementCount The number of elements.
 * @param elements An array of elements for the description.
 * @return The created shader variable group description, or NULL if it couldn't be created.
 */
typedef dsShaderVariableGroupDesc* (*dsCreateShaderVariableGroupDescFunction)(
	dsResourceManager* resourceManager, dsAllocator* allocator, uint32_t elementCount,
	const dsMaterialElement* elements);

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
 * @brief Function for creating a shader.
 * @param resourceManager The resource manager to create the shader from.
 * @param allocator The allocator to create the shader with.
 * @param module The shader module that contains the shader.
 * @param name The name of the shader.
 * @param materialDesc The description of the material type used by the shader.
 * @return The created shader, or NULL if it couldn't be created.
 */
typedef dsShader* (*dsCreateShaderFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* module, const char* name,
	const dsMaterialDesc* materialDesc);

/**
 * @brief Function for destroying a shader.
 * @param resourceManager The resource manager the shader was created with.
 * @param shader The shader.
 * @return False if the shader couldn't be destroyed.
 */
typedef bool (*dsDestroyShaderFunction)(dsResourceManager* resourceManager, dsShader* shader);

/**
 * @brief Function for creeating a framebuffer.
 * @param resourceManager The resource manager to create the framebuffer from.
 * @param allocator The allocator to create the framebuffer with.
 * @param surfaces The surfaces that make up the framebuffer.
 * @param surfaceCount The number of surfaces.
 * @param width The width of the framebuffer.
 * @param height The height of the framebuffer.
 * @param layers The number of array layers in the framebuffer.
 * @return The created framebuffer, or NULL if it couldn't be created.
 */
typedef dsFramebuffer* (*dsCreateFramebufferFunction)(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, uint32_t layers);

/**
 * @brief Function for destroying a framebuffer.
 * @param resourceManager The resource manager the framebuffer was created with.
 * @param framebuffer The framebuffer.
 * @return False if the framebuffer couldn't be destroyed.
 */
typedef bool (*dsDestroyFramebufferFunction)(dsResourceManager* resourceManager,
	dsFramebuffer* framebuffer);

/** @copydoc dsResourceManager */
struct dsResourceManager
{
	// Public members

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

	/**
	 * @brief The number of resource contexts that may be created for other threads.
	 */
	uint32_t maxResourceContexts;

	/**
	 * @brief The minimum alignment when mapping the range of a buffer.
	 */
	uint32_t minMappingAlignment;

	/**
	 * @brief Bitmask for the supported buffer types.
	 */
	dsGfxBufferUsage supportedBuffers;

	/**
	 * @brief Enum describing how buffers may be mapped.
	 */
	dsGfxBufferMapSupport bufferMapSupport;

	/**
	 * @brief The maximum number of bits for each index.
	 */
	uint32_t maxIndexBits;

	/**
	 * @brief The maximum number of vertex attributes.
	 *
	 * This must not be greater than DS_MAX_ALLOWED_VERTEX_ATTRIBS.
	 */
	uint32_t maxVertexAttribs;

	/**
	 * @brief Whether or not instanced drawing is supported.
	 */
	bool supportsInstancedDrawing;

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
	 * @brief Boolean for whether or not textures are readable.
	 *
	 * Offscreens will always be readable.
	 */
	bool texturesReadable;

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
	 * @brief The number of shader modules currently allocated by the resource manager.
	 */
	uint32_t shaderModuleCount;

	/**
	 * @brief The number of material descriptions currently allocated by the resource manager.
	 */
	uint32_t materialDescCount;

	/**
	 * @brief The number of materials currently allocated by the resource manager.
	 */
	uint32_t materialCount;

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
	 * @brief The number of framebuffers currently allocated by the resource manager.
	 */
	uint32_t framebufferCount;

	/**
	 * @brief The number of bytes allocated for graphics buffers.
	 */
	size_t bufferMemorySize;

	/**
	 * @brief The number of bytes allocated for textures.
	 */
	size_t textureMemorySize;

	// Private members

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
	 * @brief Texture blitting function.
	 */
	dsBlitTextureFunction blitTextureFunc;

	/**
	 * @brief Texture data getting function.
	 */
	dsGetTextureDataFunction getTextureDataFunc;

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
	 * @brief Shader creation function.
	 */
	dsCreateShaderFunction createShaderFunc;

	/**
	 * @brief Shader destruction function.
	 */
	dsDestroyShaderFunction destroyShaderFunc;

	/**
	 * @brief Function for creating a framebuffer.
	 */
	dsCreateFramebufferFunction createFramebufferFunc;

	/**
	 * @brief Function for destroying a framebuffer.
	 */
	dsDestroyFramebufferFunction destroyFramebufferFunc;
};

#ifdef __cplusplus
}
#endif
