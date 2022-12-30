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

#include <arm_neon.h>
#include <stdint.h>

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
#define DS_SIMD_START_FLOAT4()
#define DS_SIMD_START_HADD()
#define DS_SIMD_START_FMA()
#define DS_SIMD_START_HALF_FLOAT()
#define DS_SIMD_END()

#define DS_SIMD_ALWAYS_FLOAT4 1
#define DS_SIMD_ALWAYS_HADD 1
#define DS_SIMD_ALWAYS_FMA 1
#define DS_SIMD_ALWAYS_HALF_FLOAT 1

#if DS_MSC
#define DS_ASSUME_SIMD_ALIGNED(x) (((uintptr_t)(x) & 0xF) == 0 ? (x) : __assume(0))
#else
#define DS_ASSUME_SIMD_ALIGNED(x) __builtin_assume_aligned((void*)(x), 16)
#endif
/// @endcond

/**
 * @brief Type for a SIMD vector of 4 floats.
 */
typedef float32x4_t dsSIMD4f;

/**
 * @brief Type for a SIMD vector of 4 bool results.
 *
 * Each boolean value will be stored in a 32-bit value.
 */
typedef uint32x4_t dsSIMD4b;

/**
 * @brief Type for a SIMD vector of 4 half floats.
 */
typedef float16x4_t dsSIMD4hf;

/**
 * @brief Loads float values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param fp A pointer to the float values to load. This should be aligned to 16 bytes.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_load(const void* fp)
{
	return vld1q_f32((const float*)DS_ASSUME_SIMD_ALIGNED(fp));
}

/**
 * @brief Loads float values into a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param fp A pointer to the float values to load. This may be unaligned.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_loadUnaligned(const void* fp)
{
	return vld1q_f32((const float*)fp);
}

/**
 * @brief Sets a float value into all elements of a SIMD register.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param f The value to set.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_set1(float f)
{
	return vdupq_n_f32(f);
}

/**
 * @brief Sets a SIMD value with four floats.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param x The first value.
 * @param y The second value.
 * @param z The third value.
 * @param w The fourth value.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_set4(float x, float y, float z, float w)
{
	dsSIMD4f temp = {x, y, z, w};
	return temp;
}

/**
 * @brief Stores a SIMD register into four float values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] fp A pointer to the float values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4f_store(void* fp, dsSIMD4f a)
{
	vst1q_f32((float*)DS_ASSUME_SIMD_ALIGNED(fp), a);
}

/**
 * @brief Stores a SIMD register into four float values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] fp A pointer to the float values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4f_storeUnaligned(void* fp, dsSIMD4f a)
{
	vst1q_f32((float*)fp, a);
}

/**
 * @brief Gets a float element from a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to get the element from.
 * @param i The index of the element.
 * @return The element value.
 */
#define dsSIMD4f_get(a, i) vgetq_lane_f32((a), (i))

/**
 * @brief Negates a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to negate.
 * @return The result of -a.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_neg(dsSIMD4f a)
{
	return vnegq_f32(a);
}

/**
 * @brief Adds two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of a + b.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_add(dsSIMD4f a, dsSIMD4f b)
{
	return vaddq_f32(a, b);
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
	return vsubq_f32(a, b);
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
	return vmulq_f32(a, b);
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
#if DS_ARM_64
	return vdivq_f32(a, b);
#else
	return vmulq_f32(a, vrecpeq_f32(b));
#endif
}

/**
 * @brief Takes the approximate reciprical of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The approximate result of 1/a.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_rcp(dsSIMD4f a)
{
	return vrecpeq_f32(a);
}

/**
 * @brief Takes the square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The result of sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_sqrt(dsSIMD4f a)
{
#if DS_ARM_64
	return vsqrtq_f32(a);
#else
	return vrecpeq_f32(vrsqrteq_f32(a));
#endif
}

/**
 * @brief Takes the approximate reciprical square root of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the reciprical.
 * @return The approximate result of 1/sqrt(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_rsqrt(dsSIMD4f a)
{
	return vrsqrteq_f32(a);
}

/**
 * @brief Takes the absolute value of a SIMD value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to take the absolute value.
 * @return The result of abs(a).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_abs(dsSIMD4f a)
{
	return vabsq_f32(a);
}

/**
 * @brief Transposes the values in 4 SIMD vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first SIMD values.
 * @param b The second SIMD values.
 * @param c The third SIMD values.
 * @param d The fourth SIMD values.
 */
