/*
 * Copyright 2018-2025 Aaron Barany
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

#include "VectorStroke.h"

#include "VectorHelpers.h"
#include "VectorScratchDataImpl.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox2.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector4.h>

#include <DeepSea/VectorDraw/VectorMaterialSet.h>

#include <limits.h>

static bool findLineDir(dsVector2f* outDirection, const dsVectorScratchData* scratchData,
	uint32_t curIndex)
{
	for (uint32_t j = curIndex + 1; j < scratchData->pointCount &&
		!(scratchData->points[j - 1].type & PointType_End); ++j)
	{
		if (scratchData->points[curIndex].point.x != scratchData->points[j].point.x ||
			scratchData->points[curIndex].point.y != scratchData->points[j].point.y)
		{
			dsVector2_sub(*outDirection, scratchData->points[j].point,
				scratchData->points[curIndex].point);
			dsVector2f_normalize(outDirection, outDirection);
			return true;
		}
	}

	return false;
}

static bool addCap(dsVectorScratchData* scratchData, const dsVector2f* position,
	const dsVector2f* direction, float lineWidth, uint32_t* firstVertex, uint32_t* secondVertex,
	uint32_t materialIndex, uint32_t shapeIndex, dsLineCap capType, float distance,
	float totalDistance, float pixelSize, bool start, dsAlignedBox2f* bounds)
{
	DS_ASSERT(materialIndex <= SHRT_MAX);
	DS_ASSERT(shapeIndex <= SHRT_MAX);

	float halfWidth = lineWidth*0.5f;
	dsVector2f offset = {{direction->y, -direction->x}};
	dsVector2_scale(offset, offset, halfWidth);

	// Add the end points.
	uint32_t newFirstVertex = scratchData->shapeVertexCount;
	ShapeVertex* curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2_add(curVertex->position, *position, offset);
	dsAlignedBox2_addPoint(*bounds, curVertex->position);
	curVertex->distance.x = distance;
	curVertex->distance.y = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t newSecondVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	if (!start)
	{
		if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &newFirstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;

		if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &newFirstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;
	}

	*firstVertex = newFirstVertex;
	*secondVertex = newSecondVertex;

	dsVector2_sub(curVertex->position, *position, offset);
	dsAlignedBox2_addPoint(*bounds, curVertex->position);
	curVertex->distance.x = distance;
	curVertex->distance.y = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	// Add the cap.
	switch (capType)
	{
		case dsLineCap_Round:
		{
			dsMatrix33f matrix =
			{{
				{offset.x, offset.y, 0.0f},
				{-offset.y, offset.x, 0.0f},
				{position->x, position->y, 1.0f}
			}};
			float pixelTheta = dsVectorPixelTheta(pixelSize, lineWidth);
			unsigned int pointCount = (unsigned int)(M_PI/pixelTheta);
			pointCount = dsMax(pointCount, 2U);
			float incr = M_PIf/(float)pointCount;
			if (start)
				incr = -incr;

			uint32_t firstPointVert = scratchData->shapeVertexCount;
			for (unsigned int i = 1; i < pointCount; ++i)
			{
				float theta = (float)i*incr;
				dsVector3f basePos = {{cosf(theta), sinf(theta), 1.0f}};
				dsVector3f pos;
				dsMatrix33_transform(pos, matrix, basePos);

				curVertex = dsVectorScratchData_addShapeVertex(scratchData);
				if (!curVertex)
					return false;

				dsAlignedBox2_addPoint(*bounds, pos);
				curVertex->position.x = pos.x;
				curVertex->position.y = pos.y;
				curVertex->distance.x = distance;
				curVertex->distance.y = totalDistance;
				curVertex->materialIndex = (uint16_t)materialIndex;
				curVertex->shapeIndex = (uint16_t)shapeIndex;
			}

			uint32_t pointVertCount = scratchData->shapeVertexCount - firstPointVert;
			DS_ASSERT((pointCount == 0 && pointVertCount == 0) ||
				(pointCount > 0 && pointVertCount == pointCount - 1));
			if (pointVertCount == 0)
				return true;

			if (start)
			{
				uint32_t curVertIndex = firstPointVert;
				if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;

				for (uint32_t i = 1; i < pointVertCount; ++i)
				{
					if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
						return false;
					uint32_t curVertIndex = firstPointVert + i - 1;
					if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
						return false;
					++curVertIndex;
					if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
						return false;
				}
			}
			else
			{
				uint32_t curVertIndex = firstPointVert;
				if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;

				for (uint32_t i = 1; i < pointVertCount; ++i)
				{
					if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
						return false;
					uint32_t curVertIndex = firstPointVert + i;
					if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
						return false;
					--curVertIndex;
					if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
						return false;
				}
			}

			return true;
		}
		case dsLineCap_Square:
		{
			dsVector2f squareOffset;
			if (start)
				dsVector2_scale(squareOffset, *direction, -halfWidth);
			else
				dsVector2_scale(squareOffset, *direction, halfWidth);
			uint32_t firstSquareVert = scratchData->shapeVertexCount;
			curVertex = dsVectorScratchData_addShapeVertex(scratchData);
			if (!curVertex)
				return false;

			dsVector2_add(curVertex->position, *position, offset);
			dsVector2_add(curVertex->position, curVertex->position, squareOffset);
			dsAlignedBox2_addPoint(*bounds, curVertex->position);
			curVertex->distance.x = distance;
			curVertex->distance.y = totalDistance;
			curVertex->materialIndex = (uint16_t)materialIndex;
			curVertex->shapeIndex = (uint16_t)shapeIndex;

			uint32_t secondSquareVert = scratchData->shapeVertexCount;
			curVertex = dsVectorScratchData_addShapeVertex(scratchData);
			if (!curVertex)
				return false;

			dsVector2_sub(curVertex->position, *position, offset);
			dsVector2_add(curVertex->position, curVertex->position, squareOffset);
			dsAlignedBox2_addPoint(*bounds, curVertex->position);
			curVertex->distance.x = distance;
			curVertex->distance.y = totalDistance;
			curVertex->materialIndex = (uint16_t)materialIndex;
			curVertex->shapeIndex = (uint16_t)shapeIndex;

			if (start)
			{
				if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &firstSquareVert))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;

				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &firstSquareVert))
					return false;
				return dsVectorScratchData_addIndex(scratchData, &secondSquareVert);
			}
			else
			{
				if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &firstSquareVert))
					return false;

				if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &secondSquareVert))
					return false;
				return dsVectorScratchData_addIndex(scratchData, &firstSquareVert);
			}
		}
		default:
			return true;
	}
}

static bool addSimpleJoin(dsVectorScratchData* scratchData, const dsVector2f* position,
	const dsVector2f* toDirection, float lineWidth, uint32_t* firstVertex, uint32_t* secondVertex,
	uint32_t materialIndex, uint32_t shapeIndex, float distance, float totalDistance,
	dsAlignedBox2f* bounds)
{
	DS_ASSERT(materialIndex <= SHRT_MAX);
	DS_ASSERT(shapeIndex <= SHRT_MAX);

	float halfWidth = lineWidth*0.5f;
	dsVector2f offset = {{toDirection->y, -toDirection->x}};
	dsVector2_scale(offset, offset, halfWidth);

	// Add the end points.
	uint32_t newFirstVertex = scratchData->shapeVertexCount;
	ShapeVertex* curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2_add(curVertex->position, *position, offset);
	dsAlignedBox2_addPoint(*bounds, curVertex->position);
	curVertex->distance.x = distance;
	curVertex->distance.y = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t newSecondVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2_sub(curVertex->position, *position, offset);
	dsAlignedBox2_addPoint(*bounds, curVertex->position);
	curVertex->distance.x = distance;
	curVertex->distance.y = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	if (*firstVertex != NOT_FOUND && *secondVertex != NOT_FOUND)
	{
		if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &newFirstVertex))
			return false;

		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &newSecondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &newFirstVertex))
			return false;
	}

	*firstVertex = newFirstVertex;
	*secondVertex = newSecondVertex;
	return true;
}

static bool addJoin(dsVectorScratchData* scratchData, const dsVector2f* position,
	const dsVector2f* fromDirection, const dsVector2f* toDirection, float lineWidth,
	uint32_t* firstVertex, uint32_t* secondVertex, uint32_t materialIndex, uint32_t shapeIndex,
	dsLineJoin joinType, float cosMiterThetaLimit, float segmentDistance, float distance,
	float totalDistance, float pixelSize, dsAlignedBox2f* bounds, bool end)
{
	float cosTheta = dsVector2_dot(*fromDirection, *toDirection);
	// Check for a straight line.
	const float epsilon = 1e-3f;
	if (cosTheta >= 1.0f - epsilon)
	{
		return addSimpleJoin(scratchData, position, toDirection, lineWidth, firstVertex,
			secondVertex, materialIndex, shapeIndex, distance, totalDistance, bounds);
	}

	dsVector2f fromDirRight = {{fromDirection->y, -fromDirection->x}};
	bool right = dsVector2_dot(fromDirRight, *toDirection) > 0.0f;
	float theta = acosf(cosTheta);

	float halfWidth = lineWidth*0.5f;
	dsVector2f fromOffset = {{fromDirection->y, -fromDirection->x}};
	dsVector2_scale(fromOffset, fromOffset, halfWidth);

	dsVector2f fromFirstPos;
	dsVector2_add(fromFirstPos, *position, fromOffset);
	dsAlignedBox2_addPoint(*bounds, fromFirstPos);

	dsVector2f fromSecondPos;
	dsVector2_sub(fromSecondPos, *position, fromOffset);
	dsAlignedBox2_addPoint(*bounds, fromSecondPos);

	dsVector2f toOffset = {{toDirection->y, -toDirection->x}};
	dsVector2_scale(toOffset, toOffset, halfWidth);

	dsVector2f toFirstPos;
	dsVector2_add(toFirstPos, *position, toOffset);
	dsAlignedBox2_addPoint(*bounds, toFirstPos);

	dsVector2f toSecondPos;
	dsVector2_sub(toSecondPos, *position, toOffset);
	dsAlignedBox2_addPoint(*bounds, toSecondPos);

	uint32_t centerVertex = scratchData->shapeVertexCount;
	ShapeVertex*  curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	curVertex->position.x = position->x;
	curVertex->position.y = position->y;
	curVertex->distance.x = distance;
	curVertex->distance.y = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	/*
	 * Find the point where the stroke joins on the inside of the join. This is the inverse
	 * of the miter join on the outside, nad will be re-used if miter join type is used.
	 * Miter forms a right angle triangle with:
	 * - The outer point of the line end/start
	 * - The join location of the centerline.
	 * - The intersection point of the miter.
	 * The last two points form a right angle. The angle at the miter point is half of the
	 * angle between the two lines. Therefore, with right angle triangle:
	 * tan(miterTheta) = opposite/adjacent = halfWidth/extendLength
	 * extendLength = halfWidth/tan(miterTheta)
	 */

	// We have the outside angle, we need the inside angle.
	float miterTheta = (M_PIf - theta)/2.0f;
	float extendLength = halfWidth/tanf(miterTheta);
	float innerExtendLength = dsMin(extendLength, segmentDistance);

	uint32_t fromFirstVertex, fromSecondVertex, toFirstVertex, toSecondVertex;
	if (right)
	{
		dsVector2f miterPos, miterOffset;
		dsVector2_scale(miterOffset, *fromDirection, innerExtendLength);
		dsVector2_sub(miterPos, fromFirstPos, miterOffset);

		// Same position for inner connected vertex, but different distance to ensure correct
		// dashing.
		fromFirstVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		dsAlignedBox2_addPoint(*bounds, miterPos);
		curVertex->position.x = miterPos.x;
		curVertex->position.y = miterPos.y;
		curVertex->distance.x = distance - innerExtendLength;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		toFirstVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		dsAlignedBox2_addPoint(*bounds, miterPos);
		curVertex->position.x = miterPos.x;
		curVertex->position.y = miterPos.y;
		curVertex->distance.x = distance + innerExtendLength;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		// Connect the "second" vertices to the center and inner vertices.
		fromSecondVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		curVertex->position.x = fromSecondPos.x;
		curVertex->position.y = fromSecondPos.y;
		curVertex->distance.x = distance;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		toSecondVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		curVertex->position.x = toSecondPos.x;
		curVertex->position.y = toSecondPos.y;
		curVertex->distance.x = distance;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		if (!end)
		{
			if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
				return false;

			if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
				return false;
		}
	}
	else
	{
		dsVector2f miterPos, miterOffset;
		dsVector2_scale(miterOffset, *fromDirection, innerExtendLength);
		dsVector2_sub(miterPos, fromSecondPos, miterOffset);

		// Same position for inner connected vertex, but different distance to ensure correct
		// dashing.
		fromSecondVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		dsAlignedBox2_addPoint(*bounds, miterPos);
		curVertex->position.x = miterPos.x;
		curVertex->position.y = miterPos.y;
		curVertex->distance.x = distance - innerExtendLength;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		toSecondVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		dsAlignedBox2_addPoint(*bounds, miterPos);
		curVertex->position.x = miterPos.x;
		curVertex->position.y = miterPos.y;
		curVertex->distance.x = distance + innerExtendLength;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		// Connect the "first" vertices to the center and inner vertices.
		fromFirstVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		curVertex->position.x = fromFirstPos.x;
		curVertex->position.y = fromFirstPos.y;
		curVertex->distance.x = distance;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		toFirstVertex = scratchData->shapeVertexCount;
		curVertex = dsVectorScratchData_addShapeVertex(scratchData);
		if (!curVertex)
			return false;

		curVertex->position.x = toFirstPos.x;
		curVertex->position.y = toFirstPos.y;
		curVertex->distance.x = distance;
		curVertex->distance.y = totalDistance;
		curVertex->materialIndex = (uint16_t)materialIndex;
		curVertex->shapeIndex = (uint16_t)shapeIndex;

		if (!end)
		{
			if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
				return false;

			if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
				return false;
			if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
				return false;
		}
	}

	if (*firstVertex != NOT_FOUND && *secondVertex != NOT_FOUND)
	{
		if (!dsVectorScratchData_addIndex(scratchData, firstVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
			return false;

		if (!dsVectorScratchData_addIndex(scratchData, secondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
			return false;
		if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
			return false;
	}

	if (end)
	{
		*firstVertex = toFirstVertex;
		*secondVertex = toSecondVertex;
		return true;
	}

	switch (joinType)
	{
		case dsLineJoin_Miter:
		{
			bool miter = cosTheta >= cosMiterThetaLimit;
			if (right)
			{
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
					return false;

				if (miter)
				{
					dsVector2f miterPos, miterOffset;
					dsVector2_scale(miterOffset, *fromDirection, extendLength);
					dsVector2_add(miterPos, fromSecondPos, miterOffset);

					uint32_t miterVertex = scratchData->shapeVertexCount;
					curVertex = dsVectorScratchData_addShapeVertex(scratchData);
					if (!curVertex)
						return false;

					dsAlignedBox2_addPoint(*bounds, miterPos);
					curVertex->position.x = miterPos.x;
					curVertex->position.y = miterPos.y;
					curVertex->distance.x = distance;
					curVertex->distance.y = totalDistance;
					curVertex->materialIndex = (uint16_t)materialIndex;
					curVertex->shapeIndex = (uint16_t)shapeIndex;

					if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &miterVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
						return false;
				}
			}
			else
			{
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
					return false;

				if (miter)
				{
					dsVector2f miterPos, miterOffset;
					dsVector2_scale(miterOffset, *fromDirection, extendLength);
					dsVector2_add(miterPos, fromFirstPos, miterOffset);

					uint32_t miterVertex = scratchData->shapeVertexCount;
					curVertex = dsVectorScratchData_addShapeVertex(scratchData);
					if (!curVertex)
						return false;

					dsAlignedBox2_addPoint(*bounds, miterPos);
					curVertex->position.x = miterPos.x;
					curVertex->position.y = miterPos.y;
					curVertex->distance.x = distance;
					curVertex->distance.y = totalDistance;
					curVertex->materialIndex = (uint16_t)materialIndex;
					curVertex->shapeIndex = (uint16_t)shapeIndex;

					if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &miterVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
						return false;
				}
			}
			break;
		}
		case dsLineJoin_Bevel:
			if (right)
			{
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
					return false;
			}
			else
			{
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
					return false;
			}
			break;
		case dsLineJoin_Round:
		{
			dsMatrix33f matrix;
			if (right)
			{
				matrix.columns[0].x = toOffset.x;
				matrix.columns[0].y = toOffset.y;
				matrix.columns[0].z = 0.0f;
				matrix.columns[1].x = -toOffset.y;
				matrix.columns[1].y = toOffset.x;
				matrix.columns[1].z = 0.0f;
				matrix.columns[2].x = position->x;
				matrix.columns[2].y = position->y;
				matrix.columns[2].z = 1.0f;
			}
			else
			{
				matrix.columns[0].x = fromOffset.x;
				matrix.columns[0].y = fromOffset.y;
				matrix.columns[0].z = 0.0f;
				matrix.columns[1].x = -fromOffset.y;
				matrix.columns[1].y = fromOffset.x;
				matrix.columns[1].z = 0.0f;
				matrix.columns[2].x = position->x;
				matrix.columns[2].y = position->y;
				matrix.columns[2].z = 1.0f;
			}

			float thetaOffset = 0.0f;
			if (right)
				thetaOffset = M_PIf;

			float pixelTheta = dsVectorPixelTheta(pixelSize, lineWidth);
			unsigned int pointCount = (unsigned int)(theta/pixelTheta);
			pointCount = dsMax(pointCount, 2U);
			float incr = (float)theta/(float)pointCount;

			uint32_t firstPointVert = scratchData->shapeVertexCount;
			for (unsigned int i = 1; i < pointCount; ++i)
			{
				float theta = thetaOffset + (float)i*incr;
				dsVector3f basePos = {{cosf(theta), sinf(theta), 1.0f}};
				dsVector3f pos;
				dsMatrix33_transform(pos, matrix, basePos);

				curVertex = dsVectorScratchData_addShapeVertex(scratchData);
				if (!curVertex)
					return false;

				dsAlignedBox2_addPoint(*bounds, pos);
				curVertex->position.x = pos.x;
				curVertex->position.y = pos.y;
				curVertex->distance.x = distance;
				curVertex->distance.y = totalDistance;
				curVertex->materialIndex = (uint16_t)materialIndex;
				curVertex->shapeIndex = (uint16_t)shapeIndex;
			}

			uint32_t pointVertCount = scratchData->shapeVertexCount - firstPointVert;
			DS_ASSERT((pointCount == 0 && pointVertCount == 0) ||
				(pointCount > 0 && pointVertCount == pointCount - 1));
			if (pointVertCount == 0)
			{
				if (right)
				{
					if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
						return false;
				}
				else
				{
					if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
						return false;
					if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
						return false;
				}
				break;
			}

			// Ending triangles.
			if (right)
			{
				uint32_t curVertIndex = firstPointVert;
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toSecondVertex))
					return false;

				curVertIndex = firstPointVert + pointVertCount - 1;
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromSecondVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
			}
			else
			{
				uint32_t curVertIndex = firstPointVert;
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &fromFirstVertex))
					return false;

				curVertIndex = firstPointVert + pointVertCount - 1;
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &toFirstVertex))
					return false;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
			}

			// Triangles in the middle..
			for (uint32_t i = 1; i < pointVertCount; ++i)
			{
				if (!dsVectorScratchData_addIndex(scratchData, &centerVertex))
					return false;
				uint32_t curVertIndex = firstPointVert + i;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
				--curVertIndex;
				if (!dsVectorScratchData_addIndex(scratchData, &curVertIndex))
					return false;
			}
			break;
		}
		default:
			DS_ASSERT(false);
			return false;
	}

	*firstVertex = toFirstVertex;
	*secondVertex = toSecondVertex;
	return true;
}

