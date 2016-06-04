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

#ifdef __cplusplus
}
#endif