#define dsSIMD4f_transpose(a, b, c, d) \
do \
{ \
	float32x4x2_t _tmpAB = vtrnq_f32(a, b); \
	float32x4x2_t _tmpCD = vtrnq_f32(c, d); \
	\
	a = vcombine_f32(vget_low_f32(_tmpAB.val[0]), vget_low_f32(_tmpCD.val[0])); \
	b = vcombine_f32(vget_low_f32(_tmpAB.val[1]), vget_low_f32(_tmpCD.val[1])); \
	c = vcombine_f32(vget_high_f32(_tmpAB.val[0]), vget_high_f32(_tmpCD.val[0])); \
	d = vcombine_f32(vget_high_f32(_tmpAB.val[1]), vget_high_f32(_tmpCD.val[1])); \
} while (0)

/**
 * @brief Performs a horizontal add between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_HAdd is available.
 * @param a The first value to add.
 * @param b The second value to add.
 * @return The result of (a.x + a.y, a.z + a.w, b.x + b.y, b.z + b.w)
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_hadd(dsSIMD4f a, dsSIMD4f b)
{
#if DS_ARM_64
	return vpaddq_f32(a, b);
#else
	return vcombine_f32(vpadd_f32(vget_low_f32(a), vget_high_f32(a)),
		vpadd_f32(vget_low_f32(b), vget_high_f32(b)));
#endif
}

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
	return vfmaq_f32(c, a, b);
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
	return vfmaq_f32(vnegq_f32(c), a, b);
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
	return vfmsq_f32(c, a, b);
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
	return vnegq_f32(vfmaq_f32(c, a, b));
}

/**
 * @brief Gets the minimum elements between two SIMD values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to take the min of.
 * @param b The second value to take the min of.
 * @return The result of min(a, b).
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_min(dsSIMD4f a, dsSIMD4f b)
{
	return vminq_f32(a, b);
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
	return vmaxq_f32(a, b);
}

/**
 * @brief Selects between two vectors based on a boolean mask.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first SIMD values to select from.
 * @param b The second SIMD values to select from.
 * @param c The boolean mask to select with.
 * @return Values from a or b for whether c is true or false, respectively.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4f_select(dsSIMD4f a, dsSIMD4f b, dsSIMD4b c)
{
	return vbslq_f32(c, a, b);
}

/**
 * @brief Checks if two SIMD values are equal.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a == b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmpeq(dsSIMD4f a, dsSIMD4f b)
{
	return vceqq_f32(a, b);
}

/**
 * @brief Checks if two SIMD values are not equal.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a != b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmpne(dsSIMD4f a, dsSIMD4f b)
{
	return vmvnq_u32(vceqq_f32(a, b));
}

/**
 * @brief Checks if one SIMD values is less than another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a < b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmplt(dsSIMD4f a, dsSIMD4f b)
{
	return vcltq_f32(a, b);
}

/**
 * @brief Checks if one SIMD values is less than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a <= b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmple(dsSIMD4f a, dsSIMD4f b)
{
	return vcleq_f32(a, b);
}

/**
 * @brief Checks if one SIMD values is greater than another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a > b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmpgt(dsSIMD4f a, dsSIMD4f b)
{
	return vcgtq_f32(a, b);
}

/**
 * @brief Checks if one SIMD values is greater than or equal to another.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to compare.
 * @param b The second value to compare.
 * @return The result of a >= b as a dsSIMD4b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4f_cmpge(dsSIMD4f a, dsSIMD4f b)
{
	return vcgeq_f32(a, b);
}

/**
 * @brief Creates a SIMD value for true.
 * @return A SIMD value with true on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_true(void)
{
	return vdupq_n_u32(0xFFFFFFFF);
}

/**
 * @brief Creates a SIMD value for false.
 * @return A SIMD value with false on all elements.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_false(void)
{
	return vdupq_n_u32(0);
}

/**
 * @brief Stores a SIMD bool register into four int values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] ip A pointer to the int values to store to. This should be aligned to 16 bytes.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4b_store(void* ip, dsSIMD4b a)
{
	vst1q_u32((unsigned int*)DS_ASSUME_SIMD_ALIGNED(ip), a);
}

/**
 * @brief Stores a SIMD register into four int values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param[out] ip A pointer to the float values to store to. This may be unaligned.
 * @param a The value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4b_storeUnaligned(void* ip, dsSIMD4b a)
{
	vst1q_u32((unsigned int*)ip, a);
}

/**
 * @brief Performs a logical not on a SIMD bool value.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The value to not.
 * @return The result of !a.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_not(dsSIMD4b a)
{
	return vmvnq_u32(a);
}

/**
 * @brief Performs a logical and between two SIMD bool values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to and.
 * @param b The second value to and.
 * @return The result of a & b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_and(dsSIMD4b a, dsSIMD4b b)
{
	return vandq_u32(a, b);
}

/**
 * @brief Performs a logical and not between two SIMD bool values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to not then and.
 * @param b The second value to and.
 * @return The result of (!a) & b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_andnot(dsSIMD4b a, dsSIMD4b b)
{
	return vandq_u32(vmvnq_u32(a), b);
}

/**
 * @brief Performs a logical or between two SIMD bool values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to or.
 * @param b The second value to or.
 * @return The result of a | b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_or(dsSIMD4b a, dsSIMD4b b)
{
	return vorrq_u32(a, b);
}

/**
 * @brief Performs a logical or not between two SIMD bool values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to or.
 * @param b The second value to not then or.
 * @return The result of a | (!b).
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_ornot(dsSIMD4b a, dsSIMD4b b)
{
	return vornq_u32(a, b);
}

/**
 * @brief Performs a logical xor between two SIMD bool values.
 * @remark This can be used when dsSIMDFeatures_Float4 is available.
 * @param a The first value to xor.
 * @param b The first value to xor.
 * @return The result of a ^ b.
 */
