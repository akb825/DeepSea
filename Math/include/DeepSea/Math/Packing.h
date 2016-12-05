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
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for packing float values into integers and half floats.
 */

/**
 * @brief Packs a float into a half float.
 * @param x The value to pack.
 * @return The packed value.
 */
DS_MATH_EXPORT dsHalfFloat dsPackHalfFloat(float x);

/**
 * @brief Unpcks a half float into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
DS_MATH_EXPORT float dsUnpackHalfFloat(dsHalfFloat x);

/**
 * @brief Packs a float into a 32-bit integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline int32_t dsPackInt32(float x);

/**
 * @brief Unpcks a 32-bit integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackInt32(int32_t x);

/**
 * @brief Packs a float into a 32-bit unsigned integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline uint32_t dsPackUInt32(float x);

/**
 * @brief Unpcks a 32-bit unsigned integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackUInt32(uint32_t x);

/**
 * @brief Packs a float into a 16-bit integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline int16_t dsPackInt16(float x);

/**
 * @brief Unpcks a 16-bit integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackInt16(int16_t x);

/**
 * @brief Packs a float into a 16-bit unsigned integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline uint16_t dsPackUInt16(float x);

/**
 * @brief Unpcks a 16-bit unsigned integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackUInt16(uint16_t x);

/**
 * @brief Packs a float into a 8-bit integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline int8_t dsPackInt8(float x);

/**
 * @brief Unpcks a 8-bit integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackInt8(int8_t x);

/**
 * @brief Packs a float into a 8-bit unsigned integer.
 * @param x The value to pack.
 * @return The packed value.
 */
inline uint8_t dsPackUInt8(float x);

/**
 * @brief Unpcks a 8-bit unsigned integer into a float.
 * @param x The value to unpack.
 * @return The unpacked value.
 */
inline float dsUnpackUInt8(uint8_t x);

/**
 * @brief Packs two floats into a 8-bit integer in XY order, 4 bits each.
 * @param xy The values to pack.
 * @return The packed integer.
 */
inline uint8_t dsPackIntX4Y4(const dsVector2f* xy);

/**
 * @brief Unpacks two floats from a 8-bit integer in XY order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntX4Y4(dsVector2f* result, uint8_t value);

/**
 * @brief Packs two floats into a 8-bit unsigned integer in XY order, 4 bits each.
 * @param xy The values to pack.
 * @return The packed integer.
 */
inline uint8_t dsPackUIntX4Y4(const dsVector2f* xy);

/**
 * @brief Unpacks two floats from a 8-bit unsigned integer in XY order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntX4Y4(dsVector2f* result, uint8_t value);

/**
 * @brief Packs two floats into a 8-bit integer in YX order, 4 bits each.
 * @param xy The values to pack.
 * @return The packed integer.
 */
inline uint8_t dsPackIntY4X4(const dsVector2f* xy);

/**
 * @brief Unpacks two floats from a 8-bit integer in YX order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntY4X4(dsVector2f* result, uint8_t value);

/**
 * @brief Packs two floats into a 8-bit unsigned integer in YX order, 4 bits each.
 * @param xy The values to pack.
 * @return The packed integer.
 */
inline uint8_t dsPackUIntY4X4(const dsVector2f* xy);

