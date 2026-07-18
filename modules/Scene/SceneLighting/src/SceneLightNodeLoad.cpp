/*
 * Copyright 2022-2026 Aaron Barany
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

#include "SceneLightNodeLoad.h"

#include "SceneLightLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightNode.h>

#include <string.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/LightNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneLightNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize, void*,
	dsOpenRelativePathStreamFunction, dsCloseRelativePathStreamFunction)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifyLightNodeBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Invalid light node flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	constexpr uint32_t maxStackItemLists = 16384;
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);

	auto fbLightNode = DeepSeaSceneLighting::GetLightNode(data);

	const char* lightBaseName = fbLightNode->lightBaseName()->c_str();
	dsSceneLight templateLight;
	memset(&templateLight, 0, sizeof(dsSceneLight));
	if (!DeepSeaSceneLighting::extractLightData(templateLight, fbLightNode->templateLight_type(),
			fbLightNode->templateLight()))
	{
		DS_LOG_ERROR_F(
			DS_SCENE_LIGHTING_LOG_TAG, "Invalid light '%s' for scene light node.", lightBaseName);
		errno = EFORMAT;
		return nullptr;
	}

	auto fbItemLists = fbLightNode->itemLists();
	uint32_t itemListCount = fbItemLists ? fbItemLists->size() : 0U;
	bool heapItemLists = itemListCount > maxStackItemLists;
	const char** itemLists = nullptr;
	if (itemListCount > 0)
	{
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
			auto fbItemList = (*fbItemLists)[i];
			if (!fbItemList)
			{
				DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "Light node item list name is null.");
				if (heapItemLists)
					DS_VERIFY(dsAllocator_free(scratchAllocator, itemLists));
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	auto node = reinterpret_cast<dsSceneNode*>(dsSceneLightNode_create(allocator, &templateLight,
		lightBaseName, fbLightNode->singleInstance(), itemLists, itemListCount));
	if (heapItemLists)
		DS_VERIFY(dsAllocator_free(scratchAllocator, itemLists));
	return node;
}
