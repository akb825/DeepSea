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
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for manipulating dsVector4* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector4* type. All parameters are
 * by value. In all cases, it is safe to have the result be the same as one of the parameters.
 *
 * The functions have different versions for the supported Vector4 types. These are used when the
 * implementation cannot be practically done within a macro.
 */

/**
 * @brief Adds the components of two vectors.
 * @param result The result of a + b.
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector4_add(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] + (b).values[0]; \
		(result).values[1] = (a).values[1] + (b).values[1]; \
		(result).values[2] = (a).values[2] + (b).values[2]; \
		(result).values[3] = (a).values[3] + (b).values[3]; \
	} while (0)

/**
 * @brief Sebutracts the components of two vectors.
 * @param result The result of a - (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector4_sub(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] - (b).values[0]; \
		(result).values[1] = (a).values[1] - (b).values[1]; \
		(result).values[2] = (a).values[2] - (b).values[2]; \
		(result).values[3] = (a).values[3] - (b).values[3]; \
	} while (0)

/**
 * @brief Multiplies the components of two vectors.
 * @param result The result of a * (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector4_mul(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(b).values[0]; \
		(result).values[1] = (a).values[1]*(b).values[1]; \
		(result).values[2] = (a).values[2]*(b).values[2]; \
		(result).values[3] = (a).values[3]*(b).values[3]; \
	} while (0)

/**
 * @brief Divides the components of two vectors.
 * @param result The result of a / (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector4_div(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]/(b).values[0]; \
		(result).values[1] = (a).values[1]/(b).values[1]; \
		(result).values[2] = (a).values[2]/(b).values[2]; \
		(result).values[3] = (a).values[3]/(b).values[3]; \
	} while (0)

/**
 * @brief Scales a vector.
 * @param result The result of a * s.
 * @param a The vector.
 * @param s The scale factor.
 */
#define dsVector4_scale(result, a, s) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(s); \
		(result).values[1] = (a).values[1]*(s); \
		(result).values[2] = (a).values[2]*(s); \
		(result).values[3] = (a).values[3]*(s); \
	} while (0)

/**
 * @brief Takes the dot product between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of a dot b.
 */
#define dsVector4_dot(a, b) \
	((a).values[0]*(b).values[0] + \
	 (a).values[1]*(b).values[1] + \
	 (a).values[2]*(b).values[2] + \
	 (a).values[3]*(b).values[3])

/**
 * @brief Gets the squared length of a vector.
 * @param a The vector.
 * @return The squared length.
 */
#define dsVector4_len2(a) \
	dsVector4_dot((a), (a))

/**
 * @brief Gets the squared distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The squared distance.
 */
#define dsVector4_dist2(a, b) \
	(dsPow2((a).values[0] - (b).values[0]) + \
	 dsPow2((a).values[1] - (b).values[1]) + \
	 dsPow2((a).values[2] - (b).values[2]) + \
	 dsPow2((a).values[3] - (b).values[3]))

/**
 * @brief Gets the length of a vector.
 * @param a The first vector.
 * @return The length.
 */
inline float dsVector4f_len(dsVector4f* a);

/** @copydoc dsVector4f_len() */
inline double dsVector4d_len(dsVector4d* a);

/**
 * @brief Gets the distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The length.
 */
inline float dsVector4f_dist(dsVector4f* a, dsVector4f* b);

/** @copydoc dsVector4f_dist() */
inline double dsVector4d_dist(dsVector4d* a, dsVector4d* b);

/**
 * @brief Normalizes a vector.
 * @param result The normalized vector.
 * @param a The vector to normalize.
 */
inline void dsVector4f_normalize(dsVector4f* result, dsVector4f* a);

/** @copydoc dsVector4f_normalize() */
inline void dsVector4d_normalize(dsVector4d* result, dsVector4d* a);

inline float dsVector4f_len(dsVector4f* a)
{
	DS_ASSERT(a);
	return sqrtf(dsVector4_len2(*a));
}

inline double dsVector4d_len(dsVector4d* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector4_len2(*a));
}

inline float dsVector4f_dist(dsVector4f* a, dsVector4f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrtf(dsVector4_dist2(*a, *b));
}

inline double dsVector4d_dist(dsVector4d* a, dsVector4d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector4_dist2(*a, *b));
}

inline void dsVector4f_normalize(dsVector4f* result, dsVector4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	float length = dsVector4f_len(a);
	DS_ASSERT(length > 0);
	dsVector4_scale(*result, *a, 1/length);
}

inline void dsVector4d_normalize(dsVector4d* result, dsVector4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	double length = dsVector4d_len(a);
	DS_ASSERT(length > 0);
	dsVector4_scale(*result, *a, 1/length);
}

#ifdef __cplusplus
}
#endif
