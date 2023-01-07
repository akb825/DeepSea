/*
 * Copyright 2021-2023 Aaron Barany
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
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Geometry/Ray3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Shadows/ShadowProjection.h>

#include <float.h>

#define SHADOW_BOX_SEGMENTS 12
// This maximum should never occur, but should be safe regardless.
#define MAX_ADDED_SHADOW_POINTS (SHADOW_BOX_SEGMENTS*2 + DS_MAX_SHADOW_CULL_CORNERS)

typedef bool (*PointInBoxFunction)(const void* box, const dsVector3f* point);

// Since the original computations were done with floats, be a bit loose with the epsilon values.
const double baseEpsilon = 1e-5;

static dsVector4f normalizedBoxCorners[DS_BOX3_CORNER_COUNT] =
{
	{{-1, -1, -1, 1}},
	{{-1, -1,  1, 1}},
	{{-1,  1, -1, 1}},
	{{-1,  1,  1, 1}},
	{{ 1, -1, -1, 1}},
	{{ 1, -1,  1, 1}},
	{{ 1,  1, -1, 1}},
	{{ 1,  1,  1, 1}},
};

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

	return -(dsVector3_dot(plane->n, ray->origin) + plane->d)/denom;
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

static inline void boxMatrixCorners(dsVector3f outCorners[DS_BOX3_CORNER_COUNT],
	const dsMatrix44f* boxMatrix)
{
	dsVector4f boxCorner;
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsMatrix44f_transform(&boxCorner, boxMatrix, normalizedBoxCorners + i);
		outCorners[i] = *(dsVector3f*)&boxCorner;
	}
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
static inline void boxMatrixCornersSIMD(dsVector4f outCorners[DS_BOX3_CORNER_COUNT],
	const dsMatrix44f* boxMatrix)
{
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsMatrix44f_transformSIMD(outCorners + i, boxMatrix, normalizedBoxCorners + i);
}
DS_SIMD_END()

DS_SIMD_START_FMA()
static inline void boxMatrixCornersFMA(dsVector4f outCorners[DS_BOX3_CORNER_COUNT],
	const dsMatrix44f* boxMatrix)
{
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsMatrix44f_transformFMA(outCorners + i, boxMatrix, normalizedBoxCorners + i);
}
DS_SIMD_END()
#endif

static void addClampedPointsToProjection(const dsShadowCullVolume* volume,
	const float* corners, unsigned int components, dsShadowProjection* shadowProj,
	PointInBoxFunction pointInBoxFunc, const void* box)
{
	// Limit the segments of the box with the cull volume. When a corner of the volume lies inside
	// the box use that to handle very large boxes. However, there are still some other corner cases
	// that aren't caught, so only recommended for larger bounds that would otherwise cause the
	// shadow projection to be too large.
	typedef dsBox3Corner CornerPair[2];
	static const CornerPair segmentCorners[SHADOW_BOX_SEGMENTS] =
	{
		// Bottom loop.
		{dsBox3Corner_xyz, dsBox3Corner_Xyz},
		{dsBox3Corner_Xyz, dsBox3Corner_XYz},
		{dsBox3Corner_XYz, dsBox3Corner_xYz},
		{dsBox3Corner_xYz, dsBox3Corner_xyz},
		// Top loop.
		{dsBox3Corner_xyZ, dsBox3Corner_XyZ},
		{dsBox3Corner_XyZ, dsBox3Corner_XYZ},
		{dsBox3Corner_XYZ, dsBox3Corner_xYZ},
		{dsBox3Corner_xYZ, dsBox3Corner_xyZ},
		// Connecting bottom and top.
		{dsBox3Corner_xyz, dsBox3Corner_xyZ},
		{dsBox3Corner_xYz, dsBox3Corner_xYZ},
		{dsBox3Corner_Xyz, dsBox3Corner_XyZ},
		{dsBox3Corner_XYz, dsBox3Corner_XYZ}
	};

	// Points that aren't limited by plains are only added for the min points of the loops along the
	// top and bottom of the box.
	const uint32_t topBottomCount = 8;

	dsVector3f points[MAX_ADDED_SHADOW_POINTS];
	uint32_t pointCount = 0;
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(segmentCorners); ++i)
	{
		const CornerPair* curCorners = segmentCorners + i;
		dsRay3f segmentRay;
		segmentRay.origin = *(const dsVector3f*)(corners + (*curCorners)[0]*components);
		const dsVector3f* end = (const dsVector3f*)(corners + (*curCorners)[1]*components);
		dsVector3_sub(segmentRay.direction, *end, segmentRay.origin);

		// Find the extents of the segment that intersect with the cull volume.
		bool valid = true;
		float minT = 0.0f;
		float maxT = 1.0f;
		for (uint32_t j = 0; j < volume->planeCount; ++j)
		{
			const dsPlane3f* plane = volume->planes + j;
			float t = dsPlane3f_rayIntersection(plane, &segmentRay);
			if (t == FLT_MAX)
			{
				// If parallel, check if the ray is inside the plane.
				if (dsPlane3_distanceToPoint(*plane, segmentRay.origin) < 0)
				{
					valid = false;
					break;
				}
				continue;
			}

			if (dsVector3_dot(plane->n, segmentRay.direction) > 0)
				minT = dsMax(minT, t);
			else
				maxT = dsMin(maxT, t);
		}

		if (!valid || minT >= maxT)
			continue;

		if (minT > 0.0f || i < topBottomCount)
		{
			dsVector3f* newPoint = points + (pointCount++);
			dsRay3_evaluate(*newPoint, segmentRay, minT);
		}

		if (maxT < 1.0f)
		{
			dsVector3f* newPoint = points + (pointCount++);
			dsRay3_evaluate(*newPoint, segmentRay, maxT);
		}
	}

	// Check for any corners of the volume that lie inside the box. Otherwise large boxes or boxes
	// along corners will miss too many points during segment intersections.
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsVector3f* corner = &volume->corners[i].point;
		if (pointInBoxFunc(box, corner))
			points[pointCount++] = *corner;
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
		{dsFrustumPlanes_Left, dsFrustumPlanes_Bottom},
		{dsFrustumPlanes_Bottom, dsFrustumPlanes_Right},
		{dsFrustumPlanes_Right, dsFrustumPlanes_Top},
		{dsFrustumPlanes_Top, dsFrustumPlanes_Left},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Left},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Right},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Bottom},
		{dsFrustumPlanes_Far, dsFrustumPlanes_Top},
	};

	unsigned int count = DS_ARRAY_SIZE(boundaries);
	// Ignore far planes when infinite.
	if (dsFrustum3f_isInfinite(viewFrustum))
		count -= 4;
	for (unsigned int i = 0; i < count; ++i)
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
		// Should face roughly the same direction to the plane it's most closely aligned with.
		dsVector3d_normalize(&boundaryPlane.n, &boundaryPlane.n);
		double dotFirst = dsVector3_dot(boundaryPlane.n, first->n);
		double dotSecond = dsVector3_dot(boundaryPlane.n, second->n);
		bool flip = fabs(dotFirst) > fabs(dotSecond) ? dotFirst < 0 : dotSecond < 0;
		if (flip)
			dsVector3_neg(boundaryPlane.n, boundaryPlane.n);
		boundaryPlane.d = -dsVector3_dot(boundaryPlane.n, line.origin);
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
		if (i != dsFrustumPlanes_Far || !dsFrustum3f_isInfinite(viewFrustum))
		{
			dsConvertFloatToDouble(plane, viewFrustum->planes[i]);
			addPlane(volume, planes, &plane, baseEpsilon);
		}

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
	const dsAlignedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsAlignedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(volume->planes + i,
			&boxMatrix);
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
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 3, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3f_containsPoint, box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
dsIntersectResult dsShadowCullVolume_intersectAlignedBoxSIMD(const dsShadowCullVolume* volume,
	const dsAlignedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsAlignedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			volume->planes + i, &boxMatrix);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		dsAlignedBox3_corners(corners, *box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3f_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()

DS_SIMD_START_FMA()
dsIntersectResult dsShadowCullVolume_intersectAlignedBoxFMA(const dsShadowCullVolume* volume,
	const dsAlignedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsAlignedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
			volume->planes + i, &boxMatrix);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		dsAlignedBox3_corners(corners, *box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3f_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()
#endif

dsIntersectResult dsShadowCullVolume_intersectOrientedBox(const dsShadowCullVolume* volume,
	const dsOrientedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(volume->planes + i,
			&boxMatrix);
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
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 3, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
dsIntersectResult dsShadowCullVolume_intersectOrientedBoxSIMD(const dsShadowCullVolume* volume,
	const dsOrientedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			volume->planes + i, &boxMatrix);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		boxMatrixCornersSIMD(corners, &boxMatrix);
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()

DS_SIMD_START_FMA()
dsIntersectResult dsShadowCullVolume_intersectOrientedBoxFMA(const dsShadowCullVolume* volume,
	const dsOrientedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
			volume->planes + i, &boxMatrix);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		boxMatrixCornersFMA(corners, &boxMatrix);
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()
#endif

dsIntersectResult dsShadowCullVolume_intersectBoxMatrix(const dsShadowCullVolume* volume,
	const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !boxMatrix)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transpose(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(volume->planes + i,
			&boxMatrixTranspose);
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
		boxMatrixCorners(corners, boxMatrix);
		if (clampToVolume && intersects)
		{
			dsOrientedBox3f box;
			dsOrientedBox3f_fromMatrix(&box, boxMatrix);
			addClampedPointsToProjection(volume, (const float*)corners, 3, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, &box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
dsIntersectResult dsShadowCullVolume_intersectBoxMatrixSIMD(const dsShadowCullVolume* volume,
	const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !boxMatrix)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			volume->planes + i, &boxMatrixTranspose);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		boxMatrixCornersSIMD(corners, boxMatrix);
		if (clampToVolume && intersects)
		{
			dsOrientedBox3f box;
			dsOrientedBox3f_fromMatrix(&box, boxMatrix);
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, &box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()

DS_SIMD_START_FMA()
dsIntersectResult dsShadowCullVolume_intersectBoxMatrixFMA(const dsShadowCullVolume* volume,
	const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !boxMatrix)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			volume->planes + i, &boxMatrixTranspose);
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
		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		boxMatrixCornersFMA(corners, boxMatrix);
		if (clampToVolume && intersects)
		{
			dsOrientedBox3f box;
			dsOrientedBox3f_fromMatrix(&box, boxMatrix);
			addClampedPointsToProjection(volume, (const float*)corners, 4, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3f_containsPoint, &box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
DS_SIMD_END()
#endif

dsIntersectResult dsShadowCullVolume_intersectSphere(const dsShadowCullVolume* volume,
	const dsVector3f* center, float radius, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !center || radius < 0)
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
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, (const float*)corners, 3, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3f_containsPoint, &box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}
