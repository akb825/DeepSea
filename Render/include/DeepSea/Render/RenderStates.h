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
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Collection of enums and structs for render states.
 */

/**
 * @brief Constant for no known value.
 */
#define DS_UNKNOWN (uint32_t)-1

/**
 * @brief Constant for no known float value.
 */
#define DS_UNKNOWN_FLOAT FLT_MAX

/**
 * @brief Constant for the maximum number of color attachments for a pixel shader.
 */
#define DS_MAX_ATTACHMENTS 16

/**
 * @brief Enum for a boolean value that may be unset.
 */
typedef enum dsBool
{
	dsBool_Unset = -1, ///< No value set.
	dsBool_False,      ///< false
	dsBool_True        ///< true
} dsBool;

/**
 * @brief Enum for the polygon mode.
 */
typedef enum dsPolygonMode
{
	dsPolygonMode_Unset = -1, ///< No value set.
	dsPolygonMode_Fill,       ///< Draw filled polygons.
	dsPolygonMode_Line,       ///< Draw outlines of polygons.
	dsPolygonMode_Point       ///< Draw points for each vertex.
} dsPolygonMode;

/**
 * @brief Enum for the cull mode.
 */
typedef enum dsCullMode
{
	dsCullMode_Unset = -1,  ///< No value set.
	dsCullMode_None,        ///< Don't cull any faces.
	dsCullMode_Front,       ///< Cull front faces.
	dsCullMode_Back,        ///< Cull back faces.
	dsCullMode_FrontAndBack ///< Cull front and back faces.
} dsCullMode;

/**
 * @brief Enum for the front face.
 */
typedef enum dsFrontFace
{
	dsFrontFace_Unset = -1,       ///< No value set.
	dsFrontFace_CounterClockwise, ///< Counter-clockwise faces are front.
	dsFrontFace_Clockwise         ///< Clockwise faces are front.
} dsFrontFace;

/**
 * @brief Enum for stencil operations.
 */
typedef enum dsStencilOp
{
	dsStencilOp_Unset = -1,        ///< No value set.
	dsStencilOp_Keep,              ///< Keep the current value.
	dsStencilOp_Zero,              ///< Set the value to 0.
	dsStencilOp_Replace,           ///< Replace the current value.
	dsStencilOp_IncrementAndClamp, ///< Increment the value, clamping to the maximum.
	dsStencilOp_DecrementAndClamp, ///< Decrement the value, clamping to 0.
	dsStencilOp_Invert,            ///< Inverts the bits of the value.
	dsStencilOp_IncrementAndWrap,  ///< Increments the value, wrapping around to 0.
	dsStencilOp_DecrementAndWrap   ///< Decrements the value, wrapping around to the maximum.
} dsStencilOp;

/**
 * @brief Enum for compare operations.
 */
typedef enum dsCompareOp
{
	dsCompareOp_Unset = -1,     ///< No value set.
	dsCompareOp_Never,          ///< Never succeed.
	dsCompareOp_Less,           ///< Check if a < b.
	dsCompareOp_Equal,          ///< Check if a == b.
	dsCompareOp_LessOrEqual,    ///< Check if a <= b.
	dsCompareOp_Greater,        ///< Check if a > b.
	dsCompareOp_NotEqual,       ///< Check if a != b.
	dsCompareOp_GreaterOrEqual, ///< Check if a >= b.
	dsCompareOp_Always          ///< Always succeed.
} dsCompareOp;

/**
 * @brief Enum for a blend factor.
 */
typedef enum dsBlendFactor
{
	dsBlendFactor_Unset = -1,         ///< No value set.
	dsBlendFactor_Zero,               ///< Value of 0.
	dsBlendFactor_One,                ///< Value of 1.
	dsBlendFactor_SrcColor,           ///< Source color. (from the current fragment)
	dsBlendFactor_OneMinusSrcColor,   ///< 1 - source color.
	dsBlendFactor_DstColor,           ///< Destination color. (from the framebuffer)
	dsBlendFactor_OneMinusDstColor,   ///< 1 - destination color.
	dsBlendFactor_SrcAlpha,           ///< Source alpha. (from the current fragment)
	dsBlendFactor_OneMinusSrcAlpha,   ///< 1 - source alpha.
	dsBlendFactor_DstAlpha,           ///< Destination alpha. (from the framebuffer)
	dsBlendFactor_OneMinusDstAlpha,   ///< 1 - destination alpha.
	dsBlendFactor_ConstColor,         ///< Constant user-specified color.
	dsBlendFactor_OneMinusConstColor, ///< 1 - const color.
	dsBlendFactor_ConstAlpha,         ///< Constant uer-specified alpha.
	dsBlendFactor_OneMinusConstAlpha, ///< 1 - const alpha.
	dsBlendFactor_SrcAlphaSaturate,   ///< Source alpha, clamped the range [0, 1].
	dsBlendFactor_Src1Color,          ///< Source color from the secondary color buffer.
	dsBlendFactor_OneMinusSrc1Color,  ///< 1 - secondary source color.
	dsBlendFactor_Src1Alpha,          ///< Source alpha from the secondary color buffer.
	dsBlendFactor_OneMinusSrc1Alpha   ///< 1 - secondary source alpha.
} dsBlendFactor;

