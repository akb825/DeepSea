/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorScratchData.h>

#include "VectorScratchDataImpl.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <limits.h>
#include <string.h>

#define MAX_VERTEX_INDEX (USHRT_MAX - 1)

dsVectorScratchData* dsVectorScratchData_create(dsAllocator* allocator)
{
	if (!allocator || !allocator->freeFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	dsVectorScratchData* data = DS_ALLOCATE_OBJECT(allocator, dsVectorScratchData);
	if (!data)
		return NULL;

	memset(data, 0, sizeof(*data));
	data->allocator = NULL;
	return data;
}

void dsVectorScratchData_destroy(dsVectorScratchData* data)
{
	if (!data)
		return;

	DS_ASSERT(data->allocator);
	if (data->points)
		dsAllocator_free(data->allocator, data->points);
	if (data->shapeVertices)
		dsAllocator_free(data->allocator, data->shapeVertices);
	if (data->imageVertices)
		dsAllocator_free(data->allocator, data->imageVertices);
	if (data->textVertices)
		dsAllocator_free(data->allocator, data->textVertices);
	if (data->indices)
		dsAllocator_free(data->allocator, data->indices);
	if (data->vectorInfos)
		dsAllocator_free(data->allocator, data->vectorInfos);
	if (data->pieces)
		dsAllocator_free(data->allocator, data->pieces);
	dsAllocator_free(data->allocator, data);
}

void dsVectorScratchData_reset(dsVectorScratchData* data)
{
	data->pointCount = 0;
	data->inPath = false;
	data->lastStart = 0;
	data->shapeVertexCount = 0;
	data->imageVertexCount = 0;
	data->textVertexCount = 0;
	data->indexCount = 0;
	data->vectorInfoCount = 0;
	data->pieceCount = 0;
}

bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type)
{
	uint32_t index = data->pointCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->points,
		&data->pointCount, &data->maxPoints, sizeof(PointInfo), 1))
	{
		return false;
	}

	data->points[index].point = *point;
	data->points[index].type = type;
	return true;
}

ShapeVertex* dsVectorScratchData_addShapeVertex(dsVectorScratchData* data)
{
	uint32_t index = data->shapeVertexCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->shapeVertices,
		&data->shapeVertexCount, &data->maxShapeVertices, sizeof(ShapeVertex), 1))
	{
		return NULL;
	}

	return data->shapeVertices + index;
}

ImageVertex* dsVectorScratchData_addImageVertex(dsVectorScratchData* data)
{
	uint32_t index = data->imageVertexCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->imageVertices,
		&data->imageVertexCount, &data->maxImageVertices, sizeof(ImageVertex), 1))
	{
		return NULL;
	}

	return data->imageVertices + index;
}

TextVertex* dsVectorScratchData_addTextVertex(dsVectorScratchData* data)
{
	uint32_t index = data->textVertexCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->textVertices,
		&data->textVertexCount, &data->maxTextVertices, sizeof(TextVertex), 1))
	{
		return NULL;
	}

	return data->textVertices + index;
}

VectorInfo* dsVectorScratchData_addVectorInfo(dsVectorScratchData* data)
{
	uint32_t index = data->vectorInfoCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->vectorInfos,
		&data->vectorInfoCount, &data->maxVectorInfos, sizeof(VectorInfo), 1))
	{
		return NULL;
	}

	return data->vectorInfos + index;
}

bool dsVectorScratchData_addIndex(dsVectorScratchData* data, uint32_t* vertex)
{
	DS_ASSERT(data->pieceCount > 0);
	TempPiece* piece = data->pieces + data->pieceCount - 1;
	if (*vertex < (uint32_t)piece->range.vertexOffset)
	{
		switch (piece->type)
		{
			case ShaderType_Shape:
			{
				uint32_t newVertIndex = data->shapeVertexCount;
				ShapeVertex* newVert = dsVectorScratchData_addShapeVertex(data);
				if (!newVert)
					return false;
				*newVert = data->shapeVertices[*vertex];
				*vertex = newVertIndex;
				break;
			}
			case ShaderType_Image:
			{
				uint32_t newVertIndex = data->imageVertexCount;
				ImageVertex* newVert = dsVectorScratchData_addImageVertex(data);
				if (!newVert)
					return false;
				*newVert = data->imageVertices[*vertex];
				*vertex = newVertIndex;
				break;
			}
			case ShaderType_Text:
			{
				uint32_t newVertIndex = data->textVertexCount;
				TextVertex* newVert = dsVectorScratchData_addTextVertex(data);
				if (!newVert)
					return false;
				*newVert = data->textVertices[*vertex];
				*vertex = newVertIndex;
				break;
			}
			default:
				DS_ASSERT(false);
				return false;
		}
	}

	uint32_t indexVal = *vertex - piece->range.vertexOffset;
	if (indexVal > MAX_VERTEX_INDEX)
	{
		TempPiece* oldPiece = piece;
		piece = data->pieces + data->pieceCount;
		if (!dsResizeableArray_add(data->allocator, (void**)&data->pieces,
			&data->pieceCount, &data->maxPieces, sizeof(TempPiece), 1))
		{
			return false;
		}

		*piece = *oldPiece;
		piece->range.indexCount = 0;
		piece->range.firstIndex = *vertex;
		indexVal = 0;

		// Add any remaining vertices.
		uint32_t remainingIndices = oldPiece->range.indexCount % 3;
		for (uint32_t i = 0; i < remainingIndices; ++i)
		{
			uint32_t vertexVal = data->indices[oldPiece->range.indexCount - remainingIndices + i] +
				oldPiece->range.vertexOffset;
			if (!dsVectorScratchData_addIndex(data, &vertexVal))
				return false;
		}
		oldPiece->range.indexCount -= remainingIndices;
	}

	uint32_t index = data->indexCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->indices,
		&data->indexCount, &data->maxIndices, sizeof(uint16_t), 1))
	{
		return false;
	}

	data->indices[index] = (uint16_t)index;
	++piece->range.indexCount;
	return true;
}

bool dsVectorScratchData_addPiece(dsVectorScratchData* data, ShaderType type, dsTexture* texture)
{
	if (data->pieceCount > 0 && data->pieces[data->pieceCount - 1].type == type &&
		(type == ShaderType_Shape || data->pieces[data->pieceCount - 1].texture == texture))
	{
		return true;
	}

	uint32_t index = data->pieceCount;
	if (!dsResizeableArray_add(data->allocator, (void**)&data->pieces,
		&data->pieceCount, &data->maxPieces, sizeof(TempPiece), 1))
	{
		return false;
	}

	data->pieces[index].type = type;
	data->pieces[index].range.indexCount = 0;
	data->pieces[index].range.instanceCount = 1;
	data->pieces[index].range.firstIndex = data->indexCount;
	data->pieces[index].range.firstInstance = 0;
	switch (type)
	{
		case ShaderType_Shape:
			data->pieces[index].range.vertexOffset = data->shapeVertexCount;
			break;
		case ShaderType_Image:
			data->pieces[index].range.vertexOffset = data->imageVertexCount;
			break;
		case ShaderType_Text:
			data->pieces[index].range.vertexOffset = data->textVertexCount;
			break;
		default:
			DS_ASSERT(false);
			break;
	}

	return true;
}
