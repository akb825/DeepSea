/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/Math/SIMD/SIMD.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief File containing functions to perform dot products with SIMD operations.
 *
 * These will use the generally best operations available for the given platform, including special
 * instructions not available through the generic SIMD functions exposed across platforms.
 */

#if DS_HAS_SIMD

/// @cond
#if DS_X86 && (defined(__SSE4_1__) || defined(__AVX__) || DS_X86_ARCH_LEVEL >= 2)
#define DS_SIMD_HAS_SSE_DP 1
#else
#define DS_SIMD_HAS_SSE_DP 0
#endif
/// @endcond

DS_SIMD_START(DS_SIMD_FLOAT4)

/**
 * @brief Performs the dot product between two 4-component vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available, and will use the most efficient
 *     implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w).
 */
DS_ALWAYS_INLINE dsSIMD4f dsDot4SIMD4f(dsSIMD4f a, dsSIMD4f b)
{
#if DS_SIMD_HAS_SSE_DP
	return _mm_dp_ps(a, b, 0xFF);
#elif DS_SIMD_ALWAYS_HADD
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	ab = dsSIMD4f_hadd(ab, ab);
	return dsSIMD4f_hadd(ab, ab);
#elif DS_X86
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	// Assume additions are commutative. (should be the case if IEEE compliant)
	ab = dsSIMD4f_add(ab, _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(2, 3, 0, 1)));
	return dsSIMD4f_add(ab, _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(0, 0, 2, 2)));
#else
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	dsSIMD4f abxy = dsSIMD4f_add(dsSIMD4f_set1FromVec(ab, 0), dsSIMD4f_set1FromVec(ab, 1));
	dsSIMD4f abzw = dsSIMD4f_add(dsSIMD4f_set1FromVec(ab, 2), dsSIMD4f_set1FromVec(ab, 3));
	return dsSIMD4f_add(abxy, abzw);
#endif
}

/**
 * @brief Performs the dot product between two 3-component vectors.
 * @remark This can be used when dsSIMDFeatures_Float4 is available, and will use the most efficient
 *     implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + a.z*b.z.
 */
