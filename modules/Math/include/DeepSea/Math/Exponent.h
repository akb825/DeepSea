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
#include <DeepSea/Math/Round.h>

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
 *
 * @remark In some cases the custom implementation, typically based on a Taylor series, is slower
 * than the typical standard library implementation, where a lookup table may speed up the
 * operation. In these cases, the standard library function may be used for the scalar function
 * when deterministic math isn't enabled.
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

/**
 * @brief Raises e to an exponent.
 * @param x The exponent to raise e by.
 * @return The result of e^x.
 */
DS_MATH_EXPORT inline float dsExpf(float x);

/** @copydoc dsExpf() */
DS_MATH_EXPORT inline double dsExpd(double x);

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
 * @brief Raises e to an exponent for four values.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, and
 *     dsSIMDFeatures_Rounding are available.
 * @param x The exponents to raise e by.
 * @return The results of e^x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsExpSIMD4f(dsSIMD4f x);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Raises e to an exponent for four values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Float4, dsSIMDFeatures_Int, dsSIMDFeatures_Rounding,
 *     and dsSIMDFeatures_FMA are available.
 * @param x The exponents to raise e by.
 * @return The results of e^x.
 */
DS_MATH_EXPORT inline dsSIMD4f dsExpFMA4f(dsSIMD4f x);

#endif // !DS_DETERMINISTIC_MATH

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
 * @brief Raises e to an exponent for two values.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int, and
 *     dsSIMDFeatures_Rounding are available.
 * @param x The exponents to raise e by.
 * @return The results of e^x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsExpSIMD2d(dsSIMD2d x);

#if !DS_DETERMINISTIC_MATH

/**
 * @brief Raises e to an exponent for two values with fused multiply-add operations.
 * @remark This can be used when dsSIMDFeatures_Double2, dsSIMDFeatures_Int,
 *     dsSIMDFeatures_Rounding, and dsSIMDFeatures_FMA are available.
 * @param x The exponents to raise e by.
 * @return The results of e^x.
 */
DS_MATH_EXPORT inline dsSIMD2d dsExpFMA2d(dsSIMD2d x);

#endif // !DS_DETERMINISTIC_MATH

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

/**
 * @brief Raises e to an exponent for four values.
 * @remark This can be used when dsSIMDFeatures_Double4, dsSIMDFeatures_Int, and
 *     dsSIMDFeatures_Rounding are available, and will use FMA if not disabled through enabling
 *     determinisitic math..
 * @param x The exponents to raise e by.
 * @return The results of e^x.
 */
DS_MATH_EXPORT inline dsSIMD4d dsExpSIMD4d(dsSIMD4d x);

#endif // DS_HAS_SIMD

/// @cond
// Set to 1 to test performance in all configurations.
#define DS_ALWAYS_CUSTOM_EXPONENT_IMPL DS_DETERMINISTIC_MATH

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

#define DS_LN_2_1f 0.693359375f
#define DS_LN_2_2f -2.12194440e-4f

#define DS_LN_2_1d 6.93145751953125e-1
#define DS_LN_2_2d 1.42860682030941723212e-6

#define DS_EXP_TAYLOR_1f 1.9875691500e-4f
#define DS_EXP_TAYLOR_2f 1.3981999507e-3f
#define DS_EXP_TAYLOR_3f 8.3334519073e-3f
#define DS_EXP_TAYLOR_4f 4.1665795894e-2f
#define DS_EXP_TAYLOR_5f 1.6666665459e-1f
#define DS_EXP_TAYLOR_6f 5.0000001201e-1f

#define DS_EXP_TAYLOR_P_1d 1.26177193074810590878e-4
#define DS_EXP_TAYLOR_P_2d 3.02994407707441961300e-2
#define DS_EXP_TAYLOR_P_3d 9.99999999999999999910e-1

#define DS_EXP_TAYLOR_Q_1d 3.00198505138664455042e-6
#define DS_EXP_TAYLOR_Q_2d 2.52448340349684104192e-3
#define DS_EXP_TAYLOR_Q_3d 2.27265548208155028766e-1
#define DS_EXP_TAYLOR_Q_4d 2.00000000000000000009

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

