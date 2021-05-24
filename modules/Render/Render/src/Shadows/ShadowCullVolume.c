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
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Plane3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Shadows/ShadowProjection.h>

#include <float.h>

// Since the original computations were done with floats, be a bit loose with the epsilon values.
const double baseEpsilon = 1e-5;

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

static double relaxedRayIntersection(const dsPlane3d* plane, const dsRay3d* ray, double epsilon)
{
	const double epsilon2 = dsPow2(epsilon);
	double denom = dsVector3_dot(plane->n, ray->direction);
	if (fabs(denom) < epsilon2)
		return DBL_MAX;

	return (-dsVector3_dot(plane->n, ray->origin) + plane->d)/denom;
}

static bool getTRange(double* outMinT, uint32_t* outMinPlane, double* outMaxT,
	uint32_t* outMaxPlane, const dsShadowCullVolume* volume, const dsPlane3d* planes,
	const dsRay3d* ray, uint32_t firstPlane, uint32_t secondPlane, double epsilon)
{
	*outMinT = -DBL_MAX;
	*outMinPlane = 0;
	*outMaxT = DBL_MAX;
	*outMaxPlane = 0;
	bool anySet = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		if (i == firstPlane || i == secondPlane)
			continue;

		const dsPlane3d* plane = planes + i;
		// Even though we check for point containment, still need to have a relaxed intersection
		// check due to volumes for directional lights not being fully closed.
		double t = relaxedRayIntersection(plane, ray, baseEpsilon);
		if (t == DBL_MAX)
			continue;

		// Only take into account points in the final volume.
		dsVector3d point;
		dsVector3_scale(point, ray->direction, t);
		dsVector3_add(point, point, ray->origin);
		if (!pointInVolume(volume, planes, &point, epsilon))
			continue;

		if (dsVector3_dot(plane->n, ray->direction) < 0)
		{
			if (t < *outMaxT)
			{
				*outMaxT = t;
				*outMaxPlane = i;
				anySet = true;
			}
		}
		else
		{
			if (t > *outMinT)
			{
				*outMinT = t;
				*outMinPlane = i;
				anySet = true;
			}
		}
	}

	// If the T range is inverted, we're outside of the volume.
	return anySet && *outMinT < *outMaxT + epsilon;
}

static void addPlane(dsShadowCullVolume* volume, dsPlane3d* planes, const dsPlane3d* plane,
	double epsilon)
{
	DS_ASSERT(volume->planeCount < DS_MAX_SHADOW_CULL_PLANES);
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		const dsPlane3d* curPlane = planes + i;
		if (dsVector3d_epsilonEqual(&curPlane->n, &plane->n, baseEpsilon) &&
			dsRelativeEpsilonEquald(curPlane->d, plane->d, baseEpsilon))
		{
			return;
		}
	}

	planes[volume->planeCount++] = *plane;
}

static uint32_t bitmaskTripple(uint32_t p0, uint32_t p1, uint32_t p2)
{
	return (1 << p0) | (1 << p1) | (1 << p2);
}

static void addCorner(dsShadowCullVolume* volume, dsVector3d* cornerPoints,
	const dsVector3d* point, uint32_t planes, double epsilon)
{
	DS_ASSERT(volume->cornerCount < DS_MAX_SHADOW_CULL_CORNERS);
	// Check if we already have a corner for the plane triplet.
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		dsShadowCullCorner* corner = volume->corners + i;
		if ((corner->planes & planes) == planes)
			return;
		else if (dsVector3d_relativeEpsilonEqual(cornerPoints + i, point, epsilon))
		{
			corner->planes |= planes;
			return;
		}
	}

	cornerPoints[volume->cornerCount] = *point;
	dsShadowCullCorner* corner = volume->corners + (volume->cornerCount++);
	dsConvertDoubleToFloat(corner->point, *point);
	corner->planes = planes;
}

