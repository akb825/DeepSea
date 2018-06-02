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

#include <DeepSea/Geometry/SimplePolygon.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/BVH.h>
#include <DeepSea/Math/Vector2.h>
#include <string.h>

#define NOT_FOUND (uint32_t)-1
#define EPSILON 1e-16

typedef enum ConnectingEdge
{
	ConnectingEdge_Main,
	ConnectingEdge_LeftTop,
	ConnectingEdge_LeftBottom,
	ConnectingEdge_RightTop,
	ConnectingEdge_RightBottom,
	ConnectingEdge_Count
} ConnectingEdge;

typedef struct Vertex
{
	dsVector2d point;
	uint32_t prevEdges[ConnectingEdge_Count];
	uint32_t nextEdges[ConnectingEdge_Count];
} Vertex;

typedef struct Edge
{
	uint32_t prevVertex;
	uint32_t nextVertex;
	uint32_t prevEdge;
	uint32_t nextEdge;
	bool visited;
} Edge;

typedef struct LoopVertex
{
	uint32_t vertIndex;
	uint32_t prevVert;
	uint32_t nextVert;
} LoopVertex;

struct dsSimplePolygon
{
	dsAllocator* allocator;

	void* userData;

	Vertex* vertices;
	uint32_t vertexCount;
	uint32_t maxVertices;

	Edge* edges;
	uint32_t edgeCount;
	uint32_t maxEdges;

	uint32_t* sortedVerts;
	uint32_t maxSortedVerts;

	bool builtBVH;
	dsBVH* edgeBVH;

	LoopVertex* loopVertices;
	uint32_t loopVertCount;
	uint32_t maxLoopVerts;

	uint32_t* vertexStack;
	uint32_t vertStackCount;
	uint32_t maxVertStack;

	uint32_t* indices;
	uint32_t indexCount;
	uint32_t maxIndices;
};

typedef struct EdgeIntersectInfo
{
	dsVector2d fromPos;
	dsVector2d toPos;
	uint32_t fromVert;
	uint32_t toVert;
	bool intersects;
} EdgeIntersectInfo;

static bool defaultPointPosition(dsVector2d* outPosition, const void* points,
	void* userData, uint32_t index)
{
	DS_UNUSED(userData);
	*outPosition = ((dsVector2d*)points)[index];
	return true;
}

static int comparePolygonVertex(const void* left, const void* right, void* context)
{
	const dsSimplePolygon* polygon = (const dsSimplePolygon*)context;
	const dsVector2d* leftVert = &polygon->vertices[*(const uint32_t*)left].point;
	const dsVector2d* rightVert = &polygon->vertices[*(const uint32_t*)right].point;
	if (leftVert->x < rightVert->x)
		return -1;
	else if (leftVert->x > rightVert->x)
		return 1;
	else if (leftVert->y < rightVert->y)
		return -1;
	else if (leftVert->y > rightVert->y)
		return 1;
	return 0;
}

static int compareLoopVertex(const void* left, const void* right, void* context)
{
	const dsSimplePolygon* polygon = (const dsSimplePolygon*)context;
	const LoopVertex* leftVert = (const LoopVertex*)left;
	const LoopVertex* rightVert = (const LoopVertex*)right;
	const dsVector2d* leftPos = &polygon->vertices[leftVert->vertIndex].point;
	const dsVector2d* rightPos = &polygon->vertices[rightVert->vertIndex].point;
	if (leftPos->x < rightPos->x)
		return -1;
	else if (leftPos->x > rightPos->x)
		return 1;
	else if (leftPos->y < rightPos->y)
		return -1;
	else if (leftPos->y > rightPos->y)
		return 1;
	return 0;
}

static bool getEdgeBounds(void* outBounds, const dsBVH* bvh, const void* object)
{
	DS_ASSERT(dsBVH_getAxisCount(bvh) == 2 && dsBVH_getElement(bvh) == dsGeometryElement_Double);

	const dsSimplePolygon* polygon = (const dsSimplePolygon*)dsBVH_getUserData(bvh);
	const Edge* polygonEdge = polygon->edges + (size_t)object;
	dsAlignedBox2d* bounds = (dsAlignedBox2d*)outBounds;

	const Vertex* prevVert = polygon->vertices + polygonEdge->prevVertex;
	const Vertex* nextVert = polygon->vertices +  polygonEdge->nextVertex;
	bounds->min = bounds->max = prevVert->point;
	dsAlignedBox2_addPoint(*bounds, nextVert->point);
	return true;
}

