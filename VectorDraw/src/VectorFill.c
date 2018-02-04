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

#include "VectorFill.h"

#include "VectorScratchDataImpl.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <float.h>

static bool isLeft(const dsVector2f* point, const dsVector2f* reference)
{
	return point->x < reference->x || (point->x == reference->x && point->y < reference->y);
}

static uint32_t findOtherPoint(const dsVectorScratchData* scratchData, const uint32_t* sortedVert,
	bool othersLeft)
{
	// Find the first point that's to the left/right of the vertex that doesn't intersect any edges.
	if (othersLeft)
	{
		const uint32_t* end = scratchData->sortedPolygonVerts + scratchData->polygonVertCount;
		for (const uint32_t* otherVert = sortedVert + 1; otherVert < end; ++otherVert)
		{
			if (dsVectorScratchData_canConnectPolygonEdge(scratchData, *sortedVert, *otherVert))
				return *otherVert;
		}
	}
	else
	{
		for (const uint32_t* otherVert = sortedVert; otherVert-- > scratchData->sortedPolygonVerts;)
		{
			if (dsVectorScratchData_canConnectPolygonEdge(scratchData, *sortedVert, *otherVert))
				return *otherVert;
		}
	}

	return NOT_FOUND;
}

static bool isPolygonCCW(dsVectorScratchData* data)
{
	if (data->polygonVertCount == 0)
		return true;

	// https://en.wikipedia.org/wiki/Shoelace_formula
	// Negative area is counter-clockwise, negative is clockwise.
	float doubleArea = (data->polygonVertices[data->polygonVertCount - 1].point.x +
			data->polygonVertices[0].point.x)*
		(data->polygonVertices[data->polygonVertCount - 1].point.y -
			data->polygonVertices[0].point.y);
	for (uint32_t i = 0; i < data->polygonVertCount - 1; ++i)
	{
		doubleArea += (data->polygonVertices[i].point.x + data->polygonVertices[i + 1].point.x)*
			(data->polygonVertices[i].point.y - data->polygonVertices[i + 1].point.y);
	}

	return doubleArea <= 0.0f;
}

static bool isTriangleCCW(const dsVector2f* p0, const dsVector2f* p1, const dsVector2f* p2)
{
	// Cross product of the triangle with Z = 0.
	float cross = (p1->x - p0->x)*(p2->y - p0->y) - (p2->x - p0->x)*(p1->y - p0->y);
	return cross >= 0.0f;
}

static bool triangulateLoop(dsVectorScratchData* scratchData, uint32_t startEdge, bool ccw)
{
	dsVectorScratchData_clearLoopVertices(scratchData);
	uint32_t nextEdge = startEdge;
	do
	{
		scratchData->polygonEdges[nextEdge].visited = true;
		uint32_t curEdge = nextEdge;
		nextEdge = scratchData->polygonEdges[nextEdge].nextEdge;
		if (!dsVectorScratchData_addLoopVertex(scratchData, curEdge))
			return false;
	} while (nextEdge != startEdge);

	if (scratchData->loopVertCount < 3)
		return true;

	// Monotone polygon triangulation: https://www.cs.ucsb.edu/~suri/cs235/Triangulation.pdf
	dsVectorScratchData_sortLoopVertices(scratchData);
	if (!dsVectorScratchData_pushVertex(scratchData, 0) ||
		!dsVectorScratchData_pushVertex(scratchData, 1))
	{
		return false;
	}

	uint32_t totalTriangles = 0;
	for (uint32_t i = 2; i < scratchData->loopVertCount; ++i)
	{
		DS_ASSERT(scratchData->vertStackCount > 0);
		uint32_t stackIndex = scratchData->vertStackCount - 1;
		uint32_t top = scratchData->vertexStack[stackIndex];
		uint32_t iVertIndex = scratchData->loopVertices[i].vertIndex;
		bool isPrev = scratchData->loopVertices[top].prevVert == iVertIndex;
		bool isNext = scratchData->loopVertices[top].nextVert == iVertIndex;
		// At most one should be set.
		DS_ASSERT(isPrev + isNext < 2);

		const dsVector2f* p0 =
			&scratchData->polygonVertices[scratchData->loopVertices[i].vertIndex].point;
		if (isPrev || isNext)
		{
			DS_ASSERT(scratchData->vertStackCount >= 0);
			uint32_t addedTriangles = 0;
			for (uint32_t j = stackIndex; j-- > 0; ++addedTriangles)
			{
				uint32_t p1Vert = scratchData->vertexStack[j];
				uint32_t p2Vert = scratchData->vertexStack[j + 1];
				const dsVector2f* p1 = &scratchData->polygonVertices[
					scratchData->loopVertices[p1Vert].vertIndex].point;
				const dsVector2f* p2 = &scratchData->polygonVertices[
					scratchData->loopVertices[p2Vert].vertIndex].point;
				// Add triangles along the chain so long as they are inside the polygon.
				bool expectedCCW = ccw;
				if (!isNext)
					expectedCCW = !expectedCCW;
				bool triangleCCW = isTriangleCCW(p0, p1, p2);
				if (triangleCCW != expectedCCW)
					break;

				// Use CW winding order due to upper-left being origin.
				if (triangleCCW)
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[i].vertIndex].indexValue) ||
					!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[p1Vert].vertIndex].indexValue) ||
					!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[p2Vert].vertIndex].indexValue))
				{
					return false;
				}
			}

			totalTriangles += addedTriangles;
			scratchData->vertStackCount -= addedTriangles;
			dsVectorScratchData_pushVertex(scratchData, i);
		}
		else if (scratchData->vertStackCount)
		{
			for (uint32_t j = 0; j < scratchData->vertStackCount - 1; ++j)
			{
				uint32_t p1Vert = scratchData->vertexStack[j];
				uint32_t p2Vert = scratchData->vertexStack[j + 1];
				const dsVector2f* p1 = &scratchData->polygonVertices[
					scratchData->loopVertices[p1Vert].vertIndex].point;
				const dsVector2f* p2 = &scratchData->polygonVertices[
					scratchData->loopVertices[p2Vert].vertIndex].point;

				// Use CW winding order due to upper-left being origin.
				if (isTriangleCCW(p0, p1, p2))
				{
					uint32_t temp = p1Vert;
					p1Vert = p2Vert;
					p2Vert = temp;
				}

				if (!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[i].vertIndex].indexValue) ||
					!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[p1Vert].vertIndex].indexValue) ||
					!dsVectorScratchData_addIndex(scratchData, &scratchData->polygonVertices[
						scratchData->loopVertices[p2Vert].vertIndex].indexValue))
				{
					return false;
				}
			}

			totalTriangles += scratchData->vertStackCount - 1;
			scratchData->vertStackCount = 0;
			dsVectorScratchData_pushVertex(scratchData, top);
			dsVectorScratchData_pushVertex(scratchData, i);
		}
	}

	if (totalTriangles != scratchData->loopVertCount - 2)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Polygon loop couldn't be triangulated.");
		return false;
	}

	return true;
}

