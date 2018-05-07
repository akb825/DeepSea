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

#include <DeepSea/VectorDraw/VectorImage.h>

#include "VectorFill.h"
#include "VectorHelpers.h"
#include "VectorImageImpl.h"
#include "VectorScratchDataImpl.h"
#include "VectorStroke.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
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
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <limits.h>
#include <string.h>

typedef struct TessTextVertex
{
	dsVector3f position;
	dsAlignedBox2f geometry;
	dsAlignedBox2f texCoords;
	uint16_t fillMaterialIndex;
	uint16_t outlineMaterialIndex;
} TessTextVertex;

DS_STATIC_ASSERT(sizeof(VectorInfo) == 4*sizeof(dsVector4f), unexpected_VectorInfo_size);

typedef struct dsVectorImagePiece
{
	dsTexture* geometryInfo;
	dsTexture* texture;
	ShaderType type;
	dsDrawIndexedRange range;
} dsVectorImagePiece;

struct dsVectorImage
{
	dsAllocator* allocator;
	const dsVectorMaterialSet* sharedMaterials;
	dsVectorMaterialSet* localMaterials;
	dsVectorImagePiece* imagePieces;
	dsTexture** infoTextures;
	dsDrawGeometry* drawGeometries[ShaderType_Count];
	dsGfxBuffer* buffer;
	uint32_t pieceCount;
	uint32_t infoTextureCount;
	dsVector2f size;
};

// Left and right subdivision matrices from http://algorithmist.net/docs/subdivision.pdf
static const dsMatrix44f leftBezierMatrix =
{{
	{1.0f, 0.5f, 0.25f, 0.125f},
	{0.0f, 0.5f, 0.5f , 0.375f},
	{0.0f, 0.0f, 0.25f, 0.375f},
	{0.0f, 0.0f, 0.0f , 0.125f}
}};

static const dsMatrix44f rightBezierMatrix =
{{
	{0.125f, 0.0f , 0.0f, 0.0f},
	{0.375f, 0.25f, 0.0f, 0.0f},
	{0.375f, 0.5f , 0.5f, 0.0f},
	{0.125f, 0.25f, 0.5f, 1.0f}
}};

static const dsVector4f bezierMid = {{0.125f, 0.375f, 0.375f, 0.125f}};

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

static bool isBezierStraight(const dsVector4f* curveX, const dsVector4f* curveY, float pixelSize)
{
	// Check to see if the midpoint is within a pixel of a straight line.
	dsVector2f midCurve = {{dsVector4_dot(*curveX, bezierMid), dsVector4_dot(*curveY, bezierMid)}};
	dsVector2f midLine = {{(curveX->x + curveX->w)*0.5f, (curveY->x + curveX->w)*0.5f}};
	return dsVector2_dist2(midCurve, midLine) <= dsPow2(pixelSize);
}

