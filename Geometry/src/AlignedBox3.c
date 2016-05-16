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
 * limitations under the License.OO
 */

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Core/Assert.h>
#include <float.h>
#include <limits.h>

void dsAlignedBox3f_makeInvalid(dsAlignedBox3f* result)
{
	DS_ASSERT(result);

	result->min.x = FLT_MAX;
	result->min.y = FLT_MAX;
	result->min.z = FLT_MAX;
	result->max.x = -FLT_MAX;
	result->max.y = -FLT_MAX;
	result->max.z = -FLT_MAX;
}

void dsAlignedBox3d_makeInvalid(dsAlignedBox3d* result)
{
	DS_ASSERT(result);

	result->min.x = DBL_MAX;
	result->min.y = DBL_MAX;
	result->min.z = DBL_MAX;
	result->max.x = -DBL_MAX;
	result->max.y = -DBL_MAX;
	result->max.z = -DBL_MAX;
}

void dsAlignedBox3i_makeInvalid(dsAlignedBox3i* result)
{
	DS_ASSERT(result);

	result->min.x = INT_MAX;
	result->min.y = INT_MAX;
	result->min.z = INT_MAX;
	result->max.x = INT_MIN;
	result->max.y = INT_MIN;
	result->max.z = INT_MIN;
}

float dsAlignedBox3f_distance2ToPoint(const dsAlignedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	float dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	float dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	float dz = dsMax(box->min.z - point->z, point->z - box->max.z);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

double dsAlignedBox3d_distance2ToPoint(const dsAlignedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	double dz = dsMax(box->min.z - point->z, point->z - box->max.z);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

int dsAlignedBox3i_distance2ToPoint(const dsAlignedBox3i* box, const dsVector3i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	int dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	int dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	int dz = dsMax(box->min.z - point->z, point->z - box->max.z);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

float dsAlignedBox3f_distanceToPoint(const dsAlignedBox3f* box, const dsVector3f* point)
{
	float distance2 = dsAlignedBox3f_distance2ToPoint(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsAlignedBox3d_distanceToPoint(const dsAlignedBox3d* box, const dsVector3d* point)
{
	double distance2 = dsAlignedBox3d_distance2ToPoint(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

double dsAlignedBox3i_distanceToPoint(const dsAlignedBox3i* box, const dsVector3i* point)
{
	int distance2 = dsAlignedBox3i_distance2ToPoint(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}
