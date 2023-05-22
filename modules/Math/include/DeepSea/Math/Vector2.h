/*
 * Copyright 2016-2023 Aaron Barany
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
 * @brief Macros and functions for manipulating dsVector2* structures.
 *
 * The macros are type-agnostic, allowing to be used with any dsVector2* type. All parameters are
 * by value. In all cases, it is safe to have the result be the same as one of the parameters.
 *
 * The functions have different versions for the supported Vector2 types. These are used when the
 * implementation cannot be practically done within a macro. There are also inline functions
 * provided to accompany the macro to use when desired. The inline functions may also be addressed
 * in order to interface with other languages.
 *
 * @see dsVector2f dsVector3d dsVector2i
 */

/**
 * @brief Adds the components of two vectors.
 * @param result The result of a + b.
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector2_add(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] + (b).values[0]; \
		(result).values[1] = (a).values[1] + (b).values[1]; \
	} while (0)

/**
 * @brief Sebutracts the components of two vectors.
 * @param result The result of a - (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector2_sub(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0] - (b).values[0]; \
		(result).values[1] = (a).values[1] - (b).values[1]; \
	} while (0)

/**
 * @brief Multiplies the components of two vectors.
 * @param result The result of a * (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector2_mul(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(b).values[0]; \
		(result).values[1] = (a).values[1]*(b).values[1]; \
	} while (0)

/**
 * @brief Divides the components of two vectors.
 * @param result The result of a / (b).
 * @param a The first vector.
 * @param b The second vector.
 */
#define dsVector2_div(result, a, b) \
	do \
	{ \
		(result).values[0] = (a).values[0]/(b).values[0]; \
		(result).values[1] = (a).values[1]/(b).values[1]; \
	} while (0)

/**
 * @brief Scales a vector.
 * @param result The result of a * s.
 * @param a The vector.
 * @param s The scale factor.
 */
#define dsVector2_scale(result, a, s) \
	do \
	{ \
		(result).values[0] = (a).values[0]*(s); \
		(result).values[1] = (a).values[1]*(s); \
	} while (0)

/**
 * @brief Negates the components of a vector.
 * @param result The result of -a.
 * @param a The vector.
 */
#define dsVector2_neg(result, a) \
	do \
	{ \
		(result).values[0] = -(a).values[0]; \
		(result).values[1] = -(a).values[1]; \
	} while (0)

/**
 * @brief Linearly interpolates between two values.
 * @remark For dsVector2i, it would be better to call dsVector2i_lerp() directly instead instead of
 *     of the macro version.
 * @param[out] result The interpolated result.
 * @param a The first value.
 * @param b The second value.
 * @param t The interpolation value between a and b.
 */
#define dsVector2_lerp(result, a, b, t) \
	do \
	{ \
		(result).values[0] = dsLerp((a).values[0], (b).values[0], t); \
		(result).values[1] = dsLerp((a).values[1], (b).values[1], t); \
	} while (0)

/**
 * @brief Takes the dot product between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of a dot b.
 */
#define dsVector2_dot(a, b) \
	((a).values[0]*(b).values[0] + \
	 (a).values[1]*(b).values[1])

/**
 * @brief Gets the squared length of a vector.
 * @param a The vector.
 * @return The squared length.
 */
#define dsVector2_len2(a) \
	dsVector2_dot((a), (a))

/**
 * @brief Gets the squared distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The squared distance.
 */
#define dsVector2_dist2(a, b) \
	(dsPow2((a).values[0] - (b).values[0]) + \
	 dsPow2((a).values[1] - (b).values[1]))

/**
 * @brief Gets whether or not two vectors are equal.
 * @param a The first vector.
 * @param b The second vector.
 * @return True if a == b.
 */
#define dsVector2_equal(a, b) \
	((a).values[0] == (b).values[0] && \
	 (a).values[1] == (b).values[1])

/**
 * @brief Gets the length of a vector.
 * @param a The first vector.
 * @return The length.
 */
DS_MATH_EXPORT inline float dsVector2f_len(const dsVector2f* a);

/** @copydoc dsVector2f_len() */
DS_MATH_EXPORT inline double dsVector2d_len(const dsVector2d* a);

/** @copydoc dsVector2f_len() */
DS_MATH_EXPORT inline double dsVector2i_len(const dsVector2i* a);

/** @copydoc dsVector2f_len() */
DS_MATH_EXPORT inline double dsVector2l_len(const dsVector2l* a);

/**
 * @brief Gets the distance between two vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The length.
 */
DS_MATH_EXPORT inline float dsVector2f_dist(const dsVector2f* a, const dsVector2f* b);

