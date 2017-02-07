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
 * @brief Macros and functions for manipulating dsVector3* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector3* type. All parameters are
 * by value. With the exception of dsVector3_cross(), it is safe to have the result be the same as
 * one of the parameters.
 *
 * The functions have different versions for the supported Vector3 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsVector3f dsVector3d dsVector3i
 */

/**
 * @brief Adds the components of two vectors.
 * @param result The result of a + b.
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector3_add(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] + (b).values[0]; \
		(result).values[1] = (a).values[1] + (b).values[1]; \
		(result).values[2] = (a).values[2] + (b).values[2]; \
	} while (0)

/**
 * @brief Sebutracts the components of two vectors.
 * @param result The result of a - (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector3_sub(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] - (b).values[0]; \
		(result).values[1] = (a).values[1] - (b).values[1]; \
		(result).values[2] = (a).values[2] - (b).values[2]; \
	} while (0)

/**
 * @brief Multiplies the components of two vectors.
 * @param result The result of a * (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector3_mul(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(b).values[0]; \
		(result).values[1] = (a).values[1]*(b).values[1]; \
		(result).values[2] = (a).values[2]*(b).values[2]; \
	} while (0)

/**
 * @brief Divides the components of two vectors.
 * @param result The result of a / (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector3_div(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]/(b).values[0]; \
		(result).values[1] = (a).values[1]/(b).values[1]; \
		(result).values[2] = (a).values[2]/(b).values[2]; \
	} while (0)

/**
 * @brief Scales a vector.
 * @param result The result of a * s.
 * @param a The vector.
 * @param s The scale factor.
 */
#define dsVector3_scale(result, a, s) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(s); \
		(result).values[1] = (a).values[1]*(s); \
		(result).values[2] = (a).values[2]*(s); \
	} while (0)

/**
 * @brief Takes the dot product between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of a dot b.
 */
#define dsVector3_dot(a, b) \
	((a).values[0]*(b).values[0] + \
	 (a).values[1]*(b).values[1] + \
	 (a).values[2]*(b).values[2])

/**
 * @brief Takes the cross product between two vectors.
 * @param result The result of a cross b. This may NOT be the same as a or b.
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector3_cross(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[1]*(b).values[2] - (a).values[2]*(b).values[1]; \
		(result).values[1] = (a).values[2]*(b).values[0] - (a).values[0]*(b).values[2]; \
		(result).values[2] = (a).values[0]*(b).values[1] - (a).values[1]*(b).values[0]; \
	} while (0)

/**
 * @brief Gets the squared length of a vector.
 * @param a The vector.
 * @return The squared length.
 */
#define dsVector3_len2(a) \
	dsVector3_dot((a), (a))

/**
 * @brief Gets the squared distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The squared distance.
 */
#define dsVector3_dist2(a, b) \
	(dsPow2((a).values[0] - (b).values[0]) + \
	 dsPow2((a).values[1] - (b).values[1]) + \
	 dsPow2((a).values[2] - (b).values[2]))

/**
 * @brief Gets the length of a vector.
 * @param a The first vector.
 * @return The length.
 */
inline float dsVector3f_len(const dsVector3f* a);

/** @copydoc dsVector3f_len() */
inline double dsVector3d_len(const dsVector3d* a);

/** @copydoc dsVector3f_len() */
inline double dsVector3i_len(const dsVector3i* a);

/**
 * @brief Gets the distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The length.
 */
inline float dsVector3f_dist(const dsVector3f* a, const dsVector3f* b);

/** @copydoc dsVector3f_dist() */
inline double dsVector3d_dist(const dsVector3d* a, const dsVector3d* b);

/** @copydoc dsVector3f_dist() */
inline double dsVector3i_dist(const dsVector3i* a, const dsVector3i* b);

/**
 * @brief Normalizes a vector.
 * @param[out] result The normalized vector.
 * @param a The vector to normalize.
 */
inline void dsVector3f_normalize(dsVector3f* result, const dsVector3f* a);

/** @copydoc dsVector3f_normalize() */
inline void dsVector3d_normalize(dsVector3d* result, const dsVector3d* a);

/** @copydoc dsVector3_add() */
inline void dsVector3f_add(dsVector3f* result, const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_add(*result, *a, *b);
}

/** @copydoc dsVector3_add() */
inline void dsVector3d_add(dsVector3d* result, const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_add(*result, *a, *b);
}

/** @copydoc dsVector3_add() */
inline void dsVector3i_add(dsVector3i* result, const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_add(*result, *a, *b);
}

/** @copydoc dsVector3_sub() */
inline void dsVector3f_sub(dsVector3f* result, const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_sub(*result, *a, *b);
}