static void computeEdgesAndCorners(dsShadowCullVolume* volume, const dsPlane3d* planes,
	double epsilon)
{
	for (uint32_t i = 0; i < volume->planeCount; ++i)
		dsConvertDoubleToFloat(volume->planes[i], planes[i]);

	// Find all intersecting lines between pairs of planes.
	dsVector3d cornerPoints[DS_MAX_SHADOW_CULL_CORNERS];
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
			if (!getTRange(&minT, &minPlane, &maxT, &maxPlane, volume, planes, &ray, i, j, epsilon))
				continue;

			if (dsRelativeEpsilonEquald(minT, maxT, epsilon))
			{
				dsVector3d point;
				dsVector3_scale(point, ray.direction, minT);
				dsVector3_add(point, point, ray.origin);
				if (pointInVolume(volume, planes, &point, epsilon))
				{
					uint32_t planes = bitmaskTripple(i, j, minPlane) | (1 << maxPlane);
					addCorner(volume, cornerPoints, &point, planes, epsilon);
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
				addCorner(volume, cornerPoints, &point, bitmaskTripple(i, j, minPlane), epsilon);
			}

			if (maxT != DBL_MAX)
			{
				dsVector3d point;
				dsVector3_scale(point, ray.direction, maxT);
				dsVector3_add(point, point, ray.origin);
				if (!pointInVolume(volume, planes, &point, epsilon))
					continue;
				addCorner(volume, cornerPoints, &point, bitmaskTripple(i, j, maxPlane), epsilon);
			}

			DS_ASSERT(volume->edgeCount < DS_MAX_SHADOW_CULL_EDGES);
			dsShadowCullEdge* edge = volume->edges + (volume->edgeCount++);
			dsConvertDoubleToFloat(edge->edge, ray);
			edge->planes = (1 << i) | (1 << j);
		}
	}
}

static void removeUnusedPlanes(dsShadowCullVolume* volume)
{
	for (uint32_t i = 0; i < volume->planeCount;)
	{
		uint32_t planeMask = 1 << i;
		// Only need to check corners for plane referneces, since edges will only be added if they
		// have a corresponding edge.
		bool hasPlane = false;
		for (uint32_t j = 0; j < volume->cornerCount; ++j)
		{
			const dsShadowCullCorner* corner = volume->corners + j;
			if (corner->planes & planeMask)
			{
				hasPlane = true;
				break;
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

		// Also shift the bits for all higher index planes.
		uint32_t prevPlanesMask = (1 << i) - 1;
		for (uint32_t j = 0; j < volume->edgeCount; ++j)
		{
			dsShadowCullEdge* edge = volume->edges + j;
			edge->planes = (edge->planes & prevPlanesMask) |
				((edge->planes & ~prevPlanesMask) >> 1);
		}

		for (uint32_t j = 0; j < volume->cornerCount; ++j)
		{
			dsShadowCullCorner* corner = volume->corners + j;
			corner->planes = (corner->planes & prevPlanesMask) |
				((corner->planes & ~prevPlanesMask) >> 1);
		}

		--volume->planeCount;
	}
}

static void addPointsToProjection(const dsShadowCullVolume* volume, dsVector3f* points,
	uint32_t pointCount, dsShadowProjection* shadowProj, bool intersects)
{
	if (!intersects)
	{
		DS_VERIFY(dsShadowProjection_addPoints(shadowProj, points, pointCount));
		return;
	}

	for (uint32_t i = 0; i < pointCount; ++i)
	{
		// Find which planes the point is outside of.
		dsVector3f* point = points + i;
		uint32_t outsidePlanes = 0;
		uint32_t outsideCount = 0;
		for (uint32_t j = 0; j < volume->planeCount; ++j)
		{
			float distance = dsPlane3_distanceToPoint(volume->planes[j], *point);
			if (distance < -baseEpsilon)
			{
				outsidePlanes |= 1 << j;
				++outsideCount;
			}
		}

		// All inside, leave the point alone.
		if (outsidePlanes == 0)
			continue;

		// Volume should be a convex hull, so we should be able to identify corners or edges to
		// clamp to based on the planes that are outside.
		if (outsideCount >= 3)
		{
			bool found = false;
			for (uint32_t j = 0; j < volume->cornerCount; ++j)
			{
				const dsShadowCullCorner* corner = volume->corners + j;
				if ((outsidePlanes & corner->planes) == corner->planes)
				{
					*point = corner->point;
					found = true;
					break;
				}
			}

			if (found)
				continue;
		}

		// Failed to find a corner, so next find an edge.
		if (outsideCount >= 2)
		{
			bool found = false;
			for (uint32_t j = 0; j < volume->edgeCount; ++j)
			{
				const dsShadowCullEdge* edge = volume->edges + j;
				if ((outsidePlanes & edge->planes) == edge->planes)
				{
					// Find the closest point on the line.
					dsVector3f relativeDir;
					dsVector3_sub(relativeDir, *point, edge->edge.origin);
					float t = dsVector3_dot(relativeDir, edge->edge.direction);

					dsVector3f offset;
					dsVector3_scale(offset, edge->edge.direction, t);
					dsVector3_add(*point, edge->edge.origin, offset);

					found = true;
					break;
				}
			}

			if (found)
				continue;
		}

		// Clamp to each plane it lies behind if we couldn't match against a corner or edge.
		for (uint32_t mask = outsidePlanes; mask; mask = dsRemoveLastBit(mask))
		{
			uint32_t j = dsBitmaskIndex(mask);
			const dsPlane3f* plane = volume->planes + j;
			float distance = dsPlane3_distanceToPoint(*plane, *point);
			// An earlier adjustment may have put it outside of the plane.
			if (distance > 0)
				continue;

			dsVector3f offset;
			dsVector3_scale(offset, plane->n, -distance);
			dsVector3_add(*point, *point, offset);
		}
	}

	DS_VERIFY(dsShadowProjection_addPoints(shadowProj, points, pointCount));
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
		if (dsVector3_dot(viewFrustumd.planes[i].n, *toLight) < -baseEpsilon)
			continue;

		dsPlane3d* plane = planes + (volume->planeCount++);
		*plane = viewFrustumd.planes[i];
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

		bool firstAway = dsVector3_dot(first->n, *toLight) < -baseEpsilon;
		bool secondAway = dsVector3_dot(second->n, *toLight) < -baseEpsilon;
		if (firstAway == secondAway)
			continue;

		dsRay3d line;
		if (!dsPlane3d_intersectingLine(&line, first, second))
			continue;

		dsPlane3d boundaryPlane;
		dsVector3_cross(boundaryPlane.n, line.direction, *toLight);
		// Should be roughly aligned with the plane that faces the light.
		const dsVector3d* referenceDir = firstAway ? &second->n : &first->n;
		if (dsVector3_dot(boundaryPlane.n, *referenceDir) < 0)
			dsVector3_neg(boundaryPlane.n, boundaryPlane.n);
		dsVector3d_normalize(&boundaryPlane.n, &boundaryPlane.n);
		boundaryPlane.d = dsVector3_dot(boundaryPlane.n, line.origin);
		addPlane(volume, planes, &boundaryPlane, baseEpsilon);
	}

	computeEdgesAndCorners(volume, planes, baseEpsilon);
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
		dsPlane3d plane;
		dsConvertFloatToDouble(plane, viewFrustum->planes[i]);
		addPlane(volume, planes, &plane, baseEpsilon);

		if (i != dsFrustumPlanes_Near)
		{
			dsConvertFloatToDouble(plane, lightFrustum->planes[i]);
			addPlane(volume, planes, &plane, baseEpsilon);
		}
	}

	computeEdgesAndCorners(volume, planes, baseEpsilon);
	removeUnusedPlanes(volume);
	return true;
}

