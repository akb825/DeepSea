/*
 * Copyright 2023 Aaron Barany
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

#include "SceneAnimationNodeLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationNodeMapCache.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/AnimationNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

dsSceneNode* dsSceneAnimationNode_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneAnimation::VerifyAnimationNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Invalid animation node flatbuffer format.");
		return nullptr;
	}

	auto fbAnimationNode = DeepSeaSceneAnimation::GetAnimationNode(data);

	const char* nodeMapCacheName = fbAnimationNode->nodeMapCache()->c_str();
	dsSceneResourceType resourceType;
	dsCustomSceneResource* customResource;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&customResource), scratchData, nodeMapCacheName) ||
		resourceType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneAnimationNodeMapCache_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG, "Couldn't find animation node map cache '%s'.",
			nodeMapCacheName);
		return nullptr;
	}

	auto nodeMapCache = reinterpret_cast<dsAnimationNodeMapCache*>(customResource->resource);

	auto fbItemLists = fbAnimationNode->itemLists();
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
				DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Animation node item list name is null.");
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	return (dsSceneNode*)dsSceneAnimationNode_create(allocator, nodeMapCache, itemLists,
		itemListCount);
}
