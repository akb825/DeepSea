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
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Core math functions and macros.
 */

/**
 * @brief Returns the minimum between x and y.
 * @param x The first value.
 * @param y The second value.
 * @return The minimum between x and y.
 */
#define dsMin(x, y) ((x) < (y) ? (x) : (y))

/**
 * @brief Returns the maximum between x and y.
 * @param x The first value.
 * @param y The second value.
 * @return The maximum between x and y.
 */
#define dsMax(x, y) ((x) > (y) ? (x) : (y))

/**
 * @brief Takes the second power of a value.
 * @param x The value.
 * @return The second power of x.
 */
#define dsPow2(x) ((x)*(x))

/**
 * @brief Takes the third power of a value.
 * @param x The value.
 * @return The third power of x.
 */
#define dsPow3(x) ((x)*(x)*(x))

/**
 * @brief Clamps a value between a minimum and maximum value.
 * @param x The value to clamp.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return The clamped value.
 */
#define dsClamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/**
 * @brief Linearly interpolates between two values.
 * @param x The first value.
 * @param y The second value.
 * @param t The interpolation value between x and y.
 */
#define dsLerp(x, y, t) ((x) + (t)*((y) - (x)))

/**
 * @brief Converts from any structure that contains entirely floats (e.g. dsVector3f, dsMatrix44f)
 * to any corresponding structure that contains entirely doubles. (e.g. dsVector3d, dsMatrix44d)
 * @param[out] doubleStruct The double structure to convert to.
 * @param floatStruct The float struct.
 */
#define dsConvertFloatToDouble(doubleStruct, floatStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(doubleStruct) % sizeof(double) == 0, doubleStruct_must_be_double); \
		DS_STATIC_ASSERT(sizeof(floatStruct) % sizeof(float) == 0, floatStruct_must_be_float); \
		DS_STATIC_ASSERT(sizeof(doubleStruct)/sizeof(double) == \
			sizeof(floatStruct)/sizeof(float), doubleStruct_elements_not_equal_to_floatStruct); \
		unsigned int _dsConvertLen = sizeof(floatStruct)/sizeof(float); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((double*)&(doubleStruct))[_dsConvertI] = \
				(double)((const float*)&(floatStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely doubles (e.g. dsVector3d, dsMatrix44d)
 * to any corresponding structure that contains entirely floats. (e.g. dsVector3f, dsMatrix44f)
 * @param[out] floatStruct The float structure to convert to.
 * @param doubleStruct The double struct.
 */
#define dsConvertDoubleToFloat(floatStruct, doubleStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(doubleStruct) % sizeof(double) == 0, doubleStruct_must_be_double); \
		DS_STATIC_ASSERT(sizeof(floatStruct) % sizeof(float) == 0, floatStruct_must_be_float); \
		DS_STATIC_ASSERT(sizeof(doubleStruct)/sizeof(double) == \
			sizeof(floatStruct)/sizeof(float), doubleStruct_elements_not_equal_to_floatStruct); \
		unsigned int _dsConvertLen = sizeof(floatStruct)/sizeof(float); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((float*)&(floatStruct))[_dsConvertI] = \
				(float)((const double*)&(doubleStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely floats (e.g. dsVector3f)
 * to any corresponding structure that contains entirely ints. (e.g. dsVector3i)
 * @param[out] intStruct The int structure to convert to.
 * @param floatStruct The float struct.
 */
#define dsConvertFloatToInt(intStruct, floatStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(intStruct) % sizeof(int) == 0, intStruct_must_be_int); \
		DS_STATIC_ASSERT(sizeof(floatStruct) % sizeof(float) == 0, floatStruct_must_be_float); \
		DS_STATIC_ASSERT(sizeof(intStruct)/sizeof(int) == \
			sizeof(floatStruct)/sizeof(float), intStruct_elements_not_equal_to_floatStruct); \
		unsigned int _dsConvertLen = sizeof(floatStruct)/sizeof(float); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((int*)&(intStruct))[_dsConvertI] = \
				(int)((const float*)&(floatStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely ints (e.g. dsVector3i)
 * to any corresponding structure that contains entirely floats. (e.g. dsVector3f)
 * @param[out] floatStruct The float structure to convert to.
 * @param intStruct The int struct.
 */
#define dsConvertIntToFloat(floatStruct, intStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(intStruct) % sizeof(int) == 0, intStruct_must_be_int); \
		DS_STATIC_ASSERT(sizeof(floatStruct) % sizeof(float) == 0, floatStruct_must_be_float); \
		DS_STATIC_ASSERT(sizeof(intStruct)/sizeof(int) == \
			sizeof(floatStruct)/sizeof(float), intStruct_elements_not_equal_to_floatStruct); \
		unsigned int _dsConvertLen = sizeof(floatStruct)/sizeof(float); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((float*)&(floatStruct))[_dsConvertI] = \
				(float)((const int*)&(intStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely doubles (e.g. dsVector3d)
 * to any corresponding structure that contains entirely ints. (e.g. dsVector3i)
 * @param[out] intStruct The int structure to convert to.
 * @param doubleStruct The double struct.
 */
#define dsConvertDoubleToInt(intStruct, doubleStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(intStruct) % sizeof(int) == 0, intStruct_must_be_int); \
		DS_STATIC_ASSERT(sizeof(doubleStruct) % sizeof(double) == 0, doubleStruct_must_be_double); \
		DS_STATIC_ASSERT(sizeof(intStruct)/sizeof(int) == \
			sizeof(doubleStruct)/sizeof(double), intStruct_elements_not_equal_to_doubleStruct); \
		unsigned int _dsConvertLen = sizeof(doubleStruct)/sizeof(double); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((int*)&(intStruct))[_dsConvertI] = \
				(int)((const double*)&(doubleStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely ints (e.g. dsVector3i)
 * to any corresponding structure that contains entirely doubles. (e.g. dsVector3f)
 * @param[out] doubleStruct The double structure to convert to.
 * @param intStruct The int struct.
 */
#define dsConvertIntToDouble(doubleStruct, intStruct) \
	do \
	{ \
		DS_STATIC_ASSERT(sizeof(intStruct) % sizeof(int) == 0, intStruct_must_be_int); \
		DS_STATIC_ASSERT(sizeof(doubleStruct) % sizeof(double) == 0, doubleStruct_must_be_double); \
		DS_STATIC_ASSERT(sizeof(intStruct)/sizeof(int) == \
			sizeof(doubleStruct)/sizeof(double), intStruct_elements_not_equal_to_doubleStruct); \
		unsigned int _dsConvertLen = sizeof(doubleStruct)/sizeof(double); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((double*)&(doubleStruct))[_dsConvertI] = \
				(double)((const int*)&(intStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts degrees to radians.
 * @param degrees The angle in degrees.
 * @return The angle in radians.
 */
inline double dsDegreesToRadians(double degrees);

/**
 * @brief Converts radians to degrees.
 * @param radians The angle in radians.
 * @return The angle in degrees.
 */
inline double dsRadiansToDegrees(double radians);

inline double dsDegreesToRadians(double degrees)
{
	return degrees*M_PI/180;
}

inline double dsRadiansToDegrees(double radians)
{
	return radians*180/M_PI;
}

#ifdef __cplusplus
}
#endif
