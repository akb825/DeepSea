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

#if DS_X86_32 || DS_X86_64
#include <emmintrin.h>
#include <xmmintrin.h>
#elif DS_ARM_64
#include <arm_neon.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Wrappers for sqrt functions for scalars.
 *
 * These are simple wrappers that use the hardware sqrt implementation where available, and falls
 * back to the library function when not. The C math library appears to be inconsistent in this
 * regard, for example glibc and GCC sqrt() appears to use the hardware implementation, but not
 * sqrtf(). In all cases, IEEE complience requires accuracy for sqrt operations within the full bits
 * of precision, so the results should be identical in all situations.
 *
 * Only scalar types (float, double) are covered as SIMD functions are already provided for the
 * hardware implementations on the respective platforms.
 *
 * Current targets that include hardware implementations are:
 * - x86 with SSE for float and SSE2 for double. This should always be enabled unless building for
 *   32-bit with x86 architecture level 0.
 * - ARM64
 */

/**
 * @brief Takes the square root of a value.
 * @param x The value to take the square root of.
 * @return The square root of x.
 */
DS_ALWAYS_INLINE float dsSqrtf(float x)
{
#if DS_X86_64 || defined(__SSE__) || _M_IX86_FP >= 1
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
#elif DS_ARM_64
	return vdups_lane_f32(vsqrt_f32(vdup_n_f32(x)), 0);
#else
	return sqrtf(x);
#endif
}

/** @copydoc dsSqrtf() */
DS_ALWAYS_INLINE double dsSqrtd(double x)
{
#if DS_X86_64 || defined(__SSE2__) || _M_IX86_FP >= 2
	return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(x)));
#elif DS_ARM_64
	return vget_lane_f64(vsqrt_f64(vdup_n_f64(x)), 0);
#else
	return sqrt(x);
#endif
}

#ifdef __cplusplus
}
#endif
