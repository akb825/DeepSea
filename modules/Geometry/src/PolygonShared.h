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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Geometry/Types.h>

#define NOT_FOUND (uint32_t)-1
#define EPSILON 1e-16

bool dsIntersectPolygonEdges(dsVector2d* outPoint, double* outT, double* outOtherT,
	const dsVector2d* from, const dsVector2d* to, const dsVector2d* otherFrom,
	const dsVector2d* otherTo);

inline bool dsIsPolygonTriangleCCW(const dsVector2d* p0, const dsVector2d* p1,
	const dsVector2d* p2)
{
	// Cross product of the triangle with Z = 0.
	double cross = (p1->x - p0->x)*(p2->y - p0->y) - (p2->x - p0->x)*(p1->y - p0->y);
	return cross >= 0.0f;
}

inline int dsComparePolygonPoints(const dsVector2d* left, const dsVector2d* right)
{
	if (left->x < right->x)
		return -1;
	else if (left->x > right->x)
		return 1;
	else if (left->y < right->y)
		return -1;
	else if (left->y > right->y)
		return 1;
	return 0;
}
