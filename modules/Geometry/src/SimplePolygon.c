/*
 * Copyright 2018-2021 Aaron Barany
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

#include "BasePolygon.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

// Basis of algorithm: https://www.cs.ucsb.edu/~suri/cs235/Triangulation.pdf

typedef struct LoopVertex
{
	uint32_t vertIndex;
	uint32_t prevVert;
	uint32_t nextVert;
} LoopVertex;

struct dsSimplePolygon
{
	dsBasePolygon base;

	LoopVertex* loopVertices;
	uint32_t loopVertCount;
	uint32_t maxLoopVerts;

	uint32_t* vertexStack;
	uint32_t vertStackCount;
	uint32_t maxVertStack;
};

static int compareLoopVertex(const void* left, const void* right, void* context)
{
	const dsBasePolygon* polygon = (const dsBasePolygon*)context;
	const LoopVertex* leftVert = (const LoopVertex*)left;
	const LoopVertex* rightVert = (const LoopVertex*)right;
	const dsVector2d* leftPos = &polygon->vertices[leftVert->vertIndex].point;
	const dsVector2d* rightPos = &polygon->vertices[rightVert->vertIndex].point;

	if (leftPos->x < rightPos->x - polygon->equalEpsilon)
		return -1;
	else if (leftPos->x > rightPos->x + polygon->equalEpsilon)
		return 1;
	else if (leftPos->y < rightPos->y - polygon->equalEpsilon)
		return -1;
	else if (leftPos->y > rightPos->y + polygon->equalEpsilon)
		return 1;
	return 0;
}

static bool addVerticesAndEdges(dsBasePolygon* polygon, const void* points, uint32_t pointCount,
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
		if (!pointPositionFunc(&vertex->point, polygon->userData, points, i))
			return false;

		if (i > 0 && dsVector2d_epsilonEqual(&vertex->point, &polygon->vertices[i - 1].point,
			polygon->equalEpsilon))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon may not have duplicate points in a series.");
			return false;
		}

		Edge* edge = polygon->edges + i;
		edge->prevVertex = i;
		edge->nextVertex = i == pointCount - 1 ? 0 : i + 1;
		edge->prevEdge = i == 0 ? pointCount - 1 : i - 1;
		edge->nextEdge = i == pointCount - 1 ? 0 : i + 1;
		edge->visited = false;

		vertex->prevEdges.head.edge = edge->prevEdge;
		vertex->prevEdges.head.nextConnection = NOT_FOUND;
		vertex->prevEdges.tail = NOT_FOUND;
		vertex->nextEdges.head.edge = i;
		vertex->nextEdges.head.nextConnection = NOT_FOUND;
		vertex->nextEdges.tail = NOT_FOUND;
	}

	if (dsVector2d_epsilonEqual(&polygon->vertices[0].point,
			&polygon->vertices[polygon->vertexCount - 1].point, polygon->equalEpsilon))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon may not duplicate the first and last point.");
		return false;
	}

	return dsBasePolygon_sortVertices(polygon);
}

static bool isLeft(const dsVector2d* point, const dsVector2d* reference, double epsilon)
{
	return point->x < reference->x - epsilon || (dsEpsilonEquald(point->x, reference->x, epsilon) &&
		point->y < reference->y);
}

static uint32_t findOtherPoint(const dsBasePolygon* polygon, const uint32_t* sortedVert,
	bool othersLeft, bool ccw)
{
	// Find the first point that's to the left/right of the vertex that doesn't intersect any edges.
	if (othersLeft)
	{
		const uint32_t* end = polygon->sortedVerts + polygon->vertexCount;
		for (const uint32_t* otherVert = sortedVert + 1; otherVert < end; ++otherVert)
		{
			if (dsBasePolygon_canConnectEdge(polygon, *sortedVert, *otherVert, ccw))
				return *otherVert;
		}
	}
	else
	{
		for (const uint32_t* otherVert = sortedVert; otherVert-- > polygon->sortedVerts;)
		{
			if (dsBasePolygon_canConnectEdge(polygon, *sortedVert, *otherVert, ccw))
				return *otherVert;
		}
	}

	return NOT_FOUND;
}

static bool isPolygonCCW(dsBasePolygon* polygon)
{
	if (polygon->vertexCount == 0)
		return true;

	// First vertex is the lowest X value. (ties broken by lower Y values)
	// The triangle formed by this vertex and its connecting edges should be convex and match the
	// winding order of the polygon.
	uint32_t p1Vert = polygon->sortedVerts[0];
	uint32_t p0Vert = p1Vert == 0 ? polygon->vertexCount - 1 : p1Vert - 1;
	uint32_t p2Vert = p1Vert == polygon->vertexCount - 1 ? 0 : p1Vert + 1;
	return dsIsPolygonTriangleCCW(&polygon->vertices[p0Vert].point,
		&polygon->vertices[p1Vert].point, &polygon->vertices[p2Vert].point);
}

static bool findMonotonicLoops(dsBasePolygon* polygon, bool ccw)
{
	for (uint32_t i = 0; i < polygon->vertexCount; ++i)
	{
		const uint32_t* sortedVert = polygon->sortedVerts + i;
		uint32_t prev = *sortedVert == 0 ? polygon->vertexCount - 1 : *sortedVert - 1;
		uint32_t next = *sortedVert == polygon->vertexCount - 1 ? 0 : *sortedVert + 1;

		bool prevLeft = isLeft(&polygon->vertices[prev].point,
			&polygon->vertices[*sortedVert].point, polygon->equalEpsilon);
		bool nextLeft = isLeft(&polygon->vertices[next].point,
			&polygon->vertices[*sortedVert].point, polygon->equalEpsilon);

		// Find inflection points in the X direction.
		if (prevLeft != nextLeft)
			continue;

		bool triangleCCW = dsIsPolygonTriangleCCW(&polygon->vertices[prev].point,
			&polygon->vertices[*sortedVert].point, &polygon->vertices[next].point);
		if (triangleCCW == ccw)
			continue;

		// Lazily create the BVH the first time we need it. This avoids an expensive operation for
		// polygons that are already monotone.
		if (!polygon->builtBVH)
			dsBasePolygon_buildEdgeBVH(polygon);

		uint32_t otherPoint = findOtherPoint(polygon, sortedVert, prevLeft, ccw);
		if (otherPoint == NOT_FOUND)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Invalid polygon goemetry.");
			return false;
		}

		if (!dsBasePolygon_addSeparatingEdge(polygon, *sortedVert, otherPoint, ccw))
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
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->base.allocator, polygon->loopVertices,
		polygon->loopVertCount, polygon->maxLoopVerts, 1))
	{
		return false;
	}

	LoopVertex* loopVertex = polygon->loopVertices + index;
	const dsBasePolygon* base = &polygon->base;
	loopVertex->vertIndex = base->edges[polygonEdge].prevVertex;
	loopVertex->prevVert = base->edges[base->edges[polygonEdge].prevEdge].prevVertex;
	loopVertex->nextVert = base->edges[polygonEdge].nextVertex;
	return true;
}

static bool pushVertex(dsSimplePolygon* polygon, uint32_t loopVert)
{
	uint32_t index = polygon->vertStackCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(polygon->base.allocator, polygon->vertexStack,
		polygon->vertStackCount, polygon->maxVertStack, 1))
	{
		return false;
	}

	polygon->vertexStack[index] = loopVert;
	return true;
}

static bool addIndex(dsBasePolygon* polygon, uint32_t indexValue)
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
	dsBasePolygon* base = &polygon->base;
	clearLoopVertices(polygon);
	uint32_t nextEdge = startEdge;
	do
	{
		if (base->edges[nextEdge].visited)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Unexpected polygon geometry.");
			return false;
		}

		base->edges[nextEdge].visited = true;
		uint32_t curEdge = nextEdge;
		nextEdge = base->edges[nextEdge].nextEdge;
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

		const dsVector2d* p0 = &base->vertices[polygon->loopVertices[i].vertIndex].point;
		if (isPrev || isNext)
		{
			DS_ASSERT(polygon->vertStackCount >= 0);
			uint32_t addedTriangles = 0;
			for (uint32_t j = stackIndex; j-- > 0; ++addedTriangles)
			{
				uint32_t p1Vert = polygon->vertexStack[j];
				uint32_t p2Vert = polygon->vertexStack[j + 1];
				const dsVector2d* p1 = &base->vertices[
					polygon->loopVertices[p1Vert].vertIndex].point;
				const dsVector2d* p2 = &base->vertices[
					polygon->loopVertices[p2Vert].vertIndex].point;
				// Add triangles along the chain so long as they are inside the polygon and not
				// degenerate.
				bool expectedCCW = ccw;
				if (!isNext)
					expectedCCW = !expectedCCW;
				bool triangleCCW = dsIsPolygonTriangleCCW(p0, p1, p2);
				if (triangleCCW != expectedCCW ||
					(dsEpsilonEquald(p0->x, p1->x, base->equalEpsilon) &&
						dsEpsilonEquald(p0->x, p2->x, base->equalEpsilon)))
				{
					break;
				}

				if (triangleCCW != targetCCW)
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!addIndex(base, polygon->loopVertices[i].vertIndex) ||
					!addIndex(base, polygon->loopVertices[p1Vert].vertIndex) ||
					!addIndex(base, polygon->loopVertices[p2Vert].vertIndex))
				{
					return false;
				}
			}

			totalTriangles += addedTriangles;
			polygon->vertStackCount -= addedTriangles;
		}
		else
		{
			DS_ASSERT(polygon->vertStackCount > 0);
			for (uint32_t j = 0; j < polygon->vertStackCount - 1; ++j)
			{
				uint32_t p1Vert = polygon->vertexStack[j];
				uint32_t p2Vert = polygon->vertexStack[j + 1];
				const dsVector2d* p1 =
					&base->vertices[polygon->loopVertices[p1Vert].vertIndex].point;
				const dsVector2d* p2 =
					&base->vertices[polygon->loopVertices[p2Vert].vertIndex].point;

				if (dsIsPolygonTriangleCCW(p0, p1, p2) != targetCCW)
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!addIndex(base, polygon->loopVertices[i].vertIndex) ||
					!addIndex(base, polygon->loopVertices[p1Vert].vertIndex) ||
					!addIndex(base, polygon->loopVertices[p2Vert].vertIndex))
				{
					return false;
				}
			}

			totalTriangles += polygon->vertStackCount - 1;
			polygon->vertStackCount = 0;
			pushVertex(polygon, top);
		}

		pushVertex(polygon, i);
	}

	if (totalTriangles != polygon->loopVertCount - 2)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop couldn't be triangulated.");
		return false;
	}

	return true;
}

bool dsSimplePolygon_getPointVector2f(dsVector2d* outPosition, void* userData, const void* points,
	uint32_t index)
{
	DS_UNUSED(userData);
	const dsVector2f* point = (const dsVector2f*)points + index;
	outPosition->x = point->x;
	outPosition->y = point->y;
	return true;
}

bool dsSimplePolygon_getPointVector2d(dsVector2d* outPosition, void* userData, const void* points,
	uint32_t index)
{
	DS_UNUSED(userData);
	*outPosition = ((const dsVector2d*)points)[index];
	return true;
}

bool dsSimplePolygon_getPointVector2i(dsVector2d* outPosition, void* userData, const void* points,
	uint32_t index)
{
	DS_UNUSED(userData);
	const dsVector2i* point = (const dsVector2i*)points + index;
	outPosition->x = point->x;
	outPosition->y = point->y;
	return true;
}

dsSimplePolygon* dsSimplePolygon_create(dsAllocator* allocator, void* userData, double equalEpsilon,
	double intersectEpsilon)
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
	polygon->base.allocator = dsAllocator_keepPointer(allocator);
	polygon->base.userData = userData;
	polygon->base.equalEpsilon = equalEpsilon;
	polygon->base.intersectEpsilon = intersectEpsilon;
	return polygon;
}

void* dsSimplePolygon_getUserData(const dsSimplePolygon* polygon)
{
	if (!polygon)
		return NULL;

	return polygon->base.userData;
}

void dsSimplePolygon_setUserData(dsSimplePolygon* polygon, void* userData)
{
	if (polygon)
		polygon->base.userData = userData;
}

double dsSimplePolygon_getEqualEpsilon(const dsSimplePolygon* polygon)
{
	if (!polygon)
		return 0.0;

	return polygon->base.equalEpsilon;
}

void dsSimplePolygon_setEqualEpsilon(dsSimplePolygon* polygon, double epsilon)
{
	if (polygon)
		polygon->base.equalEpsilon = epsilon;
}

double dsSimplePolygon_getIntersectEpsilon(const dsSimplePolygon* polygon)
{
	if (!polygon)
		return 0.0;

	return polygon->base.intersectEpsilon;
}

void dsSimplePolygon_setIntersectEpsilon(dsSimplePolygon* polygon, double epsilon)
{
	if (polygon)
		polygon->base.intersectEpsilon = epsilon;
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
		pointPositionFunc = &dsSimplePolygon_getPointVector2d;

	dsBasePolygon* base = &polygon->base;
	dsBasePolygon_reset(base);
	polygon->loopVertCount = 0;
	polygon->vertStackCount = 0;

	if (!addVerticesAndEdges(base, points, pointCount, pointPositionFunc))
		return NULL;

	// Add separating edges for monotone polygons.
	bool ccw = isPolygonCCW(base);
	if (!findMonotonicLoops(base, ccw))
		return NULL;

	// Triangulate each loop.
	for (uint32_t i = 0; i < base->edgeCount; ++i)
	{
		if (base->edges[i].visited)
			continue;

		if (!triangulateLoop(polygon, i, ccw, winding == dsTriangulateWinding_CCW))
			return NULL;
	}

	*outIndexCount = base->indexCount;
	return base->indices;
}

void dsSimplePolygon_destroy(dsSimplePolygon* polygon)
{
	if (!polygon || !polygon->base.allocator)
		return;

	dsBasePolygon_shutdown(&polygon->base);
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->loopVertices));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->vertexStack));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon));
}
