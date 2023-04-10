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

#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/NodeChildren_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

extern "C"
bool dsSceneNodeChildren_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyNodeChildrenBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid transform node children flatbuffer format.");
		return false;
	}

	auto fbNodeChildren = DeepSeaScene::GetNodeChildren(data);
	const char* nodeName = fbNodeChildren->node()->c_str();

	dsSceneNode* node;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&node),
			scratchData, nodeName) ||
		resourceType != dsSceneResourceType_SceneNode)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find node '%s'.", nodeName);
		return false;
	}

	auto fbChildren = fbNodeChildren->children();
	for (auto fbNode : *fbChildren)
	{
		if (!fbNode)
			continue;

		auto data = fbNode->data();
		dsSceneNode* child = dsSceneNode_load(allocator, resourceAllocator, loadContext,
			scratchData, fbNode->type()->c_str(), data->data(), data->size());
		if (!child)
			return false;

		bool success = dsSceneNode_addChild(node, child);
		dsSceneNode_freeRef(child);
		if (!success)
			return false;
	}

	return true;
}

