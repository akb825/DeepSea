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

#include <DeepSea/Geometry/CubicCurve.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector4.h>

#include <string.h>

inline static void evaluatef(void* outPoint, const dsCubicCurvef* curve, float t)
{
	float t2 = t*t;
	dsVector4f tVec = {{1.0f, t, t2, t*t2}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((float*)outPoint)[i] = dsVector4f_dot(&tVec, curve->polynomials + i);
}

inline static void evaluated(void* outPoint, const dsCubicCurved* curve, double t)
{
	double t2 = t*t;
	dsVector4d tVec = {{1.0, t, t2, t*t2}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((double*)outPoint)[i] = dsVector4d_dot(&tVec, curve->polynomials + i);
}

inline static void evaluateTangentf(void* outTangent, const dsCubicCurvef* curve, float t)
{
	// First derivative for tVec of evaluate.
	dsVector4f tVec = {{0.0f, 1.0f, 2.0f*t, 3.0f*t*t}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((float*)outTangent)[i] = dsVector4f_dot(&tVec, curve->polynomials + i);
}

inline static void evaluateTangentd(void* outTangent, const dsCubicCurved* curve, double t)
{
	// First derivative for tVec of evaluate.
	dsVector4d tVec = {{0.0, 1.0, 2.0*t, 3.0*t*t}};
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		((double*)outTangent)[i] = dsVector4d_dot(&tVec, curve->polynomials + i);
}

static bool shouldRecursef(dsVector3f* outPoint, const dsCubicCurvef* curve, float t,
	float chordalTolerance2, const dsVector3f* startPoint, const dsVector3f* endPoint)
{
	evaluatef(outPoint, curve, t);

	// Check to see if the midpoint is within the chordal tolerance.
	float dist2 = 0.0f;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		float midLine = (startPoint->values[i] + endPoint->values[i])*0.5f;
		float diff = outPoint->values[i] - midLine;
		dist2 += dsPow2(diff);
	}

	if (dist2 > chordalTolerance2)
		return true;

	// Check the tangent if it's within tolerance.
	dsVector3f tangent;
	evaluateTangentf(&tangent, curve, t);

	dsVector3f lineTangent;
	float tangentDist2 = 0.0f;
	dist2 = 0.0f;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		tangentDist2 += dsPow2(tangent.values[i]);
		lineTangent.values[i] = endPoint->values[i] - startPoint->values[i];
		dist2 += dsPow2(lineTangent.values[i]);
	}

	float tangentDist = sqrtf(tangentDist2);
	float dist = sqrtf(dist2);
	float cosTangents = 0.0f;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		cosTangents += lineTangent.values[i]*tangent.values[i]/(dist*tangentDist);

	// Squared distance perpendicular to the line.
	float sinTangents2 = 1.0f - dsPow2(cosTangents);
	return dist2*sinTangents2*0.25f >= chordalTolerance2;
}

static bool shouldRecursed(dsVector3d* outPoint, const dsCubicCurved* curve, double t,
	double chordalTolerance2, const dsVector3d* startPoint, const dsVector3d* endPoint)
{
	evaluated(outPoint, curve, t);

	// Check to see if the midpoint is within the chordal tolerance.
	double dist2 = 0.0;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		double midLine = (startPoint->values[i] + endPoint->values[i])*0.5;
		double diff = outPoint->values[i] - midLine;
		dist2 += dsPow2(diff);
	}

	if (dist2 > chordalTolerance2)
		return true;

	// Check the tangent if it's within tolerance.
	dsVector3d tangent;
	evaluateTangentd(&tangent, curve, t);

	dsVector3d lineTangent;
	double tangentDist2 = 0.0;
	dist2 = 0.0f;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
	{
		tangentDist2 += dsPow2(tangent.values[i]);
		lineTangent.values[i] = endPoint->values[i] - startPoint->values[i];
		dist2 += dsPow2(lineTangent.values[i]);
	}

	double tangentDist = sqrt(tangentDist2);
	double dist = sqrt(dist2);
	double cosTangents = 0.0;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		cosTangents += lineTangent.values[i]*tangent.values[i]/(dist*tangentDist);

	// Squared distance perpendicular to the line.
	double sinTangents2 = 1.0 - dsPow2(cosTangents);
	return dist2*sinTangents2*0.25 >= chordalTolerance2;
}

