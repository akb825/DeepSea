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

#include <DeepSea/Geometry/Plane3.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>

void dsPlane3f_normalize(dsPlane3f* result, const dsPlane3f* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);

	float invLength = 1/dsVector3f_len(&plane->n);
	dsVector3_scale(result->n, plane->n, invLength);
	result->d = plane->d*invLength;
}

void dsPlane3d_normalize(dsPlane3d* result, const dsPlane3d* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);

	double invLength = 1/dsVector3d_len(&plane->n);
	dsVector3_scale(result->n, plane->n, invLength);
	result->d = plane->d*invLength;
}

void dsPlane3f_transform(dsPlane3f* result, const dsMatrix44f* transform, const dsPlane3f* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);
	DS_ASSERT(transform);

	dsMatrix44f inverseTransposeTransform;
	dsMatrix44f_inverseTranspose(&inverseTransposeTransform, transform);

	dsVector4f planeVec = {{plane->n.x, plane->n.y, plane->n.z, -plane->d}};
	dsVector4f transformedPlaneVec;
	dsMatrix44_transform(transformedPlaneVec, inverseTransposeTransform, planeVec);

	result->n.x = transformedPlaneVec.x;
	result->n.y = transformedPlaneVec.y;
	result->n.z = transformedPlaneVec.z;
	result->d = -transformedPlaneVec.w;

	dsPlane3f_normalize(result, result);
}

void dsPlane3d_transform(dsPlane3d* result, const dsMatrix44d* transform, const dsPlane3d* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);
	DS_ASSERT(transform);

	dsMatrix44d inverseTransposeTransform;
	dsMatrix44d_inverseTranspose(&inverseTransposeTransform, transform);

	dsVector4d planeVec = {{plane->n.x, plane->n.y, plane->n.z, -plane->d}};
	dsVector4d transformedPlaneVec;
	dsMatrix44_transform(transformedPlaneVec, inverseTransposeTransform, planeVec);

	result->n.x = transformedPlaneVec.x;
	result->n.y = transformedPlaneVec.y;
	result->n.z = transformedPlaneVec.z;
	result->d = -transformedPlaneVec.w;

	dsPlane3d_normalize(result, result);
}

void dsPlane3f_transformInverseTranspose(dsPlane3f* result, const dsMatrix44f* transform,
	const dsPlane3f* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);
	DS_ASSERT(transform);

	dsVector4f planeVec = {{plane->n.x, plane->n.y, plane->n.z, -plane->d}};
	dsVector4f transformedPlaneVec;
	dsMatrix44_transform(transformedPlaneVec, *transform, planeVec);

	result->n.x = transformedPlaneVec.x;
	result->n.y = transformedPlaneVec.y;
	result->n.z = transformedPlaneVec.z;
	result->d = -transformedPlaneVec.w;

	dsPlane3f_normalize(result, result);
}

void dsPlane3d_transformInverseTranspose(dsPlane3d* result, const dsMatrix44d* transform,
	const dsPlane3d* plane)
{
	DS_ASSERT(result);
	DS_ASSERT(plane);
	DS_ASSERT(transform);

	dsVector4d planeVec = {{plane->n.x, plane->n.y, plane->n.z, -plane->d}};
	dsVector4d transformedPlaneVec;
	dsMatrix44_transform(transformedPlaneVec, *transform, planeVec);

	result->n.x = transformedPlaneVec.x;
	result->n.y = transformedPlaneVec.y;
	result->n.z = transformedPlaneVec.z;
	result->d = -transformedPlaneVec.w;

	dsPlane3d_normalize(result, result);
}

dsPlaneSide dsPlane3f_intersectAlignedBox(const dsPlane3f* plane, const dsAlignedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3f minPoint, maxPoint;
	if (plane->n.x >= 0)
	{
		minPoint.x = box->min.x;
		maxPoint.x = box->max.x;
	}
	else
	{
		minPoint.x = box->max.x;
		maxPoint.x = box->min.x;
	}

	if (plane->n.y >= 0)
	{
		minPoint.y = box->min.y;
		maxPoint.y = box->max.y;
	}
	else
	{
		minPoint.y = box->max.y;
		maxPoint.y = box->min.y;
	}

	if (plane->n.z >= 0)
	{
		minPoint.z = box->min.z;
		maxPoint.z = box->max.z;
	}
	else
	{
		minPoint.z = box->max.z;
		maxPoint.z = box->min.z;
	}

	float minD = dsVector3_dot(plane->n, minPoint);
	float maxD = dsVector3_dot(plane->n, maxPoint);

	if (minD >= plane->d)
		return dsPlaneSide_Inside;
	else if (maxD < plane->d)
		return dsPlaneSide_Outside;
	else
		return dsPlaneSide_Intersects;
}

dsPlaneSide dsPlane3d_intersectAlignedBox(const dsPlane3d* plane, const dsAlignedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3d minPoint, maxPoint;
	if (plane->n.x >= 0)
	{
		minPoint.x = box->min.x;
		maxPoint.x = box->max.x;
	}
	else
	{
		minPoint.x = box->max.x;
		maxPoint.x = box->min.x;
	}

	if (plane->n.y >= 0)
	{
		minPoint.y = box->min.y;
		maxPoint.y = box->max.y;
	}
	else
	{
		minPoint.y = box->max.y;
		maxPoint.y = box->min.y;
	}

	if (plane->n.z >= 0)
	{
		minPoint.z = box->min.z;
		maxPoint.z = box->max.z;
	}
	else
	{
		minPoint.z = box->max.z;
		maxPoint.z = box->min.z;
	}

	double minD = dsVector3_dot(plane->n, minPoint);
	double maxD = dsVector3_dot(plane->n, maxPoint);

	if (minD >= plane->d)
		return dsPlaneSide_Inside;
	else if (maxD < plane->d)
		return dsPlaneSide_Outside;
	else
		return dsPlaneSide_Intersects;
}

dsPlaneSide dsPlane3f_intersectOrientedBox(const dsPlane3f* plane, const dsOrientedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	// Do aligned box calculation in the space of the oriented box.
	dsPlane3f transformedPlane;
	dsMatrix33_transformTransposed(transformedPlane.n, box->orientation, plane->n);
	transformedPlane.d = plane->d - dsVector3_dot(plane->n, box->center);

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	return dsPlane3f_intersectAlignedBox(&transformedPlane, &localBox);
}

dsPlaneSide dsPlane3d_intersectOrientedBox(const dsPlane3d* plane, const dsOrientedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	// Do aligned box calculation in the space of the oriented box.
	dsPlane3d transformedPlane;
	dsMatrix33_transformTransposed(transformedPlane.n, box->orientation, plane->n);
	transformedPlane.d = plane->d - dsVector3_dot(plane->n, box->center);

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	return dsPlane3d_intersectAlignedBox(&transformedPlane, &localBox);
}