static bool intersectPolygonEdge(void* userData, const dsBVH* bvh, const void* object,
	const void* bounds)
{
	EdgeIntersectInfo* info = (EdgeIntersectInfo*)userData;
	DS_ASSERT(!info->intersects);
	const dsSimplePolygon* polygon = (const dsSimplePolygon*)dsBVH_getUserData(bvh);
	const Edge* otherEdge = polygon->edges + (size_t)object;
	const dsAlignedBox2d* edgeBounds = (const dsAlignedBox2d*)bounds;

	// Don't count neighboring edges.
	if (otherEdge->prevVertex == info->fromVert || otherEdge->prevVertex == info->toVert ||
		otherEdge->nextVertex == info->fromVert || otherEdge->nextVertex == info->toVert)
	{
		return true;
	}

	const dsVector2d* otherFrom = &polygon->vertices[otherEdge->prevVertex].point;
	const dsVector2d* otherTo = &polygon->vertices[otherEdge->nextVertex].point;

	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	double divisor = (info->fromPos.x - info->toPos.x)*(otherFrom->y - otherTo->y) -
		(info->fromPos.y - info->toPos.y)*(otherFrom->x - otherTo->x);
	if (divisor == 0.0)
		return true;
	divisor = 1.0/divisor;

	dsVector2d intersect =
	{{
		(info->fromPos.x*info->toPos.y - info->fromPos.y*info->toPos.x)*
				(otherFrom->x - otherTo->x) -
			(info->fromPos.x - info->toPos.x)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
		(info->fromPos.x*info->toPos.y - info->fromPos.y*info->toPos.x)*
				(otherFrom->y - otherTo->y) -
			(info->fromPos.y - info->toPos.y)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
	}};
	dsVector2_scale(intersect, intersect, divisor);

	// Check the range of the maximum extents. This avoids precision issues when the line is
	// axis-aligned.
	dsVector2d extents;
	dsAlignedBox2_extents(extents, *edgeBounds);
	if (extents.x > extents.y)
	{
		info->intersects = intersect.x >= edgeBounds->min.x - EPSILON &&
			intersect.y <= edgeBounds->max.x + EPSILON;
	}
	else
	{
		info->intersects = intersect.y >= edgeBounds->min.y - EPSILON &&
			intersect.y <= edgeBounds->max.y + EPSILON;
	}

	return !info->intersects;
}

static bool canConnectPolygonEdge(const dsSimplePolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx)
{
	const Vertex* fromVert = polygon->vertices + fromVertIdx;
	const Vertex* toVert = polygon->vertices + toVertIdx;
	uint32_t prevEdge = fromVert->prevEdges[ConnectingEdge_Main];
	uint32_t nextEdge = fromVert->nextEdges[ConnectingEdge_Main];
	if (polygon->edges[prevEdge].prevVertex == toVertIdx ||
		polygon->edges[nextEdge].nextVertex == toVertIdx)
	{
		return false;
	}

	dsAlignedBox2d edgeBounds = {fromVert->point, fromVert->point};
	dsAlignedBox2_addPoint(edgeBounds, toVert->point);

	EdgeIntersectInfo info = {fromVert->point, toVert->point, fromVertIdx, toVertIdx, false};
	dsBVH_intersect(polygon->edgeBVH, &edgeBounds, &intersectPolygonEdge, &info);
	return !info.intersects;
}

static double edgeAngle(const dsSimplePolygon* polygon, uint32_t edge, const dsVector2d* referenceDir,
	bool flip, bool ccw)
{
	const Edge* polyEdge = polygon->edges + edge;
	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, polygon->vertices[polyEdge->nextVertex].point,
		polygon->vertices[polyEdge->prevVertex].point);
	if (flip)
		dsVector2_neg(edgeDir, edgeDir);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	double cosAngle = dsVector2_dot(edgeDir, *referenceDir);
	double angle = acos(dsClamp(cosAngle, -1.0, 1.0));
	if ((referenceDir->x*edgeDir.y - edgeDir.x*referenceDir->y >= 0.0) == ccw)
		angle = 2.0*M_PI - angle;
	return angle;
}

