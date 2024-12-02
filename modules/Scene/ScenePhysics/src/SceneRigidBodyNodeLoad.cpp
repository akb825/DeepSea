/*
 * Copyright 2024 Aaron Barany
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

#include "SceneRigidBodyNodeLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/SceneRigidBody.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyNode.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneRigidBodyNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneRigidBodyNode_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyRigidBodyNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Invalid rigid body node flatbuffer format.");
		return nullptr;
	}

	auto fbRigidBodyNode = DeepSeaScenePhysics::GetRigidBodyNode(data);

	auto fbItemLists = fbRigidBodyNode->itemLists();
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
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Rigid body node item list name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	return reinterpret_cast<dsSceneNode*>(dsSceneRigidBodyNode_create(
		allocator, fbRigidBodyNode->rigidBody()->c_str(), nullptr, itemLists, itemListCount));
}

dsSceneNode* dsSceneRigidBodyNode_loadUnique(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyRigidBodyNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Invalid rigid body node flatbuffer format.");
		return nullptr;
	}

	auto fbRigidBodyNode = DeepSeaScenePhysics::GetRigidBodyNode(data);

	const char* rigidBodyName = fbRigidBodyNode->rigidBody()->c_str();
	dsSceneResourceType resourceType;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&resource),
			scratchData, rigidBodyName) ||
		resourceType != dsSceneResourceType_Custom ||
		resource->type != dsSceneRigidBody_type())
	{
		DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find rigid body '%s'.", rigidBodyName);
		errno = ENOTFOUND;
		return nullptr;
	}

	auto rigidBody = reinterpret_cast<dsRigidBody*>(resource->resource);

	auto fbItemLists = fbRigidBodyNode->itemLists();
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
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Rigid body node item list name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	return reinterpret_cast<dsSceneNode*>(dsSceneRigidBodyNode_create(
		allocator, nullptr, rigidBody, itemLists, itemListCount));
}