DS_ALWAYS_INLINE dsSIMD4f dsDot3SIMD4f(dsSIMD4f a, dsSIMD4f b)
{
#if DS_SIMD_HAS_SSE_DP
	return _mm_dp_ps(a, b, 0x7F);
#else
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	dsSIMD4f abxy = dsSIMD4f_add(dsSIMD4f_set1FromVec(ab, 0), dsSIMD4f_set1FromVec(ab, 1));
	return dsSIMD4f_add(abxy, dsSIMD4f_set1FromVec(ab, 2));
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

/**
 * @brief Performs the dot product between two 4-component vectors.
 * @remark This is intended when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available, where
 *     more assumptions can be made about available operations. No FMA operations are performed.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w).
 */
DS_ALWAYS_INLINE dsSIMD4f dsDot4FMA4f(dsSIMD4f a, dsSIMD4f b)
{
#if DS_X86
	return _mm_dp_ps(a, b, 0xFF);
#else
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	ab = dsSIMD4f_hadd(ab, ab);
	return dsSIMD4f_hadd(ab, ab);
#endif
}

/**
 * @brief Performs the dot product between two 3-component vectors.
 * @remark This is intended when dsSIMDFeatures_Float4 and dsSIMDFeatures_FMA are available, where
 *     more assumptions can be made about available operations. No FMA operations are performed.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + a.z*b.z.
 */
DS_ALWAYS_INLINE dsSIMD4f dsDot3FMA4f(dsSIMD4f a, dsSIMD4f b)
{
#if DS_X86
	return _mm_dp_ps(a, b, 0x7F);
#else
	dsSIMD4f ab = dsSIMD4f_mul(a, b);
	dsSIMD4f abxy = dsSIMD4f_add(dsSIMD4f_set1FromVec(ab, 0), dsSIMD4f_set1FromVec(ab, 1));
	return dsSIMD4f_add(abxy, dsSIMD4f_set1FromVec(ab, 2));
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2)

/**
 * @brief Performs the dot product between two 4-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available, and will use the most
 *     efficient implementation based on what is enabled at compile time.
 * @param a0 The first two components of the first vector.
 * @param a1 The second two components of the first vector.
 * @return b0 The first two components of the second vector.
 * @return b0 The second two components of the second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w).
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot4SIMD2d(dsSIMD2d a0, dsSIMD2d a1, dsSIMD2d b0, dsSIMD2d b1)
{
#if DS_SIMD_HAS_SSE_DP
	return dsSIMD2d_add(_mm_dp_pd(a0, b0, 0x33), _mm_dp_pd(a1, b1, 0x33));
#elif DS_SIMD_ALWAYS_HADD
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	dsSIMD2d dot = dsSIMD2d_hadd(ab0, ab1);
	return dsSIMD2d_hadd(dot, dot);
#elif DS_X86
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	// Assume additions are commutative. (should be the case if IEEE compliant)
	dsSIMD2d abxy = dsSIMD2d_add(ab0, _mm_shuffle_pd(ab0, ab0, 0x1));
	dsSIMD2d abzw = dsSIMD2d_add(ab1, _mm_shuffle_pd(ab1, ab1, 0x1));
	return dsSIMD2d_add(abxy, abzw);
#else
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	dsSIMD2d abxy = dsSIMD2d_add(dsSIMD2d_set1FromVec(ab0, 0), dsSIMD2d_set1FromVec(ab0, 1));
	dsSIMD2d abzw = dsSIMD2d_add(dsSIMD2d_set1FromVec(ab1, 0), dsSIMD2d_set1FromVec(ab1, 1));
	return dsSIMD2d_add(abxy, abzw);
#endif
}

/**
 * @brief Performs the dot product between two 3-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available, and will use the most
 *     efficient implementation based on what is enabled at compile time.
 * @param a0 The first two components of the first vector.
 * @param a1 The second two components of the first vector.
 * @return b0 The first two components of the second vector.
 * @return b0 The second two components of the second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + a.z*b.z.
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot3SIMD2d(dsSIMD2d a0, dsSIMD2d a1, dsSIMD2d b0, dsSIMD2d b1)
{
#if DS_SIMD_HAS_SSE_DP
	return dsSIMD2d_add(_mm_dp_pd(a0, b0, 0x33), dsSIMD2d_set1FromVec(dsSIMD2d_mul(a1, b1), 0));
#else
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	dsSIMD2d abxy = dsSIMD2d_add(dsSIMD2d_set1FromVec(ab0, 0), dsSIMD2d_set1FromVec(ab0, 1));
	return dsSIMD2d_add(abxy, dsSIMD2d_set1FromVec(ab1, 0));
#endif
}

/**
 * @brief Performs the dot product between two 2-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available, and will use the most
 *     efficient implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to a.x*b.x + a.y*b.y.
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot2SIMD2d(dsSIMD2d a, dsSIMD2d b)
{
#if DS_SIMD_HAS_SSE_DP
	return _mm_dp_pd(a, b, 0x33);
#elif DS_SIMD_ALWAYS_HADD
	dsSIMD2d ab = dsSIMD2d_mul(a, b);
	return dsSIMD2d_hadd(ab, ab);
#else
	dsSIMD2d ab = dsSIMD2d_mul(a, b);
	return dsSIMD2d_add(dsSIMD2d_set1FromVec(ab, 0), dsSIMD2d_set1FromVec(ab, 1));
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

/**
 * @brief Performs the dot product between two 4-component vectors.
 * @remark This is intended when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available, where
 *     more assumptions can be made about available operations. No FMA operations are performed.
 * @param a0 The first two components of the first vector.
 * @param a1 The second two components of the first vector.
 * @return b0 The first two components of the second vector.
 * @return b0 The second two components of the second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w).
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot4FMA2d(dsSIMD2d a0, dsSIMD2d a1, dsSIMD2d b0, dsSIMD2d b1)
{
#if DS_X86
	return dsSIMD2d_add(_mm_dp_pd(a0, b0, 0x33), _mm_dp_pd(a1, b1, 0x33));
#else
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	dsSIMD2d dot = dsSIMD2d_hadd(ab0, ab1);
	return dsSIMD2d_hadd(dot, dot);
#endif
}

/**
 * @brief Performs the dot product between two 3-component vectors.
 * @remark This is intended when dsSIMDFeatures_Double2 and dsSIMDFeatures_FMA are available, where
 *     more assumptions can be made about available operations. No FMA operations are performed.
 * @param a0 The first two components of the first vector.
 * @param a1 The second two components of the first vector.
 * @return b0 The first two components of the second vector.
 * @return b0 The second two components of the second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + a.z*b.z.
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot3FMA2d(dsSIMD2d a0, dsSIMD2d a1, dsSIMD2d b0, dsSIMD2d b1)
{
#if DS_X86
	return dsSIMD2d_add(_mm_dp_pd(a0, b0, 0x33), dsSIMD2d_set1FromVec(dsSIMD2d_mul(a1, b1), 0));
#else
	dsSIMD2d ab0 = dsSIMD2d_mul(a0, b0);
	dsSIMD2d ab1 = dsSIMD2d_mul(a1, b1);
	dsSIMD2d abxy = dsSIMD2d_add(dsSIMD2d_set1FromVec(ab0, 0), dsSIMD2d_set1FromVec(ab0, 1));
	return dsSIMD2d_add(abxy, dsSIMD2d_set1FromVec(ab1, 0));
#endif
}

/**
 * @brief Performs the dot product between two 2-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double2 is available, and will use the most
 *     efficient implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to a.x*b.x + a.y*b.y.
 */
DS_ALWAYS_INLINE dsSIMD2d dsDot2FMA2d(dsSIMD2d a, dsSIMD2d b)
{
#if DS_X86
	return _mm_dp_pd(a, b, 0x33);
#else
	dsSIMD2d ab = dsSIMD2d_mul(a, b);
	return dsSIMD2d_hadd(ab, ab);
#endif
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4)

/**
 * @brief Performs the dot product between two 4-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use the most efficient
 *     implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + (a.z*b.z + a.w*b.w).
 */
DS_ALWAYS_INLINE dsSIMD4d dsDot4SIMD4d(dsSIMD4d a, dsSIMD4d b)
{
	dsSIMD4d ab = dsSIMD4d_mul(a, b);
	ab = dsSIMD4d_hadd(ab, ab);
#if DS_X86
	// Expected to be faster since permutations are fairly slow across 128-bit boundaries.
	return dsSIMD4d_add(ab, _mm256_permute4x64_pd(ab, _MM_SHUFFLE(0, 0, 2, 2)));
#else
	return dsSIMD4d_add(dsSIMD4d_set1FromVec(ab, 0), dsSIMD4d_set1FromVec(ab, 2));
#endif
}

/**
 * @brief Performs the dot product between two 3-component vectors.
 * @remark This can be used when dsSIMDFeatures_Double4 is available, and will use the most efficient
 *     implementation based on what is enabled at compile time.
 * @param a The first vector.
 * @return b The second vector.
 * @return A vector with all components set to (a.x*b.x + a.y*b.y) + a.z*b.z.
 */
DS_ALWAYS_INLINE dsSIMD4d dsDot3SIMD4d(dsSIMD4d a, dsSIMD4d b)
{
	dsSIMD4d ab = dsSIMD4d_mul(a, b);
	dsSIMD4d abxy = dsSIMD4d_add(dsSIMD4d_set1FromVec(ab, 0), dsSIMD4d_set1FromVec(ab, 1));
	return dsSIMD4d_add(abxy, dsSIMD4d_set1FromVec(ab, 2));
}

DS_SIMD_END()

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif
