/*
 * Copyright 2021-2026 Aaron Barany
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

#include "SceneShadowManagerPrepareLoad.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>
#include <DeepSea/SceneLighting/SceneShadowManagerPrepare.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneShadowManagerPrepare_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneItemList* dsSceneShadowManagerPrepare_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneShadowManagerPrepareBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Invalid scene shadow manager prepare flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	auto fbPrepare = DeepSeaSceneLighting::GetSceneShadowManagerPrepare(data);
	auto fbViewFilter = fbPrepare->viewFilter();
	const char* shadowManagerName = fbPrepare->shadowManager()->c_str();

	dsSceneResourceType resourceType;
	dsViewFilter* viewFilter = nullptr;
	if (fbViewFilter)
	{
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&viewFilter), scratchData, fbViewFilter->c_str()) ||
			resourceType != dsSceneResourceType_ViewFilter)
		{
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find view filter '%s'.",
				fbViewFilter->c_str());
			errno = ENOTFOUND;
			return nullptr;
		}
	}

	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(
			&resourceType, reinterpret_cast<void**>(&resource), scratchData, shadowManagerName) ||
		resourceType != dsSceneResourceType_Custom || resource->type != dsSceneShadowManager_type())
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find scene shadow manager '%s'.",
			shadowManagerName);
		errno = ENOTFOUND;
		return nullptr;
	}

	auto shadowManager = reinterpret_cast<dsSceneShadowManager*>(resource->resource);
	return dsSceneShadowManagerPrepare_create(allocator, name, viewFilter, shadowManager);
}
