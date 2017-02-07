/*
 * Copyright 2016 Aaron Barany
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
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
		dsPlane3f_normalize(frustum->planes + i, frustum->planes + i);
}

void dsFrustum3d_normalize(dsFrustum3d* frustum)
{
	DS_ASSERT(frustum);
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
		dsPlane3d_normalize(frustum->planes + i, frustum->planes + i);
}

void dsFrustum3f_transform(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	dsMatrix44f inverseTranspose;
	dsMatrix44f_inverseTranspose(&inverseTranspose, transform);
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		dsPlane3f_transformInverseTranspose(frustum->planes + i, &inverseTranspose,
			frustum->planes + i);
	}
}

void dsFrustum3d_transform(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	dsMatrix44d inverseTranspose;
	dsMatrix44d_inverseTranspose(&inverseTranspose, transform);
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		dsPlane3d_transformInverseTranspose(frustum->planes + i, &inverseTranspose,
			frustum->planes + i);
	}
}

void dsFrustum3f_transformInverseTranspose(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
		dsPlane3f_transformInverseTranspose(frustum->planes + i, transform, frustum->planes + i);
}

void dsFrustum3d_transformInverseTranspose(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	DS_ASSERT(frustum);
	DS_ASSERT(transform);

	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
		dsPlane3d_transformInverseTranspose(frustum->planes + i, transform, frustum->planes + i);
}

dsIntersectResult dsFrustum3f_intersectAlignedBox(const dsFrustum3f* frustum,
	const dsAlignedBox3f* box)
{
	DS_ASSERT(frustum);
	DS_ASSERT(box);

	bool intersects = false;
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
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
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
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
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
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
	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
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

DS_GEOMETRY_EXPORT void dsFrustum3f_fromMatrix(dsFrustum3f* result, const dsMatrix44f* matrix,
	bool halfDepth);
DS_GEOMETRY_EXPORT void dsFrustum3d_fromMatrix(dsFrustum3d* result, const dsMatrix44d* matrix,
	bool halfDepth);
