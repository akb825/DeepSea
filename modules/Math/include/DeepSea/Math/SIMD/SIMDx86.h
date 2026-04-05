/*
 * Copyright 2022-2026 Aaron Barany
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

#include <immintrin.h>
#include <stdint.h>
#include <xmmintrin.h>

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
#if DS_CLANG || DS_GCC
#define DS_SIMD_FLOAT4 sse
#define DS_SIMD_INT sse2
#define DS_SIMD_DOUBLE2 sse2
#define DS_SIMD_DOUBLE4 avx2
#define DS_SIMD_HADD sse3
#define DS_SIMD_FMA fma
#define DS_SIMD_HALF_FLOAT sse2,f16c

#define DS_SIMD_PRAGMA_IMPL(x) _Pragma(#x)
#define DS_SIMD_PRAGMA(x) DS_SIMD_PRAGMA_IMPL(x)

#if DS_CLANG
#define DS_SIMD_PRAGMA_VALUE(options) \
	clang attribute push(__attribute__((target(options))), apply_to = function)
#define DS_SIMD_START_IMPL(options) DS_SIMD_PRAGMA(DS_SIMD_PRAGMA_VALUE(options))
#define DS_SIMD_END() _Pragma("clang attribute pop")
#else
#define DS_SIMD_PRAGMA_VALUE(options) \
	GCC target(options)
#define DS_SIMD_START_IMPL(options) \
	_Pragma("GCC push_options") \
	DS_SIMD_PRAGMA(DS_SIMD_PRAGMA_VALUE(options))
#define DS_SIMD_END() _Pragma("GCC pop_options")
#endif

#define DS_SIMD_STRINGIFY_ARGS_IMPL(...) #__VA_ARGS__
#define DS_SIMD_STRINGIFY_ARGS(...) DS_SIMD_STRINGIFY_ARGS_IMPL(__VA_ARGS__)

#define DS_SIMD_START(...) DS_SIMD_START_IMPL(DS_SIMD_STRINGIFY_ARGS(__VA_ARGS__))
#else
#define DS_SIMD_FLOAT4
#define DS_SIMD_DOUBLE2
#define DS_SIMD_DOUBLE4
#define DS_SIMD_HADD
#define DS_SIMD_FMA
#define DS_SIMD_HALF_FLOAT
#define DS_SIMD_START(...)
#define DS_SIMD_END()
#endif

#define DS_SIMD_EMULATED_DIV_SQRT 0

#if DS_X86_64 || defined(__SSE__) || _M_IX86_FP >= 1
#define DS_SIMD_ALWAYS_FLOAT4 1
#else
#define DS_SIMD_ALWAYS_FLOAT4 0
#endif

#if DS_X86_64 || defined(__SSE2__) || _M_IX86_FP >= 2
#define DS_SIMD_ALWAYS_INT 1
#define DS_SIMD_ALWAYS_DOUBLE2 1
#else
#define DS_SIMD_ALWAYS_INT 0
#define DS_SIMD_ALWAYS_DOUBLE2 0
#endif

#if defined(__AVX2__)
#define DS_SIMD_ALWAYS_DOUBLE4 1
#else
#define DS_SIMD_ALWAYS_DOUBLE4 0
#endif

#if defined(__SSE3__) || (DS_WINDOWS && defined(__AVX__))
#define DS_SIMD_ALWAYS_HADD 1
#else
#define DS_SIMD_ALWAYS_HADD 0
#endif

#if !DS_DETERMINISTIC_MATH && (defined(__FMA__) || (DS_WINDOWS && defined(__AVX2__)))
#define DS_SIMD_ALWAYS_FMA 1
#else
#define DS_SIMD_ALWAYS_FMA 0
#endif

#if defined(__F16C__)
#define DS_SIMD_ALWAYS_HALF_FLOAT 1
#else
#define DS_SIMD_ALWAYS_HALF_FLOAT 0
#endif
/// @endcond

/**
 * @brief Type for a SIMD vector of 4 floats.
 */
typedef __m128 dsSIMD4f;

/**
 * @brief Type for a SIMD vector of 2 doubles.
 */
typedef __m128d dsSIMD2d;

/**
 * @brief Type for a SIMD vector of 4 doubles.
 */
typedef __m256d dsSIMD4d;

/**
 * @brief Type for a SIMD vector of 4 bitfield results.
 *
 * Each bitfield value will be stored in a 32-bit value, and will often represent boolean values.
 */
#if DS_SIMD_ALWAYS_INT
typedef __m128i dsSIMD4fb;
#else
typedef __m128 dsSIMD4fb;
#endif

/**
 * @brief Type for a SIMD vector of 2 bitfield results.
 *
 * Each bitfield value will be stored in a 64-bit value, and will often represent boolean values.
 */
typedef __m128i dsSIMD2db;

/**
 * @brief Type for a SIMD vector of 4 bitfield results.
 *
 * Each bitfield value will be stored in a 64-bit value, and will often represent boolean values.
 */
typedef __m256i dsSIMD4db;

/**
 * @brief Type for a SIMD vector of 4 half floats.
 */
typedef __m128i dsSIMD4hf;

/// @cond
DS_SIMD_START(DS_SIMD_FLOAT4);

// NOTE: Use integer type directly for dsSIMD4fb when integer operations are guaranteed to be
// available for better type checking. If not guaranteed, use floating-point vector type for
// maximum compatibility.
#if DS_SIMD_ALWAYS_INT
#define DS_SSE_CAST_F4_TO_FB4(x) _mm_castps_si128((x))
#define DS_SSE_CAST_FB4_TO_F4(x) _mm_castsi128_ps((x))
#define DS_SSE_RESULT_FB4(x) (x)
#define DS_SSE_PARAM_FB4(x) (x)
#else
#define DS_SSE_CAST_F4_TO_FB4(x) (x)
#define DS_SSE_CAST_FB4_TO_F4(x) (x)
#define DS_SSE_RESULT_FB4(x) _mm_castsi128_ps((x))
#define DS_SSE_PARAM_FB4(x) _mm_castps_si128((x))
#endif

#if DS_X86_32
DS_ALWAYS_INLINE uint64_t dsSIMDDoubleToInt64(double x)
{
	return *(uint64_t*)&x;
}
#endif
/// @endcond