static void startPath(dsVectorScratchData* scratchData, const dsMatrix33f* transform)
{
	scratchData->inPath = true;
	scratchData->pathTransform = *transform;
	scratchData->pointCount = 0;
	scratchData->lastStart = 0;
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

static bool addBezierRec(dsVectorScratchData* scratchData, const dsVector2f* start,
	const dsVector2f* control1, const dsVector2f* control2, const dsVector2f* end, float pixelSize,
	uint32_t level)
{
	// Sanity check to avoid too much recursion.
	const uint32_t maxLevels = 10;

	// Subdivide the bazier: http://algorithmist.net/docs/subdivision.pdf
	dsVector4f bezierX = {{start->x, control1->x, control2->x, end->x}};
	dsVector4f bezierY = {{start->y, control1->y, control2->y, end->y}};

	// Left side.
	dsVector4f leftX;
	dsVector4f leftY;
	dsMatrix44_transform(leftX, leftBezierMatrix, bezierX);
	dsMatrix44_transform(leftY, leftBezierMatrix, bezierY);
	{
		dsVector2f nextStart = {{leftX.x, leftY.x}};
		dsVector2f nextControl1 = {{leftX.y, leftY.y}};
		dsVector2f nextControl2 = {{leftX.z, leftY.z}};
		dsVector2f nextEnd = {{leftX.w, leftY.w}};
		if (level < maxLevels && !isBezierStraight(&leftX, &leftY, pixelSize))
		{
			if (!addBezierRec(scratchData, &nextStart, &nextControl1, &nextControl2, &nextEnd,
				pixelSize, level + 1))
			{
				return false;
			}
		}

		// The end point is guaranteed to be on the curve.
		if (!dsVectorScratchData_addPoint(scratchData, &nextEnd, PointType_Normal))
			return false;
	}

	// Right side.
	dsVector4f rightX;
	dsVector4f rightY;
	dsMatrix44_transform(rightX, rightBezierMatrix, bezierX);
	dsMatrix44_transform(rightY, rightBezierMatrix, bezierY);
	{
		dsVector2f nextStart = {{rightX.x, rightY.x}};
		dsVector2f nextControl1 = {{rightX.y, rightY.y}};
		dsVector2f nextControl2 = {{rightX.z, rightY.z}};
		dsVector2f nextEnd = {{rightX.w, rightY.w}};
		if (level < maxLevels && !isBezierStraight(&rightX, &rightY, pixelSize))
		{
			if (!addBezierRec(scratchData, &nextStart, &nextControl1, &nextControl2, &nextEnd,
				pixelSize, level + 1))
			{
				return false;
			}
		}
		// The end point on the right is already handled by the call before the recursive calls.
	}

	return true;
}

static bool addBezier(dsVectorScratchData* scratchData, const dsVector2f* control1,
	const dsVector2f* control2, const dsVector2f* end, float pixelSize)
{
	if (!inPathWithPoint(scratchData))
		return false;

	dsVector2f start = scratchData->points[scratchData->pointCount - 1].point;
	// Always recurse the first time, since the overall curve may have an inflection point, causing
	// the midpoint metric to break down. Subdivisions won't have any inflection points.
	if (!addBezierRec(scratchData, &start, control1, control2, end, pixelSize, 1))
		return false;

	// Add the last point.
	return dsVectorScratchData_addPoint(scratchData, end, PointType_Corner);
}

static bool addArc(dsVectorScratchData* scratchData, const dsVector2f* end,
	const dsVector2f* radius, float rotation, bool clockwise, bool largeArc, float pixelSize,
	PointType endType)
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
	dsVector2f posPrime, posPrime2;
	dsMatrix22_transformTransposed(posPrime, rotationMat, midPrime);
	dsVector2_mul(posPrime2, posPrime, posPrime);

	dsVector2f radius2;
	dsVector2_mul(radius2, *radius, *radius);
	float centerScale =
		(radius2.x*radius2.y - radius2.x*posPrime2.y - radius2.y*posPrime.x)/
		(radius2.x*posPrime2.y + radius2.y*posPrime2.x);
	if (centerScale < 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"No arc can be fit to the provided parameters.");
		return false;
	}
	centerScale = sqrtf(centerScale);
	if (clockwise == largeArc)
		centerScale = -centerScale;

	dsVector2f centerPrime = {{radius->x*posPrime.y/radius->y,
		-radius->y*posPrime.x/radius->x}};
	dsVector2_scale(centerPrime, centerPrime, centerScale);

	dsVector2f mid, center;
	dsVector2_add(mid, start, *end);
	dsVector2_scale(mid, mid, 0.5f);
	dsMatrix22_transform(center, rotationMat, centerPrime);
	dsVector2_add(center, center, mid);

	float startTheta = acosf((posPrime.x - centerPrime.x)/radius->x);
	if (centerPrime.y > posPrime.y)
		startTheta = -startTheta;

	dsVector2f u, v;
	dsVector2_sub(u, posPrime, centerPrime);
	dsVector2_div(u, u, *radius);
	dsVector2_neg(v, posPrime);
	dsVector2_sub(v, v, centerPrime);
	dsVector2_div(v, v, *radius);

	float deltaTheta = acosf(dsVector2_dot(u, v)/(dsVector2f_len(&u)*dsVector2f_len(&v)));
	if (u.y*v.x > u.x*v.y)
		deltaTheta = -deltaTheta;

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

	if (!dsVectorScratchData_addPoint(scratchData,
		&scratchData->points[scratchData->lastStart].point, pointType | PointType_End))
	{
		return false;
	}

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
	float deltaTheta = (float)(2*M_PI);
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
	float deltaTheta = (float)M_PI_2;
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
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, (float)M_PI_2, incr, pointCount, false,
		rx < halfExtents.x))
	{
		return false;
	}

	// Lower-left
	cornerCenter.y = center.y - halfExtents.y + ry;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, (float)M_PI, incr, pointCount, false,
		ry < halfExtents.y))
	{
		return false;
	}

	// Lower-right
	cornerCenter.x = center.x + halfExtents.x - rx;
	if (!addCorner(scratchData, &cornerCenter, &finalRadius, (float)(M_PI + M_PI_2), incr,
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
	DS_ASSERT(infoIndex <= USHRT_MAX);
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
	vertex->shapeIndex = (uint16_t)infoIndex;
	vertex->padding = 0;

	uint32_t upperRight = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position.x = bounds->max.x;
	vertex->position.y = bounds->min.y;
	vertex->texCoordX = 1;
	vertex->texCoordY = 0;
	vertex->shapeIndex = (uint16_t)infoIndex;
	vertex->padding = 0;

	uint32_t lowerRight = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position = bounds->max;
	vertex->texCoordX = 1;
	vertex->texCoordY = 1;
	vertex->shapeIndex = (uint16_t)infoIndex;
	vertex->padding = 0;

	uint32_t lowerLeft = scratchData->imageVertexCount;
	vertex = dsVectorScratchData_addImageVertex(scratchData);
	if (!vertex)
		return false;
	vertex->position.x = bounds->min.x;
	vertex->position.y = bounds->max.y;
	vertex->texCoordX = 0;
	vertex->texCoordY = 1;
	vertex->shapeIndex = (uint16_t)infoIndex;
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

static bool processCommand(dsVectorScratchData* scratchData, const dsVectorCommand* commands,
	uint32_t commandCount, uint32_t* curCommand, const dsVectorMaterialSet* sharedMaterials,
	dsVectorMaterialSet* localMaterials, float pixelSize)
{
	DS_UNUSED(commandCount);
	DS_ASSERT(*curCommand < commandCount);
	switch (commands[*curCommand].commandType)
	{
		case dsVectorCommandType_StartPath:
			startPath(scratchData, &commands[(*curCommand)++].startPath.transform);
			return true;
		case dsVectorCommandType_Move:
			return moveTo(scratchData, &commands[(*curCommand)++].move.position, PointType_Corner);
		case dsVectorCommandType_Line:
			return lineTo(scratchData, &commands[(*curCommand)++].line.end, PointType_Corner);
		case dsVectorCommandType_Bezier:
		{
			const dsVectorCommandBezier* bezier = &commands[(*curCommand)++].bezier;
			return addBezier(scratchData, &bezier->control1, &bezier->control2, &bezier->end,
				pixelSize);
		}
		case dsVectorCommandType_Quadratic:
		{
			const dsVectorCommandQuadratic* quadratic = &commands[(*curCommand)++].quadratic;
			// Convert quadratic to bezier:
			// https://stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
			const float controlT = 2.0f/3.0f;
			dsVector2f start = scratchData->points[scratchData->pointCount - 1].point;
			dsVector2f control1, control2, temp;
			dsVector2_sub(temp, quadratic->control, start);
			dsVector2_scale(temp, temp, controlT);
			dsVector2_add(control1, start, temp);

			dsVector2_sub(temp, quadratic->control, quadratic->end);
			dsVector2_scale(temp, temp, controlT);
			dsVector2_add(control2, quadratic->end, temp);
			return addBezier(scratchData, &control1, &control2, &quadratic->end,
				adjustPixelSize(&scratchData->pathTransform, pixelSize));
		}
		case dsVectorCommandType_Arc:
		{
			const dsVectorCommandArc* arc = &commands[(*curCommand)++].arc;
			dsVector2f radius = {{fabsf(arc->radius.x), fabsf(arc->radius.y)}};
			return addArc(scratchData, &arc->end, &radius, arc->rotation, arc->clockwise,
				arc->largeArc, adjustPixelSize(&scratchData->pathTransform, pixelSize),
				PointType_Corner);
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
		default:
			DS_ASSERT(false);
			return false;
	}
}

static bool processCommands(dsVectorScratchData* scratchData, const dsVectorCommand* commands,
	uint32_t commandCount, const dsVectorMaterialSet* sharedMaterials,
	dsVectorMaterialSet* localMaterials, float pixelSize)
{
	dsVectorScratchData_reset(scratchData);
	for (uint32_t i = 0; i < commandCount;)
	{
		if (!processCommand(scratchData, commands, commandCount, &i, sharedMaterials,
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
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UScaled);
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

	image->drawGeometries[ShaderType_Shape] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[ShaderType_Shape] != NULL;
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
		dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UInt);
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

	image->drawGeometries[ShaderType_Image] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[ShaderType_Image] != NULL;
}

static bool createTextGeometry(dsVectorImage* image, dsVectorScratchData* scratchData,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (scratchData->textVertexCount == 0)
		return true;

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord1].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UInt);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset ==
		offsetof(TextVertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(TextVertex, texCoords));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord1].offset ==
		offsetof(TextVertex, fillMaterialIndex));
	DS_ASSERT(vertexFormat.size == sizeof(TextVertex));
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

	image->drawGeometries[ShaderType_Text] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[ShaderType_Text] != NULL;
}

