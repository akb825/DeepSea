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

#include <DeepSea/Math/SIMD/SIMD.h>
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

#if DS_HAS_SIMD

/**
 * @brief Splits the power of two exponents out of four floating-point values.
 *
 * If +-0, infinity, or NaN is provided, x will be returned unmodified and the power of two will be
 * 0.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param[out] outPow2 The powers of 2 component to multiply by the return value to yield x.
 * @param x The values to split.
 * @return The fractional portions of x, expected to be in the range +-[0.5, 1) if x is not +-0,
 *     +-infinity, or NaN.
 */
DS_MATH_EXPORT inline dsSIMD4f dsSplitPow2SIMD4f(dsSIMD4fb* outPow2, dsSIMD4f x);

/**
 * @brief Multiplies four floating-point values by four powers of two.
 *
 * If the power of two is too small, it will underflow to +-0. Similarly, if the power of two is too
 * large, it will overflow to +- infinity.
 *
 * @remark This can be used when dsSIMDFeatures_Float4 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param x The base floating-point values.
 * @param pow2 The powers of two to multiply by.
 * @return The result of x*2^pow2.
 */
DS_MATH_EXPORT inline dsSIMD4f dsMulPow2SIMD4f(dsSIMD4f x, dsSIMD4fb pow2);

/**
 * @brief Splits the power of two exponents out of two floating-point values.
 *
 * If +-0, infinity, or NaN is provided, x will be returned unmodified and the power of two will be
 * 0.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param[out] outPow2 The powers of 2 component to multiply by the return value to yield x.
 * @param x The values to split.
 * @return The fractional portions of x, expected to be in the range +-[0.5, 1) if x is not +-0,
 *     +-infinity, or NaN.
 */
DS_MATH_EXPORT inline dsSIMD2d dsSplitPow2SIMD2d(dsSIMD2db* outPow2, dsSIMD2d x);

/**
 * @brief Multiplies two floating-point values by four powers of two.
 *
 * If the power of two is too small, it will underflow to +-0. Similarly, if the power of two is too
 * large, it will overflow to +- infinity.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param x The base floating-point values.
 * @param pow2 The powers of two to multiply by.
 * @return The result of x*2^pow2.
 */
DS_MATH_EXPORT inline dsSIMD2d dsMulPow2SIMD2d(dsSIMD2d x, dsSIMD2db pow2);

/**
 * @brief Splits the power of two exponents out of four floating-point values.
 *
 * If +-0, infinity, or NaN is provided, x will be returned unmodified and the power of two will be
 * 0.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param[out] outPow2 The powers of 2 component to multiply by the return value to yield x.
 * @param x The values to split.
 * @return The fractional portions of x, expected to be in the range +-[0.5, 1) if x is not +-0,
 *     +-infinity, or NaN.
 */
DS_MATH_EXPORT inline dsSIMD4d dsSplitPow2SIMD4d(dsSIMD4db* outPow2, dsSIMD4d x);

/**
 * @brief Multiplies four floating-point values by four powers of two.
 *
 * If the power of two is too small, it will underflow to +-0. Similarly, if the power of two is too
 * large, it will overflow to +- infinity.
 *
 * @remark This can be used when dsSIMDFeatures_Double2 and dsSIMDFeatures_Int are available.
 * @remark If any subnormal values are present, this will need to fall to scalar operations at
 *     reduced performance.
 * @param x The base floating-point values.
 * @param pow2 The powers of two to multiply by.
 * @return The result of x*2^pow2.
 */
DS_MATH_EXPORT inline dsSIMD4d dsMulPow2SIMD4d(dsSIMD4d x, dsSIMD4db pow2);

#endif // DS_HAS_SIMD

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
	DS_ASSERT(xi & DS_FLT_MANTISSA_BITS);
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
	DS_ASSERT(xi & DS_DBL_MANTISSA_BITS);
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

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT)

