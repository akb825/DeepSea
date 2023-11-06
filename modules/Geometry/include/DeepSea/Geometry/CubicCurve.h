/*
 * Copyright 2023 Aaron Barany
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
 * @brief Functions for creating and sampling cubic curves.
 * @see dsBezierCurve
 */

/**
 * @brief Initializes a cubic curve with Bezier control points.
 * @remark errno will be set on failure.
 * @param[out] curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param p1 The second control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param p2 The third control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param p3 The fourth control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurvef_initializeBezier(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3);

/**
 * @brief Initializes a cubic curve with Bezier control points.
 * @remark errno will be set on failure.
 * @param[out] curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p1 The second control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p2 The third control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p3 The fourth control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurved_initializeBezier(dsCubicCurved* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3);

/**
 * @brief Initializes a cubic curve with quadratic Bezier control points.
 * @remark errno will be set on failure.
 * @param curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param p1 The second control point. This must be dsVector2f or dsVector3f depending on
 *     axisCount.
 * @param p2 The third control point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurvef_initializeQuadratic(dsCubicCurvef* curve,
	unsigned int axisCount, const void* p0, const void* p1, const void* p2);

/**
 * @brief Initializes a cubic curve with quadratic Bezier control points.
 * @remark errno will be set on failure.
 * @param curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The first control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param p1 The second control point. This must be dsVector2d or dsVector3d depending on
 *     axisCount.
 * @param p2 The third control point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurved_initializeQuadratic(dsCubicCurved* curve,
	unsigned int axisCount, const void* p0, const void* p1, const void* p2);

/**
 * @brief Initializes a cubic curve Hermite endpoints and tangents.
 * @remark errno will be set on failure.
 * @param[out] curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The start point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param t0 The tangent at the start point. This must be dsVector2f or dsVector3f depending on
 *     axisCount.
 * @param p1 The end point. This must be dsVector2f or dsVector3f depending on axisCount.
 * @param t1 The tangent at the end point. This must be dsVector2f or dsVector3f depending on
 *     axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurvef_initializeHermite(dsCubicCurvef* curve,
	unsigned int axisCount, const void* p0, const void* t0, const void* p1, const void* t1);

/**
 * @brief Initializes a cubic curve Hermite endpoints and tangents.
 * @remark errno will be set on failure.
 * @param[out] curve The curve to initialize.
 * @param axisCount The number of axes. This must be 2 or 3.
 * @param p0 The start point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param t0 The tangent at the start point. This must be dsVector2d or dsVector3d depending on
 *     axisCount.
 * @param p1 The end point. This must be dsVector2d or dsVector3d depending on axisCount.
 * @param t1 The tangent at the end point. This must be dsVector2d or dsVector3d depending on
 *     axisCount.
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurved_initializeHermite(dsCubicCurved* curve,
	unsigned int axisCount, const void* p0, const void* t0, const void* p1, const void* t1);

/**
 * @brief Evaluates the position of a cubic curve.
 * @remark errno will be set on failure.
 * @param[out] outPoint The evaluated position. This must be dsVector2d or dsVector3d depending on
 *     the axis count.
 * @param curve The curve to evaluate.
 * @param t The parametric position on the curve to evaluate. This must be in the range [0, 1].
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurvef_evaluate(void* outPoint, const dsCubicCurvef* curve,
	float t);

/** @copydoc dsCubicCurvef_evaluate() */
DS_GEOMETRY_EXPORT bool dsCubicCurved_evaluate(void* outPoint, const dsCubicCurved* curve,
	double t);

/**
 * @brief Evaluates the tangent of a cubic curve.
 * @remark errno will be set on failure.
 * @param[out] outTangent The evaluated tangent. This must be dsVector2d or dsVector3d depending on
 *     the axis count.
 * @param curve The curve to evaluate.
 * @param t The parametric position on the curve to evaluate. This must be in the range [0, 1].
 * @return False if the parameters are invalid.
 */
DS_GEOMETRY_EXPORT bool dsCubicCurvef_evaluateTangent(void* outTangent,
	const dsCubicCurvef* curve, float t);

/** @copydoc dsCubicCurvef_evaluateTangent() */
DS_GEOMETRY_EXPORT bool dsCubicCurved_evaluateTangent(void* outTangent,
	const dsCubicCurved* curve, double t);

/**
 * @brief Tessellates a cubic curve.
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
DS_GEOMETRY_EXPORT bool dsCubicCurvef_tessellate(const dsCubicCurvef* curve,
	float chordalTolerance, unsigned int maxRecursions, dsCurveSampleFunctionf sampleFunc,
	void* userData);

/** @copydoc dsCubicCurvef_tessellate() */
DS_GEOMETRY_EXPORT bool dsCubicCurved_tessellate(const dsCubicCurved* curve,
	double chordalTolerance, unsigned int maxRecursions, dsCurveSampleFunctiond sampleFunc,
	void* userData);

#ifdef __cplusplus
}
#endif