/**
 * @brief Loads float values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param fp A pointer to the float values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_load(const void* fp)
{
	return _mm_load_ps((const float*)fp);
}

/**
 * @brief Loads float values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param fp A pointer to the float values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_loadUnaligned(const void* fp)
{
	return _mm_loadu_ps((const float*)(fp));
}

/**
 * @brief Sets a float value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param f The value to set.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_set1(float f)
{
	return _mm_set1_ps(f);
}

/**
 * @brief Sets a SIMD value with four floats.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param x The first value.
 * @param y The second value.
 * @param z The third value.
 * @param w The fourth value.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_set4(float x, float y, float z, float w)
{
	return _mm_set_ps(w, z, y, x);
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#define dsSIMD4f_set1FromVec(a, i) _mm_shuffle_ps((a), (a), _MM_SHUFFLE((i), (i), (i), (i)))

/**
 * @brief Stores a SIMD register into four float values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] fp A pointer to the float values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4f_store(void* fp, dsSIMD4f a)
{
	_mm_store_ps((float*)fp, a);
}

/**
 * @brief Stores a SIMD register into four float values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] fp A pointer to the float values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4f_storeUnaligned(void* fp, dsSIMD4f a)
{
	_mm_storeu_ps((float*)fp, a);
}

/**
 * @brief Gets a float element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#define dsSIMD4f_get(a, i) _mm_cvtss_f32(_mm_shuffle_ps((a), (a), _MM_SHUFFLE(0, 0, 0, (i))))

/**
 * @brief Negates a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_neg(dsSIMD4f a)
{
	return _mm_xor_ps(_mm_set1_ps(-0.0f), a);
}

/**
 * @brief Negates specific components of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to negate.
 * @param negX Constant 1 to negate the X component or 0 to leave it unchanged.
 * @param negY Constant 1 to negate the Y component or 0 to leave it unchanged.
 * @param negZ Constant 1 to negate the Z component or 0 to leave it unchanged.
 * @param negW Constant 1 to negate the W component or 0 to leave it unchanged.
 * @return The result of the negation.
 */
#define dsSIMD4f_negComponents(a, negX, negY, negZ, negW) \
	_mm_xor_ps(_mm_set_ps((negW) ? -0.0f : 0.0f, (negZ) ? -0.0f : 0.0f, (negY) ? -0.0f : 0.0f, \
		(negX) ? -0.0f : 0.0f), (a))

