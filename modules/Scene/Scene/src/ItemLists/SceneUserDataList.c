/*
 * Copyright 2024-2025 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneUserDataList.h>

#include "SceneLoadContextInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneUserDataNode.h>
#include <DeepSea/Scene/Types.h>

#include <string.h>

typedef struct Entry
{
	const dsSceneUserDataNode* userDataNode;
	void* instanceData;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneUserDataList
{
	dsSceneItemList itemList;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;
} dsSceneUserDataList;

static uint64_t dsSceneUserDataList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsSceneUserDataList* userDataList = (dsSceneUserDataList*)itemList;

	if (!dsSceneNode_isOfType(node, dsSceneUserDataNode_type()))
		return DS_NO_SCENE_NODE;

	const dsSceneUserDataNode* userDataNode = (const dsSceneUserDataNode*)node;
	DS_ASSERT(userDataNode->createInstanceDataFunc);
	uint32_t index = userDataList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, userDataList->entries,
			userDataList->entryCount, userDataList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = userDataList->entries + index;
	entry->userDataNode = userDataNode;
	entry->instanceData = userDataNode->createInstanceDataFunc(treeNode, node->userData);
	*thisItemData = entry->instanceData;
	entry->nodeID = userDataList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneUserDataList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsSceneUserDataList* userDataList = (dsSceneUserDataList*)itemList;

	Entry* entry = (Entry*)dsSceneItemListEntries_findEntry(userDataList->entries,
		userDataList->entryCount, sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	if (!entry)
		return;

	const dsSceneUserDataNode* userDataNode = entry->userDataNode;
	if (userDataNode->destroyInstanceDataFunc)
		userDataNode->destroyInstanceDataFunc(entry->instanceData);

	uint32_t index = userDataList->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, userDataList->removeEntries,
			userDataList->removeEntryCount, userDataList->maxRemoveEntries, 1))
	{
		userDataList->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingleIndex(userDataList->entries, &userDataList->entryCount,
			sizeof(Entry), entry - userDataList->entries);
	}
}

static void dsSceneUserDataList_update(dsSceneItemList* itemList, const dsScene* scene, float time)
{
	DS_UNUSED(scene);
	DS_UNUSED(time);
	dsSceneUserDataList* userDataList = (dsSceneUserDataList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(userDataList->entries, &userDataList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), userDataList->removeEntries,
		userDataList->removeEntryCount);
	userDataList->removeEntryCount = 0;
}

static void dsSceneUserDataList_destroy(dsSceneItemList* itemList)
{
	dsSceneUserDataList* userDataList = (dsSceneUserDataList*)itemList;

	// Handle removed entries before clearing out their user data.
	dsSceneItemListEntries_removeMulti(userDataList->entries, &userDataList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), userDataList->removeEntries,
		userDataList->removeEntryCount);

	for (uint32_t i = 0; i < userDataList->entryCount; ++i)
	{
		Entry* entry = userDataList->entries + i;
		const dsSceneUserDataNode* userDataNode = entry->userDataNode;
		if (userDataNode->destroyInstanceDataFunc)
			userDataNode->destroyInstanceDataFunc(entry->instanceData);
	}

	DS_VERIFY(dsAllocator_free(itemList->allocator, userDataList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, userDataList->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneUserDataList_typeName = "UserDataList";

const dsSceneItemListType* dsSceneUserDataList_type(void)
{
	static dsSceneItemListType type =
	{
		.addNodeFunc = &dsSceneUserDataList_addNode,
		.removeNodeFunc = &dsSceneUserDataList_removeNode,
		.updateFunc = &dsSceneUserDataList_update,
		.destroyFunc = &dsSceneUserDataList_destroy
	};
	return &type;
}

dsSceneItemList* dsSceneUserDataList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return dsSceneUserDataList_create(allocator, name);
}

dsSceneItemList* dsSceneUserDataList_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene user data list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneUserDataList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneUserDataList* userDataList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneUserDataList);
	DS_ASSERT(userDataList);

	dsSceneItemList* itemList = (dsSceneItemList*)userDataList;
	itemList->allocator = allocator;
	itemList->type = dsSceneUserDataList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	userDataList->entries = NULL;
	userDataList->entryCount = 0;
	userDataList->maxEntries = 0;
	userDataList->nextNodeID = 0;

	return itemList;
}
