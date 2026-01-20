/*
 * Copyright 2023-2026 Aaron Barany
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

#include "SceneAnimationTransformNodeLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneAnimationTransformNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneAnimationTransformNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneAnimation::VerifyAnimationTransformNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
			"Invalid animation transform node flatbuffer format.");
		return nullptr;
	}

	auto fbAnimationTransformNode = DeepSeaSceneAnimation::GetAnimationTransformNode(data);

	auto fbItemLists = fbAnimationTransformNode->itemLists();
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
				DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
					"Animation tree node item list name is null.");
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	dsSceneNode* node = (dsSceneNode*)dsSceneAnimationTransformNode_create(allocator,
		fbAnimationTransformNode->animationNode()->c_str(), itemLists, itemListCount);
	if (!node)
		return nullptr;

	auto fbChildren = fbAnimationTransformNode->children();
	if (fbChildren)
	{
		for (auto fbNode : *fbChildren)
		{
			if (!fbNode)
				continue;

			auto data = fbNode->data();
			dsSceneNode* child = dsSceneNode_load(allocator, resourceAllocator, loadContext,
				scratchData, fbNode->type()->c_str(), data->data(), data->size(),
				relativePathUserData, openRelativePathStreamFunc, closeRelativePathStreamFunc);
			if (!child)
			{
				dsSceneNode_freeRef(node);
				return nullptr;
			}

			bool success = dsSceneNode_addChild(node, child);
			dsSceneNode_freeRef(child);
			if (!success)
			{
				dsSceneNode_freeRef(node);
				return nullptr;
			}
		}
	}

	return node;
}
