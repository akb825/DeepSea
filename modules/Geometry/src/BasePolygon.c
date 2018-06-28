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

#include "BasePolygon.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/BVH.h>

typedef struct EdgeIntersectInfo
{
	dsVector2d fromPos;
	dsVector2d toPos;
	uint32_t fromVert;
	uint32_t toVert;
	bool intersects;
} EdgeIntersectInfo;

static int comparePolygonVertex(const void* left, const void* right, void* context)
{
	const Vertex* vertices = (const Vertex*)context;
	const dsVector2d* leftVert = &vertices[*(const uint32_t*)left].point;
	const dsVector2d* rightVert = &vertices[*(const uint32_t*)right].point;
	return dsComparePolygonPoints(leftVert, rightVert);
}

static bool getEdgeBounds(void* outBounds, const dsBVH* bvh, const void* object)
{
	DS_ASSERT(dsBVH_getAxisCount(bvh) == 2 && dsBVH_getElement(bvh) == dsGeometryElement_Double);

	const dsBasePolygon* polygon = (const dsBasePolygon*)dsBVH_getUserData(bvh);
	const Edge* polygonEdge = polygon->edges + (size_t)object;
	dsAlignedBox2d* bounds = (dsAlignedBox2d*)outBounds;

	const Vertex* prevVert = polygon->vertices + polygonEdge->prevVertex;
	const Vertex* nextVert = polygon->vertices +  polygonEdge->nextVertex;
	bounds->min = bounds->max = prevVert->point;
	dsAlignedBox2_addPoint(*bounds, nextVert->point);
	return true;
}

static bool testEdgeIntersect(void* userData, const dsBVH* bvh, const void* object,
	const void* bounds)
{
	DS_UNUSED(bounds);
	EdgeIntersectInfo* info = (EdgeIntersectInfo*)userData;
	DS_ASSERT(!info->intersects);
	const dsBasePolygon* polygon = (const dsBasePolygon*)dsBVH_getUserData(bvh);
	const Edge* otherEdge = polygon->edges + (size_t)object;

	// Don't count neighboring edges.
	if (otherEdge->prevVertex == info->fromVert || otherEdge->prevVertex == info->toVert ||
		otherEdge->nextVertex == info->fromVert || otherEdge->nextVertex == info->toVert)
	{
		return true;
	}

	const dsVector2d* otherFrom = &polygon->vertices[otherEdge->prevVertex].point;
	const dsVector2d* otherTo = &polygon->vertices[otherEdge->nextVertex].point;
	info->intersects = dsPolygonEdgesIntersect(&info->fromPos, &info->toPos, otherFrom, otherTo);
	return !info->intersects;
}

static bool isConnected(const dsBasePolygon* polygon, const EdgeConnectionList* connections,
	uint32_t nextVertex)
{
	const EdgeConnection* curConnection = &connections->head;
	do
	{
		if (polygon->edges[curConnection->edge].nextVertex == nextVertex)
			return true;
		else if (curConnection->nextConnection == NOT_FOUND)
			return false;

		curConnection = polygon->edgeConnections + curConnection->nextConnection;
	} while (true);
}

static bool connectingEdgeInternal(const dsBasePolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx, bool ccw)
{
	const Vertex* fromVert = polygon->vertices + fromVertIdx;
	const Vertex* toVert = polygon->vertices + toVertIdx;

	// Triangle made with edge that's the closest angle to the connection should be inside the
	// polygon. (i.e. same winding order)
	uint32_t toPrevEdge = toVert->prevEdges.head.edge;
	uint32_t toNextEdge = toVert->nextEdges.head.edge;

	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, toVert->point, fromVert->point);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	double prevAngle = dsBasePolygon_edgeAngle(polygon, toPrevEdge, &edgeDir, true, ccw);
	double nextAngle = dsBasePolygon_edgeAngle(polygon, toNextEdge, &edgeDir, true, ccw);
	if (prevAngle <= nextAngle)
	{
		const dsVector2d* p1 = &polygon->vertices[polygon->edges[toPrevEdge].prevVertex].point;
		return dsIsPolygonTriangleCCW(&fromVert->point, p1, &toVert->point) == ccw;
	}
	else
	{
		const dsVector2d* p2 = &polygon->vertices[polygon->edges[toNextEdge].nextVertex].point;
		return dsIsPolygonTriangleCCW(&fromVert->point, &toVert->point, p2) == ccw;
	}
}

static void insertEdge(dsBasePolygon* polygon, EdgeConnectionList* edgeList, uint32_t connectionIdx,
	uint32_t edgeIdx)
{
	polygon->edgeConnections[connectionIdx].edge = edgeIdx;
	polygon->edgeConnections[connectionIdx].nextConnection = NOT_FOUND;

	if (edgeList->tail == NOT_FOUND)
		edgeList->head.nextConnection = connectionIdx;
	else
		polygon->edgeConnections[edgeList->tail].nextConnection = connectionIdx;
	edgeList->tail = connectionIdx;
}

