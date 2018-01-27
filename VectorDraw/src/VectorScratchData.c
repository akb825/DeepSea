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
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Geometry/AlignedBox2.h>
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
		VectorInfo* infos = DS_ALLOCATE_OBJECT_ARRAY(data->allocator, VectorInfo, newMax);
		if (!infos)
			return NULL;
		memcpy(infos, data->vectorInfos, sizeof(VectorInfo)*data->vectorInfoCount);
		memset(infos + data->vectorInfoCount, 0,
			sizeof(VectorInfo)*(newMax - data->vectorInfoCount));
		dsAllocator_free(data->allocator, data->vectorInfos);
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

	return true;
}

int compareLoopVertex(const void* left, const void* right, void* context)
{
	const dsVectorScratchData* data = (const dsVectorScratchData*)context;
	const LoopVertex* leftVert = (const LoopVertex*)left;
	const LoopVertex* rightVert = (const LoopVertex*)right;
	const dsVector2f* leftPos = &data->polygonVertices[leftVert->vertIndex].point;
	const dsVector2f* rightPos = &data->polygonVertices[rightVert->vertIndex].point;
	if (leftPos->x < rightPos->x)
		return -1;
	else if (leftPos->x > rightPos->x)
		return 1;
	return 0;
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
		dsAllocator_free(data->allocator, data->textVertices);;
	if (data->textTessVertices)
		dsAllocator_free(data->allocator, data->textTessVertices);
	if (data->indices)
		dsAllocator_free(data->allocator, data->indices);
	if (data->vectorInfos)
		dsAllocator_free(data->allocator, data->vectorInfos);
	if (data->pieces)
		dsAllocator_free(data->allocator, data->pieces);
	if (data->polygonVertices)
		dsAllocator_free(data->allocator, data->polygonVertices);
	if (data->polygonEdges)
		dsAllocator_free(data->allocator, data->polygonEdges);
	if (data->loopVertices)
		dsAllocator_free(data->allocator, data->loopVertices);
	if (data->vertexStack)
		dsAllocator_free(data->allocator, data->vertexStack);
	if (data->combinedBuffer)
		dsAllocator_free(data->allocator, data->combinedBuffer);
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
	data->textTessVertexCount = 0;
	data->indexCount = 0;
	data->vectorInfoCount = 0;
	data->pieceCount = 0;
	data->polygonVertCount = 0;
	data->polygonEdgeCount = 0;
	data->loopVertCount = 0;
}

bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type)
{
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
	const dsMatrix33f* transform)
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
	return &info->shapeInfo;
}

ShapeInfo* dsVectorScratchData_addImagePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, dsTexture* texture)
{
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, ShaderType_Image, texture, infoIndex))
		return NULL;

	dsAlignedBox2f_makeInvalid(&info->shapeInfo.bounds);
	info->shapeInfo.transformCols[0].x = transform->columns[0].x;
	info->shapeInfo.transformCols[0].y = transform->columns[0].y;
	info->shapeInfo.transformCols[1].x = transform->columns[1].x;
	info->shapeInfo.transformCols[1].y = transform->columns[1].y;
	info->shapeInfo.transformCols[2].x = transform->columns[2].x;
	info->shapeInfo.transformCols[2].y = transform->columns[2].y;
	return &info->shapeInfo;
}

TextInfo* dsVectorScratchData_addTextPiece(dsVectorScratchData* data, const dsMatrix33f* transform,
	const dsFont* font)
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
	return &info->textInfo;
}

bool dsVectorScratchData_addPolygonVertex(dsVectorScratchData* data, uint32_t vertex)
{
	uint32_t index = data->polygonVertCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->polygonVertices, data->polygonVertCount,
		data->maxPolygonVerts, 1))
	{
		return false;
	}

	data->polygonVertices[index].point = data->points[vertex].point;
	data->polygonVertices[index].hasExtraEdges = false;
	return true;
}

bool dsVectorScratchData_addPolygonEdges(dsVectorScratchData* data)
{
	uint32_t edgeCount = data->polygonVertCount;
	for (uint32_t i = 0; i < data->polygonVertCount; ++i)
	{
		uint32_t edgeIndex = data->polygonEdgeCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->polygonEdges, data->polygonEdgeCount,
			data->maxPolygonEdges, 1))
		{
			return false;
		}

		data->polygonEdges[edgeIndex].prevVertex = i;
		data->polygonEdges[edgeIndex].nextVertex = i == data->polygonVertCount - 1 ? 0 : i + 1;
		data->polygonEdges[edgeIndex].prevEdge = edgeIndex == 0 ? edgeCount - 1 : edgeIndex - 1;
		data->polygonEdges[edgeIndex].nextEdge = edgeIndex == edgeCount - 1 ? 0 : edgeIndex + 1;
		data->polygonEdges[edgeIndex].visited = false;

		data->polygonVertices[i].prevEdge = data->polygonEdges[edgeIndex].prevEdge;
		data->polygonVertices[i].nextEdge = edgeIndex;
	}

	return true;
}