static bool tessellateRecf(const dsCubicCurvef* curve, float chordalTolerance2,
	unsigned int maxRecursions, dsCurveSampleFunctionf sampleFunc, void* userData,
	const dsVector3f* startPoint, float startT, const dsVector3f* endPoint, float endT,
	unsigned int depth)
{
	float midT = (startT + endT)*0.5f;
	dsVector3f midPoint;
	if (!shouldRecursef(&midPoint, curve, midT, chordalTolerance2, startPoint, endPoint))
		return true;

	if (depth < maxRecursions)
	{
		if (!tessellateRecf(curve, chordalTolerance2, maxRecursions, sampleFunc, userData,
				startPoint, startT, &midPoint, midT, depth + 1))
		{
			return false;
		}
	}

	if (!sampleFunc(userData, &midPoint, curve->axisCount, midT))
		return false;

	if (depth < maxRecursions)
	{
		if (!tessellateRecf(curve, chordalTolerance2, maxRecursions, sampleFunc, userData,
				&midPoint, midT, endPoint, endT, depth + 1))
		{
			return false;
		}
	}

	return true;
}

static bool tessellateRecd(const dsCubicCurved* curve, double chordalTolerance2,
	unsigned int maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData,
	const dsVector3d* startPoint, double startT, const dsVector3d* endPoint, double endT,
	unsigned int depth)
{
	double midT = (startT + endT)*0.5;
	dsVector3d midPoint;
	if (!shouldRecursed(&midPoint, curve, midT, chordalTolerance2, startPoint, endPoint))
		return true;

	if (depth < maxRecursions)
	{
		if (!tessellateRecd(curve, chordalTolerance2, maxRecursions, sampleFunc, userData,
				startPoint, startT, &midPoint, midT, depth + 1))
		{
			return false;
		}
	}

	if (!sampleFunc(userData, &midPoint, curve->axisCount, midT))
		return false;

	if (depth < maxRecursions)
	{
		if (!tessellateRecd(curve, chordalTolerance2, maxRecursions, sampleFunc, userData,
				&midPoint, midT, endPoint, endT, depth + 1))
		{
			return false;
		}
	}

	return true;
}

const dsMatrix44f dsCubicCurvef_bezierToCubic =
{{
	{1.0f, -3.0f,  3.0f, -1.0f},
	{0.0f,  3.0f, -6.0f,  3.0f},
	{0.0f,  0.0f,  3.0f, -3.0f},
	{0.0f,  0.0f,  0.0f,  1.0f}
}};

DS_ALIGN(32) const dsMatrix44d dsCubicCurved_bezierToCubic =
{{
	{1.0, -3.0,  3.0, -1.0},
	{0.0,  3.0, -6.0,  3.0},
	{0.0,  0.0,  3.0, -3.0},
	{0.0,  0.0,  0.0,  1.0}
}};

const dsMatrix44f dsCubicCurvef_cubicToBezier =
{{
	{1.0f, 1.0f     , 1.0f     , 1.0f},
	{0.0f, 1.0f/3.0f, 2.0f/3.0f, 1.0f},
	{0.0f, 0.0f     , 1.0f/3.0f, 1.0f},
	{0.0f, 0.0f     , 0.0f     , 1.0f}
}};

DS_ALIGN(32) const dsMatrix44d dsCubicCurved_cubicToBezier =
{{
	{1.0, 1.0    , 1.0    , 1.0},
	{0.0, 1.0/3.0, 2.0/3.0, 1.0},
	{0.0, 0.0    , 1.0/3.0, 1.0},
	{0.0, 0.0    , 0.0    , 1.0}
}};

