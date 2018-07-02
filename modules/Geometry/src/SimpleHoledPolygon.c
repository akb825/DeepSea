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

#include <DeepSea/Geometry/SimpleHoledPolygon.h>

#include "BasePolygon.h"
#include <DeepSea/Geometry/SimplePolygon.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <float.h>
#include <string.h>

typedef struct LoopInfo
{
	uint32_t prevLoop;
	uint32_t nextLoop;
	uint32_t loopIndex;
} LoopInfo;

typedef struct LoopVertex
{
	dsVector2d point;
	uint32_t index;
} LoopVertex;

struct dsSimpleHoledPolygon
{
	dsBasePolygon base;

	LoopInfo* loops;
	uint32_t loopCount;
	uint32_t maxLoops;

	uint32_t* equalVertexList;
	uint32_t* sortedLoopVerts;
	uint32_t maxEqualVertices;
	uint32_t maxSortedLoopVerts;

	LoopVertex* loopVerts;
	uint32_t loopVertCount;
	uint32_t maxLoopVerts;

	uint32_t mainEdgeCount;

	dsSimplePolygon* simplePolygon;
};

static bool isPolygonLoopCCW(dsBasePolygon* polygon, const dsSimplePolygonLoop* loop)
{
	// https://en.wikipedia.org/wiki/Shoelace_formula
	// Negative area is counter-clockwise, positive is clockwise.
	uint32_t curIndex = loop->firstPoint + loop->pointCount - 1;
	uint32_t nextIndex = loop->firstPoint;
	double doubleArea =
		(polygon->vertices[curIndex].point.x + polygon->vertices[nextIndex].point.x)*
		(polygon->vertices[curIndex].point.y - polygon->vertices[nextIndex].point.y);

	curIndex = loop->firstPoint;
	nextIndex = loop->firstPoint + 1;
	for (uint32_t i = 0; i < loop->pointCount - 1; ++i, ++curIndex, ++nextIndex)
	{
		doubleArea += (polygon->vertices[curIndex].point.x + polygon->vertices[nextIndex].point.x)*
			(polygon->vertices[curIndex].point.y - polygon->vertices[nextIndex].point.y);
	}

	return doubleArea <= 0.0;
}

static bool addVertices(dsSimpleHoledPolygon* polygon, const void* points,
	uint32_t pointCount, dsPolygonPositionFunction pointPositionFunc)
{
	dsBasePolygon* base = &polygon->base;
	DS_ASSERT(base->vertexCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->vertices, base->vertexCount,
		base->maxVertices, pointCount))
	{
		return false;
	}

	DS_ASSERT(base->vertexCount == pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		Vertex* vertex = base->vertices + i;
		if (!pointPositionFunc(&vertex->point, base->userData, points, i))
			return false;

		vertex->prevEdges.head.edge = NOT_FOUND;
		vertex->prevEdges.head.nextConnection = NOT_FOUND;
		vertex->prevEdges.tail = NOT_FOUND;
		vertex->nextEdges.head.edge = NOT_FOUND;
		vertex->nextEdges.head.nextConnection = NOT_FOUND;
		vertex->nextEdges.tail = NOT_FOUND;
	}

	return true;
}

