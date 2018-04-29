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

#include "VectorStroke.h"

#include "VectorScratchDataImpl.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector4.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>

static bool findLineDir(dsVector2f* outDirection, const dsVectorScratchData* scratchData,
	uint32_t curIndex)
{
	for (uint32_t j = curIndex + 1; j < scratchData->pointCount &&
		!(scratchData->points[j].type & PointType_End); ++j)
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
	DS_ASSERT(materialIndex <= USHRT_MAX);
	DS_ASSERT(shapeIndex <= USHRT_MAX);

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
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
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
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
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
			// Target a max arc-length of one pixel.
			float pixelTheta = pixelSize/(lineWidth*0.5f);
			unsigned int pointCount = (unsigned int)(M_PI/pixelTheta);
			float incr = (float)M_PI/(float)(pointCount + 1);
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
				curVertex->position.z = distance;
				curVertex->position.w = totalDistance;
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
			curVertex->position.z = distance;
			curVertex->position.w = totalDistance;
			curVertex->materialIndex = (uint16_t)materialIndex;
			curVertex->shapeIndex = (uint16_t)shapeIndex;

			uint32_t secondSquareVert = scratchData->shapeVertexCount;
			curVertex = dsVectorScratchData_addShapeVertex(scratchData);
			if (!curVertex)
				return false;

			dsVector2_sub(curVertex->position, *position, offset);
			dsVector2_add(curVertex->position, curVertex->position, squareOffset);
			dsAlignedBox2_addPoint(*bounds, curVertex->position);
			curVertex->position.z = distance;
			curVertex->position.w = totalDistance;
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
	DS_ASSERT(materialIndex <= USHRT_MAX);
	DS_ASSERT(shapeIndex <= USHRT_MAX);

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
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t newSecondVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2_sub(curVertex->position, *position, offset);
	dsAlignedBox2_addPoint(*bounds, curVertex->position);
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

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

	*firstVertex = newFirstVertex;
	*secondVertex = newSecondVertex;
	return true;
}

