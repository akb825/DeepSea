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
 * @brief Macros and functions for manipulating dsVector4* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector4* type. All parameters are
 * by value. In all cases, it is safe to have the result be the same as one of the parameters.
 *
 * The functions have different versions for the supported Vector4 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsVector4f dsVector4d dsVector4i
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
 * @brief Negates the components of a vector.
 * @param result The result of -a.
 * @param a The vector.
 */
#define dsVector4_neg(result, a) \
	do \
	{ \
		(result).values[0] = -(a).values[0]; \
		(result).values[1] = -(a).values[1]; \
		(result).values[2] = -(a).values[2]; \
		(result).values[3] = -(a).values[3]; \
	} while (0)

/**
 * @brief Linearly interpolates between two values.
 * @remark For dsVector4i, it would be better to call dsVector4i_lerp() directly instead instead of
 *     of the macro version.
 * @param[out] result The interpolated result.
 * @param a The first value.
 * @param b The second value.
 * @param t The interpolation value between a and b.
 * @return The interpolated value.
 */
#define dsVector4_lerp(result, a, b, t) \
	do \
	{ \
		(result).values[0] = dsLerp((a).values[0], (b).values[0], t); \
		(result).values[1] = dsLerp((a).values[1], (b).values[1], t); \
		(result).values[2] = dsLerp((a).values[2], (b).values[2], t); \
		(result).values[3] = dsLerp((a).values[3], (b).values[3], t); \
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
 * @brief Gets whether or not two vectors are equal.
 * @param a The first vector.
 * @param b The second vector.
 * @return True if a == b.
 */
#define dsVector4_equal(a, b) \
	((a).values[0] == (b).values[0] && \
	 (a).values[1] == (b).values[1] && \
	 (a).values[2] == (b).values[2] && \
	 (a).values[3] == (b).values[3])

/**
 * @brief Gets the length of a vector.
 * @param a The first vector.
 * @return The length.
 */
DS_MATH_EXPORT inline float dsVector4f_len(const dsVector4f* a);

/** @copydoc dsVector4f_len() */
DS_MATH_EXPORT inline double dsVector4d_len(const dsVector4d* a);

/** @copydoc dsVector4f_len() */
DS_MATH_EXPORT inline double dsVector4i_len(const dsVector4i* a);

/**
 * @brief Gets the distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The length.
 */
DS_MATH_EXPORT inline float dsVector4f_dist(const dsVector4f* a, const dsVector4f* b);

/** @copydoc dsVector4f_dist() */
DS_MATH_EXPORT inline double dsVector4d_dist(const dsVector4d* a, const dsVector4d* b);

/** @copydoc dsVector4f_dist() */
DS_MATH_EXPORT inline double dsVector4i_dist(const dsVector4i* a, const dsVector4i* b);

/**
 * @brief Normalizes a vector.
 * @param[out] result The normalized vector.
 * @param a The vector to normalize.
 */
DS_MATH_EXPORT inline void dsVector4f_normalize(dsVector4f* result, const dsVector4f* a);

/** @copydoc dsVector4f_normalize() */
DS_MATH_EXPORT inline void dsVector4d_normalize(dsVector4d* result, const dsVector4d* a);

/**
 * @brief Checks to see if two values are equal within an epsilon.
 * @param a The first value.
 * @param b The second value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of a and b are within epsilon.
 */
DS_MATH_EXPORT inline bool dsVector4f_epsilonEqual(const dsVector4f* a, const dsVector4f* b,
	float epsilon);

/** @copydoc dsVector4f_epsilonEqual() */
DS_MATH_EXPORT inline bool dsVector4d_epsilonEqual(const dsVector4d* a, const dsVector4d* b,
	double epsilon);

/** @copydoc dsVector4_add() */
DS_MATH_EXPORT inline void dsVector4f_add(dsVector4f* result, const dsVector4f* a,
	const dsVector4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_add(*result, *a, *b);
}

/** @copydoc dsVector4_add() */
DS_MATH_EXPORT inline void dsVector4d_add(dsVector4d* result, const dsVector4d* a,
	const dsVector4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_add(*result, *a, *b);
}

/** @copydoc dsVector4_add() */
DS_MATH_EXPORT inline void dsVector4i_add(dsVector4i* result, const dsVector4i* a,
	const dsVector4i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_add(*result, *a, *b);
}

/** @copydoc dsVector4_sub() */
DS_MATH_EXPORT inline void dsVector4f_sub(dsVector4f* result, const dsVector4f* a,
	const dsVector4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_sub(*result, *a, *b);
}

/** @copydoc dsVector4_sub() */
DS_MATH_EXPORT inline void dsVector4d_sub(dsVector4d* result, const dsVector4d* a,
	const dsVector4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_sub(*result, *a, *b);
}

/** @copydoc dsVector4_sub() */
DS_MATH_EXPORT inline void dsVector4i_sub(dsVector4i* result, const dsVector4i* a,
	const dsVector4i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_sub(*result, *a, *b);
}

/** @copydoc dsVector4_mul() */
DS_MATH_EXPORT inline void dsVector4f_mul(dsVector4f* result, const dsVector4f* a,
	const dsVector4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_mul(*result, *a, *b);
}

/** @copydoc dsVector4_mul() */
DS_MATH_EXPORT inline void dsVector4d_mul(dsVector4d* result, const dsVector4d* a,
	const dsVector4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_mul(*result, *a, *b);
}

/** @copydoc dsVector4_mul() */
DS_MATH_EXPORT inline void dsVector4i_mul(dsVector4i* result, const dsVector4i* a,
	const dsVector4i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_mul(*result, *a, *b);
}

/** @copydoc dsVector4_div() */
DS_MATH_EXPORT inline void dsVector4f_div(dsVector4f* result, const dsVector4f* a,
	const dsVector4f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_div(*result, *a, *b);
}

/** @copydoc dsVector4_div() */
DS_MATH_EXPORT inline void dsVector4d_div(dsVector4d* result, const dsVector4d* a,
	const dsVector4d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_div(*result, *a, *b);
}

/** @copydoc dsVector4_div() */
DS_MATH_EXPORT inline void dsVector4i_div(dsVector4i* result, const dsVector4i* a,
	const dsVector4i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_div(*result, *a, *b);
}

/** @copydoc dsVector4_scale() */
DS_MATH_EXPORT inline void dsVector4f_scale(dsVector4f* result, const dsVector4f* a, float s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_scale(*result, *a, s);
}

/** @copydoc dsVector4_scale() */
DS_MATH_EXPORT inline void dsVector4d_scale(dsVector4d* result, const dsVector4d* a, double s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_scale(*result, *a, s);
}

/** @copydoc dsVector4_scale() */
DS_MATH_EXPORT inline void dsVector4i_scale(dsVector4i* result, const dsVector4i* a, int s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_scale(*result, *a, s);
}

/** @copydoc dsVector4_neg() */
DS_MATH_EXPORT inline void dsVector4f_neg(dsVector4f* result, const dsVector4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_neg(*result, *a);
}

/** @copydoc dsVector4_neg() */
DS_MATH_EXPORT inline void dsVector4d_neg(dsVector4d* result, const dsVector4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_neg(*result, *a);
}

/** @copydoc dsVector4_neg() */
DS_MATH_EXPORT inline void dsVector4i_neg(dsVector4i* result, const dsVector4i* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector4_neg(*result, *a);
}

/** @copydoc dsVector4_lerp() */
DS_MATH_EXPORT inline void dsVector4f_lerp(dsVector4f* result, const dsVector4f* a,
	const dsVector4f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_lerp(*result, *a, *b, t);
}

/** @copydoc dsVector4_lerp() */
DS_MATH_EXPORT inline void dsVector4d_lerp(dsVector4d* result, const dsVector4d* a,
	const dsVector4d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector4_lerp(*result, *a, *b, t);
}

/** @copydoc dsVector4_lerp() */
DS_MATH_EXPORT inline void dsVector4i_lerp(dsVector4i* result, const dsVector4i* a,
	const dsVector4i* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	result->values[0] = (int)dsLerp((float)a->values[0], (float)b->values[0], t);
	result->values[1] = (int)dsLerp((float)a->values[1], (float)b->values[1], t);
	result->values[2] = (int)dsLerp((float)a->values[2], (float)b->values[2], t);
	result->values[3] = (int)dsLerp((float)a->values[3], (float)b->values[3], t);
}

/** @copydoc dsVector4_dot() */
DS_MATH_EXPORT inline float dsVector4f_dot(const dsVector4f* a, const dsVector4f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dot(*a, *b);
}

/** @copydoc dsVector4_dot() */
DS_MATH_EXPORT inline double dsVector4d_dot(const dsVector4d* a, const dsVector4d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dot(*a, *b);
}

/** @copydoc dsVector4_dot() */
DS_MATH_EXPORT inline int dsVector4i_dot(const dsVector4i* a, const dsVector4i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dot(*a, *b);
}

/** @copydoc dsVector4_len2() */
DS_MATH_EXPORT inline float dsVector4f_len2(const dsVector4f* a)
{
	DS_ASSERT(a);
	return dsVector4_len2(*a);
}

/** @copydoc dsVector4_len2() */
DS_MATH_EXPORT inline double dsVector4d_len2(const dsVector4d* a)
{
	DS_ASSERT(a);
	return dsVector4_len2(*a);
}

/** @copydoc dsVector4_len2() */
DS_MATH_EXPORT inline int dsVector4i_len2(const dsVector4i* a)
{
	DS_ASSERT(a);
	return dsVector4_len2(*a);
}

/** @copydoc dsVector4_dist2() */
DS_MATH_EXPORT inline float dsVector4f_dist2(const dsVector4f* a, const dsVector4f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dist2(*a, *b);
}

/** @copydoc dsVector4_dist2() */
DS_MATH_EXPORT inline double dsVector4d_dist2(const dsVector4d* a, const dsVector4d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dist2(*a, *b);
}

/** @copydoc dsVector4_dist2() */
DS_MATH_EXPORT inline int dsVector4i_dist2(const dsVector4i* a, const dsVector4i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_dist2(*a, *b);
}

/** @copydoc dsVector4_equal() */
DS_MATH_EXPORT inline bool dsVector4f_equal(const dsVector4f* a, const dsVector4f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_equal(*a, *b);
}

/** @copydoc dsVector4_equal() */
DS_MATH_EXPORT inline bool dsVector4d_equal(const dsVector4d* a, const dsVector4d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_equal(*a, *b);
}

/** @copydoc dsVector4_equal() */
DS_MATH_EXPORT inline bool dsVector4i_equal(const dsVector4i* a, const dsVector4i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector4_equal(*a, *b);
}

inline float dsVector4f_len(const dsVector4f* a)
{
	DS_ASSERT(a);
	return sqrtf(dsVector4_len2(*a));
}

inline double dsVector4d_len(const dsVector4d* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector4_len2(*a));
}