DS_ALWAYS_INLINE float dsMulPow2fSimple(float x, int pow2)
{
	uint32_t pow2i = (pow2 + DS_FLT_EXP_OFFSET) << DS_FLT_MANTISSA_BIT_COUNT;
	return *(float*)&pow2i*x;
}

DS_ALWAYS_INLINE double dsMulPow2dSimple(double x, int pow2)
{
	uint64_t pow2i = (uint64_t)(pow2 + DS_DBL_EXP_OFFSET) << DS_DBL_MANTISSA_BIT_COUNT;
	return *(double*)&pow2i*x;
}

DS_ALWAYS_INLINE float dsMulPow2fComplete(float x, int pow2)
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

DS_ALWAYS_INLINE double dsMulPow2dComplete(double x, int pow2)
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
	if (DS_EXPECT(pow2 > -DS_FLT_EXP_OFFSET && pow2 <= DS_FLT_EXP_OFFSET, true))
		return dsMulPow2fSimple(x, pow2);
	return dsMulPow2fComplete(x, pow2);
}

DS_MATH_EXPORT inline double dsMulPow2d(double x, int pow2)
{
	if (DS_EXPECT(pow2 > -DS_DBL_EXP_OFFSET && pow2 <= DS_DBL_EXP_OFFSET, true))
		return dsMulPow2dSimple(x, pow2);
	return dsMulPow2dComplete(x, pow2);
}

DS_MATH_EXPORT inline float dsExpf(float x)
{
#if DS_ALWAYS_CUSTOM_EXPONENT_IMPL
	uint32_t xi = *(uint32_t*)&x;
	if (DS_EXPECT((xi & DS_FLT_EXP_BITS) == DS_FLT_EXP_BITS, false))
	{
		if (x == -HUGE_VALF)
			return 0;
		return x;
	}

	// Transform e^x to e^g*2^n.
	int n;
	float nf;
	dsMathImplFastRoundif(&n, &nf, M_LOG2Ef*x);
	float g = x - nf*DS_LN_2_1f - nf*DS_LN_2_2f;
	float g2 = dsPow2(g);

	float egTaylor = (((((DS_EXP_TAYLOR_1f*g + DS_EXP_TAYLOR_2f)*g + DS_EXP_TAYLOR_3f)*g +
		DS_EXP_TAYLOR_4f)*g + DS_EXP_TAYLOR_5f)*g + DS_EXP_TAYLOR_6f)*g2 + g + 1.0f;
	return dsMulPow2f(egTaylor, n);
#else
	return expf(x);
#endif
}

DS_MATH_EXPORT inline double dsExpd(double x)
{
#if DS_ALWAYS_CUSTOM_EXPONENT_IMPL
	uint64_t xi = *(uint64_t*)&x;
	if (DS_EXPECT((xi & DS_DBL_EXP_BITS) == DS_DBL_EXP_BITS, false))
	{
		if (x == -HUGE_VAL)
			return 0;
		return x;
	}

	// Transform e^x to e^g*2^n.
	int n;
	double nd;
	dsMathImplFastRoundid(&n, &nd, M_LOG2E*x);
	double g = x - nd*DS_LN_2_1d - nd*DS_LN_2_2d;
	double g2 = dsPow2(g);

	double pTaylor = ((DS_EXP_TAYLOR_P_1d*g2 + DS_EXP_TAYLOR_P_2d)*g2 + DS_EXP_TAYLOR_P_3d)*g;
	double qTaylor = ((DS_EXP_TAYLOR_Q_1d*g2 + DS_EXP_TAYLOR_Q_2d)*g2 + DS_EXP_TAYLOR_Q_3d)*g2 +
		DS_EXP_TAYLOR_Q_4d;
	double egRational = (pTaylor/(qTaylor - pTaylor))*2.0 + 1.0;
	return dsMulPow2d(egRational, (int)n);
#else
	return exp(x);
#endif
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
	dsSIMD4fb fltExpOffset = dsSIMD4fb_set1(DS_FLT_EXP_OFFSET);
	dsSIMD4fb negFltExpOffset = dsSIMD4fb_set1(-DS_FLT_EXP_OFFSET);

	dsSIMD4fb pow2InRange = dsSIMD4fb_and(
		dsSIMD4fb_cmpgts(pow2, negFltExpOffset), dsSIMD4fb_cmples(pow2, fltExpOffset));
	if (DS_EXPECT(dsSIMD4fb_all(pow2InRange) != 0, true))
	{
		dsSIMD4f pow2f = dsSIMD4fb_toFloatBitfield(dsSIMD4fb_shiftLeftConst(
			dsSIMD4fb_add(pow2, fltExpOffset), DS_FLT_MANTISSA_BIT_COUNT));
		return dsSIMD4f_mul(x, pow2f);
	}

	// Fall back to scalar implementation if the simple case couldn't be taken.
	DS_ALIGN(16) float scalarX[4];
	DS_ALIGN(16) int32_t scalarPow2[4];
	dsSIMD4f_store(scalarX, x);
	dsSIMD4fb_store(scalarPow2, pow2);
	for (unsigned int i = 0; i < 4; ++i)
		scalarX[i] = dsMulPow2fComplete(scalarX[i], scalarPow2[i]);
	return dsSIMD4f_load(scalarX);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT,DS_SIMD_ROUNDING)

