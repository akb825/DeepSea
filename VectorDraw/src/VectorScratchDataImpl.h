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

#include <DeepSea/Core/Config.h>
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define INFOS_PER_TEXTURE 1024
#define NOT_FOUND (uint32_t)-1

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

typedef enum ConnectingEdge
{
	ConnectingEdge_Main,
	ConnectingEdge_LeftTop,
	ConnectingEdge_LeftBottom,
	ConnectingEdge_RightTop,
	ConnectingEdge_RightBottom,
	ConnectingEdge_Count
} ConnectingEdge;

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

typedef struct TextTessVertex
{
	dsVector3f position;
	dsAlignedBox2f geometry;
	dsAlignedBox2f texCoords;
	uint16_t fillMaterialIndex;
	uint16_t outlineMaterialIndex;
} TextTessVertex;

typedef struct TempGeometryRange
{
	ShaderType type;
	uint32_t vertexOffset;
	uint32_t vertexCount;
} TempGeometryRange;

typedef struct TempPiece
{
	ShaderType type;
	uint32_t infoTextureIndex;
	dsDrawIndexedRange range;
	dsTexture* texture;
} TempPiece;

typedef struct PointInfo
{
	dsVector2f point;
	uint32_t type;
} PointInfo;

typedef struct PolygonVertex
{
	dsVector2f point;
	uint32_t prevEdges[ConnectingEdge_Count];
	uint32_t nextEdges[ConnectingEdge_Count];
	uint32_t indexValue;
} PolygonVertex;

typedef struct PolygonEdge
{
	uint32_t prevVertex;
	uint32_t nextVertex;
	uint32_t prevEdge;
	uint32_t nextEdge;
	bool visited;
} PolygonEdge;

typedef struct PolygonEdgeBVHNode
{
	dsAlignedBox2f bounds;
	uint32_t edgeIndex;
	uint32_t leftNode;
	uint32_t rightNode;
} PolygonEdgeBVHNode;

typedef struct LoopVertex
{
	uint32_t vertIndex;
	uint32_t prevVert;
	uint32_t nextVert;
} LoopVertex;

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

	TextTessVertex* textTessVertices;
	uint32_t textTessVertexCount;
	uint32_t maxTextTessVertices;

	uint16_t* indices;
	uint32_t indexCount;
	uint32_t maxIndices;

	VectorInfo* vectorInfos;
	uint32_t vectorInfoCount;
	uint32_t maxVectorInfos;

	TempPiece* pieces;
	uint32_t pieceCount;
	uint32_t maxPieces;

	PolygonVertex* polygonVertices;
	uint32_t polygonVertCount;
	uint32_t maxPolygonVerts;

	PolygonEdge* polygonEdges;
	uint32_t polygonEdgeCount;
	uint32_t maxPolygonEdges;

	uint32_t* sortedPolygonVerts;
	uint32_t* sortedPolygonEdges;
	uint32_t maxSortedPolygonVerts;
	uint32_t maxSortedPolygonEdges;

	PolygonEdgeBVHNode* polygonEdgeBVH;
	uint32_t polygonEdgeBVHCount;
	uint32_t maxPolygonEdgeBVH;

	LoopVertex* loopVertices;
	uint32_t loopVertCount;
	uint32_t maxLoopVerts;

	uint32_t* vertexStack;
	uint32_t vertStackCount;
	uint32_t maxVertStack;

	uint8_t* combinedBuffer;
	size_t combinedBufferSize;
};

void dsVectorScratchData_reset(dsVectorScratchData* data);
bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type);

ShapeVertex* dsVectorScratchData_addShapeVertex(dsVectorScratchData* data);
ImageVertex* dsVectorScratchData_addImageVertex(dsVectorScratchData* data);
TextVertex* dsVectorScratchData_addTextVertex(dsVectorScratchData* data);
TextTessVertex* dsVectorScratchData_addTextTessVertex(dsVectorScratchData* data);
bool dsVectorScratchData_addIndex(dsVectorScratchData* data, uint32_t* vertex);

ShapeInfo* dsVectorScratchData_addShapePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, float opacity);
ShapeInfo* dsVectorScratchData_addImagePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, dsTexture* texture, float opacity, const dsAlignedBox2f* bounds);
TextInfo* dsVectorScratchData_addTextPiece(dsVectorScratchData* data, const dsMatrix33f* transform,
	const dsFont* font, float opacity);

bool dsVectorScratchData_addPolygonVertex(dsVectorScratchData* data, uint32_t vertex,
	uint32_t shapeIndex, uint32_t materialIndex);
bool dsVectorScratchData_addPolygonEdges(dsVectorScratchData* data);
bool dsVectorScratchData_canConnectPolygonEdge(const dsVectorScratchData* data, uint32_t fromVert,
	uint32_t toVert);
bool dsVectorScratchData_addSeparatingPolygonEdge(dsVectorScratchData* data, uint32_t from,
	uint32_t to, bool ccw);
void dsVectorScratchData_resetPolygon(dsVectorScratchData* data);

bool dsVectorScratchData_addLoopVertex(dsVectorScratchData* data, uint32_t polygonEdge);
void dsVectorScratchData_sortLoopVertices(dsVectorScratchData* data);
void dsVectorScratchData_clearLoopVertices(dsVectorScratchData* data);

bool dsVectorScratchData_pushVertex(dsVectorScratchData* data, uint32_t loopVert);
void dsVectorScratchData_popVertex(dsVectorScratchData* data);

dsGfxBuffer* dsVectorScratchData_createGfxBuffer(dsVectorScratchData* data,
	dsResourceManager* resourceManager, dsAllocator* allocator);
uint32_t dsVectorScratchData_shapeVerticesOffset(const dsVectorScratchData* data);
uint32_t dsVectorScratchData_imageVerticesOffset(const dsVectorScratchData* data);
uint32_t dsVectorScratchData_textVerticesOffset(const dsVectorScratchData* data);
uint32_t dsVectorScratchData_indicesOffset(const dsVectorScratchData* data);

extern DS_VECTORDRAW_EXPORT bool dsVectorImage_testing;

#ifdef __cplusplus
}
#endif