/**
 * @brief Unpacks two floats from a 8-bit unsigned integer in YX order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntY4X4(dsVector2f* result, uint8_t value);

/**
 * @brief Packs four floats into a 16-bit integer in XYZW order, 4 bits each.
 * @param xyzw The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackIntX4Y4Z4W4(const dsVector4f* xyzw);

/**
 * @brief Unpacks four floats from a 16-bit integer in XYZW order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntX4Y4Z4W4(dsVector4f* result, uint16_t value);

/**
 * @brief Packs four floats into a 16-bit unsigned integer in XYZW order, 4 bits each.
 * @param xyzw The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackUIntX4Y4Z4W4(const dsVector4f* xyzw);

/**
 * @brief Unpacks four floats from a 16-bit unsigned integer in XYZW order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntX4Y4Z4W4(dsVector4f* result, uint16_t value);

/**
 * @brief Packs four floats into a 16-bit integer in WZYX order, 4 bits each.
 * @param wxyz The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackIntW4Z4Y4X4(const dsVector4f* wxyz);

/**
 * @brief Unpacks four floats from a 16-bit integer in WZYX order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntW4Z4Y4X4(dsVector4f* result, uint16_t value);

/**
 * @brief Packs four floats into a 16-bit unsigned integer in WZYX order, 4 bits each.
 * @param wxyz The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackUIntW4Z4Y4X4(const dsVector4f* wxyz);

/**
 * @brief Unpacks four floats from a 16-bit unsigned integer in WZYX order, 4 bits each.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntW4Z4Y4X4(dsVector4f* result, uint16_t value);

/**
 * @brief Packs three floats into a 16-bit integer in XYZ order, with 5, 6, 5 bits.
 * @param xyz The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackIntX5Y6Z5(const dsVector3f* xyz);

/**
 * @brief Unpacks three floats from a 16-bit integer in XYZ order, with 5, 6, 5 bits.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntX5Y6Z5(dsVector3f* result, uint16_t value);

/**
 * @brief Packs three floats into a 16-bit unsigned integer in XYZ order, with 5, 6, 5 bits.
 * @param xyz The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackUIntX5Y6Z5(const dsVector3f* xyz);

/**
 * @brief Unpacks three floats from a 16-bit unsigned integer in XYZ order, with 5, 6, 5 bits.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntX5Y6Z5(dsVector3f* result, uint16_t value);

/**
 * @brief Packs three floats into a 16-bit integer in ZYX order, with 5, 6, 5 bits.
 * @param zyxo The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackIntZ5Y6X5(const dsVector3f* zyxo);

/**
 * @brief Unpacks three floats from a 16-bit integer in ZYX order, with 5, 6, 5 bits.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackIntZ5Y6X5(dsVector3f* result, uint16_t value);

/**
 * @brief Packs three floats into a 16-bit unsigned integer in ZYX order, with 5, 6, 5 bits.
 * @param zyxo The values to pack.
 * @return The packed integer.
 */
inline uint16_t dsPackUIntZ5Y6X5(const dsVector3f* zyxo);

/**
 * @brief Unpacks three floats from a 16-bit unsigned integer in ZYX order, with 5, 6, 5 bits.
 * @param[out] result The unpacked values.
 * @param value The values to unpack.
 */
inline void dsUnpackUIntZ5Y6X5(dsVector3f* result, uint16_t value);

inline int32_t dsPackInt32(float x)
{
	return (int32_t)(round((double)dsClamp(x, -1, 1)*0x7FFFFFFF));
}

inline float dsUnpackInt32(int32_t x)
{
	return (float)((double)x/0x7FFFFFFF);
}

inline uint32_t dsPackUInt32(float x)
{
	return (uint32_t)(round((double)dsClamp(x, 0, 1)*0xFFFFFFFF));
}

inline float dsUnpackUInt32(uint32_t x)
{
	return (float)((double)x/0xFFFFFFFF);
}

inline int16_t dsPackInt16(float x)
{
	return (int16_t)(roundf(dsClamp(x, -1, 1)*0x7FFF));
}

inline float dsUnpackInt16(int16_t x)
{
	return (float)x/0x7FFF;
}

inline uint16_t dsPackUInt16(float x)
{
	return (uint16_t)roundf(dsClamp(x, 0, 1)*0xFFFF);
}

inline float dsUnpackUInt16(uint16_t x)
{
	return (float)x/0xFFFF;
}

inline int8_t dsPackInt8(float x)
{
	return (int8_t)(roundf(dsClamp(x, -1, 1)*0x7F));
}

inline float dsUnpackInt8(int8_t x)
{
	return (float)x/0x7F;
}

inline uint8_t dsPackUInt8(float x)
{
	return (uint8_t)roundf(dsClamp(x, 0, 1)*0xFF);
}

