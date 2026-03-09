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

#include "SceneSSAOLoad.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneSSAO.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneSSAO_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneItemList* dsSceneSSAO_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifySceneSSAOBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Invalid scene SSAO flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	auto fbSSAO = DeepSeaSceneLighting::GetSceneSSAO(data);
	auto fbViewFilter = fbSSAO->viewFilter();
	const char* shaderName = fbSSAO->shader()->c_str();

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

	dsShader* shader;
	if (!dsSceneLoadScratchData_findResource(
			&resourceType, reinterpret_cast<void**>(&shader), scratchData, shaderName) ||
		resourceType != dsSceneResourceType_Shader)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find shader '%s'.", shaderName);
		errno = ENOTFOUND;
		return nullptr;
	}

	const char* materialName = fbSSAO->material()->c_str();
	dsMaterial* material;
	if (!dsSceneLoadScratchData_findResource(
			&resourceType, reinterpret_cast<void**>(&material), scratchData, materialName) ||
		resourceType != dsSceneResourceType_Material)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find material '%s'.", materialName);
		errno = ENOTFOUND;
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	return reinterpret_cast<dsSceneItemList*>(dsSceneSSAO_create(
		allocator, resourceManager, resourceAllocator, name, viewFilter, shader, material));
}
