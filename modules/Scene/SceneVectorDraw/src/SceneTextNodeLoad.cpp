/*
 * Copyright 2020-2026 Aaron Barany
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

#include "SceneTextNodeLoad.h"

#include "SceneVectorDrawTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/Types.h>

#include <DeepSea/SceneVectorDraw/SceneTextNode.h>
#include <DeepSea/SceneVectorDraw/SceneText.h>
#include <DeepSea/SceneVectorDraw/SceneVectorShaders.h>

#include <DeepSea/VectorDraw/VectorResources.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneTextNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneTextNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyTextNodeBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Invalid text node flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	constexpr uint32_t maxStackItemLists = 16384;
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);

	auto fbTextNode = DeepSeaSceneVectorDraw::GetTextNode(data);
	auto fbEmbeddedResources = fbTextNode->embeddedResources();
	auto textUserData = static_cast<dsSceneTextNodeUserData*>(userData);
	dsSceneResources* embeddedResources = NULL;
	if (fbEmbeddedResources)
	{
		embeddedResources = dsSceneResources_loadData(allocator, resourceAllocator,
			loadContext, scratchData, fbEmbeddedResources->data(), fbEmbeddedResources->size(),
			relativePathUserData, openRelativePathStreamFunc, closeRelativePathStreamFunc);
		if (!embeddedResources)
			return nullptr;

		bool pushed = dsSceneLoadScratchData_pushSceneResources(scratchData, &embeddedResources, 1);
		dsSceneResources_freeRef(embeddedResources);
		if (!pushed)
			return nullptr;
	}

	dsSceneNode* node = nullptr;
	auto fbText = fbTextNode->text();
	auto fbShader = fbTextNode->shader();
	auto fbItemLists = fbTextNode->itemLists();

	dsSceneText* text;
	dsShader* shader;
	uint32_t itemListCount = 0;
	bool heapItemLists = false;
	const char** itemLists = nullptr;

	dsSceneResourceType resourceType;
	dsCustomSceneResource* customResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&customResource), scratchData, fbText->c_str()) ||
		resourceType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneText_type())
	{
		DS_LOG_ERROR_F(
			DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find scene text '%s'.", fbText->c_str());
		errno = ENOTFOUND;
		goto finished;
	}

	text = reinterpret_cast<dsSceneText*>(customResource->resource);

	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&shader), scratchData, fbShader->c_str()) ||
		resourceType != dsSceneResourceType_Shader)
	{
		DS_LOG_ERROR_F(
			DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find shader '%s'.", fbShader->c_str());
		errno = ENOTFOUND;
		goto finished;
	}

	itemListCount = fbItemLists ? fbItemLists->size() : 0;
	if (itemListCount > 0)
	{
		heapItemLists = itemListCount > maxStackItemLists;
		if (heapItemLists)
		{
			itemLists = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, const char*, itemListCount);
			if (!itemLists)
				return nullptr;
		}
		else
			itemLists = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, itemListCount);

		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			auto item = (*fbItemLists)[i];
			if (!item)
			{
				DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					"Vector image node extra item name is null.");
				errno = EFORMAT;
				goto finished;
			}

			itemLists[i] = item->c_str();
		}
	}

	// NOTE: May need to add more resources to the reference count later. Don't add all resources
	// since it would make circular references.
	node = reinterpret_cast<dsSceneNode*>(dsSceneTextNode_create(allocator, text,
		static_cast<dsTextAlign>(fbTextNode->alignment()), fbTextNode->maxWidth(),
		fbTextNode->lineScale(), fbTextNode->z(), fbTextNode->firstChar(), fbTextNode->charCount(),
		shader, &textUserData->textRenderInfo, itemLists, itemListCount, &embeddedResources,
		embeddedResources ? 1 : 0));

finished:
	if (embeddedResources)
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));
	if (heapItemLists)
		DS_VERIFY(dsAllocator_free(scratchAllocator, itemLists));

	return node;
}
