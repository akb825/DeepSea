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

#include "SceneLightSetLoad.h"
#include "Flatbuffers/SceneLightSetPrepare_generated.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

extern "C"
dsSceneGlobalData* dsSceneLightSetPrepare_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneLightSetPrepareBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Invalid scene light set prepare flatbuffer format.");
		return nullptr;
	}

	auto fbPrepare = DeepSeaSceneLighting::GetSceneLightSetPrepare(data);
	auto fbLightSets = fbPrepare->lightSets();

	uint32_t lightSetCount = fbLightSets->size();
	dsSceneLightSet** lightSets = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneLightSet*, lightSetCount);
	for (uint32_t i = 0; i < lightSetCount; ++i)
	{
		auto fbLightSet = (*fbLightSets)[i];
		if (!fbLightSet)
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Light set is null.");
			return nullptr;
		}

		const char* lightSetName = fbLightSet->c_str();
		dsSceneResourceType type;
		dsCustomSceneResource* resource;
		if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData,
				lightSetName) || type != dsSceneResourceType_Custom ||
			resource->type != dsSceneLightSet_type())
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find light set '%s'.",
				lightSetName);
			return nullptr;
		}

		lightSets[i] = reinterpret_cast<dsSceneLightSet*>(resource->resource);
	}

	float intensityThreshold = fbPrepare->intensityThreshold();
	if (intensityThreshold <= 0)
		intensityThreshold = DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD;
	return (dsSceneGlobalData*)dsSceneLightSetPrepare_create(allocator, lightSets, lightSetCount,
		intensityThreshold);
}
