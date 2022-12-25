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

#include <xmmintrin.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Types and intrinsics for SIMD on x86 CPUs.
 */

/**
 * @brief Define for whether or not SIMD instructions are available in general.
 */
#define DS_HAS_SIMD 1

/// @cond
#if DS_CLANG
#define DS_SIMD_START_FLOAT4() \
	_Pragma("clang attribute push(__attribute__((target(\"sse\"))), apply_to = function)")
#define DS_SIMD_START_HADD() \
	_Pragma("clang attribute push(__attribute__((target(\"sse,sse3\"))), apply_to = function)")
#define DS_SIMD_START_FMA() \
	_Pragma("clang attribute push(__attribute__((target(\"sse,fma\"))), apply_to = function)")
#define DS_SIMD_START_HALF_FLOAT() \
	_Pragma("clang attribute push(__attribute__((target(\"sse,sse2,f16c\"))), apply_to = function)")
#define DS_SIMD_END() _Pragma("clang attribute pop")
#elif DS_GCC
#define DS_SIMD_START_FLOAT4() \
	_Pragma("GCC push_options") \
	_Pragma("GCC target(\"sse\")")
#define DS_SIMD_START_HADD() \
	_Pragma("GCC push_options") \
	_Pragma("GCC target(\"sse,sse3\")")
#define DS_SIMD_START_FMA() \
	_Pragma("GCC push_options") \
	_Pragma("GCC target(\"sse,fma\")")
#define DS_SIMD_START_HALF_FLOAT() \
	_Pragma("GCC push_options") \
	_Pragma("GCC target(\"sse,sse2,f16c\")")
#define DS_SIMD_END() _Pragma("GCC pop_options")
#else
#define DS_SIMD_START_FLOAT4()
#define DS_SIMD_START_HADD()
#define DS_SIMD_START_FMA()
#define DS_SIMD_START_HALF_FLOAT()
#define DS_SIMD_END()
#endif

#if DS_X86_64 || defined(__SSE__) || _M_IX86_FP >= 1
#define DS_ALWAYS_SIMD_FLOAT4 1
#else
#define DS_ALWAYS_SIMD_FLOAT4 0
#endif

#if defined(__SSE3__) || (DS_WINDOWS && defined(__AVX__))
#define DS_ALWAYS_SIMD_HADD 1
#else
#define DS_ALWAYS_SIMD_HADD 0
#endif

#if defined(__FMA__) || (DS_WINDOWS && defined(__AVX2__))
#define DS_ALWAYS_SIMD_FMA 1
#else
#define DS_ALWAYS_SIMD_FMA 0
#endif

#if defined(__F16C__)
#define DS_ALWAYS_SIMD_HALF_FLOAT 1
#else
#define DS_ALWAYS_SIMD_HALF_FLOAT 0
#endif
/// @endcond

/**
 * @brief Type for a SIMD vector of 4 floats.
 */
typedef __m128 dsSIMD4f;

/**
 * @brief Type for a SIMD vector of 4 half floats.
 */
typedef __m128i dsSIMD4hf;

/**
 * @brief Loads float values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param fp A pointer to the float values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
#define dsSIMD4f_load(fp) _mm_load_ps((const float*)(fp))

/**
 * @brief Sets a float value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param f The value to set.
 */
#define dsSIMD4f_set1(f) _mm_set_ps1((f))

/**
 * @brief Stores a SIMD register into four float values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] fp A pointer to the float values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
#define dsSIMD4f_store(fp, a) _mm_store_ps((float*)(fp), (a))

/**
 * @brief Negates a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to negate.
 * @return The result of -a.
 */
#define dsSIMD4f_neg(a) _mm_sub_ps(_mm_set_ps1(0.0f), (a))

/**
 * @brief Adds two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
#define dsSIMD4f_add(a, b) _mm_add_ps((a), (b))

/**
 * @brief Subtracts two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
#define dsSIMD4f_sub(a, b) _mm_sub_ps((a), (b))

/**
 * @brief Multiplies two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @return The result of a*b.
 */
#define dsSIMD4f_mul(a, b) _mm_mul_ps((a), (b))

/**
 * @brief Divides two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to divide.
 * @param b The second value to divide.
 * @return The result of a/b.
 */
#define dsSIMD4f_div(a, b) _mm_div_ps((a), (b))

/**
 * @brief Takes the approximate reciprical of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The approximate result of 1/a.
 */
#define dsSIMD4f_rcp(a) _mm_rcp_ps((a))

/**
 * @brief Takes the square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The result of sqrt(a).
 */
#define dsSIMD4f_sqrt(a) _mm_sqrt_ps((a))

/**
 * @brief Takes the approximate reciprical square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The approximate result of 1/sqrt(a).
 */
#define dsSIMD4f_rsqrt(a) _mm_rsqrt_ps((a))

/**
 * @brief Transposes the values in 4 SIMD vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first SIMD values.
 * @param b The second SIMD values.
 * @param c The third SIMD values.
 * @param d The fourth SIMD values.
 */
#define dsSIMD4f_transpose(a, b, c, d) _MM_TRANSPOSE4_PS(a, b, c, d)

/**
 * @brief Performs a horizontal add between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_HAdd is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of (a.x + a.y, a.z + a.w, b.x + b.y, b.z + b.w)
 */
#define dsSIMD4f_hadd(a, b) _mm_hadd_ps((a), (b))

/**
 * @brief Performs a fused multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of a*b + c.
 */
#define dsSIMD4f_fmadd(a, b, c) _mm_fmadd_ps((a), (b), (c))

/**
 * @brief Performs a fused multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of a*b - c.
 */
#define dsSIMD4f_fmsub(a, b, c) _mm_fmsub_ps((a), (b), (c))

/**
 * @brief Performs a fused negate multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of -(a*b) + c.
 */
#define dsSIMD4f_fnmadd(a, b, c) _mm_fnmadd_ps((a), (b), (c))

/**
 * @brief Performs a fused negate multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of -(a*b) - c.
 */
#define dsSIMD4f_fnmsub(a, b, c) _mm_fnmsub_ps((a), (b), (c))

/**
 * @brief Loads a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float value to load.
 * @return The loaded SIMD value.
 */
#define dsSIMD4hf_load1(hfp) _mm_cvtsi32_si128(*(const unsigned short*)(hfp))

/**
 * @brief Loads two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
#define dsSIMD4hf_load2(hfp) _mm_cvtsi32_si128(*(const unsigned int*)(hfp))

/**
 * @brief Loads four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
#define dsSIMD4hf_load4(hfp) _mm_loadu_si64((hfp))

/**
 * @brief Stores a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float value to store.
 * @param a The SIMD value to store.
 */
#define dsSIMD4hf_store1(hfp, a) (void)(*(short*)(hfp) = (short)_mm_cvtsi128_si32((a)))

/**
 * @brief Stores two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
#define dsSIMD4hf_store2(hfp, a) (void)(*(int*)(hfp) = _mm_cvtsi128_si32((a)))

/**
 * @brief Stores four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
#define dsSIMD4hf_store4(hfp, a) _mm_storeu_si64((hfp), a)

/**
 * @brief Converts SIMD floats to half floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
#define dsSIMD4hf_fromFloat(a) _mm_cvtps_ph((a), 0)

/**
 * @brief Converts SIMD half floats to floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
#define dsSIMD4hf_toFloat(a) _mm_cvtph_ps((a))

#ifdef __cplusplus
}
#endif
