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

#include <DeepSea/Geometry/ComplexPolygon.h>

#include "Clipper/clipper.hpp"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

using namespace ClipperLib;

struct dsComplexPolygon
{
	dsAllocator* allocator;

	dsGeometryElement element;
	void* userData;

	void** loopPoints;
	uint32_t loopPointCount;
	uint32_t maxLoopPoints;

	dsComplexPolygonLoop* loops;
	uint32_t loopCount;
	uint32_t maxLoops;
};

extern "C"
{

static bool defaultGetPointFloat(void* outPosition, const dsComplexPolygon* polygon,
	const void* points, uint32_t index)
{
	DS_UNUSED(polygon);
	*(dsVector2f*)outPosition = ((const dsVector2f*)points)[index];
	return true;
}

static bool defaultGetPointDouble(void* outPosition, const dsComplexPolygon* polygon,
	const void* points, uint32_t index)
{
	DS_UNUSED(polygon);
	*(dsVector2d*)outPosition = ((const dsVector2d*)points)[index];
	return true;
}

static bool defaultGetPointInt(void* outPosition, const dsComplexPolygon* polygon,
	const void* points, uint32_t index)
{
	DS_UNUSED(polygon);
	*(dsVector2i*)outPosition = ((const dsVector2i*)points)[index];
	return true;
}

static bool simplifyPolygon(Paths& paths, dsPolygonFillRule fillRule)
{
	PolyFillType clipperFillType = pftEvenOdd;
	if (fillRule == dsPolygonFillRule_NonZero)
		clipperFillType = pftNonZero;

	Clipper c(ioStrictlySimple);
	c.AddPaths(paths, ptSubject, true);
	c.Execute(ctUnion, paths, clipperFillType);

	return true;
}

static bool simplifyFloat(dsComplexPolygon* polygon, const dsComplexPolygonLoop* loops,
	uint32_t loopCount, dsComplexPolygonPointFunction pointFunc, dsPolygonFillRule fillRule)
{
	Paths paths(loopCount);
	dsAlignedBox2f bounds;
	dsAlignedBox2f_makeInvalid(&bounds);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			dsVector2f point;
			if (!pointFunc(&point, polygon, loops[i].points, j))
				return false;
			dsAlignedBox2_addPoint(bounds, point);
		}
	}

	// Normalize to range [-1, 1] to put in the integer limit.
	const double limit = (double)loRange;
	dsVector2f offset, scale, invScale;
	dsVector2f one = {{1.0f, 1.0f}};
	dsVector2f half = {{0.5f, 0.5f}};
	dsAlignedBox2_center(offset, bounds);
	dsAlignedBox2_extents(scale, bounds);
	dsVector2_mul(scale, scale, half);
	dsVector2_div(invScale, one, scale);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		paths[i].resize(loops[i].pointCount);
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			dsVector2f point;
			if (!pointFunc(&point, polygon, loops[i].points, j))
				return false;

			dsVector2_sub(point, point, offset);
			dsVector2_mul(point, point, invScale);
			paths[i][j].X = (cInt)round((double)point.x*limit);
			paths[i][j].Y = (cInt)round((double)point.y*limit);
		}
	}

	if (!simplifyPolygon(paths, fillRule))
		return false;

	uint32_t pointCount = 0;
	for (const Path& path : paths)
		pointCount += (uint32_t)path.size();

	if (!dsResizeableArray_add(polygon->allocator, (void**)&polygon->loopPoints,
		&polygon->loopPointCount, &polygon->maxLoopPoints, sizeof(dsVector2f), pointCount))
	{
		return false;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->loops, polygon->loopCount,
		polygon->maxLoops, (uint32_t)paths.size()))
	{
		return false;
	}

	dsVector2f* curPoint = (dsVector2f*)polygon->loopPoints;
	for (uint32_t i = 0; i < paths.size(); ++i)
	{
		polygon->loops[i].points = curPoint;
		polygon->loops[i].pointCount = (uint32_t)paths[i].size();
		for (const IntPoint& pathPoint : paths[i])
		{
			dsVector2f point = {{(float)((double)pathPoint.X/limit),
				(float)((double)pathPoint.Y/limit)}};
			dsVector2_mul(point, point, scale);
			dsVector2_add(*curPoint, point, offset);
			++curPoint;
		}
	}

	return true;
}