/**
 * @brief Enum for a blend operation.
 */
typedef enum dsBlendOp
{
	dsBlendOp_Unset = -1,      ///< No value set.
	dsBlendOp_Add,             ///< Evaluates a + b.
	dsBlendOp_Subtract,        ///< Evaluates a - b.
	dsBlendOp_ReverseSubtract, ///< Evaluates b - a.
	dsBlendOp_Min,             ///< Evaluates min(a, b).
	dsBlendOp_Max              ///< Evaluates max(a, b).
} dsBlendOp;

/**
 * @brief Enum for a color mask.
 *
 * These values can be OR'd together.
 */
typedef enum dsColorMask
{
	dsColorMask_Unset =  -1, ///< No value set.
	dsColorMask_None =    0, ///< Write no color channels.
	dsColorMask_Red =   0x1, ///< Write the red channel.
	dsColorMask_Green = 0x2, ///< Write the green channel.
	dsColorMask_Blue =  0x4, ///< Write the blue channel.
	dsColorMask_Alpha = 0x8  ///< Write the alpha channel.
} dsColorMask;

/**
 * @brief Enum for a logical operation.
 */
typedef enum dsLogicOp
{
	dsLogicOp_Unset = -1,   ///< No value set.
	dsLogicOp_Clear,        ///< Clear the value to 0.
	dsLogicOp_And,          ///< Evaluate a & b.
	dsLogicOp_AndReverse,   ///< Evaluate a & ~b.
	dsLogicOp_Copy,         ///< Copy a to b.
	dsLogicOp_AndInverted,  ///< Evaluate ~a & b.
	dsLogicOp_NoOp,         ///< Don't modify the value.
	dsLogicOp_Xor,          ///< Evaluate a ^ b.
	dsLogicOp_Or,           ///< Evaluate a | b.
	dsLogicOp_Nor,          ///< Evaluate ~(a | b).
	dsLogicOp_Equivalent,   ///< Evaluate ~(a ^ b).
	dsLogicOp_Invert,       ///< Evaluate ~b.
	dsLogicOp_OrReverse,    ///< Evaluate a | ~b.
	dsLogicOp_CopyInverted, ///< Evaluate ~a.
	dsLogicOp_OrInverted,   ///< Evaluate ~a | b.
	dsLogicOp_Nand,         ///< Evaluate ~(a & b).
	dsLogicOp_Set           ///< Set the value to all 1.
} dsLogicOp;

/**
 * @brief Enum for how to filter a texture.
 */
typedef enum dsFilter
{
	dsFilter_Unset = -1, ///< No value set.
	dsFilter_Nearest,    ///< Nearest-neighbor filtering.
	dsFilter_Linear      ///< Linear filtering.
} dsFilter;

/**
 * @brief Enum for how to filter between mips.
 */
typedef enum dsMipFilter
{
	dsMipFilter_Unset = -1, ///< No value set.
	dsMipFilter_None,       ///< No mip-mapping.
	dsMipFilter_Nearest,    ///< Nearest-neighbor filtering.
	dsMipFilter_Linear,     ///< Linear filtering.
	dsMipFilter_Anisotropic ///< Anisotropic filtering.
} dsMipFilter;

/**
 * @brief Enum for how to handle texture addressing.
 */
