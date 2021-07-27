/*
 * Copyright 2021 Aaron Barany
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

#include "SceneLightShadowsLoad.h"

#include "Flatbuffers/SceneLightShadows_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>

void* dsSceneLightShadows_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneLightShadowsBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Invalid scene light shadows flatbuffer format.");
		return nullptr;
	}

	auto fbLightShadows = DeepSeaSceneLighting::GetSceneLightShadows(data);

	const char* lightSetName = fbLightShadows->lightSet()->c_str();
	dsSceneResourceType type;
	dsCustomSceneResource* resource;;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData, lightSetName) ||
		type != dsSceneResourceType_Custom || resource->type != dsSceneLightSet_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find light set '%s'.", lightSetName);
		return nullptr;
	}

	auto lightSet = reinterpret_cast<dsSceneLightSet*>(resource->resource);
	auto lightType = static_cast<dsSceneLightType>(fbLightShadows->lightType());

	auto fbLightName = fbLightShadows->light();
	const char* lightName = fbLightName ? fbLightName->c_str() : nullptr;

	const char* transformGroupDescName = fbLightShadows->transformGroupDesc()->c_str();
	dsShaderVariableGroupDesc* transformGroupDesc;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&transformGroupDesc, scratchData,
			transformGroupDescName) || type != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't find shader variable group description '%s'.", lightSetName);
		return nullptr;
	}

	dsSceneShadowParams params =
	{
		fbLightShadows->maxCascades(),
		fbLightShadows->maxFirstSplitDistance(),
		fbLightShadows->cascadedExpFactor(),
		fbLightShadows->fadeStartDistance(),
		fbLightShadows->maxDistance()
	};

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	return dsSceneLightShadows_create(allocator, resourceManager, lightSet, lightType, lightName,
		transformGroupDesc, &params);
}