/** @copydoc dsVector2f_dist() */
DS_MATH_EXPORT inline double dsVector2d_dist(const dsVector2d* a, const dsVector2d* b);

/** @copydoc dsVector2f_dist() */
DS_MATH_EXPORT inline double dsVector2i_dist(const dsVector2i* a, const dsVector2i* b);

/** @copydoc dsVector2f_dist() */
DS_MATH_EXPORT inline double dsVector2l_dist(const dsVector2l* a, const dsVector2l* b);

/**
 * @brief Normalizes a vector.
 * @param[out] result The normalized vector.
 * @param a The vector to normalize.
 */
DS_MATH_EXPORT inline void dsVector2f_normalize(dsVector2f* result, const dsVector2f* a);

/** @copydoc dsVector2f_normalize() */
DS_MATH_EXPORT inline void dsVector2d_normalize(dsVector2d* result, const dsVector2d* a);

/** @copydoc dsVector2_add() */
DS_MATH_EXPORT inline void dsVector2f_add(dsVector2f* result, const dsVector2f* a,
	const dsVector2f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_add(*result, *a, *b);
}

/**
 * @brief Checks to see if two values are equal within an epsilon.
 * @param a The first value.
 * @param b The second value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of a and b are within epsilon.
 */
DS_MATH_EXPORT inline bool dsVector2f_epsilonEqual(const dsVector2f* a, const dsVector2f* b,
	float epsilon);

/** @copydoc dsVector2f_epsilonEqual() */
DS_MATH_EXPORT inline bool dsVector2d_epsilonEqual(const dsVector2d* a, const dsVector2d* b,
	double epsilon);

/**
 * @brief Checks to see if two values are equal within a relative epsilon.
 * @param a The first value.
 * @param b The second value.
 * @param epsilon The epsilon to compare with.
 * @return True the values of a and b are within epsilon.
 */
DS_MATH_EXPORT inline bool dsVector2f_relativeEpsilonEqual(const dsVector2f* a, const dsVector2f* b,
	float epsilon);

/** @copydoc dsVector2f_epsilonEqual() */
DS_MATH_EXPORT inline bool dsVector2d_relativeEpsilonEqual(const dsVector2d* a, const dsVector2d* b,
	double epsilon);

/** @copydoc dsVector2_add() */
DS_MATH_EXPORT inline void dsVector2d_add(dsVector2d* result, const dsVector2d* a,
	const dsVector2d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_add(*result, *a, *b);
}

/** @copydoc dsVector2_add() */
DS_MATH_EXPORT inline void dsVector2i_add(dsVector2i* result, const dsVector2i* a,
	const dsVector2i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_add(*result, *a, *b);
}

/** @copydoc dsVector2_add() */
DS_MATH_EXPORT inline void dsVector2l_add(dsVector2l* result, const dsVector2l* a,
	const dsVector2l* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_add(*result, *a, *b);
}

/** @copydoc dsVector2_sub() */
DS_MATH_EXPORT inline void dsVector2f_sub(dsVector2f* result, const dsVector2f* a,
	const dsVector2f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_sub(*result, *a, *b);
}

/** @copydoc dsVector2_sub() */
DS_MATH_EXPORT inline void dsVector2d_sub(dsVector2d* result, const dsVector2d* a,
	const dsVector2d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_sub(*result, *a, *b);
}

/** @copydoc dsVector2_sub() */
DS_MATH_EXPORT inline void dsVector2i_sub(dsVector2i* result, const dsVector2i* a,
	const dsVector2i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_sub(*result, *a, *b);
}

/** @copydoc dsVector2_sub() */
DS_MATH_EXPORT inline void dsVector2l_sub(dsVector2l* result, const dsVector2l* a,
	const dsVector2l* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_sub(*result, *a, *b);
}

/** @copydoc dsVector2_mul() */
DS_MATH_EXPORT inline void dsVector2f_mul(dsVector2f* result, const dsVector2f* a,
	const dsVector2f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_mul(*result, *a, *b);
}

/** @copydoc dsVector2_mul() */
DS_MATH_EXPORT inline void dsVector2d_mul(dsVector2d* result, const dsVector2d* a,
	const dsVector2d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_mul(*result, *a, *b);
}

/** @copydoc dsVector2_mul() */
DS_MATH_EXPORT inline void dsVector2i_mul(dsVector2i* result, const dsVector2i* a,
	const dsVector2i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_mul(*result, *a, *b);
}

/** @copydoc dsVector2_mul() */
DS_MATH_EXPORT inline void dsVector2l_mul(dsVector2l* result, const dsVector2l* a,
	const dsVector2l* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_mul(*result, *a, *b);
}