static bool addLoopEdges(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t loopCount)
{
	dsBasePolygon* base = &polygon->base;

	uint32_t edgeCount = 0;
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		if (loops[i].pointCount == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Empty polygon loop.");
			return false;
		}

		if (!DS_IS_BUFFER_RANGE_VALID(loops[i].firstPoint, loops[i].pointCount, base->vertexCount))
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop points of out of range.");
			return false;
		}

		edgeCount += loops[i].pointCount;
	}

	DS_ASSERT(base->edgeCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->edges, base->edgeCount, base->maxEdges,
		edgeCount))
	{
		return false;
	}

	uint32_t baseEdgeIdx = 0;
	DS_ASSERT(base->edgeCount == edgeCount);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		const dsSimplePolygonLoop* loop = loops + i;
		bool loopCCW = isPolygonLoopCCW(base, loop);
		// Use CCW for outer loop, CW for inner loop.
		bool flip = i == 0 ? !loopCCW : loopCCW;

		for (uint32_t j = 0; j < loop->pointCount; ++j)
		{
			Vertex* vertex = base->vertices + loop->firstPoint + j;
			if (vertex->prevEdges.head.edge != NOT_FOUND)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG,
					"The same point may not be a part of multiple polygon loops.");
				return false;
			}

			Edge* edge = base->edges + baseEdgeIdx + j;
			edge->prevVertex = loop->firstPoint + j;
			if (flip)
			{
				edge->nextVertex = loop->firstPoint + (j == 0 ? loop->pointCount - 1 : j - 1);
				edge->prevEdge = baseEdgeIdx + (j == loop->pointCount - 1 ? 0 : j + 1);
				edge->nextEdge = baseEdgeIdx + (j == 0 ? loop->pointCount - 1 : j - 1);
			}
			else
			{
				edge->nextVertex = loop->firstPoint + (j == loop->pointCount - 1 ? 0 : j + 1);
				edge->prevEdge = baseEdgeIdx + (j == 0 ? loop->pointCount - 1 : j - 1);
				edge->nextEdge = baseEdgeIdx + (j == loop->pointCount - 1 ? 0 : j + 1);
			}
			edge->visited = false;

			vertex->prevEdges.head.edge = edge->prevEdge;
			vertex->nextEdges.head.edge = baseEdgeIdx + j;
		}

		if (dsVector2d_epsilonEqual(&base->vertices[loop->firstPoint].point,
			&base->vertices[loop->firstPoint + loop->pointCount - 1].point, base->equalEpsilon))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG,
				"Polygon loop may not duplicate the first and last point.");
			return false;
		}

		baseEdgeIdx += loop->pointCount;
	}

	polygon->mainEdgeCount = base->edgeCount;
	return true;
}

static bool findEqualVertices(dsSimpleHoledPolygon* polygon)
{
	dsBasePolygon* base = &polygon->base;
	if (!dsBasePolygon_sortVertices(&polygon->base))
		return false;

	if (!polygon->equalVertexList || polygon->maxEqualVertices < base->maxVertices)
	{
		DS_VERIFY(dsAllocator_free(base->allocator, polygon->equalVertexList));
		polygon->maxEqualVertices = base->maxVertices;
		polygon->equalVertexList = DS_ALLOCATE_OBJECT_ARRAY(base->allocator, uint32_t,
			polygon->maxEqualVertices);
		if (!polygon->equalVertexList)
			return false;
	}
	memset(polygon->equalVertexList, 0xFF, base->vertexCount*sizeof(uint32_t));

	double doubleEpsilon = 2*base->equalEpsilon;
	for (uint32_t i = 0; i < base->vertexCount; ++i)
	{
		uint32_t firstIndex = base->sortedVerts[i];
		if (polygon->equalVertexList[firstIndex] != NOT_FOUND)
			continue;

		// Vertices are sorted along x axis. Find all equal points until we're over two epsilon
		// away. Use indices to form a circular linked list.
		const dsVector2d* firstPoint = &base->vertices[firstIndex].point;
		uint32_t lastIndex = firstIndex;
		for (uint32_t j = i + 1; j < base->vertexCount; ++j)
		{
			uint32_t curIndex = base->sortedVerts[j];
			const dsVector2d* curPoint = &base->vertices[curIndex].point;
			if (curPoint->x > firstPoint->x + doubleEpsilon)
				break;

			// Use 2 epsilon to compare the X since we're starting on the left-most boundary.
			if (dsEpsilonEquald(firstPoint->x, curPoint->x, doubleEpsilon) &&
				dsEpsilonEquald(firstPoint->y, curPoint->y, base->equalEpsilon))
			{
				polygon->equalVertexList[lastIndex] = curIndex;
				polygon->equalVertexList[curIndex] = firstIndex;
				lastIndex = curIndex;
			}
		}
	}

	return true;
}

