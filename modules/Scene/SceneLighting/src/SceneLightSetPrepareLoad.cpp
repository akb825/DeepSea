/*
 * Copyright 2020-2023 Aaron Barany
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

#include "SceneLightSetPrepareLoad.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneLightSetPrepare_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneItemList* dsSceneLightSetPrepare_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const char* name, const uint8_t* data, size_t dataSize)
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

	const char* lightSetName = fbPrepare->lightSet()->c_str();
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData, lightSetName) ||
		type != dsSceneResourceType_Custom || resource->type != dsSceneLightSet_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find light set '%s'.", lightSetName);
		return nullptr;
	}

	float intensityThreshold = fbPrepare->intensityThreshold();
	if (intensityThreshold <= 0)
		intensityThreshold = DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD;
	return (dsSceneItemList*)dsSceneLightSetPrepare_create(allocator, name,
		reinterpret_cast<dsSceneLightSet*>(resource->resource), intensityThreshold);
}