static bool createTextTessGeometry(dsVectorImage* image, dsVectorScratchData* scratchData,
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (scratchData->textTessVertexCount == 0)
		return true;

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_Position1].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord1].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UNorm);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position0].offset ==
		offsetof(TextTessVertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position1].offset ==
		offsetof(TextTessVertex, geometry));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(TextTessVertex, texCoords));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord1].offset ==
		offsetof(TextTessVertex, fillMaterialIndex));
	DS_ASSERT(vertexFormat.size == sizeof(TextTessVertex));
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

	image->drawGeometries[ShaderType_Text] = dsDrawGeometry_create(resourceManager, allocator,
		vertexBuffers, &indexBuffer);
	return image->drawGeometries[ShaderType_Text] != NULL;
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

	if (!allocator || !initResources || !initResources->resourceManager ||
		!initResources->scratchData || (!dsVectorImage_testing && !initResources->shaderModule) ||
		!commands || commandCount == 0 || (!initResources->sharedMaterials && !localMaterials) ||
		!size || size->x <= 0.0f || size->y <= 0.0f || pixelSize <= 0.0f)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxFormat infoFormat = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (!dsGfxFormat_textureSupported(initResources->resourceManager, infoFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Floating point textures are required for vector images.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsVectorScratchData* scratchData = initResources->scratchData;
	if (!processCommands(scratchData, commands, commandCount, initResources->sharedMaterials,
		localMaterials, pixelSize))
	{
		return NULL;
	}

	uint32_t infoTextureCount = (scratchData->vectorInfoCount + INFOS_PER_TEXTURE - 1)/
		INFOS_PER_TEXTURE;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVectorImage)) +
		DS_ALIGNED_SIZE(sizeof(dsVectorImagePiece)*scratchData->pieceCount) +
		DS_ALIGNED_SIZE(sizeof(dsTexture*)*infoTextureCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsVectorImage* image = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsVectorImage);
	DS_ASSERT(image);
	memset(image, 0, sizeof(dsVectorImage));
	image->allocator = dsAllocator_keepPointer(allocator);

	dsResourceManager* resourceManager = initResources->resourceManager;
	if (infoTextureCount > 0)
	{
		DS_ASSERT(scratchData->maxVectorInfos % INFOS_PER_TEXTURE == 0);
		dsTextureInfo infoTexInfo = {infoFormat, dsTextureDim_2D, 4, 0, 0, 1, 1};
		image->infoTextures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsTexture*,
			infoTextureCount);
		for (uint32_t i = 0; i < infoTextureCount; ++i, ++image->infoTextureCount)
		{
			infoTexInfo.height = INFOS_PER_TEXTURE;
			if (i == infoTextureCount - 1 &&
				(scratchData->vectorInfoCount % INFOS_PER_TEXTURE) != 0)
			{
				infoTexInfo.height = dsNextPowerOf2(scratchData->vectorInfoCount % INFOS_PER_TEXTURE);
			}

			image->infoTextures[i] = dsTexture_create(resourceManager, resourceAllocator,
				dsTextureUsage_Texture, dsGfxMemory_Static | dsGfxMemory_GpuOnly, &infoTexInfo,
				scratchData->vectorInfos + i*INFOS_PER_TEXTURE,
				sizeof(VectorInfo)*infoTexInfo.height);
			if (!image->infoTextures[i])
			{
				DS_VERIFY(dsVectorImage_destroy(image));
				return NULL;
			}
		}

		image->buffer = dsVectorScratchData_createGfxBuffer(scratchData, resourceManager,
			resourceAllocator);
		if (!image->buffer)
		{
			DS_VERIFY(dsVectorImage_destroy(image));
			return NULL;
		}

		if (!createShapeGeometry(image, scratchData, resourceManager, resourceAllocator) ||
			!createImageGeometry(image, scratchData, resourceManager, resourceAllocator) ||
			!createTextGeometry(image, scratchData, resourceManager, resourceAllocator) ||
			!createTextTessGeometry(image, scratchData, resourceManager, resourceAllocator))
		{
			DS_VERIFY(dsVectorImage_destroy(image));
			return NULL;
		}

		DS_ASSERT(scratchData->pieceCount > 0);
		image->imagePieces = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			dsVectorImagePiece, scratchData->pieceCount);
		DS_ASSERT(image->imagePieces);
		for (uint32_t i = 0; i < scratchData->pieceCount; ++i)
		{
			image->imagePieces[i].geometryInfo =
				image->infoTextures[scratchData->pieces[i].infoTextureIndex];
			image->imagePieces[i].texture = scratchData->pieces[i].texture;
			image->imagePieces[i].type = scratchData->pieces[i].type;
			image->imagePieces[i].range = scratchData->pieces[i].range;
		}
		image->pieceCount = scratchData->pieceCount;
		DS_ASSERT(infoTextureCount > 0);
	}

	image->sharedMaterials = initResources->sharedMaterials;
	image->localMaterials = localMaterials;
	image->size = *size;
	return image;
}