DS_ALWAYS_INLINE dsSIMD4b dsSIMD4b_xor(dsSIMD4b a, dsSIMD4b b)
{
	return veorq_u32(a, b);
}

/**
 * @brief Loads a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float value to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load1(const void* hfp)
{
	const float16_t* loadPtr = (const float16_t*)hfp;
	dsSIMD4hf temp = {*loadPtr, 0, 0, 0};
	return temp;
}

/**
 * @brief Loads two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load2(const void* hfp)
{
	const float16_t* loadPtr = (const float16_t*)hfp;
	dsSIMD4hf temp = {loadPtr[0], loadPtr[1], 0, 0};
	return temp;
}

/**
 * @brief Loads four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param hfp A pointer to the half float values to load.
 * @return The loaded SIMD value.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_load4(const void* hfp)
{
	return vld1_f16((const float16_t*)hfp);
}

/**
 * @brief Stores a single half float value.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float value to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store1(void* hfp, dsSIMD4hf a)
{
	vst1_lane_f16((float16_t*)hfp, a, 0);
}

/**
 * @brief Stores two half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store2(void* hfp, dsSIMD4hf a)
{
	float16_t* storePtr = (float16_t*)hfp;
	vst1_lane_f16(storePtr, a, 0);
	vst1_lane_f16(storePtr + 1, a, 1);
}

/**
 * @brief Stores four half float values.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param[out] hfp A pointer to the half float values to store.
 * @param a The SIMD value to store.
 */
DS_ALWAYS_INLINE void dsSIMD4hf_store4(void* hfp, dsSIMD4hf a)
{
	vst1_f16((float16_t*)hfp, a);
}

/**
 * @brief Converts SIMD floats to half floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
DS_ALWAYS_INLINE dsSIMD4hf dsSIMD4hf_fromFloat(dsSIMD4f a)
{
	return vcvt_f16_f32(a);
}

/**
 * @brief Converts SIMD half floats to floats.
 * @remark This can be used when dsSIMDFeatures_HalfFloat is available.
 * @param a The value to convert.
 * @return The converted values.
 */
DS_ALWAYS_INLINE dsSIMD4f dsSIMD4hf_toFloat(dsSIMD4hf a)
{
	return vcvt_f32_f16(a);
}

#ifdef __cplusplus
}
#endif
