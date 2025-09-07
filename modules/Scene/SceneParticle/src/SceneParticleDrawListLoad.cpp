/*
 * Copyright 2022-2025 Aaron Barany
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

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;

	auto fbDrawList = DeepSeaSceneParticle::GetParticleDrawList(data);
	auto fbInstanceData = fbDrawList->instanceData();
	auto fbCullLists = fbDrawList->cullLists();
	auto fbViews = fbDrawList->views();

	dsSceneInstanceData** instanceData = nullptr;
	uint32_t instanceDataCount = 0;
	uint32_t cullListCount = 0;
	const char** cullLists = nullptr;
	uint32_t viewCount = 0;
	const char** views = nullptr;

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

	if (fbCullLists && fbCullLists->size() > 0)
	{
		cullListCount = fbCullLists->size();
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

	if (fbViews && fbViews->size() > 0)
	{
		viewCount = fbViews->size();
		views = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, viewCount);
		for (uint32_t i = 0; i < viewCount; ++i)
		{
			auto fbView = (*fbViews)[i];
			if (!fbView)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Particle draw list view name is null.");
				goto error;
			}

			views[i] = fbView->c_str();
		}
	}

	return dsSceneParticleDrawList_create(allocator, name, resourceManager, resourceAllocator,
		instanceData, instanceDataCount, cullLists, cullListCount, views, viewCount);

error:
	// instanceDataCount should be the number that we need to clean up.
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
	return nullptr;
}
