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

#include "SceneVectorImageNodeLoad.h"

#include "Flatbuffers/VectorImageNode_generated.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <DeepSea/VectorDrawScene/SceneVectorImageNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorImage.h>
#include <DeepSea/VectorDrawScene/VectorSceneShaders.h>

dsSceneNode* dsSceneVectorImageNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaVectorDrawScene::VerifyVectorImageNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid vector image node flatbuffer format.");
		return nullptr;
	}

	auto fbVectorImageNode = DeepSeaVectorDrawScene::GetVectorImageNode(data);
	auto fbEmbeddedResources = fbVectorImageNode->embeddedResources();
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
	auto fbVectorImage = fbVectorImageNode->vectorImage();
	auto fbVectorShaders = fbVectorImageNode->vectorShaders();
	auto fbMaterial = fbVectorImageNode->material();
	auto fbItemLists = fbVectorImageNode->itemLists();

	dsVectorImage* vectorImage;
	dsVectorShaders* vectorShaders;
	dsMaterial* material;
	const char** itemLists = nullptr;
	uint32_t itemListCount = 0;

	auto fbSize = fbVectorImageNode->size();
	dsVector2f size;
	bool hasSize = false;
	if (fbSize)
	{
		size = DeepSeaScene::convert(*fbSize);
		hasSize = true;
	}

	dsSceneResourceType resourceType;
	dsCustomSceneResource* customResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&customResource), scratchData, fbVectorImage->c_str()) ||
		resourceType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneVectorImage_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find scene vector image '%s'.",
			fbVectorImage->c_str());
		goto finished;
	}

	vectorImage = reinterpret_cast<dsVectorImage*>(customResource->resource);

	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&customResource), scratchData,
			fbVectorShaders->c_str()) ||
		resourceType != dsSceneResourceType_Custom ||
		customResource->type != dsVectorSceneShaders_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find vector shaders '%s'.",
			fbVectorShaders->c_str());
		goto finished;
	}

	vectorShaders = reinterpret_cast<dsVectorShaders*>(customResource->resource);

	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&material), scratchData, fbMaterial->c_str()) ||
		resourceType != dsSceneResourceType_Material)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find material '%s'.", fbMaterial->c_str());
		goto finished;
	}

	if (fbItemLists && fbItemLists->size() > 0)
	{
		itemListCount = fbItemLists->size();
		itemLists = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, itemListCount);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			auto item = (*fbItemLists)[i];
			if (!item)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Vector image node extra item name is null.");
				goto finished;
			}

			itemLists[i] = item->c_str();
		}
	}

	// NOTE: May need to add more resources to the reference count later. Don't add all resources
	// since it would make circular references.
	node = reinterpret_cast<dsSceneNode*>(dsSceneVectorImageNode_create(allocator, vectorImage,
		hasSize ? &size : nullptr, fbVectorImageNode->z(), vectorShaders, material, itemLists,
		itemListCount, &embeddedResources, embeddedResources ? 1 : 0));

finished:
	if (embeddedResources)
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));

	return node;
}