/**
 * @brief Adds two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_add(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_add_ps(a, b);
}

/**
 * @brief Subtracts two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_sub(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_sub_ps(a, b);
}

/**
 * @brief Multiplies two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @return The result of a*b.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_mul(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_mul_ps(a, b);
}

/**
 * @brief Divides two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to divide.
 * @param b The second value to divide.
 * @return The result of a/b.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_div(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_div_ps(a, b);
}

/**
 * @brief Takes the square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the square root.
 * @return The result of sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_sqrt(dsSIMD4f a)
{
	return _mm_sqrt_ps(a);
}

/**
 * @brief Takes the reciprocal of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/a.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_rcp(dsSIMD4f a)
{
	return _mm_div_ps(_mm_set1_ps(1.0f), a);
}

/**
 * @brief Takes the reciprocal square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_rsqrt(dsSIMD4f a)
{
	return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(a));
}

/**
 * @brief Takes the absolute value of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the absolute value.
 * @return The result of abs(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_abs(dsSIMD4f a)
{
	return _mm_andnot_ps(_mm_set1_ps(-0.0f), a);
}

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
 * @brief Gets the minimum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to take the min of.
 * @param b The second value to take the min of.
 * @return The result of min(a, b).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_min(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_min_ps(a, b);
}

/**
 * @brief Gets the maximum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to take the max of.
 * @param b The second value to take the max of.
 * @return The result of max(a, b).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_max(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_max_ps(a, b);
}

/**
 * @brief Selects between two vectors based on a boolean mask.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The boolean mask to select with.
 * @param b The first SIMD values to select from.
 * @param c The second SIMD values to select from.
 * @return The result of a ? b : c.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_select(dsSIMD4fb a, dsSIMD4f b, dsSIMD4f c)
{
	return _mm_or_ps(
		_mm_and_ps(DS_SSE_CAST_FB4_TO_F4(a), b), _mm_andnot_ps(DS_SSE_CAST_FB4_TO_F4(a), c));
}

/**
 * @brief Checks if two SIMD values are equal.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a == b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmpeq(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmpeq_ps(a, b));
}

/**
 * @brief Checks if two SIMD values are not equal.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a != b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmpne(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmpneq_ps(a, b));
}

/**
 * @brief Checks if one SIMD values is less than another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a < b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmplt(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmplt_ps(a, b));
}

/**
 * @brief Checks if one SIMD values is less than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a <= b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmple(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmple_ps(a, b));
}

/**
 * @brief Checks if one SIMD values is greater than another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a > b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmpgt(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmpgt_ps(a, b));
}

/**
 * @brief Checks if one SIMD values is greater than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a >= b as a dsSIMD4fb.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4f_cmpge(dsSIMD4f a, dsSIMD4f b)
{
	return DS_SSE_CAST_F4_TO_FB4(_mm_cmpge_ps(a, b));
}

/**
 * @brief Creates a SIMD value for true.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @return A SIMD value with true on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_true(void)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_set1_epi32(0xFFFFFFFF);
#else
	const unsigned int a = 0xFFFFFFFF;
	return _mm_set1_ps(*(float*)&a);
#endif
}

/**
 * @brief Creates a SIMD value for false.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @return A SIMD value with false on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_false(void)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_setzero_si128();
#else
	return _mm_setzero_ps();
#endif
}

/**
 * @brief Loads int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param ip A pointer to the int values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_load(const void* ip)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_load_si128((const dsSIMD4fb*)ip);
#else
	return _mm_load_ps((const float*)ip);
#endif
}

/**
 * @brief Loads int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param ip A pointer to the int values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_loadUnaligned(const void* ip)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_loadu_si128((const dsSIMD4fb*)(ip));
#else
	return _mm_loadu_ps((const float*)(ip));
#endif
}

/**
 * @brief Converts a SIMD float value directly to a bitfield value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The SIMD floating-point value.
 * @return The direct bitfield conversion from float.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_fromFloatBitfield(dsSIMD4f a)
{
	return DS_SSE_CAST_F4_TO_FB4(a);
}

/**
 * @brief Stores a SIMD bitfield register into four int values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] ip A pointer to the int values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4fb_store(void* ip, dsSIMD4fb a)
{
#if DS_SIMD_ALWAYS_INT
	_mm_store_si128((dsSIMD4fb*)ip, a);
#else
	_mm_store_ps((float*)ip, a);
#endif
}

/**
 * @brief Stores a SIMD register into four int values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] ip A pointer to the float values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4fb_storeUnaligned(void* ip, dsSIMD4fb a)
{
#if DS_SIMD_ALWAYS_INT
	_mm_storeu_si128((dsSIMD4fb*)ip, a);
#else
	_mm_storeu_ps((float*)ip, a);
#endif
}

/**
 * @brief Converts a SIMD bitfield value directly to a float value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The SIMD bitfield value.
 * @return The direct bitfield conversion to float.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4fb_toFloatBitfield(dsSIMD4fb a)
{
	return DS_SSE_CAST_FB4_TO_F4(a);
}

/**
 * @brief Performs a logical not on a SIMD bitfield value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to not.
 * @return The result of !a.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_not(dsSIMD4fb a)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_xor_si128(a, dsSIMD4fb_true());
#else
	return _mm_xor_ps(a, dsSIMD4fb_true());
#endif
}

/**
 * @brief Performs a logical and between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to and.
 * @param b The second value to and.
 * @return The result of a & b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_and(dsSIMD4fb a, dsSIMD4fb b)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_and_si128(a, b);
#else
	return _mm_and_ps(a, b);
#endif
}

/**
 * @brief Performs a logical and not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to not then and.
 * @param b The second value to and.
 * @return The result of (!a) & b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_andnot(dsSIMD4fb a, dsSIMD4fb b)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_andnot_si128(a, b);
#else
	return _mm_andnot_ps(a, b);
#endif
}

/**
 * @brief Performs a logical or between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to or.
 * @param b The second value to or.
 * @return The result of a | b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_or(dsSIMD4fb a, dsSIMD4fb b)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_or_si128(a, b);
#else
	return _mm_or_ps(a, b);
#endif
}

/**
 * @brief Performs a logical or not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to or.
 * @param b The second value to not then or.
 * @return The result of a | (!b).
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_ornot(dsSIMD4fb a, dsSIMD4fb b)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_or_si128(a, dsSIMD4fb_not(b));
#else
	return _mm_or_ps(a, dsSIMD4fb_not(b));
#endif
}

/**
 * @brief Performs a logical xor between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to xor.
 * @param b The first value to xor.
 * @return The result of a ^ b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_xor(dsSIMD4fb a, dsSIMD4fb b)
{
#if DS_SIMD_ALWAYS_INT
	return _mm_xor_si128(a, b);
#else
	return _mm_xor_ps(a, b);
#endif
}

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT);
/// @endcond

/**
 * @brief Sets an int value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param i The value to set.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_set1(uint32_t i)
{
	return DS_SSE_RESULT_FB4(_mm_set1_epi32(i));
}

/**
 * @brief Sets a SIMD value with four int values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param x The first value.
 * @param y The second value.
 * @param z The third value.
 * @param w The fourth value.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_set4(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
	return DS_SSE_RESULT_FB4(_mm_set_epi32(w, z, y, x));
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#if DS_SIMD_ALWAYS_INT
#define dsSIMD4fb_set1FromVec(a, i) _mm_shuffle_epi32((a), _MM_SHUFFLE((i), (i), (i), (i)))
#else
#define dsSIMD4fb_set1FromVec(a, i) _mm_shuffle_ps((a), (a), _MM_SHUFFLE((i), (i), (i), (i)))
#endif

/**
 * @brief Converts a SIMD float value to an integer, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The SIMD floating-point value.
 * @return The integer representation as would be done with an int to float cast in C.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_fromFloat(dsSIMD4f a)
{
	return DS_SSE_RESULT_FB4(_mm_cvttps_epi32(a));
}

/**
 * @brief Converts a SIMD integer value to a float value, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The SIMD integer value.
 * @return The float value as would be done with an int to float cast in C.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4fb_toFloat(dsSIMD4fb a)
{
	return _mm_cvtepi32_ps(DS_SSE_PARAM_FB4(a));
}

/**
 * @brief Gets an integer element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#define dsSIMD4fb_get(a, i) _mm_cvtsi128_si32(_mm_castps_si128( \
	_mm_shuffle_ps(DS_SSE_CAST_FB4_TO_F4(a), DS_SSE_CAST_FB4_TO_F4(a), _MM_SHUFFLE(0, 0, 0, (i)))))

/**
 * @brief Negates an SIMD integer value.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_neg(dsSIMD4fb a)
{
	return DS_SSE_RESULT_FB4(_mm_sub_epi32(_mm_setzero_si128(), DS_SSE_PARAM_FB4(a)));
}

/**
 * @brief Adds two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_add(dsSIMD4fb a, dsSIMD4fb b)
{
	return DS_SSE_RESULT_FB4(_mm_add_epi32(DS_SSE_PARAM_FB4(a), DS_SSE_PARAM_FB4(b)));
}

/**
 * @brief Subtracts two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_sub(dsSIMD4fb a, dsSIMD4fb b)
{
	return DS_SSE_RESULT_FB4(_mm_sub_epi32(DS_SSE_PARAM_FB4(a), DS_SSE_PARAM_FB4(b)));
}

/**
 * @brief Left shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by.
 * @return The result of a << b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_shiftLeft(dsSIMD4fb a, unsigned int b)
{
	return DS_SSE_RESULT_FB4(_mm_sll_epi32(DS_SSE_PARAM_FB4(a), _mm_set_epi32(0, 0, 0, b)));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by as a constant integer.
 * @return The result of a << b.
 */
#define dsSIMD4fb_shiftLeftConst(a, b) DS_SSE_RESULT_FB4(_mm_slli_epi32(DS_SSE_PARAM_FB4((a)), (b)))

/**
 * @brief Right shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by.
 * @return The result of a >> b.
 */
