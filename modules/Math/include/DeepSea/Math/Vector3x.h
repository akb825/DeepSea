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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/SIMD/Dot.h>
#include <DeepSea/Math/SIMD/SIMD.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Sqrt.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Math/Vector3.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating 3-element vectors aligned for SIMD usage.
 *
 * A dsVector4* value may be used with these functions, or dsVector3x* type alias for readability.
 *
 * This allows for SIMD operations when supported, allowing for faster processing in the vast
 * majority of situations. The value of the last element is undefined for any results. Operations
 * are only provided for floating-point types, as integer types are more limited in the availability
 * of operations.
 *
 * @see dsVector3xf dsVector3xd
 */

/** @copydoc dsVector3f_add() */
DS_MATH_EXPORT inline void dsVector3xf_add(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_add(a->simd, b->simd);
#else
	dsVector3_add(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_add() */
DS_MATH_EXPORT inline void dsVector3xd_add(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_add(a->simd2[0], b->simd2[0]);
	result->simd2[1] = dsSIMD2d_add(a->simd2[1], b->simd2[1]);
#else
	dsVector3_add(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_sub() */
DS_MATH_EXPORT inline void dsVector3xf_sub(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_sub(a->simd, b->simd);
#else
	dsVector3_sub(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_sub() */
DS_MATH_EXPORT inline void dsVector3xd_sub(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_sub(a->simd2[0], b->simd2[0]);
	result->simd2[1] = dsSIMD2d_sub(a->simd2[1], b->simd2[1]);
#else
	dsVector3_sub(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_mul() */
DS_MATH_EXPORT inline void dsVector3xf_mul(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_mul(a->simd, b->simd);
#else
	dsVector3_mul(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_mul() */
DS_MATH_EXPORT inline void dsVector3xd_mul(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_mul(a->simd2[0], b->simd2[0]);
	result->simd2[1] = dsSIMD2d_mul(a->simd2[1], b->simd2[1]);
#else
	dsVector3_mul(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_div() */
DS_MATH_EXPORT inline void dsVector3xf_div(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_div(a->simd, b->simd);
#else
	dsVector3_div(*result, *a, *b);
#endif
}

/** @copydoc dsVector3f_div() */
DS_MATH_EXPORT inline void dsVector3xd_div(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_div(a->simd2[0], b->simd2[0]);
	result->simd2[1] = dsSIMD2d_div(a->simd2[1], b->simd2[1]);
#else
	dsVector3_div(*result, *a, *b);
#endif
}

/** @copydoc dsVector3_scale() */
DS_MATH_EXPORT inline void dsVector3xf_scale(dsVector3xf* result, const dsVector3xf* a, float s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_mul(a->simd, dsSIMD4f_set1(s));
#else
	dsVector3_scale(*result, *a, s);
#endif
}

/** @copydoc dsVector3_scale() */
DS_MATH_EXPORT inline void dsVector3xd_scale(dsVector3xd* result, const dsVector3xd* a, double s)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d s2 = dsSIMD2d_set1(s);
	result->simd2[0] = dsSIMD2d_mul(a->simd2[0], s2);
	result->simd2[1] = dsSIMD2d_mul(a->simd2[1], s2);
#else
	dsVector3_scale(*result, *a, s);
#endif
}

/** @copydoc dsVector3_neg() */
DS_MATH_EXPORT inline void dsVector3xf_neg(dsVector3xf* result, const dsVector3xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_neg(a->simd);
#else
	dsVector3_neg(*result, *a);
#endif
}

/** @copydoc dsVector3_neg() */
DS_MATH_EXPORT inline void dsVector3xd_neg(dsVector3xd* result, const dsVector3xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	result->simd2[0] = dsSIMD2d_neg(a->simd2[0]);
	result->simd2[1] = dsSIMD2d_neg(a->simd2[1]);
#else
	dsVector3_neg(*result, *a);
#endif
}

/** @copydoc dsVector3_lerp() */
DS_MATH_EXPORT inline void dsVector3xf_lerp(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4f_fmadd(dsSIMD4f_set1(t), dsSIMD4f_sub(b->simd, a->simd), a->simd);
#elif DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_add(
		a->simd, dsSIMD4f_mul(dsSIMD4f_set1(t), dsSIMD4f_sub(b->simd, a->simd)));
#else
	dsVector3_lerp(*result, *a, *b, t);
#endif
}

/** @copydoc dsVector3_lerp() */
DS_MATH_EXPORT inline void dsVector3xd_lerp(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d t2 = dsSIMD2d_set1(t);
#if DS_SIMD_ALWAYS_FMA
	result->simd2[0] = dsSIMD2d_fmadd(t2, dsSIMD2d_sub(b->simd2[0], a->simd2[0]), a->simd2[0]);
	result->simd2[1] = dsSIMD2d_fmadd(t2, dsSIMD2d_sub(b->simd2[1], a->simd2[1]), a->simd2[1]);
#else
	result->simd2[0] = dsSIMD2d_add(
		a->simd2[0], dsSIMD2d_mul(t2, dsSIMD2d_sub(b->simd2[0], a->simd2[0])));
	result->simd2[1] = dsSIMD2d_add(
		a->simd2[1], dsSIMD2d_mul(t2, dsSIMD2d_sub(b->simd2[1], a->simd2[1])));
#endif
#else
	dsVector3_lerp(*result, *a, *b, t);
#endif
}

/** @copydoc dsVector3_dot() */
DS_MATH_EXPORT inline float dsVector3xf_dot(const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	return dsSIMD4f_get(dsDot3SIMD4f(a->simd, b->simd), 0);
#else
	return dsVector3_dot(*a, *b);
#endif
}

/** @copydoc dsVector3_dot() */
DS_MATH_EXPORT inline double dsVector3xd_dot(const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	return dsSIMD2d_get(dsDot3SIMD2d(a->simd2[0], a->simd2[1], b->simd2[0], b->simd2[1]), 0);
#else
	return dsVector3_dot(*a, *b);
#endif
}

/** @copydoc dsVector3_cross() */
DS_MATH_EXPORT inline void dsVector3xf_cross(
	dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4 && DS_X86
	dsSIMD4f a120 = _mm_shuffle_ps(a->simd, a->simd, _MM_SHUFFLE(3, 0, 2, 1));
	dsSIMD4f a201 = _mm_shuffle_ps(a->simd, a->simd, _MM_SHUFFLE(3, 1, 0, 2));
	dsSIMD4f b120 = _mm_shuffle_ps(b->simd, b->simd, _MM_SHUFFLE(3, 0, 2, 1));
	dsSIMD4f b201 = _mm_shuffle_ps(b->simd, b->simd, _MM_SHUFFLE(3, 1, 0, 2));
#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4f_fmsub(a120, b201, dsSIMD4f_mul(a201, b120));
#else
	result->simd = dsSIMD4f_sub(dsSIMD4f_mul(a120, b201), dsSIMD4f_mul(a201, b120));
#endif
#else
	dsVector3_cross(*result, *a, *b);
#endif
}

/** @copydoc dsVector3_cross() */
DS_MATH_EXPORT inline void dsVector3xd_cross(
	dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2 && DS_X86
	dsSIMD2d a12 = _mm_shuffle_pd(a->simd2[0], a->simd2[1], _MM_SHUFFLE2(0, 1));
	dsSIMD2d a0 = a->simd2[0];
	dsSIMD2d a20 = _mm_shuffle_pd(a->simd2[1], a->simd2[0], _MM_SHUFFLE2(0, 0));
	dsSIMD2d a1 =  _mm_shuffle_pd(a->simd2[0], a->simd2[0], _MM_SHUFFLE2(0, 1));
	dsSIMD2d b12 = _mm_shuffle_pd(b->simd2[0], b->simd2[1], _MM_SHUFFLE2(0, 1));
	dsSIMD2d b0 = b->simd2[0];
	dsSIMD2d b20 = _mm_shuffle_pd(b->simd2[1], b->simd2[0], _MM_SHUFFLE2(0, 0));
	dsSIMD2d b1 =  _mm_shuffle_pd(b->simd2[0], b->simd2[0], _MM_SHUFFLE2(0, 1));
#if DS_SIMD_ALWAYS_FMA
	result->simd2[0] = dsSIMD2d_fmsub(a12, b20, dsSIMD2d_mul(a20, b12));
	result->simd2[1] = dsSIMD2d_fmsub(a0, b1, dsSIMD2d_mul(a1, b0));
#else
	result->simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(a12, b20), dsSIMD2d_mul(a20, b12));
	result->simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(a0, b1), dsSIMD2d_mul(a1, b0));
#endif
#else
	dsVector3_cross(*result, *a, *b);
#endif
}

/** @copydoc dsVector3_len2() */
DS_MATH_EXPORT inline float dsVector3xf_len2(const dsVector3xf* a)
{
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	return dsSIMD4f_get(dsDot3SIMD4f(a->simd, a->simd), 0);
#else
	return dsVector3_len2(*a);
#endif
}

/** @copydoc dsVector3_len2() */
DS_MATH_EXPORT inline double dsVector3xd_len2(const dsVector3xd* a)
{
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	return dsSIMD2d_get(dsDot3SIMD2d(a->simd2[0], a->simd2[1], a->simd2[0], a->simd2[1]), 0);
#else
	return dsVector3_len2(*a);
#endif
}

/** @copydoc dsVector3_dist2() */
DS_MATH_EXPORT inline float dsVector3xf_dist2(const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f diff = dsSIMD4f_sub(a->simd, b->simd);
	return dsSIMD4f_get(dsDot3SIMD4f(diff, diff), 0);
#else
	return dsVector3_dist2(*a, *b);
#endif
}

/** @copydoc dsVector3_dist2() */
DS_MATH_EXPORT inline double dsVector3xd_dist2(const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d diff0 = dsSIMD2d_sub(a->simd2[0], b->simd2[0]);
	dsSIMD2d diff1 = dsSIMD2d_sub(a->simd2[0], b->simd2[0]);
	return dsSIMD2d_get(dsDot3SIMD2d(diff0, diff1, diff0, diff1), 0);
#else
	return dsVector3_dist2(*a, *b);
#endif
}

/** @copydoc dsVector3_equal() */
DS_MATH_EXPORT inline bool dsVector3xf_equal(const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_equal(*a, *b);
}

/** @copydoc dsVector3_equal() */
DS_MATH_EXPORT inline bool dsVector3xd_equal(const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
	return dsVector3_equal(*a, *b);
}

/** @copydoc dsVector3f_len() */
DS_MATH_EXPORT inline float dsVector3xf_len(const dsVector3xf* a)
{
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f len2 = dsDot3SIMD4f(a->simd, a->simd);
#if DS_SIMD_EMULATED_DIV_SQRT
	return dsSqrtf(dsSIMD4f_get(len2, 0));
#else
	return dsSIMD4f_get(dsSIMD4f_sqrt(len2), 0);
#endif
#else
	return dsSqrtf(dsVector3_len2(*a));
#endif
}

/** @copydoc dsVector3f_len() */
DS_MATH_EXPORT inline double dsVector3xd_len(const dsVector3xd* a)
{
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d len2 = dsDot3SIMD2d(a->simd2[0], a->simd2[1], a->simd2[0], a->simd2[1]);
#if DS_SIMD_EMULATED_DIV_SQRT
	return dsSqrtd(dsSIMD2d_get(len2, 0));
#else
	return dsSIMD2d_get(dsSIMD2d_sqrt(len2), 0);
#endif
#else
	return dsSqrtd(dsVector3_len2(*a));
#endif
}

/** @copydoc dsVector3f_dist() */
DS_MATH_EXPORT inline float dsVector3xf_dist(const dsVector3xf* a, const dsVector3xf* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f diff = dsSIMD4f_sub(a->simd, b->simd);
	dsSIMD4f dist2 = dsDot3SIMD4f(diff, diff);
#if DS_SIMD_EMULATED_DIV_SQRT
	return dsSqrtf(dsSIMD4f_get(dist2, 0));
#else
	return dsSIMD4f_get(dsSIMD4f_sqrt(dist2), 0);
#endif
#else
	return dsSqrtf(dsVector3_dist2(*a, *b));
#endif
}

/** @copydoc dsVector3f_dist() */
DS_MATH_EXPORT inline double dsVector3xd_dist(const dsVector3xd* a, const dsVector3xd* b)
{
	DS_ASSERT(a);
	DS_ASSERT(b);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD2d diff0 = dsSIMD2d_sub(a->simd2[0], b->simd2[0]);
	dsSIMD2d diff1 = dsSIMD2d_sub(a->simd2[1], b->simd2[1]);
	dsSIMD2d dist2 = dsDot3SIMD2d(diff0, diff1, diff0, diff1);
#if DS_SIMD_EMULATED_DIV_SQRT
	return dsSqrtd(dsSIMD2d_get(dist2, 0));
#else
	return dsSIMD2d_get(dsSIMD2d_sqrt(dist2), 0);
#endif
#else
	return dsSqrtd(dsVector3_dist2(*a, *b));
#endif
}

/** @copydoc dsVector3f_normalize() */
DS_MATH_EXPORT inline void dsVector3xf_normalize(dsVector3xf* result, const dsVector3xf* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f len2 = dsDot3SIMD4f(a->simd, a->simd);
#if DS_SIMD_EMULATED_DIV_SQRT
	dsSIMD4f invLen = dsSIMD4f_set1(1/dsSqrtf(dsSIMD4f_get(len2, 0)));
#else
	dsSIMD4f invLen = dsSIMD4f_rsqrt(len2);
#endif
	result->simd = dsSIMD4f_mul(a->simd, invLen);
#else
	float length = dsVector3xf_len(a);
	DS_ASSERT(length > 0);
	dsVector3_scale(*result, *a, 1/length);
#endif
}

/** @copydoc dsVector3f_normalize() */
DS_MATH_EXPORT inline void dsVector3xd_normalize(dsVector3xd* result, const dsVector3xd* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d len2 = dsDot3SIMD2d(a->simd2[0], a->simd2[1], a->simd2[0], a->simd2[1]);
#if DS_SIMD_EMULATED_DIV_SQRT
	dsSIMD2d invLen = dsSIMD2d_set1(1/dsSqrtd(dsSIMD2d_get(len2, 0)));
#else
	dsSIMD2d invLen = dsSIMD2d_rsqrt(len2);
#endif
	result->simd2[0] = dsSIMD2d_mul(a->simd2[0], invLen);
	result->simd2[1] = dsSIMD2d_mul(a->simd2[1], invLen);
#else
	double length = dsVector3xd_len(a);
	DS_ASSERT(length > 0);
	dsVector3_scale(*result, *a, 1/length);
#endif
}

/** @copydoc dsVector3f_epsilonEqual() */
DS_MATH_EXPORT inline bool dsVector3xf_epsilonEqual(
	const dsVector3xf* a, const dsVector3xf* b, float epsilon)
{
#if DS_SIMD_ALWAYS_FLOAT4
	dsVector4i result;
	result.simd = dsSIMD4f_cmple(
		dsSIMD4f_abs(dsSIMD4f_sub(a->simd, b->simd)), dsSIMD4f_set1(epsilon));
	return result.x && result.y && result.z;
#else
	return dsVector3f_epsilonEqual((const dsVector3f*)a, (const dsVector3f*)b, epsilon);
#endif
}

/** @copydoc dsVector3f_epsilonEqual() */
DS_MATH_EXPORT inline bool dsVector3xd_epsilonEqual(
	const dsVector3xd* a, const dsVector3xd* b, double epsilon)
{
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d epsilon2 = dsSIMD2d_set1(epsilon);
	dsVector4l result;
	result.simd2[0] = dsSIMD2d_cmple(
		dsSIMD2d_abs(dsSIMD2d_sub(a->simd2[0], b->simd2[0])), epsilon2);
	result.simd2[1] = dsSIMD2d_cmple(
		dsSIMD2d_abs(dsSIMD2d_sub(a->simd2[1], b->simd2[1])), epsilon2);
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsVector3d_epsilonEqual((const dsVector3d*)a, (const dsVector3d*)b, epsilon);
#endif
}

/** @copydoc dsVector3f_relativeEpsilonEqual() */
DS_MATH_EXPORT inline bool dsVector3xf_relativeEpsilonEqual(
	const dsVector3xf* a, const dsVector3xf* b, float absoluteEps, float relativeEps)
{
#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f diff = dsSIMD4f_abs(dsSIMD4f_sub(a->simd, b->simd));
	dsSIMD4fb epsEqual = dsSIMD4f_cmple(diff, dsSIMD4f_set1(absoluteEps));
	dsSIMD4fb relativeEqual = dsSIMD4f_cmple(diff,
		dsSIMD4f_mul(dsSIMD4f_max(dsSIMD4f_abs(a->simd), dsSIMD4f_abs(b->simd)),
		dsSIMD4f_set1(relativeEps)));

	dsVector4i result;
	result.simd = dsSIMD4fb_or(epsEqual, relativeEqual);
	return result.x && result.y && result.z;
#else
	return dsVector3f_relativeEpsilonEqual(
		(const dsVector3f*)a, (const dsVector3f*)b, absoluteEps, relativeEps);
#endif
}

/** @copydoc dsVector3f_relativeEpsilonEqual() */
DS_MATH_EXPORT inline bool dsVector3xd_relativeEpsilonEqual(
	const dsVector3xd* a, const dsVector3xd* b, double absoluteEps, double relativeEps)
{
#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector3xd diff;
	diff.simd2[0] = dsSIMD2d_abs(dsSIMD2d_sub(a->simd2[0], b->simd2[0]));
	diff.simd2[1] = dsSIMD2d_abs(dsSIMD2d_sub(a->simd2[1], b->simd2[1]));
	dsSIMD2d absoluteEps2 = dsSIMD2d_set1(absoluteEps);
	dsSIMD2d relativeEps2 = dsSIMD2d_set1(relativeEps);

	dsVector4l epsEqual;
	epsEqual.simd2[0] = dsSIMD2d_cmple(diff.simd2[0], absoluteEps2);
	epsEqual.simd2[1] = dsSIMD2d_cmple(diff.simd2[1], absoluteEps2);

	dsVector4l relativeEqual;
	relativeEqual.simd2[0] = dsSIMD2d_cmple(diff.simd2[0],
		dsSIMD2d_mul(dsSIMD2d_max(dsSIMD2d_abs(a->simd2[0]), dsSIMD2d_abs(b->simd2[0])),
			relativeEps2));
	relativeEqual.simd2[1] = dsSIMD2d_cmple(diff.simd2[1],
		dsSIMD2d_mul(dsSIMD2d_max(dsSIMD2d_abs(a->simd2[1]), dsSIMD2d_abs(b->simd2[1])),
			relativeEps2));

	dsVector4l result;
	result.simd2[0] = dsSIMD2db_or(epsEqual.simd2[0], relativeEqual.simd2[0]);
	result.simd2[1] = dsSIMD2db_or(epsEqual.simd2[1], relativeEqual.simd2[1]);
	return dsSIMD2db_all(result.simd2[0]) && result.z;
#else
	return dsVector3d_relativeEpsilonEqual(
		(const dsVector3d*)a, (const dsVector3d*)b, absoluteEps, relativeEps);
#endif
}

#ifdef __cplusplus
}
#endif