dsVectorImage* dsVectorImage_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const char* filePath, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResources || !initResources->resourceManager ||
		!initResources->scratchData || !initResources->shaderModule ||
		(!initResources->resources && initResources->resourceCount > 0) || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsFileStream fileStream;
	if (!dsFileStream_openPath(&fileStream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector image file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsVectorScratchData_readUntilEnd(&size, initResources->scratchData,
		(dsStream*)&fileStream, initResources->scratchData->allocator);
	dsFileStream_close(&fileStream);
	if (!buffer)
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, filePath);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadStream(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, dsStream* stream, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResources || !initResources->resourceManager ||
		!initResources->scratchData || !initResources->shaderModule ||
		(!initResources->resources && initResources->resourceCount > 0) || !stream)
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
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsVectorImage* image = dsVectorImage_loadImpl(allocator, resourceAllocator, initResources,
		buffer, size, pixelSize, targetSize, NULL);
	DS_PROFILE_FUNC_RETURN(image);
}

dsVectorImage* dsVectorImage_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const void* data, size_t size, float pixelSize,
	const dsVector2f* targetSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !initResources || !initResources->resourceManager ||
		!initResources->scratchData || !initResources->shaderModule ||
		(!initResources->resources && initResources->resourceCount > 0) || !data || size == 0)
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