DS_ALWAYS_INLINE dsSIMD4fb dsSIMD4fb_shiftRight(dsSIMD4fb a, unsigned int b)
{
	return DS_SSE_RESULT_FB4(_mm_srl_epi32(DS_SSE_PARAM_FB4(a), _mm_set_epi32(0, 0, 0, b)));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by as a constant integer.
 * @return The result of a >> b.
 */
#define dsSIMD4fb_shiftRightConst(a, b) \
	DS_SSE_RESULT_FB4(_mm_srli_epi32(DS_SSE_PARAM_FB4((a)), (b)))

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE2);
/// @endcond

/**
 * @brief Loads double values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param dp A pointer to the double values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_load(const void* dp)
{
	return _mm_load_pd((const double*)dp);
}

/**
 * @brief Loads double values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param dp A pointer to the double values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_loadUnaligned(const void* dp)
{
	return _mm_loadu_pd((const double*)(dp));
}

/**
 * @brief Sets a double value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param d The value to set.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_set1(double d)
{
	return _mm_set1_pd(d);
}

/**
 * @brief Sets a SIMD value with two doubles.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param x The first value.
 * @param y The second value.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_set2(double x, double y)
{
	return _mm_set_pd(y, x);
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#define dsSIMD2d_set1FromVec(a, i) _mm_shuffle_pd((a), (a), _MM_SHUFFLE2((i), (i)))

/**
 * @brief Stores a SIMD register into four double values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] fp A pointer to the double values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD2d_store(void* fp, dsSIMD2d a)
{
	_mm_store_pd((double*)fp, a);
}

/**
 * @brief Stores a SIMD register into four double values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] dp A pointer to the double values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD2d_storeUnaligned(void* dp, dsSIMD2d a)
{
	_mm_storeu_pd((double*)dp, a);
}

/**
 * @brief Gets a double element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#define dsSIMD2d_get(a, i) _mm_cvtsd_f64(_mm_shuffle_pd((a), (a), _MM_SHUFFLE2(0, (i))))

/**
 * @brief Negates a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_neg(dsSIMD2d a)
{
	return _mm_xor_pd(_mm_set1_pd(-0.0), a);
}

/**
 * @brief Negates specific components of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to negate.
 * @param negX Constant 1 to negate the X component or 0 to leave it unchanged.
 * @param negY Constant 1 to negate the Y component or 0 to leave it unchanged.
 * @return The result of the negation.
 */
#define dsSIMD2d_negComponents(a, negX, negY) \
	_mm_xor_pd(_mm_set_pd((negY) ? -0.0 : 0.0, (negX) ? -0.0 : 0.0), (a))

/**
 * @brief Adds two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_add(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_add_pd(a, b);
}

/**
 * @brief Subtracts two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_sub(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_sub_pd(a, b);
}

/**
 * @brief Multiplies two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @return The result of a*b.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_mul(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_mul_pd(a, b);
}

/**
 * @brief Divides two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to divide.
 * @param b The second value to divide.
 * @return The result of a/b.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_div(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_div_pd(a, b);
}

/**
 * @brief Takes the square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to take the square root.
 * @return The result of sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_sqrt(dsSIMD2d a)
{
	return _mm_sqrt_pd(a);
}

/**
 * @brief Takes the reciprocal of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/a.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_rcp(dsSIMD2d a)
{
	return _mm_div_pd(_mm_set1_pd(1.0), a);
}

/**
 * @brief Takes the reciprocal square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_rsqrt(dsSIMD2d a)
{
	return _mm_div_pd(_mm_set1_pd(1.0), _mm_sqrt_pd(a));
}

/**
 * @brief Takes the absolute value of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to take the absolute value.
 * @return The result of abs(a).
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_abs(dsSIMD2d a)
{
	return _mm_andnot_pd(_mm_set1_pd(-0.0), a);
}

/**
 * @brief Transposes the values in 2 SIMD vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first SIMD values.
 * @param b The second SIMD values.
 */
#define dsSIMD2d_transpose(a, b) \
	do \
	{ \
		dsSIMD2d _newA = _mm_unpacklo_pd((a), (b)); \
		dsSIMD2d _newB = _mm_unpackhi_pd((a), (b)); \
		(a) = _newA; \
		(b) = _newB; \
	} while (0)

