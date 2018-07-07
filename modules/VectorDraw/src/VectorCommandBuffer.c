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

#include <DeepSea/VectorDraw/VectorCommandBuffer.h>

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <float.h>
#include <string.h>

static dsVectorCommand* addCommand(dsVectorCommandBuffer* commandBuffer)
{
	if (!commandBuffer)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, commandBuffer->commands,
		commandBuffer->commandCount, commandBuffer->maxCommands, 1))
	{
		if (errno == EINVAL)
		{
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Command buffer allocator must allow freeing "
				"memory to resize beyond the initial capacity.");
		}
		return NULL;
	}

	return commandBuffer->commands + commandBuffer->commandCount - 1;
}

bool dsVectorCommandBuffer_initialize(dsVectorCommandBuffer* commandBuffer,
	dsAllocator* allocator, uint32_t capacity)
{
	if (!commandBuffer || !allocator)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* commands = NULL;
	if (capacity > 0)
	{
		commands = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsVectorCommand, capacity);
		if (!commands)
			return false;
	}

	commandBuffer->allocator = dsAllocator_keepPointer(allocator);
	commandBuffer->commands = commands;
	commandBuffer->commandCount = 0;
	commandBuffer->maxCommands = capacity;
	return true;
}

bool dsVectorCommandBuffer_addStartPath(dsVectorCommandBuffer* commandBuffer,
	const dsMatrix33f* transform)
{
	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_StartPath;
	if (transform)
		command->startPath.transform = *transform;
	else
		dsMatrix33_identity(command->startPath.transform);
	return true;
}

bool dsVectorCommandBuffer_addMove(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* position)
{
	if (!position)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Move;
	command->move.position = *position;
	return true;
}

bool dsVectorCommandBuffer_addLine(dsVectorCommandBuffer* commandBuffer, const dsVector2f* end)
{
	if (!end)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Line;
	command->line.end = *end;
	return true;
}

bool dsVectorCommandBuffer_addBezier(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* control1, const dsVector2f* control2, const dsVector2f* end)
{
	if (!control1 || !control2 || !end)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Bezier;
	command->bezier.control1 = *control1;
	command->bezier.control2 = *control2;
	command->bezier.end = *end;
	return true;
}

bool dsVectorCommandBuffer_addQuadratic(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* control, const dsVector2f* end)
{
	if (!control || !end)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Quadratic;
	command->quadratic.control = *control;
	command->quadratic.end = *end;
	return true;
}

bool dsVectorCommandBuffer_addArc(dsVectorCommandBuffer* commandBuffer, const dsVector2f* radius,
	float rotation, bool largeArc, bool clockwise, const dsVector2f* end)
{
	if (!radius || !end)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Arc;
	command->arc.radius = *radius;
	command->arc.rotation = rotation;
	command->arc.largeArc = largeArc;
	command->arc.clockwise = clockwise;
	command->arc.end = *end;
	return true;
}

bool dsVectorCommandBuffer_addEllipse(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* center, const dsVector2f* radius)
{
	if (!center || !radius)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Ellipse;
	command->ellipse.center = *center;
	command->ellipse.radius = *radius;
	return true;
}

bool dsVectorCommandBuffer_addRectangle(dsVectorCommandBuffer* commandBuffer,
	const dsAlignedBox2f* bounds, const dsVector2f* cornerRadius)
{
	if (bounds || !dsAlignedBox2_isValid(*bounds))
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Rectangle;
	command->rectangle.bounds = *bounds;
	if (cornerRadius)
		command->rectangle.cornerRadius = *cornerRadius;
	else
	{
		command->rectangle.cornerRadius.x = 0;
		command->rectangle.cornerRadius.y = 0;
	}
	return true;
}

bool dsVectorCommandBuffer_addStrokePath(dsVectorCommandBuffer* commandBuffer, const char* material,
	float opacity, dsLineJoin joinType, dsLineCap capType, float width, float miterLimit,
	const dsVector4f* dashArray)
{
	if (!material || width <= 0.0f || miterLimit < 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_StrokePath;
	command->strokePath.material = material;
	command->strokePath.opacity = opacity;
	command->strokePath.joinType = joinType;
	command->strokePath.capType = capType;
	command->strokePath.width = width;
	command->strokePath.miterLimit = miterLimit;
	if (dashArray)
		command->strokePath.dashArray = *dashArray;
	else
		memset(&command->strokePath.dashArray, 0, sizeof(dsVector4f));
	return true;
}

bool dsVectorCommandBuffer_addText(dsVectorCommandBuffer* commandBuffer, const void* string,
	dsUnicodeType stringType, dsFont* font, dsTextJustification justification,
	const dsMatrix33f* transform, uint32_t rangeCount)
{
	if (!font)
	{
		errno = EINVAL;
		return false;
	}

	if (rangeCount == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"At least one range must follow a text path command.");
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Text;
	command->text.string = string;
	command->text.stringType = stringType;
	command->text.font = font;
	command->text.justification = justification;
	if (transform)
		command->text.transform = *transform;
	else
		dsMatrix33_identity(command->text.transform);
	command->text.rangeCount = rangeCount;
	return true;
}

bool dsVectorCommandBuffer_addTextRange(dsVectorCommandBuffer* commandBuffer, uint32_t start,
	uint32_t count, dsVectorTextPosition positionType, const dsVector2f* position,
	const char* fillMaterial, const char* outlineMaterial, float fillOpacity, float outlineOpacity,
	float size, float embolden, float slant, float outlineWidth)
{
	if ((positionType != dsVectorTextPosition_None && !position) ||
		(!fillMaterial && !outlineMaterial))
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_TextRange;
	command->textRange.start = start;
	command->textRange.count = count;
	command->textRange.positionType = positionType;
	if (command->textRange.positionType == dsVectorTextPosition_None)
	{
		command->textRange.position.x = FLT_MAX;
		command->textRange.position.y = FLT_MAX;
	}
	else
		command->textRange.position = *position;
	command->textRange.fillMaterial = fillMaterial;
	command->textRange.outlineMaterial = outlineMaterial;
	command->textRange.fillOpacity = fillOpacity;
	command->textRange.outlineOpacity = outlineOpacity;
	command->textRange.size = size;
	command->textRange.embolden = embolden;
	command->textRange.slant = slant;
	command->textRange.outlineWidth = outlineWidth;
	return true;
}

bool dsVectorCommandBuffer_addImage(dsVectorCommandBuffer* commandBuffer, dsTexture* image,
	const dsAlignedBox2f* imageBounds, float opacity, const dsMatrix33f* transform)
{
	if (!image || !imageBounds)
	{
		errno = EINVAL;
		return false;
	}

	dsVectorCommand* command = addCommand(commandBuffer);
	if (!command)
		return false;

	command->commandType = dsVectorCommandType_Image;
	command->image.image = image;
	command->image.imageBounds = *imageBounds;
	command->image.opacity = opacity;
	if (transform)
		command->image.transform = *transform;
	else
		dsMatrix33_identity(command->image.transform);
	return true;
}

void dsVectorCommandBuffer_shutdown(dsVectorCommandBuffer* commandBuffer)
{
	if (commandBuffer && commandBuffer->allocator && commandBuffer->commands)
		dsAllocator_free(commandBuffer->allocator, commandBuffer->commands);
}
