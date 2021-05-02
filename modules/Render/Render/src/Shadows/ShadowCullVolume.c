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

#include <DeepSea/Render/Shadows/ShadowCullVolume.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <float.h>

static double computeEpsilon(const dsShadowCullVolume* volume, const dsPlane3d* planes)
{
	double maxD = 0;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		double d = fabs(planes[i].d);
		maxD = dsMax(d, maxD);
	}

	const double baseEpsilon = 1e-14;
	return maxD*baseEpsilon;
}

static bool pointInVolume(const dsShadowCullVolume* volume, const dsPlane3d* planes,
	const dsVector3d* point, double epsilon)
{
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		double distance = dsPlane3_distanceToPoint(planes[i], *point);
		if (distance < -epsilon)
			return false;
	}
	return true;
}

static void getTRange(double* outMinT, uint32_t* outMinPlane, double* outMaxT,
	uint32_t* outMaxPlane, const dsShadowCullVolume* volume, const dsPlane3d* planes,
	const dsRay3d* ray)
{
	*outMinT = -DBL_MAX;
	*outMinPlane = 0;
	*outMaxT = DBL_MAX;
	*outMaxPlane = 0;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		const dsPlane3d* plane = planes + i;
		double t = dsPlane3d_rayIntersection(plane, ray);
		if (t == DBL_MAX)
			continue;

		if (dsVector3_dot(plane->n, ray->origin) > 0)
		{
			if (t < *outMaxT)
			{
				*outMaxT = t;
				*outMaxPlane = i;
			}
		}
		else
		{
			if (t > *outMinT)
			{
				*outMinT = t;
				*outMinPlane = i;
			}
		}
	}
}

static void addCorner(dsShadowCullVolume* volume, const dsVector3d* point, uint32_t p0, uint32_t p1,
	uint32_t p3)
{
	DS_ASSERT(volume->cornerCount < DS_MAX_SHADOW_CULL_CORNERS);

	uint32_t minFirstP = dsMin(p0, p1);
	uint32_t maxFirstP = dsMax(p0, p1);
	uint32_t minP = dsMin(minFirstP, p3);
	uint32_t middleP = dsMin(maxFirstP, p3);
	uint32_t maxP = dsMax(maxFirstP, p3);

	// Check if we already have a corner for the plane triplet.
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsShadowCullCorner* corner = volume->corners + i;
		if (corner->planes[0] == minP && corner->planes[1] == middleP && corner->planes[2] == maxP)
			return;
	}

	dsShadowCullCorner* corner = volume->corners + (volume->cornerCount++);
	dsConvertDoubleToFloat(corner->point, *point);
	corner->planes[0] = minP;
	corner->planes[1] = middleP;
	corner->planes[2] = maxP;
}

static void computeEdgesAndCorners(dsShadowCullVolume* volume, const dsPlane3d* planes)
{
	for (uint32_t i = 0; i < volume->planeCount; ++i)
		dsConvertDoubleToFloat(volume->planes[i], planes[i]);

	// Find all intersecting lines between pairs of planes.
	double epsilon = computeEpsilon(volume, planes);
	for (uint32_t i = 0; i < volume->planeCount - 1; ++i)
	{
		const dsPlane3d* firstPlane = planes + i;
		for (uint32_t j = i + 1; j < volume->planeCount; ++j)
		{
			const dsPlane3d* secondPlane = planes + j;
			dsRay3d ray;
			if (!dsPlane3d_intersectingLine(&ray, firstPlane, secondPlane))
				continue;

			double minT, maxT;
			uint32_t minPlane, maxPlane;
			getTRange(&minT, &minPlane, &maxT, &maxPlane, volume, planes, &ray);
			// If the T range is inverted, we're outside of the volume.
			if (minT >= maxT - epsilon)
				continue;

			if (dsEpsilonEquald(minT, maxT, epsilon))
			{
				dsVector3d point;
				dsVector3_scale(point, ray.direction, minT);
				dsVector3_add(point, point, ray.origin);
				if (pointInVolume(volume, planes, &point, epsilon))
				{
					// Add the corner for each plane triplet to handle clamping even if they resolve
					// to the same position.
					addCorner(volume, &point, i, j, minPlane);
					addCorner(volume, &point, i, j, maxPlane);
				}
				continue;
			}

			// Add the min and max point assuming they didn't go to infinity. The line isn't
			// considered inside the volume if either point is outside of the volume.
			if (minT != -DBL_MAX)
			{
				dsVector3d point;
				dsVector3_scale(point, ray.direction, minT);
				dsVector3_add(point, point, ray.origin);
				if (!pointInVolume(volume, planes, &point, epsilon))
					continue;
				addCorner(volume, &point, i, j, minPlane);
			}

			if (maxT != DBL_MAX)
			{
				dsVector3d point;
				dsVector3_scale(point, ray.direction, maxT);
				dsVector3_add(point, point, ray.origin);
				if (!pointInVolume(volume, planes, &point, epsilon))
					continue;
				addCorner(volume, &point, i, j, maxPlane);
			}

			DS_ASSERT(volume->edgeCount < DS_MAX_SHADOW_CULL_EDGES);
			dsShadowCullEdge* edge = volume->edges + (volume->edgeCount++);
			dsConvertDoubleToFloat(edge->edge, ray);
			edge->planes[0] = i;
			edge->planes[1] = j;
		}
	}
}

