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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Vector2.h>

#define NOT_FOUND ((uint32_t)-1)

typedef struct EdgeConnection
{
	uint32_t edge;
	uint32_t nextConnection;
} EdgeConnection;

typedef struct EdgeConnectionList
{
	EdgeConnection head;
	uint32_t tail;
} EdgeConnectionList;

typedef struct Vertex
{
	dsVector2d point;
	EdgeConnectionList prevEdges;
	EdgeConnectionList nextEdges;
} Vertex;

typedef struct Edge
{
	uint32_t prevVertex;
	uint32_t nextVertex;
	uint32_t prevEdge;
	uint32_t nextEdge;
	bool visited;
} Edge;

typedef struct dsBasePolygon
{
	dsAllocator* allocator;

	void* userData;
	double equalEpsilon;
	double intersectEpsilon;

	Vertex* vertices;
	uint32_t vertexCount;
	uint32_t maxVertices;

	Edge* edges;
	uint32_t edgeCount;
	uint32_t maxEdges;

	EdgeConnection* edgeConnections;
	uint32_t edgeConnectionCount;
	uint32_t maxEdgeConnections;

	uint32_t* sortedVerts;
	uint32_t maxSortedVerts;

	bool builtBVH;
	dsBVH* edgeBVH;

	uint32_t* indices;
	uint32_t indexCount;
	uint32_t maxIndices;
} dsBasePolygon;

// NOTE: First edge is exclusive of end points, second edge is inclusive. In other words, the first
// edge may touch other edges, but not vice-versa. The first edge is typically connecting two
// points, which may have multiple edges coming together. The second edge might intersect with the
// middle of the first edge unexpectedly, though.
bool dsPolygonEdgesIntersect(const dsVector2d* from, const dsVector2d* to,
	const dsVector2d* otherFrom, const dsVector2d* otherTo, double epsilon);

inline bool dsIsPolygonTriangleCCW(const dsVector2d* p0, const dsVector2d* p1,
	const dsVector2d* p2)
{
	// Cross product of the triangle with Z = 0.
	double cross = (p1->x - p0->x)*(p2->y - p0->y) - (p2->x - p0->x)*(p1->y - p0->y);
	return cross >= 0.0f;
}

inline int dsComparePolygonPoints(const dsVector2d* left, const dsVector2d* right)
{
	return dsCombineCmp(DS_CMP(left->x, right->x), DS_CMP(left->y, right->y));
}

inline void dsBasePolygon_reset(dsBasePolygon* polygon)
{
	polygon->vertexCount = 0;
	polygon->edgeCount = 0;
	polygon->edgeConnectionCount = 0;
	polygon->builtBVH = false;
	polygon->indexCount = 0;
}

double dsBasePolygon_edgeAngle(const dsBasePolygon* polygon, uint32_t edge,
	const dsVector2d* referenceDir, bool flip, bool ccw);
uint32_t dsBasePolygon_findEdge(const dsBasePolygon* polygon, const EdgeConnectionList* edgeList,
	const dsVector2d* referenceDir, bool flip, bool ccw);
bool dsBasePolygon_sortVertices(dsBasePolygon* polygon);
bool dsBasePolygon_buildEdgeBVH(dsBasePolygon* polygon);
bool dsBasePolygon_canConnectEdge(const dsBasePolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx, bool ccw);
bool dsBasePolygon_addSeparatingEdge(dsBasePolygon* polygon, uint32_t from, uint32_t to, bool ccw);
void dsBasePolygon_shutdown(dsBasePolygon* polygon);