DS_MATH_EXPORT inline dsSIMD4f dsExpSIMD4f(dsSIMD4f x)
{
	dsSIMD4fb fltExpBits = dsSIMD4fb_set1(DS_FLT_EXP_BITS);
	dsSIMD4fb fltUnsignedBits = dsSIMD4fb_set1(~DS_FLT_SIGN_BIT);

	dsSIMD4f log2e = dsSIMD4f_set1(M_LOG2Ef);
	dsSIMD4f ln2_1 = dsSIMD4f_set1(DS_LN_2_1f);
	dsSIMD4f ln2_2 = dsSIMD4f_set1(DS_LN_2_2f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f expTaylor1 = dsSIMD4f_set1(DS_EXP_TAYLOR_1f);
	dsSIMD4f expTaylor2 = dsSIMD4f_set1(DS_EXP_TAYLOR_2f);
	dsSIMD4f expTaylor3 = dsSIMD4f_set1(DS_EXP_TAYLOR_3f);
	dsSIMD4f expTaylor4 = dsSIMD4f_set1(DS_EXP_TAYLOR_4f);
	dsSIMD4f expTaylor5 = dsSIMD4f_set1(DS_EXP_TAYLOR_5f);
	dsSIMD4f expTaylor6 = dsSIMD4f_set1(DS_EXP_TAYLOR_6f);

	dsSIMD4fb xBits = dsSIMD4fb_fromFloatBitfield(x);
	dsSIMD4fb unsignedBits = dsSIMD4fb_and(xBits, fltUnsignedBits);
	dsSIMD4fb isInfinity = dsSIMD4fb_cmpeq(unsignedBits, fltExpBits);
	dsSIMD4f infinityResult = dsMathImplMaskSIMD4f(dsSIMD4fb_cmpeq(unsignedBits, xBits),
		dsSIMD4fb_toFloatBitfield(fltExpBits));

	// Transform e^x to e^g*2^n.
	dsSIMD4f nf = dsSIMD4f_round(dsSIMD4f_mul(log2e, x));
	dsSIMD4fb n = dsSIMD4fb_fromFloat(nf);
	dsSIMD4f g = dsSIMD4f_sub(dsSIMD4f_sub(x, dsSIMD4f_mul(nf, ln2_1)), dsSIMD4f_mul(nf, ln2_2));
	dsSIMD4f g2 = dsSIMD4f_mul(g, g);

	dsSIMD4f egTaylor = dsSIMD4f_add(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(dsSIMD4f_add(dsSIMD4f_mul(
		dsSIMD4f_add(dsSIMD4f_mul(expTaylor1, g), expTaylor2), g), expTaylor3), g), expTaylor4), g),
		expTaylor5), g), expTaylor6), g2), g), one);
	return dsSIMD4f_select(isInfinity, infinityResult, dsMulPow2SIMD4f(egTaylor, n));
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_INT,DS_SIMD_ROUNDING,DS_SIMD_FMA)