static uint32_t findPrevEdge(const dsSimplePolygon* polygon, const Vertex* vertex,
	const dsVector2d* referenceDir, bool ccw)
{
	uint32_t closestEdge = vertex->prevEdges[ConnectingEdge_Main];
	double closestAngle = edgeAngle(polygon, closestEdge, referenceDir, true, !ccw);
	for (int i = 1; i < ConnectingEdge_Count; ++i)
	{
		uint32_t edge = vertex->prevEdges[i];
		if (edge == NOT_FOUND)
			continue;

		double angle = edgeAngle(polygon, edge, referenceDir, true, !ccw);
		if (angle < closestAngle)
		{
			closestEdge = edge;
			closestAngle = angle;
		}
	}

	return closestEdge;
}

static uint32_t findNextEdge(const dsSimplePolygon* polygon, const Vertex* vertex,
	const dsVector2d* referenceDir, bool ccw)
{
	uint32_t closestEdge = vertex->nextEdges[ConnectingEdge_Main];
	double closestAngle = edgeAngle(polygon, closestEdge, referenceDir, false, ccw);
	for (int i = 1; i < ConnectingEdge_Count; ++i)
	{
		uint32_t edge = vertex->nextEdges[i];
		if (edge == NOT_FOUND)
			continue;

		double angle = edgeAngle(polygon, edge, referenceDir, false, ccw);
		if (angle < closestAngle)
		{
			closestEdge = edge;
			closestAngle = angle;
		}
	}

	return closestEdge;
}

static bool addVerticesAndEdges(dsSimplePolygon* polygon, const void* points, uint32_t pointCount,
	dsPolygonPositionFunction pointPositionFunc)
{
	DS_ASSERT(polygon->vertexCount == 0);
	DS_ASSERT(polygon->edgeCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->vertices, polygon->vertexCount,
			polygon->maxVertices, pointCount) ||
		!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->edges, polygon->edgeCount,
			polygon->maxEdges, pointCount))
	{
		return false;
	}

	DS_ASSERT(polygon->vertexCount == pointCount);
	DS_ASSERT(polygon->edgeCount == pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		Vertex* vertex = polygon->vertices + i;
		if (!pointPositionFunc(&vertex->point, points, polygon->userData, i))
			return false;

		if (i > 0 && dsVector2d_epsilonEqual(&vertex->point, &polygon->vertices[i - 1].point,
			EPSILON))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon may not have duplicate points in a series.");
			return false;
		}

		polygon->edges[i].prevVertex = i;
		polygon->edges[i].nextVertex = i == pointCount - 1 ? 0 : i + 1;
		polygon->edges[i].prevEdge = i == 0 ? pointCount - 1 : i - 1;
		polygon->edges[i].nextEdge = i == pointCount - 1 ? 0 : i + 1;
		polygon->edges[i].visited = false;

		vertex->prevEdges[ConnectingEdge_Main] = polygon->edges[i].prevEdge;
		vertex->nextEdges[ConnectingEdge_Main] = i;

		DS_STATIC_ASSERT(ConnectingEdge_Main == 0, unexpected_connecting_edge_index);
		for (int j = 1; j < ConnectingEdge_Count; ++j)
		{
			vertex->prevEdges[j] = NOT_FOUND;
			vertex->nextEdges[j] = NOT_FOUND;
		}
	}

	if (dsVector2d_epsilonEqual(&polygon->vertices[0].point,
		&polygon->vertices[polygon->vertexCount - 1].point, EPSILON))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon may not duplicate the first and last point.");
		return false;
	}

	if (!polygon->sortedVerts || polygon->maxSortedVerts < polygon->maxVertices)
	{
		DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->sortedVerts));
		polygon->maxSortedVerts = polygon->maxVertices;
		polygon->sortedVerts = DS_ALLOCATE_OBJECT_ARRAY(polygon->allocator, uint32_t,
			polygon->maxSortedVerts);
		if (!polygon->sortedVerts)
			return false;
	}

	for (uint32_t i = 0; i < polygon->vertexCount; ++i)
		polygon->sortedVerts[i] = i;

	dsSort(polygon->sortedVerts, polygon->vertexCount, sizeof(*polygon->sortedVerts),
		&comparePolygonVertex, polygon);
	return true;
}

