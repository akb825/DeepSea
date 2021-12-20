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

#ifdef __cplusplus
}
#endif