static bool simplifyDouble(dsComplexPolygon* polygon, const dsComplexPolygonLoop* loops,
	uint32_t loopCount, dsComplexPolygonPointFunction pointFunc, dsPolygonFillRule fillRule)
{
	Paths paths(loopCount);
	dsAlignedBox2d bounds;
	dsAlignedBox2d_makeInvalid(&bounds);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			dsVector2d point;
			if (!pointFunc(&point, polygon, loops[i].points, j))
				return false;
			dsAlignedBox2_addPoint(bounds, point);
		}
	}

	// Normalize to range [-1, 1] to put in the integer limit. The hiRange constant is too large to
	// represent with a double, so use a 52-bit limit instead. (max bits of precision)
	const double limit = (double)(0xFFFFFFFFFFFFFULL);
	dsVector2d offset, scale, invScale;
	dsVector2d one = {{1.0, 1.0}};
	dsVector2d half = {{0.5, 0.5}};
	dsAlignedBox2_center(offset, bounds);
	dsAlignedBox2_extents(scale, bounds);
	dsVector2_mul(scale, scale, half);
	dsVector2_div(invScale, one, scale);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		paths[i].resize(loops[i].pointCount);
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			dsVector2d point;
			if (!pointFunc(&point, polygon, loops[i].points, j))
				return false;

			dsVector2_sub(point, point, offset);
			dsVector2_mul(point, point, invScale);
			paths[i][j].X = (cInt)round(point.x*limit);
			paths[i][j].Y = (cInt)round(point.y*limit);
		}
	}

	if (!simplifyPolygon(paths, fillRule))
		return false;

	uint32_t pointCount = 0;
	for (const Path& path : paths)
		pointCount += (uint32_t)path.size();

	if (!dsResizeableArray_add(polygon->allocator, (void**)&polygon->loopPoints,
		&polygon->loopPointCount, &polygon->maxLoopPoints, sizeof(dsVector2d), pointCount))
	{
		return false;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->loops, polygon->loopCount,
		polygon->maxLoops, (uint32_t)paths.size()))
	{
		return false;
	}

	dsVector2d* curPoint = (dsVector2d*)polygon->loopPoints;
	for (uint32_t i = 0; i < paths.size(); ++i)
	{
		polygon->loops[i].points = curPoint;
		polygon->loops[i].pointCount = (uint32_t)paths[i].size();
		for (const IntPoint& pathPoint : paths[i])
		{
			dsVector2d point = {{(double)pathPoint.X/limit, (double)pathPoint.Y/limit}};
			dsVector2_mul(point, point, scale);
			dsVector2_add(*curPoint, point, offset);
			++curPoint;
		}
	}

	return true;
}

static bool simplifyInt(dsComplexPolygon* polygon, const dsComplexPolygonLoop* loops,
	uint32_t loopCount, dsComplexPolygonPointFunction pointFunc, dsPolygonFillRule fillRule)
{
	Paths paths(loopCount);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		paths[i].resize(loops[i].pointCount);
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			dsVector2i point;
			if (!pointFunc(&point, polygon, loops[i].points, j))
				return false;

			paths[i][j].X = point.x;
			paths[i][j].Y = point.y;
		}
	}

	if (!simplifyPolygon(paths, fillRule))
		return false;

	uint32_t pointCount = 0;
	for (const Path& path : paths)
		pointCount += (uint32_t)path.size();

	if (!dsResizeableArray_add(polygon->allocator, (void**)&polygon->loopPoints,
		&polygon->loopPointCount, &polygon->maxLoopPoints, sizeof(dsVector2i), pointCount))
	{
		return false;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->loops, polygon->loopCount,
		polygon->maxLoops, (uint32_t)paths.size()))
	{
		return false;
	}

	dsVector2i* curPoint = (dsVector2i*)polygon->loopPoints;
	for (uint32_t i = 0; i < paths.size(); ++i)
	{
		polygon->loops[i].points = curPoint;
		polygon->loops[i].pointCount = (uint32_t)paths[i].size();
		for (const IntPoint& pathPoint : paths[i])
		{
			curPoint->x = (int)pathPoint.X;
			curPoint->y = (int)pathPoint.Y;
			++curPoint;
		}
	}

	return true;
}

