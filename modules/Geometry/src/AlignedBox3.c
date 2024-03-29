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

float dsAlignedBox3f_dist2(const dsAlignedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	float dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	float dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);
	float dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

double dsAlignedBox3d_dist2(const dsAlignedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);
	double dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

int dsAlignedBox3i_dist2(const dsAlignedBox3i* box, const dsVector3i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3_isValid(*box))
		return -1;

	if (dsAlignedBox3_containsPoint(*box, *point))
		return 0;

	int dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	int dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);
	int dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
}

float dsAlignedBox3f_dist(const dsAlignedBox3f* box, const dsVector3f* point)
{
	float distance2 = dsAlignedBox3f_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsAlignedBox3d_dist(const dsAlignedBox3d* box, const dsVector3d* point)
{
	double distance2 = dsAlignedBox3d_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

double dsAlignedBox3i_dist(const dsAlignedBox3i* box, const dsVector3i* point)
{
	int distance2 = dsAlignedBox3i_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

bool dsAlignedBox3f_isValid(const dsAlignedBox3f* box);
bool dsAlignedBox3d_isValid(const dsAlignedBox3d* box);
bool dsAlignedBox3i_isValid(const dsAlignedBox3i* box);

void dsAlignedBox3f_addPoint(dsAlignedBox3f* box, const dsVector3f* point);
void dsAlignedBox3d_addPoint(dsAlignedBox3d* box, const dsVector3d* point);
void dsAlignedBox3i_addPoint(dsAlignedBox3i* box, const dsVector3i* point);

void dsAlignedBox3f_addBox(dsAlignedBox3f* box, const dsAlignedBox3f* otherBox);
void dsAlignedBox3d_addBox(dsAlignedBox3d* box, const dsAlignedBox3d* otherBox);
void dsAlignedBox3i_addBox(dsAlignedBox3i* box, const dsAlignedBox3i* otherBox);

bool dsAlignedBox3f_containsPoint(const dsAlignedBox3f* box, const dsVector3f* point);
bool dsAlignedBox3d_containsPoint(const dsAlignedBox3d* box, const dsVector3d* point);
bool dsAlignedBox3i_containsPoint(const dsAlignedBox3i* box, const dsVector3i* point);

bool dsAlignedBox3f_containsBox(const dsAlignedBox3f* box, const dsAlignedBox3f* otherBox);
bool dsAlignedBox3d_containsBox(const dsAlignedBox3d* box, const dsAlignedBox3d* otherBox);
bool dsAlignedBox3i_containsBox(const dsAlignedBox3i* box, const dsAlignedBox3i* otherBox);

bool dsAlignedBox3f_intersects(const dsAlignedBox3f* box, const dsAlignedBox3f* otherBox);
bool dsAlignedBox3d_intersects(const dsAlignedBox3d* box, const dsAlignedBox3d* otherBox);
bool dsAlignedBox3i_intersects(const dsAlignedBox3i* box, const dsAlignedBox3i* otherBox);

void dsAlignedBox3f_intersect(dsAlignedBox3f* result, const dsAlignedBox3f* a,
	const dsAlignedBox3f* b);
void dsAlignedBox3d_intersect(dsAlignedBox3d* result, const dsAlignedBox3d* a,
	const dsAlignedBox3d* b);
void dsAlignedBox3i_intersect(dsAlignedBox3i* result, const dsAlignedBox3i* a,
	const dsAlignedBox3i* b);

void dsAlignedBox3f_center(dsVector3f* result, const dsAlignedBox3f* box);
void dsAlignedBox3d_center(dsVector3d* result, const dsAlignedBox3d* box);
void dsAlignedBox3i_center(dsVector3i* result, const dsAlignedBox3i* box);

void dsAlignedBox3f_extents(dsVector3f* result, const dsAlignedBox3f* box);
void dsAlignedBox3d_extents(dsVector3d* result, const dsAlignedBox3d* box);
void dsAlignedBox3i_extents(dsVector3i* result, const dsAlignedBox3i* box);

void dsAlignedBox3f_toMatrix(dsMatrix44f* result, dsAlignedBox3f* box);
void dsAlignedBox3d_toMatrix(dsMatrix44d* result, dsAlignedBox3d* box);

void dsAlignedBox3f_toMatrixTranspose(dsMatrix44f* result, dsAlignedBox3f* box);
void dsAlignedBox3d_toMatrixTranspose(dsMatrix44d* result, dsAlignedBox3d* box);

void dsAlignedBox3f_corners(dsVector3f corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3f* box);
void dsAlignedBox3d_corners(dsVector3d corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3d* box);
void dsAlignedBox3i_corners(dsVector3i corners[DS_BOX3_CORNER_COUNT],
	const dsAlignedBox3i* box);

void dsAlignedBox3f_closestPoint(dsVector3f* result, const dsAlignedBox3f* box,
	const dsVector3f* point);
void dsAlignedBox3d_closestPoint(dsVector3d* result, const dsAlignedBox3d* box,
	const dsVector3d* point);
void dsAlignedBox3i_closestPoint(dsVector3i* result, const dsAlignedBox3i* box,
	const dsVector3i* point);
