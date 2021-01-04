/*
 * Copyright 2020-2021 Aaron Barany
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

#include "InstanceForwardLightDataLoad.h"

#include "Flatbuffers/InstanceForwardLightData_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/CustomSceneResource.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/InstanceForwardLightData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

dsSceneInstanceData* dsInstanceForwardLightData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifyInstanceForwardLightDataBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid instance forward light data flatbuffer format.");
		return nullptr;
	}

	auto fbLightData = DeepSeaSceneLighting::GetInstanceForwardLightData(data);
	const char* groupDescName = fbLightData->variableGroupDesc()->c_str();

	dsShaderVariableGroupDesc* groupDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&groupDesc),
			scratchData, groupDescName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Couldn't find forward light data shader variable group description '%s'.",
			groupDescName);
		return nullptr;
	}

	const char* lightSetName = fbLightData->lightSet()->c_str();
	dsCustomSceneResource* lightSetResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&lightSetResource), scratchData, lightSetName) ||
		resourceType != dsSceneResourceType_Custom ||
		lightSetResource->type != dsSceneLightSet_type())
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Couldn't find scene light set '%s' for instance forward light data.", groupDescName);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	return dsInstanceForwardLightData_create(allocator, renderer->resourceManager, groupDesc,
		reinterpret_cast<dsSceneLightSet*>(lightSetResource->resource));
}