static bool triangulate(dsVectorScratchData* scratchData)
{
	if (!dsVectorScratchData_addPolygonEdges(scratchData))
		return false;

	// Add separating edges for monotone polygons.
	bool ccw = isPolygonCCW(scratchData);
	for (uint32_t i = 0; i < scratchData->polygonVertCount; ++i)
	{
		const uint32_t* sortedVert = scratchData->sortedPolygonVerts + i;
		uint32_t prev = *sortedVert == 0 ? scratchData->polygonVertCount - 1 : *sortedVert - 1;
		uint32_t next = *sortedVert == scratchData->polygonVertCount - 1 ? 0 : *sortedVert + 1;

		bool prevLeft = isLeft(&scratchData->polygonVertices[prev].point,
			&scratchData->polygonVertices[*sortedVert].point);
		bool nextLeft = isLeft(&scratchData->polygonVertices[next].point,
			&scratchData->polygonVertices[*sortedVert].point);

		if (prevLeft != nextLeft)
			continue;

		bool triangleCCW = isTriangleCCW(&scratchData->polygonVertices[prev].point,
			&scratchData->polygonVertices[*sortedVert].point,
			&scratchData->polygonVertices[next].point);
		if (triangleCCW == ccw)
			continue;

		uint32_t otherPoint = findOtherPoint(scratchData, sortedVert, prevLeft);
		if (otherPoint == NOT_FOUND)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Invalid polygon goemetry.");
		}

		if (!dsVectorScratchData_addSeparatingPolygonEdge(scratchData, *sortedVert, otherPoint,
			ccw))
		{
			return false;
		}
	}

	// Need to reset the visited flags for the edges.
	for (uint32_t i = 0; i < scratchData->polygonEdgeCount; ++i)
		scratchData->polygonEdges[i].visited = false;

	// Triangulate each loop.
	for (uint32_t i = 0; i < scratchData->polygonEdgeCount; ++i)
	{
		if (scratchData->polygonEdges[i].visited)
			continue;

		if (!triangulateLoop(scratchData, i, ccw))
			return false;
	}
	return true;
}

bool dsVectorFill_add(dsVectorScratchData* scratchData, const dsVectorMaterialSet* materials,
	const dsVectorCommandFillPath* fill)
{
	if (scratchData->pointCount < 3)
		return true;

	uint32_t material = dsVectorMaterialSet_findMaterialIndex(materials,
		fill->material);
	if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", fill->material);
		return false;
	};

	uint32_t infoIndex = scratchData->vectorInfoCount;
	ShapeInfo* curInfo = dsVectorScratchData_addShapePiece(scratchData,
		&scratchData->pathTransform);
	if (!curInfo)
		return false;
	curInfo->opacity = fill->opacity;

	bool firstPoint = 0;
	bool joinStart = false;
	for (uint32_t i = 0; i < scratchData->pointCount; ++i)
	{
		bool end = i == scratchData->pointCount - 1 || scratchData->points[i].type & PointType_End;
		dsAlignedBox2_addPoint(curInfo->bounds, scratchData->points[i].point);
		if (i == firstPoint)
		{
			// Single point.
			if (scratchData->points[i].type & PointType_End)
			{
				firstPoint = i + 1;
				break;
			}

			joinStart = (scratchData->points[i].type & PointType_JoinStart) != 0;
		}

		if (!joinStart || !end)
		{
			if (!dsVectorScratchData_addPolygonVertex(scratchData, i, infoIndex, material))
				return false;
		}

		if (end)
		{
			if (!triangulate(scratchData))
				return false;

			firstPoint = i + 1;
			dsVectorScratchData_resetPolygon(scratchData);
		}
	}

	return true;
}
