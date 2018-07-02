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

struct PolygonInfo
{
	uint32_t firstLoop;
	uint32_t loopCount;
	uint32_t firstPoint;
	uint32_t pointCount;
};

struct dsComplexPolygon
{
	dsAllocator* allocator;

	dsGeometryElement element;
	uint8_t pointSize;
	void* userData;

	PolygonInfo* outPolygons;
	uint32_t outPolygonCount;
	uint32_t maxOutPolygons;

	void* outPoints;
	uint32_t outPointCount;
	uint32_t maxOutPoints;

	dsSimplePolygonLoop* outLoops;
	uint32_t outLoopCount;
	uint32_t maxOutLoops;
};

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

static bool simplifyPolygon(PolyTree& result, const Paths& paths, dsPolygonFillRule fillRule)
{
	PolyFillType clipperFillType = pftEvenOdd;
	if (fillRule == dsPolygonFillRule_NonZero)
		clipperFillType = pftNonZero;

	Clipper c(ioStrictlySimple);
	c.AddPaths(paths, ptSubject, true);
	c.Execute(ctUnion, result, clipperFillType);

	return true;
}

static void countPolyTree(uint32_t& outPolygonCount, uint32_t& outLoopCount,
	uint32_t& outPointCount, const PolyNode& node)
{
	for (PolyNode* child : node.Childs)
	{
		if (!child->IsHole())
			++outPolygonCount;
		outPointCount += (uint32_t)child->Contour.size();
		++outLoopCount;

		countPolyTree(outPolygonCount, outLoopCount, outPointCount, *child);
	}
}

template <typename F>
static void populatePolyTree(dsComplexPolygon* polygon, uint32_t& outPolygonIndex,
	uint32_t& outLoopCount, uint32_t& outPointCount, const PolyNode& node, const F& copyPointsFunc);

template <typename F>
static void populatePolyTreeHoles(dsComplexPolygon* polygon, uint32_t& outPolygonIndex,
	uint32_t& outLoopIndex, uint32_t& outPointIndex, uint32_t loopPointIndex, const PolyNode& node,
	const F& copyPointsFunc)
{
	for (PolyNode* child : node.Childs)
	{
		DS_ASSERT(child->IsHole());
		uint32_t curIndex = outPolygonIndex;
		uint32_t pointCount = (uint32_t)child->Contour.size();
		++polygon->outPolygons[outPolygonIndex].loopCount;
		polygon->outPolygons[curIndex].pointCount += pointCount;
		polygon->outLoops[outLoopIndex].firstPoint = loopPointIndex;
		polygon->outLoops[outLoopIndex].pointCount = pointCount;
		copyPointsFunc(polygon, child->Contour, outPointIndex, pointCount);

		++outLoopIndex;
		DS_ASSERT(outLoopIndex <= polygon->outLoopCount);
		outPointIndex += pointCount;
		loopPointIndex += pointCount;
		DS_ASSERT(outPointIndex <= polygon->outPointCount);
	}

	++outPolygonIndex;
	for (PolyNode* child : node.Childs)
	{
		populatePolyTree(polygon, outPolygonIndex, outLoopIndex, outPointIndex, *child,
			copyPointsFunc);
	}
}

template <typename F>
static void populatePolyTree(dsComplexPolygon* polygon, uint32_t& outPolygonIndex,
	uint32_t& outLoopIndex, uint32_t& outPointIndex, const PolyNode& node, const F& copyPointsFunc)
{
	for (PolyNode* child : node.Childs)
	{
		DS_ASSERT(!child->IsHole());
		uint32_t pointCount = (uint32_t)child->Contour.size();
		polygon->outPolygons[outPolygonIndex].firstLoop = outLoopIndex;
		polygon->outPolygons[outPolygonIndex].loopCount = 1;
		polygon->outPolygons[outPolygonIndex].firstPoint = outPointIndex;
		polygon->outPolygons[outPolygonIndex].pointCount = pointCount;
		polygon->outLoops[outLoopIndex].firstPoint = 0;
		polygon->outLoops[outLoopIndex].pointCount = pointCount;
		copyPointsFunc(polygon, child->Contour, outPointIndex, pointCount);

		++outLoopIndex;
		DS_ASSERT(outLoopIndex <= polygon->outLoopCount);
		outPointIndex += pointCount;
		DS_ASSERT(outPointIndex <= polygon->outPointCount);

		populatePolyTreeHoles(polygon, outPolygonIndex, outLoopIndex, outPointIndex, pointCount,
			*child, copyPointsFunc);
		DS_ASSERT(outPolygonIndex <= polygon->outPolygonCount);
	}
}

