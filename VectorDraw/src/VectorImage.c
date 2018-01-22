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
#include "VectorImageImpl.h"
#include "VectorScratchDataImpl.h"
#include "VectorStroke.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
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
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
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
	dsVectorMaterialSet* materials;
	dsVectorImagePiece* imagePieces;
	dsTexture** infoTextures;
	dsDrawGeometry* drawGeometries[ShaderType_Count];
	dsGfxBuffer* buffer;
	uint32_t pieceCount;
	uint32_t infoTextureCount;
	dsVector2f size;
	bool ownMaterials;
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

static bool hasMaterial(const dsVectorMaterialSet* materials, const char* name)
{
	if (!dsVectorMaterialSet_findMaterial(materials, name))
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Couldn't find material '%s'.", name);
		return false;
	}

	return true;
}

/*static dsFont* findFont(const dsVectorResources** resources, uint32_t resourceCount,
	const char* name)
{
	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		dsFont* font = dsVectorResources_findFont(resources[i], name);
		if (font)
			return font;
	}

	return NULL;
}

static dsTexture* findTexture(const dsVectorResources** resources, uint32_t resourceCount,
	const char* name)
{
	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		dsTexture* texture = dsVectorResources_findTexture(resources[i], name);
		if (texture)
			return texture;
	}

	return NULL;
}*/

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
		dsVector2f nextStart = {{leftX.x, leftY.x}};
		dsVector2f nextControl1 = {{leftX.y, leftY.y}};
		dsVector2f nextControl2 = {{leftX.z, leftY.z}};
		dsVector2f nextEnd = {{leftX.w, leftY.w}};
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

	// Target a max arc-length of one pixel.
	float pixelTheta = pixelSize/dsMax(radius->x, radius->y);
	unsigned int pointCount = (unsigned int)(fabsf(deltaTheta)/pixelTheta);
	// Amortize the remainder across all points.
	float incr = deltaTheta/(float)(pointCount + 1);
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

static bool processCommand(dsVectorScratchData* scratchData, const dsVectorCommand* commands,
	uint32_t commandCount, uint32_t* curCommand, dsVectorMaterialSet* materials, float pixelSize)
{
	DS_ASSERT(*curCommand < commandCount);
	switch (commands[*curCommand].commandType)
	{
		case dsVectorCommandType_StartPath:
			scratchData->inPath = true;
			scratchData->pathTransform = commands[*curCommand].startPath.transform;
			scratchData->pointCount = 0;
			scratchData->lastStart = 0;
			++*curCommand;
			return true;
		case dsVectorCommandType_Move:
			if (!inPath(scratchData))
				return false;

			markEnd(scratchData);
			scratchData->lastStart = scratchData->pointCount;
			if (!dsVectorScratchData_addPoint(scratchData, &commands[*curCommand].move.position,
				PointType_Corner))
			{
				return false;
			}

			++*curCommand;
			return true;
		case dsVectorCommandType_Line:
			if (!inPathWithPoint(scratchData) || !dsVectorScratchData_addPoint(scratchData,
				&commands[*curCommand].line.end, PointType_Corner))
			{
				return false;
			}

			++*curCommand;
			return true;
		case dsVectorCommandType_Bezier:
		{
			if (!inPathWithPoint(scratchData))
				return false;

			const dsVectorCommandBezier* bezier = &commands[*curCommand].bezier;
			if (!addBezier(scratchData, &bezier->control1, &bezier->control2, &bezier->end,
				pixelSize))
			{
				return false;
			}

			++*curCommand;
			return true;
		}
		case dsVectorCommandType_Quadratic:
		{
			if (!inPathWithPoint(scratchData))
				return false;

			const dsVectorCommandQuadratic* quadratic = &commands[*curCommand].quadratic;
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
			dsVector2_add(control1, quadratic->end, temp);
			if (!addBezier(scratchData, &control1, &control2, &quadratic->end, pixelSize))
				return false;

			++*curCommand;
			return true;
		}
		case dsVectorCommandType_Arc:
		{
			if (!inPathWithPoint(scratchData))
				return false;

			const dsVectorCommandArc* arc = &commands[*curCommand].arc;
			dsVector2f radius = {{fabsf(arc->radius.x), fabsf(arc->radius.y)}};
			if (!addArc(scratchData, &arc->end, &radius, arc->rotation, arc->clockwise,
				arc->largeArc, pixelSize, PointType_Corner))
			{
				return false;
			}

			++*curCommand;
			return true;
		}
		case dsVectorCommandType_ClosePath:
			if (!inPathWithPoint(scratchData))
				return false;

			if (!dsVectorScratchData_addPoint(scratchData,
				&scratchData->points[scratchData->lastStart].point,
				PointType_Corner | PointType_End))
			{
				return false;
			}

			scratchData->points[scratchData->lastStart].type |= PointType_JoinStart;
			scratchData->lastStart = scratchData->pointCount;
			++*curCommand;
			return true;
		case dsVectorCommandType_StrokePath:
		{
			if (!inPathWithPoint(scratchData))
				return false;

			if (!dsVectorStroke_add(scratchData, materials, &commands[*curCommand].strokePath,
				pixelSize))
			{
				return false;
			}

			++*curCommand;
			return true;
		}
		case dsVectorCommandType_FillPath:
		{
			if (!inPathWithPoint(scratchData))
				return false;

			if (!dsVectorFill_add(scratchData, materials, &commands[*curCommand].fillPath))
				return false;

			++*curCommand;
			return true;
		}
		default:
			DS_ASSERT(false);
			return false;
	}
}