DS_MATH_EXPORT inline dsSIMD4f dsExpFMA4f(dsSIMD4f x)
{
	dsSIMD4fb fltExpBits = dsSIMD4fb_set1(DS_FLT_EXP_BITS);
	dsSIMD4fb fltUnsignedBits = dsSIMD4fb_set1(~DS_FLT_SIGN_BIT);

	dsSIMD4f log2e = dsSIMD4f_set1(M_LOG2Ef);
	dsSIMD4f ln2_1 = dsSIMD4f_set1(DS_LN_2_1f);
	dsSIMD4f ln2_2 = dsSIMD4f_set1(DS_LN_2_2f);
	dsSIMD4f one = dsSIMD4f_set1(1.0f);

	dsSIMD4f expTaylor1 = dsSIMD4f_set1(DS_EXP_TAYLOR_1f);
	dsSIMD4f expTaylor2 = dsSIMD4f_set1(DS_EXP_TAYLOR_2f);
	dsSIMD4f expTaylor3 = dsSIMD4f_set1(DS_EXP_TAYLOR_3f);
	dsSIMD4f expTaylor4 = dsSIMD4f_set1(DS_EXP_TAYLOR_4f);
	dsSIMD4f expTaylor5 = dsSIMD4f_set1(DS_EXP_TAYLOR_5f);
	dsSIMD4f expTaylor6 = dsSIMD4f_set1(DS_EXP_TAYLOR_6f);

	dsSIMD4fb xBits = dsSIMD4fb_fromFloatBitfield(x);
	dsSIMD4fb unsignedBits = dsSIMD4fb_and(xBits, fltUnsignedBits);
	dsSIMD4fb isInfinity = dsSIMD4fb_cmpeq(unsignedBits, fltExpBits);
	dsSIMD4f infinityResult = dsMathImplMaskSIMD4f(dsSIMD4fb_cmpeq(unsignedBits, xBits),
		dsSIMD4fb_toFloatBitfield(fltExpBits));

	// Transform e^x to e^g*2^n.
	dsSIMD4f n = dsSIMD4f_round(dsSIMD4f_mul(log2e, x));
	dsSIMD4f g = dsSIMD4f_fnmadd(n, ln2_2, dsSIMD4f_fnmadd(n, ln2_1, x));
	dsSIMD4f g2 = dsSIMD4f_mul(g, g);

	dsSIMD4f egTaylor = dsSIMD4f_add(dsSIMD4f_fmadd(dsSIMD4f_fmadd(dsSIMD4f_fmadd(dsSIMD4f_fmadd(
		dsSIMD4f_fmadd( dsSIMD4f_fmadd(expTaylor1, g, expTaylor2), g, expTaylor3), g, expTaylor4),
		g, expTaylor5), g, expTaylor6), g2, g), one);
	return dsSIMD4f_select(
		isInfinity, infinityResult, dsMulPow2SIMD4f(egTaylor, dsSIMD4fb_fromFloat(n)));
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

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
	dsSIMD2db fltExpOffset = dsSIMD2db_set1(DS_DBL_EXP_OFFSET);
	dsSIMD2db negFltExpOffset = dsSIMD2db_set1(-DS_DBL_EXP_OFFSET);

	dsSIMD2db pow2InRange = dsSIMD2db_and(
		dsSIMD2db_cmpgts(pow2, negFltExpOffset), dsSIMD2db_cmples(pow2, fltExpOffset));
	if (DS_EXPECT(dsSIMD2db_all(pow2InRange) != 0, true))
	{
		dsSIMD2d pow2d = dsSIMD2db_toDoubleBitfield(dsSIMD2db_shiftLeftConst(
			dsSIMD2db_add(pow2, fltExpOffset), DS_DBL_MANTISSA_BIT_COUNT));
		return dsSIMD2d_mul(x, pow2d);
	}

	// Fall back to scalar implementation if the simple case couldn't be taken.
	DS_ALIGN(16) double scalarX[2];
	DS_ALIGN(16) int64_t scalarPow2[2];
	dsSIMD2d_store(scalarX, x);
	dsSIMD2db_store(scalarPow2, pow2);
	for (unsigned int i = 0; i < 2; ++i)
		scalarX[i] = dsMulPow2dComplete(scalarX[i], (int)scalarPow2[i]);
	return dsSIMD2d_load(scalarX);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT,DS_SIMD_ROUNDING)