DS_MATH_EXPORT inline dsSIMD4f dsSplitPow2SIMD4f(dsSIMD4fb* outPow2, dsSIMD4f x)
{
	DS_ASSERT(outPow2);

	dsSIMD4fb zero = dsSIMD4fb_set1(0);
	dsSIMD4fb fltExpBits = dsSIMD4fb_set1(DS_FLT_EXP_BITS);
	dsSIMD4fb fltMantissaBits = dsSIMD4fb_set1(DS_FLT_MANTISSA_BITS);
	dsSIMD4fb targetExpOffset = dsSIMD4fb_set1(DS_FLT_EXP_OFFSET - 1);

	dsSIMD4fb xi = dsSIMD4fb_fromFloatBitfield(x);
	dsSIMD4fb expBits = dsSIMD4fb_and(xi, fltExpBits);
	dsSIMD4fb mantissaBits = dsSIMD4fb_and(xi, fltMantissaBits);
	dsSIMD4fb startPow2;
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD4fb subnormal = dsSIMD4fb_andnot(
		dsSIMD4fb_cmpeq(mantissaBits, zero), dsSIMD4fb_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD4fb_any(subnormal), false))
	{
		DS_ALIGN(16) uint32_t subnormPow2[4] = {}, scalarXi[4], scalarSubnormal[4];
		dsSIMD4fb_store(scalarXi, xi);
		dsSIMD4fb_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
				scalarXi[i] = dsSubnormToNormBitsf(subnormPow2 + i, scalarXi[i]);
		}
		xi = dsSIMD4fb_load(scalarXi);
		startPow2 = dsSIMD4fb_neg(dsSIMD4fb_load(subnormPow2));
		expBits = dsSIMD4fb_and(xi, fltExpBits);
	}
	else
		startPow2 = dsSIMD4fb_set1(0);

	// Prefer == compares for SIMD operations for performance.
	dsSIMD4fb keepOrig = dsSIMD4fb_or(dsSIMD4fb_cmpeq(expBits, zero),
		dsSIMD4fb_cmpeq(expBits, fltExpBits));
	dsSIMD4fb adjustedPow2 = dsSIMD4fb_add(startPow2, dsSIMD4fb_sub(
		dsSIMD4fb_shiftRightConst(expBits, DS_FLT_MANTISSA_BIT_COUNT), targetExpOffset));
	dsSIMD4fb adjustedXi = dsSIMD4fb_or(dsSIMD4fb_andnot(fltExpBits, xi),
		dsSIMD4fb_shiftLeftConst(targetExpOffset, DS_FLT_MANTISSA_BIT_COUNT));
	*outPow2 = dsSIMD4fb_select(keepOrig, startPow2, adjustedPow2);
	return dsSIMD4fb_toFloatBitfield(dsSIMD4fb_select(keepOrig, xi, adjustedXi));
}

