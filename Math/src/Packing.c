/*
 * Copyright 2016 Aaron Barany
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
 *
 * Half float implementation borrowed from GLM:
 * OpenGL Mathematics (glm.g-truc.net)
 *
 * Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
 *
 * This half implementation is based on OpenEXR which is Copyright (c) 2002,
 * Industrial Light & Magic, a division of Lucas Digital Ltd. LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Restrictions:
 *		By making use of the Software for military purposes, you choose to make
 *		a Bunny unhappy.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <DeepSea/Math/Packing.h>

#if DS_MSC
#pragma warning(push)
#pragma warning(disable: 4756)
#endif

typedef union FloatInt
{
	float f;
	uint32_t i;
} FloatInt;

static float overflow()
{
	float f = 1e10f;

	for(int i = 0; i < 10; ++i)
		f *= f; // this will overflow before the for loop terminates
	return f;
}

dsHalfFloat dsPackHalfFloat(float x)
{
	dsHalfFloat halfFloat;
	FloatInt entry;
	entry.f = x;
	int i = (int)entry.i;

	//
	// Our floating point number, f, is represented by the bit
	// pattern in integer i.  Disassemble that bit pattern into
	// the sign, s, the exponent, e, and the significand, m.
	// Shift s into the position where it will go in in the
	// resulting half number.
	// Adjust e, accounting for the different exponent bias
	// of float and half (127 versus 15).
	//

	int s =  (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m =   i        & 0x007fffff;

	//
	// Now reassemble s, e and m into a half:
	//

	if(e <= 0)
	{
		if(e < -10)
		{
			//
			// E is less than -10.  The absolute value of f is
			// less than half_MIN (f may be a small normalized
			// float, a denormalized float or a zero).
			//
			// We convert f to a half zero.
			//

			halfFloat.data = (uint16_t)s;
			return halfFloat;
		}

		//
		// E is between -10 and 0.  F is a normalized float,
		// whose magnitude is less than __half_NRM_MIN.
		//
		// We convert f to a denormalized half.
		//

		m = (m | 0x00800000) >> (1 - e);

		//
		// Round to nearest, round "0.5" up.
		//
		// Rounding may cause the significand to overflow and make
		// our number normalized.  Because of the way a half's bits
		// are laid out, we don't have to treat this case separately;
		// the code below will handle it correctly.
		//

		if(m & 0x00001000)
			m += 0x00002000;

		//
		// Assemble the half from s, e (zero) and m.
		//

		halfFloat.data = (uint16_t)(s | (m >> 13));
		return halfFloat;
	}
	else if(e == 0xff - (127 - 15))
	{
		if(m == 0)
		{
			//
			// F is an infinity; convert f to a half
			// infinity with the same sign as f.
			//

			halfFloat.data = (uint16_t)(s | 0x7c00);
			return halfFloat;
		}
		else
		{
			//
			// F is a NAN; we produce a half NAN that preserves
			// the sign bit and the 10 leftmost bits of the
			// significand of f, with one exception: If the 10
			// leftmost bits are all zero, the NAN would turn
			// into an infinity, so we have to set at least one
			// bit in the significand.
			//

			m >>= 13;

			halfFloat.data = (uint16_t)(s | 0x7c00 | m | (m == 0));
			return halfFloat;
		}
	}
	else
	{
		//
		// E is greater than zero.  F is a normalized float.
		// We try to convert f to a normalized half.
		//

		//
		// Round to nearest, round "0.5" up
		//

		if(m &  0x00001000)
		{
			m += 0x00002000;

			if(m & 0x00800000)
			{
				m =  0;     // overflow in significand,
				e += 1;     // adjust exponent
			}
		}

		//
		// Handle exponent overflow
		//

		if (e > 30)
		{
			overflow();        // Cause a hardware floating point overflow;

			halfFloat.data = (uint16_t)(s | 0x7c00);
			return halfFloat;
			// if this returns, the half becomes an
		}   // infinity with the same sign as f.

		//
		// Assemble the half from s, e and m.
		//

		halfFloat.data = (uint16_t)(s | (e << 10) | (m >> 13));
		return halfFloat;
	}
}

float dsUnpackHalfFloat(dsHalfFloat x)
{
	int s = (x.data >> 15) & 0x00000001;
	int e = (x.data >> 10) & 0x0000001f;
	int m =  x.data        & 0x000003ff;

	if(e == 0)
	{
		if(m == 0)
		{
			//
			// Plus or minus zero
			//

			FloatInt result;
			result.i = (uint32_t)(s << 31);
			return result.f;
		}
		else
		{
			//
			// Denormalized number -- renormalize it
			//

			while(!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if(e == 31)
	{
		if(m == 0)
		{
			//
			// Positive or negative infinity
			//

			FloatInt result;
			result.i = (uint32_t)((s << 31) | 0x7f800000);
			return result.f;
		}
		else
		{
			//
			// Nan -- preserve sign and significand bits
			//

			FloatInt result;
			result.i = (uint32_t)((s << 31) | 0x7f800000 | (m << 13));
			return result.f;
		}
	}

	//
	// Normalized number
	//

	e = e + (127 - 15);
	m = m << 13;

	//
	// Assemble s, e and m.
	//

	FloatInt Result;
	Result.i = (uint32_t)((s << 31) | (e << 23) | m);
	return Result.f;
}

int32_t dsPackInt32(float x);
float dsUnpackInt32(int32_t x);
uint32_t dsPackUInt32(float x);
float dsUnpackUInt32(uint32_t x);
int16_t dsPackInt16(float x);
float dsUnpackInt16(int16_t x);
uint16_t dsPackUInt16(float x);
float dsUnpackUInt16(uint16_t x);
int8_t dsPackInt8(float x);
float dsUnpackInt8(int8_t x);
uint8_t dsPackUInt8(float x);
float dsUnpackUInt8(uint8_t x);
uint8_t dsPackIntX4Y4(const dsVector2f* xy);
void dsUnpackIntX4Y4(dsVector2f* result, uint8_t value);
uint8_t dsPackUIntX4Y4(const dsVector2f* xy);
void dsUnpackUIntX4Y4(dsVector2f* result, uint8_t value);
uint8_t dsPackIntY4X4(const dsVector2f* xy);
void dsUnpackIntY4X4(dsVector2f* result, uint8_t value);
uint8_t dsPackUIntY4X4(const dsVector2f* xy);
void dsUnpackUIntY4X4(dsVector2f* result, uint8_t value);
uint16_t dsPackIntX4Y4Z4W4(const dsVector4f* xyzw);
void dsUnpackIntX4Y4Z4W4(dsVector4f* result, uint16_t value);
uint16_t dsPackUIntX4Y4Z4W4(const dsVector4f* xyzw);
void dsUnpackUIntX4Y4Z4W4(dsVector4f* result, uint16_t value);
uint16_t dsPackIntZ4Y4X4W4(const dsVector4f* wxyz);
void dsUnpackIntZ4Y4X4W4(dsVector4f* result, uint16_t value);
uint16_t dsPackUIntZ4Y4X4W4(const dsVector4f* wxyz);
void dsUnpackUIntZ4Y4X4W4(dsVector4f* result, uint16_t value);
uint16_t dsPackIntX5Y6Z5(const dsVector3f* xyz);
void dsUnpackIntX5Y6Z5(dsVector3f* result, uint16_t value);
uint16_t dsPackUIntX5Y6Z5(const dsVector3f* xyz);
void dsUnpackUIntX5Y6Z5(dsVector3f* result, uint16_t value);
uint16_t dsPackIntZ5Y6X5(const dsVector3f* zyx);
void dsUnpackIntZ5Y6X5(dsVector3f* result, uint16_t value);
uint16_t dsPackUIntZ5Y6X5(const dsVector3f* zyx);
void dsUnpackUIntZ5Y6X5(dsVector3f* result, uint16_t value);
uint16_t dsPackIntX5Y5Z5W1(const dsVector4f* xyzw);
void dsUnpackIntX5Y5Z5W1(dsVector4f* result, uint16_t value);
uint16_t dsPackUIntX5Y5Z5W1(const dsVector4f* xyzw);
void dsUnpackUIntX5Y5Z5W1(dsVector4f* result, uint16_t value);
uint16_t dsPackIntZ5Y5X5W1(const dsVector4f* zyxw);
void dsUnpackIntZ5Y5X5W1(dsVector4f* result, uint16_t value);
uint16_t dsPackUIntZ5Y5X5W1(const dsVector4f* zyxw);
void dsUnpackUIntZ5Y5X5W1(dsVector4f* result, uint16_t value);
uint16_t dsPackIntW1X5Y5Z5(const dsVector4f* wxyz);
void dsUnpackIntW1X5Y5Z5(dsVector4f* result, uint16_t value);
uint16_t dsPackUIntW1X5Y5Z5(const dsVector4f* wxyz);
void dsUnpackUIntW1X5Y5Z5(dsVector4f* result, uint16_t value);
uint32_t dsPackIntW2X10Y10Z10(const dsVector4f* wxyz);
void dsUnpackIntW2X10Y10Z10(dsVector4f* result, uint32_t value);
uint32_t dsPackUIntW2X10Y10Z10(const dsVector4f* wxyz);
void dsUnpackUIntW2X10Y10Z10(dsVector4f* result, uint32_t value);
uint32_t dsPackIntW2Z10Y10X10(const dsVector4f* wzyx);
void dsUnpackIntW2Z10Y10X10(dsVector4f* result, uint32_t value);
uint32_t dsPackUIntW2Z10Y10X10(const dsVector4f* wzyx);
void dsUnpackUIntW2Z10Y10X10(dsVector4f* result, uint32_t value);

#if DS_MSC
#pragma warning(pop)
#endif
