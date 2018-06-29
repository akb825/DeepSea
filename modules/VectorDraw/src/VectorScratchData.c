/*
 * Copyright 2017-2018 Aaron Barany
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
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/ComplexPolygon.h>
#include <DeepSea/Geometry/SimplePolygon.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Text/Font.h>
#include <limits.h>
#include <string.h>

#define MAX_VERTEX_INDEX (USHRT_MAX - 1)

DS_VECTORDRAW_EXPORT bool dsVectorImage_testing;

static VectorInfo* addVectorInfo(dsVectorScratchData* data)
{
	uint32_t index = data->vectorInfoCount;
	if (data->vectorInfoCount + 1 > data->maxVectorInfos)
	{
		uint32_t newMax = data->maxVectorInfos + INFOS_PER_TEXTURE;
		VectorInfo* infos = (VectorInfo*)dsAllocator_reallocWithFallback(data->allocator,
			data->vectorInfos, sizeof(VectorInfo*)*data->vectorInfoCount,
			sizeof(VectorInfo)*newMax);
		if (!infos)
			return NULL;

		memset(infos + data->vectorInfoCount, 0,
			sizeof(VectorInfo)*(newMax - data->vectorInfoCount));
		data->vectorInfos = infos;
		data->maxVectorInfos = newMax;
	}

	++data->vectorInfoCount;
	return data->vectorInfos + index;
}

static bool addPiece(dsVectorScratchData* data, ShaderType type, dsTexture* texture,
	uint32_t infoIndex)
{
	bool force = infoIndex % INFOS_PER_TEXTURE == 0;
	if (!force && data->pieceCount > 0 && data->pieces[data->pieceCount - 1].type == type &&
		(type == ShaderType_Shape || data->pieces[data->pieceCount - 1].texture == texture))
	{
		return true;
	}

	uint32_t index = data->pieceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->pieces, data->pieceCount, data->maxPieces,
		1))
	{
		return false;
	}

	data->pieces[index].type = type;
	data->pieces[index].infoTextureIndex = infoIndex/INFOS_PER_TEXTURE;
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
	data->pieces[index].texture = texture;

	return true;
}

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
	data->allocator = allocator;
	data->polygon = dsSimplePolygon_create(allocator, data, DS_POLYGON_EQUAL_EPSILON_FLOAT,
		DS_POLYGON_INTERSECT_EPSILON_FLOAT);
	if (!data->polygon)
	{
		DS_VERIFY(dsAllocator_free(allocator, data));
		return NULL;
	}
	data->simplifier = dsComplexPolygon_create(allocator, dsGeometryElement_Float, data);
	if (!data->simplifier)
	{
		dsSimplePolygon_destroy(data->polygon);
		DS_VERIFY(dsAllocator_free(allocator, data));
		return NULL;
	}
	return data;
}

void dsVectorScratchData_destroy(dsVectorScratchData* data)
{
	if (!data)
		return;

	DS_ASSERT(data->allocator);
	DS_VERIFY(dsAllocator_free(data->allocator, data->fileBuffer));
	DS_VERIFY(dsAllocator_free(data->allocator, data->tempCommands));
	DS_VERIFY(dsAllocator_free(data->allocator, data->points));
	DS_VERIFY(dsAllocator_free(data->allocator, data->shapeVertices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->imageVertices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->textVertices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->textTessVertices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->indices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->vectorInfos));
	DS_VERIFY(dsAllocator_free(data->allocator, data->pieces));
	DS_VERIFY(dsAllocator_free(data->allocator, data->loops));
	dsSimplePolygon_destroy(data->polygon);
	dsComplexPolygon_destroy(data->simplifier);
	DS_VERIFY(dsAllocator_free(data->allocator, data->combinedBuffer));
	DS_VERIFY(dsAllocator_free(data->allocator, data));
}

void dsVectorScratchData_reset(dsVectorScratchData* data)
{
	data->pointCount = 0;
	data->inPath = false;
	data->pathSimple = false;
	data->lastStart = 0;
	data->shapeVertexCount = 0;
	data->imageVertexCount = 0;
	data->textVertexCount = 0;
	data->textTessVertexCount = 0;
	data->indexCount = 0;
	data->vectorInfoCount = 0;
	data->pieceCount = 0;
	data->loopCount = 0;
}

void* dsVectorScratchData_readUntilEnd(size_t* outSize, dsVectorScratchData* data, dsStream* stream,
	dsAllocator* allocator)
{
	if (!dsStream_readUntilEndReuse(&data->fileBuffer, outSize, &data->fileBufferCapacity, stream,
		allocator))
	{
		return NULL;
	}

	return data->fileBuffer;
}

dsVectorCommand* dsVectorScratchData_createTempCommands(dsVectorScratchData* data,
	uint32_t commandCount)
{
	if (!data->tempCommands || data->maxTempCommands < commandCount)
	{
		dsAllocator_free(data->allocator, data->tempCommands);
		data->tempCommands = DS_ALLOCATE_OBJECT_ARRAY(data->allocator, dsVectorCommand,
			commandCount);
		data->maxTempCommands = commandCount;
	}

	return data->tempCommands;
}

bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type)
{
	const float epsilon = 1e-5f;
	if (data->pointCount > 0 && dsVector2f_epsilonEqual(&data->points[data->pointCount - 1].point,
		point, epsilon))
	{
		data->points[data->pointCount - 1].type |= type;
		return true;
	}

	uint32_t index = data->pointCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->points, data->pointCount, data->maxPoints,
		1))
	{
		return false;
	}

	data->points[index].point = *point;
	data->points[index].type = type;
	return true;
}

bool dsVectorScratchData_addLoop(dsVectorScratchData* data, uint32_t firstPoint, uint32_t count)
{
	uint32_t index = data->loopCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->loops, data->loopCount, data->maxLoops, 1))
		return false;

	data->loops[index].points = data->points + firstPoint;
	data->loops[index].pointCount = count;
	return true;
}

bool dsVectorScratchData_loopPoint(void* outPoint, const dsComplexPolygon* polygon,
	const void* loop, uint32_t index)
{
	DS_UNUSED(polygon);
	*(dsVector2f*)outPoint = ((const PointInfo*)loop)[index].point;
	return true;
}

ShapeVertex* dsVectorScratchData_addShapeVertex(dsVectorScratchData* data)
{
	uint32_t index = data->shapeVertexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->shapeVertices, data->shapeVertexCount,
		data->maxShapeVertices, 1))
	{
		return NULL;
	}

	return data->shapeVertices + index;
}

ImageVertex* dsVectorScratchData_addImageVertex(dsVectorScratchData* data)
{
	uint32_t index = data->imageVertexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->imageVertices, data->imageVertexCount,
		data->maxImageVertices, 1))
	{
		return NULL;
	}

	return data->imageVertices + index;
}

TextVertex* dsVectorScratchData_addTextVertex(dsVectorScratchData* data)
{
	uint32_t index = data->textVertexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textVertices, data->textVertexCount,
		data->maxTextVertices, 1))
	{
		return NULL;
	}

	return data->textVertices + index;
}

TextTessVertex* dsVectorScratchData_addTextTessVertex(dsVectorScratchData* data)
{
	uint32_t index = data->textTessVertexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textVertices, data->textVertexCount,
		data->maxTextTessVertices, 1))
	{
		return NULL;
	}

	return data->textTessVertices + index;
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
		if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->pieces, data->pieceCount,
			data->maxPieces, 1))
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
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->indices, data->indexCount, data->maxIndices,
		1))
	{
		return false;
	}

	data->indices[index] = (uint16_t)(*vertex - piece->range.vertexOffset);
	++piece->range.indexCount;
	return true;
}

ShapeInfo* dsVectorScratchData_addShapePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, float opacity)
{
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, ShaderType_Shape, NULL, infoIndex))
		return NULL;

	dsAlignedBox2f_makeInvalid(&info->shapeInfo.bounds);
	info->shapeInfo.transformCols[0].x = transform->columns[0].x;
	info->shapeInfo.transformCols[0].y = transform->columns[0].y;
	info->shapeInfo.transformCols[1].x = transform->columns[1].x;
	info->shapeInfo.transformCols[1].y = transform->columns[1].y;
	info->shapeInfo.transformCols[2].x = transform->columns[2].x;
	info->shapeInfo.transformCols[2].y = transform->columns[2].y;
	info->shapeInfo.opacity = opacity;
	return &info->shapeInfo;
}

ShapeInfo* dsVectorScratchData_addImagePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, dsTexture* texture, float opacity, const dsAlignedBox2f* bounds)
{
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, ShaderType_Image, texture, infoIndex))
		return NULL;

	info->shapeInfo.bounds = *bounds;
	info->shapeInfo.transformCols[0].x = transform->columns[0].x;
	info->shapeInfo.transformCols[0].y = transform->columns[0].y;
	info->shapeInfo.transformCols[1].x = transform->columns[1].x;
	info->shapeInfo.transformCols[1].y = transform->columns[1].y;
	info->shapeInfo.transformCols[2].x = transform->columns[2].x;
	info->shapeInfo.transformCols[2].y = transform->columns[2].y;
	info->shapeInfo.opacity = opacity;
	return &info->shapeInfo;
}

TextInfo* dsVectorScratchData_addTextPiece(dsVectorScratchData* data, const dsMatrix33f* transform,
	const dsFont* font, float opacity)
{
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, ShaderType_Text, dsFont_getTexture(font), infoIndex))
		return NULL;

	dsAlignedBox2f_makeInvalid(&info->textInfo.bounds);
	info->textInfo.transformCols[0].x = transform->columns[0].x;
	info->textInfo.transformCols[0].y = transform->columns[0].y;
	info->textInfo.transformCols[1].x = transform->columns[1].x;
	info->textInfo.transformCols[1].y = transform->columns[1].y;
	info->textInfo.transformCols[2].x = transform->columns[2].x;
	info->textInfo.transformCols[2].y = transform->columns[2].y;
	info->textInfo.opacity = opacity;
	return &info->textInfo;
}

dsGfxBuffer* dsVectorScratchData_createGfxBuffer(dsVectorScratchData* data,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	DS_ASSERT(data->textVertexCount == 0 || data->textTessVertexCount == 0);
	size_t totalSize = data->shapeVertexCount*sizeof(ShapeVertex) +
		data->imageVertexCount*sizeof(ImageVertex) + data->textVertexCount*sizeof(TextVertex) +
		data->textTessVertexCount*sizeof(TextTessVertex) + data->indexCount*sizeof(uint16_t);
	if (totalSize == 0)
		return NULL;

	if (!data->combinedBuffer || data->combinedBufferSize < totalSize)
	{
		DS_VERIFY(dsAllocator_free(data->allocator, data->combinedBuffer));
		data->combinedBuffer = DS_ALLOCATE_OBJECT_ARRAY(data->allocator, uint8_t, totalSize);
		if (!data->combinedBuffer)
			return NULL;

		data->combinedBufferSize = totalSize;
	}

	size_t offset = 0;
	size_t curSize = data->shapeVertexCount*sizeof(ShapeVertex);
	memcpy(data->combinedBuffer + offset, data->shapeVertices, curSize);
	offset += curSize;

	DS_ASSERT(offset == dsVectorScratchData_imageVerticesOffset(data));
	curSize = data->imageVertexCount*sizeof(ImageVertex);
	memcpy(data->combinedBuffer + offset, data->imageVertices, curSize);
	offset += curSize;

	DS_ASSERT(offset == dsVectorScratchData_textVerticesOffset(data));
	curSize = data->textVertexCount*sizeof(TextVertex);
	memcpy(data->combinedBuffer + offset, data->textVertices, curSize);
	offset += curSize;

	DS_ASSERT(data->textTessVertexCount == 0 ||
		offset == dsVectorScratchData_textVerticesOffset(data));
	curSize = data->textTessVertexCount*sizeof(TextTessVertex);
	memcpy(data->combinedBuffer + offset, data->textTessVertices, curSize);
	offset += curSize;

	DS_ASSERT(offset == dsVectorScratchData_textVerticesOffset(data));
	curSize = data->textVertexCount*sizeof(TextVertex);
	memcpy(data->combinedBuffer + offset, data->textVertices, curSize);
	offset += curSize;

	DS_ASSERT(offset == dsVectorScratchData_indicesOffset(data));
	curSize = data->indexCount*sizeof(uint16_t);
	memcpy(data->combinedBuffer + offset, data->indices, curSize);
	DS_ASSERT(offset + curSize == totalSize);

	unsigned int usageFlags = dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index;
	unsigned int memoryFlags = dsGfxMemory_Static | dsGfxMemory_Draw;
	if (dsVectorImage_testing)
	{
		usageFlags |= dsGfxBufferUsage_CopyFrom;
		memoryFlags |= dsGfxMemory_Read;
	}
	else
		memoryFlags |= dsGfxMemory_GpuOnly;

	return dsGfxBuffer_create(resourceManager, allocator, usageFlags, memoryFlags,
		data->combinedBuffer, totalSize);
}

uint32_t dsVectorScratchData_shapeVerticesOffset(const dsVectorScratchData* data)
{
	DS_UNUSED(data);
	return 0;
}

uint32_t dsVectorScratchData_imageVerticesOffset(const dsVectorScratchData* data)
{
	return (uint32_t)(data->shapeVertexCount*sizeof(ShapeVertex));
}

uint32_t dsVectorScratchData_textVerticesOffset(const dsVectorScratchData* data)
{
	return (uint32_t)(data->shapeVertexCount*sizeof(ShapeVertex) +
		data->imageVertexCount*sizeof(ImageVertex));
}

uint32_t dsVectorScratchData_indicesOffset(const dsVectorScratchData* data)
{
	return (uint32_t)(data->shapeVertexCount*sizeof(ShapeVertex) +
		data->imageVertexCount*sizeof(ImageVertex) + data->textVertexCount*sizeof(TextVertex) +
		data->textTessVertexCount*sizeof(TextTessVertex));
}
