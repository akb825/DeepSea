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

#pragma once

#include <DeepSea/VectorDraw/Types.h>

typedef enum ShaderType
{
	ShaderType_Shape,
	ShaderType_Image,
	ShaderType_Text,
	ShaderType_Count
} ShaderType;

typedef enum PointType
{
	PointType_Normal = 0,
	PointType_Corner = 0x1,
	PointType_JoinStart = 0x2,
	PointType_End = 0x4
} PointType;

typedef struct ShapeVertex
{
	dsVector4f position;
	uint16_t shapeIndex;
	uint16_t materialIndex;
} ShapeVertex;

typedef struct ImageVertex
{
	dsVector2f position;
	dsVector2f texCoords;
	uint32_t shapeIndex;
} ImageVertex;

typedef struct TextVertex
{
	dsVector2f position;
	dsVector3f texCoords;
	uint16_t fillMaterialIndex;
	uint16_t outlineMaterialIndex;
} TextVertex;

typedef struct TempGeometryRange
{
	ShaderType type;
	uint32_t vertexOffset;
	uint32_t vertexCount;
} TempGeometryRange;

typedef struct TempPiece
{
	ShaderType type;
	dsDrawIndexedRange range;
	dsTexture* texture;
} TempPiece;

typedef struct PointInfo
{
	dsVector2f point;
	uint32_t type;
} PointInfo;

typedef struct ShapeInfo
{
	dsAlignedBox2f bounds;
	dsVector2f transformCols[3];
	float opacity;
	float padding;
	dsVector4f dashArray;
} ShapeInfo;

typedef struct TextInfo
{
	dsAlignedBox2f bounds;
	dsVector2f transformCols[3];
	float opacity;
	float padding;
	dsVector4f style;
} TextInfo;

typedef union VectorInfo
{
	ShapeInfo shapeInfo;
	TextInfo textInfo;
	dsVector4f baseSize[4];
} VectorInfo;

struct dsVectorScratchData
{
	dsAllocator* allocator;

	PointInfo* points;
	uint32_t pointCount;
	uint32_t maxPoints;
	uint32_t lastStart;

	bool inPath;
	dsMatrix33f pathTransform;

	ShapeVertex* shapeVertices;
	uint32_t shapeVertexCount;
	uint32_t maxShapeVertices;

	ImageVertex* imageVertices;
	uint32_t imageVertexCount;
	uint32_t maxImageVertices;

	TextVertex* textVertices;
	uint32_t textVertexCount;
	uint32_t maxTextVertices;

	uint16_t* indices;
	uint32_t indexCount;
	uint32_t maxIndices;

	VectorInfo* vectorInfos;
	uint32_t vectorInfoCount;
	uint32_t maxVectorInfos;

	TempPiece* pieces;
	uint32_t pieceCount;
	uint32_t maxPieces;
};

void dsVectorScratchData_reset(dsVectorScratchData* data);
bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type);
ShapeVertex* dsVectorScratchData_addShapeVertex(dsVectorScratchData* data);
ImageVertex* dsVectorScratchData_addImageVertex(dsVectorScratchData* data);
TextVertex* dsVectorScratchData_addTextVertex(dsVectorScratchData* data);
bool dsVectorScratchData_addIndex(dsVectorScratchData* data, uint32_t* vertex);
VectorInfo* dsVectorScratchData_addVectorInfo(dsVectorScratchData* data);
bool dsVectorScratchData_addPiece(dsVectorScratchData* data, ShaderType type, dsTexture* texture);