/** @copydoc dsVector2_div() */
DS_MATH_EXPORT inline void dsVector2f_div(dsVector2f* result, const dsVector2f* a,
	const dsVector2f* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_div(*result, *a, *b);
}

/** @copydoc dsVector2_div() */
DS_MATH_EXPORT inline void dsVector2d_div(dsVector2d* result, const dsVector2d* a,
	const dsVector2d* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_div(*result, *a, *b);
}

/** @copydoc dsVector2_div() */
DS_MATH_EXPORT inline void dsVector2i_div(dsVector2i* result, const dsVector2i* a,
	const dsVector2i* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_div(*result, *a, *b);
}

/** @copydoc dsVector2_div() */
DS_MATH_EXPORT inline void dsVector2l_div(dsVector2l* result, const dsVector2l* a,
	const dsVector2l* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_div(*result, *a, *b);
}

/** @copydoc dsVector2_scale() */
DS_MATH_EXPORT inline void dsVector2f_scale(dsVector2f* result, const dsVector2f* a, float s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_scale(*result, *a, s);
}

/** @copydoc dsVector2_scale() */
DS_MATH_EXPORT inline void dsVector2d_scale(dsVector2d* result, const dsVector2d* a, double s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_scale(*result, *a, s);
}

/** @copydoc dsVector2_scale() */
DS_MATH_EXPORT inline void dsVector2i_scale(dsVector2i* result, const dsVector2i* a, int s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_scale(*result, *a, s);
}

/** @copydoc dsVector2_scale() */
DS_MATH_EXPORT inline void dsVector2l_scale(dsVector2l* result, const dsVector2l* a, long long s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_scale(*result, *a, s);
}

/** @copydoc dsVector2_neg() */
DS_MATH_EXPORT inline void dsVector2f_neg(dsVector2f* result, const dsVector2f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_neg(*result, *a);
}

/** @copydoc dsVector2_neg() */
DS_MATH_EXPORT inline void dsVector2d_neg(dsVector2d* result, const dsVector2d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_neg(*result, *a);
}

/** @copydoc dsVector2_neg() */
DS_MATH_EXPORT inline void dsVector2i_neg(dsVector2i* result, const dsVector2i* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_neg(*result, *a);
}

/** @copydoc dsVector2_neg() */
DS_MATH_EXPORT inline void dsVector2l_neg(dsVector2l* result, const dsVector2l* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsVector2_neg(*result, *a);
}

/** @copydoc dsVector2_lerp() */
DS_MATH_EXPORT inline void dsVector2f_lerp(dsVector2f* result, const dsVector2f* a,
	const dsVector2f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_lerp(*result, *a, *b, t);
}

/** @copydoc dsVector2_lerp() */
DS_MATH_EXPORT inline void dsVector2d_lerp(dsVector2d* result, const dsVector2d* a,
	const dsVector2d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	dsVector2_lerp(*result, *a, *b, t);
}

/** @copydoc dsVector2_lerp() */
DS_MATH_EXPORT inline void dsVector2i_lerp(dsVector2i* result, const dsVector2i* a,
	const dsVector2i* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	result->values[0] = (int)dsLerp((float)a->values[0], (float)b->values[0], t);
	result->values[1] = (int)dsLerp((float)a->values[1], (float)b->values[1], t);
}

/** @copydoc dsVector2_lerp() */
DS_MATH_EXPORT inline void dsVector2l_lerp(dsVector2l* result, const dsVector2l* a,
	const dsVector2l* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	result->values[0] = (long long)dsLerp((double)a->values[0], (double)b->values[0], t);
	result->values[1] = (long long)dsLerp((double)a->values[1], (double)b->values[1], t);
}

/** @copydoc dsVector2_dot() */
DS_MATH_EXPORT inline float dsVector2f_dot(const dsVector2f* a, const dsVector2f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dot(*a, *b);
}

/** @copydoc dsVector2_dot() */
DS_MATH_EXPORT inline double dsVector2d_dot(const dsVector2d* a, const dsVector2d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dot(*a, *b);
}

/** @copydoc dsVector2_dot() */
DS_MATH_EXPORT inline int dsVector2i_dot(const dsVector2i* a, const dsVector2i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dot(*a, *b);
}

/** @copydoc dsVector2_dot() */
DS_MATH_EXPORT inline long long dsVector2l_dot(const dsVector2l* a, const dsVector2l* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dot(*a, *b);
}

/** @copydoc dsVector2_len2() */
DS_MATH_EXPORT inline float dsVector2f_len2(const dsVector2f* a)
{
	DS_ASSERT(a);
	return dsVector2_len2(*a);
}

/** @copydoc dsVector2_len2() */
DS_MATH_EXPORT inline double dsVector2d_len2(const dsVector2d* a)
{
	DS_ASSERT(a);
	return dsVector2_len2(*a);
}

