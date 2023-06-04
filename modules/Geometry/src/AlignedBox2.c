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

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Core/Assert.h>
#include <float.h>
#include <limits.h>

void dsAlignedBox2f_makeInvalid(dsAlignedBox2f* result)
{
	DS_ASSERT(result);

	result->min.x = FLT_MAX;
	result->min.y = FLT_MAX;
	result->max.x = -FLT_MAX;
	result->max.y = -FLT_MAX;
}

void dsAlignedBox2d_makeInvalid(dsAlignedBox2d* result)
{
	DS_ASSERT(result);

	result->min.x = DBL_MAX;
	result->min.y = DBL_MAX;
	result->max.x = -DBL_MAX;
	result->max.y = -DBL_MAX;
}

void dsAlignedBox2i_makeInvalid(dsAlignedBox2i* result)
{
	DS_ASSERT(result);

	result->min.x = INT_MAX;
	result->min.y = INT_MAX;
	result->max.x = INT_MIN;
	result->max.y = INT_MIN;
}

float dsAlignedBox2f_dist2(const dsAlignedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox2_isValid(*box))
		return -1;

	if (dsAlignedBox2_containsPoint(*box, *point))
		return 0;

	float dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	float dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);

	return dsPow2(dx) + dsPow2(dy);
}

double dsAlignedBox2d_dist2(const dsAlignedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox2_isValid(*box))
		return -1;

	if (dsAlignedBox2_containsPoint(*box, *point))
		return 0;

#if DS_SIMD_ALWAYS_DOUBLE2
	dsVector2d d;
	d.simd = dsSIMD2d_max(dsSIMD2d_sub(box->min.simd, point->simd),
		dsSIMD2d_sub(point->simd, box->max.simd));
	d.simd = dsSIMD2d_max(d.simd, dsSIMD2d_set1(0.0));
	d.simd = dsSIMD2d_mul(d.simd, d.simd);
	return d.x + d.y;
#else
	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);

	return dsPow2(dx) + dsPow2(dy);
#endif
}

int dsAlignedBox2i_dist2(const dsAlignedBox2i* box, const dsVector2i* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox2_isValid(*box))
		return -1;

	if (dsAlignedBox2_containsPoint(*box, *point))
		return 0;

	int dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	int dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);

	return dsPow2(dx) + dsPow2(dy);
}

float dsAlignedBox2f_dist(const dsAlignedBox2f* box, const dsVector2f* point)
{
	float distance2 = dsAlignedBox2f_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsAlignedBox2d_dist(const dsAlignedBox2d* box, const dsVector2d* point)
{
	double distance2 = dsAlignedBox2d_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

double dsAlignedBox2i_dist(const dsAlignedBox2i* box, const dsVector2i* point)
{
	int distance2 = dsAlignedBox2i_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

bool dsAlignedBox2f_isValid(const dsAlignedBox2f* box);
bool dsAlignedBox2d_isValid(const dsAlignedBox2d* box);
bool dsAlignedBox2i_isValid(const dsAlignedBox2i* box);

void dsAlignedBox2f_addPoint(dsAlignedBox2f* box, const dsVector2f* point);
void dsAlignedBox2d_addPoint(dsAlignedBox2d* box, const dsVector2d* point);
void dsAlignedBox2i_addPoint(dsAlignedBox2i* box, const dsVector2i* point);

void dsAlignedBox2f_addBox(dsAlignedBox2f* box, const dsAlignedBox2f* otherBox);
void dsAlignedBox2d_addBox(dsAlignedBox2d* box, const dsAlignedBox2d* otherBox);
void dsAlignedBox2i_addBox(dsAlignedBox2i* box, const dsAlignedBox2i* otherBox);

bool dsAlignedBox2f_containsPoint(const dsAlignedBox2f* box, const dsVector2f* point);
bool dsAlignedBox2d_containsPoint(const dsAlignedBox2d* box, const dsVector2d* point);
bool dsAlignedBox2i_containsPoint(const dsAlignedBox2i* box, const dsVector2i* point);

bool dsAlignedBox2f_containsBox(const dsAlignedBox2f* box, const dsAlignedBox2f* otherBox);
bool dsAlignedBox2d_containsBox(const dsAlignedBox2d* box, const dsAlignedBox2d* otherBox);
bool dsAlignedBox2i_containsBox(const dsAlignedBox2i* box, const dsAlignedBox2i* otherBox);

bool dsAlignedBox2f_intersects(const dsAlignedBox2f* box, const dsAlignedBox2f* otherBox);
bool dsAlignedBox2d_intersects(const dsAlignedBox2d* box, const dsAlignedBox2d* otherBox);
bool dsAlignedBox2i_intersects(const dsAlignedBox2i* box, const dsAlignedBox2i* otherBox);

void dsAlignedBox2d_intersect(dsAlignedBox2d* result, const dsAlignedBox2d* a,
	const dsAlignedBox2d* b);
void dsAlignedBox2i_intersect(dsAlignedBox2i* result, const dsAlignedBox2i* a,
	const dsAlignedBox2i* b);

void dsAlignedBox2f_center(dsVector2f* result, const dsAlignedBox2f* box);
void dsAlignedBox2d_center(dsVector2d* result, const dsAlignedBox2d* box);
void dsAlignedBox2i_center(dsVector2i* result, const dsAlignedBox2i* box);

void dsAlignedBox2f_extents(dsVector2f* result, const dsAlignedBox2f* box);
void dsAlignedBox2d_extents(dsVector2d* result, const dsAlignedBox2d* box);
void dsAlignedBox2i_extents(dsVector2i* result, const dsAlignedBox2i* box);

void dsAlignedBox2f_toMatrix(dsMatrix33f* result, dsAlignedBox2f* box);
void dsAlignedBox2d_toMatrix(dsMatrix33d* result, dsAlignedBox2d* box);

void dsAlignedBox2f_toMatrixTranspose(dsMatrix33f* result, dsAlignedBox2f* box);
void dsAlignedBox2d_toMatrixTranspose(dsMatrix33d* result, dsAlignedBox2d* box);

void dsAlignedBox2f_corners(dsVector2f corners[DS_BOX2_CORNER_COUNT], const dsAlignedBox2f* box);
void dsAlignedBox2d_corners(dsVector2d corners[DS_BOX2_CORNER_COUNT], const dsAlignedBox2d* box);
void dsAlignedBox2i_corners(dsVector2i corners[DS_BOX2_CORNER_COUNT], const dsAlignedBox2i* box);

void dsAlignedBox2f_closestPoint(dsVector2f* result, const dsAlignedBox2f* box,
	const dsVector2f* point);
void dsAlignedBox2d_closestPoint(dsVector2d* result, const dsAlignedBox2d* box,
	const dsVector2d* point);
void dsAlignedBox2i_closestPoint(dsVector2i* result, const dsAlignedBox2i* box,
	const dsVector2i* point);
