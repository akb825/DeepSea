/*
 * Copyright 2026 Aaron Barany
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

#include "InstanceDiscardBoundsDataLoad.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneVectorDraw/InstanceDiscardBoundsData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/InstanceDiscardBoundsData_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneInstanceData* dsInstanceDiscardBoundsData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyInstanceDiscardBoundsDataBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid instance discard bounds data flatbuffer format.");
		return nullptr;
	}

	auto fbDiscardBoundsData = DeepSeaSceneVectorDraw::GetInstanceDiscardBoundsData(data);
	const char* groupDescName = fbDiscardBoundsData->variableGroupDesc()->c_str();

	dsShaderVariableGroupDesc* groupDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&groupDesc),
			scratchData, groupDescName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
			"Couldn't find instance discard bounds shader variable group description '%s'.",
			groupDescName);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	return dsInstanceDiscardBoundsData_create(
		allocator, resourceAllocator, renderer->resourceManager, groupDesc);
}