DS_MATH_EXPORT inline dsSIMD2d dsExpSIMD2d(dsSIMD2d x)
{
	dsSIMD2db dblExpBits = dsSIMD2db_set1(DS_DBL_EXP_BITS);
	dsSIMD2db dblUnsignedBits = dsSIMD2db_set1(~DS_DBL_SIGN_BIT);

	dsSIMD2d log2e = dsSIMD2d_set1(M_LOG2E);
	dsSIMD2d ln2_1 = dsSIMD2d_set1(DS_LN_2_1d);
	dsSIMD2d ln2_2 = dsSIMD2d_set1(DS_LN_2_2d);
	dsSIMD2d one = dsSIMD2d_set1(1.0);
	dsSIMD2d two = dsSIMD2d_set1(2.0);

	dsSIMD2d expTaylorP1 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_1d);
	dsSIMD2d expTaylorP2 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_2d);
	dsSIMD2d expTaylorP3 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_3d);

	dsSIMD2d expTaylorQ1 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_1d);
	dsSIMD2d expTaylorQ2 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_2d);
	dsSIMD2d expTaylorQ3 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_3d);
	dsSIMD2d expTaylorQ4 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_4d);

	dsSIMD2db xBits = dsSIMD2db_fromDoubleBitfield(x);
	dsSIMD2db unsignedBits = dsSIMD2db_and(xBits, dblUnsignedBits);
	dsSIMD2db isInfinity = dsSIMD2db_cmpeq(unsignedBits, dblExpBits);
	dsSIMD2d infinityResult = dsMathImplMaskSIMD2d(dsSIMD2db_cmpeq(unsignedBits, xBits),
		dsSIMD2db_toDoubleBitfield(dblExpBits));

	// Transform e^x to e^g*2^n.
	dsSIMD2d nd = dsSIMD2d_round(dsSIMD2d_mul(log2e, x));
	dsSIMD2db n = dsSIMD2db_fromDouble(nd);
	dsSIMD2d g = dsSIMD2d_sub(dsSIMD2d_sub(x, dsSIMD2d_mul(nd, ln2_1)), dsSIMD2d_mul(nd, ln2_2));
	dsSIMD2d g2 = dsSIMD2d_mul(g, g);

	dsSIMD2d pTaylor = dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(
		expTaylorP1, g2), expTaylorP2), g2), expTaylorP3), g);
	dsSIMD2d qTaylor = dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(dsSIMD2d_mul(dsSIMD2d_add(
		dsSIMD2d_mul(expTaylorQ1, g2), expTaylorQ2), g2), expTaylorQ3), g2), expTaylorQ4);
	dsSIMD2d qRational = dsSIMD2d_add(dsSIMD2d_mul(
		dsSIMD2d_div(pTaylor, dsSIMD2d_sub(qTaylor, pTaylor)), two), one);
	return dsSIMD2d_select(isInfinity, infinityResult, dsMulPow2SIMD2d(qRational, n));
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_INT,DS_SIMD_ROUNDING,DS_SIMD_FMA)

