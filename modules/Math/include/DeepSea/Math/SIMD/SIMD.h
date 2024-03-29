/*
 * Copyright 2022-2023 Aaron Barany
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
 * @brief Define for whether or not SIMD instructions for 2 doubles are always available.
 */
#define DS_SIMD_ALWAYS_DOUBLE2 0

/**
 * @brief Define for whether or not SIMD instructions for 4 doubles are always available.
 */
#define DS_SIMD_ALWAYS_DOUBLE4 0

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
 * @brief Token to enable 4-float SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_FLOAT4

/**
 * @brief Token to enable 2-double SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_DOUBLE2

/**
 * @brief Token to enable 4-double SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_DOUBLE4

/**
 * @brief Token to enable horizontal add SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_HADD

/**
 * @brief Token to enable fused multiply-add SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_FMA

/**
 * @brief Token to enable half-float SIMD instructions.
 *
 * This should be provided as an argument to DS_SIMD_START().
 */
#define DS_SIMD_HALF_FLOAT

/**
 * @brief Starts a block that uses SIMD instructions.
 * @remark Due to limitations with the preprocessor and GCC pragmas, the parameters MUST NOT
 * be separated by spaces.
 */
#define DS_SIMD_START(...)

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
 *
 * dsSIMDFeatures_Float4 is the base feature level to SIMD support. In other words, it can be
 * assumed that if any other feature is available, Float4 will be available as well. On all current
 * platforms if Double4 is available the other math-based features (e.g. HAdd, FMA) will be
 * available as well, so there's no benefit for providing e.g. Double4 implementations with or
 * without FMA for compatibility.
 *
 * The current platforms are supported based on the platform macro:
 * - DS_X86_32: No features will be guaranteed at compile time unless explicitly enabled through
 *   compiler flags. (e.g. with -march) All features may be available on the host CPU and can be
 *   queried at runtime.
 * - DS_X86_64: Float4 and Double2 will be guaranteed at compile time, though more may be explicitly
 *   enabled through compiler flags. (e.g. with -march) All other features may be available on the
 *   host CPU and can be queried at runtime.
 * - DS_ARM_32: All features except for Double2 and Double4 are guaranteed to be available at compile
 *   time. No additional features will be detected at runtime.
 * - DS_ARM_64: All features except for Double4 are guaranteed to be available at compile time. No
 *   additional features will be detected at runtime.
 */
typedef enum dsSIMDFeatures
{
	dsSIMDFeatures_None = 0,        ///< No SIMD features are supported.
	dsSIMDFeatures_Float4 = 0x1,    ///< Standard 4 element float operations.
	dsSIMDFeatures_Double2 = 0x2,   ///< Standard 2 element double operations.
	dsSIMDFeatures_Double4 = 0x4,   ///< Standard 4 element double operations.
	dsSIMDFeatures_HAdd = 0x8,      ///< Horizontal adds.
	dsSIMDFeatures_FMA = 0x10,      ///< Fused multiply adds.
	dsSIMDFeatures_HalfFloat = 0x20 ///< Half float conversions.
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
