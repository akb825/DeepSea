/*
 * Copyright 2020 Aaron Barany
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

#include "Flatbuffers/ModelNodeReconfig_generated.h"
#include "SceneLoadContextInternal.h"

#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/Types.h>

extern "C"
dsSceneNode* dsSceneModelNode_loadReconfig(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyModelNodeReconfigBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid model node remap flatbuffer format.");
		return nullptr;
	}

	auto fbModelNode = DeepSeaScene::GetModelNodeReconfig(data);

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

	auto fbModels = fbModelNode->models();
	uint32_t modelCount = fbModels->size();
	auto fbExtraItemLists = fbModelNode->extraItemLists();
	uint32_t extraItemListCount = fbExtraItemLists ? fbExtraItemLists->size() : 0;

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	DS_ASSERT(scratchAllocator);

	size_t tempSize = DS_ALIGNED_SIZE(modelCount*sizeof(dsSceneModelReconfig)) +
		DS_ALIGNED_SIZE(extraItemListCount*sizeof(const char*));
	void* tempBuffer = dsAllocator_alloc(scratchAllocator, tempSize);
	if (!tempBuffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, tempBuffer, tempSize));

	dsSceneModelReconfig* models =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelReconfig, modelCount);
	DS_ASSERT(models);
	const char** extraItemLists =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, const char*, extraItemListCount);
	dsSceneNode* node = nullptr;

	for (uint32_t i = 0; i < modelCount; ++i)
	{
		auto fbModel = (*fbModels)[i];
		dsSceneModelReconfig& model = models[i];
		if (!fbModel)
		{
			errno = ENOTFOUND;
			DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find model node '%s'.", name);
			goto finished;
		}

		model.name = fbModel->name()->c_str();

		auto fbShader = fbModel->shader();
		if (fbShader)
		{
			if (!dsSceneLoadScratchData_findResource(&type,
					reinterpret_cast<void**>(&model.shader), scratchData,
					fbShader->c_str()) ||
				type != dsSceneResourceType_Shader)
			{
				errno = ENOTFOUND;
				DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find shader '%s'.",
					fbShader->c_str());
				goto finished;
			}
		}
		else
			model.shader = nullptr;

		auto fbMaterial = fbModel->material();
		if (fbMaterial)
		{
			if (!dsSceneLoadScratchData_findResource(&type,
					reinterpret_cast<void**>(&model.material), scratchData,
					fbMaterial->c_str()) ||
				type != dsSceneResourceType_Material)
			{
				errno = ENOTFOUND;
				DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find material '%s'.",
					fbMaterial->c_str());
				goto finished;
			}
		}
		else
			model.material = nullptr;

		model.distanceRange = DeepSeaScene::convert(*fbModel->distanceRange());
		model.modelList = fbModel->modelList()->c_str();
	}

	if (extraItemListCount > 0)
	{
		DS_ASSERT(extraItemLists);
		for (uint32_t i = 0; i < extraItemListCount; ++i)
		{
			auto extraItem = (*fbExtraItemLists)[i];
			if (!extraItem)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model node extra item name is null.");
				goto finished;
			}

			extraItemLists[i] = extraItem->c_str();
		}
	}

	node = reinterpret_cast<dsSceneNode*>(dsSceneModelNode_cloneReconfig(allocator,
		reinterpret_cast<dsSceneModelNode*>(origNode), models, modelCount, extraItemLists,
		extraItemListCount));

finished:
	DS_VERIFY(dsAllocator_free(scratchAllocator, tempBuffer));

	return node;
}
