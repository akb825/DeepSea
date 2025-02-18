/*
 * Copyright 2021-2025 Aaron Barany
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

#include "SceneShadowManagerLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneShadowManager_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

void* dsSceneShadowManager_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize, void*,
	dsOpenSceneResourcesRelativePathStreamFunction, dsCloseSceneResourcesRelativePathStreamFunction)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneShadowManagerBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Invalid scene shadow manager flatbuffer format.");
		return nullptr;
	}

	auto fbShadowManager = DeepSeaSceneLighting::GetSceneShadowManager(data);

	auto fbShadows = fbShadowManager->shadows();
	uint32_t shadowCount = fbShadows->size();
	if (shadowCount == 0)
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Scene shadow manager has no shadows to manage.");
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	dsSceneLightShadows** shadows =
		DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneLightShadows*, shadowCount);
	for (uint32_t i = 0; i < shadowCount; ++i)
	{
		auto fbLightShadows = (*fbShadows)[i];
		if (!fbLightShadows)
		{
			for (uint32_t j = 0; j < i; ++j)
				dsSceneLightShadows_destroy(shadows[j]);
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
				"Scene shadow manager contains an unset light shadows element.");
			return nullptr;
		}

		const char* shadowsName = fbLightShadows->name()->c_str();
		const char* lightSetName = fbLightShadows->lightSet()->c_str();
		dsSceneResourceType type;
		dsCustomSceneResource* resource;
		if (!dsSceneLoadScratchData_findResource(
				&type, (void**)&resource, scratchData, lightSetName) ||
			type != dsSceneResourceType_Custom || resource->type != dsSceneLightSet_type())
		{
			for (uint32_t j = 0; j < i; ++j)
				dsSceneLightShadows_destroy(shadows[j]);
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find light set '%s'.",
				lightSetName);
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
			for (uint32_t j = 0; j < i; ++j)
				dsSceneLightShadows_destroy(shadows[j]);
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
				"Couldn't find shader variable group description '%s'.", lightSetName);
			return nullptr;
		}

		auto fbTransformGroupName = fbLightShadows->transformGroupName();
		const char* transformGroupName =
			fbTransformGroupName ? fbTransformGroupName->c_str() : nullptr;

		float minDepthRanges[4] = {};
		auto fbMinDepthRanges = fbLightShadows->minDepthRanges();
		if (fbMinDepthRanges)
		{
			for (uint32_t i = 0; i < fbMinDepthRanges->size() && i < 4; ++i)
				minDepthRanges[i] = (*fbMinDepthRanges)[i];
		}

		dsSceneShadowParams params =
		{
			fbLightShadows->maxCascades(),
			fbLightShadows->maxFirstSplitDistance(),
			fbLightShadows->cascadeExpFactor(),
			{minDepthRanges[0], minDepthRanges[1], minDepthRanges[2], minDepthRanges[3]},
			fbLightShadows->fadeStartDistance(),
			fbLightShadows->maxDistance()
		};

		dsSceneLightShadows* lightShadows = dsSceneLightShadows_create(allocator, shadowsName,
			resourceManager, lightSet, lightType, lightName, transformGroupDesc, transformGroupName,
			&params);
		if (!lightShadows)
		{
			// Guarantee errno is preserved.
			int prevErrno = errno;
			for (uint32_t j = 0; j < i; ++j)
				dsSceneLightShadows_destroy(shadows[j]);
			errno = prevErrno;
			return nullptr;
		}

		shadows[i] = lightShadows;
	}

	return dsSceneShadowManager_create(allocator, shadows, shadowCount);
}
