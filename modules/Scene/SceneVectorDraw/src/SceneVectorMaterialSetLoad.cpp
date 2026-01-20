/*
 * Copyright 2020-2026 Aaron Barany
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

#include "SceneVectorMaterialSetLoad.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/VectorDraw/Gradient.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneVectorMaterialSet_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

static bool convertStops(dsAllocator* allocator, dsGradientStop*& tempStops, uint32_t& stopCount,
	uint32_t& maxStops,
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>* fbStops)
{
	stopCount = 0;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, tempStops, stopCount, maxStops, fbStops->size()))
		return false;

	for (uint32_t i = 0; i < stopCount; ++i)
	{
		auto fbStop = (*fbStops)[i];
		if (!fbStop)
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
				"Vector material set gradient stop is unset.");
			return false;
		}

		dsGradientStop& stop = tempStops[i];
		stop.position = fbStop->position();
		auto fbColor = fbStop->color();
		stop.color.r = fbColor->red();
		stop.color.g = fbColor->green();
		stop.color.b = fbColor->blue();
		stop.color.a = fbColor->alpha();
	}

	return true;
}

void* dsVectorSceneMaterialSet_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize, void*,
	dsOpenRelativePathStreamFunction, dsCloseRelativePathStreamFunction)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyVectorMaterialSetBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
			"Invalid vector scene material set flatbuffer format.");
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	auto fbVectorMaterialSet = DeepSeaSceneVectorDraw::GetVectorMaterialSet(data);
	auto fbMaterials = fbVectorMaterialSet->materials();

	uint32_t materialCount = fbMaterials->size();
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create(allocator, resourceManager,
		resourceAllocator, materialCount, fbVectorMaterialSet->srgb());
	if (!materialSet)
		return nullptr;

	dsGradientStop* tempStops = nullptr;
	uint32_t tempStopCount = 0;
	uint32_t maxTempStops = 0;

	for (uint32_t i = 0; i < materialCount; ++i)
	{
		auto fbMaterial = (*fbMaterials)[i];
		if (!fbMaterial)
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Vector material is unset.");
			goto error;
		}

		dsVectorMaterial material;
		if (auto fbColor = fbMaterial->value_as_ColorTable())
		{
			material.materialType = dsVectorMaterialType_Color;
			material.color.r = fbColor->red();
			material.color.g = fbColor->green();
			material.color.b = fbColor->blue();
			material.color.a = fbColor->alpha();
		}
		else if (auto fbLinearGradient = fbMaterial->value_as_LinearGradient())
		{
			material.materialType = dsVectorMaterialType_LinearGradient;
			dsLinearGradient& linearGradient = material.linearGradient;
			if (!convertStops(scratchAllocator, tempStops, tempStopCount, maxTempStops,
					fbLinearGradient->stops()))
			{
				goto error;
			}

			linearGradient.gradient = dsGradient_create(allocator, tempStops, tempStopCount);
			if (!linearGradient.gradient)
				goto error;

			linearGradient.start = DeepSeaScene::convert(*fbLinearGradient->start());
			linearGradient.end = DeepSeaScene::convert(*fbLinearGradient->end());
			linearGradient.edge = static_cast<dsGradientEdge>(fbLinearGradient->edge());
			linearGradient.coordinateSpace =
				static_cast<dsVectorMaterialSpace>(fbLinearGradient->coordinateSpace());
			linearGradient.transform = DeepSeaScene::convert(*fbLinearGradient->transform());
		}
		else if (auto fbRadialGradient = fbMaterial->value_as_RadialGradient())
		{
			material.materialType = dsVectorMaterialType_RadialGradient;
			dsRadialGradient& radialGradient = material.radialGradient;
			if (!convertStops(scratchAllocator, tempStops, tempStopCount, maxTempStops,
					fbRadialGradient->stops()))
			{
				goto error;
			}

			radialGradient.gradient = dsGradient_create(allocator, tempStops, tempStopCount);
			if (!radialGradient.gradient)
				goto error;

			radialGradient.center = DeepSeaScene::convert(*fbRadialGradient->center());
			radialGradient.radius = fbRadialGradient->radius();
			radialGradient.focus = DeepSeaScene::convert(*fbRadialGradient->focus());
			radialGradient.focusRadius = fbRadialGradient->focusRadius();
			radialGradient.edge = static_cast<dsGradientEdge>(fbRadialGradient->edge());
			radialGradient.coordinateSpace =
				static_cast<dsVectorMaterialSpace>(fbRadialGradient->coordinateSpace());
			radialGradient.transform = DeepSeaScene::convert(*fbRadialGradient->transform());
		}
		else
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Vector material is unset.");
			goto error;
		}

		if (!dsVectorMaterialSet_addMaterial(
				materialSet, fbMaterial->name()->c_str(), &material, true))
		{
			goto error;
		}
	}

	DS_VERIFY(dsAllocator_free(scratchAllocator, tempStops));
	return materialSet;

error:
	dsVectorMaterialSet_destroy(materialSet);
	DS_VERIFY(dsAllocator_free(scratchAllocator, tempStops));
	return nullptr;
}