static bool addSeparatingPolygonEdge(dsSimplePolygon* polygon, uint32_t from, uint32_t to, bool ccw)
{
	Vertex* fromVert = polygon->vertices + from;
	Vertex* toVert = polygon->vertices + to;
	bool fromLeft = fromVert->point.x < toVert->point.x ||
		(fromVert->point.x == toVert->point.x && fromVert->point.y < toVert->point.y);
	bool fromTop = fromVert->point.y < toVert->point.y ||
		(fromVert->point.y == toVert->point.y && fromVert->point.x < toVert->point.x);

	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, toVert->point, fromVert->point);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	ConnectingEdge fromLeftEdge, fromRightEdge, toLeftEdge, toRightEdge;
	if (fromTop)
	{
		fromLeftEdge = ConnectingEdge_LeftBottom;
		fromRightEdge = ConnectingEdge_RightBottom;
		toLeftEdge = ConnectingEdge_LeftTop;
		toRightEdge = ConnectingEdge_RightTop;
	}
	else
	{
		fromLeftEdge = ConnectingEdge_LeftTop;
		fromRightEdge = ConnectingEdge_RightTop;
		toLeftEdge = ConnectingEdge_LeftBottom;
		toRightEdge = ConnectingEdge_RightBottom;
	}

	if (fromLeft)
	{
		// Connect to the right.
		if (fromVert->prevEdges[fromRightEdge] != NOT_FOUND ||
			fromVert->nextEdges[fromRightEdge] != NOT_FOUND ||
			toVert->prevEdges[toLeftEdge] != NOT_FOUND ||
			toVert->nextEdges[toLeftEdge] != NOT_FOUND)
		{
			if (fromVert->prevEdges[fromRightEdge] == toVert->nextEdges[toLeftEdge])
				return true;

			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Invalid polygon goemetry.");
			return false;
		}
	}
	else
	{
		// Connect to the left.
		if (fromVert->prevEdges[fromLeftEdge] != NOT_FOUND ||
			fromVert->nextEdges[fromLeftEdge] != NOT_FOUND ||
			toVert->prevEdges[toRightEdge] != NOT_FOUND ||
			toVert->nextEdges[toRightEdge] != NOT_FOUND)
		{
			if (fromVert->prevEdges[fromLeftEdge] == toVert->nextEdges[toRightEdge])
				return true;

			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Invalid polygon goemetry.");
			return false;
		}
	}

	uint32_t fromPrevEdge = findPrevEdge(polygon, fromVert, &edgeDir, ccw);
	uint32_t fromNextEdge = findNextEdge(polygon, fromVert, &edgeDir, ccw);

	dsVector2_neg(edgeDir, edgeDir);
	uint32_t toPrevEdge = findPrevEdge(polygon, toVert, &edgeDir, ccw);
	uint32_t toNextEdge = findNextEdge(polygon, toVert, &edgeDir, ccw);

	// Insert two new edge in-between the edges for the "from" and "to" vertices, one for the left
	// and right sub-polygons.
	uint32_t curEdge = polygon->edgeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->edges, polygon->edgeCount,
		polygon->maxEdges, 1))
	{
		return false;
	}

	polygon->edges[curEdge].prevVertex = from;
	polygon->edges[curEdge].nextVertex = to;
	polygon->edges[curEdge].prevEdge = fromPrevEdge;
	polygon->edges[curEdge].nextEdge = toNextEdge;
	polygon->edges[curEdge].visited = false;
	polygon->edges[fromPrevEdge].nextEdge = curEdge;
	polygon->edges[toNextEdge].prevEdge = curEdge;
	if (fromLeft)
	{
		fromVert->nextEdges[fromRightEdge] = curEdge;
		toVert->prevEdges[toLeftEdge] = curEdge;
	}
	else
	{
		fromVert->nextEdges[fromLeftEdge] = curEdge;
		toVert->prevEdges[toRightEdge] = curEdge;
	}

	curEdge = polygon->edgeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->edges, polygon->edgeCount,
		polygon->maxEdges, 1))
	{
		return false;
	}

	polygon->edges[curEdge].prevVertex = to;
	polygon->edges[curEdge].nextVertex = from;
	polygon->edges[curEdge].prevEdge = toPrevEdge;
	polygon->edges[curEdge].nextEdge = fromNextEdge;
	polygon->edges[curEdge].visited = false;
	polygon->edges[toPrevEdge].nextEdge = curEdge;
	polygon->edges[fromNextEdge].prevEdge = curEdge;
	if (fromLeft)
	{
		fromVert->prevEdges[fromRightEdge] = curEdge;
		toVert->nextEdges[toLeftEdge] = curEdge;
	}
	else
	{
		fromVert->prevEdges[fromLeftEdge] = curEdge;
		toVert->nextEdges[toRightEdge] = curEdge;
	}

	return true;
}

