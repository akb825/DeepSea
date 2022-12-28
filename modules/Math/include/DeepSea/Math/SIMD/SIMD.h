/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Math/Export.h>

#if DS_X86_32 || DS_X86_64
#include <DeepSea/Math/SIMD/SIMDx86.h>
#elif DS_ARM_32 || DS_ARM_64
#include <DeepSea/Math/SIMD/SIMDNeon.h>
#else
/**
 * @brief Define for whether or not SIMD instructions are available in general.
 */
#define DS_HAS_SIMD 0

/**
 * @brief Define for whether or not SIMD instructions for 4 floats are always available.
 */
#define DS_SIMD_ALWAYS_FLOAT4 0

/**
 * @brief Define for whether or not SIMD instructions for horizontal adds are always  available.
 */
#define DS_SIMD_ALWAYS_HADD 0

/**
 * @brief Define for whether or not SIMD instructions for converting half floats will always be
 * available.
 */
#define DS_SIMD_ALWAYS_FMA 0

/**
 * @brief Define for whether or not SIMD instructions for converting half floats are always
 * available.
 */
#define DS_SIMD_ALWAYS_HALF_FLOAT 0

/**
 * @brief Starts a block that uses 4-float SIMD instructions.
 */
#define DS_SIMD_START_FLOAT4()

/**
 * @brief Starts a block that uses horizontal add SIMD instructions.
 *
 * This implies DS_SIMD_START_FLOAT4().
 */
#define DS_SIMD_START_HADD()

/**
 * @brief Starts a block that uses fused multiply-add SIMD instructions.
 *
 * This implies DS_SIMD_START_FLOAT4().
 */
#define DS_SIMD_START_FMA()

/**
 * @brief Starts a block that uses half-float SIMD instructions.
 *
 * This implies DS_SIMD_START_FLOAT4().
 */
#define DS_SIMD_START_HALF_FLOAT()

/**
 * @brief Ends a previous SIMD start block.
 *
 * If multiple start blocks where used each must be separately ended with DS_SMD_END().
 */
#define DS_SIMD_END()
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Types and wrappers for intrinsics for writing SIMD code.
 *
 * If DS_HAS_SIMD is 0 nothing in this file will be available. This will generally happen for
 * hardware targets that don't have an implementation available.
 */

#if DS_HAS_SIMD

/**
 * @brief Enum for SIMD features that are supported by the CPU.
 */
typedef enum dsSIMDFeatures
{
	dsSIMDFeatures_None = 0,       ///< No SIMD features are supported.
	dsSIMDFeatures_Float4 = 0x1,   ///< Standard 4 element float operations.
	dsSIMDFeatures_HAdd = 0x2,     ///< Horizontal adds.
	dsSIMDFeatures_FMA = 0x4,      ///< Fused multiply adds.
	dsSIMDFeatures_HalfFloat = 0x8 ///< Half float conversions.
} dsSIMDFeatures;

/**
 * @brief Constant holding the SIMD features for the current host CPU.
 */
DS_MATH_EXPORT extern const dsSIMDFeatures dsHostSIMDFeatures;

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif

#if DS_HAS_SIMD
DS_ENUM_BITMASK_OPERATORS(dsSIMDFeatures);
#endif // DS_HAS_SIMD