static bool addVerticesAndEdges(dsSimpleHoledPolygon* polygon, const void* points,
	uint32_t pointCount, const dsSimplePolygonLoop* loops, uint32_t loopCount,
	dsPolygonPositionFunction pointPositionFunc)
{
	if (!addVertices(polygon, points, pointCount, pointPositionFunc))
		return false;

	if (!addLoopEdges(polygon, loops, loopCount))
		return false;

	return findEqualVertices(polygon);
}

static bool canConnectEdge(const dsSimpleHoledPolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx)
{
	const dsBasePolygon* base = &polygon->base;

	// Check against the original edges using the acceleration structure.
	if (!dsBasePolygon_canConnectEdge(base, fromVertIdx, toVertIdx, true))
		return false;

	// Check against the new edges, sicne they should be few in number. Only need to check against
	// every other edge since they are in pairs.
	const dsVector2d* fromPos = &base->vertices[fromVertIdx].point;
	const dsVector2d* toPos = &base->vertices[toVertIdx].point;
	dsAlignedBox2d edgeBounds = {*fromPos, *fromPos};
	dsAlignedBox2_addPoint(edgeBounds, *toPos);

	DS_ASSERT(polygon->mainEdgeCount <= base->edgeCount);
	for (uint32_t i = polygon->mainEdgeCount; i < base->edgeCount; i += 2)
	{
		const Edge* otherEdge = base->edges + i;

		// Don't count neighboring edges.
		if (otherEdge->prevVertex == fromVertIdx || otherEdge->prevVertex == toVertIdx ||
			otherEdge->nextVertex == fromVertIdx || otherEdge->nextVertex == toVertIdx)
		{
			continue;
		};

		const dsVector2d* otherFrom = &base->vertices[otherEdge->prevVertex].point;
		const dsVector2d* otherTo = &base->vertices[otherEdge->nextVertex].point;
		if (dsPolygonEdgesIntersect(fromPos, toPos, otherFrom, otherTo, base->intersectEpsilon))
			return false;
	}

	return true;
}

static int compareLoopVertex(const void* left, const void* right, void* context)
{
	const Vertex* vertices = (const Vertex*)context;
	const dsVector2d* leftVert = &vertices[*(const uint32_t*)left].point;
	const dsVector2d* rightVert = &vertices[*(const uint32_t*)right].point;
	return dsComparePolygonPoints(leftVert, rightVert);
}

static bool loopsConnected(const dsSimpleHoledPolygon* polygon, uint32_t firstLoop,
	uint32_t secondLoop)
{
	uint32_t nextLoop = firstLoop;
	do
	{
		if (nextLoop == secondLoop)
			return true;

		nextLoop = polygon->loops[nextLoop].nextLoop;
	} while (nextLoop != firstLoop);
	return false;
}

static bool verticesEqual(const dsSimpleHoledPolygon* polygon, uint32_t firstVertex,
	uint32_t secondVertex)
{
	if (firstVertex == secondVertex)
		return true;

	if (polygon->equalVertexList[firstVertex] == NOT_FOUND)
		return false;

	uint32_t curVertex = firstVertex;
	do
	{
		curVertex = polygon->equalVertexList[curVertex];
		if (curVertex == secondVertex)
			return true;
	} while (curVertex != firstVertex);

	return false;
}

static void connectLoopInfos(const dsSimpleHoledPolygon* polygon, uint32_t firstLoopIdx,
	uint32_t secondLoopIdx)
{
	if (firstLoopIdx == secondLoopIdx || loopsConnected(polygon, firstLoopIdx, secondLoopIdx))
		return;

	LoopInfo* firstLoop = polygon->loops + firstLoopIdx;
	uint32_t firstNext = firstLoop->nextLoop;

	LoopInfo* secondLoop = polygon->loops + secondLoopIdx;
	uint32_t secondPrev = secondLoop->prevLoop;

	firstLoop->nextLoop = secondLoopIdx;
	secondLoop->prevLoop = firstLoopIdx;

	polygon->loops[firstNext].prevLoop = secondPrev;
	polygon->loops[secondPrev].nextLoop = firstNext;
}

