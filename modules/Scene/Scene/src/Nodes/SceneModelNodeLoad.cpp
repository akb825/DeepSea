/*
 * Copyright 2019-2023 Aaron Barany
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

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ModelNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

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
	auto fbEmbeddedResources = fbModelNode->embeddedResources();
	dsSceneResources* embeddedResources = NULL;
	if (fbEmbeddedResources)
	{
		embeddedResources = dsSceneResources_loadData(allocator, resourceAllocator,
			loadContext, scratchData, fbEmbeddedResources->data(), fbEmbeddedResources->size());
		if (!embeddedResources)
			return nullptr;

		bool pushed = dsSceneLoadScratchData_pushSceneResources(scratchData, &embeddedResources, 1);
		dsSceneResources_freeRef(embeddedResources);
		if (!pushed)
			return nullptr;
	}

	dsSceneNode* node = nullptr;
	auto fbExtraItemLists = fbModelNode->extraItemLists();
	auto fbModelInfos = fbModelNode->models();
	auto fbBounds = fbModelNode->bounds();

	uint32_t extraItemCount = 0;
	const char** extraItems = nullptr;
	uint32_t modelInfoCount = fbModelInfos->size();
	uint32_t drawRangeCount = 0;
	dsSceneModelInitInfo* modelInfos = nullptr;
	dsSceneModelDrawRange* drawRanges = nullptr;

	for (uint32_t i = 0; i < modelInfoCount; ++i)
	{
		auto fbModelInfo = (*fbModelInfos)[i];
		if (!fbModelInfo)
		{
			errno = EFORMAT;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model info item is null.");
			return nullptr;
		}

		drawRangeCount += (*fbModelInfos)[i]->drawRanges()->size();
	}

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	DS_ASSERT(scratchAllocator);

	size_t tempSize = DS_ALIGNED_SIZE(modelInfoCount*sizeof(dsSceneModelInitInfo)) +
		DS_ALIGNED_SIZE(drawRangeCount*sizeof(dsSceneModelDrawRange));
	if (fbExtraItemLists && fbExtraItemLists->size() > 0)
		tempSize += DS_ALIGNED_SIZE(fbExtraItemLists->size()*sizeof(const char*));
	void* tempBuffer = dsAllocator_alloc(scratchAllocator, tempSize);
	if (!tempBuffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, tempBuffer, tempSize));

	modelInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelInitInfo, modelInfoCount);
	DS_ASSERT(modelInfos);
	drawRanges = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelDrawRange, drawRangeCount);
	DS_ASSERT(drawRanges);
	for (uint32_t i = 0; i < modelInfoCount; ++i)
	{
		auto fbModelInfo = (*fbModelInfos)[i];
		DS_ASSERT(fbModelInfo);

		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		dsSceneModelInitInfo* modelInfo = modelInfos + i;
		auto fbName = fbModelInfo->name();
		if (fbName)
			modelInfo->name = fbName->c_str();
		else
			modelInfo->name = nullptr;

		dsSceneResourceType resourceType;
		auto fbShaderName = fbModelInfo->shader();
		if (fbShaderName)
		{
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&modelInfo->shader), scratchData,
					fbShaderName->c_str()) ||
				resourceType != dsSceneResourceType_Shader)
			{
				errno = ENOTFOUND;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find model shader '%s'.",
					fbShaderName->c_str());
				goto finished;
			}
		}
		else
			modelInfo->shader = nullptr;

		auto fbMaterialName = fbModelInfo->material();
		if (fbMaterialName)
		{
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&modelInfo->material), scratchData,
					fbMaterialName->c_str()) ||
				resourceType != dsSceneResourceType_Material)
			{
				errno = ENOTFOUND;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find model material '%s'.",
					fbMaterialName->c_str());
				goto finished;
			}
		}

		const char* geometryName = fbModelInfo->geometry()->c_str();
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&modelInfo->geometry), scratchData, geometryName) ||
			resourceType != dsSceneResourceType_DrawGeometry)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find model geometry '%s'.", geometryName);
			goto finished;
		}

		modelInfo->distanceRange = DeepSeaScene::convert(*fbModelInfo->distanceRange());

		auto fbDrawRanges = fbModelInfo->drawRanges();
		modelInfo->drawRangeCount = fbDrawRanges->size();
		modelInfo->drawRanges = drawRanges;
		drawRanges += modelInfo->drawRangeCount;
		for (uint32_t j = 0; j < modelInfo->drawRangeCount; ++j)
		{
			auto fbModelDrawRange = (*fbDrawRanges)[j];
			if (!fbModelDrawRange)
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "No valid model draw range.");
				goto finished;
			}

			if (auto fbDrawRange = fbModelDrawRange->drawRange_as_DrawRange())
			{
				if (modelInfo->geometry->indexBuffer.buffer)
				{
					errno = EFORMAT;
					DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
						"Cannot use a DrawRange with geometry with an index buffer.");
					goto finished;
				}

				auto drawRange = const_cast<dsDrawRange*>(&modelInfo->drawRanges[j].drawRange);
				drawRange->vertexCount = fbDrawRange->vertexCount();
				drawRange->instanceCount = fbDrawRange->instanceCount();
				drawRange->firstVertex = fbDrawRange->firstVertex();
				drawRange->firstInstance = fbDrawRange->firstInstance();
			}
			else if (auto fbDrawIndexedRarnge = fbModelDrawRange->drawRange_as_DrawIndexedRange())
			{
				if (!modelInfo->geometry->indexBuffer.buffer)
				{
					errno = EFORMAT;
					DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
						"Cannot use a IndexedDrawRange with geometry without an index buffer.");
					goto finished;
				}

				auto drawRange =
					const_cast<dsDrawIndexedRange*>(&modelInfo->drawRanges[j].drawIndexedRange);
				drawRange->indexCount = fbDrawIndexedRarnge->indexCount();
				drawRange->instanceCount = fbDrawIndexedRarnge->instanceCount();
				drawRange->firstIndex = fbDrawIndexedRarnge->firstIndex();
				drawRange->vertexOffset = fbDrawIndexedRarnge->vertexOffset();
				drawRange->firstInstance = fbDrawIndexedRarnge->firstInstance();
			}
			else
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "No valid model draw range.");
				goto finished;
			}
		}

		modelInfo->primitiveType = static_cast<dsPrimitiveType>(fbModelInfo->primitiveType());
		auto fbModelList = fbModelInfo->modelList();
		if (fbModelList)
			modelInfo->modelList = fbModelList->c_str();
		else
			modelInfo->modelList = nullptr;
	}

	if (fbExtraItemLists && fbExtraItemLists->size() > 0)
	{
		extraItemCount = fbExtraItemLists->size();
		extraItems = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, const char*, extraItemCount);
		DS_ASSERT(extraItems);
		for (uint32_t i = 0; i < extraItemCount; ++i)
		{
			auto extraItem = (*fbExtraItemLists)[i];
			if (!extraItem)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model node extra item list name is null.");
				goto finished;
			}

			extraItems[i] = extraItem->c_str();
		}
	}

	// NOTE: May need to add more resources to the reference count later. Don't add all resources
	// since it would make circular references.
	node = reinterpret_cast<dsSceneNode*>(dsSceneModelNode_create(allocator, modelInfos,
		modelInfoCount, extraItems, extraItemCount, &embeddedResources, embeddedResources ? 1 : 0,
		fbBounds ? &DeepSeaScene::convert(*fbBounds) : nullptr));

finished:
	DS_VERIFY(dsAllocator_free(scratchAllocator, tempBuffer));
	if (embeddedResources)
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));

	return node;
}
