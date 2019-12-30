/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include "Flatbuffers/SceneNodeRef_generated.h"
#include "SceneLoadContextInternal.h"
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Flatbuffers/SceneCommon_generated.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

extern "C"
dsSceneNode* dsSceneNodeRef_load(const dsSceneLoadContext*, dsSceneLoadScratchData* scratchData,
	dsAllocator*, dsAllocator*, void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifySceneNodeRefBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid node reference flatbuffer format.");
		return nullptr;
	}

	auto fbNodeRef = DeepSeaScene::GetSceneNodeRef(data);
	const char* name = fbNodeRef->name()->c_str();
	dsSceneNode* node;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&node), scratchData, name) ||
		resourceType != dsSceneResourceType_Buffer)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't find node '%s'.", name);
		return nullptr;
	}

	return node;
}

extern "C"
dsSceneNode* dsSceneNode_load(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const char* type,
	const void* data, size_t size)
{
	auto foundType = reinterpret_cast<dsLoadSceneNodeItem*>(
		dsHashTable_find(&loadContext->nodeTypeTable.hashTable, type));
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown scene node type '%s'.", type);
		return NULL;
	}

	dsSceneNode* node = foundType->loadFunc(loadContext, scratchData, allocator, resourceAllocator,
		foundType->userData, reinterpret_cast<const uint8_t*>(data), size);
	if (!node)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load scene node '%s': %s.", type,
			dsErrorString(errno));
	}
	return node;
}