static bool initializeLoops(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t loopCount)
{
	dsBasePolygon* base = &polygon->base;
	polygon->loopCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, polygon->loops, polygon->loopCount,
		polygon->maxLoops, loopCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < loopCount; ++i)
		polygon->loops[i].loopIndex = i;

	if (!polygon->sortedLoopVerts || polygon->maxSortedLoopVerts < base->maxVertices)
	{
		DS_VERIFY(dsAllocator_free(base->allocator, polygon->sortedLoopVerts));
		polygon->maxSortedLoopVerts = base->maxVertices;
		polygon->sortedLoopVerts = DS_ALLOCATE_OBJECT_ARRAY(base->allocator, uint32_t,
			polygon->maxSortedLoopVerts);
		if (!polygon->sortedLoopVerts)
			return false;
	}

	// Cache the loop index for now. Will replace with the vertex indices and sorted after.
	memset(polygon->sortedLoopVerts, 0, sizeof(uint32_t)*base->vertexCount);
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		polygon->loops[i].prevLoop = polygon->loops[i].nextLoop = i;
		const dsSimplePolygonLoop* loop = loops + i;
		for (uint32_t j = 0; j < loop->pointCount; ++j)
			polygon->sortedLoopVerts[loop->firstPoint + j] = i;
	}

	// Connect loop infos for connected vertices.
	for (uint32_t i = 0; i < base->vertexCount; ++i)
	{
		uint32_t vertexIdx = base->sortedVerts[i];
		uint32_t loopIdx = polygon->sortedLoopVerts[vertexIdx];
		if (loopIdx == NOT_FOUND)
			continue;

		if (polygon->equalVertexList[vertexIdx] != NOT_FOUND)
		{
			uint32_t nextVertexIdx = vertexIdx;
			do
			{
				nextVertexIdx = polygon->equalVertexList[nextVertexIdx];
				if (nextVertexIdx == vertexIdx)
					break;

				uint32_t otherLoopIdx = polygon->sortedLoopVerts[nextVertexIdx];
				if (otherLoopIdx == NOT_FOUND)
					continue;

				connectLoopInfos(polygon, loopIdx, otherLoopIdx);
			} while (true);
		}
	}

	// Sort the loop vertices to speed up hole connection.
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		const dsSimplePolygonLoop* loop = loops + i;
		for (uint32_t j = 0; j < loop->pointCount; ++j)
		{
			uint32_t vertIdx = loop->firstPoint + j;
			polygon->sortedLoopVerts[vertIdx] = vertIdx;
		}

		dsSort(polygon->sortedLoopVerts + loop->firstPoint, loop->pointCount, sizeof(uint32_t),
			&compareLoopVertex, base->vertices);
	}

	return true;
}

static int connectLoopVertices(dsSimpleHoledPolygon* polygon, uint32_t fromLoopIdx,
	uint32_t toLoopIdx, uint32_t fromVert, uint32_t toVert, bool ccw)
{
	dsBasePolygon* base = &polygon->base;
	if (canConnectEdge(polygon, fromVert, toVert))
	{
		if (!dsBasePolygon_addSeparatingEdge(base, fromVert, toVert, ccw))
			return -1;
		connectLoopInfos(polygon, fromLoopIdx, toLoopIdx);
		return true;
	}
	return false;
}

