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

#include <DeepSea/Math/Exponent.h>
#include <DeepSea/Math/Types.h>

#if DS_ALWAYS_CUSTOM_EXPONENT_IMPL

#define DS_LOG2_E_FRACd 0.44269504088896340735992

#define DS_1_16f 0.0625f
#define DS_1_8f 0.125f

#define DS_1_16d 0.0625
#define DS_1_8d 0.125

#define DS_LOG_TAYLOR_P_1d 4.97778295871696322025e-1
#define DS_LOG_TAYLOR_P_2d 3.73336776063286838734
#define DS_LOG_TAYLOR_P_3d 7.69994162726912503298
#define DS_LOG_TAYLOR_P_4d 4.66651806774358464979

#define DS_LOG_TAYLOR_Q_1d 9.33340916416696166113
#define DS_LOG_TAYLOR_Q_2d 2.79999886606328401649e1
#define DS_LOG_TAYLOR_Q_3d 3.35994905342304405431e1
#define DS_LOG_TAYLOR_Q_4d 1.39995542032307539578e1

#define DS_EXP2_TAYLOR_1d 1.49664108433729301083e-5
#define DS_EXP2_TAYLOR_2d 1.54010762792771901396e-4
#define DS_EXP2_TAYLOR_3d 1.33335476964097721140e-3
#define DS_EXP2_TAYLOR_4d 9.61812908476554225149e-3
#define DS_EXP2_TAYLOR_5d 5.55041086645832347466e-2
#define DS_EXP2_TAYLOR_6d 2.40226506959099779976e-1
#define DS_EXP2_TAYLOR_7d 6.93147180559945308821e-1