/** @copydoc dsVector2_len2() */
DS_MATH_EXPORT inline int dsVector2i_len2(const dsVector2i* a)
{
	DS_ASSERT(a);
	return dsVector2_len2(*a);
}

/** @copydoc dsVector2_len2() */
DS_MATH_EXPORT inline long long dsVector2l_len2(const dsVector2l* a)
{
	DS_ASSERT(a);
	return dsVector2_len2(*a);
}

/** @copydoc dsVector2_dist2() */
DS_MATH_EXPORT inline float dsVector2f_dist2(const dsVector2f* a, const dsVector2f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dist2(*a, *b);
}

/** @copydoc dsVector2_dist2() */
DS_MATH_EXPORT inline double dsVector2d_dist2(const dsVector2d* a, const dsVector2d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dist2(*a, *b);
}

/** @copydoc dsVector2_dist2() */
DS_MATH_EXPORT inline int dsVector2i_dist2(const dsVector2i* a, const dsVector2i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dist2(*a, *b);
}

/** @copydoc dsVector2_dist2() */
DS_MATH_EXPORT inline long long dsVector2l_dist2(const dsVector2l* a, const dsVector2l* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_dist2(*a, *b);
}

/** @copydoc dsVector2_equal() */
DS_MATH_EXPORT inline bool dsVector2f_equal(const dsVector2f* a, const dsVector2f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_equal(*a, *b);
}

/** @copydoc dsVector2_equal() */
DS_MATH_EXPORT inline bool dsVector2d_equal(const dsVector2d* a, const dsVector2d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_equal(*a, *b);
}

/** @copydoc dsVector2_equal() */
DS_MATH_EXPORT inline bool dsVector2i_equal(const dsVector2i* a, const dsVector2i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector2_equal(*a, *b);
}

inline float dsVector2f_len(const dsVector2f* a)
{
	DS_ASSERT(a);
	return sqrtf(dsVector2_len2(*a));
}

inline double dsVector2d_len(const dsVector2d* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector2_len2(*a));
}

inline double dsVector2i_len(const dsVector2i* a)
{
	DS_ASSERT(a);
	return sqrt(dsVector2_len2(*a));
}

inline double dsVector2l_len(const dsVector2l* a)
{
	DS_ASSERT(a);
	return sqrt((double)dsVector2_len2(*a));
}

inline float dsVector2f_dist(const dsVector2f* a, const dsVector2f* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrtf(dsVector2_dist2(*a, *b));
}

inline double dsVector2d_dist(const dsVector2d* a, const dsVector2d* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector2_dist2(*a, *b));
}

inline double dsVector2i_dist(const dsVector2i* a, const dsVector2i* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt(dsVector2_dist2(*a, *b));
}

inline double dsVector2l_dist(const dsVector2l* a, const dsVector2l* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return sqrt((double)dsVector2_dist2(*a, *b));
}

inline void dsVector2f_normalize(dsVector2f* result, const dsVector2f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	float length = dsVector2f_len(a);
	DS_ASSERT(length > 0);
	dsVector2_scale(*result, *a, 1/length);
}

inline void dsVector2d_normalize(dsVector2d* result, const dsVector2d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	double length = dsVector2d_len(a);
	DS_ASSERT(length > 0);
	dsVector2_scale(*result, *a, 1/length);
}

inline bool dsVector2f_epsilonEqual(const dsVector2f* a, const dsVector2f* b, float epsilon)
{
	return dsEpsilonEqualf(a->values[0], b->values[0], epsilon) &&
		dsEpsilonEqualf(a->values[1], b->values[1], epsilon);
}

inline bool dsVector2d_epsilonEqual(const dsVector2d* a, const dsVector2d* b, double epsilon)
{
	return dsEpsilonEquald(a->values[0], b->values[0], epsilon) &&
		dsEpsilonEquald(a->values[1], b->values[1], epsilon);
}

inline bool dsVector2f_relativeEpsilonEqual(const dsVector2f* a, const dsVector2f* b, float epsilon)
{
	return dsRelativeEpsilonEqualf(a->values[0], b->values[0], epsilon) &&
		dsRelativeEpsilonEqualf(a->values[1], b->values[1], epsilon);
}

inline bool dsVector2d_relativeEpsilonEqual(const dsVector2d* a, const dsVector2d* b,
	double epsilon)
{
	return dsRelativeEpsilonEquald(a->values[0], b->values[0], epsilon) &&
		dsRelativeEpsilonEquald(a->values[1], b->values[1], epsilon);
}

#ifdef __cplusplus
}
#endif