static int connectToLoop(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t fromLoopIdx, uint32_t toLoopIdx)
{
	// Try to connect the left-most vertex to any vertex in the other loop. When done with every
	// loop in the full polygon, they should eventually all connect to the outer loop.
	dsBasePolygon* base = &polygon->base;
	uint32_t fromVert = polygon->sortedLoopVerts[loops[fromLoopIdx].firstPoint];
	const dsSimplePolygonLoop* toLoop = loops + toLoopIdx;
	bool ccw = toLoopIdx == 0;

	// Start at the closest vertex, and go progressively out to test the likliest vertices first.
	uint32_t* toLoopBeginVert = polygon->sortedLoopVerts + toLoop->firstPoint;
	uint32_t* toLoopEndVert = toLoopBeginVert + toLoop->pointCount;
	uint32_t* toLoopLeftVert = (uint32_t*)dsBinarySearchLowerBound(&fromVert,
		polygon->sortedLoopVerts + toLoop->firstPoint, toLoop->pointCount, sizeof(uint32_t),
		&compareLoopVertex, base->vertices);
	if (!toLoopLeftVert)
		toLoopLeftVert = toLoopEndVert - 1;
	uint32_t* toLoopRightVert = toLoopLeftVert + 1;
	while (toLoopLeftVert >= toLoopBeginVert || toLoopRightVert < toLoopEndVert)
	{
		if (toLoopLeftVert >= toLoopBeginVert)
		{
			int result = connectLoopVertices(polygon, fromLoopIdx, toLoopIdx, fromVert,
				*toLoopLeftVert, ccw);
			if (result != false)
				return result;

			--toLoopLeftVert;
		}

		if (toLoopRightVert < toLoopEndVert)
		{
			int result = connectLoopVertices(polygon, fromLoopIdx, toLoopIdx, fromVert,
				*toLoopRightVert, ccw);
			if (result != false)
				return result;

			++toLoopRightVert;
		}
	}

	return false;
}

static int connectToLoopGroup(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t fromLoopIdx, uint32_t toLoopIdx)
{
	uint32_t curToLoopIdx = toLoopIdx;
	do
	{
		int result = connectToLoop(polygon, loops, fromLoopIdx, curToLoopIdx);
		if (result != 0)
			return result;

		curToLoopIdx = polygon->loops[curToLoopIdx].nextLoop;
	} while (curToLoopIdx != toLoopIdx);
	return false;
}

static bool connectLoops(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t loopCount)
{
	if (!initializeLoops(polygon, loops, loopCount))
		return false;

	DS_ASSERT(polygon->loopCount == loopCount);
	if (loopCount <= 1)
		return true;

	dsBasePolygon* base = &polygon->base;
	DS_ASSERT(polygon->mainEdgeCount == base->edgeCount);
	if (!dsBasePolygon_buildEdgeBVH(base))
		return false;

	uint32_t outerLoopIdx = 0;
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		if (polygon->loops[i].loopIndex == 0)
		{
			outerLoopIdx = i;
			break;
		}
	}

	for (uint32_t i = 0; i < loopCount; ++i)
	{
		if (loopsConnected(polygon, i, outerLoopIdx))
			continue;

		// Try to connect to the outer loop, or some other loop already connected to the outer loop.
		int result = connectToLoopGroup(polygon, loops, i, outerLoopIdx);
		if (result == -1)
			return false;

		if (result)
			continue;

		// If we couldn't connect to the outer loop, try to connect to some other loop that isn't
		// currently connected to this. It should eventually hook up to the outer loop once we're
		// finished processing all loops.
		for (uint32_t j = 0; j < loopCount; ++j)
		{
			if (loopsConnected(polygon, j, i) || loopsConnected(polygon, j, outerLoopIdx))
				continue;

			result = connectToLoopGroup(polygon, loops, i, j);
			if (result == -1)
				return false;
			else if (result)
				break;
		}

		if (!result)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Couldn't connect all polygon loops.");
			return false;
		}
	}

	// Double check that we could connect all loops.
	uint32_t connectedCount = 0;
	uint32_t curLoopIdx = 0;
	do
	{
		++connectedCount;
		curLoopIdx = polygon->loops[curLoopIdx].nextLoop;
	} while (curLoopIdx != 0);

	if (connectedCount != loopCount)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Couldn't connect all polygon loops.");
		return false;
	}

	return true;
}

