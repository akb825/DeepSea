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

#include "SceneSkinningDataLoad.h"

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneAnimation/SceneSkinningData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneSkinningData_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneInstanceData* dsSceneSkinningData_load(const dsSceneLoadContext* loadContext,
   dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
   void* userData, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), dataSize);
	if (!DeepSeaSceneAnimation::VerifySkinningDataBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Invalid scene skinning data flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	auto fbSkinningData = DeepSeaSceneAnimation::GetSkinningData(data);

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	dsResourceManager* resourceManager = renderer->resourceManager;
	dsMaterialType skinningType = dsSceneSkinningData_materialType(resourceManager);

	auto fbTextureInfoDesc = fbSkinningData->textureInfoDesc();
	dsShaderVariableGroupDesc* textureInfoDesc = NULL;
	if (skinningType != dsMaterialType_UniformBuffer)
	{
		if (!fbTextureInfoDesc)
		{
			DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Scene skinning data requires textureInfoDesc "
				"when using textures or texture buffers.");
			errno = EFORMAT;
			return nullptr;
		}

		const char* textureInfoDescName = fbTextureInfoDesc->c_str();
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&textureInfoDesc), scratchData, textureInfoDescName) ||
			resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
		{
			DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
				"Couldn't find skinning texture info shader variable group description '%s'.",
				textureInfoDescName);
			errno = ENOTFOUND;
			return nullptr;
		}
	}

	return dsSceneSkinningData_create(
		allocator, resourceManager, resourceAllocator, textureInfoDesc);
}