dsComplexPolygon* dsComplexPolygon_create(dsAllocator* allocator, dsGeometryElement element,
	void* userData)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Complex polygon allocator must support freeing memory.");
		return NULL;
	}

	dsComplexPolygon* polygon = DS_ALLOCATE_OBJECT(allocator, dsComplexPolygon);
	if (!polygon)
		return NULL;

	memset(polygon, 0, sizeof(dsComplexPolygon));
	polygon->allocator = dsAllocator_keepPointer(allocator);
	polygon->userData = userData;
	polygon->element = element;
	return polygon;
}

dsGeometryElement dsComplexPolygon_getElement(const dsComplexPolygon* polygon)
{
	if (!polygon)
		return dsGeometryElement_Float;

	return polygon->element;
}

void* dsComplexPolygon_getUserData(const dsComplexPolygon* polygon)
{
	if (!polygon)
		return NULL;

	return polygon->userData;
}

void dsComplexPolygon_setUserData(dsComplexPolygon* polygon, void* userData)
{
	if (polygon)
		polygon->userData = userData;
}

bool dsComplexPolygon_simplify(dsComplexPolygon* polygon, const dsComplexPolygonLoop* loops,
	uint32_t loopCount, dsComplexPolygonPointFunction pointFunc, dsPolygonFillRule fillRule)
{
	if (!polygon || (!loops && loopCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < loopCount; ++i)
	{
		if (!loops[i].points && loops[i].pointCount > 0)
		{
			errno = EINVAL;
			return false;
		}

		if (loops[i].pointCount < 3)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loops must have at least 3 vertices.");
			return false;
		}
	}

	polygon->loopPointCount = 0;
	polygon->loopCount = 0;
	switch (polygon->element)
	{
		case dsGeometryElement_Float:
			if (!pointFunc)
				pointFunc = defaultGetPointFloat;
			return simplifyFloat(polygon, loops, loopCount, pointFunc, fillRule);
		case dsGeometryElement_Double:
			if (!pointFunc)
				pointFunc = defaultGetPointDouble;
			return simplifyDouble(polygon, loops, loopCount, pointFunc, fillRule);
		case dsGeometryElement_Int:
			if (!pointFunc)
				pointFunc = defaultGetPointInt;
			return simplifyInt(polygon, loops, loopCount, pointFunc, fillRule);
		default:
			errno = EINVAL;
			return false;
	}
}

uint32_t dsComplexPolygon_getLoopCount(const dsComplexPolygon* polygon)
{
	if (!polygon)
		return 0;

	return polygon->loopCount;
}

const dsComplexPolygonLoop* dsComplexPolygon_getLoop(const dsComplexPolygon* polygon,
	uint32_t index)
{
	if (!polygon)
	{
		errno = EINVAL;
		return NULL;
	}

	if (index >= polygon->loopCount)
	{
		errno = EINDEX;
		return NULL;
	}

	return polygon->loops + index;
}

void dsComplexPolygon_destroy(dsComplexPolygon* polygon)
{
	if (!polygon || !polygon->allocator)
		return;

	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->loopPoints));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->loops));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon));
}

} // extern "C"