static uint32_t findNextEdge(const dsSimpleHoledPolygon* polygon, uint32_t edgeIdx)
{
	const dsBasePolygon* base = &polygon->base;

	const Edge* edge = base->edges + edgeIdx;
	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, base->vertices[edge->prevVertex].point,
		base->vertices[edge->nextVertex].point);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	uint32_t closestEdge = NOT_FOUND;
	double closestAngle = DBL_MAX;

	uint32_t firstVert = edge->nextVertex;
	uint32_t curVert = firstVert;
	do
	{
		const EdgeConnection* curConnection = &base->vertices[curVert].nextEdges.head;
		do
		{
			double angle = dsBasePolygon_edgeAngle(base, curConnection->edge, &edgeDir, false,
				true);
			if (angle < closestAngle)
			{
				closestEdge = curConnection->edge;
				closestAngle = angle;
			}

			if (curConnection->nextConnection == NOT_FOUND)
				curConnection = NULL;
			else
				curConnection = base->edgeConnections + curConnection->nextConnection;
		} while (curConnection);

		curVert = polygon->equalVertexList[curVert];
	} while (curVert != firstVert && curVert != NOT_FOUND);

	return base->edges[closestEdge].visited ? NOT_FOUND : closestEdge;
}

bool getLoopPosition(dsVector2d* outPosition, void* userData, const void* points, uint32_t index)
{
	DS_UNUSED(userData);
	const dsSimpleHoledPolygon* polygon = (const dsSimpleHoledPolygon*)points;
	*outPosition = polygon->loopVerts[index].point;
	return true;
}

static bool triangulateLoop(dsSimpleHoledPolygon* polygon, uint32_t startEdge,
	dsTriangulateWinding winding)
{
	dsBasePolygon* base = &polygon->base;
	polygon->loopVertCount = 0;
	uint32_t edgeIdx = startEdge;
	uint32_t startPoint = base->edges[edgeIdx].prevVertex;
	do
	{
		Edge* edge = base->edges + edgeIdx;
		DS_ASSERT(!edge->visited);
		uint32_t loopVertIdx = polygon->loopVertCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, polygon->loopVerts, polygon->loopVertCount,
			polygon->maxLoopVerts, 1))
		{
			return false;
		}

		polygon->loopVerts[loopVertIdx].point = base->vertices[edge->prevVertex].point;
		polygon->loopVerts[loopVertIdx].index = edge->prevVertex;
		edge->visited = true;

		uint32_t nextPoint = edge->nextVertex;
		edgeIdx = findNextEdge(polygon, edgeIdx);
		if (edgeIdx == NOT_FOUND)
		{
			if (!verticesEqual(polygon, nextPoint, startPoint))
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Unexpected polygon geometry.");
				return false;
			}
			break;
		}
	} while (true);

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon->simplePolygon,
		polygon, polygon->loopVertCount, &getLoopPosition, winding);
	if (!indices)
		return false;

	uint32_t firstIndex = base->indexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->indices, base->indexCount, base->maxIndices,
		indexCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < indexCount; ++i)
		base->indices[firstIndex + i] = polygon->loopVerts[indices[i]].index;

	return true;
}

static bool triangulateLoops(dsSimpleHoledPolygon* polygon, dsTriangulateWinding winding)
{
	dsBasePolygon* base = &polygon->base;
	for (uint32_t i = 0; i < base->edgeCount; ++i)
	{
		if (base->edges[i].visited)
			continue;

		if (!triangulateLoop(polygon, i, winding))
			return false;
	}

	return true;
}

