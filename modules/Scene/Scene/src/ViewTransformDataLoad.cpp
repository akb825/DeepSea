/*
 * Copyright 2019-2021 Aaron Barany
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

#include <DeepSea/Scene/ViewTransformData.h>

#include "SceneLoadContextInternal.h"
#include "Flatbuffers/ViewTransformData_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

extern "C"
dsSceneGlobalData* dsViewTransformData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyViewTransformDataBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid instance transform data flatbuffer format.");
		return nullptr;
	}

	auto fbTransformData = DeepSeaScene::GetViewTransformData(data);
	const char* groupDescName = fbTransformData->variableGroupDesc()->c_str();

	dsShaderVariableGroupDesc* groupDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&groupDesc),
			scratchData, groupDescName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Couldn't find view transform shader variable group description '%s'.", groupDescName);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	return dsViewTransformData_create(allocator, renderer->resourceManager, groupDesc);
}

