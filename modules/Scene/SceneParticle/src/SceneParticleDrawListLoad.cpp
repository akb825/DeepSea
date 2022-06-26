/*
 * Copyright 2022 Aaron Barany
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
#endif

#include "Flatbuffers/ParticleDrawList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
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
	auto fbCullList = fbDrawList->cullList();

	const char* cullList = fbCullList ? fbCullList->c_str() : nullptr;;

	dsSceneInstanceData** instanceData = nullptr;
	uint32_t instanceDataCount = 0;
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

	return dsSceneParticleDrawList_create(allocator, name, resourceManager, resourceAllocator,
		instanceData, instanceDataCount, cullList);

error:
	// instanceDataCount should be the number that we need to clean up.
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
	return nullptr;
}
