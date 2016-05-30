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

static dsPlaneSide intersectBoxImplf(const dsPlane3f* plane, const dsVector3f* points)
{
	bool inside = false, outside = false;
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT && (!inside || !outside); ++i)
	{
		if (dsVector3_dot(points[i], plane->n) >= plane->d)
			inside = true;
		else
			outside = true;
	}

	if (inside && outside)
		return dsPlaneSide_Intersects;
	else if (inside)
		return dsPlaneSide_Inside;
	else
	{
		DS_ASSERT(outside);
		return dsPlaneSide_Outside;
	}
}

static dsPlaneSide intersectBoxImpld(const dsPlane3d* plane, const dsVector3d* points)
{
	bool inside = false, outside = false;
	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT && (!inside || !outside); ++i)
	{
		if (dsVector3_dot(points[i], plane->n) >= plane->d)
			inside = true;
		else
			outside = true;
	}

	if (inside && outside)
		return dsPlaneSide_Intersects;
	else if (inside)
		return dsPlaneSide_Inside;
	else
	{
		DS_ASSERT(outside);
		return dsPlaneSide_Outside;
	}
}

dsPlaneSide dsPlane3f_intersectAlignedBox(const dsPlane3f* plane, const dsAlignedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, *box);
	return intersectBoxImplf(plane, corners);
}

dsPlaneSide dsPlane3d_intersectAlignedBox(const dsPlane3d* plane, const dsAlignedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3d corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, *box);
	return intersectBoxImpld(plane, corners);
}

dsPlaneSide dsPlane3f_intersectOrientedBox(const dsPlane3f* plane, const dsOrientedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3f_corners(corners, box);
	return intersectBoxImplf(plane, corners);
}

dsPlaneSide dsPlane3d_intersectOrientedBox(const dsPlane3d* plane, const dsOrientedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3d corners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3d_corners(corners, box);
	return intersectBoxImpld(plane, corners);
}
