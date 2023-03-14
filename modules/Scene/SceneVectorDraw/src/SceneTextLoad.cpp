/*
 * Copyright 2020-2022 Aaron Barany
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

#include "SceneTextLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>

#include <DeepSea/SceneVectorDraw/SceneText.h>
#include <DeepSea/SceneVectorDraw/SceneVectorResources.h>

#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Text.h>
#include <DeepSea/Text/TextSubstitutionTable.h>

#include <DeepSea/VectorDraw/VectorResources.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/SceneText_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

void* dsSceneText_load(const dsSceneLoadContext*, dsSceneLoadScratchData* scratchData,
	dsAllocator* allocator, dsAllocator*, void* userData, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifySceneTextBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Invalid scene text flatbuffer format.");
		return nullptr;
	}

	auto sceneTextUserData = reinterpret_cast<SceneTextUserData*>(userData);
	auto fbSceneText = DeepSeaSceneVectorDraw::GetSceneText(data);

	auto fbFont = fbSceneText->font();
	dsSceneResourceType resourceType;
	dsCustomSceneResource* fontResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&fontResource),
			scratchData, fbFont->resources()->c_str()) ||
		resourceType != dsSceneResourceType_Custom ||
		fontResource->type != dsSceneVectorResources_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find vector resources '%s'.",
			fbFont->resources()->c_str());
		return nullptr;
	}

	dsFont* font = dsVectorResources_findFont(
		reinterpret_cast<dsVectorResources*>(fontResource->resource), fbFont->name()->c_str());
	if (!font)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG,
			"Couldn't find font '%s' for vector resources '%s'.",
			fbFont->name()->c_str(), fbFont->resources()->c_str());
		return nullptr;
	}

	auto fbStyles = fbSceneText->styles();
	uint32_t styleCount = fbStyles->size();
	dsTextStyle* styles = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsTextStyle, styleCount);
	for (uint32_t i = 0; i < styleCount; ++i)
	{
		auto fbStyle = (*fbStyles)[i];
		dsTextStyle& style = styles[i];
		style.start = fbStyle->start();
		style.count = fbStyle->count();
		style.scale = fbStyle->size();
		style.embolden = fbStyle->embolden();
		style.slant = fbStyle->slant();
		style.outlineThickness = 0.5f + style.embolden*0.5f;
		style.outlineThickness = fbStyle->outlineWidth();
		DS_VERIFY(dsFont_applyHintingAndAntiAliasing(font, &style, sceneTextUserData->pixelScale,
			fbStyle->fuziness()));

		auto fbColor = fbStyle->color();
		if (fbColor)
		{
			style.color.r = fbColor->red();
			style.color.g = fbColor->green();
			style.color.b = fbColor->blue();
			style.color.a = fbColor->alpha();
		}
		else
		{
			style.color.r = 0;
			style.color.g = 0;
			style.color.b = 0;
			style.color.a = 0;
		}

		fbColor = fbStyle->outlineColor();
		if (fbColor)
		{
			style.outlineColor.r = fbColor->red();
			style.outlineColor.g = fbColor->green();
			style.outlineColor.b = fbColor->blue();
			style.outlineColor.a = fbColor->alpha();
		}
		else
			style.outlineColor = style.color;

		style.verticalOffset = fbStyle->verticalOffset();
	}

	const char* string = fbSceneText->text()->c_str();
	if (sceneTextUserData)
	{
		string = dsTextSubstitutionTable_substitute(sceneTextUserData->substitutionTable,
			sceneTextUserData->substitutionData, string, styles, styleCount);
		if (!string)
			return nullptr;
	}

	dsText* text = dsText_createUTF8(font, allocator, string, false);
	if (!text)
		return nullptr;

	return dsSceneText_create(allocator, text, nullptr, styles, styleCount);
}