/**
 * @brief Gets the minimum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to take the min of.
 * @param b The second value to take the min of.
 * @return The result of min(a, b).
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_min(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_min_pd(a, b);
}

/**
 * @brief Gets the maximum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to take the max of.
 * @param b The second value to take the max of.
 * @return The result of max(a, b).
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_max(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_max_pd(a, b);
}

/**
 * @brief Selects between two vectors based on a boolean mask.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The boolean mask to select with.
 * @param b The first SIMD values to select from.
 * @param c The second SIMD values to select from.
 * @return The result of a ? b : c.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_select(dsSIMD2db a, dsSIMD2d b, dsSIMD2d c)
{
	return _mm_or_pd(_mm_and_pd(_mm_castsi128_pd(a), b), _mm_andnot_pd(_mm_castsi128_pd(a), c));
}

/**
 * @brief Checks if two SIMD values are equal.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a == b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmpeq(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmpeq_pd(a, b));
}

/**
 * @brief Checks if two SIMD values are not equal.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a != b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmpne(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmpneq_pd(a, b));
}

/**
 * @brief Checks if one SIMD values is less than another.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a < b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmplt(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmplt_pd(a, b));
}

/**
 * @brief Checks if one SIMD values is less than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a <= b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmple(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmple_pd(a, b));
}

/**
 * @brief Checks if one SIMD values is greater than another.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a > b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmpgt(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmpgt_pd(a, b));
}

/**
 * @brief Checks if one SIMD values is greater than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a >= b as a dsSIMD2db.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2d_cmpge(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_castpd_si128(_mm_cmpge_pd(a, b));
}

/**
 * @brief Creates a SIMD value for true.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @return A SIMD value with true on all elements.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_true(void)
{
	return _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFULL);
}

/**
 * @brief Creates a SIMD value for false.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @return A SIMD value with false on all elements.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_false(void)
{
	return _mm_setzero_si128();
}

/**
 * @brief Loads 64-bit int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param ip A pointer to the 64-bit int values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_load(const void* ip)
{
	return _mm_load_si128((const dsSIMD2db*)ip);
}

/**
 * @brief Loads 64-bit int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param ip A pointer to the 64-bit int values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_loadUnaligned(const void* ip)
{
	return _mm_loadu_si128((const dsSIMD2db*)(ip));
}

/**
 * @brief Sets a 64-bit int value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param i The value to set.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_set1(uint64_t i)
{
	return _mm_set1_epi64x(i);
}

/**
 * @brief Sets a SIMD value with two 64-bit int values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param x The first value.
 * @param y The second value.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_set2(uint64_t x, uint64_t y)
{
	return _mm_set_epi64x(y, x);
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#define dsSIMD2db_set1FromVec(a, i) _mm_castpd_si128(\
	_mm_shuffle_pd(_mm_castsi128_pd((a)), _mm_castsi128_pd((a)), _MM_SHUFFLE2((i), (i))))

/**
 * @brief Converts a SIMD double value directly to a bitfield value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The SIMD floating-point value.
 * @return The direct bitfield conversion from double.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_fromDoubleBitfield(dsSIMD2d a)
{
	return _mm_castpd_si128(a);
}

/**
 * @brief Converts a SIMD double value to a 64-bit integer, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @remark Some implementations may only support integer values up to 32 bits.
 * @param a The SIMD floating-point value.
 * @return The integer representation as would be done with a 64-bit int to float cast in C.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_fromDouble(dsSIMD2d a)
{
	__m128i result32 = _mm_cvttpd_epi32(a);
#if defined(__SSE_4_1__) || defined(__AVX__)
	return _mm_cvtepi32_epi64(result32);
#else
	__m128i signedResult = _mm_shuffle_epi32(result32, _MM_SHUFFLE(3, 1, 2, 0));
	__m128i signedExtend = _mm_srai_epi32(_mm_shuffle_epi32(result32, _MM_SHUFFLE(1, 3, 0, 2)), 32);
	return _mm_or_si128(signedResult, signedExtend);
#endif
}

/**
 * @brief Stores a SIMD bitfield register into four int values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] ip A pointer to the int values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD2db_store(void* ip, dsSIMD2db a)
{
	_mm_store_si128((dsSIMD2db*)ip, a);
}

/**
 * @brief Stores a SIMD register into four int values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param[out] ip A pointer to the double values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD2db_storeUnaligned(void* ip, dsSIMD2db a)
{
	_mm_storeu_si128((dsSIMD2db*)ip, a);
}

/**
 * @brief Gets an integer element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#if DS_X86_64
#define dsSIMD2db_get(a, i) _mm_cvtsi128_si64(_mm_castpd_si128(\
	_mm_shuffle_pd(_mm_castsi128_pd((a)), _mm_castsi128_pd((a)), _MM_SHUFFLE2(0, (i)))))
#else
#define dsSIMD2db_get(a, i) dsSIMDDoubleToInt64(_mm_cvtsd_f64(\
	_mm_shuffle_pd(_mm_castsi128_pd((a)), _mm_castsi128_pd((a)), _MM_SHUFFLE2(0, (i)))))
#endif

/**
 * @brief Converts a SIMD bitfield value directly to a double value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The SIMD bitfield value.
 * @return The direct bitfield conversion to double.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2db_toDoubleBitfield(dsSIMD2db a)
{
	return _mm_castsi128_pd(a);
}

/**
 * @brief Converts a SIMD integer value to a double value, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @remark Some implementations may only support integer values up to 32 bits.
 * @param a The SIMD integer value.
 * @return The double value as would be done with a 64-bit int to double cast in C.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2db_toDouble(dsSIMD2db a)
{
	return _mm_cvtepi32_pd(_mm_shuffle_epi32(a, _MM_SHUFFLE(3, 1, 2, 0)));
}

/**
 * @brief Performs a logical not on a SIMD bitfield value.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The value to not.
 * @return The result of !a.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_not(dsSIMD2db a)
{
	return _mm_xor_si128(a, dsSIMD2db_true());
}

/**
 * @brief Performs a logical and between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to and.
 * @param b The second value to and.
 * @return The result of a & b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_and(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_and_si128(a, b);
}

/**
 * @brief Performs a logical and not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to not then and.
 * @param b The second value to and.
 * @return The result of (!a) & b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_andnot(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_andnot_si128(a, b);
}

/**
 * @brief Performs a logical or between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to or.
 * @param b The second value to or.
 * @return The result of a | b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_or(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_or_si128(a, b);
}

/**
 * @brief Performs a logical or not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to or.
 * @param b The second value to not then or.
 * @return The result of a | (!b).
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_ornot(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_or_si128(a, dsSIMD2db_not(b));
}

/**
 * @brief Performs a logical xor between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double2 is available.
 * @param a The first value to xor.
 * @param b The first value to xor.
 * @return The result of a ^ b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_xor(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_xor_si128(a, b);
}

/**
 * @brief Negates an SIMD integer value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_neg(dsSIMD2db a)
{
	return _mm_sub_epi64(_mm_setzero_si128(), a);
}

/**
 * @brief Adds two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_add(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_add_epi64(a, b);
}

/**
 * @brief Subtracts two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_sub(dsSIMD2db a, dsSIMD2db b)
{
	return _mm_sub_epi64(a, b);
}

/**
 * @brief Left shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by.
 * @return The result of a << b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_shiftLeft(dsSIMD2db a, unsigned int b)
{
	return _mm_sll_epi64(a, _mm_set_epi32(0, 0, 0, b));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by as a constant integer.
 * @return The result of a << b.
 */
#define dsSIMD2db_shiftLeftConst(a, b) _mm_slli_epi64((a), (b))

/**
 * @brief Right shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by.
 * @return The result of a >> b.
 */
DS_ALWAYS_INLINE dsSIMD2db dsSIMD2db_shiftRight(dsSIMD2db a, unsigned int b)
{
	return _mm_srl_epi64(a, _mm_set_epi32(0, 0, 0, b));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by as a constant integer.
 * @return The result of a >> b.
 */
#define dsSIMD2db_shiftRightConst(a, b) _mm_srli_epi64((a), (b))

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE4);
/// @endcond

