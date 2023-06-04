/*
 * Copyright 2018-2023 Aaron Barany
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

#include <DeepSea/Geometry/BezierCurve.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector4.h>

#include <string.h>

// Left and right subdivision matrices from http://algorithmist.net/docs/subdivision.pdf
static const dsMatrix44f leftBezierMatrixf =
{{
	{1.0f, 0.5f, 0.25f, 0.125f},
	{0.0f, 0.5f, 0.5f , 0.375f},
	{0.0f, 0.0f, 0.25f, 0.375f},
	{0.0f, 0.0f, 0.0f , 0.125f}
}};

static const dsMatrix44f rightBezierMatrixf =
{{
	{0.125f, 0.0f , 0.0f, 0.0f},
	{0.375f, 0.25f, 0.0f, 0.0f},
	{0.375f, 0.5f , 0.5f, 0.0f},
	{0.125f, 0.25f, 0.5f, 1.0f}
}};

static const dsVector4f bezierMidf = {{0.125f, 0.375f, 0.375f, 0.125f}};

static const DS_ALIGN(32) dsMatrix44d leftBezierMatrixd =
{{
	{1.0, 0.5, 0.25, 0.125},
	{0.0, 0.5, 0.5 , 0.375},
	{0.0, 0.0, 0.25, 0.375},
	{0.0, 0.0, 0.0 , 0.125}
}};

static const DS_ALIGN(32) dsMatrix44d rightBezierMatrixd =
{{
	{0.125, 0.0 , 0.0, 0.0},
	{0.375, 0.25, 0.0, 0.0},
	{0.375, 0.5 , 0.5, 0.0},
	{0.125, 0.25, 0.5, 1.0}
}};

static const DS_ALIGN(32) dsVector4d bezierMidd = {{0.125, 0.375, 0.375, 0.125}};

static bool isBezierStraightf(uint32_t axisCount, const dsVector4f* controlPoints,
	float chordalTolerance)
{
	// Check to see if the midpoint is within the chordal tolerance.
	double dist2 = 0.0f;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		float midCurve = dsVector4f_dot(controlPoints + i, &bezierMidf);
		float midLine = (controlPoints[i].x + controlPoints[i].w)*0.5f;
		float diff = midCurve - midLine;
		dist2 += dsPow2(diff);
	}
	return dist2 <= dsPow2(chordalTolerance);
}

static bool isBezierStraightd(uint32_t axisCount, const dsVector4d* controlPoints,
	double chordalTolerance)
{
	// Check to see if the midpoint is within the chordal tolerance.
	double dist2 = 0.0;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		double midCurve = dsVector4d_dot(controlPoints + i, &bezierMidd);
		double midLine = (controlPoints[i].x + controlPoints[i].w)*0.5;
		double diff = midCurve - midLine;
		dist2 += dsPow2(diff);
	}
	return dist2 <= dsPow2(chordalTolerance);
}

#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static bool tessellateRecSIMD4f(uint32_t axisCount, const dsVector4f* controlPoints,
	float chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctionf sampleFunc,
	void* userData, float t, uint32_t level)
{
	// Left side.
	float middlePoint[3];
	dsVector4f nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44f_transformSIMD(nextControlPoints + i, &leftBezierMatrixf, controlPoints + i);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecSIMD4f(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	float middleT = t + 1.0f/(float)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44f_transformSIMD(nextControlPoints + i, &rightBezierMatrixf, controlPoints + i);

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecSIMD4f(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static bool tessellateRecFMA4f(uint32_t axisCount, const dsVector4f* controlPoints,
	float chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctionf sampleFunc,
	void* userData, float t, uint32_t level)
{
	// Left side.
	float middlePoint[3];
	dsVector4f nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44f_transformFMA(nextControlPoints + i, &leftBezierMatrixf, controlPoints + i);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA4f(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	float middleT = t + 1.0f/(float)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44f_transformFMA(nextControlPoints + i, &rightBezierMatrixf, controlPoints + i);

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA4f(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE2)
static bool tessellateRecSIMD2d(uint32_t axisCount, const dsVector4d* controlPoints,
	double chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc,
	void* userData, double t, uint32_t level)
{
	// Left side.
	double middlePoint[3];
	dsVector4d nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44d_transformSIMD2(nextControlPoints + i, &leftBezierMatrixd, controlPoints + i);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecSIMD2d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	double middleT = t + 1.0/(double)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44d_transformSIMD2(nextControlPoints + i, &rightBezierMatrixd, controlPoints + i);

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecSIMD2d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)
static bool tessellateRecFMA2d(uint32_t axisCount, const dsVector4d* controlPoints,
	double chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc,
	void* userData, double t, uint32_t level)
{
	// Left side.
	double middlePoint[3];
	dsVector4d nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44d_transformFMA2(nextControlPoints + i, &leftBezierMatrixd, controlPoints + i);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA2d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	double middleT = t + 1.0/(double)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44d_transformFMA2(nextControlPoints + i, &rightBezierMatrixd, controlPoints + i);

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA2d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA)
static bool tessellateRecFMA4d(uint32_t axisCount,
	const dsVector4d* DS_ALIGN_PARAM(32) controlPoints, double chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData, double t,
	uint32_t level)
{
	// Left side.
	double middlePoint[3];
	DS_ALIGN(32) dsVector4d nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44d_transformFMA4(nextControlPoints + i, &leftBezierMatrixd, controlPoints + i);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA4d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	double middleT = t + 1.0/(double)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44d_transformFMA4(nextControlPoints + i, &rightBezierMatrixd, controlPoints + i);

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecFMA4d(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}
DS_SIMD_END()
#endif

static bool tessellateRecf(uint32_t axisCount, const dsVector4f* controlPoints,
	float chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctionf sampleFunc,
	void* userData, float t, uint32_t level)
{
	// Left side.
	float middlePoint[3];
	dsVector4f nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44_transform(nextControlPoints[i], leftBezierMatrixf, controlPoints[i]);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecf(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	float middleT = t + 1.0f/(float)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44_transform(nextControlPoints[i], rightBezierMatrixf, controlPoints[i]);

	if (level < maxRecursions && !isBezierStraightf(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecf(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}

static bool tessellateRecd(uint32_t axisCount, const dsVector4d* controlPoints,
	double chordalTolerance, uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc,
	void* userData, double t, uint32_t level)
{
	// Left side.
	double middlePoint[3];
	dsVector4d nextControlPoints[3];
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		dsMatrix44_transform(nextControlPoints[i], leftBezierMatrixd, controlPoints[i]);
		middlePoint[i] = nextControlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecd(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, t, level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	double middleT = t + 1.0/(double)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < axisCount; ++i)
		dsMatrix44_transform(nextControlPoints[i], rightBezierMatrixd, controlPoints[i]);

	if (level < maxRecursions && !isBezierStraightd(axisCount, nextControlPoints, chordalTolerance))
	{
		if (!tessellateRecd(axisCount, nextControlPoints, chordalTolerance, maxRecursions,
				sampleFunc, userData, middleT, level + 1))
		{
			return false;
		}
	}

	return true;
}

#if DS_GCC
#pragma GCC diagnostic pop
#endif

bool dsBezierCurvef_initialize(dsBezierCurvef* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2 || !p3)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		curve->controlPoints[i].x = ((const float*)p0)[i];
		curve->controlPoints[i].y = ((const float*)p1)[i];
		curve->controlPoints[i].z = ((const float*)p2)[i];
		curve->controlPoints[i].w = ((const float*)p3)[i];
	}

	return true;
}

bool dsBezierCurved_initialize(dsBezierCurved* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2 || !p3)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		curve->controlPoints[i].x = ((const double*)p0)[i];
		curve->controlPoints[i].y = ((const double*)p1)[i];
		curve->controlPoints[i].z = ((const double*)p2)[i];
		curve->controlPoints[i].w = ((const double*)p3)[i];
	}

	return true;
}

bool dsBezierCurvef_initializeQuadratic(dsBezierCurvef* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2)
	{
		errno = EINVAL;
		return false;
	}

	// https://stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
	curve->axisCount = axisCount;
	const float controlT = 2.0f/3.0f;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		float start = ((const float*)p0)[i];
		float control = ((const float*)p1)[i];
		float end = ((const float*)p2)[i];

		curve->controlPoints[i].x = start;
		curve->controlPoints[i].y = start + (control - start)*controlT;
		curve->controlPoints[i].z = end + (control - end)*controlT;
		curve->controlPoints[i].w = end;
	}

	return true;
}

bool dsBezierCurved_initializeQuadratic(dsBezierCurved* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2)
	{
		errno = EINVAL;
		return false;
	}

	// https://stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
	curve->axisCount = axisCount;
	const double controlT = 2.0/3.0;
	for (uint32_t i = 0; i < axisCount; ++i)
	{
		double start = ((const double*)p0)[i];
		double control = ((const double*)p1)[i];
		double end = ((const double*)p2)[i];

		curve->controlPoints[i].x = start;
		curve->controlPoints[i].y = start + (control - start)*controlT;
		curve->controlPoints[i].z = end + (control - end)*controlT;
		curve->controlPoints[i].w = end;
	}

	return true;
}

bool dsBezierCurvef_evaluate(void* outPoint, const dsBezierCurvef* curve, float t)
{
	if (!outPoint || !curve)
	{
		errno = EINVAL;
		return false;
	}

	if (t < 0.0f || t > 1.0f)
	{
		errno = ERANGE;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	float invT = 1.0f - t;
	dsVector4f tMul = {{dsPow3(invT), 3.0f*dsPow2(invT)*t, 3.0f*dsPow2(t)*invT, dsPow3(t)}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((float*)outPoint)[i] = dsVector4f_dot(&tMul, curve->controlPoints + i);

	return true;
}

bool dsBezierCurved_evaluate(void* outPoint, const dsBezierCurved* curve, double t)
{
	if (!outPoint || !curve)
	{
		errno = EINVAL;
		return false;
	}

	if (t < 0.0 || t > 1.0)
	{
		errno = ERANGE;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	double invT = 1.0 - t;
	dsVector4d tMul = {{dsPow3(invT), 3.0*dsPow2(invT)*t, 3.0*dsPow2(t)*invT, dsPow3(t)}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((double*)outPoint)[i] = dsVector4d_dot(&tMul, curve->controlPoints + i);

	return true;
}

bool dsBezierCurvef_evaluateTangent(void* outTangent, const dsBezierCurvef* curve, float t)
{
	if (!outTangent || !curve)
	{
		errno = EINVAL;
		return false;
	}

	if (t < 0.0f || t > 1.0f)
	{
		errno = ERANGE;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	float invT = 1.0f - t;
	dsVector3f tMul = {{3.0f*dsPow2(invT), 6.0f*invT*t, 3.0f*dsPow2(t)}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		dsVector3f controlTangent = {{curve->controlPoints[i].y - curve->controlPoints[i].x,
			curve->controlPoints[i].z - curve->controlPoints[i].y,
			curve->controlPoints[i].w - curve->controlPoints[i].z}};
		((float*)outTangent)[i] = dsVector3_dot(tMul, controlTangent);;
	}

	return true;
}

bool dsBezierCurved_evaluateTangent(void* outTangent, const dsBezierCurved* curve, double t)
{
	if (!outTangent || !curve)
	{
		errno = EINVAL;
		return false;
	}

	if (t < 0.0 || t > 1.0)
	{
		errno = ERANGE;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	double invT = 1.0 - t;
	dsVector3d tMul = {{3.0*dsPow2(invT), 6.0*invT*t, 3.0*dsPow2(t)}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		dsVector3d controlTangent = {{curve->controlPoints[i].y - curve->controlPoints[i].x,
			curve->controlPoints[i].z - curve->controlPoints[i].y,
			curve->controlPoints[i].w - curve->controlPoints[i].z}};
		((double*)outTangent)[i] = dsVector3_dot(tMul, controlTangent);;
	}

	return true;
}

bool dsBezierCurvef_tessellate(const dsBezierCurvef* curve, float chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunctionf sampleFunc, void* userData)
{
	if (!curve || chordalTolerance <= 0.0f || maxRecursions > DS_MAX_CURVE_RECURSIONS ||
		!sampleFunc)
	{
		errno = EINVAL;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	float endPoint[3];

	// First point.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		endPoint[i] = curve->controlPoints[i].x;
	if (!sampleFunc(userData, endPoint, curve->axisCount, 0.0))
		return false;

	// Subdivide the bazier: http://algorithmist.net/docs/subdivision.pdf
	// Don't check chordal tolerance for the first point since it might pass through the center
	// line.
	if (maxRecursions > 0)
	{
#if DS_HAS_SIMD
		if (dsHostSIMDFeatures & dsSIMDFeatures_FMA)
		{
			if (!tessellateRecFMA4f(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0f, 1))
			{
				return false;
			}
		}
		else if (dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		{
			if (!tessellateRecSIMD4f(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0f, 1))
			{
				return false;
			}
		}
		else
#endif
		{
			if (!tessellateRecf(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0f, 1))
			{
				return false;
			}
		}
	}

	// Last point.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		endPoint[i] = curve->controlPoints[i].w;
	return sampleFunc(userData, endPoint, curve->axisCount, 1.0f);
}

bool dsBezierCurved_tessellate(const dsBezierCurved* curve, double chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData)
{
	if (!curve || chordalTolerance <= 0.0 || maxRecursions > DS_MAX_CURVE_RECURSIONS || !sampleFunc)
	{
		errno = EINVAL;
		return false;
	}

	DS_ASSERT(curve->axisCount >= 2 && curve->axisCount <= 3);
	double endPoint[3];

	// First point.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		endPoint[i] = curve->controlPoints[i].x;
	if (!sampleFunc(userData, endPoint, curve->axisCount, 0.0))
		return false;

	// Subdivide the bazier: http://algorithmist.net/docs/subdivision.pdf
	// Don't check chordal tolerance for the first point since it might pass through the center
	// line.
	if (maxRecursions > 0)
	{
#if DS_HAS_SIMD
		const dsSIMDFeatures fma4 = dsSIMDFeatures_Double4 | dsSIMDFeatures_FMA;
		const dsSIMDFeatures fma2 = dsSIMDFeatures_Double4 | dsSIMDFeatures_FMA;
		if ((dsHostSIMDFeatures & fma4) == fma4)
		{
			DS_ALIGN(32) dsVector4d alignedControlPoints[3];
			memcpy(alignedControlPoints, curve->controlPoints, sizeof(dsVector4d)*curve->axisCount);
			if (!tessellateRecFMA4d(curve->axisCount, alignedControlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0, 1))
			{
				return false;
			}
		}
		else if ((dsHostSIMDFeatures & fma2) == fma2)
		{
			if (!tessellateRecFMA2d(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0, 1))
			{
				return false;
			}
		}
		else if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		{
			if (!tessellateRecSIMD2d(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0, 1))
			{
				return false;
			}
		}
		else
#endif
		{
			if (!tessellateRecd(curve->axisCount, curve->controlPoints, chordalTolerance,
					maxRecursions, sampleFunc, userData, 0.0, 1))
			{
				return false;
			}
		}
	}

	// Last point.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		endPoint[i] = curve->controlPoints[i].w;
	return sampleFunc(userData, endPoint, curve->axisCount, 1.0);
}