bool dsVectorImage_draw(const dsVectorImage* vectorImage, dsCommandBuffer* commandBuffer,
	const dsVectorShaders* shaders, dsMaterial* material, const dsMatrix44f* modelViewProjection,
	const dsVolatileMaterialValues* volatileValues, const dsDynamicRenderStates* renderStates)
{
	if (!vectorImage || !commandBuffer || !shaders || !material || !modelViewProjection)
	{
		errno = EINVAL;
		return false;
	}

	if (shaders->shaderModule->materialDesc != dsMaterial_getDescription(material))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
			"Material wasn't created with the same shader module as the shaders.");
		return false;
	}

	if (vectorImage->pieceCount == 0)
		return true;

	dsVectorShaderModule* shaderModule = shaders->shaderModule;
	if (!dsMaterial_setElementData(material, shaderModule->modelViewProjectionElement,
			modelViewProjection, dsMaterialType_Mat4, 0, 1) ||
		!dsMaterial_setElementData(material, shaderModule->sizeElement, &vectorImage->size,
			dsMaterialType_Vec2, 0, 1))
	{
		return false;
	}

	dsTexture* sharedMaterialInfoTexture = dsVectorMaterialSet_getInfoTexture(
		vectorImage->sharedMaterials);
	dsTexture* sharedMaterialColorTexture = dsVectorMaterialSet_getColorTexture(
		vectorImage->sharedMaterials);
	dsTexture* localMaterialInfoTexture = dsVectorMaterialSet_getInfoTexture(
		vectorImage->localMaterials);
	dsTexture* localMaterialColorTexture = dsVectorMaterialSet_getColorTexture(
		vectorImage->localMaterials);
	if (!dsMaterial_setTexture(material, shaderModule->sharedMaterialInfoTextureElement,
			sharedMaterialInfoTexture) ||
		!dsMaterial_setTexture(material, shaderModule->sharedMaterialColorTextureElement,
			sharedMaterialColorTexture) ||
		!dsMaterial_setTexture(material, shaderModule->localMaterialInfoTextureElement,
			localMaterialInfoTexture) ||
		!dsMaterial_setTexture(material, shaderModule->localMaterialColorTextureElement,
			localMaterialColorTexture))
	{
		return false;
	}

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
		const dsVectorImagePiece* piece = vectorImage->imagePieces + i;
		textureSizes.x = (float)piece->geometryInfo->info.height;
		if (!dsMaterial_setElementData(material, shaderModule->textureSizesElement, &textureSizes,
				dsMaterialType_Vec2, 0, 1) ||
			!dsMaterial_setTexture(material, shaderModule->shapeInfoTextureElement,
				piece->geometryInfo) ||
			!dsMaterial_setTexture(material, shaderModule->otherTextureElement, piece->texture))
		{
			success = false;
			break;
		}

		dsShader* shader;
		switch (piece->type)
		{
			case ShaderType_Shape:
				shader = shaders->shapeShader;
				break;
			case ShaderType_Image:
				shader = shaders->imageShader;
				break;
			case ShaderType_Text:
				shader = shaders->textShader;
				break;
			default:
				DS_ASSERT(false);
				shader = NULL;
				break;
		}

		if (!dsShader_bind(shader, commandBuffer, material, volatileValues, renderStates))
		{
			success = false;
			break;
		}
		success = dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer,
				vectorImage->drawGeometries[piece->type], &piece->range);
		// Make sure we unbind the shader even if the above draw failed.
		if (!dsShader_unbind(shader, commandBuffer) || !success)
		{
			success = false;
			break;
		}
	}

	// Clean up textures that might be deleted before the next draw.
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->sharedMaterialInfoTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->sharedMaterialColorTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->localMaterialInfoTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->localMaterialColorTextureElement,
		NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->shapeInfoTextureElement, NULL));
	DS_VERIFY(dsMaterial_setTexture(material, shaderModule->otherTextureElement, NULL));

	return success;
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

	for (uint32_t i = 0; i < vectorImage->infoTextureCount; ++i)
	{
		if (!dsTexture_destroy(vectorImage->infoTextures[i]))
			return false;
	}

	for (int i = 0; i < ShaderType_Count; ++i)
	{
		if (!dsDrawGeometry_destroy(vectorImage->drawGeometries[i]))
			return false;
	}

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
