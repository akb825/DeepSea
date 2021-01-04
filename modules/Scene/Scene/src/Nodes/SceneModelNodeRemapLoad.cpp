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

#include <DeepSea/Scene/Nodes/SceneModelNode.h>

#include "Flatbuffers/ModelNodeRemap_generated.h"
#include "SceneLoadContextInternal.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/Types.h>


extern "C"
dsSceneNode* dsSceneModelNode_loadRemap(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyModelNodeRemapBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid model node remap flatbuffer format.");
		return nullptr;
	}

	auto fbModelNode = DeepSeaScene::GetModelNodeRemap(data);

	const char* name = fbModelNode->name()->c_str();
	dsSceneNode* origNode;
	dsSceneResourceType type;
	if (!dsSceneLoadScratchData_findResource(&type, reinterpret_cast<void**>(&origNode),
			scratchData, name) || type != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType(origNode, dsSceneModelNode_type()))
	{
		errno = ENOTFOUND;
		DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find model node '%s'.", name);
		return NULL;
	}

	auto fbRemaps = fbModelNode->materialRemaps();
	dsSceneMaterialRemap* remaps = nullptr;
	uint32_t remapCount = 0;
	if (fbRemaps)
	{
		remapCount = fbRemaps->size();
		remaps = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneMaterialRemap, remapCount);
		for (uint32_t i = 0; i < remapCount; ++i)
		{
			auto fbRemap = (*fbRemaps)[i];
			dsSceneMaterialRemap& remap = remaps[i];
			if (fbRemap)
			{
				remap.name = fbRemap->name()->c_str();

				auto fbModelList = fbRemap->modelList();
				if (fbModelList)
					remap.modelList = fbModelList->c_str();
				else
					remap.modelList = nullptr;

				auto fbShader = fbRemap->shader();
				if (fbShader)
				{
					if (!dsSceneLoadScratchData_findResource(&type,
							reinterpret_cast<void**>(&remap.shader), scratchData,
							fbShader->c_str()) ||
						type != dsSceneResourceType_Shader)
					{
						errno = ENOTFOUND;
						DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find shader '%s'.",
							fbShader->c_str());
						return NULL;
					}
				}
				else
					remap.shader = nullptr;

				auto fbMaterial = fbRemap->material();
				if (fbMaterial)
				{
					if (!dsSceneLoadScratchData_findResource(&type,
							reinterpret_cast<void**>(&remap.material), scratchData,
							fbMaterial->c_str()) ||
						type != dsSceneResourceType_Material)
					{
						errno = ENOTFOUND;
						DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find material '%s'.",
							fbMaterial->c_str());
						return NULL;
					}
				}
				else
					remap.material = nullptr;
			}
			else
			{
				// Prevent invalid param errors.
				remap.name = "";
				remap.modelList = nullptr;
				remap.shader = nullptr;
				remap.material = nullptr;
			}
		}
	}

	return reinterpret_cast<dsSceneNode*>(dsSceneModelNode_cloneRemap(allocator,
		reinterpret_cast<dsSceneModelNode*>(origNode), remaps, remapCount));
}