dsIntersectResult dsShadowCullVolume_intersectAlignedBox(const dsShadowCullVolume* volume,
	const dsAlignedBox3f* box, dsShadowProjection* shadowProj)
{
	if (!volume || !box)
		return dsIntersectResult_Outside;

	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectAlignedBox(volume->planes + i, box);
		switch (planeResult)
		{
			case dsIntersectResult_Outside:
				return dsIntersectResult_Outside;
			case dsIntersectResult_Intersects:
				intersects = true;
				break;
			default:
				break;
		}
	}

	if (shadowProj)
	{
		dsVector3f corners[DS_BOX3_CORNER_COUNT];
		dsAlignedBox3_corners(corners, *box);
		addPointsToProjection(volume, corners, DS_BOX3_CORNER_COUNT, shadowProj, intersects);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsShadowCullVolume_intersectOrientedBox(const dsShadowCullVolume* volume,
	const dsOrientedBox3f* box, dsShadowProjection* shadowProj)
{
	if (!volume || !box)
		return dsIntersectResult_Outside;

	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectOrientedBox(volume->planes + i, box);
		switch (planeResult)
		{
			case dsIntersectResult_Outside:
				return dsIntersectResult_Outside;
			case dsIntersectResult_Intersects:
				intersects = true;
				break;
			default:
				break;
		}
	}

	if (shadowProj)
	{
		dsVector3f corners[DS_BOX3_CORNER_COUNT];
		dsOrientedBox3f_corners(corners, box);
		addPointsToProjection(volume, corners, DS_BOX3_CORNER_COUNT, shadowProj, intersects);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsShadowCullVolume_intersectSphere(const dsShadowCullVolume* volume,
	const dsVector3f* center, float radius, dsShadowProjection* shadowProj)
{
	if (!volume || !center || radius < 0)
		return dsIntersectResult_Outside;

	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		float distance = dsPlane3_distanceToPoint(volume->planes[i], *center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	if (shadowProj)
	{
		dsAlignedBox3f box;
		dsVector3f radiusVec = {{radius, radius, radius}};
		dsVector3_sub(box.min, *center, radiusVec);
		dsVector3_add(box.max, *center, radiusVec);

		dsVector3f corners[DS_BOX3_CORNER_COUNT];
		dsAlignedBox3_corners(corners, box);
		addPointsToProjection(volume, corners, DS_BOX3_CORNER_COUNT, shadowProj, intersects);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