bool dsPolygonEdgesIntersect(const dsVector2d* from, const dsVector2d* to,
	const dsVector2d* otherFrom, const dsVector2d* otherTo)
{
	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	dsVector2d offset;
	dsVector2_sub(offset, *to, *from);

	double divisor = (from->x - to->x)*(otherFrom->y - otherTo->y) -
		(from->y - to->y)*(otherFrom->x - otherTo->x);
	if (dsEpsilonEquald(divisor, 0.0, EPSILON))
	{
		// Check if the lines are on top of each other.
		dsVector2d otherRef;
		if (dsVector2d_epsilonEqual(otherFrom, to, EPSILON))
			otherRef = *otherTo;
		else
			otherRef = *otherFrom;
		double betweenDivisor = (otherRef.x - to->x)*(from->y - otherTo->y) -
			(otherRef.y - to->y)*(otherFrom->x - otherTo->x);
		// Parallel, but not coincident.
		if (!dsEpsilonEquald(betweenDivisor, 0.0, EPSILON))
			return false;

		double distance = dsVector2d_len(&offset);
		dsVector2d otherFromOffset;
		dsVector2_sub(otherFromOffset, *otherFrom, *from);
		double otherFromOffsetSign = dsVector2_dot(otherFromOffset, offset) >= 0.0 ? 1.0 : -1.0;
		double otherFromT = dsVector2d_len(&otherFromOffset)*otherFromOffsetSign/distance;

		dsVector2d otherToOffset;
		dsVector2_sub(otherToOffset, *otherTo, *from);
		double otherToOffsetSign = dsVector2_dot(otherToOffset, offset) >= 0.0 ? 1.0 : -1.0;
		double otherToT = dsVector2d_len(&otherToOffset)*otherToOffsetSign/distance;

		double otherMinT = dsMin(otherFromT, otherToT);
		double otherMaxT = dsMax(otherFromT, otherToT);
		if (otherMaxT <= EPSILON || otherMinT >= 1.0 - EPSILON)
			return false;
		return true;
	}

	dsVector2d intersect =
	{{
		(from->x*to->y - from->y*to->x)*(otherFrom->x - otherTo->x) -
			(from->x - to->x)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
		(from->x*to->y - from->y*to->x)*(otherFrom->y - otherTo->y) -
			(from->y - to->y)*(otherFrom->x*otherTo->y - otherFrom->y*otherTo->x),
	}};

	divisor = 1.0/divisor;
	dsVector2_scale(intersect, intersect, divisor);

	double t;
	// Find T based on the largest difference to avoid issues with axis-aligned lines.
	if (fabs(offset.x) > fabs(offset.y))
		t = (intersect.x - from->x)/offset.x;
	else
		t = (intersect.y - from->y)/offset.y;

	return t > EPSILON && t < 1.0 - EPSILON;
}

bool dsIsPolygonTriangleCCW(const dsVector2d* p0, const dsVector2d* p1, const dsVector2d* p2);
int dsComparePolygonPoints(const dsVector2d* left, const dsVector2d* right);
void dsBasePolygon_reset(dsBasePolygon* polygon);

double dsBasePolygon_edgeAngle(const dsBasePolygon* polygon, uint32_t edge,
	const dsVector2d* referenceDir, bool flip, bool ccw)
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

uint32_t dsBasePolygon_findEdge(const dsBasePolygon* polygon, const EdgeConnectionList* edgeList,
	const dsVector2d* referenceDir, bool flip, bool ccw)
{
	if (flip)
		ccw = !ccw;

	const EdgeConnection* curEdge = &edgeList->head;
	uint32_t closestEdge = curEdge->edge;
	double closestAngle = dsBasePolygon_edgeAngle(polygon, closestEdge, referenceDir, flip, ccw);
	while (curEdge->nextConnection != NOT_FOUND)
	{
		curEdge = polygon->edgeConnections + curEdge->nextConnection;
		double angle = dsBasePolygon_edgeAngle(polygon, curEdge->edge, referenceDir, flip, ccw);
		if (angle < closestAngle)
		{
			closestEdge = curEdge->edge;
			closestAngle = angle;
		}
	}

	return closestEdge;
}

bool dsBasePolygon_sortVertices(dsBasePolygon* polygon)
{
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
		&comparePolygonVertex, polygon->vertices);
	return true;
}

bool dsBasePolygon_buildEdgeBVH(dsBasePolygon* polygon)
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
	if (!dsBVH_build(polygon->edgeBVH, NULL, polygon->edgeCount, DS_GEOMETRY_OBJECT_INDICES,
		&getEdgeBounds, false))
	{
		return false;
	}

	polygon->builtBVH = true;
	return true;
}

