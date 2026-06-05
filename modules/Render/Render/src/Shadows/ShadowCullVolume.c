/*
 * Copyright 2021-2026 Aaron Barany
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

#include <DeepSea/Geometry/AlignedBox3x.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Geometry/OrientedBox3x.h>
#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Geometry/Ray3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3x.h>

#include <DeepSea/Render/Shadows/ShadowProjection.h>

#include <float.h>

#define SHADOW_BOX_SEGMENTS 12
// This maximum should never occur, but should be safe regardless.
#define MAX_ADDED_SHADOW_POINTS (SHADOW_BOX_SEGMENTS*2 + DS_MAX_SHADOW_CULL_CORNERS)

// Since the original computations were done with floats, be a bit loose with the epsilon values.
#define BASE_EPSILON 1e-5

typedef bool (*PointInBoxFunction)(const void* box, const dsVector3xf* point);

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
	const dsVector3xd* point, double epsilon)
{
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		double distance = dsPlane3d_distanceToPoint(planes + i, (const dsVector3d*)point);
		if (distance < -epsilon)
			return false;
	}
	return true;
}

static void addPlane(
	dsShadowCullVolume* volume, dsPlane3d* planes, const dsPlane3d* plane, double epsilon)
{
	DS_ASSERT(volume->planeCount < DS_MAX_SHADOW_CULL_PLANES);
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		const dsPlane3d* curPlane = planes + i;
		if (dsVector4d_epsilonEqual((const dsVector4d*)curPlane, (const dsVector4d*)plane, epsilon))
			return;
	}

	planes[volume->planeCount++] = *plane;
}

static uint32_t bitmaskTripple(uint32_t p0, uint32_t p1, uint32_t p2)
{
	return (1 << p0) | (1 << p1) | (1 << p2);
}

static void addCorner(dsShadowCullVolume* volume, dsVector3xd* cornerPoints,
	const dsVector3xd* point, uint32_t planes, double epsilon)
{
	DS_ASSERT(volume->cornerCount < DS_MAX_SHADOW_CULL_CORNERS);
	// Check if we already have a corner for the plane triplet.
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		dsShadowCullCorner* corner = volume->corners + i;
		if ((corner->planes & planes) == planes)
			return;
		else if (dsVector3xd_relativeEpsilonEqual(cornerPoints + i, point, epsilon, epsilon))
		{
			corner->planes |= planes;
			return;
		}
	}

	cornerPoints[volume->cornerCount] = *point;
	dsShadowCullCorner* corner = volume->corners + (volume->cornerCount++);
	corner->point.x = (float)point->x;
	corner->point.y = (float)point->y;
	corner->point.z = (float)point->z;
	corner->planes = planes;
}

static void computeCorners(dsShadowCullVolume* volume, const dsPlane3d* planes, double epsilon)
{
	for (uint32_t i = 0; i < volume->planeCount; ++i)
		dsConvertDoubleToFloat(volume->planes[i], planes[i]);

	double epsilon2 = dsPow2(epsilon);
	dsVector3xd cornerPoints[DS_MAX_SHADOW_CULL_CORNERS];
	for (uint32_t i = 0; i < volume->planeCount - 1; ++i)
	{
		const dsPlane3d* firstPlane = planes + i;
		for (uint32_t j = i + 1; j < volume->planeCount; ++j)
		{
			const dsPlane3d* secondPlane = planes + j;
			dsRay3d ray;
			if (!dsPlane3d_intersectingLine(&ray, firstPlane, secondPlane))
				continue;

			for (uint32_t k = j + 1; k < volume->planeCount; ++k)
			{
				const dsPlane3d* thirdPlane = planes + k;

				// Relaxed intersection check to avoid nearly infinite points.
				double denom = dsVector3xd_dot(&thirdPlane->xyzd, &ray.direction);
				if (fabs(denom) < epsilon2)
					continue;

				dsVector4d origin = ray.origin;
				origin.w = 1.0;
				double t = -dsVector4d_dot(&thirdPlane->xyzd, &origin)/denom;
				dsVector3xd point;
				dsRay3d_evaluate3x(&point, &ray, t);
				if (pointInVolume(volume, planes, &point, epsilon))
					addCorner(volume, cornerPoints, &point, bitmaskTripple(i, j, k), epsilon);
			}
		}
	}
}

static void removeUnusedPlanes(dsShadowCullVolume* volume)
{
	// If no corners the full shadow volume is out of view.
	if (volume->cornerCount == 0)
	{
		volume->planeCount = 0;
		return;
	}

	for (uint32_t i = 0; i < volume->planeCount;)
	{
		uint32_t planeMask = 1 << i;
		// Remove any plane that isn't referenced by a corner.
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
		for (uint32_t j = 0; j < volume->cornerCount; ++j)
		{
			dsShadowCullCorner* corner = volume->corners + j;
			corner->planes = (corner->planes & prevPlanesMask) |
				((corner->planes & ~prevPlanesMask) >> 1);
		}

		--volume->planeCount;
	}
}

#if !DS_SIMD_ALWAYS_FLOAT4

static inline void boxMatrixCorners(
	dsVector4f outCorners[DS_BOX3_CORNER_COUNT], const dsMatrix44f* boxMatrix)
{
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsMatrix44f_transform(outCorners + i, boxMatrix, normalizedBoxCorners + i);
}

#endif // !DS_SIMD_ALWAYS_FLOAT4

#if DS_HAS_SIMD

DS_SIMD_START(DS_SIMD_FLOAT4)
static inline void boxMatrixCornersSIMD(
	dsVector4f outCorners[DS_BOX3_CORNER_COUNT], const dsMatrix44f* boxMatrix)
{
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsMatrix44f_transformSIMD(outCorners + i, boxMatrix, normalizedBoxCorners + i);
}
DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static inline void boxMatrixCornersFMA(
	dsVector4f outCorners[DS_BOX3_CORNER_COUNT], const dsMatrix44f* boxMatrix)
{
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		dsMatrix44f_transformFMA(outCorners + i, boxMatrix, normalizedBoxCorners + i);
}
DS_SIMD_END()

#endif // !DS_DETERMINISTIC_MATH
#endif // DS_HAS_SIMD

static void addClampedPointsToProjection(const dsShadowCullVolume* volume,
	const dsVector3xf* corners, dsShadowProjection* shadowProj, PointInBoxFunction pointInBoxFunc,
	const void* box)
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

	dsVector4f points[MAX_ADDED_SHADOW_POINTS];
	uint32_t pointCount = 0;
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(segmentCorners); ++i)
	{
		const CornerPair* curCorners = segmentCorners + i;
		dsRay3f segmentRay;
		segmentRay.origin = corners[(*curCorners)[0]];
		const dsVector3xf* end = corners + (*curCorners)[1];
		dsVector3xf_sub(&segmentRay.direction, end, &segmentRay.origin);

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
				if (dsPlane3f_distanceToPoint(plane, (const dsVector3f*)&segmentRay.origin) < 0)
				{
					valid = false;
					break;
				}
				continue;
			}

			if (dsVector3xf_dot(&plane->xyzd, &segmentRay.direction) > 0)
				minT = dsMax(minT, t);
			else
				maxT = dsMin(maxT, t);
		}

		if (!valid || minT >= maxT)
			continue;

		if (minT > 0.0f || i < topBottomCount)
		{
			dsVector4f* newPoint = points + (pointCount++);
			dsRay3f_evaluate3x(newPoint, &segmentRay, minT);
			newPoint->w = 1.0;
		}

		if (maxT < 1.0f)
		{
			dsVector4f* newPoint = points + (pointCount++);
			dsRay3f_evaluate3x(newPoint, &segmentRay, maxT);
			newPoint->w = 1.0;
		}
	}

	// Check for any corners of the volume that lie inside the box. Otherwise large boxes or boxes
	// along corners will miss too many points during segment intersections.
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsVector3f* corner = &volume->corners[i].point;
		dsVector4f point = {{corner->x, corner->y, corner->z, 1.0f}};
		if (pointInBoxFunc(box, &point))
			points[pointCount++] = point;
	}

	DS_VERIFY(dsShadowProjection_addPoints(shadowProj, points, pointCount));
}

bool dsShadowCullVolume_buildDirectional(
	dsShadowCullVolume* volume, const dsFrustum3f* viewFrustum, const dsVector3f* toLight)
{
	if (!volume || !viewFrustum || !toLight)
	{
		errno = EINVAL;
		return false;
	}

	volume->planeCount = 0;
	volume->cornerCount = 0;

	// Use doubles for intersections to avoid large frustums causing numeric instability.
	dsFrustum3d viewFrustumd;
	dsConvertFloatToDouble(viewFrustumd, *viewFrustum);
	dsPlane3d planes[DS_MAX_SHADOW_CULL_PLANES];

	// Add any planes that face the light.
	dsVector3xd toLight3d = {{toLight->x, toLight->y, toLight->z}};
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		if (dsVector3xd_dot(&viewFrustumd.planes[i].xyzd, &toLight3d) < -BASE_EPSILON)
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

		bool firstAway = dsVector3xd_dot(&first->xyzd, &toLight3d) < -BASE_EPSILON;
		bool secondAway = dsVector3xd_dot(&second->xyzd, &toLight3d) < -BASE_EPSILON;
		if (firstAway == secondAway)
			continue;

		dsRay3d line;
		if (!dsPlane3d_intersectingLine(&line, first, second))
			continue;

		dsPlane3d boundaryPlane;
		dsVector3xd_cross(&boundaryPlane.xyzd, &line.direction, &toLight3d);
		// Should face roughly the same direction to the plane it's most closely aligned with.
		dsVector3xd_normalize(&boundaryPlane.xyzd, &boundaryPlane.xyzd);
		double dotFirst = dsVector3xd_dot(&boundaryPlane.xyzd, &first->xyzd);
		double dotSecond = dsVector3xd_dot(&boundaryPlane.xyzd, &second->xyzd);
		bool flip = fabs(dotFirst) > fabs(dotSecond) ? dotFirst < 0 : dotSecond < 0;
		if (flip)
			dsVector3xd_neg(&boundaryPlane.xyzd, &boundaryPlane.xyzd);
		boundaryPlane.d = -dsVector3xd_dot(&boundaryPlane.xyzd, &line.origin);
		addPlane(volume, planes, &boundaryPlane, BASE_EPSILON);
	}

	computeCorners(volume, planes, BASE_EPSILON);
	return true;
}

bool dsShadowCullVolume_buildSpot(
	dsShadowCullVolume* volume, const dsFrustum3f* viewFrustum, const dsFrustum3f* lightFrustum)
{
	if (!volume || !viewFrustum || !lightFrustum)
	{
		errno = EINVAL;
		return false;
	}

	volume->planeCount = 0;
	volume->cornerCount = 0;

	// Add the planes from both the view frustum and light frustum (minus the near plane for the
	// light), then let the corner computation take care of the rest.
	dsPlane3d planes[DS_MAX_SHADOW_CULL_PLANES];
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		dsPlane3d plane;
		if (i != dsFrustumPlanes_Far || !dsFrustum3f_isInfinite(viewFrustum))
		{
			dsConvertFloatToDouble(plane, viewFrustum->planes[i]);
			addPlane(volume, planes, &plane, BASE_EPSILON);
		}

		if (i != dsFrustumPlanes_Near)
		{
			dsConvertFloatToDouble(plane, lightFrustum->planes[i]);
			addPlane(volume, planes, &plane, BASE_EPSILON);
		}
	}

	computeCorners(volume, planes, BASE_EPSILON);
	removeUnusedPlanes(volume);
	return true;
}

dsIntersectResult dsShadowCullVolume_intersectAlignedBox(const dsShadowCullVolume* volume,
	const dsAlignedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsAlignedBox3xf box3x = {{{box->min.x, box->min.y, box->min.z}},
		{{box->max.x, box->max.y, box->max.z}}};
	return dsShadowCullVolume_intersectAlignedBox3x(volume, &box3x, shadowProj, clampToVolume);
}

dsIntersectResult dsShadowCullVolume_intersectAlignedBox3x(const dsShadowCullVolume* volume,
	const dsAlignedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
#if DS_SIMD_ALWAYS_FMA
	return dsShadowCullVolume_intersectAlignedBoxFMA(volume, box, shadowProj, clampToVolume);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsShadowCullVolume_intersectAlignedBoxSIMD(volume, box, shadowProj, clampToVolume);
#else
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix;
	dsAlignedBox3xf_toMatrixTranspose(&boxMatrix, box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
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
		dsAlignedBox3xf_corners(corners, box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1.0f;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3xf_containsPoint, box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsShadowCullVolume_intersectOrientedBox(const dsShadowCullVolume* volume,
	const dsOrientedBox3f* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsOrientedBox3xf box3x =
	{
		{{
			{box->orientation.values[0][0], box->orientation.values[0][1],
				box->orientation.values[0][2]},
			{box->orientation.values[1][0], box->orientation.values[1][1],
				box->orientation.values[1][2]},
			{box->orientation.values[2][0], box->orientation.values[2][1],
				box->orientation.values[2][2]}
		}},
		{{box->center.x, box->center.y, box->center.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};
	return dsShadowCullVolume_intersectOrientedBox3x(volume, &box3x, shadowProj, clampToVolume);
}

dsIntersectResult dsShadowCullVolume_intersectOrientedBox3x(const dsShadowCullVolume* volume,
	const dsOrientedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
#if DS_SIMD_ALWAYS_FMA
	return dsShadowCullVolume_intersectOrientedBoxFMA(volume, box, shadowProj, clampToVolume);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsShadowCullVolume_intersectOrientedBoxSIMD(volume, box, shadowProj, clampToVolume);
#else
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsOrientedBox3xf_toMatrixTranspose(&boxMatrixTranspose, box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
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
		dsOrientedBox3xf_corners(corners, box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1.0f;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsShadowCullVolume_intersectBoxMatrix(const dsShadowCullVolume* volume,
	const dsMatrix44f* boxMatrix, dsShadowProjection* shadowProj, bool clampToVolume)
{
#if DS_SIMD_ALWAYS_FMA
	return dsShadowCullVolume_intersectBoxMatrixFMA(volume, boxMatrix, shadowProj, clampToVolume);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsShadowCullVolume_intersectBoxMatrixSIMD(volume, boxMatrix, shadowProj, clampToVolume);
#else
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
		dsVector3xf corners[DS_BOX3_CORNER_COUNT];
		boxMatrixCorners(corners, boxMatrix);
		if (clampToVolume && intersects)
		{
			dsOrientedBox3xf box;
			dsOrientedBox3xf_fromMatrix(&box, boxMatrix);
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, &box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsShadowCullVolume_intersectSphere(const dsShadowCullVolume* volume,
	const dsVector3f* center, float radius, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !center || radius < 0)
		return dsIntersectResult_Outside;

	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		float distance = dsPlane3f_distanceToPoint(volume->planes + i, center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	if (shadowProj)
	{
		dsAlignedBox3xf box;
		dsVector3xf center3x = {{center->x, center->y, center->z}};
		dsVector3xf radiusVec = {{radius, radius, radius}};
		dsVector3xf_sub(&box.min, &center3x, &radiusVec);
		dsVector3xf_add(&box.max, &center3x, &radiusVec);

		dsVector4f corners[DS_BOX3_CORNER_COUNT];
		dsAlignedBox3xf_corners(corners, &box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1.0f;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3xf_containsPoint, &box);
		}
		else
			DS_VERIFY(dsShadowProjection_addPoints(shadowProj, corners, DS_BOX3_CORNER_COUNT));
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}


#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)

dsIntersectResult dsShadowCullVolume_intersectAlignedBoxSIMD(const dsShadowCullVolume* volume,
	const dsAlignedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsAlignedBox3xf_toMatrixTransposeSIMD(&boxMatrixTranspose, box);
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
		dsAlignedBox3xf_corners(corners, box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1.0f;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3xf_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsShadowCullVolume_intersectOrientedBoxSIMD(const dsShadowCullVolume* volume,
	const dsOrientedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix, boxMatrixTranspose;
	dsOrientedBox3xf_toMatrixSIMD(&boxMatrix, box);
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, &boxMatrix);
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
		boxMatrixCornersSIMD(corners, &boxMatrix);
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

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
			dsOrientedBox3xf box;
			dsOrientedBox3xf_fromMatrixSIMD(&box, boxMatrix);
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, &box);
		}
		else
			dsShadowProjection_addPointsSIMD(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

dsIntersectResult dsShadowCullVolume_intersectAlignedBoxFMA(const dsShadowCullVolume* volume,
	const dsAlignedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrixTranspose;
	dsAlignedBox3xf_toMatrixTransposeSIMD(&boxMatrixTranspose, box);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
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
		dsAlignedBox3xf_corners(corners, box);
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
			corners[i].w = 1.0f;
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsAlignedBox3xf_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsShadowCullVolume_intersectOrientedBoxFMA(const dsShadowCullVolume* volume,
	const dsOrientedBox3xf* box, dsShadowProjection* shadowProj, bool clampToVolume)
{
	if (!volume || volume->planeCount == 0 || !box)
		return dsIntersectResult_Outside;

	dsMatrix44f boxMatrix, boxMatrixTranspose;
	dsOrientedBox3xf_toMatrixSIMD(&boxMatrix, box);
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, &boxMatrix);
	bool intersects = false;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
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
		boxMatrixCornersFMA(corners, &boxMatrix);
		if (clampToVolume && intersects)
		{
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

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
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
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
			dsOrientedBox3xf box;
			dsOrientedBox3xf_fromMatrixFMA(&box, boxMatrix);
			addClampedPointsToProjection(volume, corners, shadowProj,
				(PointInBoxFunction)&dsOrientedBox3xf_containsPoint, &box);
		}
		else
			dsShadowProjection_addPointsFMA(shadowProj, corners, DS_BOX3_CORNER_COUNT);
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH
#endif // DS_HAS_SIMD