bool dsVectorScratchData_addSeparatingPolygonEdge(dsVectorScratchData* data, uint32_t from,
	uint32_t to, bool ccw)
{
	if ((ccw && data->polygonVertices[from].point.y < data->polygonVertices[to].point.y) ||
		(!ccw && data->polygonVertices[from].point.y > data->polygonVertices[to].point.y))
	{
		uint32_t tmp = from;
		from = to;
		to = tmp;
	}

	uint32_t fromPrevEdge, fromNextEdge;
	if (data->polygonVertices[from].hasExtraEdges)
	{
		// Search for the last edge that goes to the "from" vertex
		fromPrevEdge = 0;
		for (uint32_t i = data->polygonEdgeCount; i-- > 0;)
		{
			if (data->polygonEdges[i].nextVertex == from)
			{
				fromPrevEdge = i;
				break;
			}
		}
		DS_ASSERT(fromPrevEdge > data->polygonVertices[from].prevEdge);
		fromNextEdge = data->polygonVertices[from].nextEdge;
	}
	else
	{
		fromPrevEdge = data->polygonVertices[from].prevEdge;
		fromNextEdge = data->polygonVertices[from].nextEdge;
		data->polygonVertices[from].hasExtraEdges = true;
	}

	// Since we seep left to right, there shouldn't be anything in the "to"
	uint32_t toPrevEdge, toNextEdge;
	if (data->polygonVertices[to].hasExtraEdges)
	{
		// Search for the last edge that goes to the "to" vertex
		toPrevEdge = 0;
		for (uint32_t i = data->polygonEdgeCount; i-- > 0;)
		{
			if (data->polygonEdges[i].nextVertex == to)
			{
				toPrevEdge = i;
				break;
			}
		}
		DS_ASSERT(toPrevEdge > data->polygonVertices[to].prevEdge);
		toNextEdge = data->polygonVertices[to].nextEdge;
	}
	else
	{
		toPrevEdge = data->polygonVertices[to].prevEdge;
		toNextEdge = data->polygonVertices[to].nextEdge;
		data->polygonVertices[to].hasExtraEdges = true;
	}

	// Insert two new edge in-between the edges for the "from" and "to" vertices, one for the left
	// and right sub-polygons.
	uint32_t curEdge = data->polygonEdgeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->polygonEdges, data->polygonEdgeCount,
		data->maxPolygonEdges, 1))
	{
		return false;
	}
	data->polygonEdges[curEdge].prevVertex = from;
	data->polygonEdges[curEdge].nextVertex = to;
	data->polygonEdges[curEdge].prevEdge = fromPrevEdge;
	data->polygonEdges[curEdge].nextEdge = toNextEdge;
	data->polygonEdges[curEdge].visited = false;
	data->polygonEdges[fromPrevEdge].nextEdge = curEdge;
	data->polygonEdges[toNextEdge].prevEdge = curEdge;

	curEdge = data->polygonEdgeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->polygonEdges, data->polygonEdgeCount,
		data->maxPolygonEdges, 1))
	{
		return false;
	}
	data->polygonEdges[curEdge].prevVertex = to;
	data->polygonEdges[curEdge].nextVertex = from;
	data->polygonEdges[curEdge].prevEdge = toPrevEdge;
	data->polygonEdges[curEdge].nextEdge = fromNextEdge;
	data->polygonEdges[curEdge].visited = false;
	data->polygonEdges[toPrevEdge].nextEdge = curEdge;
	data->polygonEdges[toNextEdge].prevEdge = curEdge;

	return true;
}

void dsVectorScratchData_resetPolygon(dsVectorScratchData* data)
{
	data->polygonVertCount = 0;
	data->polygonEdgeCount = 0;
}

bool dsVectorScratchData_addLoopVertex(dsVectorScratchData* data, uint32_t polygonEdge,
	uint32_t shapeIndex, uint32_t materialIndex)
{
	uint32_t index = data->loopVertCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->loopVertices, data->loopVertCount,
		data->maxLoopVerts, 1))
	{
		return false;
	}

	data->loopVertices[index].vertIndex = data->polygonEdges[polygonEdge].prevVertex;
	data->loopVertices[index].indexValue = data->shapeVertexCount;
	data->loopVertices[index].prevVert =
		data->polygonEdges[data->polygonEdges[polygonEdge].prevEdge].prevVertex;
	data->loopVertices[index].nextVert = data->polygonEdges[polygonEdge].nextVertex;

	ShapeVertex* vertex = dsVectorScratchData_addShapeVertex(data);
	if (!vertex)
		return false;

	DS_ASSERT(materialIndex <= USHRT_MAX);
	DS_ASSERT(shapeIndex <= USHRT_MAX);
	const PolygonVertex* polygonVert = data->polygonVertices + data->loopVertices[index].vertIndex;
	vertex->position.x = polygonVert->point.x;
	vertex->position.y = polygonVert->point.y;
	vertex->position.z = -1.0f;
	vertex->position.w = -1.0f;
	vertex->shapeIndex = (uint16_t)shapeIndex;
	vertex->materialIndex = (uint16_t)materialIndex;
	return true;
}

void dsVectorScratchData_sortLoopVertices(dsVectorScratchData* data)
{
	dsSort(data->loopVertices, data->loopVertCount, sizeof(LoopVertex), &compareLoopVertex, data);
}

void dsVectorScratchData_clearLoopVertices(dsVectorScratchData* data)
{
	data->loopVertCount = 0;
}

bool dsVectorScratchData_pushVertex(dsVectorScratchData* data, uint32_t loopVert)
{
	uint32_t index = data->vertStackCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->vertexStack, data->vertStackCount,
		data->maxVertStack, 1))
	{
		return false;
	}

	data->vertexStack[index] = loopVert;
	return true;
}

void dsVectorScratchData_popVertex(dsVectorScratchData* data)
{
	DS_ASSERT(data->vertStackCount > 0);
	--data->vertStackCount;
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
		dsAllocator_free(data->allocator, data->combinedBuffer);
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