static bool isLeft(const dsVector2d* point, const dsVector2d* reference)
{
	return point->x < reference->x || (point->x == reference->x && point->y < reference->y);
}

static uint32_t findOtherPoint(const dsSimplePolygon* polygon, const uint32_t* sortedVert,
	bool othersLeft)
{
	// Find the first point that's to the left/right of the vertex that doesn't intersect any edges.
	if (othersLeft)
	{
		const uint32_t* end = polygon->sortedVerts + polygon->vertexCount;
		for (const uint32_t* otherVert = sortedVert + 1; otherVert < end; ++otherVert)
		{
			if (canConnectPolygonEdge(polygon, *sortedVert, *otherVert))
				return *otherVert;
		}
	}
	else
	{
		for (const uint32_t* otherVert = sortedVert; otherVert-- > polygon->sortedVerts;)
		{
			if (canConnectPolygonEdge(polygon, *sortedVert, *otherVert))
				return *otherVert;
		}
	}

	return NOT_FOUND;
}

static bool isTriangleCCW(const dsVector2d* p0, const dsVector2d* p1, const dsVector2d* p2)
{
	// Cross product of the triangle with Z = 0.
	double cross = (p1->x - p0->x)*(p2->y - p0->y) - (p2->x - p0->x)*(p1->y - p0->y);
	return cross >= 0.0f;
}

static bool isPolygonCCW(dsSimplePolygon* polygon)
{
	if (polygon->vertexCount == 0)
		return true;

	// First vertex is the lowest X value. (ties broken by lower Y values)
	// The triangle formed by this vertex and its connecting edges should be convex and match the
	// winding order of the polygon.
	uint32_t p0Vert = polygon->sortedVerts[0];
	uint32_t p1Vert =
		polygon->edges[polygon->vertices[p0Vert].nextEdges[ConnectingEdge_Main]].nextVertex;
	uint32_t p2Vert =
		polygon->edges[polygon->vertices[p0Vert].prevEdges[ConnectingEdge_Main]].prevVertex;
	return isTriangleCCW(&polygon->vertices[p0Vert].point, &polygon->vertices[p1Vert].point,
		&polygon->vertices[p2Vert].point);
}

static bool findPolygonLoops(dsSimplePolygon* polygon, bool ccw)
{
	for (uint32_t i = 0; i < polygon->vertexCount; ++i)
	{
		const uint32_t* sortedVert = polygon->sortedVerts + i;
		uint32_t prev = *sortedVert == 0 ? polygon->vertexCount - 1 : *sortedVert - 1;
		uint32_t next = *sortedVert == polygon->vertexCount - 1 ? 0 : *sortedVert + 1;

		bool prevLeft =
			isLeft(&polygon->vertices[prev].point, &polygon->vertices[*sortedVert].point);
		bool nextLeft =
			isLeft(&polygon->vertices[next].point, &polygon->vertices[*sortedVert].point);

		if (prevLeft != nextLeft)
			continue;

		bool triangleCCW = isTriangleCCW(&polygon->vertices[prev].point,
			&polygon->vertices[*sortedVert].point, &polygon->vertices[next].point);
		if (triangleCCW == ccw)
			continue;

		// Lazily create the BVH the first time we need it. This avoids an expensive operation for
		// polygons that are already monotone.
		if (!polygon->builtBVH)
		{
			if (!polygon->edgeBVH)
			{
				polygon->edgeBVH = dsBVH_create(polygon->allocator, 2, dsGeometryElement_Double,
					polygon);
				if (!polygon->edgeBVH)
					return false;
			}

			// Use indices since the edge array may be re-allocated, invalidating the pointers into
			// the array.
			if (!dsBVH_build(polygon->edgeBVH, NULL, polygon->edgeCount, DS_BVH_OBJECT_INDICES,
				&getEdgeBounds, false))
			{
				return false;
			}

			polygon->builtBVH = true;
		}

		uint32_t otherPoint = findOtherPoint(polygon, sortedVert, prevLeft);
		if (otherPoint == NOT_FOUND)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Invalid polygon goemetry.");
			return false;
		}

		if (!addSeparatingPolygonEdge(polygon, *sortedVert, otherPoint, ccw))
			return false;
	}

	// Need to reset the visited flags for the edges.
	for (uint32_t i = 0; i < polygon->edgeCount; ++i)
		polygon->edges[i].visited = false;

	return true;
}

