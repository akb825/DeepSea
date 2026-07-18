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

#include "SceneParticleDrawListLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneParticle/SceneParticleDrawList.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ParticleDrawList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneItemList* dsSceneParticleDrawList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneParticle::VerifyParticleDrawListBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG, "Invalid particle draw list flatbuffer format.");
		return nullptr;
	}

	constexpr uint32_t maxStackItems = 8192;
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;

	auto fbDrawList = DeepSeaSceneParticle::GetParticleDrawList(data);
	auto fbViewFilter = fbDrawList->viewFilter();
	auto fbInstanceData = fbDrawList->instanceData();
	auto fbCullLists = fbDrawList->cullLists();

	dsSceneResourceType resourceType;
	dsViewFilter* viewFilter = nullptr;
	uint32_t instanceDataCount = 0;
	bool heapInstanceData = false;
	dsSceneInstanceData** instanceData = nullptr;
	uint32_t cullListCount = 0;
	bool heapCullLists = false;
	const char** cullLists = nullptr;
	dsSceneItemList* particleDrawList;

	if (fbViewFilter)
	{
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&viewFilter), scratchData, fbViewFilter->c_str()) ||
			resourceType != dsSceneResourceType_ViewFilter)
		{
			DS_LOG_ERROR_F(DS_SCENE_PARTICLE_LOG_TAG, "Couldn't find view filter '%s'.",
				fbViewFilter->c_str());
			errno = ENOTFOUND;
			return nullptr;
		}
	}

	instanceDataCount = fbInstanceData ? fbInstanceData->size() : 0;
	if (instanceDataCount > 0)
	{
		heapInstanceData = instanceDataCount > maxStackItems;
		if (heapInstanceData)
		{
			instanceData = DS_ALLOCATE_OBJECT_ARRAY(
				scratchAllocator, dsSceneInstanceData*, instanceDataCount);
			if (!instanceData)
				return nullptr;
		}
		else
			instanceData = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneInstanceData*, instanceDataCount);

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
				DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
					"Particle draw list instance data is null.");
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

	cullListCount = fbCullLists ? fbCullLists->size() : 0;
	if (cullListCount > 0)
	{
		heapCullLists = cullListCount > maxStackItems;
		if (heapCullLists)
		{
			cullLists = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, const char*, cullListCount);
			if (!cullLists)
				goto error;
		}
		else
			cullLists = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, cullListCount);

		for (uint32_t i = 0; i < cullListCount; ++i)
		{
			auto fbCullList = (*fbCullLists)[i];
			if (!fbCullList)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Particle draw list cull list name is null.");
				goto error;
			}

			cullLists[i] = fbCullList->c_str();
		}
	}

	particleDrawList = dsSceneParticleDrawList_create(allocator, name, viewFilter, resourceManager,
		resourceAllocator, instanceData, instanceDataCount, cullLists, cullListCount);
	if (heapInstanceData)
		DS_VERIFY(dsAllocator_free(scratchAllocator, instanceData));
	if (heapCullLists)
		DS_VERIFY(dsAllocator_free(scratchAllocator, cullLists));
	return particleDrawList;

error:
	// instanceDataCount should be the number that we need to clean up.
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
	if (heapInstanceData)
		DS_VERIFY(dsAllocator_free(scratchAllocator, instanceData));
	if (heapCullLists)
		DS_VERIFY(dsAllocator_free(scratchAllocator, cullLists));
	return nullptr;
}