DS_MATH_EXPORT inline dsSIMD4f dsMulPow2SIMD4f(dsSIMD4f x, dsSIMD4fb pow2)
{
	dsSIMD4fb zero = dsSIMD4fb_set1(0);
	dsSIMD4fb fltSignBit = dsSIMD4fb_set1(DS_FLT_SIGN_BIT);
	dsSIMD4fb fltExpBits = dsSIMD4fb_set1(DS_FLT_EXP_BITS);
	dsSIMD4fb fltMantissaBits = dsSIMD4fb_set1(DS_FLT_MANTISSA_BITS);
	dsSIMD4fb fltExpOffset = dsSIMD4fb_set1(DS_FLT_EXP_OFFSET);
	dsSIMD4fb negFltExpOffset = dsSIMD4fb_set1(-DS_FLT_EXP_OFFSET);

	dsSIMD4fb xi = dsSIMD4fb_fromFloatBitfield(x);
	dsSIMD4fb expBits = dsSIMD4fb_and(xi, fltExpBits);
	dsSIMD4fb mantissaBits = dsSIMD4fb_and(xi, fltMantissaBits);
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD4fb subnormal = dsSIMD4fb_andnot(
		dsSIMD4fb_cmpeq(mantissaBits, zero), dsSIMD4fb_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD4fb_any(subnormal), false))
	{
		DS_ALIGN(16) uint32_t subnormPow2[4] = {}, scalarXi[4], scalarSubnormal[4];
		dsSIMD4fb_store(scalarXi, xi);
		dsSIMD4fb_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
				scalarXi[i] = dsSubnormToNormBitsf(subnormPow2 + i, scalarXi[i]);
		}
		xi = dsSIMD4fb_load(scalarXi);
		pow2 = dsSIMD4fb_sub(pow2, dsSIMD4fb_load(subnormPow2));
		expBits = dsSIMD4fb_and(xi, fltExpBits);
	}

	// Prefer == compares for SIMD operations for performance.
	dsSIMD4fb keepOrig = dsSIMD4fb_or(dsSIMD4fb_cmpeq(expBits, zero),
		dsSIMD4fb_cmpeq(expBits, fltExpBits));
	pow2 = dsSIMD4fb_add(pow2, dsSIMD4fb_sub(
		dsSIMD4fb_shiftRightConst(expBits, DS_FLT_MANTISSA_BIT_COUNT), fltExpOffset));

	// Need to adjust any subnormal values with scalar operations.
	subnormal = dsSIMD4fb_andnot(keepOrig, dsSIMD4fb_cmples(pow2, negFltExpOffset));
	dsSIMD4fb subnormVals = xi;
	if (DS_EXPECT(dsSIMD4fb_any(subnormal), false))
	{
		DS_ALIGN(16) uint32_t scalarXi[4], scalarPow2[4], scalarSubnormal[4];
		dsSIMD4fb_store(scalarXi, xi);
		dsSIMD4fb_store(scalarPow2, pow2);
		dsSIMD4fb_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
				scalarXi[i] = dsNormToSubnormBitsf(scalarXi[i], scalarPow2[i]);
		}
		subnormVals = dsSIMD4fb_load(scalarXi);
	}

	// Set any overflow values to +-infinity.
	dsSIMD4fb overflow = dsSIMD4fb_cmpgts(pow2, fltExpOffset);
	dsSIMD4fb overflowVals = dsSIMD4fb_or(dsSIMD4fb_and(xi, fltSignBit), fltExpBits);

	// Expected value in the standard case.
	dsSIMD4fb adjustedXi = dsSIMD4fb_or(dsSIMD4fb_andnot(fltExpBits, xi),
		dsSIMD4fb_shiftLeftConst(dsSIMD4fb_add(pow2, fltExpOffset), DS_FLT_MANTISSA_BIT_COUNT));

	// Choose which to use.
	dsSIMD4fb chosenVals = dsSIMD4fb_select(subnormal, subnormVals, overflowVals);
	chosenVals = dsSIMD4fb_select(dsSIMD4fb_or(subnormal, overflow), chosenVals, adjustedXi);
	return dsSIMD4fb_toFloatBitfield(dsSIMD4fb_select(keepOrig, xi, chosenVals));
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT)

DS_MATH_EXPORT inline dsSIMD2d dsSplitPow2SIMD2d(dsSIMD2db* outPow2, dsSIMD2d x)
{
	DS_ASSERT(outPow2);

	dsSIMD2db zero = dsSIMD2db_set1(0);
	dsSIMD2db fltExpBits = dsSIMD2db_set1(DS_DBL_EXP_BITS);
	dsSIMD2db fltMantissaBits = dsSIMD2db_set1(DS_DBL_MANTISSA_BITS);
	dsSIMD2db targetExpOffset = dsSIMD2db_set1(DS_DBL_EXP_OFFSET - 1);

	dsSIMD2db xi = dsSIMD2db_fromDoubleBitfield(x);
	dsSIMD2db expBits = dsSIMD2db_and(xi, fltExpBits);
	dsSIMD2db mantissaBits = dsSIMD2db_and(xi, fltMantissaBits);
	dsSIMD2db startPow2;
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD2db subnormal = dsSIMD2db_andnot(
		dsSIMD2db_cmpeq(mantissaBits, zero), dsSIMD2db_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD2db_any(subnormal) != 0, false))
	{
		DS_ALIGN(16) uint64_t subnormPow2[2] = {}, scalarXi[2], scalarSubnormal[2];
		dsSIMD2db_store(scalarXi, xi);
		dsSIMD2db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 2; ++i)
		{
			if (scalarSubnormal[i])
			{
				unsigned int pow2;
				scalarXi[i] = dsSubnormToNormBitsd(&pow2, scalarXi[i]);
				subnormPow2[i] = pow2;
			}
		}
		xi = dsSIMD2db_load(scalarXi);
		startPow2 = dsSIMD2db_neg(dsSIMD2db_load(subnormPow2));
		expBits = dsSIMD2db_and(xi, fltExpBits);
	}
	else
		startPow2 = dsSIMD2db_set1(0);

	// Prefer == compares for SIMD operations for performance.
	dsSIMD2db keepOrig = dsSIMD2db_or(dsSIMD2db_cmpeq(expBits, zero),
		dsSIMD2db_cmpeq(expBits, fltExpBits));
	dsSIMD2db adjustedPow2 = dsSIMD2db_add(startPow2, dsSIMD2db_sub(
		dsSIMD2db_shiftRightConst(expBits, DS_DBL_MANTISSA_BIT_COUNT), targetExpOffset));
	dsSIMD2db adjustedXi = dsSIMD2db_or(dsSIMD2db_andnot(fltExpBits, xi),
		dsSIMD2db_shiftLeftConst(targetExpOffset, DS_DBL_MANTISSA_BIT_COUNT));
	*outPow2 = dsSIMD2db_select(keepOrig, startPow2, adjustedPow2);
	return dsSIMD2db_toDoubleBitfield(dsSIMD2db_select(keepOrig, xi, adjustedXi));
}

