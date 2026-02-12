/*
 * Copyright 2018-2026 Aaron Barany
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

#include "VectorScratchDataImpl.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector2.h>

#include <DeepSea/Render/Resources/ResourceManager.h>

#include <DeepSea/Text/Font.h>

#include <DeepSea/VectorDraw/Gradient.h>
#include <DeepSea/VectorDraw/VectorMaterial.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorResources.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/VectorImage_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

static void printFlatbufferError(const char* name)
{
	if (name)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Invalid vector image flatbuffer format for '%s'.", name);
	}
	else
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Invalid vector image flatbuffer format.");
}

static dsGradient* readGradient(dsAllocator* allocator,
	const flatbuffers::Vector<const DeepSeaVectorDraw::GradientStop*>& stopArray, const char* name)
{
	uint32_t stopCount = stopArray.size();
	if (stopCount == 0)
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	auto gradientStops = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsGradientStop, stopCount);
	for (uint32_t i = 0; i < stopCount; ++i)
	{
		auto stopRef = stopArray[i];
		if (!stopRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			return nullptr;
		}

		gradientStops[i].position = stopRef->position();
		gradientStops[i].color = reinterpret_cast<const dsColor&>(stopRef->color());
	}

	return dsGradient_create(allocator, gradientStops, stopCount);
}

static bool readMaterials(dsVectorMaterialSet** outMaterials, dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const DeepSeaVectorDraw::VectorImage* fbVectorImage, bool srgb, const char* name)
{
	auto colorMaterials = fbVectorImage->colorMaterials();
	auto linearGradients = fbVectorImage->linearGradients();
	auto radialGradients = fbVectorImage->radialGradients();

	uint32_t colorMaterialCount = 0, linearGradientCount = 0, radialGradientCount = 0;
	if (colorMaterials)
		colorMaterialCount = colorMaterials->size();
	if (linearGradients)
		linearGradientCount = linearGradients->size();
	if (radialGradients)
		radialGradientCount = radialGradients->size();
	uint32_t totalMaterialCount = colorMaterialCount + linearGradientCount + radialGradientCount;
	if (totalMaterialCount == 0)
	{
		*outMaterials = nullptr;
		return true;
	}

	dsVectorMaterialSet* materials = dsVectorMaterialSet_create(allocator, resourceManager,
		resourceAllocator, totalMaterialCount, srgb);
	if (!materials)
		return false;

	for (uint32_t i = 0; i < colorMaterialCount; ++i)
	{
		auto colorRef = (*colorMaterials)[i];
		if (!colorRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			dsVectorMaterialSet_destroy(materials);
			return false;
		}

		dsVectorMaterial material;
		DS_VERIFY(dsVectorMaterial_setColor(&material,
			*reinterpret_cast<const dsColor*>(colorRef->color())));
		if (!dsVectorMaterialSet_addMaterial(materials, colorRef->name()->c_str(), &material, true))
		{
			dsVectorMaterialSet_destroy(materials);
			return false;
		}
	}

	for (uint32_t i = 0; i < linearGradientCount; ++i)
	{
		auto linearGradientRef = (*linearGradients)[i];
		if (!linearGradientRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			dsVectorMaterialSet_destroy(materials);
			return false;
		}

		dsGradient* gradient = readGradient(allocator, *linearGradientRef->gradient(), name);
		if (!gradient)
		{
			dsVectorMaterialSet_destroy(materials);
			return false;
		}

		dsVectorMaterial material;
		DS_VERIFY(dsVectorMaterial_setLinearGradient(&material, gradient,
			reinterpret_cast<const dsVector2f*>(linearGradientRef->start()),
			reinterpret_cast<const dsVector2f*>(linearGradientRef->end()),
			static_cast<dsGradientEdge>(linearGradientRef->edge()),
			static_cast<dsVectorMaterialSpace>(linearGradientRef->coordinateSpace()),
			reinterpret_cast<const dsMatrix33f*>(linearGradientRef->transform())));
		if (!dsVectorMaterialSet_addMaterial(materials, linearGradientRef->name()->c_str(),
			&material, true))
		{
			dsGradient_destroy(gradient);
			dsVectorMaterialSet_destroy(materials);
			return false;
		}
	}

	for (uint32_t i = 0; i < radialGradientCount; ++i)
	{
		auto radialGradientRef = (*radialGradients)[i];
		if (!radialGradientRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			dsVectorMaterialSet_destroy(materials);
			return false;
		}

		dsGradient* gradient = readGradient(allocator, *radialGradientRef->gradient(), name);
		if (!gradient)
		{
			dsVectorMaterialSet_destroy(materials);
			return false;
		}

		dsVectorMaterial material;
		DS_VERIFY(dsVectorMaterial_setRadialGradient(&material, gradient,
			reinterpret_cast<const dsVector2f*>(radialGradientRef->center()),
			radialGradientRef->radius(),
			reinterpret_cast<const dsVector2f*>(radialGradientRef->focus()),
			radialGradientRef->focusRadius(),
			static_cast<dsGradientEdge>(radialGradientRef->edge()),
			static_cast<dsVectorMaterialSpace>(radialGradientRef->coordinateSpace()),
			reinterpret_cast<const dsMatrix33f*>(radialGradientRef->transform())));
		if (!dsVectorMaterialSet_addMaterial(materials, radialGradientRef->name()->c_str(),
				&material, true))
		{
			dsGradient_destroy(gradient);
			dsVectorMaterialSet_destroy(materials);
			return false;
		}
	}

	*outMaterials = materials;
	return true;
}

static dsFont* findFont(dsVectorResources* const* resources, uint32_t resourceCount,
	const char* fontName, const char* name)
{
	if (resources)
	{
		for (uint32_t i = 0; i < resourceCount; ++i)
		{
			dsVectorResources* curResources = resources[i];
			if (!curResources)
				continue;

			dsVectorResourceType resourceType;
			dsFont* font;
			if (dsVectorResources_findResource(
					&resourceType, (void**)&font, curResources, fontName) &&
				resourceType == dsVectorResourceType_Font)
			{
				return font;
			}
		}
	}

	errno = ENOTFOUND;
	if (name)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Font '%s' isn't present in vector resources for vector image '%s'.", fontName,
			name);
	}
	else
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Font '%s' isn't present in vector resources for vector image.", fontName);
	}
	return nullptr;
}

static dsTexture* findTexture(dsVectorResources* const* resources, uint32_t resourceCount,
	const char* textureName, const char* name)
{
	if (resources)
	{
		for (uint32_t i = 0; i < resourceCount; ++i)
		{
			dsVectorResources* curResources = resources[i];
			if (!curResources)
				continue;

			dsVectorResourceType resourceType;
			dsTexture* texture;
			if (dsVectorResources_findResource(
					&resourceType, (void**)&texture, curResources, textureName) &&
				resourceType == dsVectorResourceType_Texture)
			{
				return texture;
			}
		}
	}

	errno = ENOTFOUND;
	if (name)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Texture '%s' isn't present in vector resources for vector image '%s'.",
			textureName, name);
	}
	else
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Texture '%s' isn't present in vector resources for vector image.", textureName);
	}
	return nullptr;
}

static dsVectorImage* readVectorImage(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources,
	const DeepSeaVectorDraw::VectorImage* fbVectorImage, dsVectorMaterialSet* localMaterials,
	float pixelSize, const dsVector2f* targetSize, const char* name)
{
	auto commandList = fbVectorImage->commands();
	uint32_t commandCount = commandList->size();
	if (commandCount == 0)
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	dsVectorCommand* commands = dsVectorScratchData_createTempCommands(
		initResources->scratchData, commandCount);
	if (!commands)
		return nullptr;

	for (uint32_t i = 0; i < commandCount; ++i)
	{
		auto commandRef = (*commandList)[i];
		if (!commandRef)
		{
			errno = EFORMAT;
			printFlatbufferError(name);
			return nullptr;
		}

		dsVectorCommand* command = commands + i;
		switch (commandRef->command_type())
		{
			case DeepSeaVectorDraw::VectorCommandUnion::StartPathCommand:
			{
				auto startCommand = commandRef->command_as_StartPathCommand();
				command->commandType = dsVectorCommandType_StartPath;
				command->startPath.transform =
					*reinterpret_cast<const dsMatrix33f*>(startCommand->transform());
				command->startPath.simple = startCommand->simple();
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::MoveCommand:
			{
				auto moveCommand = commandRef->command_as_MoveCommand();
				command->commandType = dsVectorCommandType_Move;
				command->move.position =
					*reinterpret_cast<const dsVector2f*>(moveCommand->position());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::LineCommand:
			{
				auto lineCommand = commandRef->command_as_LineCommand();
				command->commandType = dsVectorCommandType_Line;
				command->line.end = *reinterpret_cast<const dsVector2f*>(lineCommand->end());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::BezierCommand:
			{
				auto bezierCommand = commandRef->command_as_BezierCommand();
				command->commandType = dsVectorCommandType_Bezier;
				command->bezier.control1 =
					*reinterpret_cast<const dsVector2f*>(bezierCommand->control1());
				command->bezier.control2 =
					*reinterpret_cast<const dsVector2f*>(bezierCommand->control2());
				command->bezier.end = *reinterpret_cast<const dsVector2f*>(bezierCommand->end());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::QuadraticCommand:
			{
				auto quadraticCommand = commandRef->command_as_QuadraticCommand();
				command->commandType = dsVectorCommandType_Quadratic;
				command->quadratic.control =
					*reinterpret_cast<const dsVector2f*>(quadraticCommand->control());
				command->quadratic.end =
					*reinterpret_cast<const dsVector2f*>(quadraticCommand->end());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::ArcCommand:
			{
				auto arcCommand = commandRef->command_as_ArcCommand();
				command->commandType = dsVectorCommandType_Arc;
				command->arc.radius = *reinterpret_cast<const dsVector2f*>(arcCommand->radius());
				command->arc.rotation = arcCommand->rotation();
				command->arc.largeArc = arcCommand->largeArc();
				command->arc.clockwise = arcCommand->clockwise();
				command->arc.end = *reinterpret_cast<const dsVector2f*>(arcCommand->end());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::ClosePathCommand:
				command->commandType = dsVectorCommandType_ClosePath;
				break;
			case DeepSeaVectorDraw::VectorCommandUnion::EllipseCommand:
			{
				auto ellipseCommand = commandRef->command_as_EllipseCommand();
				command->commandType = dsVectorCommandType_Ellipse;
				command->ellipse.center =
					*reinterpret_cast<const dsVector2f*>(ellipseCommand->center());
				command->ellipse.radius =
					*reinterpret_cast<const dsVector2f*>(ellipseCommand->radius());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::RectangleCommand:
			{
				auto rectangleCommand = commandRef->command_as_RectangleCommand();
				command->commandType = dsVectorCommandType_Rectangle;
				command->rectangle.bounds.min =
					*reinterpret_cast<const dsVector2f*>(rectangleCommand->upperLeft());
				command->rectangle.bounds.max =
					*reinterpret_cast<const dsVector2f*>(rectangleCommand->lowerRight());
				command->rectangle.cornerRadius =
					*reinterpret_cast<const dsVector2f*>(rectangleCommand->cornerRadius());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::StrokePathCommand:
			{
				auto strokePathCommand = commandRef->command_as_StrokePathCommand();
				command->commandType = dsVectorCommandType_StrokePath;
				command->strokePath.material = strokePathCommand->material()->c_str();
				command->strokePath.opacity = strokePathCommand->opacity();
				command->strokePath.joinType =
					static_cast<dsLineJoin>(strokePathCommand->joinType());
				command->strokePath.capType =
					static_cast<dsLineCap>(strokePathCommand->capType());
				command->strokePath.width = strokePathCommand->width();
				command->strokePath.miterLimit = strokePathCommand->miterLimit();
				// dsVector4f is aligned, so need to memcpy rather than cast to copy.
				std::memcpy(&command->strokePath.dashArray, strokePathCommand->dashArray(),
					sizeof(dsVector4f));
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::FillPathCommand:
			{
				auto fillPathCommand = commandRef->command_as_FillPathCommand();
				command->commandType = dsVectorCommandType_FillPath;
				command->fillPath.material = fillPathCommand->material()->c_str();
				command->fillPath.opacity = fillPathCommand->opacity();
				command->fillPath.fillRule =
					static_cast<dsPolygonFillRule>(fillPathCommand->fillRule());
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::TextCommand:
			{
				auto textCommand = commandRef->command_as_TextCommand();
				command->commandType = dsVectorCommandType_Text;
				command->text.string = textCommand->text()->c_str();
				command->text.stringType = dsUnicodeType_UTF8;
				command->text.font = findFont(initResources->resources,
					initResources->resourceCount, textCommand->font()->c_str(), name);
				if (!command->text.font)
					return nullptr;
				command->text.alignment =
					static_cast<dsTextAlign>(textCommand->alignment());
				command->text.maxLength = textCommand->maxLength();
				command->text.lineHeight = textCommand->lineHeight();
				command->text.transform =
					*reinterpret_cast<const dsMatrix33f*>(textCommand->transform());
				command->text.rangeCount = textCommand->rangeCount();
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::TextRangeCommand:
			{
				auto textRangeCommand = commandRef->command_as_TextRangeCommand();
				command->commandType = dsVectorCommandType_TextRange;
				command->textRange.start = textRangeCommand->start();
				command->textRange.count = textRangeCommand->count();
				command->textRange.positionType =
					static_cast<dsVectorTextPosition>(textRangeCommand->positionType());
				command->textRange.position =
					*reinterpret_cast<const dsVector2f*>(textRangeCommand->position());
				auto fillMaterial = textRangeCommand->fillMaterial();
				command->textRange.fillMaterial = fillMaterial ? fillMaterial->c_str() : nullptr;
				auto outlineMaterial = textRangeCommand->outlineMaterial();
				command->textRange.outlineMaterial =
					outlineMaterial ? outlineMaterial->c_str() : nullptr;
				command->textRange.fillOpacity = textRangeCommand->fillOpacity();
				command->textRange.outlineOpacity = textRangeCommand->outlineOpacity();
				command->textRange.size = textRangeCommand->size();
				command->textRange.embolden = textRangeCommand->embolden();
				command->textRange.slant = textRangeCommand->slant();
				command->textRange.outlineWidth = textRangeCommand->outlineWidth();
				command->textRange.fuziness = textRangeCommand->fuziness();
				break;
			}
			case DeepSeaVectorDraw::VectorCommandUnion::ImageCommand:
			{
				auto imageCommand = commandRef->command_as_ImageCommand();
				command->commandType = dsVectorCommandType_Image;
				command->image.image = findTexture(initResources->resources,
					initResources->resourceCount, imageCommand->image()->c_str(), name);
				if (!command->image.image)
					return nullptr;
				command->image.imageBounds.min =
					*reinterpret_cast<const dsVector2f*>(imageCommand->upperLeft());
				command->image.imageBounds.max =
					*reinterpret_cast<const dsVector2f*>(imageCommand->lowerRight());
				command->image.opacity = imageCommand->opacity();
				command->image.transform =
					*reinterpret_cast<const dsMatrix33f*>(imageCommand->transform());
				break;
			}
			default:
				errno = EFORMAT;
				printFlatbufferError(name);
				return nullptr;
		}
	}

	// If target size is set, adjust the pixel size by the target size relative to the original.
	auto size = reinterpret_cast<const dsVector2f*>(fbVectorImage->size());
	if (targetSize)
	{
		dsVector2f scale;
		dsVector2_div(scale, *size, *targetSize);
		// Find the smallest change in size to reach the target.
		float scaleXMag = scale.x > 1.0f ? scale.x : 1.0f/scale.x;
		float scaleYMag = scale.y > 1.0f ? scale.y : 1.0f/scale.y;
		if (scaleXMag < scaleYMag)
			pixelSize *= scale.x;
		else
			pixelSize *= scale.y;
	}

	return dsVectorImage_create(allocator, resourceAllocator, initResources,
		commands, commandCount, localMaterials, size, pixelSize);
}

extern "C"
dsVectorImage* dsVectorImage_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsVectorImageInitResources* initResources, const void* data, size_t size, float pixelSize,
	const dsVector2f* targetSize, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaVectorDraw::VerifyVectorImageBuffer(verifier))
	{
		errno = EFORMAT;
		printFlatbufferError(name);
		return nullptr;
	}

	auto fbVectorImage = DeepSeaVectorDraw::GetVectorImage(data);
	dsVectorMaterialSet* localMaterials = NULL;
	if (!readMaterials(&localMaterials, allocator, initResources->resourceManager,
			resourceAllocator, fbVectorImage, initResources->srgb, name))
	{
		return nullptr;
	}

	if (localMaterials)
	{
		dsCommandBuffer* commandBuffer = initResources->commandBuffer;
		if (!commandBuffer)
		{
			commandBuffer =
				dsResourceManager_getResourceCommandBuffer(initResources->resourceManager);
			if (!commandBuffer)
			{
				DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG,
					"Vector image loaded without a command buffer or resource context acquired.");
				dsVectorMaterialSet_destroy(localMaterials);
				return nullptr;
			}
		}

		if (!dsVectorMaterialSet_update(localMaterials, commandBuffer))
		{
			dsVectorMaterialSet_destroy(localMaterials);
			return nullptr;
		}
	}

	dsVectorImage* vectorImage = readVectorImage(allocator, resourceAllocator, initResources,
		fbVectorImage, localMaterials, pixelSize, targetSize, name);
	if (!vectorImage)
	{
		dsVectorMaterialSet_destroy(localMaterials);
		return nullptr;
	}

	return vectorImage;
}