typedef enum dsAddressMode
{
	dsAddressMode_Unset = -1,     ///< No value set.
	dsAddressMode_Repeat,         ///< Repeat the texture beyond the boundary.
	dsAddressMode_MirroredRepeat, ///< Repeat the textore, mirroring on each odd repeat.
	dsAddressMode_ClampToEdge,    ///< Clamp to the edge, using the texture value along the edge.
	dsAddressMode_ClampToBorder,  ///< Clamp to the edge, using the border color.
	dsAddressMode_MirrorOnce      ///< Mirror the texture once before clamping it.
} dsAddressMode;

/**
 * @brief Enum for the border color when using AddressMode ClampToBorder.
 */
typedef enum dsBorderColor
{
	dsBorderColor_Unset = -1,         ///< No value set.
	dsBorderColor_TransparentBlack,   ///< All color channels and alpha 0.
	dsBorderColor_TransparentIntZero, ///< All color channels and alpha 0. (as integers)
	dsBorderColor_OpaqueBlack,        ///< Color channels 0, alpha value 1.
	dsBorderColor_OpaqueIntZero,      ///< Color channels 0, alpha value as the int value 1.
	dsBorderColor_OpaqueWhite,        ///< All color channelsa and alpha 1.
	dsBorderColor_OpaqueIntOne        ///< All color channels and alpha as the int value 1.
} dsBorderColor;

/**
 * @brief Structure holding the render states used for rasterization.
 */
typedef struct dsRasterizationState
{
	/**
	 * @brief Clamp the depth values in range rather than clipping.
	 */
	dsBool depthClampEnable;

	/**
	 * @brief Whether or not to discard all samples.
	 */
	dsBool rasterizerDiscardEnable;

	/**
	 * @brief Mode for how to draw polygons.
	 */
	dsPolygonMode polygonMode;

	/**
	 * @brief Mode for how to cull front and back faces.
	 */
	dsCullMode cullMode;

	/**
	 * @brief The polygon front face.
	 */
	dsFrontFace frontFace;

	/**
	 * @brief Whether or not to use depth bias.
	 */
	dsBool depthBiasEnable;

	/**
	 * @brief The constant depth bias to apply.
	 */
	float depthBiasConstantFactor;

	/**
	 * @brief The minimum or maximum value to clamp the depth bias to.
	 */
	float depthBiasClamp;

	/**
	 * @brief The depth bias to apply based on the slope of the polygon.
	 */
	float depthBiasSlopeFactor;

	/**
	 * @brief The width of lines.
	 */
	float lineWidth;
} dsRasterizationState;

/**
 * @brief Structure for holding multisampling render states.
 */
typedef struct dsMultisampleState
{
	/**
	 * @brief Whether or not to run the shader for multiple samples.
	 */
	dsBool sampleShadingEnable;

	/**
	 * @brief Hint for how many samples to run the shader on.
	 */
	float minSampleShading;

	/**
	 * @brief Mask for which samples to run the shader on.
	 */
	uint32_t sampleMask;

	/**
	 * @brief Whether or not to use the alpha value to control how many samples to use.
	 */
	dsBool alphaToCoverageEnable;

	/**
	 * @brief Whether or not to force the alpha value to 1.
	 */
	dsBool alphaToOneEnable;
} dsMultisampleState;

/**
 * @brief Structure for holding the stencil state.
 */
typedef struct dsStencilOpState
{
	/**
	 * @brief The operation to perform when failing the stencil test.
	 */
	dsStencilOp failOp;

	/**
	 * @brief The operation to perform when passing the stencil test.
	 */
	dsStencilOp passOp;

	/**
	 * @brief The operation to perform when failing the depth test.
	 */
	dsStencilOp depthFailOp;

	/**
	 * @brief The compare operation for stencil values.
	 */
	dsCompareOp compareOp;

	/**
	 * @brief The mask to apply to the values for comparisson.
	 */
	uint32_t compareMask;

	/**
	 * @brief The mask to apply to the value before writing to the stencil buffer.
	 */
	uint32_t writeMask;

	/**
	 * @brief Constant reference value.
	 */
	uint32_t reference;
} dsStencilOpState;

/**
 * @brief Structure for holding the depth render states.
 */
