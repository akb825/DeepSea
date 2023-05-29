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

#include <DeepSea/Scene/ItemLists/SceneModelList.h>

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ModelList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneItemList* dsSceneModelList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyModelListBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid model list flatbuffer format.");
		return nullptr;
	}

	auto fbModelList = DeepSeaScene::GetModelList(data);
	auto fbInstanceData = fbModelList->instanceData();
	auto fbDynamicRenderStates = fbModelList->dynamicRenderStates();
	auto fbCullList = fbModelList->cullList();

	uint32_t instanceDataCount = 0;
	dsSceneInstanceData** instanceData = nullptr;
	dsDynamicRenderStates dynamicRenderStates;

	if (fbInstanceData && fbInstanceData->size() > 0)
	{
		instanceDataCount = fbInstanceData->size();
		instanceData = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneInstanceData*, instanceDataCount);
		DS_ASSERT(instanceData);

		for (uint32_t i = 0; i < instanceDataCount; ++i)
		{
			auto fbInstance = (*fbInstanceData)[i];
			dsSceneInstanceData* instance = nullptr;
			if (fbInstance)
			{
				auto fbData = fbInstance->data();
				instance = dsSceneInstanceData_load(allocator, resourceAllocator, loadContext,
					scratchData, fbInstance->type()->c_str(), fbData->data(), fbData->size());
			}
			else
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model list instance data is null.");
			}

			if (!instance)
			{
				// Only clean up the instances loaded so far.
				instanceDataCount = i;
				goto error;
			}

			instanceData[i] = instance;
		}
	}

	if (fbDynamicRenderStates)
		dynamicRenderStates = DeepSeaScene::convert(*fbDynamicRenderStates);

	return reinterpret_cast<dsSceneItemList*>(dsSceneModelList_create(allocator, name, instanceData,
		instanceDataCount, static_cast<dsModelSortType>(fbModelList->sortType()),
		fbDynamicRenderStates ? &dynamicRenderStates : nullptr,
		fbCullList ? fbCullList->c_str() : nullptr));

error:
	// instanceDataCount should be the number that we need to clean up.
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
	return nullptr;
}