template <typename F>
static bool processPolygon(dsComplexPolygon* polygon, const Paths& paths,
	dsPolygonFillRule fillRule, const F& copyPointsFunc)
{
	PolyTree result;
	if (!simplifyPolygon(result, paths, fillRule))
		return false;

	uint32_t outPolygonCount = 0, outLoopCount = 0, outPointCount = 0;
	countPolyTree(outPolygonCount, outLoopCount, outPointCount, result);

	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->outPolygons, polygon->outPolygonCount,
		polygon->maxOutPolygons, outPolygonCount))
	{
		return false;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->outLoops, polygon->outLoopCount,
		polygon->maxOutLoops, outLoopCount))
	{
		return false;
	}

	if (!dsResizeableArray_add(polygon->allocator, (void**)&polygon->outPoints,
		&polygon->outPointCount, &polygon->maxOutPoints, polygon->pointSize, outPointCount))
	{
		return false;
	}

	outPolygonCount = 0;
	outLoopCount = 0;
	outPointCount = 0;
	populatePolyTree(polygon, outPolygonCount, outLoopCount, outPointCount, result, copyPointsFunc);
	DS_ASSERT(outPolygonCount == polygon->outPolygonCount);
	DS_ASSERT(outPointCount == polygon->outPointCount);
	DS_ASSERT(outLoopCount == polygon->outLoopCount);

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

	return processPolygon(polygon, paths, fillRule,
		[&](dsComplexPolygon* polygon, const Path& path, uint32_t firstPoint, uint32_t pointCount)
		{
			dsVector2f* points = (dsVector2f*)polygon->outPoints + firstPoint;
			for (uint32_t i = 0; i < pointCount; ++i)
			{
				dsVector2f point = {{(float)((double)path[i].X/limit),
					(float)((double)path[i].Y/limit)}};
				dsVector2_mul(point, point, scale);
				dsVector2_add(points[i], point, offset);
			}
		});
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

	return processPolygon(polygon, paths, fillRule,
		[&](dsComplexPolygon* polygon, const Path& path, uint32_t firstPoint, uint32_t pointCount)
		{
			dsVector2d* points = (dsVector2d*)polygon->outPoints + firstPoint;
			for (uint32_t i = 0; i < pointCount; ++i)
			{
				dsVector2d point = {{(double)path[i].X/limit, (double)path[i].Y/limit}};
				dsVector2_mul(point, point, scale);
				dsVector2_add(points[i], point, offset);
			}
		});
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

	return processPolygon(polygon, paths, fillRule,
		[](dsComplexPolygon* polygon, const Path& path, uint32_t firstPoint, uint32_t pointCount)
		{
			dsVector2i* points = (dsVector2i*)polygon->outPoints + firstPoint;
			for (uint32_t i = 0; i < pointCount; ++i)
			{
				points[i].x = (int)path[i].X;
				points[i].y = (int)path[i].Y;
			}
		});
}

extern "C"
{

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

	uint8_t pointSize;
	switch (element)
	{
		case dsGeometryElement_Float:
			pointSize = (uint8_t)sizeof(dsVector2f);
			break;
		case dsGeometryElement_Double:
			pointSize = (uint8_t)sizeof(dsVector2d);
			break;
		case dsGeometryElement_Int:
			pointSize = (uint8_t)sizeof(dsVector2i);
			break;
		default:
			errno = EINVAL;
			return NULL;
	}

	dsComplexPolygon* polygon = DS_ALLOCATE_OBJECT(allocator, dsComplexPolygon);
	if (!polygon)
		return NULL;

	memset(polygon, 0, sizeof(dsComplexPolygon));
	polygon->allocator = dsAllocator_keepPointer(allocator);
	polygon->userData = userData;
	polygon->element = element;
	polygon->pointSize = pointSize;
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

	polygon->outPolygonCount = 0;
	polygon->outPointCount = 0;
	polygon->outLoopCount = 0;
	try
	{
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
	catch (const std::bad_alloc& e)
	{
		errno = ENOMEM;
		return false;
	}
}

uint32_t dsComplexPolygon_getHoledPolygonCount(const dsComplexPolygon* polygon)
{
	if (!polygon)
		return 0;

	return polygon->outPolygonCount;
}

uint32_t dsComplexPolygon_getHoledPolygonLoopCount(const dsComplexPolygon* polygon, uint32_t index)
{
	if (!polygon || index >= polygon->outPolygonCount)
		return 0;

	return polygon->outPolygons[index].loopCount;
}

const dsSimplePolygonLoop* dsComplexPolygon_getHoledPolygonLoops(const dsComplexPolygon* polygon,
	uint32_t index)
{
	if (!polygon || index >= polygon->outPolygonCount)
	{
		errno = EINVAL;
		return NULL;
	}

	return polygon->outLoops + polygon->outPolygons[index].firstLoop;
}

uint32_t dsComplexPolygon_getHoledPolygonPointCount(const dsComplexPolygon* polygon, uint32_t index)
{
	if (!polygon || index >= polygon->outPolygonCount)
		return 0;

	return polygon->outPolygons[index].pointCount;
}

const void* dsComplexPolygon_getHoledPolygonPoints(const dsComplexPolygon* polygon, uint32_t index)
{
	if (!polygon || index >= polygon->outPolygonCount)
	{
		errno = EINVAL;
		return NULL;
	}

	return (const uint8_t*)polygon->outPoints +
		polygon->outPolygons[index].firstPoint*polygon->pointSize;
}

void dsComplexPolygon_destroy(dsComplexPolygon* polygon)
{
	if (!polygon || !polygon->allocator)
		return;

	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->outPolygons));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->outPoints));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->outLoops));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon));
}

} // extern "C"
