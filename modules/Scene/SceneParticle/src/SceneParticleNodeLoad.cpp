/*
 * Copyright 2022-2023 Aaron Barany
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

#include "SceneParticleNodeLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneParticle/SceneParticleEmitterFactory.h>
#include <DeepSea/SceneParticle/SceneParticleNode.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ParticleNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneParticleNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneParticle::VerifyParticleNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG, "Invalid particle node flatbuffer format.");
		return nullptr;
	}

	auto fbParticleNode = DeepSeaSceneParticle::GetParticleNode(data);

	const char* factoryName = fbParticleNode->particleEmitterFactory()->c_str();
	dsSceneResourceType resourceType;
	dsCustomSceneResource* customResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&customResource), scratchData, factoryName) ||
		resourceType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneParticleEmitterFactory_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_PARTICLE_LOG_TAG, "Couldn't find particle emitter factory '%s'.",
			factoryName);
		return nullptr;
	}

	auto factory = reinterpret_cast<dsSceneParticleEmitterFactory*>(customResource->resource);

	auto fbItemLists = fbParticleNode->itemLists();
	uint32_t itemListCount = fbItemLists ? fbItemLists->size() : 0U;
	const char** itemLists = NULL;
	if (itemListCount > 0)
	{
		itemLists = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, itemListCount);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			auto fbItemList = (*fbItemLists)[i];
			if (!fbItemList)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG, "Particle node item list name is null.");
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	return (dsSceneNode*)dsSceneParticleNode_create(allocator, allocator,
		factory->createEmitterFunc, factory->updateEmitterFunc, factory->userData, nullptr,
		itemLists, itemListCount);
}