bool dsBasePolygon_canConnectEdge(const dsBasePolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx, bool ccw)
{
	const Vertex* fromVert = polygon->vertices + fromVertIdx;
	const Vertex* toVert = polygon->vertices + toVertIdx;
	if (dsVector2d_epsilonEqual(&fromVert->point, &toVert->point, EPSILON))
		return false;

	uint32_t fromPrevEdge = fromVert->prevEdges.head.edge;
	uint32_t fromNextEdge = fromVert->nextEdges.head.edge;
	if (polygon->edges[fromPrevEdge].prevVertex == toVertIdx ||
		polygon->edges[fromNextEdge].nextVertex == toVertIdx)
	{
		return false;
	}

	if (!connectingEdgeInternal(polygon, fromVertIdx, toVertIdx, ccw))
		return false;

	dsAlignedBox2d edgeBounds = {fromVert->point, fromVert->point};
	dsAlignedBox2_addPoint(edgeBounds, toVert->point);

	EdgeIntersectInfo info = {fromVert->point, toVert->point, fromVertIdx, toVertIdx, false};
	dsBVH_intersect(polygon->edgeBVH, &edgeBounds, &testEdgeIntersect, &info);
	return !info.intersects;
}

bool dsBasePolygon_addSeparatingEdge(dsBasePolygon* polygon, uint32_t from, uint32_t to, bool ccw)
{
	Vertex* fromVert = polygon->vertices + from;
	Vertex* toVert = polygon->vertices + to;
	if (isConnected(polygon, &fromVert->nextEdges, to))
		return true;

	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, toVert->point, fromVert->point);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	uint32_t fromPrevEdge = dsBasePolygon_findEdge(polygon, &fromVert->prevEdges, &edgeDir, true,
		ccw);
	uint32_t fromNextEdge = dsBasePolygon_findEdge(polygon, &fromVert->nextEdges, &edgeDir, false,
		ccw);

	dsVector2_neg(edgeDir, edgeDir);
	uint32_t toPrevEdge = dsBasePolygon_findEdge(polygon, &toVert->prevEdges, &edgeDir, true, ccw);
	uint32_t toNextEdge = dsBasePolygon_findEdge(polygon, &toVert->nextEdges, &edgeDir, false, ccw);

	// Insert two new edge in-between the edges for the "from" and "to" vertices, one for the left
	// and right sub-polygons.
	uint32_t firstEdgeIdx = polygon->edgeCount;
	uint32_t secondEdgeIdx = polygon->edgeCount + 1;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->edges, polygon->edgeCount,
		polygon->maxEdges, 2))
	{
		return false;
	}

	uint32_t fromFirstConnectionIdx = polygon->edgeConnectionCount;
	uint32_t toFirstConnectionIdx = polygon->edgeConnectionCount + 1;
	uint32_t fromSecondConnectionIdx = polygon->edgeConnectionCount + 2;
	uint32_t toSecondConnectionIdx = polygon->edgeConnectionCount + 3;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->allocator, polygon->edgeConnections,
		polygon->edgeConnectionCount, polygon->maxEdgeConnections, 4))
	{
		return false;
	}

	Edge* firstEdge = polygon->edges + firstEdgeIdx;
	firstEdge->prevVertex = from;
	firstEdge->nextVertex = to;
	firstEdge->prevEdge = fromPrevEdge;
	firstEdge->nextEdge = toNextEdge;
	firstEdge->visited = false;

	polygon->edges[fromPrevEdge].nextEdge = firstEdgeIdx;
	polygon->edges[toNextEdge].prevEdge = firstEdgeIdx;

	insertEdge(polygon, &fromVert->nextEdges, fromFirstConnectionIdx, firstEdgeIdx);
	insertEdge(polygon, &toVert->prevEdges, toFirstConnectionIdx, firstEdgeIdx);

	Edge* secondEdge = polygon->edges + secondEdgeIdx;
	secondEdge->prevVertex = to;
	secondEdge->nextVertex = from;
	secondEdge->prevEdge = toPrevEdge;
	secondEdge->nextEdge = fromNextEdge;
	secondEdge->visited = false;

	polygon->edges[toPrevEdge].nextEdge = secondEdgeIdx;
	polygon->edges[fromNextEdge].prevEdge = secondEdgeIdx;

	insertEdge(polygon, &fromVert->prevEdges, fromSecondConnectionIdx, secondEdgeIdx);
	insertEdge(polygon, &toVert->nextEdges, toSecondConnectionIdx, secondEdgeIdx);

	return true;
}

void dsBasePolygon_shutdown(dsBasePolygon* polygon)
{
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->vertices));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->edges));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->edgeConnections));
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->sortedVerts));
	dsBVH_destroy(polygon->edgeBVH);
	DS_VERIFY(dsAllocator_free(polygon->allocator, polygon->indices));
}