typedef struct dsDepthStencilState
{
	/**
	 * @brief Whether or not to enable the depth test.
	 */
	dsBool depthTestEnable;

	/**
	 * @brief Whether or not to write the depth value to the depth buffer.
	 */
	dsBool depthWriteEnable;

	/**
	 * @brief The comparisson operation for depth values.
	 */
	dsCompareOp depthCompareOp;

	/**
	 * @brief Whether or not to limit the depth range.
	 */
	dsBool depthBoundsTestEnable;

	/**
	 * @brief Whether or not to enable the stencil test.
	 */
	dsBool stencilTestEnable;

	/**
	 * @brief Stencil operations for front faces.
	 */
	dsStencilOpState frontStencil;

	/**
	 * @brief Stencil operations for back faces.
	 */
	dsStencilOpState backStencil;

	/**
	 * @brief Minimum value when limiting the depth range.
	 */
	float minDepthBounds;

	/**
	 * @brief Maximum value when limiting the depth range.
	 */
	float maxDepthBounds;
} dsDepthStencilState;

/**
 * @brief Structure for holding the blend states for a color attachment.
 */
typedef struct dsBlendAttachmentState
{
	/**
	 * @brief Whether or not to enable blending.
	 */
	dsBool blendEnable;

	/**
	 * @brief Blend factor for the source color.
	 */
	dsBlendFactor srcColorBlendFactor;

	/**
	 * @brief Blend factor for the destination color.
	 */
	dsBlendFactor dstColorBlendFactor;

	/**
	 * @brief The operation to apply to the source and destination color factors.
	 */
	dsBlendOp colorBlendOp;

	/**
	 * @brief Blend factor for the source alpha.
	 */
	dsBlendFactor srcAlphaBlendFactor;

	/**
	 * @brief Blend factor for the destination alpha.
	 */
	dsBlendFactor dstAlphaBlendFactor;

	/**
	 * @brief The operation to apply to the source and destination alpha factors.
	 */
	dsBlendOp alphaBlendOp;

	/**
	 * @brief Mask of color channels to write to.
	 */
	dsColorMask colorWriteMask;
} dsBlendAttachmentState;

/**
 * @brief Structure for holding the blend states.
 */
typedef struct dsBlendState
{
	/**
	 * @brief Whether or not to enable logical operations.
	 */
	dsBool logicalOpEnable;

	/**
	 * @brief The logical operation to apply.
	 */
	dsLogicOp logicalOp;

	/**
	 * @brief Whether or not to apply separate blending operations for each attachment.
	 *
	 * If unset, only the first attachment blend states should be used.
	 */
	dsBool separateAttachmentBlendingEnable;

	/**
	 * @brief The blend states for each color attachment.
	 */
	dsBlendAttachmentState blendAttachments[DS_MAX_ATTACHMENTS];

	/**
	 * @brief The constant blend factor.
	 */
	float blendConstants[4];
} dsBlendState;

/**
 * @brief Structure for holding the render states.
 */
typedef struct dsRenderState
{
	/**
	 * @brief The rasterization states.
	 */
	dsRasterizationState rasterizationState;

	/**
	 * @brief The multisample states.
	 */
	dsMultisampleState multisampleState;

	/**
	 * @brief The depth-stencil states.
	 */
	dsDepthStencilState depthStencilState;

	/**
	 * @brief The blending states.
	 */
	dsBlendState blendState;

	/**
	 * @brief The number of control points for each patch for tessellation.
	 */
	uint32_t patchControlPoints;
} dsRenderState;

/**
 * @brief Structure for holding the states used for a texture sampler.
 */
typedef struct dsSamplerState
{
	/**
	 * @brief The filter used for minification.
	 */
	dsFilter minFilter;

	/**
	 * @brief The filter used for magnification.
	 */
	dsFilter magFilter;

	/**
	 * @brief The filter used for mip-mapping.
	 */
	dsMipFilter mipFilter;

	/**
	 * @brief How to address the U (or S) texture coordinate.
	 */
	dsAddressMode addressModeU;

	/**
	 * @brief How to address the U (or T) texture coordinate.
	 */
	dsAddressMode addressModeV;

	/**
	 * @brief How to address the W (or R) texture coordinate.
	 */
	dsAddressMode addressModeW;

	/**
	 * @brief Bias to apply when calculating the mip-mapping LOD.
	 */
	float mipLodBias;

	/**
	 * @brief The maximum anisotropy factor to apply.
	 */
	float maxAnisotropy;

	/**
	 * @brief The minimum mip level to use.
	 */
	float minLod;

	/**
	 * @brief The maximum mip level to use.
	 */
	float maxLod;

	/**
	 * @brief The border color to apply.
	 */
	dsBorderColor borderColor;
} dsSamplerState;

#ifdef __cplusplus
}
#endif