DS_MATH_EXPORT inline dsSIMD2d dsMulPow2SIMD2d(dsSIMD2d x, dsSIMD2db pow2)
{
	dsSIMD2db zero = dsSIMD2db_set1(0);
	dsSIMD2db fltSignBit = dsSIMD2db_set1(DS_DBL_SIGN_BIT);
	dsSIMD2db fltExpBits = dsSIMD2db_set1(DS_DBL_EXP_BITS);
	dsSIMD2db fltMantissaBits = dsSIMD2db_set1(DS_DBL_MANTISSA_BITS);
	dsSIMD2db fltExpOffset = dsSIMD2db_set1(DS_DBL_EXP_OFFSET);
	dsSIMD2db negFltExpOffset = dsSIMD2db_set1(-DS_DBL_EXP_OFFSET);

	dsSIMD2db xi = dsSIMD2db_fromDoubleBitfield(x);
	dsSIMD2db expBits = dsSIMD2db_and(xi, fltExpBits);
	dsSIMD2db mantissaBits = dsSIMD2db_and(xi, fltMantissaBits);
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD2db subnormal = dsSIMD2db_andnot(
		dsSIMD2db_cmpeq(mantissaBits, zero), dsSIMD2db_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD2db_any(subnormal) != 0, false))
	{
		DS_ALIGN(16) uint64_t subnormPow2[2] = {}, scalarXi[2], scalarSubnormal[2];
		dsSIMD2db_store(scalarXi, xi);
		dsSIMD2db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 2; ++i)
		{
			if (scalarSubnormal[i])
			{
				unsigned int pow2;
				scalarXi[i] = dsSubnormToNormBitsd(&pow2, scalarXi[i]);
				subnormPow2[i] = pow2;
			}
		}
		xi = dsSIMD2db_load(scalarXi);
		pow2 = dsSIMD2db_sub(pow2, dsSIMD2db_load(subnormPow2));
		expBits = dsSIMD2db_and(xi, fltExpBits);
	}

	// Prefer == compares for SIMD operations for performance.
	dsSIMD2db keepOrig = dsSIMD2db_or(dsSIMD2db_cmpeq(expBits, zero),
		dsSIMD2db_cmpeq(expBits, fltExpBits));
	pow2 = dsSIMD2db_add(pow2, dsSIMD2db_sub(
		dsSIMD2db_shiftRightConst(expBits, DS_DBL_MANTISSA_BIT_COUNT), fltExpOffset));

	// Need to adjust any subnormal values with scalar operations.
	subnormal = dsSIMD2db_andnot(keepOrig, dsSIMD2db_cmples(pow2, negFltExpOffset));
	dsSIMD2db subnormVals = xi;
	if (DS_EXPECT(dsSIMD2db_any(subnormal) != 0, false))
	{
		DS_ALIGN(16) uint64_t scalarXi[2], scalarPow2[2], scalarSubnormal[2];
		dsSIMD2db_store(scalarXi, xi);
		dsSIMD2db_store(scalarPow2, pow2);
		dsSIMD2db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 2; ++i)
		{
			if (scalarSubnormal[i])
				scalarXi[i] = dsNormToSubnormBitsd(scalarXi[i], (int)scalarPow2[i]);
		}
		subnormVals = dsSIMD2db_load(scalarXi);
	}

	// Set any overflow values to +-infinity.
	dsSIMD2db overflow = dsSIMD2db_cmpgts(pow2, fltExpOffset);
	dsSIMD2db overflowVals = dsSIMD2db_or(dsSIMD2db_and(xi, fltSignBit), fltExpBits);

	// Expected value in the standard case.
	dsSIMD2db adjustedXi = dsSIMD2db_or(dsSIMD2db_andnot(fltExpBits, xi),
		dsSIMD2db_shiftLeftConst(dsSIMD2db_add(pow2, fltExpOffset), DS_DBL_MANTISSA_BIT_COUNT));

	// Choose which to use.
	dsSIMD2db chosenVals = dsSIMD2db_select(subnormal, subnormVals, overflowVals);
	chosenVals = dsSIMD2db_select(dsSIMD2db_or(subnormal, overflow), chosenVals, adjustedXi);
	return dsSIMD2db_toDoubleBitfield(dsSIMD2db_select(keepOrig, xi, chosenVals));
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT)