void removeUnusedPlanes(dsShadowCullVolume* volume)
{
	for (uint32_t i = 0; i < volume->planeCount;)
	{
		// Only need to check corners for plane referneces, since edges will only be added if they
		// have a corresponding edge.
		bool hasPlane = false;
		for (uint32_t j = 0; j < volume->cornerCount && !hasPlane; ++j)
		{
			const dsShadowCullCorner* corner = volume->corners + j;
			for (unsigned int k = 0; k < 3; ++k)
			{
				if (corner->planes[k] == i)
				{
					hasPlane = true;
					break;
				}
			}
		}

		if (hasPlane)
		{
			++i;
			continue;
		}

		// Shift all the planes back one.
		for (uint32_t j = i + 1; j < volume->planeCount; ++j)
			volume->planes[j - 1] = volume->planes[j];

		// Also subtract one for each plane index greater than the one being removed.
		for (uint32_t j = 0; j < volume->edgeCount; ++j)
		{
			dsShadowCullEdge* edge = volume->edges + j;
			for (unsigned int k = 0; k < 2; ++k)
			{
				if (edge->planes[k] > i)
					--edge->planes[k];
			}
		}

		for (uint32_t j = 0; j < volume->cornerCount; ++j)
		{
			dsShadowCullCorner* corner = volume->corners + j;
			for (unsigned int k = 0; k < 3; ++k)
			{
				if (corner->planes[k] > i)
					--corner->planes[k];
			}
		}

		--volume->planeCount;
	}
}

bool dsShadowCullVolume_buildDirectional(dsShadowCullVolume* volume,
	const dsFrustum3f* viewFrustum, const dsVector3f* toLight)
{
	if (!volume || !viewFrustum || !toLight)
	{
		errno = EINVAL;
		return false;
	}

	volume->planeCount = 0;
	volume->edgeCount = 0;
	volume->cornerCount = 0;

	// Use doubles for intersections to avoid large frustums causing numeric instability.
	dsFrustum3d viewFrustumd;
	dsConvertFloatToDouble(viewFrustumd, *viewFrustum);
	dsPlane3d planes[DS_MAX_SHADOW_CULL_PLANES];

	// Add any planes that face the light.
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		if (dsVector3_dot(viewFrustumd.planes[i].n, *toLight) <= 0)
			continue;

		DS_ASSERT(volume->planeCount < DS_MAX_SHADOW_CULL_PLANES);
		planes[volume->planeCount++] = viewFrustumd.planes[i];
	}

	// Detect any boundaries between pairs of planes that go from facing away from the light to
	// facing towards the light.
	typedef dsFrustumPlanes PlanePair[2];
	PlanePair boundaries[] =
	{
		{dsFrustumPlanes_Near, dsFrustumPlanes_Left},
		{dsFrustumPlanes_Near, dsFrustumPlanes_Right},
		{dsFrustumPlanes_Near, dsFrustumPlanes_Bottom},
		{dsFrustumPlanes_Near, dsFrustumPlanes_Top},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Left},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Right},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Bottom},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Top},
		{dsFrustumPlanes_Left, dsFrustumPlanes_Bottom},
		{dsFrustumPlanes_Bottom, dsFrustumPlanes_Right},
		{dsFrustumPlanes_Right, dsFrustumPlanes_Top},
		{dsFrustumPlanes_Top, dsFrustumPlanes_Left},
	};

	for (unsigned int i = 0; i < DS_ARRAY_SIZE(boundaries); ++i)
	{
		const PlanePair* curPlanes = boundaries + i;
		const dsPlane3d* first = viewFrustumd.planes + (*curPlanes)[0];
		const dsPlane3d* second = viewFrustumd.planes + (*curPlanes)[1];

		bool firstAway = dsVector3_dot(first->n, *toLight) < 0;
		bool secondAway = dsVector3_dot(second->n, *toLight) < 0;
		if (firstAway == secondAway)
			continue;

		dsRay3d line;
		if (!dsPlane3d_intersectingLine(&line, first, second))
			continue;

		DS_ASSERT(volume->planeCount < DS_MAX_SHADOW_CULL_PLANES);
		dsPlane3d* boundaryPlane = planes + volume->planeCount++;

		dsVector3_cross(boundaryPlane->n, line.direction, *toLight);
		// Should be roughly aligned with the plane that faces away from the light.
		const dsVector3d* referenceDir = firstAway ? &first->n : &second->n;
		if (dsVector3_dot(boundaryPlane->n, *referenceDir) < 0)
			dsVector3_neg(boundaryPlane->n, boundaryPlane->n);
		dsVector3d_normalize(&boundaryPlane->n, &boundaryPlane->n);
		boundaryPlane->d = dsVector3_dot(boundaryPlane->n, line.origin);
	}

	computeEdgesAndCorners(volume, planes);
	return true;
}

bool dsShadowCullVolume_buildSpot(dsShadowCullVolume* volume, const dsFrustum3f* viewFrustum,
	const dsFrustum3f* lightFrustum)
{
	if (!volume || !viewFrustum || !lightFrustum)
	{
		errno = EINVAL;
		return false;
	}

	volume->planeCount = 0;
	volume->edgeCount = 0;
	volume->cornerCount = 0;

	// Add the planes from both the view frustum and light frustum (minus the near plane for the
	// light), then let the edge and corner computation take care of the rest.
	dsPlane3d planes[DS_MAX_SHADOW_CULL_PLANES];
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		dsPlane3d* plane = planes + (volume->planeCount++);
		dsConvertFloatToDouble(*plane, viewFrustum->planes[i]);

		if (i != dsFrustumPlanes_Near)
		{
			plane = planes + (volume->planeCount++);
			dsConvertFloatToDouble(*plane, lightFrustum->planes[i]);
		}
	}

	computeEdgesAndCorners(volume, planes);
	removeUnusedPlanes(volume);
	return true;
}
