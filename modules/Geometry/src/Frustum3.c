/*
 * Copyright 2016-2023 Aaron Barany
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
#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Math/Matrix44.h>

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
		dsPlane3f_transformInverseTranspose(frustum->planes + i, &inverseTranspose,
			frustum->planes + i);
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
		dsPlane3d_transformInverseTranspose(frustum->planes + i, &inverseTranspose,
			frustum->planes + i);
	}
}

void dsFrustum3f_transformInverseTranspose(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3f_transformInverseTranspose(frustum->planes + i, transform,
			frustum->planes + i);
	}
}

void dsFrustum3d_transformInverseTranspose(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsPlane3d_transformInverseTranspose(frustum->planes + i, transform,
			frustum->planes + i);
	}
}

bool dsFrustum3f_isInfinite(const dsFrustum3f* frustum)
{
	DS_ASSERT(frustum);

	float epsilon = 1e-6f;
	dsVector3f zero = {{0.0f, 0.0f, 0.0f}};
	return dsVector3f_epsilonEqual(&frustum->planes[dsFrustumPlanes_Far].n, &zero, epsilon);
}

bool dsFrustum3d_isInfinite(const dsFrustum3d* frustum)
{
	DS_ASSERT(frustum);

	double epsilon = 1e-14;
	dsVector3d zero = {{0.0, 0.0, 0.0}};
	return dsVector3d_epsilonEqual(&frustum->planes[dsFrustumPlanes_Far].n, &zero, epsilon);
}

dsIntersectResult dsFrustum3f_intersectAlignedBox(const dsFrustum3f* frustum,
	const dsAlignedBox3f* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectAlignedBox(frustum->planes + i, box);
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

dsIntersectResult dsFrustum3d_intersectAlignedBox(const dsFrustum3d* frustum,
	const dsAlignedBox3d* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectAlignedBox(frustum->planes + i, box);
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

dsIntersectResult dsFrustum3f_intersectOrientedBox(const dsFrustum3f* frustum,
	const dsOrientedBox3f* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3f_intersectOrientedBox(frustum->planes + i, box);
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

dsIntersectResult dsFrustum3d_intersectOrientedBox(const dsFrustum3d* frustum,
	const dsOrientedBox3d* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		dsIntersectResult planeResult = dsPlane3d_intersectOrientedBox(frustum->planes + i, box);
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

dsIntersectResult dsFrustum3f_intersectSphere(const dsFrustum3f* frustum, const dsVector3f* center,
	float radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3f_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		float distance = dsPlane3_distanceToPoint(frustum->planes[i], *center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

dsIntersectResult dsFrustum3d_intersectSphere(const dsFrustum3d* frustum, const dsVector3d* center,
	double radius)
{
	DS_ASSERT(frustum);
	DS_ASSERT(center);

	bool intersects = false;
	int count = dsFrustum3d_isInfinite(frustum) ? dsFrustumPlanes_Far : dsFrustumPlanes_Count;
	for (int i = 0; i < count; ++i)
	{
		double distance = dsPlane3_distanceToPoint(frustum->planes[i], *center);
		if (distance < -radius)
			return dsIntersectResult_Outside;
		else if (distance <= radius)
			intersects = true;
	}

	return intersects ? dsIntersectResult_Intersects : dsIntersectResult_Inside;
}

void dsFrustum3f_fromMatrix(dsFrustum3f* result, const dsMatrix44f* matrix,
	dsProjectionMatrixOptions options);
void dsFrustum3d_fromMatrix(dsFrustum3d* result, const dsMatrix44d* matrix,
	dsProjectionMatrixOptions options);