DS_MATH_EXPORT inline dsSIMD2d dsExpFMA2d(dsSIMD2d x)
{
	dsSIMD2db dblExpBits = dsSIMD2db_set1(DS_DBL_EXP_BITS);
	dsSIMD2db dblUnsignedBits = dsSIMD2db_set1(~DS_DBL_SIGN_BIT);

	dsSIMD2d log2e = dsSIMD2d_set1(M_LOG2E);
	dsSIMD2d ln2_1 = dsSIMD2d_set1(DS_LN_2_1d);
	dsSIMD2d ln2_2 = dsSIMD2d_set1(DS_LN_2_2d);
	dsSIMD2d one = dsSIMD2d_set1(1.0);
	dsSIMD2d two = dsSIMD2d_set1(2.0);

	dsSIMD2d expTaylorP1 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_1d);
	dsSIMD2d expTaylorP2 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_2d);
	dsSIMD2d expTaylorP3 = dsSIMD2d_set1(DS_EXP_TAYLOR_P_3d);

	dsSIMD2d expTaylorQ1 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_1d);
	dsSIMD2d expTaylorQ2 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_2d);
	dsSIMD2d expTaylorQ3 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_3d);
	dsSIMD2d expTaylorQ4 = dsSIMD2d_set1(DS_EXP_TAYLOR_Q_4d);

	dsSIMD2db xBits = dsSIMD2db_fromDoubleBitfield(x);
	dsSIMD2db unsignedBits = dsSIMD2db_and(xBits, dblUnsignedBits);
	dsSIMD2db isInfinity = dsSIMD2db_cmpeq(unsignedBits, dblExpBits);
	dsSIMD2d infinityResult = dsMathImplMaskSIMD2d(dsSIMD2db_cmpeq(unsignedBits, xBits),
		dsSIMD2db_toDoubleBitfield(dblExpBits));

	// Transform e^x to e^g*2^n.
	dsSIMD2d n = dsSIMD2d_round(dsSIMD2d_mul(log2e, x));
	dsSIMD2d g = dsSIMD2d_fnmadd(n, ln2_2, dsSIMD2d_fnmadd(n, ln2_1, x));
	dsSIMD2d g2 = dsSIMD2d_mul(g, g);

	dsSIMD2d pTaylor = dsSIMD2d_mul(dsSIMD2d_fmadd(dsSIMD2d_fmadd(expTaylorP1, g2, expTaylorP2),
		g2, expTaylorP3), g);
	dsSIMD2d qTaylor = dsSIMD2d_fmadd(dsSIMD2d_fmadd(dsSIMD2d_fmadd(expTaylorQ1, g2, expTaylorQ2),
		g2, expTaylorQ3), g2, expTaylorQ4);
	dsSIMD2d qRational = dsSIMD2d_fmadd(
		dsSIMD2d_div(pTaylor, dsSIMD2d_sub(qTaylor, pTaylor)), two, one);
	return dsSIMD2d_select(
		isInfinity, infinityResult, dsMulPow2SIMD2d(qRational, dsSIMD2db_fromDouble(n)));
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

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
	dsSIMD4db fltExpOffset = dsSIMD4db_set1(DS_DBL_EXP_OFFSET);
	dsSIMD4db negFltExpOffset = dsSIMD4db_set1(-DS_DBL_EXP_OFFSET);

	dsSIMD4db pow2InRange = dsSIMD4db_and(
		dsSIMD4db_cmpgts(pow2, negFltExpOffset), dsSIMD4db_cmples(pow2, fltExpOffset));
	if (DS_EXPECT(dsSIMD4db_all(pow2InRange) != 0, true))
	{
		dsSIMD4d pow4d = dsSIMD4db_toDoubleBitfield(dsSIMD4db_shiftLeftConst(
			dsSIMD4db_add(pow2, fltExpOffset), DS_DBL_MANTISSA_BIT_COUNT));
		return dsSIMD4d_mul(x, pow4d);
	}

	// Fall back to scalar implementation if the simple case couldn't be taken.
	DS_ALIGN(32) double scalarX[4];
	DS_ALIGN(32) int64_t scalarPow2[4];
	dsSIMD4d_store(scalarX, x);
	dsSIMD4db_store(scalarPow2, pow2);
	for (unsigned int i = 0; i < 4; ++i)
		scalarX[i] = dsMulPow2dComplete(scalarX[i], (int)scalarPow2[i]);
	return dsSIMD4d_load(scalarX);
}

DS_SIMD_END()
DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_INT,DS_SIMD_ROUNDING,DS_SIMD_FMA)

