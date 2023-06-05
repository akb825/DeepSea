/*
 * Copyright 2018-2023 Aaron Barany
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
	double epsilon;
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
	dsAlignedBox2d_addPoint(bounds, &nextVert->point);
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
	info->intersects = dsPolygonEdgesIntersect(&info->fromPos, &info->toPos, otherFrom, otherTo,
		info->epsilon);
	return !info->intersects;
}

#if DS_HAS_SIMD
static bool testEdgeIntersectSIMD(void* userData, const dsBVH* bvh, const void* object,
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
	info->intersects = dsPolygonEdgesIntersectSIMD(&info->fromPos, &info->toPos, otherFrom, otherTo,
		info->epsilon);
	return !info->intersects;
}
#endif

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

static double angleBetween(const dsVector2d* fromDir, const dsVector2d* toDir, bool ccw)
{
	dsVector2d invFromDir;
	dsVector2_neg(invFromDir, *fromDir);
	double cosAngle = dsVector2_dot(invFromDir, *toDir);
	double angle = acos(dsClamp(cosAngle, -1.0, 1.0));
	if ((fromDir->x*toDir->y - toDir->x*fromDir->y >= 0.0) != ccw)
		angle = 2.0*M_PI - angle;
	return angle;
}

static bool connectingEdgeInternal(const dsBasePolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx, bool ccw)
{
	const Vertex* fromVert = polygon->vertices + fromVertIdx;
	const Vertex* toVert = polygon->vertices + toVertIdx;
	uint32_t fromEdge = toVert->prevEdges.head.edge;
	uint32_t toEdge = toVert->nextEdges.head.edge;
	const Vertex* toPrevVert = polygon->vertices + polygon->edges[fromEdge].prevVertex;
	const Vertex* toNextVert = polygon->vertices + polygon->edges[toEdge].nextVertex;

	// Sum of angles for connecting edge should match the angle between the original edges.
	dsVector2d fromToDir, toFromDir, prevToDir, toNextDir;
	dsVector2_sub(fromToDir, toVert->point, fromVert->point);
	dsVector2d_normalize(&fromToDir, &fromToDir);
	dsVector2_neg(toFromDir, fromToDir);
	dsVector2_sub(prevToDir, toVert->point, toPrevVert->point);
	dsVector2d_normalize(&prevToDir, &prevToDir);
	dsVector2_sub(toNextDir, toNextVert->point, toVert->point);
	dsVector2d_normalize(&toNextDir, &toNextDir);

	double targetAngle = angleBetween(&prevToDir, &toNextDir, ccw);
	double combinedAngle = angleBetween(&prevToDir, &toFromDir, ccw) +
		angleBetween(&fromToDir, &toNextDir, ccw);
	return dsEpsilonEquald(targetAngle, combinedAngle, polygon->intersectEpsilon);
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
	const dsVector2d* otherFrom, const dsVector2d* otherTo, double epsilon)
{
	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	dsVector2d offset;
	dsVector2_sub(offset, *to, *from);

	double divisor = (from->x - to->x)*(otherFrom->y - otherTo->y) -
		(from->y - to->y)*(otherFrom->x - otherTo->x);
	if (dsEpsilonEqualsZerod(divisor, epsilon*epsilon))
	{
		// Check if the lines are on top of each other.
		dsVector2d otherRef;
		if (dsVector2d_epsilonEqual(otherFrom, to, epsilon))
			otherRef = *otherTo;
		else
			otherRef = *otherFrom;
		double betweenDivisor = (otherRef.x - to->x)*(from->y - otherTo->y) -
			(otherRef.y - to->y)*(otherFrom->x - otherTo->x);
		// Parallel, but not coincident.
		if (!dsEpsilonEqualsZerod(betweenDivisor, epsilon))
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
		if (otherMaxT <= epsilon || otherMinT >= 1.0 - epsilon)
			return false;
		return true;
	}

	double thisFactor = from->x*to->y - from->y*to->x;
	double otherFactor = otherFrom->x*otherTo->y - otherFrom->y*otherTo->x;
	dsVector2d intersect =
	{{
		thisFactor*(otherFrom->x - otherTo->x) - (from->x - to->x)*otherFactor,
		thisFactor*(otherFrom->y - otherTo->y) - (from->y - to->y)*otherFactor
	}};

	divisor = 1.0/divisor;
	dsVector2_scale(intersect, intersect, divisor);

	// Find T based on the largest difference to avoid issues with axis-aligned lines.
	double t;
	if (fabs(offset.x) > fabs(offset.y))
		t = (intersect.x - from->x)/offset.x;
	else
		t = (intersect.y - from->y)/offset.y;

	dsVector2d otherOffset;
	dsVector2_sub(otherOffset, *otherTo, *otherFrom);
	double otherT;
	if (fabs(otherOffset.x) > fabs(otherOffset.y))
		otherT = (intersect.x - otherFrom->x)/otherOffset.x;
	else
		otherT = (intersect.y - otherFrom->y)/otherOffset.y;

	// Don't count the endpoints of the first line, but count the endpoints of the second line as
	// an intersection.
	return t > epsilon && t < 1.0 - epsilon && otherT > -epsilon && otherT < 1.0 + epsilon;
}

#if DS_HAS_SIMD
#if DS_X86_32 || DS_X86_64
#define DS_SWAP_SIMD(a) _mm_shuffle_pd((a), (a), 0x1)
#elif DS_ARM_64
#define DS_SWAP_SIMD(a) vextq_f64((a), (a), 1)
#else
#define DS_SWAP_SIMD(a) a
#endif

DS_SIMD_START(DS_SIMD_DOUBLE2)
bool dsPolygonEdgesIntersectSIMD(const dsVector2d* from, const dsVector2d* to,
	const dsVector2d* otherFrom, const dsVector2d* otherTo, double epsilon)
{
	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	dsVector2d offset;
	offset.simd = dsSIMD2d_sub(to->simd, from->simd);

	dsSIMD2d fromToDiff = dsSIMD2d_sub(from->simd, to->simd);
	dsSIMD2d otherFromToDiff = dsSIMD2d_sub(otherFrom->simd, otherTo->simd);
	dsVector2d divisor2;
	divisor2.simd = dsSIMD2d_mul(fromToDiff, DS_SWAP_SIMD(otherFromToDiff));
	double divisor = divisor2.x - divisor2.y;
	if (dsEpsilonEqualsZerod(divisor, epsilon*epsilon))
	{
		// Check if the lines are on top of each other.
		dsVector2d otherRef;
		if (dsVector2d_epsilonEqual(otherFrom, to, epsilon))
			otherRef = *otherTo;
		else
			otherRef = *otherFrom;
		double betweenDivisor = (otherRef.x - to->x)*(from->y - otherTo->y) -
			(otherRef.y - to->y)*(otherFrom->x - otherTo->x);
		// Parallel, but not coincident.
		if (!dsEpsilonEqualsZerod(betweenDivisor, epsilon))
			return false;

		double distance = dsVector2d_len(&offset);
		dsVector2d otherFromOffset;
		dsVector2d_sub(&otherFromOffset, otherFrom, from);
		double otherFromOffsetSign = dsVector2d_dot(&otherFromOffset, &offset) >= 0.0 ? 1.0 : -1.0;
		double otherFromT = dsVector2d_len(&otherFromOffset)*otherFromOffsetSign/distance;

		dsVector2d otherToOffset;
		dsVector2d_sub(&otherToOffset, otherTo, from);
		double otherToOffsetSign = dsVector2d_dot(&otherToOffset, &offset) >= 0.0 ? 1.0 : -1.0;
		double otherToT = dsVector2d_len(&otherToOffset)*otherToOffsetSign/distance;

		double otherMinT = dsMin(otherFromT, otherToT);
		double otherMaxT = dsMax(otherFromT, otherToT);
		if (otherMaxT <= epsilon || otherMinT >= 1.0 - epsilon)
			return false;
		return true;
	}

	dsSIMD2d thisFactor2 = dsSIMD2d_mul(from->simd, DS_SWAP_SIMD(to->simd));
	dsSIMD2d otherFactor2 = dsSIMD2d_mul(otherFrom->simd, DS_SWAP_SIMD(otherTo->simd));
	dsSIMD2d_transpose(thisFactor2, otherFactor2);
	dsVector2d factors2;
	factors2.simd = dsSIMD2d_sub(thisFactor2, otherFactor2);
	dsVector2d intersect;
#if DS_SIMD_ALWAYS_FMA
	intersect.simd = dsSIMD2d_fmsub(dsSIMD2d_set1(factors2.x), otherFromToDiff,
		dsSIMD2d_mul(fromToDiff, dsSIMD2d_set1(factors2.y)));
#else
	intersect.simd = dsSIMD2d_sub(dsSIMD2d_mul(dsSIMD2d_set1(factors2.x), otherFromToDiff),
		dsSIMD2d_mul(fromToDiff, dsSIMD2d_set1(factors2.y)));
#endif

	intersect.simd = dsSIMD2d_div(intersect.simd, dsSIMD2d_set1(divisor));

	// Find T based on the largest difference to avoid issues with axis-aligned lines.
	dsVector2d offsetAbs;
	offsetAbs.simd = dsSIMD2d_abs(offset.simd);
	double t;
	if (offsetAbs.x > offsetAbs.y)
		t = (intersect.x - from->x)/offset.x;
	else
		t = (intersect.y - from->y)/offset.y;

	dsVector2d otherOffset;
	dsVector2_sub(otherOffset, *otherTo, *otherFrom);
	offsetAbs.simd = dsSIMD2d_abs(otherOffset.simd);
	double otherT;
	if (offsetAbs.x > offsetAbs.y)
		otherT = (intersect.x - otherFrom->x)/otherOffset.x;
	else
		otherT = (intersect.y - otherFrom->y)/otherOffset.y;

	// Don't count the endpoints of the first line, but count the endpoints of the second line as
	// an intersection.
	return t > epsilon && t < 1.0 - epsilon && otherT > -epsilon && otherT < 1.0 + epsilon;
}
DS_SIMD_END()
#endif

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
	double edgeCross = referenceDir->x*edgeDir.y - edgeDir.x*referenceDir->y;
	bool edgeCCW = edgeCross > 0;
	bool edgeCollinear = dsEpsilonEqualsZerod(edgeCross, polygon->intersectEpsilon);
	if (edgeCollinear || edgeCCW == ccw)
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
	if (dsVector2d_epsilonEqual(&fromVert->point, &toVert->point, polygon->equalEpsilon))
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

	EdgeIntersectInfo info = {fromVert->point, toVert->point, polygon->intersectEpsilon,
		fromVertIdx, toVertIdx, false};
	dsBVHVisitFunction visitorFunc;
#if DS_HAS_SIMD
	if (dsHostSIMDFeatures & dsSIMDFeatures_Double2)
		visitorFunc = &testEdgeIntersectSIMD;
	else
#endif
		visitorFunc = testEdgeIntersect;
	dsBVH_intersectBounds(polygon->edgeBVH, &edgeBounds, visitorFunc, &info);
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
