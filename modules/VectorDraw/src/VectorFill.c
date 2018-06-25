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
#include <DeepSea/Geometry/ComplexPolygon.h>
#include <DeepSea/Geometry/SimplePolygon.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <float.h>

static bool getShapePosition(dsVector2d* outPosition, void* userData, const void* points,
	uint32_t index)
{
	DS_UNUSED(userData);
	const ShapeVertex* shapeVertices = (const ShapeVertex*)points;
	outPosition->x = shapeVertices[index].position.x;
	outPosition->y = shapeVertices[index].position.y;
	return true;
}

bool dsVectorFill_add(dsVectorScratchData* scratchData, const dsVectorMaterialSet* sharedMaterials,
	const dsVectorMaterialSet* localMaterials, const dsVectorCommandFillPath* fill)
{
	if (scratchData->pointCount < 3)
		return true;

	uint32_t material = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
		fill->material);
	if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
	{
		material = dsVectorMaterialSet_findMaterialIndex(localMaterials, fill->material);
		if (material == DS_VECTOR_MATERIAL_NOT_FOUND)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", fill->material);
			return false;
		}
		material += DS_VECTOR_LOCAL_MATERIAL_OFFSET;
	}

	uint32_t infoIndex = scratchData->vectorInfoCount;
	ShapeInfo* curInfo = dsVectorScratchData_addShapePiece(scratchData,
		&scratchData->pathTransform, fill->opacity);
	if (!curInfo)
		return false;

	uint32_t indexOffset = scratchData->shapeVertexCount;
	uint32_t firstPoint = 0;
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

		if (scratchData->pathSimple && (!joinStart || !end))
		{
			ShapeVertex* curVertex = dsVectorScratchData_addShapeVertex(scratchData);
			if (!curVertex)
				return false;

			curVertex->position.x = scratchData->points[i].point.x;
			curVertex->position.y = scratchData->points[i].point.y;
			curVertex->position.z = 0.0f;
			curVertex->position.w = 0.0f;
			curVertex->materialIndex = (uint16_t)material;
			curVertex->shapeIndex = (uint16_t)infoIndex;
		}

		if (end)
		{
			uint32_t pointCount = i + 1 - firstPoint;
			if (joinStart)
				--pointCount;

			if (scratchData->pathSimple)
			{
				uint32_t indexCount;
				// Use clockwise winding due to origin in the upper-left.
				const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount,
					scratchData->polygon, scratchData->shapeVertices + indexOffset, pointCount,
					&getShapePosition, dsTriangulateWinding_CW);
				if (!indices)
					return false;

				for (uint32_t j = 0; j < indexCount; ++j)
				{
					uint32_t index = indices[j] + indexOffset;
					if (!dsVectorScratchData_addIndex(scratchData, &index))
						return false;
				}

				indexOffset = scratchData->shapeVertexCount;
			}
			else
			{
				if (!dsVectorScratchData_addLoop(scratchData, firstPoint, pointCount))
					return false;
			}

			firstPoint = i + 1;
		}
	}

	// Need to simplify the loops to triangulate if the path isn't already simple.
	if (!scratchData->pathSimple)
	{
		if (!dsComplexPolygon_simplify(scratchData->simplifier, scratchData->loops,
			scratchData->loopCount, &dsVectorScratchData_loopPoint, fill->fillRule))
		{
			return false;
		}

		uint32_t loopCount = dsComplexPolygon_getLoopCount(scratchData->simplifier);
		for (uint32_t i = 0; i < loopCount; ++i)
		{
			const dsComplexPolygonLoop* loop = dsComplexPolygon_getLoop(scratchData->simplifier, i);
			const dsVector2f* loopPoints = (const dsVector2f*)loop->points;
			for (uint32_t j = 0; j < loop->pointCount; ++j)
			{
				ShapeVertex* curVertex = dsVectorScratchData_addShapeVertex(scratchData);
				if (!curVertex)
					return false;

				curVertex->position.x = loopPoints[j].x;
				curVertex->position.y = loopPoints[j].y;
				curVertex->position.z = 0.0f;
				curVertex->position.w = 0.0f;
				curVertex->materialIndex = (uint16_t)material;
				curVertex->shapeIndex = (uint16_t)infoIndex;
			}

			uint32_t indexCount;
			// Use clockwise winding due to origin in the upper-left.
			const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount,
				scratchData->polygon, scratchData->shapeVertices + indexOffset, loop->pointCount,
				&getShapePosition, dsTriangulateWinding_CW);
			if (!indices)
				return false;

			for (uint32_t j = 0; j < indexCount; ++j)
			{
				uint32_t index = indices[j] + indexOffset;
				if (!dsVectorScratchData_addIndex(scratchData, &index))
					return false;
			}

			indexOffset = scratchData->shapeVertexCount;
		}
	}

	return true;
}