static void clearLoopVertices(dsSimplePolygon* polygon)
{
	polygon->loopVertCount = 0;
	polygon->vertStackCount = 0;
}

static bool addLoopVertex(dsSimplePolygon* polygon, uint32_t polygonEdge)
{
	uint32_t index = polygon->loopVertCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->loopVertices, polygon->loopVertCount,
		polygon->maxLoopVerts, 1))
	{
		return false;
	}

	polygon->loopVertices[index].vertIndex = polygon->edges[polygonEdge].prevVertex;
	polygon->loopVertices[index].prevVert =
		polygon->edges[polygon->edges[polygonEdge].prevEdge].prevVertex;
	polygon->loopVertices[index].nextVert = polygon->edges[polygonEdge].nextVertex;
	return true;
}

static bool pushVertex(dsSimplePolygon* polygon, uint32_t loopVert)
{
	uint32_t index = polygon->vertStackCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->vertexStack, polygon->vertStackCount,
		polygon->maxVertStack, 1))
	{
		return false;
	}

	polygon->vertexStack[index] = loopVert;
	return true;
}

static bool addIndex(dsSimplePolygon* polygon, uint32_t indexValue)
{
	uint32_t index = polygon->indexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->indices, polygon->indexCount,
		polygon->maxIndices, 1))
	{
		return false;
	}

	polygon->indices[index] = indexValue;
	return true;
}

static bool triangulateLoop(dsSimplePolygon* polygon, uint32_t startEdge, bool ccw, bool targetCCW)
{
	clearLoopVertices(polygon);
	uint32_t nextEdge = startEdge;
	do
	{
		polygon->edges[nextEdge].visited = true;
		uint32_t curEdge = nextEdge;
		nextEdge = polygon->edges[nextEdge].nextEdge;
		if (!addLoopVertex(polygon, curEdge))
			return false;
	} while (nextEdge != startEdge);

	if (polygon->loopVertCount < 3)
		return true;

	// Monotone polygon triangulation: https://www.cs.ucsb.edu/~suri/cs235/Triangulation.pdf
	dsSort(polygon->loopVertices, polygon->loopVertCount, sizeof(*polygon->loopVertices),
		&compareLoopVertex, polygon);
	if (!pushVertex(polygon, 0) || !pushVertex(polygon, 1))
		return false;

	uint32_t totalTriangles = 0;
	for (uint32_t i = 2; i < polygon->loopVertCount; ++i)
	{
		DS_ASSERT(polygon->vertStackCount > 0);
		uint32_t stackIndex = polygon->vertStackCount - 1;
		uint32_t top = polygon->vertexStack[stackIndex];
		uint32_t iVertIndex = polygon->loopVertices[i].vertIndex;
		bool isPrev = polygon->loopVertices[top].prevVert == iVertIndex;
		bool isNext = polygon->loopVertices[top].nextVert == iVertIndex;
		// At most one should be set.
		DS_ASSERT(isPrev + isNext < 2);

		const dsVector2d* p0 = &polygon->vertices[polygon->loopVertices[i].vertIndex].point;
		if (isPrev || isNext)
		{
			DS_ASSERT(polygon->vertStackCount >= 0);
			uint32_t addedTriangles = 0;
			for (uint32_t j = stackIndex; j-- > 0; ++addedTriangles)
			{
				uint32_t p1Vert = polygon->vertexStack[j];
				uint32_t p2Vert = polygon->vertexStack[j + 1];
				const dsVector2d* p1 = &polygon->vertices[
					polygon->loopVertices[p1Vert].vertIndex].point;
				const dsVector2d* p2 = &polygon->vertices[
					polygon->loopVertices[p2Vert].vertIndex].point;
				// Add triangles along the chain so long as they are inside the polygon.
				bool expectedCCW = ccw;
				if (!isNext)
					expectedCCW = !expectedCCW;
				bool triangleCCW = isTriangleCCW(p0, p1, p2);
				if (triangleCCW != expectedCCW)
					break;

				if (triangleCCW != targetCCW)
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!addIndex(polygon, polygon->loopVertices[i].vertIndex) ||
					!addIndex(polygon, polygon->loopVertices[p1Vert].vertIndex) ||
					!addIndex(polygon, polygon->loopVertices[p2Vert].vertIndex))
				{
					return false;
				}
			}

			totalTriangles += addedTriangles;
			polygon->vertStackCount -= addedTriangles;
			pushVertex(polygon, i);
		}
		else if (polygon->vertStackCount)
		{
			for (uint32_t j = 0; j < polygon->vertStackCount - 1; ++j)
			{
				uint32_t p1Vert = polygon->vertexStack[j];
				uint32_t p2Vert = polygon->vertexStack[j + 1];
				const dsVector2d* p1 =
					&polygon->vertices[polygon->loopVertices[p1Vert].vertIndex].point;
				const dsVector2d* p2 =
					&polygon->vertices[polygon->loopVertices[p2Vert].vertIndex].point;

				if (isTriangleCCW(p0, p1, p2) != targetCCW)
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!addIndex(polygon, polygon->loopVertices[i].vertIndex) ||
					!addIndex(polygon, polygon->loopVertices[p1Vert].vertIndex) ||
					!addIndex(polygon, polygon->loopVertices[p2Vert].vertIndex))
				{
					return false;
				}
			}

			totalTriangles += polygon->vertStackCount - 1;
			polygon->vertStackCount = 0;
			pushVertex(polygon, top);
			pushVertex(polygon, i);
		}
	}

	if (totalTriangles != polygon->loopVertCount - 2)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop couldn't be triangulated.");
		return false;
	}

	return true;
}

