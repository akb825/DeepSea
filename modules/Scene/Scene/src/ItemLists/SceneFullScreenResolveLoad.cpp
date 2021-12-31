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

#include <DeepSea/Scene/ItemLists/SceneFullScreenResolve.h>

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/FullScreenResolve_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

extern "C"
dsSceneItemList* dsSceneFullScreenResolve_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyFullScreenResolveBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid full screen resolve data flatbuffer format.");
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	auto fbResolve = DeepSeaScene::GetFullScreenResolve(data);
	const char* shaderName = fbResolve->shader()->c_str();
	const char* materialName = fbResolve->material()->c_str();
	auto fbDynamicRenderStates = fbResolve->dynamicRenderStates();

	dsShader* shader;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&shader),
			scratchData, shaderName) ||
		resourceType != dsSceneResourceType_Shader)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find full screen resolve shader '%s'.",
			shaderName);
		return nullptr;
	}

	dsMaterial* material;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&material),
			scratchData, materialName) ||
		resourceType != dsSceneResourceType_Material)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find full screen resolve material '%s'.",
			materialName);
		return nullptr;
	}

	dsDynamicRenderStates dynamicRenderStates;
	if (fbDynamicRenderStates)
		dynamicRenderStates = DeepSeaScene::convert(*fbDynamicRenderStates);

	return reinterpret_cast<dsSceneItemList*>(dsSceneFullScreenResolve_create(allocator, name,
		resourceManager, shader, material, fbDynamicRenderStates ? &dynamicRenderStates : NULL));
}
