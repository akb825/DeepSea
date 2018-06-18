/*
 * Copyright 2018 Aaron Barany
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

#include "PolygonShared.h"
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Vector2.h>

bool dsIntersectPolygonEdges(dsVector2d* outPoint, double* outT, double* outOtherT,
	const dsVector2d* from, const dsVector2d* to, const dsVector2d* otherFrom,
	const dsVector2d* otherTo)
{
	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	double divisor = (from->x - to->x)*(otherFrom->y - otherTo->y) -
		(from->y - to->y)*(otherFrom->x - otherTo->x);
	if (divisor == 0.0)
		return true;

	dsVector2d intersect =
	{{
		(from->x*to->y - from->y*to->x)*(otherFrom->x - otherTo->x) -
			(from->x - to->x)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
		(from->x*to->y - from->y*to->x)*(otherFrom->y - otherTo->y) -
			(from->y - to->y)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
	}};

	divisor = 1.0/divisor;
	dsVector2_scale(intersect, intersect, divisor);

	dsVector2d offset;
	dsVector2_sub(offset, *to, *from);
	double t;
	// Find T based on the largest difference to avoid issues with axis-aligned lines.
	if (fabs(offset.x) > fabs(offset.y))
		t = (intersect.x - from->x)/offset.x;
	else
		t = (intersect.y - from->y)/offset.y;

	if (t < 0.0 || t > 1.0)
		return false;

	if (outPoint)
		*outPoint = intersect;
	if (outT)
		*outT = t;
	if (outOtherT)
	{
		dsVector2_sub(offset, *otherTo, *otherFrom);
		if (fabs(offset.x) > fabs(offset.y))
			t = (intersect.x - otherFrom->x)/offset.x;
		else
			t = (intersect.y - otherFrom->y)/offset.y;
	}

	return true;
}

bool dsIsPolygonTriangleCCW(const dsVector2d* p0, const dsVector2d* p1, const dsVector2d* p2);
int dsComparePolygonPoints(const dsVector2d* left, const dsVector2d* right);
