/*
 * Copyright 2025 Aaron Barany
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

#include "FixtureBase.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Scene.h>

#include <gtest/gtest.h>

namespace
{

const char* testListNames[] = {"TestScene1", "TestScene2", "TestScene3", "TestScene4"};

void destroyMockNode(dsSceneNode* node)
{
	dsAllocator_free(node->allocator, node);
}

dsSceneNodeType createMockSceneNodeType()
{
	dsSceneNodeType type = {};
	type.destroyFunc = &destroyMockNode;
	return type;
}

dsSceneNodeType mockSceneNodeType = createMockSceneNodeType();

dsSceneNode* createMockNode(dsAllocator* allocator)
{
	dsSceneNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneNode);
	if (!node)
		return nullptr;

	EXPECT_TRUE(dsSceneNode_initialize(
		node, allocator, &mockSceneNodeType, testListNames, DS_ARRAY_SIZE(testListNames)));
	return node;
}

struct ItemInfo
{
	const dsSceneNode* node;
	uint32_t updateCount;
	uint64_t nodeID;
};

struct MockSceneItemList
{
	dsSceneItemList sceneItemList;

	uint32_t id;
	bool* isAlive;

	ItemInfo* items;
	uint32_t itemCount;
	uint32_t maxItems;
	uint64_t nextNodeID;

	uint64_t* removeItems;
	uint32_t removeItemCount;
	uint32_t maxRemoveItems;
};

uint64_t addMockSceneItem(dsSceneItemList* itemList, dsSceneNode* node, dsSceneTreeNode* treeNode,
	const dsSceneNodeItemData*, void**)
{
	if (!dsSceneNode_isOfType(node, &mockSceneNodeType))
		return DS_NO_SCENE_NODE;

	auto mockList = reinterpret_cast<MockSceneItemList*>(itemList);
	uint32_t index = mockList->itemCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, mockList->items, mockList->itemCount,
			mockList->maxItems, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	mockList->items[index].node = node;
	mockList->items[index].updateCount = 0;
	mockList->items[index].nodeID = mockList->nextNodeID;
	return mockList->nextNodeID++;
}

void removeMockSceneItem(dsSceneItemList* itemList, dsSceneTreeNode*, uint64_t nodeID)
{
	auto mockList = reinterpret_cast<MockSceneItemList*>(itemList);

	uint32_t index = mockList->removeItemCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, mockList->removeItems,
			mockList->removeItemCount, mockList->maxRemoveItems, 1))
	{
		mockList->removeItems[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingle(mockList->items, &mockList->itemCount,
			sizeof(ItemInfo), offsetof(ItemInfo, nodeID), nodeID);
	}
}

void updateMockSceneItem(dsSceneItemList* itemList, dsSceneTreeNode*, uint64_t nodeID)
{
	auto mockList = reinterpret_cast<MockSceneItemList*>(itemList);

	auto item = (ItemInfo*)dsSceneItemListEntries_findEntry(mockList->items,
		mockList->itemCount, sizeof(ItemInfo), offsetof(ItemInfo, nodeID), nodeID);
	if (item)
		++item->updateCount;
}

void updateMockSceneItems(dsSceneItemList* itemList, const dsScene*, float)
{
	auto mockList = reinterpret_cast<MockSceneItemList*>(itemList);

	// Lazily remove items.
	dsSceneItemListEntries_removeMulti(mockList->items, &mockList->itemCount,
		sizeof(ItemInfo), offsetof(ItemInfo, nodeID), mockList->removeItems,
		mockList->removeItemCount);
	mockList->removeItemCount = 0;
}

void commitMockSceneItems(dsSceneItemList*, const dsView*, dsCommandBuffer*)
{
}

uint32_t hashMockSceneItems(const dsSceneItemList* itemList, uint32_t commonHash)
{
	auto mockList = reinterpret_cast<const MockSceneItemList*>(itemList);
	return dsHashCombine32(commonHash, &mockList->id);
}

bool mockSceneItemsEqual(const dsSceneItemList* left, const dsSceneItemList* right)
{
	auto leftMockList = reinterpret_cast<const MockSceneItemList*>(left);
	auto rightMockList = reinterpret_cast<const MockSceneItemList*>(right);
	return leftMockList->id == rightMockList->id;
}

void destroyMockSceneItems(dsSceneItemList* itemList)
{
	auto mockList = reinterpret_cast<MockSceneItemList*>(itemList);
	*mockList->isAlive = false;
	EXPECT_TRUE(dsAllocator_free(itemList->allocator, mockList->items));
	EXPECT_TRUE(dsAllocator_free(itemList->allocator, mockList->removeItems));
	EXPECT_TRUE(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemListType createType()
{
	dsSceneItemListType type = {};
	type.addNodeFunc = &addMockSceneItem;
	type.updateNodeFunc = &updateMockSceneItem;
	type.removeNodeFunc = &removeMockSceneItem;
	type.updateFunc = &updateMockSceneItems;
	type.commitFunc = &commitMockSceneItems;
	type.hashFunc = &hashMockSceneItems;
	type.equalFunc = &mockSceneItemsEqual;
	type.destroyFunc = &destroyMockSceneItems;
	return type;
}

dsSceneItemListType type = createType();

MockSceneItemList* createMockSceneItems(
	dsAllocator* allocator, const char* name, uint32_t id, bool& isAlive)
{
	MockSceneItemList* mockItems = DS_ALLOCATE_OBJECT(allocator, MockSceneItemList);
	if (!mockItems)
		return nullptr;

	dsSceneItemList* baseItems = (dsSceneItemList*)mockItems;
	baseItems->allocator = dsAllocator_keepPointer(allocator);
	baseItems->type = &type;
	baseItems->name = name;
	baseItems->nameID = dsUniqueNameID_create(name);
	baseItems->globalValueCount = 0;
	baseItems->needsCommandBuffer = false;
	baseItems->skipPreRenderPass = false;

	mockItems->id = id;
	mockItems->isAlive = &isAlive;
	isAlive = true;

	mockItems->items = nullptr;
	mockItems->itemCount = 0;
	mockItems->maxItems = 0;
	mockItems->nextNodeID = 0;

	mockItems->removeItems = nullptr;
	mockItems->removeItemCount = 0;
	mockItems->maxRemoveItems = 0;

	return mockItems;
}

} // namespace

class SceneTest : public FixtureBase
{
};

TEST_F(SceneTest, CreateExistingScene)
{
	constexpr unsigned int firstListCount = 3;
	bool firstListsAlive[firstListCount];
	MockSceneItemList* firstMockLists[firstListCount];
	for (unsigned int i = 0; i < firstListCount; ++i)
	{
		firstMockLists[i] = createMockSceneItems(
			(dsAllocator*)&allocator, testListNames[i], i, firstListsAlive[i]);
		ASSERT_TRUE(firstMockLists[i]);
	}

	dsScenePipelineItem firstPipeline[firstListCount] =
	{
		{nullptr, (dsSceneItemList*)firstMockLists[0]},
		{nullptr, (dsSceneItemList*)firstMockLists[1]},
		{nullptr, (dsSceneItemList*)firstMockLists[2]}
	};

	dsScene* firstScene = dsScene_create((dsAllocator*)&allocator, renderer, nullptr, 0,
		firstPipeline, firstListCount, nullptr, nullptr, nullptr);
	ASSERT_TRUE(firstScene);

	dsSceneNode* mockNode1 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode1);
	dsSceneNode* mockNode2 = createMockNode((dsAllocator*)&allocator);
	ASSERT_TRUE(mockNode2);

	dsMatrix44f matrix1, matrix2;
	dsMatrix44f_makeRotate(&matrix1, M_PI_2f, -M_PI_4f, M_PIf);
	dsMatrix44f_makeTranslate(&matrix2, 3.2f, -5.3f, 1.3f);
	dsSceneTransformNode* transform1 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix1);
	ASSERT_TRUE(transform1);
	dsSceneTransformNode* transform2 =
		dsSceneTransformNode_create((dsAllocator*)&allocator, &matrix2);
	ASSERT_TRUE(transform2);

	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, mockNode1));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform1, (dsSceneNode*)transform2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode2));
	ASSERT_TRUE(dsSceneNode_addChild((dsSceneNode*)transform2, mockNode1));

	ASSERT_TRUE(dsScene_addNode(firstScene, (dsSceneNode*)transform1));
	EXPECT_TRUE(dsScene_update(firstScene, 0));

	auto transform1Node = reinterpret_cast<dsSceneNode*>(transform1);
	for (uint32_t i = 0; i < transform1Node->treeNodeCount; ++i)
		dsSceneTreeNode_markDirty(transform1Node->treeNodes[i]);
	EXPECT_TRUE(dsScene_update(firstScene, 0));

	for (unsigned int i = 0; i < firstListCount; ++i)
	{
		EXPECT_EQ(3U, firstMockLists[i]->itemCount);
		for (unsigned int j = 0; j < firstMockLists[i]->itemCount; ++j)
			EXPECT_EQ(1U, firstMockLists[i]->items[j].updateCount);
	}

	// First is same name and ID as previous list. Second is same node but different ID. Third is a
	// fully new list.
	constexpr unsigned int secondListCount = 3;
	bool secondListsAlive[secondListCount];
	uint32_t secondListIDs[secondListCount] = {1, 7, 3};
	MockSceneItemList* secondMockLists[secondListCount];
	for (unsigned int i = 0; i < secondListCount; ++i)
	{
		secondMockLists[i] = createMockSceneItems(
			(dsAllocator*)&allocator, testListNames[i + 1], secondListIDs[i], secondListsAlive[i]);
		ASSERT_TRUE(secondMockLists[i]);
	}

	dsScenePipelineItem secondPipeline[secondListCount] =
	{
		{nullptr, (dsSceneItemList*)secondMockLists[0]},
		{nullptr, (dsSceneItemList*)secondMockLists[1]},
		{nullptr, (dsSceneItemList*)secondMockLists[2]}
	};

	dsScene* secondScene = dsScene_create((dsAllocator*)&allocator, renderer, nullptr, 0,
		secondPipeline, secondListCount, nullptr, nullptr, firstScene);
	ASSERT_TRUE(secondScene);

	ASSERT_FALSE(firstListsAlive[0]);
	ASSERT_TRUE(firstListsAlive[1]);
	ASSERT_FALSE(firstListsAlive[2]);

	ASSERT_FALSE(secondListsAlive[0]);
	ASSERT_TRUE(secondListsAlive[1]);
	ASSERT_TRUE(secondListsAlive[2]);

	for (uint32_t i = 0; i < transform1Node->treeNodeCount; ++i)
		dsSceneTreeNode_markDirty(transform1Node->treeNodes[i]);
	EXPECT_TRUE(dsScene_update(secondScene, 0));

	MockSceneItemList* finalLists[secondListCount] =
		{firstMockLists[1], secondMockLists[1], secondMockLists[2]};

	for (unsigned int i = 0; i < secondListCount; ++i)
	{
		EXPECT_EQ(3U, finalLists[i]->itemCount);
		uint32_t expectedUpdateCount = finalLists[i] == secondMockLists[i] ? 1 : 2;
		for (unsigned int j = 0; j < finalLists[i]->itemCount; ++j)
			EXPECT_EQ(expectedUpdateCount, finalLists[i]->items[j].updateCount);
	}

	dsScene_destroy(secondScene);
	for (unsigned int i = 0; i < firstListCount; ++i)
		EXPECT_FALSE(firstListsAlive[i]);
	for (unsigned int i = 0; i < secondListCount; ++i)
		EXPECT_FALSE(secondListsAlive[i]);

	dsSceneNode_freeRef(mockNode1);
	dsSceneNode_freeRef(mockNode2);
	dsSceneNode_freeRef((dsSceneNode*)transform1);
	dsSceneNode_freeRef((dsSceneNode*)transform2);
}