static bool addJoin(dsVectorScratchData* scratchData, const dsVector2f* position,
	const dsVector2f* fromDirection, const dsVector2f* toDirection, float lineWidth,
	uint32_t* firstVertex, uint32_t* secondVertex, uint32_t materialIndex, uint32_t shapeIndex,
	dsLineJoin joinType, float cosMiterThetaLimit, float distance, float totalDistance,
	float pixelSize, dsAlignedBox2f* bounds)
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

	float halfWidth = lineWidth*0.5f;
	dsVector2f fromOffset = {{fromDirection->y, -fromDirection->x}};
	dsVector2_scale(fromOffset, fromOffset, halfWidth);

	// Add the end points and triangles for the "from" line.
	uint32_t fromFirstVertex = scratchData->shapeVertexCount;
	ShapeVertex* curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2f fromFirstPos;
	dsVector2_add(fromFirstPos, *position, fromOffset);
	dsAlignedBox2_addPoint(*bounds, fromFirstPos);
	curVertex->position.x = fromFirstPos.x;
	curVertex->position.y = fromFirstPos.y;
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t fromSecondVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2f fromSecondPos;
	dsVector2_sub(fromSecondPos, *position, fromOffset);
	dsAlignedBox2_addPoint(*bounds, fromSecondPos);
	curVertex->position.x = fromSecondPos.x;
	curVertex->position.y = fromSecondPos.y;
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

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

	dsVector2f toOffset = {{toDirection->y, -toDirection->x}};
	dsVector2_scale(toOffset, toOffset, halfWidth);

	// Add the start points for the "to" line.
	// NOTE: The inner portion of the join will overlap. Attempting to find the intersection point
	// could break down if the intersection point is beyond the line boundaries. Shading should be
	// cnosistent in most cases, making the overlap not be an issue.
	uint32_t toFirstVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2f toFirstPos;
	dsVector2_add(toFirstPos, *position, toOffset);
	dsAlignedBox2_addPoint(*bounds, toFirstPos);
	curVertex->position.x = toFirstPos.x;
	curVertex->position.y = toFirstPos.y;
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t toSecondVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	dsVector2f toSecondPos;
	dsVector2_sub(toSecondPos, *position, toOffset);
	dsAlignedBox2_addPoint(*bounds, toSecondPos);
	curVertex->position.x = toSecondPos.x;
	curVertex->position.y = toSecondPos.y;
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	uint32_t centerVertex = scratchData->shapeVertexCount;
	curVertex = dsVectorScratchData_addShapeVertex(scratchData);
	if (!curVertex)
		return false;

	curVertex->position.x = position->x;
	curVertex->position.y = position->y;
	curVertex->position.z = distance;
	curVertex->position.w = totalDistance;
	curVertex->materialIndex = (uint16_t)materialIndex;
	curVertex->shapeIndex = (uint16_t)shapeIndex;

	switch (joinType)
	{
		case dsLineJoin_Miter:
		{
			/*
			 * Miter forms a right angle triangle with:
			 * - The outer point of the line end/start
			 * - The join location of the centerline.
			 * - The intersection point of the miter.
			 * The last two points form a right angle. The angle at the miter point is half of the
			 * angle between the two lines. Therefore, with right angle triangle:
			 * tan(miterTheta) = opposite/adjacent = halfWidth/extendLength
			 * extendLength = halfWidth/tan(miterTheta)
			 */
			float extendLength = 0.0f;
			bool miter = cosTheta >= cosMiterThetaLimit;
			if (miter)
			{
				float theta = acosf(cosTheta);
				// We have the outside angle, we need the inside angle.
				theta = (float)M_PI - theta;
				float miterTheta = theta/2.0f;
				extendLength = halfWidth/tanf(miterTheta);
			}

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
					curVertex->position.z = distance;
					curVertex->position.w = totalDistance;
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
					curVertex->position.z = distance;
					curVertex->position.w = totalDistance;
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
			float theta = acosf(cosTheta);

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
				thetaOffset = (float)M_PI;

			// Target a max arc-length of one pixel.
			float pixelTheta = pixelSize/(lineWidth*0.5f);
			unsigned int pointCount = (unsigned int)(theta/pixelTheta);
			float incr = (float)theta/(float)(pointCount + 1);

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
				curVertex->position.z = distance;
				curVertex->position.w = totalDistance;
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
	if (scratchData->pointCount == 0)
		return true;

	uint32_t material = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
		stroke->material);
	if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
	{
		material = dsVectorMaterialSet_findMaterialIndex(localMaterials, stroke->material);
		if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", stroke->material);
			return false;
		}
		material += DS_VECTOR_LOCAL_MATERIAL_OFFSET;
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
		float theta = (float)M_PI - asinf(1.0f/stroke->miterLimit)*2.0f;
		cosMiterThetaLimit = cosf(theta);
	}

	// Expand by a minimum of a pixel, using alpha for sub-pixel sizes.
	float expandSize = dsMax(stroke->width, pixelSize);
	float sizeAlpha = stroke->width/expandSize;

	uint32_t infoIndex = scratchData->vectorInfoCount;
	ShapeInfo* curInfo = dsVectorScratchData_addShapePiece(scratchData,
		&scratchData->pathTransform, stroke->opacity*sizeAlpha);
	if (!curInfo)
		return false;
	curInfo->dashArray = stroke->dashArray;

	float subpathDistance = 0.0f, distance = 0.0f;
	uint32_t firstPoint = 0;
	bool joinStart = false;
	dsVector2f lastDir = {{1.0f, 0.0f}};
	dsVector2f firstDir = lastDir;
	uint32_t firstVertex = 0, secondVertex = 1;
	for (uint32_t i = 0; i < scratchData->pointCount; ++i)
	{
		bool end = i == scratchData->pointCount - 1 || scratchData->points[i].type & PointType_End;
		if (i == firstPoint)
		{
			// Single point.
			if (end)
			{
				firstPoint = i + 1;
				break;
			}

			subpathDistance = 0.0f;
			distance = 0.0f;
			for (uint32_t j = i + 1; j < scratchData->pointCount; ++j)
			{
				subpathDistance += dsVector2f_dist(&scratchData->points[j - 1].point,
					&scratchData->points[j].point);
				if (scratchData->points[j].type & PointType_End)
					break;
			}

			// Line cap. If the start joins with the end, use a butt style cap for the later join.
			findLineDir(&lastDir, scratchData, i);
			joinStart = (scratchData->points[i].type & PointType_JoinStart) != 0;
			dsLineCap capType = stroke->capType;
			if (joinStart)
			{
				capType = dsLineCap_Butt;
				firstDir = lastDir;
			}
			if (!addCap(scratchData, &scratchData->points[i].point, &lastDir, expandSize,
				&firstVertex, &secondVertex, material, infoIndex, capType, distance,
				subpathDistance, pixelSize, true, &curInfo->bounds))
			{
				return false;
			}
			continue;
		}

		distance += dsVector2f_dist(&scratchData->points[i - 1].point,
			&scratchData->points[i].point);

		dsVector2f nextDir;
		findLineDir(&nextDir, scratchData, i);
		if (scratchData->points[i].type & PointType_Corner)
		{
			if (!addJoin(scratchData, &scratchData->points[i].point, &lastDir, &nextDir, expandSize,
				&firstVertex, &secondVertex, material, infoIndex, stroke->joinType,
				cosMiterThetaLimit, distance, subpathDistance, pixelSize, &curInfo->bounds))
			{
				return false;
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
				return false;
			}
		}

		if (end)
		{
			// Either cap or join based on the first point.
			if (joinStart)
			{
				if (scratchData->points[firstPoint].type & PointType_Corner)
				{
					if (!addJoin(scratchData, &scratchData->points[firstPoint].point, &nextDir,
						&firstDir, expandSize, &firstVertex, &secondVertex, material, infoIndex,
						stroke->joinType, cosMiterThetaLimit, distance, subpathDistance, pixelSize,
						&curInfo->bounds))
					{
						return false;
					}
				}
				else
				{
					if (!addSimpleJoin(scratchData, &scratchData->points[firstPoint].point,
						&firstDir, expandSize, &firstVertex, &secondVertex, material, infoIndex,
						distance, subpathDistance, &curInfo->bounds))
					{
						return false;
					}
				}
			}
			else
			{
				if (!addCap(scratchData, &scratchData->points[i].point, &nextDir, expandSize,
					&firstVertex, &secondVertex, material, infoIndex, stroke->capType, distance,
					subpathDistance, pixelSize, false, &curInfo->bounds))
				{
					return false;
				}
			}
			firstPoint = i + 1;
		}

		lastDir = nextDir;
	}

	return true;
}