const dsMatrix44f dsCubicCurvef_hermiteToCubic =
{{
	{ 1.0f, 0.0f, -3.0f,  2.0f},
	{ 0.0f, 0.0f,  3.0f, -2.0f},
	{ 0.0f, 1.0f, -2.0f,  1.0f},
	{ 0.0f, 0.0f, -1.0f,  1.0f}
}};

DS_ALIGN(32) const dsMatrix44d dsCubicCurved_hermiteToCubic =
{{
	{ 1.0, 0.0, -3.0,  2.0},
	{ 0.0, 0.0,  3.0, -2.0},
	{ 0.0, 1.0, -2.0,  1.0},
	{ 0.0, 0.0, -1.0,  1.0}
}};

const dsMatrix44f dsCubicCurvef_cubicToHermite =
{{
	{1.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 1.0f, 1.0f},
	{0.0f, 1.0f, 0.0f, 2.0f},
	{0.0f, 1.0f, 0.0f, 3.0f}
}};

DS_ALIGN(32) const dsMatrix44d dsCubicCurved_cubicToHermite =
{{
	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 1.0},
	{0.0, 1.0, 0.0, 2.0},
	{0.0, 1.0, 0.0, 3.0}
}};

bool dsCubicCurvef_initializeBezier(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2 || !p3)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p3, sizeof(float)*axisCount);

	for (unsigned int i = 0; i < axisCount; ++i)
	{
		dsVector4f thisDim = {{((const float*)p0)[i], ((const float*)p1)[i], ((const float*)p2)[i],
			((const float*)p3)[i]}};
		dsMatrix44f_transform(curve->polynomials + i, &dsCubicCurvef_bezierToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurved_initializeBezier(dsCubicCurved* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2, const void* p3)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2 || !p3)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p3, sizeof(double)*axisCount);

	for (unsigned int i = 0; i < axisCount; ++i)
	{
		dsVector4d thisDim = {{((const double*)p0)[i], ((const double*)p1)[i],
			((const double*)p2)[i], ((const double*)p3)[i]}};
		dsMatrix44d_transform(curve->polynomials + i, &dsCubicCurved_bezierToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurvef_initializeQuadratic(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* p1, const void* p2)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p2, sizeof(float)*axisCount);

	// https://stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
	const float controlT = 2.0f/3.0f;
	for (unsigned int i = 0; i < axisCount; ++i)
	{
		float start = ((const float*)p0)[i];
		float control = ((const float*)p1)[i];
		float end = ((const float*)p2)[i];
		dsVector4f thisDim = {{start, start + (control - start)*controlT,
			end + (control - end)*controlT, end}};
		dsMatrix44f_transform(curve->polynomials + i, &dsCubicCurvef_bezierToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurved_initializeQuadratic(dsCubicCurved* curve, uint32_t axisCount,
	const void* p0, const void* p1, const void* p2)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !p1 || !p2)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p2, sizeof(double)*axisCount);

	// https://stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
	const double controlT = 2.0f/3.0f;
	for (unsigned int i = 0; i < axisCount; ++i)
	{
		double start = ((const double*)p0)[i];
		double control = ((const double*)p1)[i];
		double end = ((const double*)p2)[i];
		dsVector4d thisDim = {{start, start + (control - start)*controlT,
			end + (control - end)*controlT, end}};
		dsMatrix44d_transform(curve->polynomials + i, &dsCubicCurved_bezierToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurvef_initializeHermite(dsCubicCurvef* curve, unsigned int axisCount,
	const void* p0, const void* t0, const void* p1, const void* t1)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !t0 || !p1 || !t1)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p1, sizeof(float)*axisCount);

	for (unsigned int i = 0; i < axisCount; ++i)
	{
		dsVector4f thisDim = {{((const float*)p0)[i], ((const float*)p1)[i], ((const float*)t0)[i],
			((const float*)t1)[i]}};
		dsMatrix44f_transform(curve->polynomials + i, &dsCubicCurvef_hermiteToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurved_initializeHermite(dsCubicCurved* curve, unsigned int axisCount,
	const void* p0, const void* t0, const void* p1, const void* t1)
{
	if (!curve || axisCount < 2 || axisCount > 3 || !p0 || !t0 || !p1 || !t1)
	{
		errno = EINVAL;
		return false;
	}

	curve->axisCount = axisCount;
	memcpy(curve->endPoint, p1, sizeof(double)*axisCount);

	for (unsigned int i = 0; i < axisCount; ++i)
	{
		dsVector4d thisDim = {{((const double*)p0)[i], ((const double*)p1)[i],
			((const double*)t0)[i], ((const double*)t1)[i]}};
		dsMatrix44d_transform(curve->polynomials + i, &dsCubicCurved_hermiteToCubic, &thisDim);
	}

	return true;
}

bool dsCubicCurvef_evaluate(void* outPoint, const dsCubicCurvef* curve, float t)
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

	if (t == 1.0f)
	{
		// Exact value of the end since the evaluation may introduce some error.
		memcpy(outPoint, curve->endPoint, sizeof(float)*curve->axisCount);
		return true;
	}

	evaluatef(outPoint, curve, t);
	return true;
}

bool dsCubicCurved_evaluate(void* outPoint, const dsCubicCurved* curve, double t)
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

	if (t == 1.0)
	{
		// Exact value of the end since the evaluation may introduce some error.
		memcpy(outPoint, curve->endPoint, sizeof(double)*curve->axisCount);
		return true;
	}

	evaluated(outPoint, curve, t);
	return true;
}

bool dsCubicCurvef_evaluateTangent(void* outTangent, const dsCubicCurvef* curve, float t)
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

	evaluateTangentf(outTangent, curve, t);
	return true;
}

