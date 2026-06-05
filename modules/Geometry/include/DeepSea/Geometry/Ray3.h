/*
 * Copyright 2021 Aaron Barany
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
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with rays.
 */

/**
 * @brief Evaluates a ray.
 * @param[out] result The resulting output position.
 * @param ray The ray to evaluate.
 * @param t The T value to move along the direction of the ray.
 */
#define dsRay3_evaluate(result, ray, t) \
	do \
	{ \
		(result).values[0] = (ray).origin.values[0] + (ray).direction.values[0]*(t); \
		(result).values[1] = (ray).origin.values[1] + (ray).direction.values[1]*(t); \
		(result).values[2] = (ray).origin.values[2] + (ray).direction.values[2]*(t); \
	} while (0)

/** @copydoc dsRay3_evaluate() */
DS_GEOMETRY_EXPORT inline void dsRay3f_evaluate(dsVector3f* result, const dsRay3f* ray, float t)
{
	dsRay3_evaluate(*result, *ray, t);
}

/** @copydoc dsRay3_evaluate() */
DS_GEOMETRY_EXPORT inline void dsRay3d_evaluate(dsVector3d* result, const dsRay3d* ray, double t)
{
	dsRay3_evaluate(*result, *ray, t);
}

/** @copydoc dsRay3_evaluate() */
DS_GEOMETRY_EXPORT inline void dsRay3f_evaluate3x(dsVector3xf* result, const dsRay3f* ray, float t)
{
#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4f_fmadd(ray->direction.simd, dsSIMD4f_set1(t), ray->origin.simd);
#elif DS_SIMD_ALWAYS_FLOAT4
	result->simd = dsSIMD4f_add(
		ray->origin.simd, dsSIMD4f_mul(ray->direction.simd, dsSIMD4f_set1(t)));
#else
	dsRay3_evaluate(*result, *ray, t);
#endif
}

/** @copydoc dsRay3_evaluate() */
DS_GEOMETRY_EXPORT inline void dsRay3d_evaluate3x(dsVector3xd* result, const dsRay3d* ray, double t)
{
#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d t2 = dsSIMD2d_set1(t);
#if DS_SIMD_ALWAYS_FMA
	result->simd2[0] = dsSIMD2d_fmadd(ray->direction.simd2[0], t2, ray->origin.simd2[0]);
	result->simd2[1] = dsSIMD2d_fmadd(ray->direction.simd2[1], t2, ray->origin.simd2[1]);
#else
	result->simd2[0] = dsSIMD2d_add(
		ray->origin.simd2[0], dsSIMD2d_mul(ray->direction.simd2[0], t2));
	result->simd2[1] = dsSIMD2d_add(
		ray->origin.simd2[1], dsSIMD2d_mul(ray->direction.simd2[1], t2));
#endif
#else
	dsRay3_evaluate(*result, *ray, t);
#endif
}

#ifdef __cplusplus
}
#endif
