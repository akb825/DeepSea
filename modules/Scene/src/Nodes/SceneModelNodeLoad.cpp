/*
 * Copyright 2019 Aaron Barany
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

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Flatbuffers/ModelNode_generated.h>
#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/Types.h>

extern "C"
dsSceneNode* dsSceneModelNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyModelNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid model node flatbuffer format.");
		return nullptr;
	}

	auto fbModelNode = DeepSeaScene::GetModelNode(data);
	auto embeddedResources = fbModelNode->embeddedResources();
	if (embeddedResources)
	{
		dsSceneResources* sceneResources = dsSceneResources_loadData(allocator, resourceAllocator,
			loadContext, scratchData, embeddedResources->data(), embeddedResources->size());
		if (!sceneResources)
			return nullptr;

		if (!dsSceneLoadScratchData_pushSceneResources(scratchData, &sceneResources, 1))
			return nullptr;
	}

	dsSceneNode* node = nullptr;
	auto fbExtraItemLists = fbModelNode->extraItemLists();
	auto fbModelInfos = fbModelNode->models();
	auto fbBounds = fbModelNode->bounds();
	uint32_t extraItemListSize = 0;
	uint32_t extraItemCount = 0;
	const char** extraItems = nullptr;
	uint32_t modelInfoCount = fbModelInfos->size();
	uint32_t modelInfoSize = 0;
	dsSceneModelInitInfo* modelInfos = nullptr;
	uint32_t resourceCount = 0;
	dsSceneResources** resources = nullptr;

	if (fbExtraItemLists)
	{
		extraItemCount = fbExtraItemLists->size();
		extraItemListSize = static_cast<uint32_t>(sizeof(const char*)*extraItemCount);
		extraItems = reinterpret_cast<const char**>(dsSceneLoadScratchData_allocate(scratchData,
			extraItemListSize));
		if (!extraItems)
			goto finished;

		for (uint32_t i = 0; i < extraItemCount; ++i)
		{
			auto extraItem = (*fbExtraItemLists)[i];
			if (!extraItem)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model node extra item name is null.");
				goto finished;
			}
		}
	}

	modelInfoSize = static_cast<uint32_t>(modelInfoCount*sizeof(dsSceneModelInitInfo));
	modelInfos = reinterpret_cast<dsSceneModelInitInfo*>(dsSceneLoadScratchData_allocate(
		scratchData, modelInfoSize));
	if (!modelInfos)
		goto finished;

	for (uint32_t i = 0; i < modelInfoCount; ++i)
	{
		auto fbModelInfo = (*fbModelInfos)[i];
		if (!fbModelInfo)
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model info item name is null.");
			goto finished;
		}

		dsSceneModelInitInfo* modelInfo = modelInfos + i;
		const char* shaderName = fbModelInfo->shader()->c_str();
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&modelInfo->shader), scratchData, shaderName) ||
			resourceType != dsSceneResourceType_Shader)
		{
			DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find model shader '%s'.", shaderName);
			goto finished;
		}

		const char* materialName = fbModelInfo->material()->c_str();
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&modelInfo->material), scratchData, materialName) ||
			resourceType != dsSceneResourceType_Material)
		{
			DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find model material '%s'.", materialName);
			goto finished;
		}

		const char* geometryName = fbModelInfo->geometry()->c_str();
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&modelInfo->geometry), scratchData, geometryName) ||
			resourceType != dsSceneResourceType_DrawGeometry)
		{
			DS_LOG_INFO_F(DS_SCENE_LOG_TAG, "Couldn't find model geometry '%s'.", geometryName);
			goto finished;
		}

		modelInfo->distanceRange = DeepSeaScene::convert(*fbModelInfo->distanceRange());
		if (auto fbDrawRange = fbModelInfo->drawRange_as_DrawRange())
		{
			modelInfo->drawRange.vertexCount = fbDrawRange->vertexCount();
			modelInfo->drawRange.instanceCount = fbDrawRange->instanceCount();
			modelInfo->drawRange.firstVertex = fbDrawRange->firstVertex();
			modelInfo->drawRange.firstInstance = fbDrawRange->firstInstance();
		}
		else if (auto fbDrawIndexedRarnge = fbModelInfo->drawRange_as_DrawIndexedRange())
		{
			modelInfo->drawIndexedRange.indexCount = fbDrawIndexedRarnge->indexCount();
			modelInfo->drawIndexedRange.instanceCount = fbDrawIndexedRarnge->instanceCount();
			modelInfo->drawIndexedRange.firstIndex = fbDrawIndexedRarnge->firstIndex();
			modelInfo->drawIndexedRange.vertexOffset = fbDrawIndexedRarnge->vertexOffset();
			modelInfo->drawIndexedRange.firstInstance = fbDrawIndexedRarnge->firstInstance();
		}
		else
		{
			DS_LOG_INFO(DS_SCENE_LOG_TAG, "No valid model draw range.");
			goto finished;
		}

		modelInfo->primitiveType = static_cast<dsPrimitiveType>(fbModelInfo->primitiveType());
		modelInfo->listName = fbModelInfo->listName()->c_str();
	}

	resources = dsSceneLoadScratchData_getSceneResources(&resourceCount, scratchData);
	node = reinterpret_cast<dsSceneNode*>(dsSceneModelNode_create(allocator, modelInfos,
		modelInfoCount, extraItems, extraItemCount, resources, resourceCount,
		fbBounds ? &DeepSeaScene::convert(*fbBounds) : nullptr));

finished:
	DS_VERIFY(dsSceneLoadScratchData_popData(scratchData, modelInfoSize));
	DS_VERIFY(dsSceneLoadScratchData_popData(scratchData, extraItemListSize));
	if (embeddedResources)
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));

	return node;
}