/**
 * @brief Loads double values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param dp A pointer to the double values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_load(const void* dp)
{
	return _mm256_load_pd((const double*)dp);
}

/**
 * @brief Loads double values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param dp A pointer to the double values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_loadUnaligned(const void* dp)
{
	return _mm256_loadu_pd((const double*)(dp));
}

/**
 * @brief Sets a double value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param d The value to set.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_set1(double d)
{
	return _mm256_set1_pd(d);
}

/**
 * @brief Sets a SIMD value with four doubles.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param x The first value.
 * @param y The second value.
 * @param z The third value.
 * @param w The fourth value.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_set4(double x, double y, double z, double w)
{
	return _mm256_set_pd(w, z, y, x);
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#define dsSIMD4d_set1FromVec(a, i) _mm256_permute4x64_pd((a), _MM_SHUFFLE((i), (i), (i), (i)))

/**
 * @brief Stores a SIMD register into four double values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] dp A pointer to the double values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4d_store(void* dp, dsSIMD4d a)
{
	_mm256_store_pd((double*)dp, a);
}

/**
 * @brief Stores a SIMD register into four double values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] dp A pointer to the double values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4d_storeUnaligned(void* dp, dsSIMD4d a)
{
	_mm256_storeu_pd((double*)dp, a);
}

/**
 * @brief Gets a double element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#define dsSIMD4d_get(a, i) _mm256_cvtsd_f64(_mm256_permute4x64_pd((a), _MM_SHUFFLE(0, 0, 0, (i))))

/**
 * @brief Negates a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_neg(dsSIMD4d a)
{
	return _mm256_xor_pd(_mm256_set1_pd(-0.0), a);
}

/**
 * @brief Negates specific components of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to negate.
 * @param negX Constant 1 to negate the X component or 0 to leave it unchanged.
 * @param negY Constant 1 to negate the Y component or 0 to leave it unchanged.
 * @param negZ Constant 1 to negate the Z component or 0 to leave it unchanged.
 * @param negW Constant 1 to negate the W component or 0 to leave it unchanged.
 * @return The result of the negation.
 */
#define dsSIMD4d_negComponents(a, negX, negY, negZ, negW) \
	_mm256_xor_pd(_mm256_set_pd((negW) ? -0.0 : 0.0, (negZ) ? -0.0 : 0.0, (negY) ? -0.0 : 0.0, \
		(negX) ? -0.0 : 0.0), (a))

/**
 * @brief Adds two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_add(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_add_pd(a, b);
}

/**
 * @brief Subtracts two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_sub(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_sub_pd(a, b);
}

/**
 * @brief Multiplies two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @return The result of a*b.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_mul(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_mul_pd(a, b);
}

/**
 * @brief Divides two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to divide.
 * @param b The second value to divide.
 * @return The result of a/b.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_div(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_div_pd(a, b);
}

/**
 * @brief Takes the square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to take the square root.
 * @return The result of sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_sqrt(dsSIMD4d a)
{
	return _mm256_sqrt_pd(a);
}

/**
 * @brief Takes the reciprocal of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/a.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_rcp(dsSIMD4d a)
{
	return _mm256_div_pd(_mm256_set1_pd(1.0), a);
}

/**
 * @brief Takes the reciprocal square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to take the reciprocal.
 * @return The result of 1/sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_rsqrt(dsSIMD4d a)
{
	return _mm256_div_pd(_mm256_set1_pd(1.0), _mm256_sqrt_pd(a));
}

/**
 * @brief Takes the absolute value of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to take the absolute value.
 * @return The result of abs(a).
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_abs(dsSIMD4d a)
{
	return _mm256_andnot_pd(_mm256_set1_pd(-0.0), a);
}

/**
 * @brief Transposes the values in 4 SIMD vectors.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first SIMD values.
 * @param b The second SIMD values.
 * @param c The third SIMD values.
 * @param d The fourth SIMD values.
 */
#define dsSIMD4d_transpose(a, b, c, d) \
	do \
	{ \
		__m256d _tmpD, _tmpC, _tmpB, _tmpA; \
		_tmpA = _mm256_unpacklo_pd((a), (b)); \
		_tmpC = _mm256_unpacklo_pd((c), (d)); \
		_tmpB = _mm256_unpackhi_pd((a), (b)); \
		_tmpD = _mm256_unpackhi_pd((c), (d)); \
		(a) = _mm256_permute2f128_pd(_tmpA, _tmpC, _MM_SHUFFLE(0, 2, 0, 0)); \
		(b) = _mm256_permute2f128_pd(_tmpB, _tmpD, _MM_SHUFFLE(0, 2, 0, 0)); \
		(c) = _mm256_permute2f128_pd(_tmpC, _tmpA, _MM_SHUFFLE(0, 1, 0, 3)); \
		(d) = _mm256_permute2f128_pd(_tmpD, _tmpB, _MM_SHUFFLE(0, 1, 0, 3)); \
	} while (0)

/**
 * @brief Gets the minimum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to take the min of.
 * @param b The second value to take the min of.
 * @return The result of min(a, b).
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_min(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_min_pd(a, b);
}

/**
 * @brief Gets the maximum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to take the max of.
 * @param b The second value to take the max of.
 * @return The result of max(a, b).
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_max(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_max_pd(a, b);
}

/**
 * @brief Selects between two vectors based on a boolean mask.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The boolean mask to select with.
 * @param b The first SIMD values to select from.
 * @param c The second SIMD values to select from.
 * @return The result of a ? b : c.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_select(dsSIMD4db a, dsSIMD4d b, dsSIMD4d c)
{
	return _mm256_or_pd(_mm256_and_pd(_mm256_castsi256_pd(a), b),
		_mm256_andnot_pd(_mm256_castsi256_pd(a), c));
}

/**
 * @brief Checks if two SIMD values are equal.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a == b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmpeq(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_EQ_OQ));
}

/**
 * @brief Checks if two SIMD values are not equal.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a != b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmpne(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_NEQ_OQ));
}

/**
 * @brief Checks if one SIMD values is less than another.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a < b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmplt(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_LT_OQ));
}

/**
 * @brief Checks if one SIMD values is less than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a <= b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmple(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_LE_OQ));
}

/**
 * @brief Checks if one SIMD values is greater than another.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a > b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmpgt(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_GT_OQ));
}

/**
 * @brief Checks if one SIMD values is greater than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a >= b as a dsSIMD4db.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4d_cmpge(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_GE_OQ));
}

/**
 * @brief Creates a SIMD value for true.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @return A SIMD value with true on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_true(void)
{
	return _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFFULL);
}

/**
 * @brief Creates a SIMD value for false.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @return A SIMD value with false on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_false(void)
{
	return _mm256_setzero_si256();
}

/**
 * @brief Loads 64-bit int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param ip A pointer to the 64-bit int values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_load(const void* ip)
{
	return _mm256_load_si256((const dsSIMD4db*)ip);
}

/**
 * @brief Loads 64-bit int values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param ip A pointer to the 64-bit int values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_loadUnaligned(const void* ip)
{
	return _mm256_loadu_si256((const dsSIMD4db*)(ip));
}

/**
 * @brief Sets a 64-bit int value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param i The value to set.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_set1(uint64_t i)
{
	return _mm256_set1_epi64x(i);
}

/**
 * @brief Sets a SIMD value with four 64-bit int values.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available.
 * @param x The first value.
 * @param y The second value.
 * @param z The third value.
 * @param w The fourth value.
 * @return The SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_set4(uint64_t x, uint64_t y, uint64_t z, uint64_t w)
{
	return _mm256_set_epi64x(w, z, y, x);
}

/**
 * @brief Sets an element from a vector to all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element to get.
 * @return The SIMD value.
 */