DS_MATH_EXPORT inline dsSIMD4d dsSplitPow2SIMD4d(dsSIMD4db* outPow2, dsSIMD4d x)
{
	DS_ASSERT(outPow2);

	dsSIMD4db zero = dsSIMD4db_set1(0);
	dsSIMD4db fltExpBits = dsSIMD4db_set1(DS_DBL_EXP_BITS);
	dsSIMD4db fltMantissaBits = dsSIMD4db_set1(DS_DBL_MANTISSA_BITS);
	dsSIMD4db targetExpOffset = dsSIMD4db_set1(DS_DBL_EXP_OFFSET - 1);

	dsSIMD4db xi = dsSIMD4db_fromDoubleBitfield(x);
	dsSIMD4db expBits = dsSIMD4db_and(xi, fltExpBits);
	dsSIMD4db mantissaBits = dsSIMD4db_and(xi, fltMantissaBits);
	dsSIMD4db startPow2;
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD4db subnormal = dsSIMD4db_andnot(
		dsSIMD4db_cmpeq(mantissaBits, zero), dsSIMD4db_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD4db_any(subnormal) != 0, false))
	{
		DS_ALIGN(32) uint64_t subnormPow2[4] = {}, scalarXi[4], scalarSubnormal[4];
		dsSIMD4db_store(scalarXi, xi);
		dsSIMD4db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
			{
				unsigned int pow2;
				scalarXi[i] = dsSubnormToNormBitsd(&pow2, scalarXi[i]);
				subnormPow2[i] = pow2;
			}
		}
		xi = dsSIMD4db_load(scalarXi);
		startPow2 = dsSIMD4db_neg(dsSIMD4db_load(subnormPow2));
		expBits = dsSIMD4db_and(xi, fltExpBits);
	}
	else
		startPow2 = dsSIMD4db_set1(0);

	// Prefer == compares for SIMD operations for performance.
	dsSIMD4db keepOrig = dsSIMD4db_or(dsSIMD4db_cmpeq(expBits, zero),
		dsSIMD4db_cmpeq(expBits, fltExpBits));
	dsSIMD4db adjustedPow2 = dsSIMD4db_add(startPow2, dsSIMD4db_sub(
		dsSIMD4db_shiftRightConst(expBits, DS_DBL_MANTISSA_BIT_COUNT), targetExpOffset));
	dsSIMD4db adjustedXi = dsSIMD4db_or(dsSIMD4db_andnot(fltExpBits, xi),
		dsSIMD4db_shiftLeftConst(targetExpOffset, DS_DBL_MANTISSA_BIT_COUNT));
	*outPow2 = dsSIMD4db_select(keepOrig, startPow2, adjustedPow2);
	return dsSIMD4db_toDoubleBitfield(dsSIMD4db_select(keepOrig, xi, adjustedXi));
}

