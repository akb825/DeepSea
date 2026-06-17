/*
 * Copyright 2016-2026 Aaron Barany
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

#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Core/Assert.h>

#include <DeepSea/Geometry/AlignedBox3x.h>
#include <DeepSea/Geometry/OrientedBox3x.h>
#include <DeepSea/Geometry/Plane3.h>

#include <DeepSea/Math/Vector3x.h>

void dsFrustum3f_normalize(dsFrustum3f* frustum)
{
	DS_ASSERT(frustum);
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
		dsPlane3f_normalize(frustum->planes + i, frustum->planes + i);
}

void dsFrustum3d_normalize(dsFrustum3d* frustum)
{
	DS_ASSERT(frustum);
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
		dsPlane3d_normalize(frustum->planes + i, frustum->planes + i);
}

void dsFrustum3f_transform(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	dsMatrix44f inverse, inverseTranspose;
	dsMatrix44f_affineInvert(&inverse, transform);
	dsMatrix44f_transpose(&inverseTranspose, &inverse);
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3f_transformInverseTranspose(
			frustum->planes + i, &inverseTranspose, frustum->planes + i);
	}
}

void dsFrustum3d_transform(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	dsMatrix44d inverse, inverseTranspose;
	dsMatrix44d_affineInvert(&inverse, transform);
	dsMatrix44_transpose(inverseTranspose, inverse);
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3d_transformInverseTranspose(
			frustum->planes + i, &inverseTranspose, frustum->planes + i);
	}
}

void dsFrustum3f_transformInverseTranspose(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3f_transformInverseTranspose(
			frustum->planes + i, transform, frustum->planes + i);
	}
}

void dsFrustum3d_transformInverseTranspose(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3d_transformInverseTranspose(
			frustum->planes + i, transform, frustum->planes + i);
	}
}

bool dsFrustum3f_isInfinite(const dsFrustum3f* frustum)
{
	DS_ASSERT(frustum);

	float epsilon = 1e-6f;
	dsVector3xf zero = {{0.0f, 0.0f, 0.0f, 0.0f}};
	return dsVector3xf_epsilonEqual(&frustum->planes[dsFrustumPlanes_Far].xyzd, &zero, epsilon);
}

bool dsFrustum3d_isInfinite(const dsFrustum3d* frustum)
{
	DS_ASSERT(frustum);

	double epsilon = 1e-14;
	dsVector3xd zero = {{0.0, 0.0, 0.0, 0.0}};
	return dsVector3xd_epsilonEqual(&frustum->planes[dsFrustumPlanes_Far].xyzd, &zero, epsilon);
}

dsIntersectResult dsFrustum3f_intersectAlignedBox(
	const dsFrustum3f* frustum, const dsAlignedBox3f* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsAlignedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectAlignedBox(
	const dsFrustum3d* frustum, const dsAlignedBox3d* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsAlignedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectAlignedBox3x(
	const dsFrustum3f* frustum, const dsAlignedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3f_intersectAlignedBoxFMA(frustum, box);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsFrustum3f_intersectAlignedBoxSIMD(frustum, box);
#else
	dsMatrix44f boxMatrix;
	dsAlignedBox3xf_toMatrixTranspose(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3d_intersectAlignedBox3x(
	const dsFrustum3d* frustum, const dsAlignedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);
#if DS_SIMD_PREFER_DOUBLE4
	return dsFrustum3d_intersectAlignedBoxSIMD4(frustum, box);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3d_intersectAlignedBoxFMA2(frustum, box);
#else
	return dsFrustum3d_intersectAlignedBoxSIMD2(frustum, box);
#endif
#else
	dsMatrix44d boxMatrix;
	dsAlignedBox3xd_toMatrixTranspose(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3f_intersectOrientedBox(
	const dsFrustum3f* frustum, const dsOrientedBox3f* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectOrientedBox(
	const dsFrustum3d* frustum, const dsOrientedBox3d* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectOrientedBox3x(
	const dsFrustum3f* frustum, const dsOrientedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3f_intersectOrientedBoxFMA(frustum, box);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsFrustum3f_intersectOrientedBoxSIMD(frustum, box);
#else
	dsMatrix44f boxMatrix;
	dsOrientedBox3xf_toMatrixTranspose(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3d_intersectOrientedBox3x(
	const dsFrustum3d* frustum, const dsOrientedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);
#if DS_SIMD_PREFER_DOUBLE4
	return dsFrustum3d_intersectOrientedBoxSIMD4(frustum, box);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3d_intersectOrientedBoxFMA2(frustum, box);
#else
	return dsFrustum3d_intersectOrientedBoxSIMD2(frustum, box);
#endif
#else
	dsMatrix44d boxMatrix;
	dsOrientedBox3xd_toMatrixTranspose(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3f_intersectBoxMatrix(
	const dsFrustum3f* frustum, const dsMatrix44f* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3f_intersectBoxMatrixFMA(frustum, boxMatrix);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsFrustum3f_intersectBoxMatrixSIMD(frustum, boxMatrix);
#else
	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transpose(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3d_intersectBoxMatrix(
	const dsFrustum3d* frustum, const dsMatrix44d* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);
#if DS_SIMD_PREFER_DOUBLE4
	return dsFrustum3d_intersectBoxMatrixSIMD4(frustum, boxMatrix);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3d_intersectBoxMatrixFMA2(frustum, boxMatrix);
#else
	return dsFrustum3d_intersectBoxMatrixSIMD2(frustum, boxMatrix);
#endif
#else
	dsMatrix44d boxMatrixTranspose;
	dsMatrix44_transpose(boxMatrixTranspose, *boxMatrix);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTranspose(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3f_intersectSphere(
	const dsFrustum3f* frustum, const dsVector3f* center, float radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		float distance = dsPlane3f_distanceToPoint(frustum->planes + i, center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectSphere(
	const dsFrustum3d* frustum, const dsVector3d* center, double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		double distance = dsPlane3d_distanceToPoint(frustum->planes + i, center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectSphere3x(
	const dsFrustum3f* frustum, const dsVector3xf* center, float radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3f_intersectSphereFMA(frustum, center, radius);
#elif DS_SIMD_ALWAYS_FLOAT4
	return dsFrustum3f_intersectSphereSIMD(frustum, center, radius);
#else
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		float distance = dsPlane3f_distanceToPoint(frustum->planes + i, (const dsVector3f*)center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

dsIntersectResult dsFrustum3d_intersectSphere3x(
	const dsFrustum3d* frustum, const dsVector3xd* center, double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);
#if DS_SIMD_PREFER_DOUBLE4
	return dsFrustum3d_intersectSphereSIMD4(frustum, center, radius);
#elif DS_SIMD_ALWAYS_DOUBLE2
#if DS_SIMD_ALWAYS_FMA
	return dsFrustum3d_intersectSphereFMA2(frustum, center, radius);
#else
	return dsFrustum3d_intersectSphereSIMD2(frustum, center, radius);
#endif
#else
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		double distance = dsPlane3d_distanceToPoint(frustum->planes + i, (const dsVector3d*)center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
#endif
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)

dsIntersectResult dsFrustum3f_intersectAlignedBoxSIMD(
	const dsFrustum3f* frustum, const dsAlignedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsAlignedBox3xf_toMatrixTransposeSIMD(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectOrientedBoxSIMD(
	const dsFrustum3f* frustum, const dsOrientedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsOrientedBox3xf_toMatrixTransposeSIMD(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectBoxMatrixSIMD(
	const dsFrustum3f* frustum, const dsMatrix44f* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);

	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeSIMD(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectSphereSIMD(
	const dsFrustum3f* frustum, const dsVector3xf* center, float radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	dsSIMD4f centerPos = dsSIMD4f_set4(center->x, center->y, center->z, 1.0f);
	for (int i = 0; i < count; ++i)
	{
		dsSIMD4f dist4 = dsDot4SIMD4f(frustum->planes[i].simd, centerPos);
		float distance = dsSIMD4f_get(dist4, 0);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)

dsIntersectResult dsFrustum3f_intersectAlignedBoxFMA(
	const dsFrustum3f* frustum, const dsAlignedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsAlignedBox3xf_toMatrixTransposeSIMD(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectOrientedBoxFMA(
	const dsFrustum3f* frustum, const dsOrientedBox3xf* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsOrientedBox3xf_toMatrixTransposeSIMD(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectBoxMatrixFMA(
	const dsFrustum3f* frustum, const dsMatrix44f* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);

	dsMatrix44f boxMatrixTranspose;
	dsMatrix44f_transposeSIMD(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectBoxMatrixTransposeFMA(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3f_intersectSphereFMA(
	const dsFrustum3f* frustum, const dsVector3xf* center, float radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	dsSIMD4f centerPos = dsSIMD4f_set4(center->x, center->y, center->z, 1.0f);
	for (int i = 0; i < count; ++i)
	{
		dsSIMD4f dist4 = dsDot4FMA4f(frustum->planes[i].simd, centerPos);
		float distance = dsSIMD4f_get(dist4, 0);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE2)

dsIntersectResult dsFrustum3d_intersectAlignedBoxSIMD2(
	const dsFrustum3d* frustum, const dsAlignedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsAlignedBox3xd_toMatrixTransposeSIMD2(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD2(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectOrientedBoxSIMD2(
	const dsFrustum3d* frustum, const dsOrientedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsOrientedBox3xd_toMatrixTransposeSIMD2(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD2(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectBoxMatrixSIMD2(
	const dsFrustum3d* frustum, const dsMatrix44d* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);

	dsMatrix44d boxMatrixTranspose;
	dsMatrix44d_transposeSIMD2(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD2(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectSphereSIMD2(
	const dsFrustum3d* frustum, const dsVector3xd* center, double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	dsSIMD2d centerPos0 = center->simd2[0];
	dsSIMD2d centerPos1 = dsSIMD2d_set2(center->z, 1.0);
	for (int i = 0; i < count; ++i)
	{
		const dsPlane3d* plane = frustum->planes + i;
		dsSIMD2d dist2 = dsDot4SIMD2d(plane->simd2[0], plane->simd2[1], centerPos0, centerPos1);
		double distance = dsSIMD2d_get(dist2, 0);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()

#if !DS_DETERMINISTIC_MATH
DS_SIMD_START(DS_SIMD_DOUBLE2,DS_SIMD_FMA)

dsIntersectResult dsFrustum3d_intersectAlignedBoxFMA2(
	const dsFrustum3d* frustum, const dsAlignedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsAlignedBox3xd_toMatrixTransposeSIMD2(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeFMA2(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectOrientedBoxFMA2(
	const dsFrustum3d* frustum, const dsOrientedBox3xd* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsOrientedBox3xd_toMatrixTransposeSIMD2(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeFMA2(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectBoxMatrixFMA2(
	const dsFrustum3d* frustum, const dsMatrix44d* boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);

	dsMatrix44d boxMatrixTranspose;
	dsMatrix44d_transposeSIMD2(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeFMA2(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectSphereFMA2(
	const dsFrustum3d* frustum, const dsVector3xd* center, double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	dsSIMD2d centerPos0 = center->simd2[0];
	dsSIMD2d centerPos1 = dsSIMD2d_set2(center->z, 1.0);
	for (int i = 0; i < count; ++i)
	{
		const dsPlane3d* plane = frustum->planes + i;
		dsSIMD2d dist2 = dsDot4FMA2d(plane->simd2[0], plane->simd2[1], centerPos0, centerPos1);
		double distance = dsSIMD2d_get(dist2, 0);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()
#endif // !DS_DETERMINISTIC_MATH

DS_SIMD_START(DS_SIMD_DOUBLE4,DS_SIMD_FMA)

dsIntersectResult dsFrustum3d_intersectAlignedBoxSIMD4(
	const dsFrustum3d* DS_ALIGN_PARAM(32) frustum, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	DS_ALIGN(32) dsMatrix44d boxMatrix;
	dsAlignedBox3xd_toMatrixTransposeSIMD4(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD4(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectOrientedBoxSIMD4(
	const dsFrustum3d* DS_ALIGN_PARAM(32) frustum, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	DS_ALIGN(32) dsMatrix44d boxMatrix;
	dsOrientedBox3xd_toMatrixTransposeSIMD4(&boxMatrix, box);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD4(
			frustum->planes + i, &boxMatrix);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectBoxMatrixSIMD4(
	const dsFrustum3d* DS_ALIGN_PARAM(32) frustum, const dsMatrix44d* DS_ALIGN_PARAM(32) boxMatrix)
{
	DS_ASSERT(frustum);
	DS_ASSERT(boxMatrix);

	DS_ALIGN(32) dsMatrix44d boxMatrixTranspose;
	dsMatrix44d_transposeSIMD4(&boxMatrixTranspose, boxMatrix);
	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectBoxMatrixTransposeSIMD4(
			frustum->planes + i, &boxMatrixTranspose);
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

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectSphereSIMD4(const dsFrustum3d* DS_ALIGN_PARAM(32) frustum,
	const dsVector3xd* DS_ALIGN_PARAM(32) center, double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	dsSIMD4d centerPos = dsSIMD4d_set4(center->x, center->y, center->z, 1.0);
	for (int i = 0; i < count; ++i)
	{
		dsSIMD4d plane = dsSIMD4d_load(frustum->planes + i);
		dsSIMD4d dist4 = dsDot4SIMD4d(plane, centerPos);
		double distance = dsSIMD4d_get(dist4, 0);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

DS_SIMD_END()
#endif // DS_HAS_SIMD

void dsFrustum3f_fromMatrix(
	dsFrustum3f* result, const dsMatrix44f* matrix, dsProjectionMatrixOptions options);
void dsFrustum3d_fromMatrix(
	dsFrustum3d* result, const dsMatrix44d* matrix, dsProjectionMatrixOptions options);
