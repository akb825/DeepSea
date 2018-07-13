/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and sampling Bezier curves.
 * @see dsBezierCurve
 */

/**
 * @brief Initializes a Bezier curve with the control points.
 * @remark errno will be set on failure.
 * @param[out] curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p1 The second control point. This must be dsVector2d or dsVector3d depending on
 *     axisCount.
 * @param p2 The third control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p3 The fourth control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsBezierCurve_initialize(dsBezierCurve* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3);

/**
 * @brief Initializes a quadratic Bezier curve with the control points.
 * @remark errno will be set on failure.
 * @param curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p1 The second control point. This must be dsVector2d or dsVector3d depending on
 *     axisCount.
 * @param p2 The third control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsBezierCurve_initializeQuadratic(dsBezierCurve* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2);

/**
 * @brief Evaluates the position of a curve.
 * @remark errno will be set on failure.
 * @param[out] outPoint The evaluated position. This must be dsVector2d or dsVector3d depending on
 *     the axis count.
 * @param curve The curve to evaluate.
 * @param t The parametric position on the curve to evaluate. This must be in the range [0, 1].
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsBezierCurve_evaluate(void* outPoint, const dsBezierCurve* curve,
	double t);

/**
 * @brief Evaluates the tangent of a curve.
 * @remark errno will be set on failure.
 * @param[out] outTangent The evaluated tangent. This must be dsVector2d or dsVector3d depending on
 *     the axis count.
 * @param curve The curve to evaluate.
 * @param t The parametric position on the curve to evaluate. This must be in the range [0, 1].
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsBezierCurve_evaluateTangent(void* outTangent, const dsBezierCurve* curve,
	double t);

/**
 * @brief Tessellates a bezier curve.
 * @remark errno will be set on failure.
 * @param curve The curve to tessellate.
 * @param chordalTolerance The maximum distance between the centerpoint of a line and the curve to
 *     stop subdivision.
 * @param maxRecursions The maximum number of times to recurse. This will have a total of
 *     pow(2, maxRecursions) points. This may not be larger than DS_MAX_CURVE_RECURSIONS.
 * @param sampleFunc The function to process the samples.
 * @param userData The user data to provide to sampleFunc.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsBezierCurve_tessellate(const dsBezierCurve* curve,
	double chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunction sampleFunc,
	void* userData);

#ifdef __cplusplus
}
#endif
