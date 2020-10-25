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

#include <DeepSea/Geometry/BezierCurve.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector4.h>

// Left and right subdivision matrices from http://algorithmist.net/docs/subdivision.pdf
static const dsMatrix44d leftBezierMatrix =
{{
	{1.0, 0.5, 0.25, 0.125},
	{0.0, 0.5, 0.5 , 0.375},
	{0.0, 0.0, 0.25, 0.375},
	{0.0, 0.0, 0.0 , 0.125}
}};

static const dsMatrix44d rightBezierMatrix =
{{
	{0.125, 0.0 , 0.0, 0.0},
	{0.375, 0.25, 0.0, 0.0},
	{0.375, 0.5 , 0.5, 0.0},
	{0.125, 0.25, 0.5, 1.0}
}};

static const dsVector4d bezierMid = {{0.125, 0.375, 0.375, 0.125}};

static bool isBezierStraight(const dsBezierCurve* curve, double chordalTolerance)
{
	// Check to see if the midpoint is within the chordal tolerance.
	double dist2 = 0.0;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		double midCurve = dsVector4_dot(curve->controlPoints[i], bezierMid);
		double midLine = (curve->controlPoints[i].x + curve->controlPoints[i].w)*0.5;
		double diff = midCurve - midLine;
		dist2 += dsPow2(diff);
	}
	return dist2 <= dsPow2(chordalTolerance);
}

static bool tessellateRec(const dsBezierCurve* curve, double chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunction sampleFunc, void* userData, double t,
	uint32_t level)
{
	// Left side.
	double middlePoint[3];
	dsBezierCurve nextCurve;
	nextCurve.axisCount = curve->axisCount;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		dsMatrix44_transform(nextCurve.controlPoints[i], leftBezierMatrix, curve->controlPoints[i]);
		middlePoint[i] = nextCurve.controlPoints[i].w;
	}

	if (level < maxRecursions && !isBezierStraight(&nextCurve, chordalTolerance))
	{
		if (!tessellateRec(&nextCurve, chordalTolerance, maxRecursions, sampleFunc, userData, t,
			level + 1))
		{
			return false;
		}
	}

	// The middle point is guaranteed to be on the curve.
	double middleT = t + 1.0/(double)(1ULL << level);
	if (!sampleFunc(userData, middlePoint, curve->axisCount, middleT))
		return false;

	// Right side.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		dsMatrix44_transform(nextCurve.controlPoints[i], rightBezierMatrix,
			curve->controlPoints[i]);
	}

	if (level < maxRecursions && !isBezierStraight(&nextCurve, chordalTolerance))
	{
		if (!tessellateRec(&nextCurve, chordalTolerance, maxRecursions, sampleFunc, userData, middleT,
			level + 1))
		{
			return false;
		}
	}

	return true;
}

bool dsBezierCurve_initialize(dsBezierCurve* curve, uint32_t axisCount,
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

bool dsBezierCurve_initializeQuadratic(dsBezierCurve* curve, uint32_t axisCount,
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

bool dsBezierCurve_evaluate(void* outPoint, const dsBezierCurve* curve, double t)
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
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		((double*)outPoint)[i] =
			dsPow3(invT)*curve->controlPoints[i].x +
			3.0*dsPow2(invT)*t*curve->controlPoints[i].y +
			3.0*dsPow2(t)*invT*curve->controlPoints[i].z +
			dsPow3(t)*curve->controlPoints[i].w;
	}

	return true;
}

bool dsBezierCurve_evaluateTangent(void* outTangent, const dsBezierCurve* curve, double t)
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
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		((double*)outTangent)[i] =
			3.0*dsPow2(invT)*(curve->controlPoints[i].y - curve->controlPoints[i].x) +
			6.0*invT*t*(curve->controlPoints[i].z - curve->controlPoints[i].y) +
			3.0*dsPow2(t)*(curve->controlPoints[i].w - curve->controlPoints[i].z);
	}

	return true;
}

bool dsBezierCurve_tessellate(const dsBezierCurve* curve, double chordalTolerance,
	uint32_t maxRecursions, dsCurveSampleFunction sampleFunc, void* userData)
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
		if (!tessellateRec(curve, chordalTolerance, maxRecursions, sampleFunc, userData, 0.0, 1))
			return false;
	}

	// Last point.
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		endPoint[i] = curve->controlPoints[i].w;
	return sampleFunc(userData, endPoint, curve->axisCount, 1.0);
}