#define dsSIMD4db_set1FromVec(a, i) _mm256_permute4x64_epi64((a), _MM_SHUFFLE((i), (i), (i), (i)))

/**
 * @brief Converts a SIMD double value directly to a bitfield value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The SIMD floating-point value.
 * @return The direct bitfield conversion from double.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_fromDoubleBitfield(dsSIMD4d a)
{
	return _mm256_castpd_si256(a);
}

/**
 * @brief Converts a SIMD double value to a 64-bit integer, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Double4 and dsSIMDFeatures_Int are available.
 * @remark Some implementations may only support integer values up to 32 bits.
 * @param a The SIMD floating-point value.
 * @return The integer representation as would be done with a 64-bit int to float cast in C.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_fromDouble(dsSIMD4d a)
{
	return _mm256_cvtepi32_epi64(_mm256_cvttpd_epi32(a));
}

/**
 * @brief Stores a SIMD bitfield register into four int values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] ip A pointer to the int values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4db_store(void* ip, dsSIMD4db a)
{
	_mm256_store_si256((dsSIMD4db*)ip, a);
}

/**
 * @brief Stores a SIMD register into four int values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param[out] ip A pointer to the double values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4db_storeUnaligned(void* ip, dsSIMD4db a)
{
	_mm256_storeu_si256((dsSIMD4db*)ip, a);
}

/**
 * @brief Gets a 64-bit int element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#if DS_X86_64
#define dsSIMD4db_get(a, i) _mm256_extract_epi64((a), (i))
#else
#define dsSIMD4db_get(a, i) dsSIMDDoubleToInt64(\
	_mm256_cvtsd_f64(_mm256_castsi256_pd(_mm256_permute4x64_epi64((a), _MM_SHUFFLE(0, 0, 0, (i))))))
#endif

/**
 * @brief Converts a SIMD bitfield value directly to a double value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The SIMD bitfield value.
 * @return The direct bitfield conversion to double.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4db_toDoubleBitfield(dsSIMD4db a)
{
	return _mm256_castsi256_pd(a);
}

/**
 * @brief Converts a SIMD integer value to a double value, similar to a cast in C.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @remark Some implementations may only support integer values up to 32 bits.
 * @param a The SIMD integer value.
 * @return The double value as would be done with a 64-bit int to double cast in C.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4db_toDouble(dsSIMD4db a)
{
	return _mm256_cvtepi32_pd(_mm256_castsi256_si128(_mm256_permutevar8x32_epi32(
		a, _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0))));
}

/**
 * @brief Performs a logical not on a SIMD bitfield value.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The value to not.
 * @return The result of !a.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_not(dsSIMD4db a)
{
	return _mm256_xor_si256(a, dsSIMD4db_true());
}

/**
 * @brief Performs a logical and between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to and.
 * @param b The second value to and.
 * @return The result of a & b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_and(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_and_si256(a, b);
}

/**
 * @brief Performs a logical and not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to not then and.
 * @param b The second value to and.
 * @return The result of (!a) & b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_andnot(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_andnot_si256(a, b);
}

/**
 * @brief Performs a logical or between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to or.
 * @param b The second value to or.
 * @return The result of a | b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_or(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_or_si256(a, b);
}

/**
 * @brief Performs a logical or not between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to or.
 * @param b The second value to not then or.
 * @return The result of a | (!b).
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_ornot(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_or_si256(a, dsSIMD4db_not(b));
}

/**
 * @brief Performs a logical xor between two SIMD bitfield values.
 * @remark This can be used when dsSIMDFeatures_Double4 is available.
 * @param a The first value to xor.
 * @param b The first value to xor.
 * @return The result of a ^ b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_xor(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_xor_si256(a, b);
}

/**
 * @brief Negates an SIMD integer value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_neg(dsSIMD4db a)
{
	return _mm256_sub_epi64(_mm256_setzero_si256(), a);
}

/**
 * @brief Adds two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_add(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_add_epi64(a, b);
}

/**
 * @brief Subtracts two SIMD integer values.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The first value to subtract.
 * @param b The second value to subtract.
 * @return The result of a - b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_sub(dsSIMD4db a, dsSIMD4db b)
{
	return _mm256_sub_epi64(a, b);
}

/**
 * @brief Left shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by.
 * @return The result of a << b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_shiftLeft(dsSIMD4db a, unsigned int b)
{
	return _mm256_sll_epi64(a, _mm_set_epi32(0, 0, 0, b));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift left.
 * @param b The number of bits to shift left by as a constant integer.
 * @return The result of a << b.
 */
#define dsSIMD4db_shiftLeftConst(a, b) _mm256_slli_epi64((a), (b))

/**
 * @brief Right shifts an integer SIMD value.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by.
 * @return The result of a >> b.
 */
DS_ALWAYS_INLINE dsSIMD4db dsSIMD4db_shiftRight(dsSIMD4db a, unsigned int b)
{
	return _mm256_srl_epi64(a, _mm_set_epi32(0, 0, 0, b));
}