double dsPowd(double x, double y)
{
	if (DS_EXPECT(y == 0.0, false))
		return 1.0;

	// Will ultimately take the form pow(x, y) = exp2(y*log2(x)).

	// 2^(-i/16), continuation digits for 2^(-i/16), 1/2^(-i/16), for even values of i.
	// Set as vector 2 for optimized loading as a multiple of 16 bytes and usable with SIMD.
	static const dsVector2d exp2NegEven16thsInfos[9] =
	{
		{{1.0, 0.0}},
		{{9.17004043204671215328e-1, 1.64155361212281360176e-17}},
		{{8.40896415253714502036e-1, 4.09950501029074826006e-17}},
		{{7.71105412703970372057e-1, 3.97491740484881042808e-17}},
		{{7.07106781186547572737e-1, -4.83364665672645672553e-17}},
		{{6.48419777325504820276e-1, 1.26912513974441574796e-17}},
		{{5.94603557501360513449e-1, 1.99100761573282305549e-17}},
		{{5.45253866332628844837e-1, -1.52339103990623557348e-17}},
		{{0.5, 0.0}}
	};

	// 2^(-i/16) for odd values of i, excluding the last one.
	static const double exp2NegOdd16ths[8] =
	{
		9.57603280698573700036e-1,
		8.78126080186649726755e-1,
		8.05245165974627141736e-1,
		7.38413072969749673113e-1,
		6.77127773468446325644e-1,
		6.20928906036742001007e-1,
		5.69394317378345782288e-1,
		5.22136891213706877402e-1
	};

	// 2^(-i/16)
	static const double exp2Neg16ths[17] =
	{
		1.0,
		9.57603280698573700036e-1,
		9.17004043204671215328e-1,
		8.78126080186649726755e-1,
		8.40896415253714502036e-1,
		8.05245165974627141736e-1,
		7.71105412703970372057e-1,
		7.38413072969749673113e-1,
		7.07106781186547572737e-1,
		6.77127773468446325644e-1,
		6.48419777325504820276e-1,
		6.20928906036742001007e-1,
		5.94603557501360513449e-1,
		5.69394317378345782288e-1,
		5.45253866332628844837e-1,
		5.22136891213706877402e-1,
		0.5
	};

	int xPow2;
	double xm = dsSplitPow2d(&xPow2, fabs(x));

	// Search for the nearest even value.
	unsigned int foundI = 0;
	for (unsigned int i = 0; i < DS_ARRAY_SIZE(exp2NegOdd16ths); ++i)
		foundI += xm <= exp2NegOdd16ths[i];

	const dsVector2d* foundExp2Neg16Info = exp2NegEven16thsInfos + foundI;
	double exp2Neg16, exp2Neg16Cont;
#if DS_SIMD_ALWAYS_DOUBLE2
	exp2Neg16 = dsSIMD2d_get(foundExp2Neg16Info->simd, 0);
	exp2Neg16Cont = dsSIMD2d_get(foundExp2Neg16Info->simd, 1);
#else
	exp2Neg16 = foundExp2Neg16Info->x;
	exp2Neg16Cont = foundExp2Neg16Info->y;
#endif

	// Taylor series expansion is in the form log(1 + x), so find (xm - 2^(-i/16))/2^(-i/16) to
	// cancel out the 1 to find log(xm/2^(-i/16)).
	double logFactor = (xm - exp2Neg16 - exp2Neg16Cont)/exp2Neg16;
	double logFactor2 = dsPow2(logFactor);

	double logXmP = (((DS_LOG_TAYLOR_P_1d*logFactor + DS_LOG_TAYLOR_P_2d)*logFactor +
		DS_LOG_TAYLOR_P_3d)*logFactor + DS_LOG_TAYLOR_P_4d)*logFactor2;
	double logXmQ = (((logFactor + DS_LOG_TAYLOR_Q_1d)*logFactor + DS_LOG_TAYLOR_Q_2d)*logFactor +
		DS_LOG_TAYLOR_Q_3d)*logFactor + DS_LOG_TAYLOR_Q_4d;
	double logXm = logFactor*(logXmP/logXmQ) - 0.5*logFactor2;
	logXm += DS_LOG2_E_FRACd*logXm + DS_LOG2_E_FRACd*logFactor + logFactor;

	// foundI is the index of the even 16th, so multiply by 1/8.
	double logXExp = xPow2 - foundI*DS_1_8d;

	// Multiply log2(x) and y with increased precision. The final "big" value will be 16 times the
	// value for the following integer math.
	double yBig = DS_1_16d*dsFloord(16.0*y);
	double ySmall = y - yBig;

	double yLog2X1 = logXm*y + logXExp*ySmall;
	double yLog2X1Big = DS_1_16d*dsFloord(16.0*yLog2X1);
	double yLog2X1Small = yLog2X1 - yLog2X1Big;

	double yLog2X2 = yLog2X1Big + logXExp*yBig;
	double yLog2X2Big16 = dsFloord(16.0*yLog2X2);
	double yLog2X2Big = DS_1_16d*yLog2X2Big16;
	double yLog2X2Small = yLog2X2 - yLog2X2Big;

	double yLog2X3 = yLog2X1Small + yLog2X2Small;
	double yLog2X3Big16 = dsFloord(16.0*yLog2X3);
	double yLog2X3Big = DS_1_16d*yLog2X3Big16;

	int yLog2XBig16 = (int)(yLog2X2Big16 + yLog2X3Big16);
	double yLog2XSmall = yLog2X3 - yLog2X3Big;

	// Make sure that yLog2XSmall is in the range [-1/16, 0].
	uint64_t yLog2XSmallPosMask = (uint64_t)(yLog2XSmall <= 0.0) - 1;
	yLog2XBig16 += yLog2XSmallPosMask & 1;
	yLog2XSmall -= dsMathImplMaskd(yLog2XSmallPosMask, DS_1_16d);

	// 2^yLog2XSmall - 1
	double powSmall = ((((((DS_EXP2_TAYLOR_1d*yLog2XSmall + DS_EXP2_TAYLOR_2d)*yLog2XSmall +
		DS_EXP2_TAYLOR_3d)*yLog2XSmall + DS_EXP2_TAYLOR_4d)*yLog2XSmall +
		DS_EXP2_TAYLOR_5d)*yLog2XSmall + DS_EXP2_TAYLOR_6d)*yLog2XSmall +
		DS_EXP2_TAYLOR_7d)*yLog2XSmall;

	// 2^yLog2XBig, based on the 1/16 component separated from the integer component.
	uint32_t yLog2XBig16PosMask = (uint32_t)(yLog2XBig16 < 0) - 1;
	int pow2Neg = -(-yLog2XBig16 >> 4);
	int pow2Pos = (yLog2XBig16 >> 4) + 1;
	int pow2 = (pow2Pos & yLog2XBig16PosMask) | (pow2Neg & ~yLog2XBig16PosMask);
	double powBig = exp2Neg16ths[(pow2 << 4) - yLog2XBig16];

	// powSmall*powBig, canceling out the -1 component for powSmall.
	double powResult = dsMulPow2d(powBig + powBig*powSmall, pow2);

	// Keep sign for odd exponents. Only actually valid if an integer exponent and x is negative,
	// but not interested in error checking within here for performance.
	uint64_t oddExponentMask = ~(((int)y & 1) - 1);
	return dsMathImplConditionalNegated(powResult, dsMathImplExtractSignBitd(x) & oddExponentMask);
}
#endif // DS_ALWAYS_CUSTOM_EXPONENT_IMPL

