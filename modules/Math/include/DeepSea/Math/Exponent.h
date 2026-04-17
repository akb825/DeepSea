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
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/MathImpl.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Custom implementations for exponential and logarithmic functions.
 *
 * Custom implementations are provided both for performance and cross-platform consistent results
 * when deterministic math is enabled. SIMD versions are also provided to allow for parallelization
 * at the instruction level.
 *
 * Limited validity checks are performed to ensure maximum performance. Overflow and underflow
 * checks are performed to return infinity or zero where appropriate. However, mathematically
 * invalid situations such as lograithm of 0 or a negative number aren't explicitly checked.
 *
 * Subnormal values are handled by these functions, however they will come at a performance cost. In
 * the case of SIMD implementations, some operations will perform a quick check for subnormal and
 * fall back to scalar implementaitons for that operation if any subnormals appear.
 *
 * Implementations are adapted from the Cephes library by Stephen L. Moshier, found at
 * http://www.moshier.net/ From the original library's readme: may be used freely but it comes with
 * no support or guarantee.
 */

/**
 * @brief Splits the power of two exponent out of a floating-point value.
 *
 * If +-0, infinity, or NaN is provided, x will be returned unmodified and the power of two will be
 * 0.
 *
 * @param[out] outPow2 The power of 2 component to multiply by the return value to yield x.
 * @param x The value to split.
 * @return The fractional portion of x, expected to be in the range +-[0.5, 1) if x is not +-0,
 *     +-infinity, or NaN.
 */
DS_MATH_EXPORT inline float dsSplitPow2f(int* outPow2, float x);

/** @copydoc dsSplitPow2f() */
DS_MATH_EXPORT inline double dsSplitPow2d(int* outPow2, double x);

/**
 * @brief Multiplies a floating-point value by a power of two.
 *
 * If the power of two is too small, it will underflow to +-0. Similarly, if the power of two is too
 * large, it will overflow to +- infinity.
 *
 * @param x The base floating-point value.
 * @param pow2 The power of two to multiply by.
 * @return The result of x*2^pow2.
 */
DS_MATH_EXPORT inline float dsMulPow2f(float x, int pow2);

/** @copydoc dsMulPow2f() */
DS_MATH_EXPORT inline double dsMulPow2d(double x, int pow2);

/// @cond
#define DS_FLT_SIGN_BIT 0x80000000
#define DS_FLT_EXP_BITS 0x7F800000
#define DS_FLT_MANTISSA_BITS 0x7FFFFF
#define DS_FLT_EXP_BIT_COUNT 8
#define DS_FLT_MANTISSA_BIT_COUNT 23
#define DS_FLT_EXP_OFFSET 127

#define DS_DBL_SIGN_BIT 0x8000000000000000ULL
#define DS_DBL_EXP_BITS 0x7FF0000000000000ULL
#define DS_DBL_MANTISSA_BITS 0xFFFFFFFFFFFFFULL
#define DS_DBL_EXP_BIT_COUNT 11
#define DS_DBL_MANTISSA_BIT_COUNT 52
#define DS_DBL_EXP_OFFSET 1023

DS_ALWAYS_INLINE uint32_t dsSubnormToNormBitsf(unsigned int* outPow2, uint32_t xi)
{
	DS_ASSERT(outPow2);
	DS_ASSERT(xi);
	DS_ASSERT((xi & DS_FLT_EXP_BITS) == 0);

	uint32_t signBit = xi & DS_FLT_SIGN_BIT;
	uint32_t mantissa = xi & DS_FLT_MANTISSA_BITS;
	unsigned int normalOffset = dsClz(mantissa) - DS_FLT_EXP_BIT_COUNT;
	*outPow2 = normalOffset;
	return (mantissa << normalOffset) | signBit;
}

DS_ALWAYS_INLINE uint64_t dsSubnormToNormBitsd(unsigned int* outPow2, uint64_t xi)
{
	DS_ASSERT(outPow2);
	DS_ASSERT(xi);
	DS_ASSERT((xi & DS_DBL_EXP_BITS) == 0);

	uint64_t signBit = xi & DS_DBL_SIGN_BIT;
	uint64_t mantissa = xi & DS_DBL_MANTISSA_BITS;
	unsigned int normalOffset = dsClz64(mantissa) - DS_DBL_EXP_BIT_COUNT;
	*outPow2 = normalOffset;
	return (mantissa << normalOffset) | signBit;
}