/**
 * @brief Left shifts an integer SIMD value by a constant number of bits.
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @param a The value to shift right.
 * @param b The number of bits to shift right by as a constant integer.
 * @return The result of a >> b.
 */
#define dsSIMD4db_shiftRightConst(a, b) _mm256_srli_epi64((a), (b))

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_HADD);
/// @endcond

/**
 * @brief Performs a horizontal add between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_HAdd is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of (a.x + a.y, a.z + a.w, b.x + b.y, b.z + b.w)
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_hadd(dsSIMD4f a, dsSIMD4f b)
{
	return _mm_hadd_ps(a, b);
}

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_HADD);
/// @endcond

/**
 * @brief Performs a horizontal add between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_HAdd and dsSIMDFeatures_Double2 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of (a.x + a.y, b.x + b.y)
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_hadd(dsSIMD2d a, dsSIMD2d b)
{
	return _mm_hadd_pd(a, b);
}

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_HADD);
/// @endcond

/**
 * @brief Performs a horizontal add between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_HAdd and dsSIMDFeatures_Double4 is available.
 * @remark The order of outputs is different from dsSIMD4f_hadd() since the hardware treats each
 *     dsSIMD4d as two dsSIMD2d elements that are processed in parallel.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of (a.x + a.y, b.x + b.y, a.z + a.w, b.z + b.w)
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_hadd(dsSIMD4d a, dsSIMD4d b)
{
	return _mm256_hadd_pd(a, b);
}

/// @cond
DS_SIMD_END();

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA);
/// @endcond

/**
 * @brief Performs a fused multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of a*b + c.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_fmadd(dsSIMD4f a, dsSIMD4f b, dsSIMD4f c)
{
	return _mm_fmadd_ps(a, b, c);
}

/**
 * @brief Performs a fused multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of a*b - c.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_fmsub(dsSIMD4f a, dsSIMD4f b, dsSIMD4f c)
{
	return _mm_fmsub_ps(a, b, c);
}

/**
 * @brief Performs a fused negate multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of -(a*b) + c.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_fnmadd(dsSIMD4f a, dsSIMD4f b, dsSIMD4f c)
{
	return _mm_fnmadd_ps(a, b, c);
}

/**
 * @brief Performs a fused negate multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of -(a*b) - c.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_fnmsub(dsSIMD4f a, dsSIMD4f b, dsSIMD4f c)
{
	return _mm_fnmsub_ps(a, b, c);
}

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA);
/// @endcond

/**
 * @brief Performs a fused multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double2 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of a*b + c.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_fmadd(dsSIMD2d a, dsSIMD2d b, dsSIMD2d c)
{
	return _mm_fmadd_pd(a, b, c);
}

/**
 * @brief Performs a fused multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double2 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of a*b - c.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_fmsub(dsSIMD2d a, dsSIMD2d b, dsSIMD2d c)
{
	return _mm_fmsub_pd(a, b, c);
}

/**
 * @brief Performs a fused negate multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double2 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of -(a*b) + c.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_fnmadd(dsSIMD2d a, dsSIMD2d b, dsSIMD2d c)
{
	return _mm_fnmadd_pd(a, b, c);
}

/**
 * @brief Performs a fused negate multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double2 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of -(a*b) - c.
 */
DS_ALWAYS_INLINE dsSIMD2d dsSIMD2d_fnmsub(dsSIMD2d a, dsSIMD2d b, dsSIMD2d c)
{
	return _mm_fnmsub_pd(a, b, c);
}

/// @cond
DS_SIMD_END();
DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA);
/// @endcond

/**
 * @brief Performs a fused multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of a*b + c.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_fmadd(dsSIMD4d a, dsSIMD4d b, dsSIMD4d c)
{
	return _mm256_fmadd_pd(a, b, c);
}

/**
 * @brief Performs a fused multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of a*b - c.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_fmsub(dsSIMD4d a, dsSIMD4d b, dsSIMD4d c)
{
	return _mm256_fmsub_pd(a, b, c);
}

/**
 * @brief Performs a fused negate multiply add with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to add.
 * @return The result of -(a*b) + c.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_fnmadd(dsSIMD4d a, dsSIMD4d b, dsSIMD4d c)
{
	return _mm256_fnmadd_pd(a, b, c);
}

/**
 * @brief Performs a fused negate multiply subtract with three SIMD values.
 * @remark This can be used when dsSIMDFeatures_FMA and dsSIMDFeatures_Double4 is available.
 * @param a The first value to multiply.
 * @param b The second value to multiply.
 * @param c The third value to subtract.
 * @return The result of -(a*b) - c.
 */
DS_ALWAYS_INLINE dsSIMD4d dsSIMD4d_fnmsub(dsSIMD4d a, dsSIMD4d b, dsSIMD4d c)
{
	return _mm256_fnmsub_pd(a, b, c);
}

/// @cond
DS_SIMD_END();
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_HALF_FLOAT);
/// @endcond

/**
 * @brief Loads a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float value to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load1(const void* hfp)
{
	return _mm_cvtsi32_si128(*(const unsigned short*)hfp);
}

/**
 * @brief Loads two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load2(const void* hfp)
{
	return _mm_cvtsi32_si128(*(const unsigned int*)hfp);
}

/**
 * @brief Loads four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load4(const void* hfp)
{
	return _mm_loadu_si64(hfp);
}

/**
 * @brief Stores a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float value to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store1(void* hfp, dsSIMD4hf a)
{
	*(short*)hfp = (short)_mm_cvtsi128_si32(a);
}

/**
 * @brief Stores two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store2(void* hfp, dsSIMD4hf a)
{
	*(int*)hfp = _mm_cvtsi128_si32(a);
}

/**
 * @brief Stores four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store4(void* hfp, dsSIMD4hf a)
{
	_mm_storeu_si64(hfp, a);
}

/**
 * @brief Converts SIMD floats to half floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_fromFloat(dsSIMD4f a)
{
	return _mm_cvtps_ph((a), 0);
}

/**
 * @brief Converts SIMD half floats to floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4hf_toFloat(dsSIMD4hf a)
{
	return _mm_cvtph_ps(a);
}

/// @cond
DS_SIMD_END();
/// @endcond

#ifdef __cplusplus
}
#endif