bool dsVectorStroke_add(dsVectorScratchData* scratchData,
	const dsVectorMaterialSet* sharedMaterials, const dsVectorMaterialSet* localMaterials,
	const dsVectorCommandStrokePath* stroke, float pixelSize)
{
	DS_PROFILE_FUNC_START();

	if (scratchData->pointCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	uint32_t material = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
		stroke->material);
	dsVectorMaterialType materialType = dsVectorMaterialType_Color;
	MaterialSource materialSource;
	if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
	{
		material = dsVectorMaterialSet_findMaterialIndex(localMaterials, stroke->material);
		if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", stroke->material);
			DS_PROFILE_FUNC_RETURN(false);
		}
		materialType = dsVectorMaterialSet_getMaterialType(localMaterials, stroke->material);
		materialSource = MaterialSource_Local;
	}
	else
	{
		materialType = dsVectorMaterialSet_getMaterialType(sharedMaterials, stroke->material);
		materialSource = MaterialSource_Shared;
	}

	float cosMiterThetaLimit = 0.0f;
	if (stroke->joinType == dsLineJoin_Miter)
	{
		/*
		 * strokeWidth/miterLength = sin(theta/2)
		 * theta = asin(strokeWidth/miterLength)*2
		 *
		 * maxMiterLength = miterLimit*strokeWidth
		 * thetaLimit = asin((strokeWidth/(miterLimit*strokeWidth))*2
		 * thetaLimit = asin(1.0/miterLimit)*2
		 *
		 * Theta is based on the inside angle. We use the outside angle with the dot product.
		 */
		DS_ASSERT(stroke->miterLimit >= 1.0f);
		float theta = M_PIf - asinf(1.0f/stroke->miterLimit)*2.0f;
		cosMiterThetaLimit = cosf(theta);
	}

	// Expand by a minimum of a pixel, using alpha for sub-pixel sizes.
	float expandSize = dsMax(stroke->width, pixelSize*0.5f);
	float sizeAlpha = stroke->width/expandSize;

	// As an optimization, use the fill shader when no dashing.
	float dashDistance = stroke->dashArray.x + stroke->dashArray.y + stroke->dashArray.z +
		stroke->dashArray.w;
	bool dashed = dashDistance > 0.0f;

	uint32_t infoIndex = scratchData->vectorInfoCount;
	ShapeInfo* curInfo = dsVectorScratchData_addShapePiece(scratchData,
		&scratchData->pathTransform, stroke->opacity*sizeAlpha, dashed, materialType,
		materialSource);
	if (!curInfo)
		DS_PROFILE_FUNC_RETURN(false);
	curInfo->dashArray = stroke->dashArray;

	float subpathDistance = 0.0f, distance = 0.0f;
	uint32_t firstPoint = 0;
	bool joinStart = false;
	dsVector2f lastDir = {{1.0f, 0.0f}};
	dsVector2f firstDir = lastDir;
	uint32_t firstVertex = NOT_FOUND, secondVertex = NOT_FOUND;
	for (uint32_t i = 0; i < scratchData->pointCount; ++i)
	{
		bool end = i == scratchData->pointCount - 1 || scratchData->points[i].type & PointType_End;
		if (i == firstPoint)
		{
			// Single point.
			if (end)
			{
				firstPoint = i + 1;
				continue;
			}

			subpathDistance = 0.0f;
			distance = 0.0f;
			uint32_t endIndex = i + 1;
			for (; endIndex < scratchData->pointCount; ++endIndex)
			{
				subpathDistance += dsVector2f_dist(&scratchData->points[endIndex - 1].point,
					&scratchData->points[endIndex].point);
				if (scratchData->points[endIndex].type & PointType_End)
					break;
			}

			// Line cap. If the start joins with the end, use a butt style cap for the later join.
			DS_VERIFY(findLineDir(&firstDir, scratchData, i));
			joinStart = (scratchData->points[i].type & PointType_JoinStart) != 0;
			if (joinStart)
			{
				float segmentDistance = dsVector2f_dist(&scratchData->points[endIndex - 1].point,
					&scratchData->points[i].point);

				DS_VERIFY(findLineDir(&lastDir, scratchData, endIndex - 1));
				dsVector2f nextDir = firstDir;
				if (scratchData->points[i].type & PointType_Corner)
				{
					if (!addJoin(scratchData, &scratchData->points[i].point, &lastDir, &nextDir,
						expandSize, &firstVertex, &secondVertex, material, infoIndex,
						stroke->joinType, cosMiterThetaLimit, segmentDistance, distance,
						subpathDistance, pixelSize, &curInfo->bounds, false))
					{
						DS_PROFILE_FUNC_RETURN(false);
					}
				}
				else
				{
					if (!addSimpleJoin(scratchData, &scratchData->points[i].point, &nextDir,
						expandSize, &firstVertex, &secondVertex, material, infoIndex, distance,
						subpathDistance, &curInfo->bounds))
					{
						DS_PROFILE_FUNC_RETURN(false);
					}
				}
			}
			else if (!addCap(scratchData, &scratchData->points[i].point, &firstDir, expandSize,
				&firstVertex, &secondVertex, material, infoIndex, stroke->capType, distance,
				subpathDistance, pixelSize, true, &curInfo->bounds))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
			lastDir = firstDir;
			continue;
		}

		float segmentDistance = dsVector2f_dist(&scratchData->points[i - 1].point,
			&scratchData->points[i].point);
		distance += segmentDistance;

		dsVector2f nextDir;
		if (end && joinStart)
			nextDir = firstDir;
		else if (end)
			nextDir = lastDir;
		else
			DS_VERIFY(findLineDir(&nextDir, scratchData, i));

		if (scratchData->points[i].type & PointType_Corner)
		{
			if (!addJoin(scratchData, &scratchData->points[i].point, &lastDir, &nextDir, expandSize,
				&firstVertex, &secondVertex, material, infoIndex, stroke->joinType,
				cosMiterThetaLimit, segmentDistance, distance, subpathDistance, pixelSize,
				&curInfo->bounds, end))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
		else
		{
			// If the angle difference is long enough the boundaries for the line won't be parallel,
			// but given that the tessellation minimizes curvature for each segment it shouldn't be
			// noticeable.
			if (!addSimpleJoin(scratchData, &scratchData->points[i].point, &nextDir, expandSize,
				&firstVertex, &secondVertex, material, infoIndex, distance, subpathDistance,
				&curInfo->bounds))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}

		if (end)
		{
			if (!joinStart)
			{
				if (!addCap(scratchData, &scratchData->points[i].point, &lastDir, expandSize,
					&firstVertex, &secondVertex, material, infoIndex, stroke->capType, distance,
					subpathDistance, pixelSize, false, &curInfo->bounds))
				{
					DS_PROFILE_FUNC_RETURN(false);
				}
			}

			firstPoint = i + 1;
			firstVertex = NOT_FOUND;
			secondVertex = NOT_FOUND;
			continue;
		}
		lastDir = nextDir;
	}

	DS_PROFILE_FUNC_RETURN(true);
}
