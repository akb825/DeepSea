/*
 * Copyright 2017-2025 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorImage.h>

#include "VectorFill.h"
#include "VectorHelpers.h"
#include "VectorImageImpl.h"
#include "VectorScratchDataImpl.h"
#include "VectorStroke.h"
#include "VectorText.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/CubicCurve.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector4.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>

#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>

#include <limits.h>
#include <string.h>

_Static_assert(sizeof(VectorInfo) == 4*sizeof(dsVector4f), "Unexpected sizeof(VectorInfo).");

typedef enum BaseType
{
	BaseType_Shape,
	BaseType_Image,
	BaseType_Text,
	BaseType_Count
} BaseType;

typedef struct VectorImagePiece
{
	dsTexture* geometryInfo;
	dsTexture* texture;
	dsTextRenderBuffer* textRender;
	dsVectorShaderType type;
	MaterialSource materialSource;
	MaterialSource textOutlineMaterialSource;
	dsDrawIndexedRange range;
} VectorImagePiece;

struct dsVectorImage
{
	dsAllocator* allocator;
	const dsVectorMaterialSet* sharedMaterials;
	dsVectorMaterialSet* localMaterials;
	VectorImagePiece* imagePieces;
	dsTexture** infoTextures;
	dsDrawGeometry* drawGeometries[BaseType_Count];
	dsTextLayout** textLayouts;
	TextDrawInfo* textDrawInfos;
	dsGfxBuffer* buffer;
	uint32_t pieceCount;
	uint32_t infoTextureCount;
	uint32_t textLayoutCount;
	uint32_t textDrawInfoCount;
	dsVector2f size;
};

static bool initResourcesValid(const dsVectorImageInitResources* initResources)
{
	if (!initResources || !initResources->resourceManager || !initResources->scratchData ||
		(!initResources->resources && initResources->resourceCount > 0))
	{
		return false;
	}

	return dsVectorImage_testing || (initResources->commandBuffer &&initResources->shaderModule);
}

static float adjustPixelSize(const dsMatrix33f* transform, float pixelSize)
{
	float xScale = dsVector2f_len((const dsVector2f*)&transform->columns[0]);
	float yScale = dsVector2f_len((const dsVector2f*)&transform->columns[1]);
	return pixelSize*dsMax(xScale, yScale);
}

static bool inPath(const dsVectorScratchData* scratchData)
{
	if (!scratchData->inPath)
	{
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Path command given without a start path command.");
		return false;
	}

	return true;
}

static bool inPathWithPoint(const dsVectorScratchData* scratchData)
{
	if (!inPath(scratchData))
		return false;

	if (scratchData->pointCount == 0 && scratchData->lastStart < scratchData->pointCount)
	{
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"Path continuation command given without an initial move.");
		return false;
	}

	return true;
}

static void markEnd(dsVectorScratchData* scratchData)
{
	if (scratchData->pointCount == 0)
		return;

	scratchData->points[scratchData->pointCount - 1].type |= PointType_End;
}

static void startPath(dsVectorScratchData* scratchData, const dsMatrix33f* transform, bool simple)
{
	scratchData->inPath = true;
	scratchData->pathSimple = simple;
	scratchData->pathTransform = *transform;
	scratchData->pointCount = 0;
	scratchData->lastStart = 0;
	scratchData->loopCount = 0;
}

static bool moveTo(dsVectorScratchData* scratchData, const dsVector2f* position,
	PointType pointType)
{
	if (!inPath(scratchData))
		return false;

	markEnd(scratchData);
	scratchData->lastStart = scratchData->pointCount;
	return dsVectorScratchData_addPoint(scratchData, position, pointType);
}

static bool lineTo(dsVectorScratchData* scratchData, const dsVector2f* position,
	PointType pointType)
{
	return inPathWithPoint(scratchData) && dsVectorScratchData_addPoint(scratchData, position,
		pointType);
}

static bool addBezierPoint(void* userData, const void* point, uint32_t axisCount, float t)
{
	DS_ASSERT(axisCount == 2);
	DS_UNUSED(axisCount);

	if (t == 0.0)
		return true;

	dsVectorScratchData* scratchData = (dsVectorScratchData*)userData;
	PointType pointType = t == 1.0 ? PointType_Corner : PointType_Normal;
	return dsVectorScratchData_addPoint(scratchData, (const dsVector2f*)point, pointType);
}

static bool addBezier(dsVectorScratchData* scratchData, const dsCubicCurvef* curve,
	float pixelSize)
{
	return dsCubicCurvef_tessellate(curve, pixelSize*0.25f, 10, &addBezierPoint, scratchData);
}

static bool addCubic(dsVectorScratchData* scratchData, const dsVector2f* control1,
	const dsVector2f* control2, const dsVector2f* end, float pixelSize)
{
	if (!inPathWithPoint(scratchData))
		return false;

	const dsVector2f* p0 = &scratchData->points[scratchData->pointCount - 1].point;
	dsVector2f p1 = {{control1->x, control1->y}};
	dsVector2f p2 = {{control2->x, control2->y}};
	dsVector2f p3 = {{end->x, end->y}};

	dsCubicCurvef curve;
	DS_VERIFY(dsCubicCurvef_initializeBezier(&curve, 2, p0, &p1, &p2, &p3));
	return addBezier(scratchData, &curve, pixelSize);
}

static bool addQuadratic(dsVectorScratchData* scratchData, const dsVector2f* control,
	const dsVector2f* end, float pixelSize)
{
	if (!inPathWithPoint(scratchData))
		return false;

	const dsVector2f* p0 = &scratchData->points[scratchData->pointCount - 1].point;
	dsVector2f p1 = {{control->x, control->y}};
	dsVector2f p2 = {{end->x, end->y}};

	dsCubicCurvef curve;
	DS_VERIFY(dsCubicCurvef_initializeQuadratic(&curve, 2, p0, &p1, &p2));
	return addBezier(scratchData, &curve, pixelSize);
}

static bool addArc(dsVectorScratchData* scratchData, const dsVector2f* end,
	const dsVector2f* radius, float rotation, bool clockwise, bool largeArc, float pixelSize,
	PointType endType, bool forceCenterScale0)
{
	if (!inPathWithPoint(scratchData))
		return false;

	// https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
	// Straight line if a radius is 0.
	if (radius->x == 0.0f || radius->y == 0.0f)
		return dsVectorScratchData_addPoint(scratchData, end, endType);

	dsMatrix22f rotationMat;
	dsMatrix22f_makeRotate(&rotationMat, rotation);

	dsVector2f start = scratchData->points[scratchData->pointCount - 1].point;
	dsVector2f midPrime;
	dsVector2_sub(midPrime, start, *end);
	dsVector2_scale(midPrime, midPrime, 0.5f);
	dsVector2f posPrime;
	dsMatrix22_transformTransposed(posPrime, rotationMat, midPrime);

	float centerScale = 0.0f;
	if (!forceCenterScale0)
	{
		dsVector2f minRadius = {{fabsf(posPrime.x), fabsf(posPrime.y)}};
		if (radius->x < minRadius.x || radius->y < minRadius.y)
		{
			// Scale up the radius if impossible to find an arc.
			dsVector2f scale;
			dsVector2_div(scale, minRadius, *radius);
			float maxScale = dsMax(scale.x, scale.y);
			dsVector2f scaledRadius;
			dsVector2_scale(scaledRadius, *radius, maxScale);
			return addArc(scratchData, end, &scaledRadius, rotation, clockwise, largeArc, pixelSize,
				endType, true);
		}

		dsVector2f posPrime2, radius2;
		dsVector2_mul(posPrime2, posPrime, posPrime);
		dsVector2_mul(radius2, *radius, *radius);
		centerScale =
			(radius2.x*radius2.y - radius2.x*posPrime2.y - radius2.y*posPrime2.x)/
			(radius2.x*posPrime2.y + radius2.y*posPrime2.x);
		DS_ASSERT(centerScale >= 0.0f);
		centerScale = sqrtf(centerScale);
		if (clockwise == largeArc)
			centerScale = -centerScale;
	}

	dsVector2f centerPrime = {{radius->x*posPrime.y/radius->y,
		-radius->y*posPrime.x/radius->x}};
	dsVector2_scale(centerPrime, centerPrime, centerScale);

	dsVector2f mid, center;
	dsVector2_add(mid, start, *end);
	dsVector2_scale(mid, mid, 0.5f);
	dsMatrix22_transform(center, rotationMat, centerPrime);
	dsVector2_add(center, center, mid);

	dsVector2f v;
	dsVector2_sub(v, posPrime, centerPrime);
	dsVector2_div(v, v, *radius);
	float vLen = dsVector2f_len(&v);
	float cosStartTheta = v.x/vLen;
	float startTheta = acosf(dsClamp(cosStartTheta, -1.0f, 1.0f));
	if (centerPrime.y > posPrime.y)
		startTheta = -startTheta;

	dsVector2f u;
	dsVector2_sub(u, posPrime, centerPrime);
	dsVector2_div(u, u, *radius);
	dsVector2_neg(v, posPrime);
	dsVector2_sub(v, v, centerPrime);
	dsVector2_div(v, v, *radius);

	float cosDeltaTheta = dsVector2_dot(u, v)/(dsVector2f_len(&u)*dsVector2f_len(&v));
	float deltaTheta = acosf(dsClamp(cosDeltaTheta, -1.0f, 1.0f));
	if (u.y*v.x > u.x*v.y)
		deltaTheta = -deltaTheta;

	if (clockwise && deltaTheta < 0.0f)
		deltaTheta += 2*M_PIf;
	else if (!clockwise && deltaTheta > 0.0f)
		deltaTheta -= 2*M_PIf;

	// Target a max curve error of one pixel.
	float pixelTheta = dsVectorPixelTheta(pixelSize, dsMax(radius->x, radius->y));
	unsigned int pointCount = (unsigned int)(fabsf(deltaTheta)/pixelTheta);
	// Amortize the remainder across all points.
	float incr = deltaTheta/(float)pointCount;
	for (unsigned int i = 1; i < pointCount; ++i)
	{
		float theta = startTheta + (float)i*incr;
		dsVector2f basePos = {{cosf(theta), sinf(theta)}};
		dsVector2_mul(basePos, basePos, *radius);

		dsVector2f position;
		dsMatrix22_transform(position, rotationMat, basePos);
		dsVector2_add(position, position, center);
		if (!dsVectorScratchData_addPoint(scratchData, &position, PointType_Normal))
			return false;
	}

	return dsVectorScratchData_addPoint(scratchData, end, PointType_Corner);
}

static bool closePath(dsVectorScratchData* scratchData, PointType pointType)
{
	if (!inPathWithPoint(scratchData))
		return false;

	// Create a copy of the point since the array might be re-allocated.
	dsVector2f startPoint = scratchData->points[scratchData->lastStart].point;
	if (!dsVectorScratchData_addPoint(scratchData, &startPoint, pointType | PointType_End))
		return false;

	scratchData->points[scratchData->lastStart].type |= PointType_JoinStart;
	scratchData->lastStart = scratchData->pointCount;
	return true;
}

static bool addEllipse(dsVectorScratchData* scratchData, const dsVector2f* center,
	const dsVector2f* radius, float pixelSize)
{
	dsVector2f start;
	dsVector2f offset = {{radius->x, 0.0f}};
	dsVector2_add(start, *center, offset);
	if (!moveTo(scratchData, &start, PointType_Normal))
		return false;

	float pixelTheta = dsVectorPixelTheta(pixelSize, dsMax(radius->x, radius->y));
	float deltaTheta = 2*M_PIf;
	unsigned int pointCount = (unsigned int)(deltaTheta/pixelTheta);
	// Amortize the remainder across all points.
	float incr = deltaTheta/(float)pointCount;

	for (unsigned int i = 1; i < pointCount; ++i)
	{
		float theta = (float)i*incr;
		dsVector2f position = {{cosf(theta), sinf(theta)}};
		dsVector2_mul(position, position, *radius);
		dsVector2_add(position, *center, position);
		if (!dsVectorScratchData_addPoint(scratchData, &position, PointType_Normal))
			return false;
	}

	return closePath(scratchData, PointType_Normal);
}

static bool addCorner(dsVectorScratchData* scratchData, const dsVector2f* center,
	const dsVector2f* radius, float startTheta, float incr, unsigned int pointCount,
	bool firstPoint, bool joinPrev)
{
	if (firstPoint || joinPrev)
	{
		dsVector2f position = {{cosf(startTheta), sinf(startTheta)}};
		dsVector2_mul(position, position, *radius);
		dsVector2_add(position, *center, position);
		if (firstPoint)
		{
			if (!moveTo(scratchData, &position, PointType_Normal))
				return false;
		}
		else
		{
			if (!dsVectorScratchData_addPoint(scratchData, &position, PointType_Normal))
				return false;
		}
	}

	for (unsigned int i = 1; i <= pointCount; ++i)
	{
		float theta = startTheta + (float)i*incr;
		dsVector2f position = {{cosf(theta), sinf(theta)}};
		dsVector2_mul(position, position, *radius);
		dsVector2_add(position, *center, position);
		if (!dsVectorScratchData_addPoint(scratchData, &position, PointType_Normal))
			return false;
	}

	return true;
}

static bool addRectangle(dsVectorScratchData* scratchData, const dsAlignedBox2f* bounds,
	const dsVector2f* cornerRadius, float pixelSize)
{
	if (!dsAlignedBox2_isValid(*bounds))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Rectangle bounds are invalid.");
		return false;
	}

	float rx = cornerRadius->x;
	float ry = cornerRadius->y;
	if (rx <= 0.0f && ry > 0.0f)
		rx = ry;
	else if (ry <= 0.0f && rx > 0.0f)
		ry = rx;

	dsVector2f point;
	if (rx <= 0.0f && ry <= 0.0f)
	{
		if (!moveTo(scratchData, &bounds->min, PointType_Corner))
			return false;

		point.x = bounds->max.x;
		point.y = bounds->min.y;
		if (!dsVectorScratchData_addPoint(scratchData, &point, PointType_Corner))
			return false;
		if (!dsVectorScratchData_addPoint(scratchData, &bounds->max, PointType_Corner))
			return false;

		point.x = bounds->min.x;
		point.y = bounds->max.y;
		if (!dsVectorScratchData_addPoint(scratchData, &point, PointType_Corner))
			return false;

		return closePath(scratchData, PointType_Corner);
	}

	dsVector2f halfExtents;
	dsAlignedBox2_extents(halfExtents, *bounds);
	dsVector2_scale(halfExtents, halfExtents, 0.5f);
	if (rx > halfExtents.x)
		rx = halfExtents.x;
	if (ry > halfExtents.y)
		ry = halfExtents.y;

	dsVector2f center;
	dsAlignedBox2_center(center, *bounds);

	float pixelTheta = dsVectorPixelTheta(pixelSize, dsMax(rx, ry));
	float deltaTheta = M_PI_2f;
	unsigned int pointCount = (unsigned int)(deltaTheta/pixelTheta);
	// Amortize the remainder across all points.
	float incr = deltaTheta/(float)pointCount;

	// Corner name comments are based on Cartesian coordinates, since that's more convenient for the
	// math. However, the actual drawing will usually be in image space, where Y is inverted.

	// Upper-right
	dsVector2f finalRadius = {{rx, ry}};
	dsVector2f cornerCenter;
	cornerCenter.x = center.x + halfExtents.x - rx;
	cornerCenter.y = center.y + halfExtents.y - ry;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, 0.0f, incr, pointCount, true, false))
		return false;

	// Upper-left
	cornerCenter.x = center.x - halfExtents.x + rx;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, M_PI_2f, incr, pointCount, false,
			rx < halfExtents.x))
	{
		return false;
	}

	// Lower-left
	cornerCenter.y = center.y - halfExtents.y + ry;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, M_PIf, incr, pointCount, false,
			ry < halfExtents.y))
	{
		return false;
	}

	// Lower-right
	cornerCenter.x = center.x + halfExtents.x - rx;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, M_PIf + M_PI_2f, incr,
			pointCount, false, rx < halfExtents.x))
	{
		return false;
	}

	// Connect to upper-right. Need to remove the last point if the edge is degenerate.
	if (ry >= halfExtents.y)
		--scratchData->pointCount;
	return closePath(scratchData, PointType_Normal);
}

static bool addImage(dsVectorScratchData* scratchData, const dsMatrix33f* transform,
	dsTexture* image, float opacity, const dsAlignedBox2f* bounds)
{
	if (!image)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "No texture to draw image.");
		return false;
	}

	if (!dsAlignedBox2_isValid(*bounds))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Image bounds are invalid.");
		return false;
	}

	uint32_t infoIndex = scratchData->vectorInfoCount;
	DS_ASSERT(infoIndex <= SHRT_MAX);
	ShapeInfo* curInfo = dsVectorScratchData_addImagePiece(scratchData, transform, image, opacity,
		bounds);
	if (!curInfo)
		return false;

	uint32_t upperLeft = scratchData->imageVertexCount;
	ImageVertex* vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position = bounds->min;
	vertex->texCoordX = 0;
	vertex->texCoordY = 0;
	vertex->shapeIndex = (int16_t)infoIndex;
	vertex->padding = 0;

	uint32_t upperRight = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position.x = bounds->min.x;
	vertex->position.y = bounds->max.y;
	vertex->texCoordX = 0;
	vertex->texCoordY = 1;
	vertex->shapeIndex = (int16_t)infoIndex;
	vertex->padding = 0;

	uint32_t lowerRight = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position = bounds->max;
	vertex->texCoordX = 1;
	vertex->texCoordY = 1;
	vertex->shapeIndex = (int16_t)infoIndex;
	vertex->padding = 0;

	uint32_t lowerLeft = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position.x = bounds->max.x;
	vertex->position.y = bounds->min.y;
	vertex->texCoordX = 1;
	vertex->texCoordY = 0;
	vertex->shapeIndex = (int16_t)infoIndex;
	vertex->padding = 0;

	// Clockwise in image space, but will be counter-clockwise in clip space.
	if (!dsVectorScratchData_addIndex(scratchData, &upperLeft) ||
		!dsVectorScratchData_addIndex(scratchData, &upperRight) ||
		!dsVectorScratchData_addIndex(scratchData, &lowerRight))
	{
		return false;
	}

	if (!dsVectorScratchData_addIndex(scratchData, &lowerRight) ||
		!dsVectorScratchData_addIndex(scratchData, &lowerLeft) ||
		!dsVectorScratchData_addIndex(scratchData, &upperLeft))
	{
		return false;
	}

	return true;
}

static bool processCommand(dsVectorScratchData* scratchData, dsCommandBuffer* commandBuffer,
	const dsVectorCommand* commands, uint32_t commandCount, uint32_t* curCommand,
	const dsVectorMaterialSet* sharedMaterials, dsVectorMaterialSet* localMaterials,
	float pixelSize)
{
	DS_UNUSED(commandCount);
	DS_ASSERT(*curCommand < commandCount);
	switch (commands[*curCommand].commandType)
	{
		case dsVectorCommandType_StartPath:
		{
			const dsVectorCommandStartPath* start = &commands[(*curCommand)++].startPath;
			startPath(scratchData, &start->transform, start->simple);
			return true;
		}
		case dsVectorCommandType_Move:
			return moveTo(scratchData, &commands[(*curCommand)++].move.position, PointType_Corner);
		case dsVectorCommandType_Line:
			return lineTo(scratchData, &commands[(*curCommand)++].line.end, PointType_Corner);
		case dsVectorCommandType_Bezier:
		{
			const dsVectorCommandBezier* bezier = &commands[(*curCommand)++].bezier;
			return addCubic(scratchData, &bezier->control1, &bezier->control2, &bezier->end,
				pixelSize);
		}
		case dsVectorCommandType_Quadratic:
		{
			const dsVectorCommandQuadratic* quadratic = &commands[(*curCommand)++].quadratic;
			return addQuadratic(scratchData, &quadratic->control, &quadratic->end, pixelSize);
		}
		case dsVectorCommandType_Arc:
		{
			const dsVectorCommandArc* arc = &commands[(*curCommand)++].arc;
			dsVector2f radius = {{fabsf(arc->radius.x), fabsf(arc->radius.y)}};
			return addArc(scratchData, &arc->end, &radius, arc->rotation, arc->clockwise,
				arc->largeArc, adjustPixelSize(&scratchData->pathTransform, pixelSize),
				PointType_Corner, false);
		}
		case dsVectorCommandType_ClosePath:
			++(*curCommand);
			return closePath(scratchData, PointType_Corner);
		case dsVectorCommandType_Ellipse:
		{
			const dsVectorCommandEllipse* ellipse = &commands[(*curCommand)++].ellipse;
			return addEllipse(scratchData, &ellipse->center, &ellipse->radius,
				adjustPixelSize(&scratchData->pathTransform, pixelSize));
		}
		case dsVectorCommandType_Rectangle:
		{
			const dsVectorCommandRectangle* rectangle = &commands[(*curCommand)++].rectangle;
			return addRectangle(scratchData, &rectangle->bounds, &rectangle->cornerRadius,
				adjustPixelSize(&scratchData->pathTransform, pixelSize));
		}
		case dsVectorCommandType_StrokePath:
			if (!inPathWithPoint(scratchData))
				return false;
			return dsVectorStroke_add(scratchData, sharedMaterials, localMaterials,
				&commands[(*curCommand)++].strokePath, adjustPixelSize(&scratchData->pathTransform,
				pixelSize));
		case dsVectorCommandType_FillPath:
			if (!inPathWithPoint(scratchData))
				return false;
			return dsVectorFill_add(scratchData, sharedMaterials, localMaterials,
				&commands[(*curCommand)++].fillPath);
		case dsVectorCommandType_Image:
		{
			const dsVectorCommandImage* image = &commands[(*curCommand)++].image;
			return addImage(scratchData, &image->transform, image->image, image->opacity,
				&image->imageBounds);
		}
		case dsVectorCommandType_Text:
		{
			const dsVectorCommandText* text = &commands[(*curCommand)++].text;
			if (!DS_IS_BUFFER_RANGE_VALID(*curCommand, text->rangeCount, commandCount))
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Text ranges out of command range.");
				return false;
			}

			if (!dsVectorText_addText(scratchData, commandBuffer, sharedMaterials, localMaterials,
				text, commands + *curCommand, pixelSize))
			{
				return false;
			}
			*curCommand += text->rangeCount;
			return true;
		}
		default:
			errno = EINVAL;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Invalid vector command.");
			return false;
	}
}

static bool processCommands(dsVectorScratchData* scratchData, dsCommandBuffer* commandBuffer,
	const dsVectorCommand* commands, uint32_t commandCount,
	const dsVectorMaterialSet* sharedMaterials, dsVectorMaterialSet* localMaterials,
	float pixelSize)
{
	dsVectorScratchData_reset(scratchData);
	for (uint32_t i = 0; i < commandCount;)
	{
		if (!processCommand(scratchData, commandBuffer, commands, commandCount, &i, sharedMaterials,
			localMaterials, pixelSize))
		{
			return false;
		}
	}

	return true;
}

static bool createShapeGeometry(dsVectorImage* image, dsVectorScratchData* scratchData,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (scratchData->shapeVertexCount == 0)
		return true;

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_SInt);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset ==
		offsetof(ShapeVertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(ShapeVertex, shapeIndex));
	DS_ASSERT(vertexFormat.size == sizeof(ShapeVertex));
	dsVertexBuffer vertexBuffer =
	{
		image->buffer, dsVectorScratchData_shapeVerticesOffset(scratchData),
			scratchData->shapeVertexCount, vertexFormat
	};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};
	dsIndexBuffer indexBuffer =
	{
		image->buffer, dsVectorScratchData_indicesOffset(scratchData), scratchData->indexCount,
		sizeof(uint16_t)
	};

	image->drawGeometries[BaseType_Shape] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[BaseType_Shape] != NULL;
}

static bool createImageGeometry(dsVectorImage* image, dsVectorScratchData* scratchData,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (scratchData->imageVertexCount == 0)
		return true;

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SInt);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset ==
		offsetof(ImageVertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(ImageVertex, texCoordX));
	DS_ASSERT(vertexFormat.size == sizeof(ImageVertex));
	dsVertexBuffer vertexBuffer =
	{
		image->buffer, dsVectorScratchData_shapeVerticesOffset(scratchData),
			scratchData->shapeVertexCount, vertexFormat
	};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};
	dsIndexBuffer indexBuffer =
	{
		image->buffer, dsVectorScratchData_indicesOffset(scratchData), scratchData->indexCount,
		sizeof(uint16_t)
	};

	image->drawGeometries[BaseType_Image] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[BaseType_Image] != NULL;
}

static bool addTextRanges(dsVectorImage* vectorImage, dsCommandBuffer* commandBuffer)
{
	for (uint32_t i = 0; i < vectorImage->pieceCount; ++i)
	{
		const VectorImagePiece* piece = vectorImage->imagePieces + i;
		if (piece->type != dsVectorShaderType_TextColor &&
			piece->type != dsVectorShaderType_TextColorOutline &&
			piece->type != dsVectorShaderType_TextGradient &&
			piece->type != dsVectorShaderType_TextGradientOutline)
		{
			continue;
		}

		DS_ASSERT(piece->textRender);
		DS_VERIFY(dsTextRenderBuffer_clear(piece->textRender));
		for (uint32_t j = 0; j < piece->range.indexCount; ++j)
		{
			DS_ASSERT(piece->range.firstIndex + j < vectorImage->textDrawInfoCount);
			TextDrawInfo* drawInfo = vectorImage->textDrawInfos + piece->range.firstIndex + j;
			DS_VERIFY(dsTextRenderBuffer_addTextRange(piece->textRender, drawInfo->layout, drawInfo,
				drawInfo->firstCharacter, drawInfo->characterCount));
		}

		if (!dsTextRenderBuffer_commit(piece->textRender, commandBuffer))
			return false;
	}

	return true;
}

static BaseType getBaseType(dsVectorShaderType type)
{
	switch (type)
	{
		case dsVectorShaderType_FillColor:
		case dsVectorShaderType_FillLinearGradient:
		case dsVectorShaderType_FillRadialGradient:
		case dsVectorShaderType_Line:
			return BaseType_Shape;
		case dsVectorShaderType_Image:
			return BaseType_Image;
		case dsVectorShaderType_TextColor:
		case dsVectorShaderType_TextColorOutline:
		case dsVectorShaderType_TextGradient:
		case dsVectorShaderType_TextGradientOutline:
			return BaseType_Text;
		default:
			DS_ASSERT(false);
			return BaseType_Count;
	}
}

dsVectorImage* dsVectorImage_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* intResources, const void* data, size_t size,
	float pixelSize, const dsVector2f* targetSize, const char* name);

dsVectorImage* dsVectorImage_create(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const dsVectorCommand* commands,
	uint32_t commandCount, dsVectorMaterialSet* localMaterials, const dsVector2f* size,
	float pixelSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !commands || commandCount == 0 ||
		!size || size->x <= 0.0f || size->y <= 0.0f || pixelSize <= 0.0f)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxFormat infoFormat = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (!dsGfxFormat_textureSupported(initResources->resourceManager, infoFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"Floating point textures are required for vector images.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsVectorScratchData* scratchData = initResources->scratchData;
	if (!processCommands(scratchData, initResources->commandBuffer, commands, commandCount,
			initResources->sharedMaterials, localMaterials, pixelSize))
	{
		dsVectorScratchData_reset(scratchData);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint32_t infoTextureCount = (scratchData->vectorInfoCount + INFOS_PER_TEXTURE - 1)/
		INFOS_PER_TEXTURE;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVectorImage)) +
		DS_ALIGNED_SIZE(sizeof(VectorImagePiece)*scratchData->pieceCount) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*infoTextureCount) +
		DS_ALIGNED_SIZE(sizeof(dsTextLayout*)*scratchData->textLayoutCount) +
		DS_ALIGNED_SIZE(sizeof(TextDrawInfo)*scratchData->textDrawInfoCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		dsVectorScratchData_reset(scratchData);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVectorImage* image = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVectorImage);
	DS_ASSERT(image);
	memset(image, 0, sizeof(dsVectorImage));
	image->allocator = dsAllocator_keepPointer(allocator);

	dsResourceManager* resourceManager = initResources->resourceManager;
	// If no info textures, the image is empty.
	if (infoTextureCount > 0)
	{
		// Create the info textures.
		DS_ASSERT(scratchData->maxVectorInfos % INFOS_PER_TEXTURE == 0);
		dsTextureInfo infoTexInfo = {infoFormat, dsTextureDim_2D, 4, 0, 0, 1, 1};
		image->infoTextures = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsTexture*, infoTextureCount);
		for (uint32_t i = 0; i < infoTextureCount; ++i, ++image->infoTextureCount)
		{
			infoTexInfo.height = INFOS_PER_TEXTURE;
			if (i == infoTextureCount - 1 &&
				(scratchData->vectorInfoCount % INFOS_PER_TEXTURE) != 0)
			{
				infoTexInfo.height =
					dsNextPowerOf2(scratchData->vectorInfoCount % INFOS_PER_TEXTURE);
			}

			image->infoTextures[i] = dsTexture_create(resourceManager, resourceAllocator,
				dsTextureUsage_Texture, dsGfxMemory_Static | dsGfxMemory_GPUOnly, &infoTexInfo,
				scratchData->vectorInfos + i*INFOS_PER_TEXTURE,
				sizeof(VectorInfo)*infoTexInfo.height);
			if (!image->infoTextures[i])
			{
				dsVectorScratchData_reset(scratchData);
				DS_VERIFY(dsVectorImage_destroy(image));
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		// Create the geometry for the shapes and images in a single combined buffer.
		if (dsVectorScratchData_hasGeometry(scratchData))
		{
			image->buffer = dsVectorScratchData_createGfxBuffer(scratchData, resourceManager,
				resourceAllocator);
			if (!image->buffer)
			{
				dsVectorScratchData_reset(scratchData);
				DS_VERIFY(dsVectorImage_destroy(image));
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (!createShapeGeometry(image, scratchData, resourceManager, resourceAllocator) ||
				!createImageGeometry(image, scratchData, resourceManager, resourceAllocator))
			{
				dsVectorScratchData_reset(scratchData);
				DS_VERIFY(dsVectorImage_destroy(image));
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		// Create the pieces to define each draw call.
		DS_ASSERT(scratchData->pieceCount > 0);
		image->imagePieces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, VectorImagePiece,
			scratchData->pieceCount);
		DS_ASSERT(image->imagePieces);
		for (uint32_t i = 0; i < scratchData->pieceCount; ++i)
		{
			TempPiece* tempPiece = scratchData->pieces + i;
			VectorImagePiece* imagePiece = image->imagePieces + i;
			imagePiece->geometryInfo = image->infoTextures[tempPiece->infoTextureIndex];
			imagePiece->texture = tempPiece->texture;
			imagePiece->textRender = NULL;
			imagePiece->type = tempPiece->type;
			imagePiece->materialSource = tempPiece->materialSource;
			imagePiece->textOutlineMaterialSource = tempPiece->textOutlineMaterialSource;
			imagePiece->range = tempPiece->range;
		}
		image->pieceCount = scratchData->pieceCount;
		DS_ASSERT(infoTextureCount > 0);

		// Create the text objects.
		if (scratchData->textLayoutCount > 0)
		{
			// Move the text layouts from the scratch data to the image.
			image->textLayoutCount = scratchData->textLayoutCount;
			image->textLayouts = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsTextLayout*,
				image->textLayoutCount);
			DS_ASSERT(image->textLayouts);
			memcpy(image->textLayouts, scratchData->textLayouts,
				sizeof(dsTextLayout*)*image->textLayoutCount);
			dsVectorScratchData_relinquishText(scratchData);

			// Get the draw infos for the different ranges of text with different materials.
			DS_ASSERT(scratchData->textDrawInfoCount > 0);
			image->textDrawInfoCount = scratchData->textDrawInfoCount;
			image->textDrawInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, TextDrawInfo,
				image->textDrawInfoCount);
			DS_ASSERT(image->textDrawInfos);
			memcpy(image->textDrawInfos, scratchData->textDrawInfos,
				sizeof(TextDrawInfo)*image->textDrawInfoCount);

			dsVertexFormat textVertexFormat;
			if (!dsVectorText_createVertexFormat(&textVertexFormat, initResources))
			{
				dsVectorScratchData_reset(scratchData);
				DS_VERIFY(dsVectorImage_destroy(image));
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			// Create the text renders for each piece of text, tying the vector image data with the
			// draw info for how to draw the text itself.
			for (uint32_t i = 0; i < image->pieceCount; ++i)
			{
				if (image->imagePieces[i].type != dsVectorShaderType_TextColor &&
					image->imagePieces[i].type != dsVectorShaderType_TextColorOutline &&
					image->imagePieces[i].type != dsVectorShaderType_TextGradient &&
					image->imagePieces[i].type != dsVectorShaderType_TextGradientOutline)
				{
					continue;
				}

				image->imagePieces[i].textRender = dsVectorText_createRenderBuffer(allocator,
					resourceManager, &textVertexFormat, &image->imagePieces[i].range,
					image->textDrawInfos, image->textDrawInfoCount);
				if (!image->imagePieces[i].textRender)
				{
					dsVectorScratchData_reset(scratchData);
					DS_VERIFY(dsVectorImage_destroy(image));
					DS_PROFILE_FUNC_RETURN(NULL);
				}
			}

			if (!addTextRanges(image, initResources->commandBuffer))
			{
				dsVectorScratchData_reset(scratchData);
				DS_VERIFY(dsVectorImage_destroy(image));
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}
	}

	dsVectorScratchData_reset(scratchData);

	image->sharedMaterials = initResources->sharedMaterials;
	image->localMaterials = localMaterials;
	image->size = *size;
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const char* filePath, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector image file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsVectorScratchData_readUntilEnd(&size, initResources->scratchData,
		(dsStream*)&stream, initResources->scratchData->allocator);
	dsFileStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, filePath);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadResource(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, dsFileResourceType type, const char* filePath,
	float pixelSize, const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector image file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsVectorScratchData_readUntilEnd(&size, initResources->scratchData,
		(dsStream*)&stream, initResources->scratchData->allocator);
	dsResourceStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, filePath);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadArchive(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const dsFileArchive* archive,
	const char* filePath, float pixelSize, const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !archive || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector image file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsVectorScratchData_readUntilEnd(&size, initResources->scratchData, stream,
		initResources->scratchData->allocator);
	dsStream_close(stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, filePath);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadStream(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, dsStream* stream, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t size;
	void* buffer = dsVectorScratchData_readUntilEnd(&size, initResources->scratchData, stream,
		initResources->scratchData->allocator);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, NULL);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const void* data, size_t size, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResourcesValid(initResources) || !data || size == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources, data,
		size, pixelSize, targetSize, NULL);
	DS_PROFILE_FUNC_RETURN(image);
}

bool dsVectorImage_updateText(dsVectorImage* vectorImage, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!vectorImage || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (vectorImage->textLayoutCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	for (uint32_t i = 0; i < vectorImage->textLayoutCount; ++i)
	{
		if (!dsTextLayout_refresh(vectorImage->textLayouts[i], commandBuffer))
			DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = addTextRanges(vectorImage, commandBuffer);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsVectorImage_draw(const dsVectorImage* vectorImage, dsCommandBuffer* commandBuffer,
	const dsVectorShaders* shaders, dsMaterial* material, const dsMatrix44f* modelViewProjection,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates)
{
	DS_PROFILE_FUNC_START();

	if (!vectorImage || !commandBuffer || !shaders || !material || !modelViewProjection)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (shaders->shaderModule->materialDesc != dsMaterial_getDescription(material))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"Material wasn't created with the same shader module as the shaders.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (vectorImage->pieceCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsVectorShaderModule* shaderModule = shaders->shaderModule;
	if (!dsMaterial_setElementData(material, shaderModule->modelViewProjectionElement,
			modelViewProjection, dsMaterialType_Mat4, 0, 1) ||
		!dsMaterial_setElementData(material, shaderModule->sizeElement, &vectorImage->size,
			dsMaterialType_Vec2, 0, 1))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsTexture* sharedMaterialInfoTexture = dsVectorMaterialSet_getInfoTexture(
		vectorImage->sharedMaterials);
	dsTexture* sharedMaterialColorTexture = dsVectorMaterialSet_getColorTexture(
		vectorImage->sharedMaterials);
	dsTexture* localMaterialInfoTexture = dsVectorMaterialSet_getInfoTexture(
		vectorImage->localMaterials);
	dsTexture* localMaterialColorTexture = dsVectorMaterialSet_getColorTexture(
		vectorImage->localMaterials);

	bool success = true;
	dsVector3f textureSizes = {{0.0f, 0.0f, 0.0f}};
	if (sharedMaterialInfoTexture)
	{
		DS_ASSERT(sharedMaterialInfoTexture->info.height == sharedMaterialInfoTexture->info.height);
		textureSizes.y = (float)sharedMaterialInfoTexture->info.height;
	}

	if (localMaterialInfoTexture)
	{
		DS_ASSERT(localMaterialInfoTexture->info.height == localMaterialInfoTexture->info.height);
		textureSizes.z = (float)localMaterialInfoTexture->info.height;
	}

	for (uint32_t i = 0; i < vectorImage->pieceCount; ++i)
	{
		const VectorImagePiece* piece = vectorImage->imagePieces + i;
		textureSizes.x = (float)piece->geometryInfo->info.height;

		dsTexture* materialInfoTexture;
		dsTexture* materialColorTexture;
		dsTexture* textOutlineMaterialInfoTexture;
		dsTexture* textOutlineMaterialColorTexture;
		if (piece->materialSource == MaterialSource_Local)
		{
			materialInfoTexture = localMaterialInfoTexture;
			materialColorTexture = localMaterialColorTexture;
		}
		else
		{
			DS_ASSERT(piece->materialSource == MaterialSource_Shared);
			materialInfoTexture = sharedMaterialInfoTexture;
			materialColorTexture = sharedMaterialColorTexture;
		}

		if (piece->textOutlineMaterialSource == MaterialSource_Local)
		{
			textOutlineMaterialInfoTexture = localMaterialInfoTexture;
			textOutlineMaterialColorTexture = localMaterialColorTexture;
		}
		else
		{
			DS_ASSERT(piece->textOutlineMaterialSource == MaterialSource_Shared);
			textOutlineMaterialInfoTexture = sharedMaterialInfoTexture;
			textOutlineMaterialColorTexture = sharedMaterialColorTexture;
		}

		textureSizes.y = (float)materialInfoTexture->info.height;
		textureSizes.z = (float)textOutlineMaterialInfoTexture->info.height;

		// Need to have a non-NULL texture to bind. If the piece doesn't have a specific texture
		// (i.e. not an image or texture element), just re-use the material color.
		dsTexture* otherTexture = piece->texture;
		if (!otherTexture)
			otherTexture = materialColorTexture;

		if (!dsMaterial_setElementData(material, shaderModule->textureSizesElement, &textureSizes,
				dsMaterialType_Vec3, 0, 1) ||
			!dsMaterial_setTexture(material, shaderModule->shapeInfoTextureElement,
				piece->geometryInfo) ||
			!dsMaterial_setTexture(material, shaderModule->otherTextureElement, otherTexture) ||
			!dsMaterial_setTexture(material, shaderModule->materialInfoTextureElement,
				materialInfoTexture) ||
			!dsMaterial_setTexture(material, shaderModule->materialColorTextureElement,
				materialColorTexture) ||
			!dsMaterial_setTexture(material,
				shaderModule->textOutlineMaterialInfoTextureElement,
				textOutlineMaterialInfoTexture) ||
			!dsMaterial_setTexture(material,
				shaderModule->textOutlineMaterialColorTextureElement,
				textOutlineMaterialColorTexture))
		{
			success = false;
			break;
		}

		dsShader* shader = shaders->shaders[piece->type];
		if (!dsShader_bind(shader, commandBuffer, material, globalValues, renderStates))
		{
			success = false;
			break;
		}
		if (piece->type == dsVectorShaderType_TextColor ||
			piece->type == dsVectorShaderType_TextColorOutline ||
			piece->type == dsVectorShaderType_TextGradient ||
			piece->type == dsVectorShaderType_TextGradientOutline)
		{
			DS_ASSERT(piece->textRender);
			success = dsTextRenderBuffer_draw(piece->textRender, commandBuffer);
		}
		else
		{
			success = dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer,
				vectorImage->drawGeometries[getBaseType(piece->type)], &piece->range,
				dsPrimitiveType_TriangleList);
		}
		// Make sure we unbind the shader even if the above draw failed.
		if (!dsShader_unbind(shader, commandBuffer) || !success)
		{
			success = false;
			break;
		}
	}

	// Clean up textures that might be deleted before the next draw.
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->materialInfoTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->materialColorTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->textOutlineMaterialInfoTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->textOutlineMaterialColorTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->shapeInfoTextureElement, NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->otherTextureElement, NULL));

	DS_PROFILE_FUNC_RETURN(success);
}

bool dsVectorImage_getSize(dsVector2f* outSize, const dsVectorImage* vectorImage)
{
	if (!outSize || !vectorImage)
	{
		errno = EINVAL;
		return false;
	}

	*outSize = vectorImage->size;
	return true;
}

const dsVectorMaterialSet* dsVectorImage_getSharedMaterials(const dsVectorImage* vectorImage)
{
	if (!vectorImage)
		return NULL;

	return vectorImage->sharedMaterials;
}

dsVectorMaterialSet* dsVectorImage_getLocalMaterials(dsVectorImage* vectorImage)
{
	if (!vectorImage)
		return NULL;

	return vectorImage->localMaterials;
}

bool dsVectorImage_destroy(dsVectorImage* vectorImage)
{
	if (!vectorImage)
		return true;

	for (uint32_t i = 0; i < vectorImage->pieceCount; ++i)
	{
		if (!dsTextRenderBuffer_destroy(vectorImage->imagePieces[i].textRender))
			return false;
	}

	for (uint32_t i = 0; i < vectorImage->infoTextureCount; ++i)
	{
		if (!dsTexture_destroy(vectorImage->infoTextures[i]))
			return false;
	}

	for (int i = 0; i < BaseType_Count; ++i)
	{
		if (!dsDrawGeometry_destroy(vectorImage->drawGeometries[i]))
			return false;
	}

	for (uint32_t i = 0; i < vectorImage->textLayoutCount; ++i)
		dsTextLayout_destroyLayoutAndText(vectorImage->textLayouts[i]);

	if (!dsGfxBuffer_destroy(vectorImage->buffer))
		return false;

	DS_VERIFY(dsVectorMaterialSet_destroy(vectorImage->localMaterials));

	if (vectorImage->allocator)
		DS_VERIFY(dsAllocator_free(vectorImage->allocator, vectorImage));
	return true;
}

dsGfxBuffer* dsVectorImage_getBuffer(dsVectorImage* image)
{
	return image->buffer;
}