dsSimplePolygon* dsSimplePolygon_create(dsAllocator* allocator, void* userData)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Simple polygon allocator must support freeing memory.");
		return NULL;
	}

	dsSimplePolygon* polygon = DS_ALLOCATE_OBJECT(allocator, dsSimplePolygon);
	if (!polygon)
		return NULL;

	memset(polygon, 0, sizeof(dsSimplePolygon));
	polygon->allocator = dsAllocator_keepPointer(allocator);
	polygon->userData = userData;
	return polygon;
}

void* dsSimplePolygon_getUserData(const dsSimplePolygon* polygon)
{
	if (!polygon)
		return NULL;

	return polygon->userData;
}

void dsSimplePolygon_setUserData(dsSimplePolygon* polygon, void* userData)
{
	if (polygon)
		polygon->userData = userData;
}

const uint32_t* dsSimplePolygon_triangulate(uint32_t* outIndexCount,
	dsSimplePolygon* polygon, const void* points, uint32_t pointCount,
	dsPolygonPositionFunction pointPositionFunc, dsTriangulateWinding winding)
{
	if (outIndexCount)
		*outIndexCount = 0;

	if (!outIndexCount || !polygon || !points || pointCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!pointPositionFunc)
		pointPositionFunc = &defaultPointPosition;

	polygon->vertexCount = 0;
	polygon->edgeCount = 0;
	polygon->loopVertCount = 0;
	polygon->vertStackCount = 0;
	polygon->indexCount = 0;
	polygon->builtBVH = false;

	if (!addVerticesAndEdges(polygon, points, pointCount, pointPositionFunc))
		return NULL;

	// Add separating edges for monotone polygons.
	bool ccw = isPolygonCCW(polygon);
	if (!findPolygonLoops(polygon, ccw))
		return NULL;

	// Triangulate each loop.
	for (uint32_t i = 0; i < polygon->edgeCount; ++i)
	{
		if (polygon->edges[i].visited)
			continue;

		if (!triangulateLoop(polygon, i, ccw, winding == dsTriangulateWinding_CCW))
			return NULL;
	}

	*outIndexCount = polygon->indexCount;
	return polygon->indices;
}

void dsSimplePolygon_destroy(dsSimplePolygon* polygon)
{
	if (!polygon || !polygon->allocator)
		return;

	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->vertices));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->edges));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->sortedVerts));
	dsBVH_destroy(polygon->edgeBVH);
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->loopVertices));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->vertexStack));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->indices));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon));
}
