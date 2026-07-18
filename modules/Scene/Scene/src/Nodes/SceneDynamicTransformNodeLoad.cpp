/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneDynamicTransformNode.h>

#include "SceneLoadContextInternal.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/Types.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/DynamicTransformNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

extern "C"
dsSceneNode* dsSceneDynamicTransformNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyDynamicTransformNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid Dynamictransform node flatbuffer format.");
		return nullptr;
	}

	constexpr uint32_t maxStackItemLists = 16384;
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);

	auto fbTransformNode = DeepSeaScene::GetDynamicTransformNode(data);

	dsRigidTransform3f transform;
	const DeepSeaScene::Vector3f* fbScale = fbTransformNode->scale();
	if (fbScale)
		transform.scale = DeepSeaScene::convert3x(*fbScale);
	else
		transform.scale.x = transform.scale.y = transform.scale.z = transform.scale.w = 1.0f;

	const DeepSeaScene::Quaternion4f* fbOrientation = fbTransformNode->orientation();
	if (fbOrientation)
		transform.orientation = DeepSeaScene::convert(*fbOrientation);
	else
		dsQuaternion4_identityRotation(transform.orientation);

	const DeepSeaScene::Vector3f* fbPosition = fbTransformNode->position();
	if (fbPosition)
		transform.position = DeepSeaScene::convert3x(*fbPosition);
	else
	{
		transform.position.x = transform.position.y = transform.position.z = transform.position.w =
			0.0f;
	}

	auto fbItemLists = fbTransformNode->itemLists();
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
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Dynamic transform node item list name is null.");
				if (heapItemLists)
					DS_VERIFY(dsAllocator_free(scratchAllocator, itemLists));
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	auto node = reinterpret_cast<dsSceneNode*>(
		dsSceneDynamicTransformNode_create(allocator, &transform, itemLists, itemListCount));
	if (heapItemLists)
		DS_VERIFY(dsAllocator_free(scratchAllocator, itemLists));
	if (!node)
		return nullptr;

	auto fbChildren = fbTransformNode->children();
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
