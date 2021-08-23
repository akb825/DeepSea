/*
 * Copyright 2020 Aaron Barany
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

#include "SceneLightSetLoad.h"

#include "Flatbuffers/SceneLightSet_generated.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLight.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

void* dsSceneLightSet_load(const dsSceneLoadContext*, dsSceneLoadScratchData*,
	dsAllocator* allocator, dsAllocator*, void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneLightSetBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Invalid scene light set flatbuffer format.");
		return nullptr;
	}

	auto fbLightSet = DeepSeaSceneLighting::GetSceneLightSet(data);
	auto fbLights = fbLightSet->lights();
	uint32_t maxLights = fbLightSet->maxLights();
	if (maxLights == 0)
		maxLights = fbLights->size();
	if (maxLights == 0)
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Scene light set has no maximum lights.");
		return nullptr;
	}
	else if (fbLights && maxLights < fbLights->size())
	{
		errno = EFORMAT;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene light set's maximum lights (%u) is too small to hold the initial lights (%u).",
			maxLights, fbLights->size());
		return nullptr;
	}

	dsColor3f ambientColor = {{0.0f, 0.0f, 0.0f}};
	auto fbAmbientColor = fbLightSet->ambientColor();
	if (fbAmbientColor)
		ambientColor = DeepSeaScene::convert(*fbAmbientColor);

	dsSceneLightSet* lightSet = dsSceneLightSet_create(allocator, maxLights, &ambientColor,
		fbLightSet->ambientIntensity());
	if (!lightSet)
		return nullptr;

	if (fbLights)
	{
		uint32_t lightCount = fbLights->size();
		for (uint32_t i = 0; i < lightCount; ++i)
		{
			auto fbLight = (*fbLights)[i];
			if (!fbLight)
				continue;

			const char* name = fbLight->name()->c_str();
			dsSceneLight* light = dsSceneLightSet_addLightName(lightSet, name);
			if (!light)
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
					"Light '%s' is present multiple times in scene light set.", name);
				dsSceneLightSet_destroy(lightSet);
				return nullptr;
			}

			if (auto fbDirectionalLight = fbLight->light_as_DirectionalLight())
			{
				dsVector3f direction = DeepSeaScene::convert(*fbDirectionalLight->direction());
				dsVector3f_normalize(&direction, &direction);
				dsColor3f color = DeepSeaScene::convert(*fbDirectionalLight->color());
				DS_VERIFY(dsSceneLight_makeDirectional(light, &direction, &color,
					fbDirectionalLight->intensity()));
			}
			else if (auto fbPointLight = fbLight->light_as_PointLight())
			{
				dsVector3f position = DeepSeaScene::convert(*fbPointLight->position());
				dsColor3f color = DeepSeaScene::convert(*fbPointLight->color());
				if (!dsSceneLight_makePoint(light, &position, &color,
						fbPointLight->intensity(), fbPointLight->linearFalloff(),
						fbPointLight->quadraticFalloff()))
				{
					errno = EFORMAT;
					DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
						"Invalid point light '%s' in scene light set.", name);
					dsSceneLightSet_destroy(lightSet);
					return nullptr;
				}
			}
			else if (auto fbSpotLight = fbLight->light_as_SpotLight())
			{
				dsVector3f position = DeepSeaScene::convert(*fbSpotLight->position());
				dsVector3f direction = DeepSeaScene::convert(*fbSpotLight->direction());
				dsVector3f_normalize(&direction, &direction);
				float cosInnerSpotAngle = cosf(fbSpotLight->innerSpotAngle());
				float cosOuterSpotAngle = cosf(fbSpotLight->outerSpotAngle());
				dsColor3f color = DeepSeaScene::convert(*fbSpotLight->color());
				if (!dsSceneLight_makeSpot(light, &position, &direction, &color,
						fbSpotLight->intensity(), fbSpotLight->linearFalloff(),
						fbSpotLight->quadraticFalloff(), cosInnerSpotAngle, cosOuterSpotAngle))
				{
					errno = EFORMAT;
					DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
						"Invalid spotlight '%s' in scene light set.", name);
					dsSceneLightSet_destroy(lightSet);
					return nullptr;
				}
			}
			else
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Invalid light '%s' in scene light set.",
					name);
				dsSceneLightSet_destroy(lightSet);
				return nullptr;
			}
		}
	}

	auto fbMainLight = fbLightSet->mainLight();
	if (fbMainLight)
		DS_VERIFY(dsSceneLightSet_setMainLightName(lightSet, fbMainLight->c_str()));

	return lightSet;
}
