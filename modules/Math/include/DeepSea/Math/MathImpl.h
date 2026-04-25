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
#include <DeepSea/Math/Round.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Shared implementations for various math functions.
 *
 * These are typically manual bit manipulation to avoid branching, such as conditionally swapping
 * the sign bit or selecting a value without a branch, avoiding situations where optimizers may not
 * make the "correct" decision.
 */

/// @cond

DS_ALWAYS_INLINE uint32_t dsMathImplExtractSignBitf(float value)
{
	union {float f; uint32_t i;} valuefi;
	valuefi.f = value;
	return valuefi.i & 0x80000000;
}

DS_ALWAYS_INLINE uint64_t dsMathImplExtractSignBitd(double value)
{
	union {double f; uint64_t i;} valuefi;
	valuefi.f = value;
	return valuefi.i & 0x8000000000000000ULL;
}

DS_ALWAYS_INLINE float dsMathImplConditionalNegatef(float value, uint32_t signBit)
{
	union {float f; uint32_t i;} valuefi;
	valuefi.f = value;
	valuefi.i ^= signBit;
	return valuefi.f;
}

DS_ALWAYS_INLINE double dsMathImplConditionalNegated(double value, uint64_t signBit)
{
	union {double f; uint64_t i;} valuefi;
	valuefi.f = value;
	valuefi.i ^= signBit;
	return valuefi.f;
}

DS_ALWAYS_INLINE float dsMathImplSelectf(uint32_t which, float trueValue, float falseValue)
{
	union {float f; uint32_t i;} trueValuefi, falseValuefi;
	trueValuefi.f = trueValue;
	falseValuefi.f = falseValue;
	uint32_t mask = which - 1;
	trueValuefi.i = (falseValuefi.i & mask) | (trueValuefi.i & ~mask);
	return trueValuefi.f;
}

DS_ALWAYS_INLINE double dsMathImplSelectd(uint64_t which, double trueValue, double falseValue)
{
	union {double f; uint64_t i;} trueValuefi, falseValuefi;
	trueValuefi.f = trueValue;
	falseValuefi.f = falseValue;
	uint64_t mask = which - 1;
	trueValuefi.i = (falseValuefi.i & mask) | (trueValuefi.i & ~mask);
	return trueValuefi.f;
}

DS_ALWAYS_INLINE float dsMathImplMaskf(uint32_t mask, float value)
{
	union {float f; uint32_t i;} valuefi;
	valuefi.f = value;
	valuefi.i = valuefi.i & mask;
	return valuefi.f;
}

DS_ALWAYS_INLINE double dsMathImplMaskd(uint64_t mask, double value)
{
	union {double f; uint64_t i;} valuefi;
	valuefi.f = value;
	valuefi.i = valuefi.i & mask;
	return valuefi.f;
}

DS_ALWAYS_INLINE void dsMathImplFastRoundif(int* outInt, float* outFloat, float value)
{
#if !DS_HAS_HARDWARE_ROUNDING && (DS_X86_64 || defined(__SSE2__) || _M_IX86_FP >= 2)
	// Want to use this path for both deterministic and non-deterministic math when direct
	// hardware rounding not available.
	*outInt = _mm_cvtsi128_si32(_mm_cvtps_epi32(_mm_set_ss(value)));
	*outFloat = (float)*outInt;
#elif DS_HAS_HARDWARE_ROUNDING || DS_DETERMINISTIC_MATH
	*outFloat = dsRoundf(value);
	*outInt = (int)*outFloat;
#else
	uint32_t valuei = *(uint32_t*)&value;
	uint32_t signBit = valuei & 0x80000000;
	*outInt = (int)(value + dsMathImplConditionalNegatef(0.5f, signBit));
	*outFloat = (float)*outInt;
#endif
}

DS_ALWAYS_INLINE void dsMathImplFastRoundid(int* outInt, double* outDouble, double value)
{
#if !DS_HAS_HARDWARE_ROUNDING && (DS_X86_64 || defined(__SSE2__) || _M_IX86_FP >= 2)
	// Want to use this path for both deterministic and non-deterministic math when direct
	// hardware rounding not available.
	*outInt = _mm_cvtsi128_si32(_mm_cvtpd_epi32(_mm_set_sd(value)));
	*outDouble = (double)*outInt;
#elif DS_HAS_HARDWARE_ROUNDING || DS_DETERMINISTIC_MATH
	*outDouble = dsRoundd(value);
	*outInt = (int)*outDouble;
#else
	uint64_t valuei = *(uint64_t*)&value;
	uint64_t signBit = valuei & 0x8000000000000000ULL;
	*outInt = (int)(value + dsMathImplConditionalNegated(0.5, signBit));
	*outDouble = (double)*outInt;
#endif
}

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)

DS_ALWAYS_INLINE dsSIMD4fb dsMathImplExtractSignBitSIMD4f(dsSIMD4f value)
{
	return dsSIMD4fb_and(dsSIMD4fb_fromFloatBitfield(value), dsSIMD4fb_set1(0x80000000));
}

DS_ALWAYS_INLINE dsSIMD4f dsMathImplConditionalNegateSIMD4f(dsSIMD4f value, dsSIMD4fb signBit)
{
	return dsSIMD4fb_toFloatBitfield(dsSIMD4fb_xor(dsSIMD4fb_fromFloatBitfield(value), signBit));
}

DS_ALWAYS_INLINE dsSIMD4f dsMathImplMaskSIMD4f(dsSIMD4fb mask, dsSIMD4f value)
{
	return dsSIMD4fb_toFloatBitfield(dsSIMD4fb_and(mask, dsSIMD4fb_fromFloatBitfield(value)));
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)

DS_ALWAYS_INLINE dsSIMD2db dsMathImplExtractSignBitSIMD2d(dsSIMD2d value)
{
	return dsSIMD2db_and(
		dsSIMD2db_fromDoubleBitfield(value), dsSIMD2db_set1(0x8000000000000000ULL));
}

DS_ALWAYS_INLINE dsSIMD2d dsMathImplConditionalNegateSIMD2d(dsSIMD2d value, dsSIMD2db signBit)
{
	return dsSIMD2db_toDoubleBitfield(dsSIMD2db_xor(dsSIMD2db_fromDoubleBitfield(value), signBit));
}

DS_ALWAYS_INLINE dsSIMD2d dsMathImplMaskSIMD2d(dsSIMD2db mask, dsSIMD2d value)
{
	return dsSIMD2db_toDoubleBitfield(dsSIMD2db_and(mask, dsSIMD2db_fromDoubleBitfield(value)));
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT)

DS_ALWAYS_INLINE dsSIMD4db dsMathImplExtractSignBitSIMD4d(dsSIMD4d value)
{
	return dsSIMD4db_and(
		dsSIMD4db_fromDoubleBitfield(value), dsSIMD4db_set1(0x8000000000000000ULL));
}

DS_ALWAYS_INLINE dsSIMD4d dsMathImplConditionalNegateSIMD4d(dsSIMD4d value, dsSIMD4db signBit)
{
	return dsSIMD4db_toDoubleBitfield(dsSIMD4db_xor(dsSIMD4db_fromDoubleBitfield(value), signBit));
}

DS_ALWAYS_INLINE dsSIMD4d dsMathImplMaskSIMD4d(dsSIMD4db mask, dsSIMD4d value)
{
	return dsSIMD4db_toDoubleBitfield(dsSIMD4db_and(mask, dsSIMD4db_fromDoubleBitfield(value)));
}

DS_SIMD_END()

#endif

/// @endcond

#ifdef __cplusplus
}
#endif
