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

#include "SceneShadowInstanceDataLoad.h"

#include "Flatbuffers/SceneShadowInstanceData_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneShadowInstanceData.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

extern "C"
dsSceneInstanceData* dsSceneShadowInstanceData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneShadowInstanceDataBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid scene shadow instance data flatbuffer format.");
		return nullptr;
	}

	auto fbShadowInstanceData = DeepSeaSceneLighting::GetSceneShadowInstanceData(data);
	const char* shadowManagerName = fbShadowInstanceData->shadowManager()->c_str();
	const char* shadowsName = fbShadowInstanceData->shadows()->c_str();
	const char* transformGroupName = fbShadowInstanceData->transformGroupName()->c_str();
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData,
			shadowManagerName) || type != dsSceneResourceType_Custom ||
		resource->type != dsSceneShadowManager_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find scene shadow manager '%s'.",
			shadowManagerName);
		return nullptr;
	}

	auto shadowManager = reinterpret_cast<dsSceneShadowManager*>(resource->resource);
	dsSceneLightShadows* shadows = dsSceneShadowManager_findLightShadows(shadowManager,
		shadowsName);
	if (!shadows)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't find shadows '%s' in scene shadow manager '%s'.", shadowsName,
			shadowManagerName);
		return nullptr;
	}

	return dsSceneShadowInstanceData_create(allocator, shadows, transformGroupName);
}