DS_ALWAYS_INLINE uint32_t dsNormToSubnormBitsf(uint32_t xi, int targetPow2)
{
	DS_ASSERT(targetPow2 <= -DS_FLT_EXP_OFFSET);
	unsigned int subnormPow2 = -(targetPow2 + DS_FLT_EXP_OFFSET - 1);
	subnormPow2 = dsMin(subnormPow2, DS_FLT_MANTISSA_BIT_COUNT + 1);
	uint32_t signBit = xi & DS_FLT_SIGN_BIT;
	// Mantissa plus implicit leading 1.
	uint32_t mantissa = (xi & DS_FLT_MANTISSA_BITS) | 0x800000;
	return (mantissa >> subnormPow2) | signBit;
}

DS_ALWAYS_INLINE uint64_t dsNormToSubnormBitsd(uint64_t xi, int targetPow2)
{
	DS_ASSERT(targetPow2 <= -DS_DBL_EXP_OFFSET);
	unsigned int subnormPow2 = -(targetPow2 + DS_DBL_EXP_OFFSET - 1);
	subnormPow2 = dsMin(subnormPow2, DS_DBL_MANTISSA_BIT_COUNT + 1);
	uint64_t signBit = xi & DS_DBL_SIGN_BIT;
	// Mantissa plus implicit leading 1.
	uint64_t mantissa = (xi & DS_DBL_MANTISSA_BITS) | 0x10000000000000ULL;
	return (mantissa >> subnormPow2) | signBit;
}

/// @endcond

DS_MATH_EXPORT inline float dsSplitPow2f(int* outPow2, float x)
{
	DS_ASSERT(outPow2);

	uint32_t xi = *(uint32_t*)&x;
	uint32_t expBits = xi & DS_FLT_EXP_BITS;
	uint32_t mantissaBits = xi & DS_FLT_MANTISSA_BITS;
	*outPow2 = 0;
	// Check for subnormal.
	if (DS_EXPECT(!expBits && mantissaBits, false))
	{
		unsigned int subnormPow2;
		xi = dsSubnormToNormBitsf(&subnormPow2, xi);
		*outPow2 = -subnormPow2;
		expBits = xi & DS_FLT_EXP_BITS;
	}

	if (expBits && expBits != DS_FLT_EXP_BITS)
	{
		// Aim for a final expornent of -1 to have the value in the range [0.5, 1.0).
		unsigned int targetExpOffset = DS_FLT_EXP_OFFSET - 1;
		*outPow2 += (int)(expBits >> DS_FLT_MANTISSA_BIT_COUNT) - targetExpOffset;
		xi = (xi & ~DS_FLT_EXP_BITS) | (targetExpOffset << DS_FLT_MANTISSA_BIT_COUNT);
	}
	return *(float*)&xi;
}

DS_MATH_EXPORT inline double dsSplitPow2d(int* outPow2, double x)
{
	DS_ASSERT(outPow2);

	uint64_t xi = *(uint64_t*)&x;
	uint64_t expBits = xi & DS_DBL_EXP_BITS;
	uint64_t mantissaBits = xi & DS_DBL_MANTISSA_BITS;
	*outPow2 = 0;
	// Check for subnormal.
	if (DS_EXPECT(!expBits && mantissaBits, false))
	{
		unsigned int subnormPow2;
		xi = dsSubnormToNormBitsd(&subnormPow2, xi);
		*outPow2 = -subnormPow2;
		expBits = xi & DS_DBL_EXP_BITS;
	}

	if (expBits && expBits != DS_DBL_EXP_BITS)
	{
		unsigned int targetExpOffset = DS_DBL_EXP_OFFSET - 1;
		*outPow2 += (int)(expBits >> DS_DBL_MANTISSA_BIT_COUNT) - targetExpOffset;
		xi = (xi & ~DS_DBL_EXP_BITS) | ((uint64_t)targetExpOffset << DS_DBL_MANTISSA_BIT_COUNT);
	}
	return *(double*)&xi;
}