inline double dsVector4i_len(const dsVector4i* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector4_len2(*a));
}

inline float dsVector4f_dist(const dsVector4f* a, const dsVector4f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrtf(dsVector4_dist2(*a, *b));
}

inline double dsVector4d_dist(const dsVector4d* a, const dsVector4d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector4_dist2(*a, *b));
}

inline double dsVector4i_dist(const dsVector4i* a, const dsVector4i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector4_dist2(*a, *b));
}

inline void dsVector4f_normalize(dsVector4f* result, const dsVector4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	float length = dsVector4f_len(a);
	DS_ASSERT(length > 0);
	dsVector4_scale(*result, *a, 1/length);
}

inline void dsVector4d_normalize(dsVector4d* result, const dsVector4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	double length = dsVector4d_len(a);
	DS_ASSERT(length > 0);
	dsVector4_scale(*result, *a, 1/length);
}

inline bool dsVector4f_epsilonEqual(const dsVector4f* a, const dsVector4f* b, float epsilon)
{
	return dsEpsilonEqualf(a->values[0], b->values[0], epsilon) &&
		dsEpsilonEqualf(a->values[1], b->values[1], epsilon) &&
		dsEpsilonEqualf(a->values[2], b->values[2], epsilon) &&
		dsEpsilonEqualf(a->values[3], b->values[3], epsilon);
}

inline bool dsVector4d_epsilonEqual(const dsVector4d* a, const dsVector4d* b, double epsilon)
{
	return dsEpsilonEquald(a->values[0], b->values[0], epsilon) &&
		dsEpsilonEquald(a->values[1], b->values[1], epsilon) &&
		dsEpsilonEquald(a->values[2], b->values[2], epsilon) &&
		dsEpsilonEquald(a->values[3], b->values[3], epsilon);
}

#ifdef __cplusplus
}
#endif
