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

#include <DeepSea/Scene/Nodes/SceneUserDataNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Scene/ItemLists/SceneUserDataList.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

static void dsSceneUserDataNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneUserDataNode_typeName = "UserDataNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneUserDataNode_type(void)
{
	return &nodeType;
}

dsSceneUserDataNode* dsSceneUserDataNode_create(dsAllocator* allocator, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc,
	dsCreateSceneInstanceUserDataFunction createInstanceDataFunc,
	dsDestroyUserDataFunction destroyInstanceDataFunc, const char* const* itemLists,
	uint32_t itemListCount)
{
	if (!allocator || !createInstanceDataFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneUserDataNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneUserDataNode* userDataNode = DS_ALLOCATE_OBJECT(allocator, dsSceneUserDataNode);
	if (!userDataNode)
		return NULL;

	dsSceneNode* node = (dsSceneNode*)userDataNode;

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize(node, allocator, dsSceneUserDataNode_type(), itemListsCopy,
			itemListCount, &dsSceneUserDataNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->userData = userData;
	node->destroyUserDataFunc = destroyUserDataFunc;

	userDataNode->createInstanceDataFunc = createInstanceDataFunc;
	userDataNode->destroyInstanceDataFunc = destroyInstanceDataFunc;
	return userDataNode;
}

void* dsSceneUserDataNode_getInstanceData(const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneUserDataNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsSceneUserDataList_type())
			return itemData->itemData[i].data;
	}

	return NULL;
}