DS_MATH_EXPORT inline float dsMulPow2f(float x, int pow2)
{
	uint32_t xi = *(uint32_t*)&x;
	uint32_t expBits = xi & DS_FLT_EXP_BITS;
	uint32_t mantissaBits = xi & DS_FLT_MANTISSA_BITS;
	// Check for subnormal.
	if (DS_EXPECT(!expBits && mantissaBits, false))
	{
		unsigned int subnormPow2;
		xi = dsSubnormToNormBitsf(&subnormPow2, xi);
		pow2 -= subnormPow2;
		expBits = xi & DS_FLT_EXP_BITS;
	}

	if (expBits && expBits != DS_FLT_EXP_BITS)
	{
		pow2 += (int)(expBits >> DS_FLT_MANTISSA_BIT_COUNT) - DS_FLT_EXP_OFFSET;
		if (pow2 <= -DS_FLT_EXP_OFFSET)
		{
			// Underflow: try to change to subnormal.
			xi = dsNormToSubnormBitsf(xi, pow2);
		}
		else
		{
			uint32_t signedMantissaBits;
			if (pow2 > DS_FLT_EXP_OFFSET)
			{
				// Overflow: set up the parameters to construct infinity.
				signedMantissaBits = xi & DS_FLT_SIGN_BIT;
				pow2 = DS_FLT_EXP_OFFSET + 1;
			}
			else
				signedMantissaBits = xi & ~DS_FLT_EXP_BITS;
			xi = signedMantissaBits | ((pow2 + DS_FLT_EXP_OFFSET) << DS_FLT_MANTISSA_BIT_COUNT);
		}
	}
	return *(float*)&xi;
}

DS_MATH_EXPORT inline double dsMulPow2d(double x, int pow2)
{
	uint64_t xi = *(uint64_t*)&x;
	uint64_t expBits = xi & DS_DBL_EXP_BITS;
	uint64_t mantissaBits = xi & DS_DBL_MANTISSA_BITS;
	// Check for subnormal.
	if (DS_EXPECT(!expBits && mantissaBits, false))
	{
		unsigned int subnormPow2;
		xi = dsSubnormToNormBitsd(&subnormPow2, xi);
		pow2 -= subnormPow2;
		expBits = xi & DS_DBL_EXP_BITS;
	}

	if (expBits && expBits != DS_DBL_EXP_BITS)
	{
		pow2 += (int)(expBits >> DS_DBL_MANTISSA_BIT_COUNT) - DS_DBL_EXP_OFFSET;
		if (pow2 <= -DS_DBL_EXP_OFFSET)
		{
			// Underflow: try to change to subnormal.
			xi = dsNormToSubnormBitsd(xi, pow2);
		}
		else
		{
			uint64_t signedMantissaBits;
			if (pow2 > DS_DBL_EXP_OFFSET)
			{
				// Overflow: set up the parameters to construct infinity.
				signedMantissaBits = xi & DS_DBL_SIGN_BIT;
				pow2 = DS_DBL_EXP_OFFSET + 1;
			}
			else
				signedMantissaBits = xi & ~DS_DBL_EXP_BITS;
			xi = signedMantissaBits |
				((uint64_t)(pow2 + DS_DBL_EXP_OFFSET) << DS_DBL_MANTISSA_BIT_COUNT);
		}
	}
	return *(double*)&xi;
}

#undef DS_FLT_SIGN_BIT
#undef DS_FLT_EXP_BITS
#undef DS_FLT_MANTISSA_BITS
#undef DS_FLT_EXP_BIT_COUNT
#undef DS_FLT_MANTISSA_BIT_COUNT
#undef DS_FLT_EXP_OFFSET

#undef DS_DBL_SIGN_BIT
#undef DS_DBL_EXP_BITS
#undef DS_DBL_MANTISSA_BITS
#undef DS_DBL_EXP_BIT_COUNT
#undef DS_DBL_MANTISSA_BIT_COUNT
#undef DS_DBL_EXP_OFFSET

#ifdef __cplusplus
}
#endif