dsSimpleHoledPolygon* dsSimpleHoledPolygon_create(dsAllocator* allocator, void* userData,
	double equalEpsilon, double intersectEpsilon)
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

	dsSimpleHoledPolygon* polygon = DS_ALLOCATE_OBJECT(allocator, dsSimpleHoledPolygon);
	if (!polygon)
		return NULL;

	memset(polygon, 0, sizeof(dsSimpleHoledPolygon));
	polygon->base.allocator = dsAllocator_keepPointer(allocator);
	polygon->base.userData = userData;
	polygon->base.equalEpsilon = equalEpsilon;
	polygon->base.intersectEpsilon = intersectEpsilon;
	polygon->simplePolygon = dsSimplePolygon_create(allocator, userData, equalEpsilon,
		intersectEpsilon);
	if (!polygon->simplePolygon)
	{
		DS_VERIFY(dsAllocator_free(allocator, polygon));
		return NULL;
	}
	return polygon;
}

void* dsSimpleHoledPolygon_getUserData(const dsSimpleHoledPolygon* polygon)
{
	if (!polygon)
		return NULL;

	return polygon->base.userData;
}

void dsSimpleHoledPolygon_setUserData(dsSimpleHoledPolygon* polygon, void* userData)
{
	if (polygon)
		polygon->base.userData = userData;
}

double dsSimpleHoledPolygon_getEqualEpsilon(const dsSimpleHoledPolygon* polygon)
{
	if (!polygon)
		return 0.0;

	return polygon->base.equalEpsilon;
}

void dsSimpleHoledPolygon_setEqualEpsilon(dsSimpleHoledPolygon* polygon, double epsilon)
{
	if (!polygon)
		return;

	polygon->base.equalEpsilon = epsilon;
	dsSimplePolygon_setEqualEpsilon(polygon->simplePolygon, epsilon);
}

double dsSimpleHoledPolygon_getIntersectEpsilon(const dsSimpleHoledPolygon* polygon)
{
	if (!polygon)
		return 0.0;

	return polygon->base.intersectEpsilon;
}

void dsSimpleHoledPolygon_setIntersectEpsilon(dsSimpleHoledPolygon* polygon, double epsilon)
{
	if (!polygon)
		return;

	polygon->base.intersectEpsilon = epsilon;
	dsSimplePolygon_setIntersectEpsilon(polygon->simplePolygon, epsilon);
}

const uint32_t* dsSimpleHoledPolygon_triangulate(uint32_t* outIndexCount,
	dsSimpleHoledPolygon* polygon, const void* points, uint32_t pointCount,
	const dsSimplePolygonLoop* loops, uint32_t loopCount,
	dsPolygonPositionFunction pointPositionFunc, dsTriangulateWinding winding)
{
	if (outIndexCount)
		*outIndexCount = 0;

	if (!outIndexCount || !polygon || !points || pointCount == 0 || !loops || loopCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	dsBasePolygon* base = &polygon->base;
	dsBasePolygon_reset(base);

	// Single loop is the same as a normal simple polygon.
	if (loopCount == 1 && loops[0].firstPoint == 0)
	{
		if (loops[0].pointCount > pointCount)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop points of out of range.");
			return NULL;
		}

		return dsSimplePolygon_triangulate(outIndexCount, polygon->simplePolygon, points,
			loops[0].pointCount, pointPositionFunc, winding);
	}

	if (!pointPositionFunc)
		pointPositionFunc = &dsSimplePolygon_getPointVector2d;

	if (!addVerticesAndEdges(polygon, points, pointCount, loops, loopCount, pointPositionFunc))
		return NULL;

	if (!connectLoops(polygon, loops, loopCount))
		return NULL;

	if (!triangulateLoops(polygon, winding))
		return NULL;

	*outIndexCount = base->indexCount;
	return base->indices;
}

void dsSimpleHoledPolygon_destroy(dsSimpleHoledPolygon* polygon)
{
	if (!polygon || !polygon->base.allocator)
		return;

	dsBasePolygon_shutdown(&polygon->base);
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->equalVertexList));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->sortedLoopVerts));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->loops));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->loopVerts));
	dsSimplePolygon_destroy(polygon->simplePolygon);
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon));
}