DS_MATH_EXPORT inline dsSIMD4d dsExpSIMD4d(dsSIMD4d x)
{
	dsSIMD4db dblExpBits = dsSIMD4db_set1(DS_DBL_EXP_BITS);
	dsSIMD4db dblUnsignedBits = dsSIMD4db_set1(~DS_DBL_SIGN_BIT);

	dsSIMD4d log2e = dsSIMD4d_set1(M_LOG2E);
	dsSIMD4d ln2_1 = dsSIMD4d_set1(DS_LN_2_1d);
	dsSIMD4d ln2_2 = dsSIMD4d_set1(DS_LN_2_2d);
	dsSIMD4d one = dsSIMD4d_set1(1.0);
	dsSIMD4d two = dsSIMD4d_set1(2.0);

	dsSIMD4d expTaylorP1 = dsSIMD4d_set1(DS_EXP_TAYLOR_P_1d);
	dsSIMD4d expTaylorP2 = dsSIMD4d_set1(DS_EXP_TAYLOR_P_2d);
	dsSIMD4d expTaylorP3 = dsSIMD4d_set1(DS_EXP_TAYLOR_P_3d);

	dsSIMD4d expTaylorQ1 = dsSIMD4d_set1(DS_EXP_TAYLOR_Q_1d);
	dsSIMD4d expTaylorQ2 = dsSIMD4d_set1(DS_EXP_TAYLOR_Q_2d);
	dsSIMD4d expTaylorQ3 = dsSIMD4d_set1(DS_EXP_TAYLOR_Q_3d);
	dsSIMD4d expTaylorQ4 = dsSIMD4d_set1(DS_EXP_TAYLOR_Q_4d);

	dsSIMD4db xBits = dsSIMD4db_fromDoubleBitfield(x);
	dsSIMD4db unsignedBits = dsSIMD4db_and(xBits, dblUnsignedBits);
	dsSIMD4db isInfinity = dsSIMD4db_cmpeq(unsignedBits, dblExpBits);
	dsSIMD4d infinityResult = dsMathImplMaskSIMD4d(dsSIMD4db_cmpeq(unsignedBits, xBits),
		dsSIMD4db_toDoubleBitfield(dblExpBits));

	// Transform e^x to e^g*2^n.
	dsSIMD4d nd = dsSIMD4d_round(dsSIMD4d_mul(log2e, x));
	dsSIMD4db n = dsSIMD4db_fromDouble(nd);
#if DS_DETERMINISTIC_MATH
	dsSIMD4d g = dsSIMD4d_sub(dsSIMD4d_sub(x, dsSIMD4d_mul(nd, ln2_1)), dsSIMD4d_mul(nd, ln2_2));
#else
	dsSIMD4d g = dsSIMD4d_fnmadd(nd, ln2_2, dsSIMD4d_fnmadd(nd, ln2_1, x));
#endif
	dsSIMD4d g2 = dsSIMD4d_mul(g, g);

#if DS_DETERMINISTIC_MATH
	dsSIMD4d pTaylor = dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(
		expTaylorP1, g2), expTaylorP2), g2), expTaylorP3), g);
	dsSIMD4d qTaylor = dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(dsSIMD4d_mul(dsSIMD4d_add(
		dsSIMD4d_mul(expTaylorQ1, g2), expTaylorQ2), g2), expTaylorQ3), g2), expTaylorQ4);
	dsSIMD4d qRational = dsSIMD4d_add(dsSIMD4d_mul(
		dsSIMD4d_div(pTaylor, dsSIMD4d_sub(qTaylor, pTaylor)), two), one);
#else
	dsSIMD4d pTaylor = dsSIMD4d_mul(dsSIMD4d_fmadd(dsSIMD4d_fmadd(expTaylorP1, g2, expTaylorP2),
		g2, expTaylorP3), g);
	dsSIMD4d qTaylor = dsSIMD4d_fmadd(dsSIMD4d_fmadd(dsSIMD4d_fmadd(expTaylorQ1, g2, expTaylorQ2),
		g2, expTaylorQ3), g2, expTaylorQ4);
	dsSIMD4d qRational = dsSIMD4d_fmadd(
		dsSIMD4d_div(pTaylor, dsSIMD4d_sub(qTaylor, pTaylor)), two, one);
#endif
	return dsSIMD4d_select(isInfinity, infinityResult, dsMulPow2SIMD4d(qRational, n));
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

#undef DS_LN_2_1f
#undef DS_LN_2_2f

#undef DS_LN_2_1d
#undef DS_LN_2_2d

#undef DS_EXP_TAYLOR_1f
#undef DS_EXP_TAYLOR_2f
#undef DS_EXP_TAYLOR_3f
#undef DS_EXP_TAYLOR_4f
#undef DS_EXP_TAYLOR_5f
#undef DS_EXP_TAYLOR_6f

#undef DS_EXP_TAYLOR_P_1d
#undef DS_EXP_TAYLOR_P_2d
#undef DS_EXP_TAYLOR_P_3d

#undef DS_EXP_TAYLOR_Q_1d
#undef DS_EXP_TAYLOR_Q_2d
#undef DS_EXP_TAYLOR_Q_3d
#undef DS_EXP_TAYLOR_Q_4d

#ifdef __cplusplus
}
#endif
