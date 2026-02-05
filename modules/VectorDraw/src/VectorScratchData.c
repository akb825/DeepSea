/*
 * Copyright 2017-2026 Aaron Barany
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
#include <DeepSea/Geometry/SimpleHoledPolygon.h>

#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector2.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>

#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Text.h>
#include <DeepSea/Text/TextLayout.h>

#include <DeepSea/VectorDraw/VectorMaterialSet.h>

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

static bool hasTexture(dsVectorShaderType type)
{
	switch (type)
	{
		case dsVectorShaderType_Image:
		case dsVectorShaderType_TextColor:
		case dsVectorShaderType_TextColorOutline:
		case dsVectorShaderType_TextGradient:
		case dsVectorShaderType_TextGradientOutline:
			return true;
		default:
			return false;
	}
}

static bool addPiece(dsVectorScratchData* data, dsVectorShaderType type, dsTexture* texture,
	uint32_t infoIndex, MaterialSource materialSource, MaterialSource textOutlineMaterialSource)
{
	bool force = (infoIndex % INFOS_PER_TEXTURE) == 0;
	const TempPiece* prevPiece = data->pieces + data->pieceCount - 1;
	if (!force && data->pieceCount > 0 && prevPiece->type == type &&
		(!hasTexture(type) || prevPiece->texture == texture) &&
		prevPiece->materialSource == materialSource &&
		prevPiece->textOutlineMaterialSource == textOutlineMaterialSource)
	{
		return true;
	}

	uint32_t index = data->pieceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(
			data->allocator, data->pieces, data->pieceCount, data->maxPieces, 1))
	{
		return false;
	}

	TempPiece* piece = data->pieces + index;
	piece->type = type;
	piece->materialSource = materialSource;
	piece->textOutlineMaterialSource = textOutlineMaterialSource;
	piece->infoTextureIndex = infoIndex/INFOS_PER_TEXTURE;
	piece->range.indexCount = 0;
	piece->range.instanceCount = 1;
	piece->range.firstIndex = data->indexCount;
	piece->range.firstInstance = 0;
	switch (type)
	{
		case dsVectorShaderType_FillColor:
		case dsVectorShaderType_FillLinearGradient:
		case dsVectorShaderType_FillRadialGradient:
		case dsVectorShaderType_Line:
			piece->range.vertexOffset = data->shapeVertexCount;
			break;
		case dsVectorShaderType_Image:
			piece->range.vertexOffset = data->imageVertexCount;
			break;
		case dsVectorShaderType_TextColor:
		case dsVectorShaderType_TextColorOutline:
		case dsVectorShaderType_TextGradient:
		case dsVectorShaderType_TextGradientOutline:
			piece->range.firstIndex = data->textDrawInfoCount;
			piece->range.vertexOffset = 0;
			break;
		default:
			DS_ASSERT(false);
			break;
	}
	piece->texture = texture;

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
	data->polygon = dsSimpleHoledPolygon_create(allocator, data, DS_POLYGON_EQUAL_EPSILON_FLOAT,
		DS_POLYGON_INTERSECT_EPSILON_FLOAT);
	if (!data->polygon)
	{
		DS_VERIFY(dsAllocator_free(allocator, data));
		return NULL;
	}
	data->simplifier = dsComplexPolygon_create(allocator, dsGeometryElement_Float, data,
		DS_POLYGON_EQUAL_EPSILON_FLOAT);
	if (!data->simplifier)
	{
		dsSimpleHoledPolygon_destroy(data->polygon);
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
	DS_VERIFY(dsAllocator_free(data->allocator, data->indices));
	DS_VERIFY(dsAllocator_free(data->allocator, data->vectorInfos));
	DS_VERIFY(dsAllocator_free(data->allocator, data->pieces));
	DS_VERIFY(dsAllocator_free(data->allocator, data->loops));
	dsSimpleHoledPolygon_destroy(data->polygon);
	dsComplexPolygon_destroy(data->simplifier);
	for (uint32_t i = 0; i < data->textLayoutCount; ++i)
		dsTextLayout_destroyLayoutAndText(data->textLayouts[i]);
	DS_VERIFY(dsAllocator_free(data->allocator, data->textLayouts));
	DS_VERIFY(dsAllocator_free(data->allocator, data->textDrawInfos));
	DS_VERIFY(dsAllocator_free(data->allocator, data->textStyles));
	DS_VERIFY(dsAllocator_free(data->allocator, data->combinedBuffer));
	DS_VERIFY(dsAllocator_free(data->allocator, data));
}

void dsVectorScratchData_reset(dsVectorScratchData* data)
{
	data->pointCount = 0;
	data->lastStart = 0;
	data->inPath = false;
	data->pathSimple = false;
	data->shapeVertexCount = 0;
	data->imageVertexCount = 0;
	data->indexCount = 0;
	data->vectorInfoCount = 0;
	data->pieceCount = 0;
	data->loopCount = 0;

	for (uint32_t i = 0; i < data->textLayoutCount; ++i)
		dsTextLayout_destroyLayoutAndText(data->textLayouts[i]);
	data->textLayoutCount = 0;
	data->textDrawInfoCount = 0;
}

void* dsVectorScratchData_readUntilEnd(size_t* outSize, dsVectorScratchData* data, dsStream* stream,
	dsAllocator* allocator)
{
	if (!dsStream_readUntilEndReuse(
			&data->fileBuffer, outSize, &data->fileBufferCapacity, stream, allocator))
	{
		return NULL;
	}

	return data->fileBuffer;
}

dsVectorCommand* dsVectorScratchData_createTempCommands(dsVectorScratchData* data,
	uint32_t commandCount)
{
	uint32_t tempCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->tempCommands,
			tempCount, data->maxTempCommands, commandCount))
	{
		return NULL;
	}

	return data->tempCommands;
}

bool dsVectorScratchData_addPoint(dsVectorScratchData* data, const dsVector2f* point,
	uint32_t type)
{
	const float epsilon = 1e-5f;
	if (data->pointCount > 0 &&
		dsVector2f_epsilonEqual(&data->points[data->pointCount - 1].point, point, epsilon))
	{
		data->points[data->pointCount - 1].type |= type;
		return true;
	}

	uint32_t index = data->pointCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(
			data->allocator, data->points, data->pointCount, data->maxPoints, 1))
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

	dsComplexPolygonLoop* loop = data->loops + index;
	loop->points = data->points + firstPoint;
	loop->pointCount = count;
	return true;
}

bool dsVectorScratchData_loopPoint(void* outPoint, const dsComplexPolygon* polygon,
	const void* loop, uint32_t index)
{
	DS_UNUSED(polygon);
	*(dsVector2f*)outPoint = ((const PointInfo*)loop)[index].point;
	return true;
}

dsTextLayout* dsVectorScratchData_shapeText(dsVectorScratchData* data,
	dsCommandBuffer* commandBuffer, const void* string, dsUnicodeType stringType, dsFont* font,
	dsTextAlign alignment, float maxLength, float lineHeight, const dsVectorCommand* ranges,
	uint32_t rangeCount, float pixelSize)
{
	uint32_t tempCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textStyles, tempCount, data->maxTextStyles,
			rangeCount))
	{
		return NULL;
	}

	const dsColor white = {{255, 255, 255, 255}};
	for (uint32_t i = 0; i < rangeCount; ++i)
	{
		if (ranges[i].commandType != dsVectorCommandType_TextRange)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Vector command isn't a text range.");
			return NULL;
		}

		const dsVectorCommandTextRange* range = &ranges[i].textRange;
		dsTextStyle* style = data->textStyles + i;
		style->start = range->start;
		style->count = range->count;
		style->size = range->size;
		style->embolden = range->embolden;
		style->slant = range->slant;
		style->outlinePosition = range->embolden;
		style->outlineThickness = range->outlineWidth;
		style->color = white;
		style->outlineColor = white;
		style->verticalOffset = 0.0f;
		DS_VERIFY(dsFont_applyHintingAndAntiAliasing(font, style, 1.0f/pixelSize, range->fuziness));
	}

	dsText* text = dsText_create(font, data->allocator, string, stringType, false);
	if (!text)
		return NULL;

	dsTextLayout* layout = dsTextLayout_create(data->allocator, text, data->textStyles, rangeCount);
	if (!layout)
	{
		dsText_destroy(text);
		return NULL;
	}

	if (!dsTextLayout_layout(layout, commandBuffer, alignment, maxLength, lineHeight))
	{
		dsTextLayout_destroyLayoutAndText(layout);
		return NULL;
	}

	uint32_t layoutIdx = data->textLayoutCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textLayouts, data->textLayoutCount,
		data->maxLayouts, 1))
	{
		dsTextLayout_destroyLayoutAndText(layout);
		return NULL;
	}

	data->textLayouts[layoutIdx] = layout;
	return layout;
}

void dsVectorScratchData_relinquishText(dsVectorScratchData* data)
{
	data->textLayoutCount = 0;
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

bool dsVectorScratchData_addIndex(dsVectorScratchData* data, uint32_t* vertex)
{
	DS_ASSERT(data->pieceCount > 0);
	TempPiece* piece = data->pieces + data->pieceCount - 1;
	if (*vertex < (uint32_t)piece->range.vertexOffset)
	{
		switch (piece->type)
		{
			case dsVectorShaderType_FillColor:
			case dsVectorShaderType_FillLinearGradient:
			case dsVectorShaderType_FillRadialGradient:
			case dsVectorShaderType_Line:
			{
				uint32_t newVertIndex = data->shapeVertexCount;
				ShapeVertex* newVert = dsVectorScratchData_addShapeVertex(data);
				if (!newVert)
					return false;
				*newVert = data->shapeVertices[*vertex];
				*vertex = newVertIndex;
				break;
			}
			case dsVectorShaderType_Image:
			{
				uint32_t newVertIndex = data->imageVertexCount;
				ImageVertex* newVert = dsVectorScratchData_addImageVertex(data);
				if (!newVert)
					return false;
				*newVert = data->imageVertices[*vertex];
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
		uint32_t oldPieceIdx = (uint32_t)(piece - data->pieces);
		uint32_t pieceIdx = data->pieceCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->pieces, data->pieceCount,
				data->maxPieces, 1))
		{
			return false;
		}

		TempPiece* oldPiece = data->pieces + oldPieceIdx;
		piece = data->pieces + pieceIdx;
		*piece = *oldPiece;
		piece->range.indexCount = 0;
		piece->range.firstIndex = data->indexCount;
		piece->range.vertexOffset = *vertex;
		indexVal = 0;

		// Add any remaining vertices.
		uint32_t remainingIndices = oldPiece->range.indexCount % 3;
		uint32_t firstRemainingIndex = oldPiece->range.firstIndex + oldPiece->range.indexCount -
			remainingIndices;
		for (uint32_t i = 0; i < remainingIndices; ++i)
		{
			uint32_t vertexVal = data->indices[firstRemainingIndex + i] +
				oldPiece->range.vertexOffset;
			if (!dsVectorScratchData_addIndex(data, &vertexVal))
				return false;
		}
		oldPiece->range.indexCount -= remainingIndices;
		DS_ASSERT(piece->range.indexCount == remainingIndices);
	}

	uint32_t index = data->indexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(
			data->allocator, data->indices, data->indexCount, data->maxIndices, 1))
	{
		return false;
	}

	data->indices[index] = (uint16_t)indexVal;
	++piece->range.indexCount;
	return true;
}

ShapeInfo* dsVectorScratchData_addShapePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, float opacity, bool line, dsVectorMaterialType materialType,
	MaterialSource materialSource)
{
	dsVectorShaderType type;
	if (line)
		type = dsVectorShaderType_Line;
	else
	{
		switch (materialType)
		{
			case dsVectorMaterialType_Color:
				type = dsVectorShaderType_FillColor;
				break;
			case dsVectorMaterialType_LinearGradient:
				type = dsVectorShaderType_FillLinearGradient;
				break;
			case dsVectorMaterialType_RadialGradient:
				type = dsVectorShaderType_FillRadialGradient;
				break;
			default:
				DS_ASSERT(false);
				return NULL;
		}
	}

	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, type, NULL, infoIndex, materialSource, MaterialSource_Local))
		return NULL;

	dsAlignedBox2f_makeInvalid(&info->shapeInfo.bounds);
	info->shapeInfo.transformCols[0].x = transform->columns[0].x;
	info->shapeInfo.transformCols[0].y = transform->columns[0].y;
	info->shapeInfo.transformCols[1].x = transform->columns[1].x;
	info->shapeInfo.transformCols[1].y = transform->columns[1].y;
	info->shapeInfo.transformCols[2].x = transform->columns[2].x;
	info->shapeInfo.transformCols[2].y = transform->columns[2].y;
	info->shapeInfo.opacity = opacity;
	info->shapeInfo.padding = 0;
	info->shapeInfo.dashArray.x = info->shapeInfo.dashArray.y = info->shapeInfo.dashArray.z =
		info->shapeInfo.dashArray.w = 0.0f;
	return &info->shapeInfo;
}

ShapeInfo* dsVectorScratchData_addImagePiece(dsVectorScratchData* data,
	const dsMatrix33f* transform, dsTexture* texture, float opacity, const dsAlignedBox2f* bounds)
{
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, dsVectorShaderType_Image, texture, infoIndex, MaterialSource_Local,
			MaterialSource_Local))
	{
		return NULL;
	}

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

bool dsVectorScratchData_addTextPiece(dsVectorScratchData* data, const dsAlignedBox2f* bounds,
	const dsMatrix33f* transform, const dsVector2f* offset, const dsFont* font, float fillOpacity,
	float outlineOpacity, const dsTextLayout* layout, const dsTextStyle* style,
	uint32_t fillMaterial, uint32_t outlineMaterial, dsVectorMaterialType fillMaterialType,
	dsVectorMaterialType outlineMaterialType, MaterialSource fillMaterialSource,
	MaterialSource outlineMaterialSource)
{
	dsVectorShaderType type;
	if (fillMaterialType == dsVectorMaterialType_Color &&
		outlineMaterialType == dsVectorMaterialType_Color)
	{
		if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			type = dsVectorShaderType_TextColor;
		else
			type = dsVectorShaderType_TextColorOutline;
	}
	else
	{
		if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			type = dsVectorShaderType_TextGradient;
		else
			type = dsVectorShaderType_TextGradientOutline;
	}

	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, type, dsFont_getTexture(font), infoIndex, fillMaterialSource,
			outlineMaterialSource))
	{
		return false;
	}

	uint32_t drawInfoIndex = data->textDrawInfoCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textDrawInfos, data->textDrawInfoCount,
		data->maxTextDrawInfos, 1))
	{
		return false;
	}

	TextDrawInfo* drawInfo = data->textDrawInfos + drawInfoIndex;
	drawInfo->layout = layout;
	drawInfo->firstCharacter = style->start;
	drawInfo->characterCount = style->count;
	drawInfo->fillMaterial = fillMaterial;
	drawInfo->outlineMaterial = outlineMaterial;
	drawInfo->infoIndex = infoIndex % INFOS_PER_TEXTURE;
	drawInfo->offset = *offset;
	drawInfo->firstIconGlyph = 0;
	drawInfo->iconGlyphCount = 0;

	TempPiece* piece = data->pieces + data->pieceCount - 1;
	DS_ASSERT(piece->range.firstIndex + piece->range.indexCount == drawInfoIndex);
	++piece->range.indexCount;

	info->textInfo.bounds = *bounds;
	info->textInfo.fillOpacity = fillOpacity;
	info->textInfo.outlineOpacity = outlineOpacity;
	info->textInfo.style.x = style->embolden;
	info->textInfo.style.y = style->slant;
	info->textInfo.style.z = style->outlineThickness;
	info->textInfo.style.w = style->antiAlias;

	for (unsigned int i = 0; i < 3; ++i)
	{
		const dsVector3f* column = transform->columns + i;
		dsVector2f* drawInfoColumn = drawInfo->transformCols + i;
		dsVector2f* infoColumn = info->textInfo.transformCols + i;
		drawInfoColumn->x = column->x;
		drawInfoColumn->y = column->y;
		infoColumn->x = column->x;
		infoColumn->y = column->y;
	}
	return true;
}

bool dsVectorScratchData_addTextRange(dsVectorScratchData* data, const dsVector2f* offset,
	float fillOpacity, float outlineOpacity, const dsTextLayout* layout, const dsTextStyle* style,
	uint32_t fillMaterial, uint32_t outlineMaterial, dsVectorMaterialType fillMaterialType,
	dsVectorMaterialType outlineMaterialType, MaterialSource fillMaterialSource,
	MaterialSource outlineMaterialSource)
{
	dsVectorShaderType type;
	if (fillMaterialType == dsVectorMaterialType_Color &&
		outlineMaterialType == dsVectorMaterialType_Color)
	{
		if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			type = dsVectorShaderType_TextColor;
		else
			type = dsVectorShaderType_TextColorOutline;
	}
	else
	{
		if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			type = dsVectorShaderType_TextGradient;
		else
			type = dsVectorShaderType_TextGradientOutline;
	}

	DS_ASSERT(data->pieceCount > 0);
	TempPiece* prevPiece = data->pieces + data->pieceCount - 1;
	DS_ASSERT(prevPiece->type == dsVectorShaderType_TextColor ||
		prevPiece->type == dsVectorShaderType_TextColorOutline ||
		prevPiece->type == dsVectorShaderType_TextGradient ||
		prevPiece->type == dsVectorShaderType_TextGradientOutline);
	DS_ASSERT(data->vectorInfoCount > 0);
	uint32_t prevInfoIndex = data->vectorInfoCount - 1;
	VectorInfo* prevInfo = data->vectorInfos + prevInfoIndex;

	uint32_t drawInfoIndex = data->textDrawInfoCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->textDrawInfos, data->textDrawInfoCount,
			data->maxTextDrawInfos, 1))
	{
		return false;
	}

	TextDrawInfo* drawInfo = data->textDrawInfos + drawInfoIndex;
	drawInfo->layout = layout;
	drawInfo->firstCharacter = style->start;
	drawInfo->characterCount = style->count;
	drawInfo->fillMaterial = fillMaterial;
	drawInfo->outlineMaterial = outlineMaterial;
	drawInfo->infoIndex = prevInfoIndex % INFOS_PER_TEXTURE;
	drawInfo->offset = *offset;
	memcpy(
		drawInfo->transformCols, prevInfo->textInfo.transformCols, sizeof(drawInfo->transformCols));
	drawInfo->firstIconGlyph = 0;
	drawInfo->iconGlyphCount = 0;

	// Check if the previous info was compatible.
	if (prevInfo->textInfo.fillOpacity == fillOpacity &&
		prevInfo->textInfo.outlineOpacity == outlineOpacity &&
		prevInfo->textInfo.style.x == style->embolden &&
		prevInfo->textInfo.style.y == style->slant &&
		prevInfo->textInfo.style.z == style->outlineThickness &&
		prevInfo->textInfo.style.w == style->antiAlias &&
		prevPiece->type == type &&
		prevPiece->materialSource == fillMaterialSource &&
		prevPiece->textOutlineMaterialSource == outlineMaterialSource)
	{
		DS_ASSERT(prevPiece->range.firstIndex + prevPiece->range.indexCount == drawInfoIndex);
		++prevPiece->range.indexCount;
		return true;
	}

	// Temporarily decrement draw info count so the piece gets the right index.
	--data->textDrawInfoCount;
	uint32_t infoIndex = data->vectorInfoCount;
	VectorInfo* info = addVectorInfo(data);
	if (!info || !addPiece(data, type, prevPiece->texture, infoIndex, fillMaterialSource,
			outlineMaterialSource))
	{
		return false;
	}
	// Put the draw info back.
	++data->textDrawInfoCount;

	// Need to get the prev info again since the array could have been re-allocated.
	prevInfo = data->vectorInfos + prevInfoIndex;
	*info = *prevInfo;
	drawInfo->infoIndex = infoIndex % INFOS_PER_TEXTURE;

	TempPiece* piece = data->pieces + data->pieceCount - 1;
	DS_ASSERT(piece->range.firstIndex + piece->range.indexCount == drawInfoIndex);
	++piece->range.indexCount;

	info->textInfo.fillOpacity = fillOpacity;
	info->textInfo.outlineOpacity = outlineOpacity;
	info->textInfo.style.x = style->embolden;
	info->textInfo.style.y = style->slant;
	info->textInfo.style.z = style->outlineThickness;
	info->textInfo.style.w = style->antiAlias;
	return true;
}

bool dsVectorScratchData_hasGeometry(const dsVectorScratchData* data)
{
	return data->shapeVertexCount*sizeof(ShapeVertex) + data->imageVertexCount*sizeof(ImageVertex) +
		data->indexCount*sizeof(uint16_t) > 0;
}

dsGfxBuffer* dsVectorScratchData_createGfxBuffer(dsVectorScratchData* data,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	size_t shapeVertexSize = data->shapeVertexCount*sizeof(ShapeVertex);
	size_t imageVertexSize = data->imageVertexCount*sizeof(ImageVertex);
	size_t indexSize = data->indexCount*sizeof(uint16_t);

	// End of the buffer must be a multiple of 4 for some platforms.
	size_t alignedIndexSize = indexSize;
	alignedIndexSize = DS_CUSTOM_ALIGNED_SIZE(indexSize, 4);

	size_t totalSize = shapeVertexSize + imageVertexSize + alignedIndexSize;
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
	memcpy(data->combinedBuffer + offset, data->shapeVertices, shapeVertexSize);
	offset += shapeVertexSize;

	data->imageVertexOffset = (uint32_t)offset;
	memcpy(data->combinedBuffer + offset, data->imageVertices, imageVertexSize);
	offset += imageVertexSize;

	data->indexOffset = (uint32_t)offset;
	memcpy(data->combinedBuffer + offset, data->indices, indexSize);
	DS_ASSERT(offset + alignedIndexSize == totalSize);

	dsGfxBufferUsage usageFlags =
		(dsGfxBufferUsage)(dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index);
	unsigned int memoryFlags = dsGfxMemory_Static | dsGfxMemory_Draw;
	if (dsVectorImage_testing)
	{
		usageFlags |= dsGfxBufferUsage_CopyFrom;
		memoryFlags |= dsGfxMemory_Read;
	}
	else
		memoryFlags |= dsGfxMemory_GPUOnly;

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
	return data->imageVertexOffset;
}

uint32_t dsVectorScratchData_indicesOffset(const dsVectorScratchData* data)
{
	return data->indexOffset;
}