static bool processCommands(dsVectorScratchData* scratchData, const dsVectorCommand* commands,
	uint32_t commandCount, dsVectorMaterialSet* materials, float pixelSize)
{
	dsVectorScratchData_reset(scratchData);
	for (uint32_t i = 0; i < commandCount;)
	{
		if (!processCommand(scratchData, commands, commandCount, &i, materials, pixelSize))
			return false;
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
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UNorm);
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
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord1].format =
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_UNorm);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset ==
		offsetof(ImageVertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(ImageVertex, texCoords));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord1].offset ==
		offsetof(ImageVertex, shapeIndex));
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
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UNorm);
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

dsVectorImage* dsVectorImage_create(dsAllocator* allocator, dsVectorScratchData* scratchData,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const dsVectorCommand* commands, uint32_t commandCount, dsVectorMaterialSet* materials,
	bool ownMaterials, dsVectorShaderModule* shaderModule, const dsVector2f* size, float pixelSize)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !scratchData || !resourceManager || !commands || commandCount == 0 ||
		!materials || (!dsVectorImage_testing && !shaderModule) || !size || size->x <= 0.0f ||
		size->y <= 0.0f || pixelSize <= 0.0f)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxFormat infoFormat = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (!dsGfxFormat_textureSupported(resourceManager, infoFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Floating point textures are required for vector images.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	for (uint32_t i = 0; i < commandCount; ++i)
	{
		switch (commands[i].commandType)
		{
			case dsVectorCommandType_StrokePath:
				if (!hasMaterial(materials, commands[i].strokePath.material))
					DS_PROFILE_FUNC_RETURN(NULL);
				break;
			case dsVectorCommandType_FillPath:
				if (!hasMaterial(materials, commands[i].fillPath.material))
					DS_PROFILE_FUNC_RETURN(NULL);
				break;
			case dsVectorCommandType_TextRange:
				if (commands[i].textRange.fillMaterial &&
					!hasMaterial(materials, commands[i].textRange.fillMaterial))
				{
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				if (commands[i].textRange.outlineMaterial &&
					!hasMaterial(materials, commands[i].textRange.outlineMaterial))
				{
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				break;
			default:
				break;
		}
	}

	if (!processCommands(scratchData, commands, commandCount, materials, pixelSize))
		return NULL;

	uint32_t infoTextureCount = (scratchData->vectorInfoCount + INFOS_PER_TEXTURE - 1)/
		INFOS_PER_TEXTURE;
	uint32_t fullSize = DS_ALIGNED_SIZE(sizeof(dsVectorImage)) +
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

	if (infoTextureCount > 0)
	{
		DS_ASSERT(scratchData->maxVectorInfos % INFOS_PER_TEXTURE == 0);
		image->infoTextures = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsTexture*,
			infoTextureCount);
		for (uint32_t i = 0; i < infoTextureCount; ++i)
		{
			image->infoTextures[i] = dsTexture_create(resourceManager, resourceAllocator,
				dsTextureUsage_Texture, dsGfxMemory_Static | dsGfxMemory_GpuOnly, infoFormat,
				dsTextureDim_2D, 4, INFOS_PER_TEXTURE, 0, 1,
				scratchData->vectorInfos + i*INFOS_PER_TEXTURE,
				sizeof(VectorInfo)*INFOS_PER_TEXTURE);
			if (!image->infoTextures[i])
			{
				for (uint32_t j = i + 1; j < infoTextureCount; ++j)
					image->infoTextures[j] = NULL;
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

	image->materials = materials;
	image->ownMaterials = ownMaterials;
	return image;
}

bool dsVectorImage_destroy(dsVectorImage* vectorImage)
{
	if (!vectorImage)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < vectorImage->infoTextureCount; ++i)
	{
		if (vectorImage->infoTextures[i] && !dsTexture_destroy(vectorImage->infoTextures[i]))
			return false;
	}

	for (int i = 0; i < ShaderType_Count; ++i)
	{
		if (vectorImage->drawGeometries[i] &&
			!dsDrawGeometry_destroy(vectorImage->drawGeometries[i]))
		{
			return false;
		}
	}

	if (vectorImage->buffer && !dsGfxBuffer_destroy(vectorImage->buffer))
		return false;

	if (vectorImage->materials && vectorImage->ownMaterials &&
		!dsVectorMaterialSet_destroy(vectorImage->materials))
	{
		return false;
	}

	if (vectorImage->allocator)
		return dsAllocator_free(vectorImage->allocator, vectorImage);
	return true;
}

dsGfxBuffer* dsVectorImage_getBuffer(dsVectorImage* image)
{
	return image->buffer;
}