bool dsCubicCurved_evaluateTangent(void* outTangent, const dsCubicCurved* curve, double t)
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

	evaluateTangentd(outTangent, curve, t);
	return true;
}

bool dsCubicCurvef_tessellate(const dsCubicCurvef* curve, float chordalTolerance,
	unsigned int maxRecursions, dsCurveSampleFunctionf sampleFunc, void* userData)
{
	if (!curve || chordalTolerance <= 0.0f || maxRecursions > DS_MAX_CURVE_RECURSIONS ||
		!sampleFunc)
	{
		errno = EINVAL;
		return false;
	}

	// Exact start and end points.
	dsVector3f startPoint, endPoint;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		startPoint.values[i] = curve->polynomials[i].x;
	memcpy(&endPoint, curve->endPoint, sizeof(float)*curve->axisCount);

	if (!sampleFunc(userData, &startPoint, curve->axisCount, 0.0f))
		return false;

	if (maxRecursions > 0 && !tessellateRecf(curve, dsPow2(chordalTolerance), maxRecursions,
			sampleFunc, userData, &startPoint, 0.0f, &endPoint, 1.0f, 1))
	{
		return false;
	}

	return sampleFunc(userData, &endPoint, curve->axisCount, 1.0f);
}

bool dsCubicCurved_tessellate(const dsCubicCurved* curve, double chordalTolerance,
	unsigned int maxRecursions, dsCurveSampleFunctiond sampleFunc, void* userData)
{
	if (!curve || chordalTolerance <= 0.0 || maxRecursions > DS_MAX_CURVE_RECURSIONS ||
		!sampleFunc)
	{
		errno = EINVAL;
		return false;
	}

	// Exact start and end points.
	dsVector3d startPoint, endPoint;
	for (uint32_t i = 0; i < curve->axisCount; ++i)
		startPoint.values[i] = curve->polynomials[i].x;
	memcpy(&endPoint, curve->endPoint, sizeof(double)*curve->axisCount);

	if (!sampleFunc(userData, &startPoint, curve->axisCount, 0.0))
		return false;

	if (maxRecursions > 0 && !tessellateRecd(curve, dsPow2(chordalTolerance), maxRecursions,
			sampleFunc, userData, &startPoint, 0.0, &endPoint, 1.0, 1))
	{
		return false;
	}

	return sampleFunc(userData, &endPoint, curve->axisCount, 1.0);
}