/** @copydoc dsVector3_sub() */
inline void dsVector3d_sub(dsVector3d* result, const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_sub(*result, *a, *b);
}

/** @copydoc dsVector3_sub() */
inline void dsVector3i_sub(dsVector3i* result, const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_sub(*result, *a, *b);
}

/** @copydoc dsVector3_mul() */
inline void dsVector3f_mul(dsVector3f* result, const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_mul(*result, *a, *b);
}

/** @copydoc dsVector3_mul() */
inline void dsVector3d_mul(dsVector3d* result, const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_mul(*result, *a, *b);
}

/** @copydoc dsVector3_mul() */
inline void dsVector3i_mul(dsVector3i* result, const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_mul(*result, *a, *b);
}

/** @copydoc dsVector3_div() */
inline void dsVector3f_div(dsVector3f* result, const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_div(*result, *a, *b);
}

/** @copydoc dsVector3_div() */
inline void dsVector3d_div(dsVector3d* result, const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_div(*result, *a, *b);
}

/** @copydoc dsVector3_div() */
inline void dsVector3i_div(dsVector3i* result, const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_div(*result, *a, *b);
}

/** @copydoc dsVector3_scale() */
inline void dsVector3f_scale(dsVector3f* result, const dsVector3f* a, float s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector3_scale(*result, *a, s);
}

/** @copydoc dsVector3_scale() */
inline void dsVector3d_scale(dsVector3d* result, const dsVector3d* a, double s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector3_scale(*result, *a, s);
}

/** @copydoc dsVector3_scale() */
inline void dsVector3i_scale(dsVector3i* result, const dsVector3i* a, int s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector3_scale(*result, *a, s);
}

/** @copydoc dsVector3_dot() */
inline float dsVector3f_dot(const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dot(*a, *b);
}

/** @copydoc dsVector3_dot() */
inline double dsVector3d_dot(const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dot(*a, *b);
}

/** @copydoc dsVector3_dot() */
inline int dsVector3i_dot(const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dot(*a, *b);
}

/** @copydoc dsVector3_cross() */
inline void dsVector3f_cross(dsVector3f* result, const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_cross(*result, *a, *b);
}

/** @copydoc dsVector3_cross() */
inline void dsVector3d_cross(dsVector3d* result, const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_cross(*result, *a, *b);
}

/** @copydoc dsVector3_cross() */
inline void dsVector3i_cross(dsVector3i* result, const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector3_cross(*result, *a, *b);
}

/** @copydoc dsVector3_len2() */
inline float dsVector3f_len2(const dsVector3f* a)
{
	DS_ASSERT(a);
	return dsVector3_len2(*a);
}

/** @copydoc dsVector3_len2() */
inline double dsVector3d_len2(const dsVector3d* a)
{
	DS_ASSERT(a);
	return dsVector3_len2(*a);
}

/** @copydoc dsVector3_len2() */
inline int dsVector3i_len2(const dsVector3i* a)
{
	DS_ASSERT(a);
	return dsVector3_len2(*a);
}

/** @copydoc dsVector3_dist2() */
inline float dsVector3f_dist2(const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dist2(*a, *b);
}

/** @copydoc dsVector3_dist2() */
inline double dsVector3d_dist2(const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dist2(*a, *b);
}

/** @copydoc dsVector3_dist2() */
inline int dsVector3i_dist2(const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_dist2(*a, *b);
}

inline float dsVector3f_len(const dsVector3f* a)
{
	DS_ASSERT(a);
	return sqrtf(dsVector3_len2(*a));
}

inline double dsVector3d_len(const dsVector3d* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector3_len2(*a));
}

inline double dsVector3i_len(const dsVector3i* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector3_len2(*a));
}

inline float dsVector3f_dist(const dsVector3f* a, const dsVector3f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrtf(dsVector3_dist2(*a, *b));
}

inline double dsVector3d_dist(const dsVector3d* a, const dsVector3d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector3_dist2(*a, *b));
}

inline double dsVector3i_dist(const dsVector3i* a, const dsVector3i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector3_dist2(*a, *b));
}

inline void dsVector3f_normalize(dsVector3f* result, const dsVector3f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	float length = dsVector3f_len(a);
	DS_ASSERT(length > 0);
	dsVector3_scale(*result, *a, 1/length);
}

inline void dsVector3d_normalize(dsVector3d* result, const dsVector3d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	double length = dsVector3d_len(a);
	DS_ASSERT(length > 0);
	dsVector3_scale(*result, *a, 1/length);
}

#ifdef __cplusplus
}
#endif
