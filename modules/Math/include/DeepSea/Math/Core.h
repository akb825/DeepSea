/*
 * Copyright 2016-2021 Aaron Barany
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
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Math/Export.h>
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
 * @return The interpolated value.
 */
#define dsLerp(x, y, t) ((x) + (t)*((y) - (x)))

/**
 * @brief Converts from any structure that contains entirely floats (e.g. dsVector3f, dsMatrix44f)
 *     to any corresponding structure that contains entirely doubles. (e.g. dsVector3d, dsMatrix44d)
 * @param[out] doubleStruct The double structure to convert to.
 * @param floatStruct The float struct.
 */
#define dsConvertFloatToDouble(doubleStruct, floatStruct) \
	do \
	{ \
		_Static_assert(sizeof(doubleStruct) % sizeof(double) == 0, \
			"doubleStruct must be double."); \
		_Static_assert(sizeof(floatStruct) % sizeof(float) == 0, "floatStruct must be float."); \
		_Static_assert(sizeof(doubleStruct)/sizeof(double) == \
			sizeof(floatStruct)/sizeof(float), "doubleStruct elements not equal to floatStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(floatStruct)/sizeof(float)); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((double*)&(doubleStruct))[_dsConvertI] = \
				(double)((const float*)&(floatStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely doubles (e.g. dsVector3d, dsMatrix44d)
 *     to any corresponding structure that contains entirely floats. (e.g. dsVector3f, dsMatrix44f)
 * @param[out] floatStruct The float structure to convert to.
 * @param doubleStruct The double struct.
 */
#define dsConvertDoubleToFloat(floatStruct, doubleStruct) \
	do \
	{ \
		_Static_assert(sizeof(doubleStruct) % sizeof(double) == 0, \
			"doubleStruct must be double."); \
		_Static_assert(sizeof(floatStruct) % sizeof(float) == 0, "floatStruct must be float."); \
		_Static_assert(sizeof(doubleStruct)/sizeof(double) == \
			sizeof(floatStruct)/sizeof(float), "doubleStruct elements not equal to floatStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(floatStruct)/sizeof(float)); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((float*)&(floatStruct))[_dsConvertI] = \
				(float)((const double*)&(doubleStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely floats (e.g. dsVector3f)
 *     to any corresponding structure that contains entirely ints. (e.g. dsVector3i)
 * @param[out] intStruct The int structure to convert to.
 * @param floatStruct The float struct.
 */
#define dsConvertFloatToInt(intStruct, floatStruct) \
	do \
	{ \
		_Static_assert(sizeof(intStruct) % sizeof(int) == 0, "intStruct must be int."); \
		_Static_assert(sizeof(floatStruct) % sizeof(float) == 0, "floatStruct must be float."); \
		_Static_assert(sizeof(intStruct)/sizeof(int) == \
			sizeof(floatStruct)/sizeof(float), "intStruct elements not equal to floatStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(floatStruct)/sizeof(float)); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((int*)&(intStruct))[_dsConvertI] = \
				(int)((const float*)&(floatStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely ints (e.g. dsVector3i)
 *     to any corresponding structure that contains entirely floats. (e.g. dsVector3f)
 * @param[out] floatStruct The float structure to convert to.
 * @param intStruct The int struct.
 */
#define dsConvertIntToFloat(floatStruct, intStruct) \
	do \
	{ \
		_Static_assert(sizeof(intStruct) % sizeof(int) == 0, "intStruct must be int."); \
		_Static_assert(sizeof(floatStruct) % sizeof(float) == 0, "floatStruct must be float."); \
		_Static_assert(sizeof(intStruct)/sizeof(int) == \
			sizeof(floatStruct)/sizeof(float), "intStruct elements not equal to floatStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(floatStruct)/sizeof(float)); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((float*)&(floatStruct))[_dsConvertI] = \
				(float)((const int*)&(intStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely doubles (e.g. dsVector3d)
 *     to any corresponding structure that contains entirely ints. (e.g. dsVector3i)
 * @param[out] intStruct The int structure to convert to.
 * @param doubleStruct The double struct.
 */
#define dsConvertDoubleToInt(intStruct, doubleStruct) \
	do \
	{ \
		_Static_assert(sizeof(intStruct) % sizeof(int) == 0, "intStruct must be int."); \
		_Static_assert(sizeof(doubleStruct) % sizeof(double) == 0, \
			"doubleStruct must be double."); \
		_Static_assert(sizeof(intStruct)/sizeof(int) == \
			sizeof(doubleStruct)/sizeof(double), "intStruct elements not equal to doubleStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(doubleStruct)/sizeof(double)); \
		for (unsigned int _dsConvertI = 0; _dsConvertI < _dsConvertLen; ++_dsConvertI) \
		{ \
			((int*)&(intStruct))[_dsConvertI] = \
				(int)((const double*)&(doubleStruct))[_dsConvertI]; \
		} \
	} while (0)

/**
 * @brief Converts from any structure that contains entirely ints (e.g. dsVector3i) to any
 *     corresponding structure that contains entirely doubles. (e.g. dsVector3f)
 * @param[out] doubleStruct The double structure to convert to.
 * @param intStruct The int struct.
 */
#define dsConvertIntToDouble(doubleStruct, intStruct) \
	do \
	{ \
		_Static_assert(sizeof(intStruct) % sizeof(int) == 0, "intStruct must be int."); \
		_Static_assert(sizeof(doubleStruct) % sizeof(double) == 0, \
			"doubleStruct must be double."); \
		_Static_assert(sizeof(intStruct)/sizeof(int) == \
			sizeof(doubleStruct)/sizeof(double), "intStruct elements not equal to doubleStruct."); \
		unsigned int _dsConvertLen = (unsigned int)(sizeof(doubleStruct)/sizeof(double)); \
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
DS_MATH_EXPORT inline double dsDegreesToRadiansd(double degrees);

/** @copydoc dsDegreesToRadiansd() */
DS_MATH_EXPORT inline float dsDegreesToRadiansf(float degrees);

/**
 * @brief Converts radians to degrees.
 * @param radians The angle in radians.
 * @return The angle in degrees.
 */
DS_MATH_EXPORT inline double dsRadiansToDegreesd(double radians);

/** @copydoc dsRadiansToDegreesd() */
DS_MATH_EXPORT inline float dsRadiansToDegreesf(float radians);

/**
 * @brief Gets the next power of two.
 * @param i The integer. This must be > 0.
 * @return The next power of two of i. If i is already a power of two, i will be returned.
 */
DS_MATH_EXPORT inline uint32_t dsNextPowerOf2(uint32_t i);

/**
 * @brief Wraps a value in the range [min, max).
 * @param x The value to wrap.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return The wrapped into the range [min, max).
 */
DS_MATH_EXPORT inline int dsWrapi(int x, int min, int max);

/** @copydoc dsWrapi() */
DS_MATH_EXPORT inline float dsWrapf(float x, float min, float max);

/** @copydoc dsWrapi() */
DS_MATH_EXPORT inline double dsWrapd(double x, double min, double max);

/**
 * @brief Checks to see if two values are equal within an epsilon.
 * @param x The first value.
 * @param y The second value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsEpsilonEqualf(float x, float y, float epsilon);

/** @copydoc dsEpsilonEqualf() */
DS_MATH_EXPORT inline bool dsEpsilonEquald(double x, double y, double epsilon);

/**
 * @brief Checks to see if two values are equal within a relative epislon.
 *
 * This will check first if the values are within epsilon of each-other, then scales the epsilon
 * based on the maximum value passed in.
 *
 * @param x The first value.
 * @param y The second value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of x and y are within epsilon.
 */
DS_MATH_EXPORT inline bool dsRelativeEpsilonEqualf(float x, float y, float epsilon);

/** @copydoc dsEpsilonEqualf() */
DS_MATH_EXPORT inline bool dsRelativeEpsilonEquald(double x, double y, double epsilon);

/**
 * @brief Checks to see if a value is equal to zero within an epsilon.
 * @param x The value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of x is within epsilon of zero.
 */
DS_MATH_EXPORT inline bool dsEpsilonEqualsZerof(float x, float epsilon);

/** @copydoc dsEpsilonEqualsZerof() */
DS_MATH_EXPORT inline bool dsEpsilonEqualsZerod(double x, double epsilon);

DS_MATH_EXPORT inline double dsDegreesToRadiansd(double degrees)
{
	return degrees*M_PI/180;
}

DS_MATH_EXPORT inline float dsDegreesToRadiansf(float degrees)
{
	return degrees*(float)M_PI/180;
}

DS_MATH_EXPORT inline double dsRadiansToDegreesd(double radians)
{
	return radians*180/M_PI;
}

DS_MATH_EXPORT inline float dsRadiansToDegreesf(float radians)
{
	return radians*180/(float)M_PI;
}

DS_MATH_EXPORT inline uint32_t dsNextPowerOf2(uint32_t i)
{
	return 1 << (32 - dsClz(i - 1));
}

DS_MATH_EXPORT inline int dsWrapi(int x, int min, int max)
{
	int delta = max - min;
	int relX = x - min;
	if (relX < 0)
		relX -= delta;
	return x - delta*(relX/delta);
}

DS_MATH_EXPORT inline float dsWrapf(float x, float min, float max)
{
	float delta = max - min;
	float relX = x - min;
	return x - delta*floorf(relX/delta);
}

DS_MATH_EXPORT inline double dsWrapd(double x, double min, double max)
{
	double delta = max - min;
	double relX = x - min;
	return x - delta*floor(relX/delta);
}

DS_MATH_EXPORT inline bool dsEpsilonEqualf(float x, float y, float epsilon)
{
	return fabsf(x - y) <= epsilon;
}

DS_MATH_EXPORT inline bool dsEpsilonEquald(double x, double y, double epsilon)
{
	return fabs(x - y) <= epsilon;
}

DS_MATH_EXPORT inline bool dsRelativeEpsilonEqualf(float x, float y, float epsilon)
{
	float diff = fabsf(x - y);
	if (diff <= epsilon)
		return true;

	float absX = fabsf(x);
	float absY = fabsf(y);
	return diff <= dsMax(absX, absY)*epsilon;
}

DS_MATH_EXPORT inline bool dsRelativeEpsilonEquald(double x, double y, double epsilon)
{
	double diff = fabs(x - y);
	if (diff <= epsilon)
		return true;

	double absX = fabs(x);
	double absY = fabs(y);
	return diff <= dsMax(absX, absY)*epsilon;
}

DS_MATH_EXPORT inline bool dsEpsilonEqualsZerof(float x, float epsilon)
{
	return fabsf(x) <= epsilon;
}

DS_MATH_EXPORT inline bool dsEpsilonEqualsZerod(double x, double epsilon)
{
	return fabs(x) <= epsilon;
}

#ifdef __cplusplus
}
#endif