inline float dsUnpackUInt8(uint8_t x)
{
	return (float)x/0xFF;
}

inline uint8_t dsPackIntX4Y4(const dsVector2f* xy)
{
	DS_ASSERT(xy);
	uint8_t x = (uint8_t)(roundf(dsClamp(xy->x, -1, 1)*0x7)) & 0xF;
	uint8_t y = (uint8_t)(roundf(dsClamp(xy->y, -1, 1)*0x7)) & 0xF;
	return (uint8_t)(x | (y << 4));
}

inline void dsUnpackIntX4Y4(dsVector2f* result, uint8_t value)
{
	DS_ASSERT(result);
	int8_t component = (int8_t)(value & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->x = (float)component/0x7;

	component = (((int8_t)value >> 4) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->y = (float)component/0x7;
}

inline uint8_t dsPackUIntX4Y4(const dsVector2f* xy)
{
	DS_ASSERT(xy);
	uint8_t x = (uint8_t)(roundf(dsClamp(xy->x, 0, 1)*0xF)) & 0xF;
	uint8_t y = (uint8_t)(roundf(dsClamp(xy->y, 0, 1)*0xF)) & 0xF;
	return (uint8_t)(x | (y << 4));
}

inline void dsUnpackUIntX4Y4(dsVector2f* result, uint8_t value)
{
	DS_ASSERT(result);
	result->x = (float)(value & 0xF)/0xF;
	result->y = (float)((value >> 4) & 0xF)/0xF;
}

inline uint8_t dsPackIntY4X4(const dsVector2f* xy)
{
	DS_ASSERT(xy);
	uint8_t x = (uint8_t)(roundf(dsClamp(xy->x, -1, 1)*0x7)) & 0xF;
	uint8_t y = (uint8_t)(roundf(dsClamp(xy->y, -1, 1)*0x7)) & 0xF;
	return (uint8_t)(y | (x << 4));
}

inline void dsUnpackIntY4X4(dsVector2f* result, uint8_t value)
{
	DS_ASSERT(result);
	int8_t component = (int8_t)(value & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->y = (float)component/0x7;

	component = (int8_t)((value >> 4) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->x = (float)component/0x7;
}

inline uint8_t dsPackUIntY4X4(const dsVector2f* xy)
{
	DS_ASSERT(xy);
	uint8_t x = (uint8_t)(roundf(dsClamp(xy->x, 0, 1)*0xF)) & 0xF;
	uint8_t y = (uint8_t)(roundf(dsClamp(xy->y, 0, 1)*0xF)) & 0xF;
	return (uint8_t)(y | (x << 4));
}

inline void dsUnpackUIntY4X4(dsVector2f* result, uint8_t value)
{
	DS_ASSERT(result);
	result->y = (float)(value & 0xF)/0xF;
	result->x = (float)((value >> 4) & 0xF)/0xF;
}

inline uint16_t dsPackIntX4Y4Z4W4(const dsVector4f* xyzw)
{
	DS_ASSERT(xyzw);
	uint16_t x = (uint16_t)(roundf(dsClamp(xyzw->x, -1, 1)*0x7)) & 0xF;
	uint16_t y = (uint16_t)(roundf(dsClamp(xyzw->y, -1, 1)*0x7)) & 0xF;
	uint16_t z = (uint16_t)(roundf(dsClamp(xyzw->z, -1, 1)*0x7)) & 0xF;
	uint16_t w = (uint16_t)(roundf(dsClamp(xyzw->w, -1, 1)*0x7)) & 0xF;
	return (uint16_t)(x | (y << 4) | (z << 8) | (w << 12));
}

inline void dsUnpackIntX4Y4Z4W4(dsVector4f* result, uint16_t value)
{
	DS_ASSERT(result);
	int8_t component = (int8_t)(value & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->x = (float)component/0x7;

	component = (int8_t)((value >> 4) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->y = (float)component/0x7;

	component = (int8_t)((value >> 8) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->z = (float)component/0x7;

	component = (int8_t)((value >> 12) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->w = (float)component/0x7;
}

inline uint16_t dsPackUIntX4Y4Z4W4(const dsVector4f* xyzw)
{
	DS_ASSERT(xyzw);
	uint16_t x = (uint16_t)(roundf(dsClamp(xyzw->x, 0, 1)*0xF)) & 0xF;
	uint16_t y = (uint16_t)(roundf(dsClamp(xyzw->y, 0, 1)*0xF)) & 0xF;
	uint16_t z = (uint16_t)(roundf(dsClamp(xyzw->z, 0, 1)*0xF)) & 0xF;
	uint16_t w = (uint16_t)(roundf(dsClamp(xyzw->w, 0, 1)*0xF)) & 0xF;
	return (uint16_t)(x | (y << 4) | (z << 8) | (w << 12));
}

inline void dsUnpackUIntX4Y4Z4W4(dsVector4f* result, uint16_t value)
{
	DS_ASSERT(result);
	result->x = (float)(value & 0xF)/0xF;
	result->y = (float)((value >> 4) & 0xF)/0xF;
	result->z = (float)((value >> 8) & 0xF)/0xF;
	result->w = (float)((value >> 12) & 0xF)/0xF;
}

inline uint16_t dsPackIntW4Z4Y4X4(const dsVector4f* wxyz)
{
	DS_ASSERT(wxyz);
	uint16_t x = (uint16_t)(roundf(dsClamp(wxyz->x, -1, 1)*0x7)) & 0xF;
	uint16_t y = (uint16_t)(roundf(dsClamp(wxyz->y, -1, 1)*0x7)) & 0xF;
	uint16_t z = (uint16_t)(roundf(dsClamp(wxyz->z, -1, 1)*0x7)) & 0xF;
	uint16_t w = (uint16_t)(roundf(dsClamp(wxyz->w, -1, 1)*0x7)) & 0xF;
	return (uint16_t)(w | (z << 4) | (y << 8) | (x << 12));
}

inline void dsUnpackIntW4Z4Y4X4(dsVector4f* result, uint16_t value)
{
	DS_ASSERT(result);
	int8_t component = (int8_t)(value & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->w = (float)component/0x7;

	component = (int8_t)((value >> 4) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->z = (float)component/0x7;

	component = (int8_t)((value >> 8) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->y = (float)component/0x7;

	component = (int8_t)((value >> 12) & 0xF);
	if (component & 0x8)
		component |= (int8_t)0xF0;
	result->x = (float)component/0x7;
}

inline uint16_t dsPackUIntW4Z4Y4X4(const dsVector4f* wxyz)
{
	DS_ASSERT(wxyz);
	uint16_t x = (uint16_t)(roundf(dsClamp(wxyz->x, 0, 1)*0xF)) & 0xF;
	uint16_t y = (uint16_t)(roundf(dsClamp(wxyz->y, 0, 1)*0xF)) & 0xF;
	uint16_t z = (uint16_t)(roundf(dsClamp(wxyz->z, 0, 1)*0xF)) & 0xF;
	uint16_t w = (uint16_t)(roundf(dsClamp(wxyz->w, 0, 1)*0xF)) & 0xF;
	return (uint16_t)(w | (z << 4) | (y << 8) | (x << 12));
}

inline void dsUnpackUIntW4Z4Y4X4(dsVector4f* result, uint16_t value)
{
	DS_ASSERT(result);
	result->w = (float)(value & 0xF)/0xF;
	result->z = (float)((value >> 4) & 0xF)/0xF;
	result->y = (float)((value >> 8) & 0xF)/0xF;
	result->x = (float)((value >> 12) & 0xF)/0xF;
}

inline uint16_t dsPackIntX5Y6Z5(const dsVector3f* xyz)
{
	DS_ASSERT(xyz);
	uint16_t x = (uint16_t)(roundf(dsClamp(xyz->x, -1, 1)*0xF)) & 0x1F;
	uint16_t y = (uint16_t)(roundf(dsClamp(xyz->y, -1, 1)*0x1F)) & 0x3F;
	uint16_t z = (uint16_t)(roundf(dsClamp(xyz->z, -1, 1)*0xF)) & 0x1F;
	return (uint16_t)(x | (y << 5) | (z << 11));
}

inline void dsUnpackIntX5Y6Z5(dsVector3f* result, uint16_t value)
{
	DS_ASSERT(result);
	int16_t component = (int8_t)(value & 0x1F);
	if (component & 0x10)
		component |= (int8_t)0xE0;
	result->x = (float)component/0xF;

	component = (int8_t)((value >> 5) & 0x3F);
	if (component & 0x20)
		component |= (int8_t)0xC0;
	result->y = (float)component/0x1F;

	component = (int8_t)((value >> 11) & 0x1F);
	if (component & 0x10)
		component |= (int8_t)0xE0;
	result->z = (float)component/0xF;
}

inline uint16_t dsPackUIntX5Y6Z5(const dsVector3f* xyz)
{
	DS_ASSERT(xyz);
	uint16_t x = (uint16_t)(roundf(dsClamp(xyz->x, 0, 1)*0x1F)) & 0x1F;
	uint16_t y = (uint16_t)(roundf(dsClamp(xyz->y, 0, 1)*0x3F)) & 0x3F;
	uint16_t z = (uint16_t)(roundf(dsClamp(xyz->z, 0, 1)*0x1F)) & 0x1F;
	return (uint16_t)(x | (y << 5) | (z << 11));
}

inline void dsUnpackUIntX5Y6Z5(dsVector3f* result, uint16_t value)
{
	DS_ASSERT(result);
	result->x = (float)(value & 0x1F)/0x1F;
	result->y = (float)((value >> 5) & 0x3F)/0x3F;
	result->z = (float)((value >> 11) & 0x1F)/0x1F;
}

inline uint16_t dsPackIntZ5Y6X5(const dsVector3f* zyx)
{
	DS_ASSERT(zyx);
	uint16_t x = (uint16_t)(roundf(dsClamp(zyx->x, -1, 1)*0xF)) & 0x1F;
	uint16_t y = (uint16_t)(roundf(dsClamp(zyx->y, -1, 1)*0x1F)) & 0x3F;
	uint16_t z = (uint16_t)(roundf(dsClamp(zyx->z, -1, 1)*0xF)) & 0x1F;
	return (uint16_t)(z | (y << 5) | (x << 11));
}

inline void dsUnpackIntZ5Y6X5(dsVector3f* result, uint16_t value)
{
	DS_ASSERT(result);
	int16_t component = (int8_t)(value & 0x1F);
	if (component & 0x10)
		component |= (int8_t)0xE0;
	result->z = (float)component/0xF;

	component = (int8_t)((value >> 5) & 0x3F);
	if (component & 0x20)
		component |= (int8_t)0xC0;
	result->y = (float)component/0x1F;

	component = (int8_t)((value >> 11) & 0x1F);
	if (component & 0x10)
		component |= (int8_t)0xE0;
	result->x = (float)component/0xF;
}

inline uint16_t dsPackUIntZ5Y6X5(const dsVector3f* zyx)
{
	DS_ASSERT(zyx);
	uint16_t x = (uint16_t)(roundf(dsClamp(zyx->x, 0, 1)*0x1F)) & 0x1F;
	uint16_t y = (uint16_t)(roundf(dsClamp(zyx->y, 0, 1)*0x3F)) & 0x3F;
	uint16_t z = (uint16_t)(roundf(dsClamp(zyx->z, 0, 1)*0x1F)) & 0x1F;
	return (uint16_t)(z | (y << 5) | (x << 11));
}

inline void dsUnpackUIntZ5Y6X5(dsVector3f* result, uint16_t value)
{
	DS_ASSERT(result);
	result->z = (float)(value & 0x1F)/0x1F;
	result->y = (float)((value >> 5) & 0x3F)/0x3F;
	result->x = (float)((value >> 11) & 0x1F)/0x1F;
}

#ifdef __cplusplus
}
#endif
