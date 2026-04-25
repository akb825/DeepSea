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
#include <math.h>

#if DS_X86
#include <smmintrin.h>
#elif DS_ARM_64
#include <arm_neon.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Wrappers for rounding functions for scalars.
 *
 * These are simple wrappers that use the hardware rounding implementations where available, and
 * falls back to the library function when not. This avoids relying on the compiler deciding whether
 * to use a hardware variant, which can be inconsistent between versions. The primary rounding types
 * are provided: round (closest integer), truncate (toward 0), floor (toward -infinity), and ceil
 * (toward +infinity).
 *
 * Only scalar types (float, double) are covered as SIMD functions are already provided for the
 * hardware implementations on the respective platforms.
 *
 * Current targets that include hardware implementations are:
 * - x86 with SSE 4.1.
 * - ARM64
 */

/**
 * @brief Define for whether hardware rounding is supported.
 *
 * If 0, callers may choose to use fallback implementations that are perhaps not 100% bit for bit
 * consistent but faster at runtime.
 */
#if DS_ARM_64 || defined(__SSE4_1__) || defined(__AVX__) || DS_X86_ARCH_LEVEL >= 2
#define DS_HAS_HARDWARE_ROUNDING 1
#else
#define DS_HAS_HARDWARE_ROUNDING 0
#endif

/**
 * @brief Rounds a value to the nearest integer.
 *
 * When ending with exactly 0.5, the value is rounded to the nearest even integer. This is
 * consistent with the typical IEEE rounding bode, though different from the standard library round
 * functions that always round 0.5 up.
 *
 * @param x The value to round.
 * @return The value of x rounded to the nearest integer.
 */
DS_ALWAYS_INLINE float dsRoundf(float x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtss_f32(_mm_round_ps(_mm_set_ss(x), _MM_FROUND_TO_NEAREST_INT));
#elif DS_ARM
	return vdups_lane_f32(vrndn_f32(vdup_n_f32(x)), 0);
#endif
#else
	return rintf(x);
#endif
}

/** @copydoc dsRoundf() */
DS_ALWAYS_INLINE double dsRoundd(double x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtsd_f64(_mm_round_pd(_mm_set_sd(x), _MM_FROUND_TO_NEAREST_INT));
#elif DS_ARM
	return vget_lane_f64(vrndn_f64(vdup_n_f64(x)), 0);
#endif
#else
	return rint(x);
#endif
}

/**
 * @brief Rounds a value towards zero.
 * @param x The value to round.
 * @return The value of x rounded towards zero.
 */
DS_ALWAYS_INLINE float dsTruncf(float x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtss_f32(_mm_round_ps(_mm_set_ss(x), _MM_FROUND_TO_ZERO));
#elif DS_ARM
	return vdups_lane_f32(vrnd_f32(vdup_n_f32(x)), 0);
#endif
#else
	return truncf(x);
#endif
}

/** @copydoc dsTruncf() */
DS_ALWAYS_INLINE double dsTruncd(double x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtsd_f64(_mm_round_pd(_mm_set_sd(x), _MM_FROUND_TO_ZERO));
#elif DS_ARM
	return vget_lane_f64(vrnd_f64(vdup_n_f64(x)), 0);
#endif
#else
	return trunc(x);
#endif
}

/**
 * @brief Rounds a value towards -infinity.
 * @param x The value to round.
 * @return The value of x rounded towards -infinity.
 */
DS_ALWAYS_INLINE float dsFloorf(float x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtss_f32(_mm_round_ps(_mm_set_ss(x), _MM_FROUND_TO_NEG_INF));
#elif DS_ARM
	return vdups_lane_f32(vrndm_f32(vdup_n_f32(x)), 0);
#endif
#else
	return floorf(x);
#endif
}

/** @copydoc dsFloorf() */
DS_ALWAYS_INLINE double dsFloord(double x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtsd_f64(_mm_round_pd(_mm_set_sd(x), _MM_FROUND_TO_NEG_INF));
#elif DS_ARM
	return vget_lane_f64(vrndm_f64(vdup_n_f64(x)), 0);
#endif
#else
	return floor(x);
#endif
}

/**
 * @brief Rounds a value towards +infinity.
 * @param x The value to round.
 * @return The value of x rounded towards +infinity.
 */
DS_ALWAYS_INLINE float dsCeilf(float x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtss_f32(_mm_round_ps(_mm_set_ss(x), _MM_FROUND_TO_POS_INF));
#elif DS_ARM
	return vdups_lane_f32(vrndp_f32(vdup_n_f32(x)), 0);
#endif
#else
	return ceilf(x);
#endif
}

/** @copydoc dsCeilf() */
DS_ALWAYS_INLINE double dsCeild(double x)
{
#if DS_HAS_HARDWARE_ROUNDING
#if DS_X86
	return _mm_cvtsd_f64(_mm_round_pd(_mm_set_sd(x), _MM_FROUND_TO_POS_INF));
#elif DS_ARM
	return vget_lane_f64(vrndp_f64(vdup_n_f64(x)), 0);
#endif
#else
	return ceil(x);
#endif
}

#ifdef __cplusplus
}
#endif