float dsSplitPow2f(int* outPow2, float x);
double dsSplitPow2d(int* outPow2, double x);
float dsMulPow2f(float x, int pow2);
double dsMulPow2d(double x, int pow2);
float dsLnf(float x);
double dsLnd(double x);
float dsLog2f(float x);
double dsLog2d(double x);
float dsLog10f(float x);
double dsLog10d(double x);
float dsExpf(float x);
double dsExpd(double x);
float dsExp2f(float x);
double dsExp2d(double x);
float dsExp10f(float x);
double dsExp10d(double x);
float dsPowf(float x, float y);

#if DS_HAS_SIMD

dsSIMD4f dsSplitPow2SIMD4f(dsSIMD4fb* outPow2, dsSIMD4f x);
dsSIMD4f dsMulPow2SIMD4f(dsSIMD4f x, dsSIMD4fb pow2);
dsSIMD4f dsLnSIMD4f(dsSIMD4f x);
dsSIMD4f dsLog2SIMD4f(dsSIMD4f x);
dsSIMD4f dsLog10SIMD4f(dsSIMD4f x);
dsSIMD4f dsExpSIMD4f(dsSIMD4f x);
dsSIMD4f dsExp2SIMD4f(dsSIMD4f x);
dsSIMD4f dsExp10SIMD4f(dsSIMD4f x);

#if !DS_DETERMINISTIC_MATH
dsSIMD4f dsLnFMA4f(dsSIMD4f x);
dsSIMD4f dsLog2FMA4f(dsSIMD4f x);
dsSIMD4f dsLog10FMA4f(dsSIMD4f x);
dsSIMD4f dsExpFMA4f(dsSIMD4f x);
dsSIMD4f dsExp2FMA4f(dsSIMD4f x);
dsSIMD4f dsExp10FMA4f(dsSIMD4f x);
#endif // DS_DETERMINISTIC_MATH

dsSIMD2d dsSplitPow2SIMD2d(dsSIMD2db* outPow2, dsSIMD2d x);
dsSIMD2d dsMulPow2SIMD2d(dsSIMD2d x, dsSIMD2db pow2);
dsSIMD2d dsLnSIMD2d(dsSIMD2d x);
dsSIMD2d dsLog2SIMD2d(dsSIMD2d x);
dsSIMD2d dsLog10SIMD2d(dsSIMD2d x);
dsSIMD2d dsExpSIMD2d(dsSIMD2d x);
dsSIMD2d dsExp2SIMD2d(dsSIMD2d x);
dsSIMD2d dsExp10SIMD2d(dsSIMD2d x);

#if !DS_DETERMINISTIC_MATH
dsSIMD2d dsLnFMA2d(dsSIMD2d x);
dsSIMD2d dsLog2FMA2d(dsSIMD2d x);
dsSIMD2d dsLog10FMA2d(dsSIMD2d x);
dsSIMD2d dsExpFMA2d(dsSIMD2d x);
dsSIMD2d dsExp2FMA2d(dsSIMD2d x);
dsSIMD2d dsExp10FMA2d(dsSIMD2d x);
#endif // DS_DETERMINISTIC_MATH

dsSIMD4d dsSplitPow2SIMD4d(dsSIMD4db* outPow2, dsSIMD4d x);
dsSIMD4d dsMulPow2SIMD4d(dsSIMD4d x, dsSIMD4db pow2);
dsSIMD4d dsLnSIMD4d(dsSIMD4d x);
dsSIMD4d dsLog2SIMD4d(dsSIMD4d x);
dsSIMD4d dsLog10SIMD4d(dsSIMD4d x);
dsSIMD4d dsExpSIMD4d(dsSIMD4d x);
dsSIMD4d dsExp2SIMD4d(dsSIMD4d x);
dsSIMD4d dsExp10SIMD4d(dsSIMD4d x);

#endif // DS_HAS_SIMD
