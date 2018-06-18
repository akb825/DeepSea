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
#include "PolygonShared.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

struct dsComplexPolygon
{
	dsAllocator* allocator;

	dsGeometryElement element;
	void* userData;

	void** loopPoints;
	uint32_t loopPointCount;
	uint32_t maxLoopPoints;

	dsPolygonLoop* loops;
	uint32_t loopCount;
	uint32_t maxLoops;
};

static bool simplifyFloat(dsComplexPolygon* polygon, const dsPolygonLoop* loops, uint32_t loopCount,
	dsPolygonFillRule fillRule)
{
	ClipperLib::Paths paths(loopCount);
	dsAlignedBox2f bounds;
	dsAlignedBox2f_makeInvalid(&bounds);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
			dsAlignedBox2_addPoint(bounds, ((const dsVector2f*)loops[i].points)[j]);
	}

	// Normalize to range [-1, 1] to put in the integer limit.
	const double limit = (double)ClipperLib::loRange;
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
			dsVector2_sub(point, ((const dsVector2f*)loops[i].points)[j], offset);
			dsVector2_mul(point, point, invScale);
			paths[i][j].X = (ClipperLib::cInt)round((double)point.x*limit);
			paths[i][j].Y = (ClipperLib::cInt)round((double)point.y*limit);
		}
	}

	ClipperLib::SimplifyPolygons(paths,
		fillRule == dsPolygonFillRule_NonZero ? ClipperLib::pftNonZero : ClipperLib::pftEvenOdd);

	uint32_t pointCount = 0;
	for (const ClipperLib::Path& path : paths)
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
		for (const ClipperLib::IntPoint& pathPoint : paths[i])
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

static bool simplifyDouble(dsComplexPolygon* polygon, const dsPolygonLoop* loops, uint32_t loopCount,
	dsPolygonFillRule fillRule)
{
	ClipperLib::Paths paths(loopCount);
	dsAlignedBox2d bounds;
	dsAlignedBox2d_makeInvalid(&bounds);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
			dsAlignedBox2_addPoint(bounds, ((const dsVector2d*)loops[i].points)[j]);
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
			dsVector2_sub(point, ((const dsVector2d*)loops[i].points)[j], offset);
			dsVector2_mul(point, point, invScale);
			paths[i][j].X = (ClipperLib::cInt)round(point.x*limit);
			paths[i][j].Y = (ClipperLib::cInt)round(point.y*limit);
		}
	}

	ClipperLib::SimplifyPolygons(paths,
		fillRule == dsPolygonFillRule_NonZero ? ClipperLib::pftNonZero : ClipperLib::pftEvenOdd);

	uint32_t pointCount = 0;
	for (const ClipperLib::Path& path : paths)
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
		for (const ClipperLib::IntPoint& pathPoint : paths[i])
		{
			dsVector2d point = {{(double)pathPoint.X/limit, (double)pathPoint.Y/limit}};
			dsVector2_mul(point, point, scale);
			dsVector2_add(*curPoint, point, offset);
			++curPoint;
		}
	}

	return true;
}

static bool simplifyInt(dsComplexPolygon* polygon, const dsPolygonLoop* loops, uint32_t loopCount,
	dsPolygonFillRule fillRule)
{
	ClipperLib::Paths paths(loopCount);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		paths[i].resize(loops[i].pointCount);
		for (uint32_t j = 0; j < loops[i].pointCount; ++j)
		{
			const dsVector2i* point = ((const dsVector2i*)loops[i].points) + j;
			paths[i][j].X = point->x;
			paths[i][j].Y = point->y;
		}
	}

	ClipperLib::SimplifyPolygons(paths,
		fillRule == dsPolygonFillRule_NonZero ? ClipperLib::pftNonZero : ClipperLib::pftEvenOdd);

	uint32_t pointCount = 0;
	for (const ClipperLib::Path& path : paths)
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
		for (const ClipperLib::IntPoint& pathPoint : paths[i])
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

bool dsComplexPolygon_simplify(dsComplexPolygon* polygon, const dsPolygonLoop* loops,
	uint32_t loopCount, dsPolygonFillRule fillRule)
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
	}

	polygon->loopPointCount = 0;
	polygon->loopCount = 0;
	switch (polygon->element)
	{
		case dsGeometryElement_Float:
			return simplifyFloat(polygon, loops, loopCount, fillRule);
		case dsGeometryElement_Double:
			return simplifyDouble(polygon, loops, loopCount, fillRule);
		case dsGeometryElement_Int:
			return simplifyInt(polygon, loops, loopCount, fillRule);
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

const dsPolygonLoop* dsComplexPolygon_getLoop(const dsComplexPolygon* polygon, uint32_t index)
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