DS_MATH_EXPORT inline dsSIMD4d dsMulPow2SIMD4d(dsSIMD4d x, dsSIMD4db pow2)
{
	dsSIMD4db zero = dsSIMD4db_set1(0);
	dsSIMD4db fltSignBit = dsSIMD4db_set1(DS_DBL_SIGN_BIT);
	dsSIMD4db fltExpBits = dsSIMD4db_set1(DS_DBL_EXP_BITS);
	dsSIMD4db fltMantissaBits = dsSIMD4db_set1(DS_DBL_MANTISSA_BITS);
	dsSIMD4db fltExpOffset = dsSIMD4db_set1(DS_DBL_EXP_OFFSET);
	dsSIMD4db negFltExpOffset = dsSIMD4db_set1(-DS_DBL_EXP_OFFSET);

	dsSIMD4db xi = dsSIMD4db_fromDoubleBitfield(x);
	dsSIMD4db expBits = dsSIMD4db_and(xi, fltExpBits);
	dsSIMD4db mantissaBits = dsSIMD4db_and(xi, fltMantissaBits);
	// Check for subnormal. Using only == with andnot should be better on x86 and same on ARM
	// compared with having and with != for mantissa check.
	dsSIMD4db subnormal = dsSIMD4db_andnot(
		dsSIMD4db_cmpeq(mantissaBits, zero), dsSIMD4db_cmpeq(expBits, zero));
	if (DS_EXPECT(dsSIMD4db_any(subnormal) != 0, false))
	{
		DS_ALIGN(32) uint64_t subnormPow2[4] = {}, scalarXi[4], scalarSubnormal[4];
		dsSIMD4db_store(scalarXi, xi);
		dsSIMD4db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
			{
				unsigned int pow2;
				scalarXi[i] = dsSubnormToNormBitsd(&pow2, scalarXi[i]);
				subnormPow2[i] = pow2;
			}
		}
		xi = dsSIMD4db_load(scalarXi);
		pow2 = dsSIMD4db_sub(pow2, dsSIMD4db_load(subnormPow2));
		expBits = dsSIMD4db_and(xi, fltExpBits);
	}

	// Prefer == compares for SIMD operations for performance.
	dsSIMD4db keepOrig = dsSIMD4db_or(dsSIMD4db_cmpeq(expBits, zero),
		dsSIMD4db_cmpeq(expBits, fltExpBits));
	pow2 = dsSIMD4db_add(pow2, dsSIMD4db_sub(
		dsSIMD4db_shiftRightConst(expBits, DS_DBL_MANTISSA_BIT_COUNT), fltExpOffset));

	// Need to adjust any subnormal values with scalar operations.
	subnormal = dsSIMD4db_andnot(keepOrig, dsSIMD4db_cmples(pow2, negFltExpOffset));
	dsSIMD4db subnormVals = xi;
	if (DS_EXPECT(dsSIMD4db_any(subnormal) != 0, false))
	{
		DS_ALIGN(32) uint64_t scalarXi[4], scalarPow2[4], scalarSubnormal[4];
		dsSIMD4db_store(scalarXi, xi);
		dsSIMD4db_store(scalarPow2, pow2);
		dsSIMD4db_store(scalarSubnormal, subnormal);
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (scalarSubnormal[i])
				scalarXi[i] = dsNormToSubnormBitsd(scalarXi[i], (int)scalarPow2[i]);
		}
		subnormVals = dsSIMD4db_load(scalarXi);
	}

	// Set any overflow values to +-infinity.
	dsSIMD4db overflow = dsSIMD4db_cmpgts(pow2, fltExpOffset);
	dsSIMD4db overflowVals = dsSIMD4db_or(dsSIMD4db_and(xi, fltSignBit), fltExpBits);

	// Expected value in the standard case.
	dsSIMD4db adjustedXi = dsSIMD4db_or(dsSIMD4db_andnot(fltExpBits, xi),
		dsSIMD4db_shiftLeftConst(dsSIMD4db_add(pow2, fltExpOffset), DS_DBL_MANTISSA_BIT_COUNT));

	// Choose which to use.
	dsSIMD4db chosenVals = dsSIMD4db_select(subnormal, subnormVals, overflowVals);
	chosenVals = dsSIMD4db_select(dsSIMD4db_or(subnormal, overflow), chosenVals, adjustedXi);
	return dsSIMD4db_toDoubleBitfield(dsSIMD4db_select(keepOrig, xi, chosenVals));
}

DS_SIMD_END()

#endif // DS_HAS_SIMD

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
