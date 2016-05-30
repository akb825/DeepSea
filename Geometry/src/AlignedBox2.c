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

	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0);

	return dsPow2(dx) + dsPow2(dy);
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
